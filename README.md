OPEN SOURCE EZ-USB WAVEFORM COMPILER
====================================

[![Build status](https://ci.appveyor.com/api/projects/status/github/Ho-Ro/ezusb_gpif_compiler?branch=master&svg=true)](https://ci.appveyor.com/project/Ho-Ro/ezusb-gpif-compiler/branch/master)

This is a simple command-line compiler that generates GPIF wave tables for Cypress EZ-USB® devices.

## Building instructions
If you want to build this tool from source you need the package `libusb-1.0-0-dev`.
To build the binaries simply type `make`

If you want to build a simple Debian package that can be installed and uninstalled cleanly,
you need also the packages `fakeroot` and `checkinstall` (checkinstall is currently not in *buster* but it's available in [*buster-backports*](https://packages.debian.org/de/buster-backports/checkinstall)
as well as in *stretch*, *bullseye* and *sid*). To build the Debian package type `make deb`.
This can be done as user, but you must be root to install the package, e.g. `sudo apt install ./ezusb-gpif-compiler_*.deb`.

This tool is developed and built on a Debian stable system. An experimental Debian package from latest CI build (on an Ubuntu system) is available at [appveyor](https://ci.appveyor.com/project/Ho-Ro/ezusb-gpif-compiler/build/artifacts).

## Source code format (UPPERCASE ONLY):

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
        .EP             { 2 | 4 | 6 | 8 }       ; Select endpoint, default=2 (unused)
        .WAVEFORM       n                       ; Names output C code array

     NDP (non decision point) OPCODES:
        [S][+][G][D][N]         [count=1] [OEn] [CTLn]
     or Z                       [count=1] [OEn] [CTLn]

     DP (decision point) OPCODES:
        J[S][+][G][D][N][*]     A OP B $1 $2 [OEn] [CTLn]
     where:
        A/B is one of:          RDY0 RDY1 RDY2 RDY3 RDY4 RDY5 TC PF EF FF INTRDY
                                  These are subject to environment.
     and OP is one of:          AND OR XOR /AND (/A AND B)
                                If ( A OP B ) then
                                    goto $1 // ($1 = 0..7)
                                else
                                    goto $2 // ($2 = 0..7)

     OPCODE CHARACTERS:
        S       SGL (Single)
        +       INCAD
        G       GINT
        D       Data
        N       Next/SGLCRC
        Z       Placeholder when none of the above
        *       Re-execute (DP only)


## Compiling

This program accepts the source code from stdin and generates the C code on stdout.
Listing and errors are put to stderr.

`cat testwave.wvf`

    ; Test waveform file for gpif_compiler.cpp
    ;
    ; Comment header
    ;
            .TRICTL         1               ; Assume TRICTL=1
            .EP             4               ; Assume for Endpoint 4
            .WAVEFORM       7               ; Name this waveform7
            .EPXGPIFFLGSEL  EF
            SG+DN                           ; Simple NDP
            J       RDY1 AND RDY1 $4 $2 OE3 ; DP example
            S+GDN   1 OE3 OE1 CTL3 CTL2
            Z       20
            JS+GDN* RDY0 AND RDY4 $4 $5
            JSG     RDY0 XOR RDY2 $1 $7 OE3 CTL2
            JSG     RDY0 /AND EF $0 $5 OE3 CTL1
    ; End

`./gpif_compiler <testwave.wvf >testwave.inc`

    ;
    ;       Environment in effect:
    ;
            .IFCLKSRC       1
            .3048MHZ        0
            .IFCLKOE        0
            .TRICTL 1
            .GPIFREADYCFG5  0
            .GPIFREADYCFG7  0
            .EPXGPIFFLGSEL  EF
            .EP     4
            .WAVEFORM       7
    ;
    $0  013E0000    SG+DN           ;  Simple NDP
    $1  20010980    J       RDY1 AND RDY1 $4 $2 OE3         ;  DP example
    $2  013E00AC    S+GDN   1 OE3 OE1 CTL3 CTL2
    $3  14000000    Z       20
    $4  953F0400    JS+GDN* RDY0 AND RDY4 $4 $5
    $5  0F318284    JSG     RDY0 XOR RDY2 $1 $7 OE3 CTL2
    $6  0531C682    JSG     RDY0 /AND EF $0 $5 OE3 CTL1

`cat testwave.inc`

    static const unsigned char ifconfig_7 = 0x8a;

    static const unsigned char waveform_7[ 32 ] = {
            0x01,0x22,0x01,0x14,0xA5,0x0F,0x05,0x00,
            0x3E,0x01,0x3E,0x00,0x3F,0x31,0x31,0x00,
            0x00,0x80,0xAC,0x00,0x00,0x84,0x82,0x00,
            0x00,0x09,0x00,0x00,0x04,0x82,0xC6,0x00,
    };


## Decompiling

There is limited capability to decompile a gpif.c module into
 "assembler form". This decompile is unable to know when
TRICTL is in effect, unless OE3 or OE2 are referenced. Other
modes such as PF|EF|FF are also not known but emitted as such.

To decompile, specify a file name:

    $ ./gpif_decompiler testgpif.c
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

## Show the structure of the GPIF

The program gpif_show displays the GPIF structure similar to the picture in the TRM (fig. 10-12).
It takes the output of gpif_compile as input on stdin and displays the result on stdout with additional info on stderr.

`./gpif_show <testwave.inc`

stderr:

    static const unsigned char ifconfig_7 = 0x8a;

    static const unsigned char waveform_7[ 32 ] = {
        0x01,0x22,0x01,0x14,0xA5,0x0F,0x05,0x00,
        0x3E,0x01,0x3E,0x00,0x3F,0x31,0x31,0x00,
        0x00,0x80,0xAC,0x00,0x00,0x84,0x82,0x00,
        0x00,0x09,0x00,0x00,0x04,0x82,0xC6,0x00,
    };

stdout:

    State:                $0        $1        $2        $3        $4        $5        $6
                       --------  --------  --------  --------  --------  --------  --------
    Inc Addr:             ++                  ++                  ++
    DataMode:            DATA                DATA                DATA
    NextData:          UDMACRC             UDMACRC             UDMACRC    SGLDAT    SGLDAT
    Int Trig:            INT4                INT4                INT4      INT4      INT4
    cycles:                1         1         1        20         1         1         1
                                    if                            if        if        if
                                   RDY1                          RDY0      RDY0      RDY0
                                    AND                           AND       XOR      /AND
                                   RDY1                          RDY4      RDY2      FIFO
                                  then 4                        then 4    then 1    then 0
                                  else 2                        else 5    else 7    else 5
    ReExecute:                                                  ReExec
    CTL0:
    CTL1:                                      0
    CTL2:
    CTL3:                            0         1                             0         0


# HowTo: Create GPIF waveform files for the `gpif-compiler`

The files in the `examples` directory are based on the real hardware of the Hantek6022BE, this is a cheap digital storage scope.
It consists mainly of an EzUSB with a dual channel 8-bit ADC [AD9288](https://www.analog.com/media/en/technical-documentation/data-sheets/AD9288.pdf).
The ADC can operate on variable clock frequencies (20kHz..48MHz) coming from either the IFCLK or CTL2 output of the EzUSB to sample signals of different speeds.
The waveform files are used in the open source [firmware](https://github.com/Ho-Ro/Hantek6022API/tree/master/PyHT6022/HantekFirmware)
for the DSO program [OpenHantek6022](https://github.com/OpenHantek/OpenHantek6022).


## Hantek6022BE ADC backend

             .---------.           .---------------.
             | AD9288  |           |     EzUSB     |
             |         |           |               |
             |         |     8     |               |
    CH0 ---->|  DA0..7 |=====/====>| PB0..7        |
             |         |           |               |
             |         |           |               |
             |    ENCA |<----+-----| IFCLK         |
             |         |     |     |               |
             |         |     |     |          USB  |<====>
             |         |     |     |               |
             |    ENCB |<----+-----| CTL2          |
             |         |           |               |
             |         |     8     |               |
    CH1 ---->|  DB0..7 |=====/====>| PD0..7        |
             |         |           |               |
             |         |           |               |
             '---------'           '---------------'


## CTLx setting
These values need to be set globally once at device init.

	GPIFIDLECTL = 0x00; // Don't enable CTLx outputs
	GPIFCTLCFG  = 0x80; // TRICTL=1. CTL0..3: CMOS outputs, tri-statable


## IFCONFIG settings
To be adapted individually for each sample rate, IFCONFIG[7..5] can be set by pseudo opcodes.

        // IFCONFIG.7 : IFCLKSRC, 0: ext, 1: int (30/48 MHz)
        // IFCONFIG.6 : 3048MHZ,  0: 30MHz, 1: 48MHz
        // IFCONFIG.5 : IFCLKOE,  0: tri-state, 1: drive
        // IFCONFIG.4 : IFCLKPOL, 0: normal polarity, 1: inverted
        // IFCONFIG.3 : ASYNC,    0: synchronously, ext. clock supplied to IFCLK pin,
        //                        1: asynchronously, FIFO provides r/w strobes
        // IFCONFIG.2 : GSTATE,   1: PE.[10] = GSTATE.[10]
        // IFCONFIG.[10] :       00: ports,
        //                       01: reserved,
        //                       10: GPIF (internal) master,
        //                       11: slave FIFO (external master)


## Timing definition examples for different sample rates

        //
        // The program for fastest speed 30/48 MS/s is:
        // jump 0,  CTL2=Z, FIFO, LOOP
        //
        // The program for 24 MS/s (@48MHz) or 16 MS/s (@30MHz) is:
        // wait 1,  CTL2=0, OE2=1, FIFO
        // jump 0,  CTL2=1, OE2=1
        //
        // The program for low-speed, e.g. 500 kS/s (@30MHz), is:
        // wait 30, CTL2=0, OE2=1, FIFO
        // wait 29, CTL2=1, OE2=1
        // jump 0,  CTL2=1
        //
        // The program for very low-speed, e.g. 20 kS/s (@30MHz), is:
        // wait 250, CTL2=0, OE2=1, FIFO
        // wait 250, CTL2=1, OE2=1
        // wait 250, CTL2=1, OE2=1
        // wait 250, CTL2=1, OE2=1
        // wait 250, CTL2=1, OE2=1
        // wait 249, CTL2=1, OE2=1
        // jump 0,   CTL2=1
        //


## Sample waveform files

My naming convention from the real application is:
- all Mega-sample-values are simple numbers, e.g. 1MS/s -> 1, 16 MS/s -> 16
- all kilo-sample-values are 100 + k-sample-value/10, e.g. 500 kS/s -> 150, 20 kS/s -> 102

### 30 MS/s waveform file example
Save this as `gpif_30.wvf`

	.WAVEFORM       30              ; Name this waveform

	.TRICTL         1               ; Assume TRICTL=1

	.IFCLKSRC       1               ; feed internal 30/48 MHz clock to the GPIF
	.3048MHZ        0               ; select 30 MHz
	.IFCLKOE        1               ; IFCLK active output drives the ADC

	JD*     RDY0 AND RDY0 $0 $0                     ; 1 cycle, CTL2 tri-state, repeat, jp 0
	                                                ; 1 cycle total @30MHz -> 30 MS/s

Execute `gpif_compiler < gpif_30.wvf > gpif_30.inc` to create the C include file `gpif_30.inc`

	static const unsigned char ifconfig_30 = 0xaa;

	static const unsigned char waveform_30[ 32 ] = {
		0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};



### 24 MS/s waveform file example
Save this as `gpif_24.wvf`

	.WAVEFORM       24              ; Name this waveform

	.TRICTL         1               ; Assume TRICTL=1

	.IFCLKSRC       1               ; feed internal 30/48 MHz clock to the GPIF
	.3048MHZ        1               ; select 48 MHz
	.IFCLKOE        0               ; IFCLK tri-state, CTL2 drives the ADC

	D       1                       OE2             ; 1 cycles, CTL2 active and low, DATA
	J       RDY0 AND RDY0 $0 $0     CTL2 OE2        ; 1 cycle,  CTL2 active and high, jp 0
	                                                ; 2 cycles @48MHz -> 24 MS/s

Execute `gpif_compiler < gpif_24.wvf > gpif_24.inc` to create the C include file `gpif_24.inc`

	static const unsigned char ifconfig_24 = 0xca;

	static const unsigned char waveform_24[ 32 ] = {
		0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
		0x40,0x44,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};



### 500 kS/s waveform file example
Save this as `gpif_150.wvf` and execute `gpif_compiler < gpif_150.wvf > gpif_150.inc` to create the C include file

	.WAVEFORM       150             ; Name this waveform

	.TRICTL         1               ; Assume TRICTL=1

	.IFCLKSRC       1               ; feed internal 30/48 MHz clock to the GPIF
	.3048MHZ        0               ; select 30 MHz
	.IFCLKOE        0               ; IFCLK tri-state, CTL2 drives the ADC

	D       30                      OE2             ; 30 cycles, CTL2 active and low, DATA
	Z       29                      CTL2 OE2        ; 29 cycles, CTL2 active and high
	J       RDY0 AND RDY0 $0 $0     CTL2 OE2        ;  1 cycle,  CTL2 active and high, jp 0
	                                                ; 60 cycles @30MHz -> 500 kS/s


### 20 kS/s waveform file example
Save this as `gpif_102.wvf` and execute `gpif_compiler < gpif_102.wvf > gpif_102.inc` to create the C include file

	.WAVEFORM       102             ; Name this waveform

	.TRICTL         1               ; Assume TRICTL=1

	.IFCLKSRC       1               ; feed internal 30/48 MHz clock to the GPIF
	.3048MHZ        0               ; select 30 MHz
	.IFCLKOE        0               ; IFCLK tri-state, CTL2 drives the ADC

	D       250                     OE2             ; 250 cycles, CTL2 active and low, DATA
	Z       250                     CTL2 OE2        ; 250 cycles, CTL2 active and high
	Z       250                     CTL2 OE2        ; 250 cycles, CTL2 active and high
	Z       250                     CTL2 OE2        ; 250 cycles, CTL2 active and high
	Z       250                     CTL2 OE2        ; 250 cycles, CTL2 active and high
	Z       249                     CTL2 OE2        ; 249 cycles, CTL2 active and high
	J       RDY0 AND RDY0 $0 $0     CTL2 OE2        ;   1 cycle,  CTL2 active and high, jp 0
	                                                ;1500 cycles @30MHz -> 20kS/s

Repeat these steps for all other wanted sample rates to create more include files and use them in your software.

