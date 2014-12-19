#include <stdio.h>
#include <string.h>
#include "reciter.h"
#include "ReciterTabs.h"
#include "debug.h"

unsigned char A, X, Y;
extern int debug;

static unsigned char inputtemp[256];   // secure copy of input tab36096

void Code37055(unsigned char mem59)
{
	X = mem59;
	X--;
	A = inputtemp[X];
	Y = A;
	A = tab36376[Y];
	return;
}

void Code37066(unsigned char mem58)
{
	X = mem58;
	X++;
	A = inputtemp[X];
	Y = A;
	A = tab36376[Y];
}

unsigned char GetRuleByte(unsigned short mem62, unsigned char Y)
{
	unsigned int address = mem62;
	
	if (mem62 >= 37541) 
	{
		address -= 37541;
		return rules2[address+Y];
	}
	address -= 32000;
	return rules[address+Y];
}

int TextToPhonemes(char *input) 
{
	unsigned char mem56;      //output position for phonemes
	unsigned char mem57;
	unsigned char mem58;
	unsigned char mem59;
	unsigned char mem60;
	unsigned char mem61;
	unsigned short mem62;     // memory position of current rule
	unsigned char mem64;     // position of '=' or current character
	unsigned char mem65;     // position of ')'
	unsigned char mem66;     // position of '('

	inputtemp[0] = 32;

	// secure copy of input
	// because input will be overwritten by phonemes
	X = 1;
	Y = 0;

    do {
		A = input[Y] & 127;
		if ( A >= 112) A = A & 95;
		else if ( A >= 96) A = A & 79;

		inputtemp[X] = A;
		X++;
	} while (++Y != 255);

	inputtemp[255] = 27;
	mem61 = 255;

	mem56 = 255;

pos36554:
    do {

	while(1)
	{
		X = ++mem61;
		A = inputtemp[mem61];
		mem64 = A;
		if (A == '[')
		{
			++mem56;
			X = mem56;
			A = 155;
			input[X] = 155;
			return 1;
		}

		if (A != '.') break;
		X++;
		Y = inputtemp[X];
		A = tab36376[Y] & 1;
		if(A != 0) break;
		mem56++;
		X = mem56;
		A = '.';
		input[X] = '.';
	}

	A = mem64;
	Y = A;
	A = tab36376[A];
	mem57 = A;
	if((A&2) != 0)
	{
		mem62 = 37541;
		goto pos36700;
	}

	A = mem57;
    if(A != 0) goto pos36677;
    A = 32;
    inputtemp[X] = ' ';
    X = ++mem56;
    input[X] = A;
    } while (mem56 <= 120);

	input[X] = 155;
	A = mem61;

	return 1;

pos36677:
	A = mem57 & 128;
	if(A == 0) return 0;

	// go to the right rules for this character.
	X = mem64 - 'A';
	mem62 = tab37489[X] | (tab37515[X]<<8);

	// -------------------------------------
	// go to next rule
	// -------------------------------------
pos36700:
	// find next rule
	Y = 0;
	do {
		A = GetRuleByte(++mem62, Y);
	} while ((A & 128) == 0);
	++Y;

	// find '('
	while(1) {
		A = GetRuleByte(mem62, Y);
		if (A == '(') break;
		Y++;
	}
	mem66 = Y;

	while (GetRuleByte(mem62, ++Y) != ')');
	mem65 = Y;

	// find '='
	while ((GetRuleByte(mem62, ++Y) & 127) != '=');
	mem64 = Y;

	mem60 = X = mem61;

	// compare the string within the bracket
	Y = mem66 + 1;

	while(1) {
		mem57 = inputtemp[X];
		if (GetRuleByte(mem62, Y) != mem57) goto pos36700;
		++Y;
		if(Y == mem65) break;
		++X;
		mem60 = X;
	}

    // the string in the bracket is correct
	mem59 = mem61;

 pos36791:
	while(1) {
		Y = --mem66;
		mem57 = GetRuleByte(mem62, Y);
		if ((mem57 & 128) != 0) {
            mem58 = A = mem60;
            goto pos37184;
        }
		X = mem57 & 127;
		A = tab36376[X] & 128;
		if (A == 0) break;
		X = mem59-1;
		A = inputtemp[X];
		if (A != mem57) goto pos36700;
		mem59 = X;
	}

	A = mem57;
    switch (A) {
    case ' ':
        Code37055(mem59);
        if ((A & 128) != 0) goto pos36700;
        break;
    case '#':
        Code37055(mem59);
        if((A&64) == 0) goto pos36700;
        break;
    case '.':
        Code37055(mem59);
        if((A&8) == 0) goto pos36700;
        break;
    case '&':
        Code37055(mem59);
        if((A & 16) != 0) break;
        A = inputtemp[X];
        if (A != 72) goto pos36700;
        X--;
        A = inputtemp[X];
        if ((A == 67) || (A == 83)) break;
        goto pos36700;
    case '@':
        Code37055(mem59);
        if(A & 4!= 0) break;
        A = inputtemp[X];
        if (A != 72) goto pos36700;
        if ((A != 84) && (A != 67) && (A != 83)) goto pos36700;
        break;
    case '^':
        Code37055(mem59);
        if((A & 32) == 0) goto pos36700;
        break;
	case '+':
        X = mem59;
        X--;
        A = inputtemp[X];
        if ((A == 'E') || (A == 'I') || (A == 'Y')) break;
        goto pos36700;
    case ':':
        while((tab36376[inputtemp[mem59-1]] & 32)) --mem59;
        goto pos36791;
    default:
        return 0;
    };

	mem59 = X;
	goto pos36791;

 pos37077:
    X = mem58+1;
    A = inputtemp[X];
    if (A != 'E') goto pos37157;
	Y = inputtemp[X+1];
	A = tab36376[Y] & 128;
	if(A == 0) {
        mem58 = X;
        goto pos37184;
    }
	A = inputtemp[++X];


	if ((A == 'R') || (A == 'S') || (A == 'D')) {
        mem58 = X;
        goto pos37184;
    }
	if (A == 'L') {
        X++;
        A = inputtemp[X];
        if (A != 89) goto pos36700;
        mem58 = X;
        goto pos37184;
    }

	if (A == 70) {
        A = inputtemp[++X];
        if (A == 85) {
            A = inputtemp[++X];
            if (A == 76) {
                mem58 = X;
                goto pos37184;
            }
        }
    }
	goto pos36700;

pos37157:
	if (A == 73) {
        A = inputtemp[++X];
        if (A == 78) {
            A = inputtemp[++X];
            if (A == 71) {
                mem58 = X;
                goto pos37184;
            }
        }
    }
	goto pos36700;

	// -----------------------------------------

pos37184:
    while (1) {
        Y = mem65 + 1;
        if(Y == mem64) {
            Y = mem64;
            mem61 = mem60;
            
            if (debug) PrintRule(mem62);

            while(1) {
                mem57 = GetRuleByte(mem62, Y);
                A = mem57 & 127;
                if (A != '=') {
                    mem56++;
                    X = mem56;
                    input[X] = A;
                }
                if ((mem57 & 128) != 0) goto pos36554;
                Y++;
            }
        }
        mem65 = Y;
        X = mem57 = GetRuleByte(mem62, Y);
        A = tab36376[X] & 128;
        if(A == 0) break;
        X = mem58+1;
        A = inputtemp[X];
        if (A != mem57) goto pos36700;
        mem58 = X;
    }

	A = mem57;
    switch(A) {
    case ' ':
        Code37066(mem58);
        A = A & 128;
        if(A != 0) goto pos36700;
        mem58 = X;
        goto pos37184;
    case '#':
        Code37066(mem58);
        if((A & 64) == 0) goto pos36700;
        mem58 = X;
        goto pos37184;
    case '.':
        Code37066(mem58);
        if((A & 8) == 0) goto pos36700;
        mem58 = X;
        goto pos37184;
    case '&':
        Code37066(mem58);
        A = A & 16;
        if(A != 0) {
            mem58 = X;
            goto pos37184;
        }
        A = inputtemp[X];
        if (A != 72) goto pos36700;
        X++;
        A = inputtemp[X];
        if ((A == 67) || (A == 83)) {
            mem58 = X;
            goto pos37184;
        }
        goto pos36700;
    case 64:
        Code37066(mem58);
        if((A & 4) != 0) {
            mem58 = X;
            goto pos37184;
        }
        A = inputtemp[X];
        if (A != 72) goto pos36700;
        if ((A != 84) && (A != 67) && (A != 83)) goto pos36700;
        mem58 = X;
        goto pos37184;

    case 94:
        Code37066(mem58);
        if((A & 32) == 0) goto pos36700;
        mem58 = X;
        goto pos37184;
       
    case '+':
        X = mem59;
        X--;
        A = inputtemp[X];
        if ((A == 'E') || (A == 'I') || (A == 'Y')) {
            mem58 = X;
            goto pos37184;
        }
        goto pos36700;
    case ':':
        while (1) {
            Code37066(mem58);
            A = A & 32;
            if(A == 0) goto pos37184;
            mem58 = X;
        }
    case '%':
        goto pos37077;
    }


	return 0;
}



