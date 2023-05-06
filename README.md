dos-sam
===

This is a DOS text-to-speech program that was first created by [s-macke](https://github.com/s-macke/SAM) through reverse-engineering then improved by [vidarh](https://github.com/vidarh/SAM).

This version has been slightly modified from vidarh's version to be compilable using OpenWatcom 2.0 to target DOS and Win95. SDL feature has been disabled so this program can only generate WAV files.

## Usage

Grab the binaries from the releases folder.

For basic use run the program with these parameters.

```bash
# To generate a output.wav file reading the words "hello world".
dos-sam.exe -wav output.wav hello world
```

More options are available by just running the `dos-sam.exe` standalone or reading the original readme file attached below.

## Compilation

To compile this application, you have to use Open Watcom (OWC) 2.0 beta which you can download from [here](https://github.com/open-watcom/open-watcom-v2/releases/tag/2023-04-01-Build). Open Watcom 2.0 for 64-bit Windows which was released on 2023-04-01 02:52:44 is used.

During installation, Open Watcom may prompt to install the environment variables. I chose to not do that to avoid having those variables being permanent. Instead I use a batch file to set the variables whenever I need to compile.

The program is compiled via a Makefile. By default the Makefile is set to compile DOS using the open-source [DOS/32](https://en.wikipedia.org/wiki/DOS/32) extender.

```bash
# Open cmd.exe
cd src

# To set environment variables assuming OWC is installed to C:\WATCOM2
20setenv.bat

# To compile
wmake

# To clean
wmake clean
```

A 32-bit extender was used due to memory requirements as well as the the original code was written with a 32-bit platform in mind. To compile for other platforms, uncomment the specific parts in the Makefile.

`DOS32A.EXE` was obtained from [here](https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/distributions/1.2/repos/pkg-html/dos32a.html).

# SAM (Original README file below) 
===

Software Automatic Mouth - Tiny Speech Synthesizer 


What is SAM?
============

Sam is a very small Text-To-Speech (TTS) program written in C, that runs on most popular platforms.
It is an adaption to C of the speech software SAM (Software Automatic Mouth) for the Commodore C64 published 
in the year 1982 by Don't Ask Software (now SoftVoice, Inc.). It includes a Text-To-Phoneme converter called reciter and a Phoneme-To-Speech routine for the 
final output. It is so small that it will work also on embedded computers. On my computer it takes
less than 39KB (much smaller on embedded devices as the executable-overhead is not necessary) of disk space and is a fully stand alone program. 
For immediate output it uses the SDL-library, otherwise it can save .wav files. 

An online version and executables for Windows can be found on the web site: http://simulationcorner.net/index.php?page=sam

Compile
=======

Simply type "make" in your command prompt.
In order to compile without SDL remove the SDL statements from the CFLAGS and LFLAGS variables in the file "Makefile".

It should compile on every UNIX-like operating system. For Windows you need Cygwin or MinGW( + libsdl).


Usage
=====

type

	./sam I am Sam

for the first output.

If you have disabled SDL try

	./sam -wav i_am_sam.wav I am Sam

to get a wav file. This file can be played by many media players available for the PC.

you can try other options like
	-pitch number
	-speed number
	-throat number
	-mouth number

Some typical values written in the original manual are:

	DESCRIPTION          SPEED     PITCH     THROAT    MOUTH
	Elf                   72        64        110       160
	Little Robot          92        60        190       190
	Stuffy Guy            82        72        110       105
	Little Old Lady       82        32        145       145
	Extra-Terrestrial    100        64        150       200
	SAM                   72        64        128       128


It can even sing
look at the file "sing"
for a small example.

For the phoneme input table look in the Wiki.


A description of additional features can be found in the original manual at
	http://www.retrobits.net/atari/sam.shtml
or in the manual of the equivalent Apple II program
	http://www.apple-iigs.info/newdoc/sam.pdf


Adaption To C
=============

This program was converted semi-automatic into C by converting each assembler opcode.
e. g. 

	lda 56		=>	A = mem[56];
	jmp 38018  	=>	goto pos38018;
	inc 38		=>	mem[38]++;
	.			.
	.			.

Then it was manually rewritten to remove most of the 
jumps and register variables in the code and rename the variables to proper names. 
Most of the description below is a result of this rewriting process.

Unfortunately its still a not very good readable. But you should see where I started :)



Short description
=================

First of all I will limit myself here to a very coarse description. 
There are very many exceptions defined in the source code that I will not explain. 
Also a lot of code is unknown for me e. g. Code47503. 
For a complete understanding of the code I need more time and especially more eyes have a look on the code. 

Reciter
-------

It changes the english text to phonemes by a ruleset shown in the wiki.

The rule
	" ANT(I)",	"AY",
means that if he find an "I" with previous letters " ANT", exchange the I by the phoneme "AY".

There are some special signs in this rules like
	#
	&
	@
	^
	+
	:
	%
which can mean e. g. that there must be a vocal or a consonant or something else. 

With the -debug option you will get the corresponding rules and the resulting phonemes.


Output
------

Here is the full tree of subroutine calls:

SAMMain()
	Parser1()
	Parser2()
		Insert()
	CopyStress()
	SetPhonemeLength()
	Code48619()
	Code41240()
		Insert()
	Code48431()
		Insert()
		
	Code48547
		Code47574
			Special1
			Code47503
			Code48227


SAMMain() is the entry routine and calls all further routines. 
Parser1 transforms the phoneme input and transforms it to three tables
	phonemeindex[]
	stress[]
	phonemelength[] (zero at this moment)
	
This tables are now changed: 

Parser2 exchanges some phonemes by others and inserts new. 
CopyStress adds 1 to the stress under some circumstances
SetPhonemeLength sets phoneme lengths. 
Code48619 changes the phoneme lengths
Code41240 adds some additional phonemes
Code48431 has some extra rules


The wiki shows all possible phonemes and some flag fields.  
The final content of these tables can be seen with the -debug command.


In the function PrepareOutput() these tables are partly copied into the small tables:
	phonemeindexOutput[]
	stressOutput[]
	phonemelengthOutput[]
for output.

Final Output
------------

Except of some special phonemes the output is build by a linear combination:
	
	A =   A1 * sin ( f1 * t ) +
	      A2 * sin ( f2 * t ) +
	      A3 * rect( f3 * t )

where rect is a rectangular function with the same periodicity like sin. 
It seems really strange, but this is really enough for most types of phonemes. 

Therefore the above phonemes are converted with some tables to 
	pitches[]
	frequency1[]  =  f1
	frequency2[]  =  f2
	frequency3[]  =  f3
	amplitude1[]  =  A1
	amplitude2[]  =  A2
	amplitude3[]  =  A3
	
Above formula is calculated in one very good omptimized routine.
It only consist of 26 commands:

    48087: 	LDX 43		; get phase	
    CLC		
	LDA 42240,x	; load sine value (high 4 bits)
	ORA TabAmpl1,y	; get amplitude (in low 4 bits)
	TAX		
	LDA 42752,x	; multiplication table
	STA 56		; store 

	LDX 42		; get phase
	LDA 42240,x	; load sine value (high 4 bits)
	ORA TabAmpl2,y	; get amplitude (in low 4 bits)
	TAX		
	LDA 42752,x	; multiplication table
	ADC Var56	; add with previous values
	STA 56		; and store

	LDX 41		; get phase
	LDA 42496,x	; load rect value (high 4 bits)
	ORA TabAmpl3,y	; get amplitude (in low 4 bits)
	TAX		
	LDA 42752,x	; multiplication table
	ADC 56		; add with previous values

	ADC #136		
	LSR A		; get highest 4 bits
	LSR A		
	LSR A		
	LSR A		
	STA 54296	;SID   main output command


The rest is handled in a special way. At the moment I cannot figure out in which way. 
But it seems that it uses some noise (e. g. for "s") using a table with random values. 

License
=======

The software is a reverse-engineered version of a commercial software published more than 30 years ago.
The current copyright holder is SoftVoice, Inc. (www.text2speech.com)

Any attempt to contact the company failed. The website was last updated in the year 2009.
The status of the original software can therefore best described as Abandonware 
(http://en.wikipedia.org/wiki/Abandonware)

As long this is the case I cannot put my code under any specific open source software license
Use it at your own risk.



Contact
=======

If you have questions don' t hesitate to ask me.
If you discovered some new knowledge about the code please mail me.

Sebastian Macke
Email: sebastian@macke.de