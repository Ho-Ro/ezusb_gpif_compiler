#include <stdio.h>
#include <stdlib.h>


static unsigned char* get_waveform( FILE *infile) {
	static unsigned char waveform[ 32 ];
	char one_line[ 256 ];
	char *lptr = one_line;
	unsigned char *wptr = waveform;
	int value;

	while ( fgets( one_line, 255, infile ) ) { // get one line
		lptr = one_line;
		fputs( lptr, stderr );
		while ( *lptr == ' ' || *lptr == '\t' || *lptr == '\n' ) {
			++lptr; // skip whitespace and empty lines etc.
		}
		if ( *lptr != '0' || *(lptr+1) != 'x' )
			continue; //parse only lines wstarting with 0x...
		while( true ) {
			value = strtoul( lptr, &lptr, 0 );
			if ( ! *++lptr ) // until EOL
				break;
			*wptr++ = value; // store next byte
			// printf( "value: %d\n", value );
			// printf( "lptr-> :%s:\n", lptr );
		}
	}
	return waveform;
}



int main() {


	char term[][8] = {
		"  RDY0 ",
		"  RDY1 ",
		"  RDY2 ",
		"  RDY3 ",
		"  RDY4 ",
		"  RDY5 ",
		"  FIFO ",
		" INTRDY",
	};

	char lfunc[][5] = {
		" AND",
		" OR ",
		" XOR",
		"/AND",
	};


	unsigned char *len_br = get_waveform( stdin );
	unsigned char *opcode = len_br + 8;
	unsigned char *output = len_br + 16;
	unsigned char *logic  = len_br + 24;

	printf(   "                  " );
	for ( int state = 0; state < 7; ++state )
		printf( " State %d  ", state );
	printf(   "\n                 " );
	for ( int state = 0; state < 7; ++state )
		printf( " ---------" );
	printf( "\nInc Addr:      " ); // INCAD
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00001000 )
			printf( "       ++ " );
		else
			printf( "          " );
	}
	printf( "\nDataMode:      " ); // DATA
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00000010 )
			printf( "      DATA" );
		else
			printf( "          " );
	}
	printf( "\nNextData:       " ); // NEXT/SGLCRC
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00100000 ) {
			if ( opcode[ state ] & 0b00000100 )
				printf( "   UDMACRC" );
			else
				printf( "    SGLDAT" );
		} else {
			if ( opcode[ state ] & 0b00000100 )
				printf( "      ++  " );
			else
				printf( "          " );
		}
	}
	printf( "\nInt Trig:      " ); // GINT
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00010000 )
			printf( "      INT4" );
		else
			printf( "          " );
	}

	printf( "\ncycles:         " ); // LENGTH/BRANCH
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00000001 ) { // DP
			printf( "       1  " );
		} else { // NDP
			if ( len_br[ state ] )
				printf( "     %3d  ", len_br[ state ] );
			else
				printf( "      256 " );
		}
	}
	printf( "\n                " ); // LENGTH/BRANCH
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00000001 ) { // DP
			printf( "      if  " );
		} else {
			printf( "          " );
		}
	}
	printf( "\n                " ); // LENGTH/BRANCH
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00000001 ) { // DP
			printf( "   %s", term[ (logic[ state ] >> 3) & 0x07 ] );
		} else { // NDP
			printf( "          " );
		}
	}
	printf( "\n                " ); // LENGTH/BRANCH
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00000001 ) { // DP
			printf( "     %s ", lfunc[ (logic[ state ] >> 6) & 0x03 ] );
		} else { // NDP
			printf( "          " );
		}
	}
	printf( "\n                " ); // LENGTH/BRANCH
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00000001 ) { // DP
			printf( "   %s", term[ logic[ state ] & 0x07 ] );
		} else { // NDP
			printf( "          " );
		}
	}
	printf( "\n                " ); // LENGTH/BRANCH
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00000001 ) { // DP
			printf( "    then %d", (len_br[ state ] >> 3) & 0x07 );
		} else { // NDP
			printf( "          " );
		}
	}
	printf( "\n                " ); // LENGTH/BRANCH
	for ( int state = 0; state < 7; ++state ) {
		if ( opcode[ state ] & 0b00000001 ) { // DP
			printf( "    else %d", len_br[ state ] & 0x07 );
		} else { // NDP
			printf( "          " );
		}
	}
	printf( "\nReExecute:      " ); // LENGTH/BRANCH
	for ( int state = 0; state < 7; ++state ) {
		if ( ( opcode[ state ] & 0b00000001 ) && len_br[ state ] & 0x80 ) { // DP
			printf( "    ReExec");
		} else { // NDP
			printf( "          " );
		}
	}

	for ( int ctl = 0; ctl < 4; ++ctl ){
		printf( "\nCTL%d:         ", ctl );
		for ( int state = 0; state < 7; ++state ) {
			switch ( ( output[ state ] >> ctl ) & 0b00010001 ) {
			    case 0x00:
			    case 0x01:
				printf( "          " );
				break;
			    case 0x10:
				printf( "         0" );
				break;
			    case 0x11:
				printf( "         1" );
				break;
			}
		}
	}
	printf( "\n" );

	return 0;
}


