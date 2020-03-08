OPEN SOURCE EZ-USB WAVEFORM COMPILER
====================================


This is a simple compiler that generates GPIF wave tables. 
This program accepts the source code from stdin and generates the C code on stdout.
Listing and errors are put to stderr.

SOURCE CODE FORMAT (UPPERCASE ONLY):


     ; Comments..

        .PSEUDOOP	<arg>                   ; Comment
        ...
        OPCODE          operand1 ... operandn   ; comment
    
     PSEUDO OPS:
    
        .IFCLKSRC       { 0 | 1 }               ; IFCLKSRC, 0: ext, 1: int (30/48 MHz)
        .MHZ3048        { 0 | 1 }               ; 3048MHZ, 0: 30MHz, 1: 48MHz
        .IFCLKOE        { 0 | 1 }               ; IFCLKOE, 0: tri-state, 1: drive
        .TRICTL         { 0 | 1 }               ; Affects Outputs
        .GPIFREADYCFG5  { 0 | 1 }               ; TC when 1, else RDY5
        .GPIFREADYCFG7  { 0 | 1 }               ; INTRDY available when 1
        .EPXGPIFFLGSEL  { PF | EF | FF }        ; Selected FIFO flag
        .EP             { 2 | 4 | 6 | 8 }       ; Default 2
        .WAVEFORM       n                       ; Names output C code array
    
     NDP (non decision point) OPCODES:
        [S][+][G][D][N]         [count=1] [OEn] [CTLn]
     or Z                       [count=1] [OEn] [CTLn]

     DP (decision point) OPCODES:
        J[S][+][G][D][N][*]     A OP B [OEn] [CTLn] $1 $2
     where:
        A/B is one of:          RDY0 RDY1 RDY2 RDY3 RDY4 RDY5 TC PF EF FF INTRDY
                                  These are subject to environment.
     and  OP is one of:         AND OR XOR /AND (/A AND B)

     OPCODE CHARACTERS:
        S       SGL (Single)
        +       INCAD
        G       GINT
        D       Data
        N       Next/SGLCRC
        Z       Placeholder when none of the above
        *       Re-execute (DP only)




DECOMPILING:
============

There is limited capability to decompile a gpif.c module into
 "assembler form". This decompile is unable to know when
TRICTL is in effect, unless OE3 or OE2 are referenced. Other
modes such as PF|EF|FF are also not known but emitted as such.

To decompile, specify a file name:

    $ ./ezusbcc gpif.c
    128 bytes.
    ; WaveForm 0
    01000007    Z       1 CTL2 CTL1 CTL0 
    02000002    Z       2 CTL1 
    01020002    D       1 CTL1 
    01000007    Z       1 CTL2 CTL1 CTL0 
    3F013F07    J       INTRDY AND INTRDY CTL2 CTL1 CTL0 $7 $7
    01000007    Z       1 CTL2 CTL1 CTL0 
    01000007    Z       1 CTL2 CTL1 CTL0 
    ; WaveForm 1
    03020005    D       3 CTL2 CTL0 
    01020007    D       1 CTL2 CTL1 CTL0 
    3F053F07    JN      INTRDY AND INTRDY CTL2 CTL1 CTL0 $7 $7
    01000007    Z       1 CTL2 CTL1 CTL0 
    01000007    Z       1 CTL2 CTL1 CTL0 
    01000007    Z       1 CTL2 CTL1 CTL0 
    01000007    Z       1 CTL2 CTL1 CTL0 
    ; WaveForm 2
    ...
