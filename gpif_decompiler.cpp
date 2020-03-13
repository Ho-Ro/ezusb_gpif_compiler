//////////////////////////////////////////////////////////////////////
// ezusbcc.cpp -- GPIF assembler for EZ-USB
// Date: Thu Mar 22 22:29:43 2018   (C) Warren W. Gay VE3WWG
///////////////////////////////////////////////////////////////////////
//
// This is a simple assember, to generate wave tables. This program
// accepts the source code from stdin and generates the C code on
// stdout. Listing and errors are put to stderr.
//
// TO DECOMPILE:
//
//
//    specify one or more filename(s), for example:
//
//    $ ./gpif_decompile gpif1.c [gpif2.c] ...
//
// Note that the decompile doesn't figure out the environment
// that it runs within. As a result, some values will show as
// RDY5|TC or PF|EF|FF where it can't know. It may also get
// the OEx CTLx wrong. If it sees OE3 or OE2, it will assume
// from that point on that TRICTL is in effect.

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "gpif.h"


static void
decompile(unsigned waveformx,uint8_t data[32]) {
	unsigned ux;
	bool trictl = false;

	std::cout << "; WaveForm " << waveformx << '\n';

	for ( ux=0; ux<32-4; ux += 4 ) {
		u_branch branch;
		u_opcode opcode;
		u_logfunc logfunc;
		u_output output;
		std::stringstream opc, oper;

		branch.byte= data[ux+0];
		opcode.byte = data[ux+1];
		logfunc.byte = data[ux+2];
		output.byte = data[ux+3];

		if ( opcode.bits.dp == 1 )
			opc << 'J';
		if ( opcode.bits.sgl )
			opc << 'S';
		if ( opcode.bits.incad )
			opc << '+';
		if ( opcode.bits.gint )
			opc << 'G';
		if ( opcode.bits.next )
			opc << 'N';
		if ( opcode.bits.data )
			opc << 'D';
		if ( !opcode.byte )
			opc << 'Z';
		if ( opcode.bits.dp && branch.bits.reexecute )
			opc << '*';

		if ( output.bits1.oes3 || output.bits1.oes2 )
			trictl = true;		// Assume TRICTL

		auto outs = [&]() {
			if ( trictl ) {
				if ( output.bits1.oes3 )
					oper << " OES3";
				if ( output.bits1.oes2 )
					oper << " OES2";
				if ( output.bits1.oes1 )
					oper << " OES1";
				if ( output.bits1.oes0 )
					oper << " OES0";
				if ( output.bits1.ctl3 )
					oper << " CTL3";
				if ( output.bits1.ctl2 )
					oper << " CTL2";
				if ( output.bits1.ctl1 )
					oper << " CTL1";
				if ( output.bits1.ctl0 )
					oper << " CTL0";
			} else	{
				if ( output.bits0.ctl5 )
					oper << " CTL5";
				if ( output.bits0.ctl4 )
					oper << " CTL4";
				if ( output.bits0.ctl3 )
					oper << " CTL3";
				if ( output.bits0.ctl2 )
					oper << " CTL2";
				if ( output.bits0.ctl1 )
					oper << " CTL1";
				if ( output.bits0.ctl0 )
					oper << " CTL0";
			}
		};

		if ( opcode.bits.dp == 0 ) {
			// NDP

			if ( branch.byte == 0 )
				oper << "256 ";
			else	oper << unsigned(branch.byte) << ' ';
			outs();

		} else	{
			auto aorb = [&](unsigned term) {
				switch ( term ) {
				case 0:
					oper << "RDY0 ";
					break;
				case 1:
					oper << "RDY1 ";
					break;
				case 2:
					oper << "RDY2 ";
					break;
				case 3:
					oper << "RDY3 ";
					break;
				case 4:
					oper << "RDY4 ";
					break;
				case 5:
					oper << "RDY5|TC ";
					break;
				case 6:
					oper << "PF|EF|FF ";
					break;
				case 7:
					oper << "INTRDY ";
					break;
				}
			};
			aorb(logfunc.bits.terma);
			switch ( logfunc.bits.lfunc ) {
			case 0b00:
				oper << "AND ";
				break;
			case 0b01:
				oper << "OR ";
				break;
			case 0b10:
				oper << "XOR ";
				break;
			case 0b11:
				oper << "/AND ";
				break;
			}
			aorb(logfunc.bits.termb);

			oper << "$" << unsigned(branch.bits.branchon1)
				<< " $" << unsigned(branch.bits.branchon0);

			outs();
		}

		std::cout.width(2);
		std::cout.fill('0');
		std::cout << std::uppercase << std::hex << unsigned(branch.byte);
		std::cout.width(2);
		std::cout.fill('0');
		std::cout << std::uppercase << std::hex << unsigned(opcode.byte);
		std::cout.width(2);
		std::cout.fill('0');
		std::cout << std::uppercase << std::hex << unsigned(logfunc.byte);
		std::cout.width(2);
		std::cout.fill('0');
		std::cout << std::uppercase << std::hex << unsigned(output.byte)
			<< '\t' << opc.str()
			<< '\t' << oper.str() << '\n';
	}
}

