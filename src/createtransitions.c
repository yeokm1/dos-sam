
#include <stdio.h>
#include <stdlib.h>
#include "render.h"

// CREATE TRANSITIONS
//
// Linear transitions are now created to smoothly connect each
// phoeneme. This transition is spread between the ending frames
// of the old phoneme (outBlendLength), and the beginning frames 
// of the new phoneme (inBlendLength).
//
// To determine how many frames to use, the two phonemes are 
// compared using the blendRank[] table. The phoneme with the 
// smaller score is used. In case of a tie, a blend of each is used:
//
//      if blendRank[phoneme1] ==  blendRank[phomneme2]
//          // use lengths from each phoneme
//          outBlendFrames = outBlend[phoneme1]
//          inBlendFrames = outBlend[phoneme2]
//      else if blendRank[phoneme1] < blendRank[phoneme2]
//          // use lengths from first phoneme
//          outBlendFrames = outBlendLength[phoneme1]
//          inBlendFrames = inBlendLength[phoneme1]
//      else
//          // use lengths from the second phoneme
//          // note that in and out are swapped around!
//          outBlendFrames = inBlendLength[phoneme2]
//          inBlendFrames = outBlendLength[phoneme2]
//
//  Blend lengths can't be less than zero.
//
// For most of the parameters, SAM interpolates over the range of the last
// outBlendFrames-1 and the first inBlendFrames.
//
// The exception to this is the Pitch[] parameter, which is interpolates the
// pitch from the center of the current phoneme to the center of the next
// phoneme.

// From render.c
extern unsigned char phonemeIndexOutput[60]; //tab47296
extern unsigned char phonemeLengthOutput[60]; //tab47416

// from RenderTabs.h
extern unsigned char blendRank[];
extern unsigned char outBlendLength[];
extern unsigned char inBlendLength[];
extern unsigned char pitches[];

extern unsigned char Read(unsigned char p, unsigned char Y);
extern void Write(unsigned char p, unsigned char Y, unsigned char value);

extern unsigned char frequency1[256];
extern unsigned char frequency2[256];
extern unsigned char frequency3[256];

extern unsigned char amplitude1[256];
extern unsigned char amplitude2[256];
extern unsigned char amplitude3[256];

//written by me because of different table positions.
// mem[47] = ...
// 168=pitches
// 169=frequency1
// 170=frequency2
// 171=frequency3
// 172=amplitude1
// 173=amplitude2
// 174=amplitude3
unsigned char Read(unsigned char p, unsigned char Y)
{
	switch(p)
	{
	case 168: return pitches[Y];
	case 169: return frequency1[Y];
	case 170: return frequency2[Y];
	case 171: return frequency3[Y];
	case 172: return amplitude1[Y];
	case 173: return amplitude2[Y];
	case 174: return amplitude3[Y];
	}
	printf("Error reading to tables");
	return 0;
}

void Write(unsigned char p, unsigned char Y, unsigned char value)
{

	switch(p)
	{
	case 168: pitches[Y] = value; return;
	case 169: frequency1[Y] = value;  return;
	case 170: frequency2[Y] = value;  return;
	case 171: frequency3[Y] = value;  return;
	case 172: amplitude1[Y] = value;  return;
	case 173: amplitude2[Y] = value;  return;
	case 174: amplitude3[Y] = value;  return;
	}
	printf("Error writing to tables\n");
}


unsigned char CreateTransitions()
{
    unsigned char phase1;
    unsigned char phase2;
    unsigned char mem53;
    unsigned char mem44 = 0;
	unsigned char mem49 = 0; 
	unsigned char pos = 0;
	while(1) {
        // get the current and following phoneme
		unsigned char phoneme = phonemeIndexOutput[pos];
		unsigned char A = phonemeIndexOutput[pos+1];
		pos++;

		// exit loop at end token
		if (A == 255) break;

        // get the ranking of each phoneme
		pos = A;
		unsigned char mem56 = blendRank[A];

		unsigned char rank = blendRank[phoneme];
		
		// compare the rank - lower rank value is stronger
		if (rank == mem56) {
            // same rank, so use out blend lengths from each phoneme
			phase1 = outBlendLength[phoneme];
			phase2 = outBlendLength[pos];
		} else if (rank < mem56) {
            // first phoneme is stronger, so us it's blend lengths
			phase1 = inBlendLength[pos];
			phase2 = outBlendLength[pos];
		} else {
            // second phoneme is stronger, so use it's blend lengths
            // note the out/in are swapped
			phase1 = outBlendLength[phoneme];
			phase2 = inBlendLength[phoneme];
		}

		mem49 += phonemeLengthOutput[mem44]; 

		unsigned char speedcounter = mem49 + phase2;
		unsigned table = 168;
		unsigned char phase3 = mem49 - phase1;
		unsigned char transition = phase1 + phase2; // total transition?
		
		pos = transition;
		pos -= 2;
		if ((pos & 128) == 0) {
            do {
                // mem47 is used to index the tables:
                // 168  pitches[]
                // 169  frequency1
                // 170  frequency2
                // 171  frequency3
                // 172  amplitude1
                // 173  amplitude2
                // 174  amplitude3
                
                unsigned char mem40 = transition;
            
                if (table == 168) {     // pitch
                    // unlike the other values, the pitches[] interpolates from 
                    // the middle of the current phoneme to the middle of the 
                    // next phoneme
                      
                    // half the width of the current phoneme
                    unsigned char mem36 = phonemeLengthOutput[mem44] >> 1;
                    // half the width of the next phoneme
                    unsigned char mem37 = phonemeLengthOutput[mem44+1] >> 1;
                    // sum the values
                    mem40 = mem36 + mem37; // length of both halves
                    mem53 = pitches[mem37 + mem49] - pitches[mem49-mem36];
                } else {
                    // Interpolate <from> - <to>
                    mem53 = Read(table, speedcounter) - Read(table, phase3);
                }
			
                //Code47503(mem40);
                // ML : Code47503 is division with remainder, and mem50 gets the sign
			
                // calculate change per frame
                unsigned char sign = ((char)(mem53) < 0);
                unsigned char mem51 = abs((char)mem53) % mem40;
                mem53 = (unsigned char)((char)(mem53) / mem40);

                // interpolation range
                pos = mem40;  // number of frames to interpolate over
                unsigned char frame = phase3; // starting frame

                // linearly interpolate values
                unsigned char mem56 = 0;
                while(1) {
                    unsigned char mem48 = Read(table, frame) + mem53; //carry alway cleared
                    frame++;
                    pos--;
                    if(pos == 0) break;
                    
                    mem56 += mem51;
                    if (mem56 >= mem40) {  //???
                        mem56 -= mem40; //carry? is set
                         if (!sign) {
                            if(mem48 != 0) mem48++;
                        } else mem48--;
                    }
                    Write(table, frame, mem48);
                } //while No. 3

                
                table++;
            } while (table != 175);     //while No. 2
        }
		pos = ++mem44;
	} 

    // add the length of this phoneme
    return mem49 + phonemeLengthOutput[mem44];
}



