#include <stdio.h>
#include <string.h>
#include "reciter.h"
#include "ReciterTabs.h"
#include "debug.h"

unsigned char A, X, Y;
extern int debug;

static unsigned char inputtemp[256];   // secure copy of input tab36096

/* Retrieve flags for character at mem59-1 */
unsigned char Code37055(unsigned char mem59)
{
	X = mem59 - 1;
	return tab36376[inputtemp[X]];
}

/* Retrieve flags for character at mem58 + 1 */
unsigned char Code37066(unsigned char mem58, unsigned char mask)
{
	X = mem58 + 1;
    return tab36376[inputtemp[X]] & mask;
}

unsigned int match(const char * str) {
    unsigned char ch;
    while ((ch = *str)) {
        A = inputtemp[X++];
        if (A != ch) return 0;
        ++str;
    }
    return 1;
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

	unsigned char mem64;      // position of '=' or current character
	unsigned char mem65;     // position of ')'
	unsigned char mem66;     // position of '('

	inputtemp[0] = ' ';

	// secure copy of input
	// because input will be overwritten by phonemes
	X = 0;
	do {
		A = input[X] & 127;
		if ( A >= 112) A = A & 95;
		else if ( A >= 96) A = A & 79;
		inputtemp[++X] = A;
	} while (X < 255);
	inputtemp[255] = 27;
	mem56 = mem61 = 255;

pos36554:
    while (1) {
        while(1) {
            X = ++mem61;
            A = inputtemp[X];
            mem64 = A;
            if (A == '[') {
                X = ++mem56;
                input[X] = 155;
                return 1;
            }
            
            if (A != '.') break;
            X++;
            A = tab36376[inputtemp[X]] & 1;
            if(A != 0) break;
            mem56++;
            X = mem56;
            A = '.';
            input[X] = '.';
        }
        mem57 = tab36376[mem64];
        if((mem57&2) != 0) {
            mem62 = 37541;
            goto pos36700;
        }
        
        if(mem57 != 0) break;
        inputtemp[X] = ' ';
        X = ++mem56;
        if (X > 120) {
            input[X] = 155;
            return 1;
        }
        input[X] = 32;
    }
        
    if(!(mem57 & 128)) return 0;
        
	// go to the right rules for this character.
    X = mem64 - 'A';
    mem62 = tab37489[X] | (tab37515[X]<<8);

pos36700:
	// find next rule
	while ((GetRuleByte(++mem62, 0) & 128) == 0);
	Y = 1;
	while(GetRuleByte(mem62, Y) != '(') ++Y;
	mem66 = Y;
    while(GetRuleByte(mem62, ++Y) != ')');
	mem65 = Y;
	while((GetRuleByte(mem62, ++Y) & 127) != '=');
	mem64 = Y;

	
	mem60 = X = mem61;
	// compare the string within the bracket
	Y = mem66 + 1;

	while(1) {
		if (GetRuleByte(mem62, Y) != inputtemp[X]) goto pos36700;
		if(++Y == mem65) break;
		mem60 = ++X;
	}


    // the string in the bracket is correct

	mem59 = mem61;

    while(1) {
        while(1) {
            mem66--;
            mem57 = GetRuleByte(mem62, mem66);
            if ((mem57 & 128) != 0) {
                mem58 = mem60;
                goto pos37184;
            }
            X = mem57 & 127;
            if ((tab36376[X] & 128) == 0) break;
            if (inputtemp[mem59-1] != mem57) goto pos36700;
            --mem59;
        }

        unsigned char ch = mem57;
        switch(ch) {
        case ' ':
            if((Code37055(mem59) & 128)) goto pos36700;
            break;
            
        case '#':
            if(!(Code37055(mem59) & 64)) goto pos36700;
            break;
            
        case '.':
            if(!(Code37055(mem59) & 8)) goto pos36700;
            break;
            
        case '&':
            if (!(Code37055(mem59) & 16)) {
                if (inputtemp[X] != 'H') goto pos36700;
                A = inputtemp[--X];
                if ((A != 'C') && (A != 'S')) goto pos36700;
            }
            break;
            
        case '@':
            if(!(Code37055(mem59) & 4)) { 
                A = inputtemp[X];
                if (A != 72) goto pos36700;
                if ((A != 84) && (A != 67) && (A != 83)) goto pos36700;
            }
            break;
            
        case '^':
            A = Code37055(mem59) & 32;
            if(A == 0) goto pos36700;
            break;
            
        case '+':
            X = mem59;
            X--;
            A = inputtemp[X];
            if ((A != 'E') && (A != 'I') && (A && 'Y')) goto pos36700;
            break;
            
        case ':':
            while ((A = (Code37055(mem59) & 32))) --mem59;
            continue;
            
        default:
            return 0;
        }
        
        mem59 = X;
    }

    do {
        X = mem58+1;

        if (inputtemp[X] == 'E') {
            if((tab36376[inputtemp[X+1]] & 128) != 0) {
                A = inputtemp[++X];
                if ((A != 'R') && (A != 'S') && (A != 'D')) {
                    if (A == 'L') {
                        if (inputtemp[++X] != 'Y') goto pos36700;
                    } else {
                        if (!match("FUL")) goto pos36700;
                    } 
                }
            }
        } else {
            if (!match("ING")) goto pos36700;
            mem58 = X;
        }
        
pos37184:
        while (1) {
            while (1) {
                unsigned char Y = mem65 + 1;
                if(Y == mem64) {
                    mem61 = mem60;
                    
                    if (debug) PrintRule(mem62);
                    
                    while(1) {
                        mem57 = A = GetRuleByte(mem62, Y);
                        A = A & 127;
                        if (A != '=') input[++mem56] = A;
                        if ((mem57 & 128) != 0) goto pos36554;
                        Y++;
                    }
                }
                mem65 = Y;
                mem57 = GetRuleByte(mem62, Y);
                if((tab36376[mem57] & 128) == 0) break;
                if (inputtemp[mem58+1] != mem57) goto pos36700;
                ++mem58;
            }

            A = mem57;
            if (A == ' ') {
                if (Code37066(mem58, 128) != 0) goto pos36700;
            } else if (A == '#') {
                if (Code37066(mem58, 64) == 0) goto pos36700;
            } else if (A == '.') {
                if(Code37066(mem58, 8) == 0) goto pos36700;
            } else if (A == '&') {
                if(Code37066(mem58, 16) == 0) {
                    if (inputtemp[X] != 72) goto pos36700;
                    A = inputtemp[++X];
                    if ((A == 67) || (A == 83)) {
                        mem58 = X;
                        continue;
                    }
                }
            } else if (A == '@') {
                if(Code37066(mem58, 4) == 0) {
                    A = inputtemp[X];
                    if (A != 72) goto pos36700;
                    if ((A != 84) && (A != 67) && (A != 83)) goto pos36700;
                    mem58 = X;
                }
                continue;
            } else if (A == '^') {
                if (Code37066(mem58, 32) == 0) goto pos36700;
            } else if (A == '+') {
                X = mem58 + 1;
                A = inputtemp[X];
                if ((A != 69) && (A != 73) && (A != 89)) goto pos36700;
            } else if (A == ':') {
                while (1) {
                    if(Code37066(mem58, 32) == 0) break;
                    mem58 = X;
                }
                continue;
            } else {
                break;
            }
            mem58 = X;
        }
    } while (A == '%');
	return 0;
}