static void
decompile(const char *path) {
	std::ifstream gpif_c;
	char buf[2048];
	bool foundf = false;

	gpif_c.open(path,std::ifstream::in);
	if ( gpif_c.fail() ) {
		fprintf(stderr,"%s: Opening %s for read\n",
			strerror(errno),path);
		exit(1);
	}

	while ( gpif_c.good() ) {
		if ( !gpif_c.getline(buf,sizeof buf).good() )
			break;
		if ( !strncmp(buf,"const char xdata WaveData[128] =",32) ) {
			foundf = true;
			break;
		}
	}

	if ( !foundf ) {
		std::cerr << "Did not find line: 'const char xdata WaveData[128] =' in " << path << '\n';
		gpif_c.close();
		exit(1);
	}

	char ch;

	foundf = false;
	while ( gpif_c.good() ) {
		ch = gpif_c.get();
		if ( !gpif_c.good() )
			break;
		if ( ch == '{' ) {
			foundf = true;
			break;
		}
	}

	if ( !foundf ) {
		std::cerr << "Missing opening brace: " << path << '\n';
		gpif_c.close();
		exit(1);
	}

	std::vector<uint8_t> raw;
	std::stringstream sbuf;

	while ( gpif_c.good() ) {
		ch = gpif_c.get();
		if ( ch == '/' ) {
			if ((ch = gpif_c.get()) == '/' ) {
				while ( gpif_c.good() && gpif_c.get() != '\n' )
					;
				continue;
			} else if ( ch == '*' ) {
				do	{
					while ( gpif_c.good() && (ch = gpif_c.get()) != '*' )
						;
					if ( !gpif_c.good() )
						break;
					ch = gpif_c.get();
				} while ( gpif_c.good() && ch != '/' );
				if ( !gpif_c.good() )
					break;
				continue;
			}
		}
		if ( ch == '}' )
			break;
		if ( strchr("\n\r\t\b ",ch) != nullptr )
			continue;

		if ( ch != ',' ) {
			sbuf << ch;
		} else	{
			std::string data(sbuf.str());
			sbuf.clear();
			sbuf.str("");

			char *ep;
			unsigned long udata = strtoul(data.c_str(),&ep,0);

			if ( (ep && *ep != 0) || udata > 0xFF ) {
				std::cerr << "Invalid data: '" << data << "'\n";
				gpif_c.close();
				exit(1);
			}
			raw.push_back(uint8_t(udata));
		}
	}

	std::cout << raw.size() << " bytes.\n";
	gpif_c.close();

	switch ( raw.size() ) {
	case 32:
	case 64:
	case 96:
	case 128:
		break;
	default:
		std::cerr << "Unusual data size! Extraction failed.\n";
		exit(1);
	}

	uint8_t unpacked[32];

	memset(unpacked,0,sizeof unpacked);
	for ( unsigned ux=0; ux < raw.size(); ux += 32 ) {
		unsigned lx = ux;		// Length index
		unsigned opx = lx + 8;		// Opcode index
		unsigned otx = opx + 8;		// Output index
		unsigned lfx = otx + 8;		// Logical function index

		for ( unsigned bx=0; bx < 32; ) {
			unpacked[bx++] = raw[lx++];
			unpacked[bx++] = raw[opx++];
			unpacked[bx++] = raw[lfx++];
			unpacked[bx++] = raw[otx++];
		}
		decompile(ux/32,unpacked);
	}

	exit(0);
}

int main (int argc,char **argv) {

	for ( int ax=1; ax < argc; ++ax )
		decompile(argv[ax]);

	return 0;
}

// End gpif_decompile.cpp
