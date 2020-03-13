//////////////////////////////////////////////////////////////////////
// ezusbcc.cpp -- GPIF assembler for EZ-USB
// Date: Thu Mar 22 22:29:43 2018   (C) Warren W. Gay VE3WWG
///////////////////////////////////////////////////////////////////////
//

union u_opcode {
	uint8_t			byte;
	struct s_opcode {
		uint8_t	dp : 1;		// 1 == DP
		uint8_t	data : 1;	// 1 == Drive FIFO / Sample
		uint8_t	next : 1;	// 1 == move next to FIFO, or use UDMACRCH:L
		uint8_t	incad : 1;	// 1 == increment GPIFADR
		uint8_t	gint : 1;	// 1 == generate a GPIFWF
		uint8_t	sgl : 1;	// 1 == use SGLDATH:L / UDMACRCH:L
		uint8_t	reserved : 2;	// Ignored
	}			bits;
};

union u_logfunc {
	enum class e_logfunc {
		a_and_b = 0,
		a_or_b = 1,
		a_xor_b = 2,
		na_and_b = 3
	};
	uint8_t			byte;
	struct s_logfunc {
		uint8_t	termb : 3;	// TERM B
		uint8_t	terma : 3;	// TERM A
		uint8_t	lfunc : 2;	// e_log_func
	}			bits;
};

union u_branch {
	uint8_t			byte;
	struct s_branch {
		uint8_t	branchon0 : 3;
		uint8_t	branchon1 : 3;
		uint8_t	reserved  : 1;
		uint8_t	reexecute : 1;
	}			bits;
};

union u_output {
	uint8_t			byte;
	struct s_output1 {
		uint8_t	ctl0 : 1;
		uint8_t	ctl1 : 1;
		uint8_t	ctl2 : 1;
		uint8_t	ctl3 : 1;
		uint8_t	oes0 : 1;
		uint8_t	oes1 : 1;
		uint8_t	oes2 : 1;
		uint8_t	oes3 : 1;
	}			bits1;
	struct s_output0 {
		uint8_t	ctl0 : 1;
		uint8_t	ctl1 : 1;
		uint8_t	ctl2 : 1;
		uint8_t	ctl3 : 1;
		uint8_t	ctl4 : 1;
		uint8_t	ctl5 : 1;
		uint8_t	reserved : 2;
	}			bits0;
};
#if 0
struct s_instr {
	std::string		stropcode;
	std::vector<std::string> stroperands;
	std::string		strcomment;
	std::string		error;

	u_branch		branch;
	u_opcode		opcode;
	u_logfunc		logfunc;
	u_output		output;

	void clear() {
		stropcode.clear();
		stroperands.clear();
		strcomment.clear();
		opcode.byte = 0;
		logfunc.byte = 0;
		branch.byte = 0;
		output.byte = 0;
	};
};
#endif


// End gpif.h
