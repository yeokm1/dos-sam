
#include "render.h"

extern unsigned char mem39;
extern unsigned char Y;
extern unsigned char mem44;
extern unsigned char speed;

// From RenderTabs.h
extern unsigned char multtable[];
extern unsigned char sinus[];
extern unsigned char rectangle[];

// From render.c
extern unsigned char pitches[256]; 
extern unsigned char sampledConsonantFlag[256]; // tab44800
extern unsigned char amplitude1[256];
extern unsigned char amplitude2[256];
extern unsigned char amplitude3[256];
extern unsigned char frequency1[256];
extern unsigned char frequency2[256];
extern unsigned char frequency3[256];

extern void Output(int index, unsigned char A);


// PROCESS THE FRAMES
//
// In traditional vocal synthesis, the glottal pulse drives filters, which
// are attenuated to the frequencies of the formants.
//
// SAM generates these formants directly with sin and rectangular waves.
// To simulate them being driven by the glottal pulse, the waveforms are
// reset at the beginning of each glottal pulse.
//
void ProcessFrames(unsigned char mem48)
{

    unsigned char speedcounter = 72;
	unsigned char phase1 = 0;
    unsigned char phase2 = 0;
	unsigned char phase3 = 0;
    unsigned char mem66;
    
    Y = 0;
    mem44 = pitches[0];

    unsigned char mem38 = mem44 - (mem44 >> 2);

	while(1) {
		mem39 = sampledConsonantFlag[Y];
		
		// unvoiced sampled phoneme?
        if(mem39 & 248) {
			RenderSample(&mem66, mem39);
			// skip ahead two in the phoneme buffer
			Y += 2;
			mem48 -= 2;
            if(mem48 == 0) 	return;
            speedcounter = speed;
		} else {
            // simulate the glottal pulse and formants
			unsigned char mem56 = multtable[sinus[phase1] | amplitude1[Y]];
			unsigned char carry = ((mem56+multtable[sinus[phase2] | amplitude2[Y]] ) > 255);
			mem56 += multtable[sinus[phase2] | amplitude2[Y]];
			unsigned char tmp = mem56 + multtable[rectangle[phase3] | amplitude3[Y]] + (carry?1:0);
			tmp = ((tmp + 136) & 255) >> 4; //there must be also a carry
			
			// output the accumulated value
			Output(0, tmp);
			speedcounter--;
			if (speedcounter == 0) { 
                Y++; //go to next amplitude
                // decrement the frame count
                mem48--;
                if(mem48 == 0) 	return;
                speedcounter = speed;
            }
		}
         
        // decrement the remaining length of the glottal pulse
		mem44--;
		
		// finished with a glottal pulse?
		if(mem44 != 0) {
            // decrement the count
            mem38--;
            
            // is the count non-zero and the sampled flag is zero?
            if((mem38 != 0) || (mem39 == 0)) {
                // reset the phase of the formants to match the pulse
                phase1 += frequency1[Y];
                phase2 += frequency2[Y];
                phase3 += frequency3[Y];
                continue;
            }
		
            // voiced sampled phonemes interleave the sample with the
            // glottal pulse. The sample flag is non-zero, so render
            // the sample for the phoneme.
            RenderSample(&mem66, mem39);
        }

        // fetch the next glottal pulse length
        mem44 = pitches[Y];
        mem38 = mem44 - (mem44>>2); // mem44 * 0.75

        // reset the formant wave generators to keep them in 
        // sync with the glottal pulse
        phase1 = 0;
        phase2 = 0;
        phase3 = 0;
	}
}

