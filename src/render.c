#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "render.h"
#include "RenderTabs.h"

#include "debug.h"
extern int debug;

unsigned char wait1 = 7;
unsigned char wait2 = 6;

extern unsigned char A, X, Y;
extern unsigned char mem44;
extern unsigned char mem47;
extern unsigned char mem49;
extern unsigned char mem39;
extern unsigned char mem50;
extern unsigned char mem51;
extern unsigned char mem53;

extern unsigned char mem56;

extern unsigned char speed;
extern unsigned char pitch;
extern int singmode;


extern unsigned char phonemeIndexOutput[60]; //tab47296
extern unsigned char stressOutput[60]; //tab47365
extern unsigned char phonemeLengthOutput[60]; //tab47416

unsigned char pitches[256]; // tab43008

unsigned char frequency1[256];
unsigned char frequency2[256];
unsigned char frequency3[256];

unsigned char amplitude1[256];
unsigned char amplitude2[256];
unsigned char amplitude3[256];

unsigned char sampledConsonantFlag[256]; // tab44800


void AddInflection(unsigned char mem48, unsigned char phase1);
unsigned char trans(unsigned char mem39212, unsigned char mem39213);


// contains the final soundbuffer
extern int bufferpos;
extern char *buffer;



//timetable for more accurate c64 simulation
int timetable[5][5] =
{
	{162, 167, 167, 127, 128},
	{226, 60, 60, 0, 0},
	{225, 60, 59, 0, 0},
	{200, 0, 0, 54, 55},
	{199, 0, 0, 54, 54}
};

void Output(int index, unsigned char A)
{
	static unsigned oldtimetableindex = 0;
	int k;
	bufferpos += timetable[oldtimetableindex][index];
	oldtimetableindex = index;
	// write a little bit in advance
	for(k=0; k<5; k++)
		buffer[bufferpos/50 + k] = (A & 15)*16;
}







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


static unsigned char RenderVoicedSample(unsigned short hi, unsigned char off, unsigned char phase1)
{
	do {
		unsigned char sample = sampleTable[hi+off];
		unsigned char bit = 8;
		do {
			if ((sample & 128) != 0) Output(3, 26);
			else Output(4, 6);
			sample <<= 1;
		} while(--bit != 0);
		off++;
	} while (++phase1 != 0);
	return off;
}

static void RenderUnvoicedSample(unsigned short hi, unsigned char off, unsigned char mem53)
{
    do {
        unsigned char bit = 8;
        unsigned char sample = sampleTable[hi+off];
        do {
            if ((sample & 128) != 0) Output(2, 5);
            else Output(1, mem53);
            sample <<= 1;
        } while (--bit != 0);
    } while (++off != 0);
}



// -------------------------------------------------------------------------
//Code48227
// Render a sampled sound from the sampleTable.
//
//   Phoneme   Sample Start   Sample End
//   32: S*    15             255
//   33: SH    257            511
//   34: F*    559            767
//   35: TH    583            767
//   36: /H    903            1023
//   37: /X    1135           1279
//   38: Z*    84             119
//   39: ZH    340            375
//   40: V*    596            639
//   41: DH    596            631
//
//   42: CH
//   43: **    399            511
//
//   44: J*
//   45: **    257            276
//   46: **
// 
//   66: P*
//   67: **    743            767
//   68: **
//
//   69: T*
//   70: **    231            255
//   71: **
//
// The SampledPhonemesTable[] holds flags indicating if a phoneme is
// voiced or not. If the upper 5 bits are zero, the sample is voiced.
//
// Samples in the sampleTable are compressed, with bits being converted to
// bytes from high bit to low, as follows:
//
//   unvoiced 0 bit   -> X
//   unvoiced 1 bit   -> 5
//
//   voiced 0 bit     -> 6
//   voiced 1 bit     -> 24
//
// Where X is a value from the table:
//
//   { 0x18, 0x1A, 0x17, 0x17, 0x17 };
//
// The index into this table is determined by masking off the lower
// 3 bits from the SampledPhonemesTable:
//
//        index = (SampledPhonemesTable[i] & 7) - 1;
//
// For voices samples, samples are interleaved between voiced output.


void RenderSample(unsigned char *mem66)
{     
	// current phoneme's index
	mem49 = Y;

	// mask low three bits and subtract 1 get value to 
	// convert 0 bits on unvoiced samples.
	unsigned char hibyte = (mem39&7)-1;
	
	// determine which offset to use from table { 0x18, 0x1A, 0x17, 0x17, 0x17 }
	// T, S, Z                0          0x18
	// CH, J, SH, ZH          1          0x1A
	// P, F*, V, TH, DH       2          0x17
	// /H                     3          0x17
	// /X                     4          0x17

    mem44 = 1;
	
    unsigned short hi = hibyte*256;
	// voiced sample?
	unsigned char pitch = mem39 & 248;
	if(pitch == 0) {
        // voiced phoneme: Z*, ZH, V*, DH
		pitch = pitches[mem49] >> 4;
        *mem66 = RenderVoicedSample(hi, *mem66, pitch ^ 255);
        return;
	}
    RenderUnvoicedSample(hi, pitch^255, tab48426[hibyte]);
}


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
			RenderSample(&mem66);
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
			unsigned char A = mem56 + multtable[rectangle[phase3] | amplitude3[Y]] + (carry?1:0);
			A = ((A + 136) & 255) >> 4; //there must be also a carry
			
			// output the accumulated value
			Output(0, A);
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
		if(mem44 == 0) {
pos48159:
            // fetch the next glottal pulse length
			
			mem44 = pitches[Y];
			mem38 = A = mem44 - (mem44>>2);
			
			// reset the formant wave generators to keep them in 
			// sync with the glottal pulse
			phase1 = 0;
			phase2 = 0;
			phase3 = 0;
			continue;
		}
		
		// decrement the count
		mem38--;
		
		// is the count non-zero and the sampled flag is zero?
		if((mem38 != 0) || (mem39 == 0))
		{
            // reset the phase of the formants to match the pulse
			phase1 += frequency1[Y];
			phase2 += frequency2[Y];
			phase3 += frequency3[Y];
			continue;
		}
		
		// voiced sampled phonemes interleave the sample with the
		// glottal pulse. The sample flag is non-zero, so render
		// the sample for the phoneme.
		RenderSample(&mem66);
		goto pos48159;
	} //while

}



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

unsigned char CreateTransitions()
{
    unsigned char phase1;
    unsigned char phase2;
	unsigned char A = 0;
	mem44 = 0;
	mem49 = 0; 
	unsigned char pos = 0;
	while(1) {
        // get the current and following phoneme
		unsigned char phoneme = phonemeIndexOutput[pos];
		A = phonemeIndexOutput[pos+1];
		pos++;

		// exit loop at end token
		if (A == 255) break;

        // get the ranking of each phoneme
		pos = A;
		mem56 = blendRank[A];

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

		A = mem49 + phonemeLengthOutput[mem44]; // A is mem49 + length
		mem49 = A; // mem49 now holds length + position
		A = mem49 + phase2; //Maybe Problem because of carry flag

		//47776: ADC 42
		unsigned char speedcounter = A;
		mem47 = 168;
		unsigned char phase3 = mem49 - phase1; // what is mem49
		unsigned char mem38 = phase1 + phase2; // total transition?
		
		pos = mem38;
		pos -= 2;
		if ((pos & 128) == 0)
            do {
                // mem47 is used to index the tables:
                // 168  pitches[]
                // 169  frequency1
                // 170  frequency2
                // 171  frequency3
                // 172  amplitude1
                // 173  amplitude2
                // 174  amplitude3
                
                unsigned char mem40 = mem38;
            
                if (mem47 == 168) {     // pitch
                    // unlike the other values, the pitches[] interpolates from 
                    // the middle of the current phoneme to the middle of the 
                    // next phoneme
                      
                    // half the width of the current phoneme
                    unsigned char mem36 = phonemeLengthOutput[mem44] >> 1;
                    // half the width of the next phoneme
                    unsigned char mem37 = phonemeLengthOutput[mem44+1] >> 1;
                    // sum the values
                    mem40 = mem36 + mem37; // length of both halves
                    mem37 += mem49; // center of next phoneme
                    mem36 = mem49 - mem36; // center index of current phoneme
                    unsigned char A = Read(mem47, mem37); // value at center of next phoneme - end interpolation value
                    mem53 = A - Read(mem47, mem36); // value to center of current phoneme
                } else {
                    // value to interpolate to
                    unsigned char A = Read(mem47, speedcounter);
                    // position to start interpolation from
                    // value to interpolate from
                    mem53 = A - Read(mem47, phase3);
                }
			
                //Code47503(mem40);
                // ML : Code47503 is division with remainder, and mem50 gets the sign
			
                // calculate change per frame
                mem50 = (((char)(mem53) < 0) ? 128 : 0);
                mem51 = abs((char)mem53) % mem40;
                mem53 = (unsigned char)((char)(mem53) / mem40);

                // interpolation range
                pos = mem40;  // number of frames to interpolate over
                unsigned char frame = phase3; // starting frame

                // linearly interpolate values
                mem56 = 0;
                while(1) {
                    A = Read(mem47, frame) + mem53; //carry alway cleared

                    unsigned char mem48 = A;
                    frame++;
                    pos--;
                    if(pos == 0) break;
                    
                    mem56 += mem51;
                    if (mem56 >= mem40) {  //???
                        mem56 -= mem40; //carry? is set
                         if ((mem50 & 128)==0) {
                            if(mem48 != 0) mem48++;
                        } else mem48--;
                    }
                    Write(mem47, frame, mem48);
                } //while No. 3
                
                mem47++;
            } while (mem47 != 175);     //while No. 2
		pos = ++mem44;
	} 

    // add the length of this phoneme
    return mem49 + phonemeLengthOutput[mem44];
}



// CREATE FRAMES
//
// The length parameter in the list corresponds to the number of frames
// to expand the phoneme to. Each frame represents 10 milliseconds of time.
// So a phoneme with a length of 7 = 7 frames = 70 milliseconds duration.
//
// The parameters are copied from the phoneme to the frame verbatim.
//
static void CreateFrames()
{
    unsigned char mem44 = 0;
	unsigned char phase1 = 0;

	X = 0;
    do {
        // get the phoneme at the index
        unsigned char phoneme = phonemeIndexOutput[mem44];
	
        // if terminal phoneme, exit the loop
        if (phoneme == 255) break;
	
        // period phoneme *.
        if (phoneme == 1)  AddInflection(1, phase1); // rising
        // question mark phoneme?
        if (phoneme == 2) AddInflection(255, phase1); // falling

        // get the stress amount (more stress = higher pitch)
        phase1 = tab47492[stressOutput[mem44] + 1];
	
        // get number of frames to write
        unsigned phase2 = phonemeLengthOutput[mem44];
	
        // copy from the source to the frames list
        do {
            frequency1[X] = freq1data[phoneme];     // F1 frequency
            frequency2[X] = freq2data[phoneme];     // F2 frequency
            frequency3[X] = freq3data[phoneme];     // F3 frequency
            amplitude1[X] = ampl1data[phoneme];     // F1 amplitude
            amplitude2[X] = ampl2data[phoneme];     // F2 amplitude
            amplitude3[X] = ampl3data[phoneme];     // F3 amplitude
            sampledConsonantFlag[X] = sampledConsonantFlags[phoneme];        // phoneme data for sampled consonants
            pitches[X] = pitch + phase1;      // pitch
            X++;
        } while(--phase2 != 0);
        mem44++;
    } while(mem44 != 0);
}


// RESCALE AMPLITUDE
//
// Rescale volume from a linear scale to decibels.
//
void RescaleAmplitude() 
{
    int i;
    for(i=255; i>=0; i--)
        {
            amplitude1[i] = amplitudeRescale[amplitude1[i]];
            amplitude2[i] = amplitudeRescale[amplitude2[i]];
            amplitude3[i] = amplitudeRescale[amplitude3[i]];
        }
}



// ASSIGN PITCH CONTOUR
//
// This subtracts the F1 frequency from the pitch to create a
// pitch contour. Without this, the output would be at a single
// pitch level (monotone).

void AssignPitchContour()
{	
	// don't adjust pitch if in sing mode
	if (!singmode) {
        // iterate through the buffer
        int i;
        for(i=0; i<256; i++) {
            // subtract half the frequency of the formant 1.
            // this adds variety to the voice
    		pitches[i] -= (frequency1[i] >> 1);
        }
	}
}


// RENDER THE PHONEMES IN THE LIST
//
// The phoneme list is converted into sound through the steps:
//
// 1. Copy each phoneme <length> number of times into the frames list,
//    where each frame represents 10 milliseconds of sound.
//
// 2. Determine the transitions lengths between phonemes, and linearly
//    interpolate the values across the frames.
//
// 3. Offset the pitches by the fundamental frequency.
//
// 4. Render the each frame.
void Render()
{
	if (phonemeIndexOutput[0] == 255) return; //exit if no data

    CreateFrames();
    unsigned char t = CreateTransitions();

    AssignPitchContour();
    RescaleAmplitude();

    if (debug) {
        PrintOutput(sampledConsonantFlag, frequency1, frequency2, frequency3, amplitude1, amplitude2, amplitude3, pitches);
    }

    ProcessFrames(t);
}


// Create a rising or falling inflection 30 frames prior to 
// index X. A rising inflection is used for questions, and 
// a falling inflection is used for statements.

void AddInflection(unsigned char mem48, unsigned char phase1)
{
	//pos48372:
	//	mem48 = 255;
//pos48376:
           
    // store the location of the punctuation
	mem49 = X;
	A = X;
	int Atemp = A;
	
	// backup 30 frames
	A = A - 30; 
	// if index is before buffer, point to start of buffer
	if (Atemp <= 30) A=0;
	X = A;

	// FIXME: Explain this fix better, it's not obvious
	// ML : A =, fixes a problem with invalid pitch with '.'
	while( (A=pitches[X]) == 127) X++;

    while (1)
    {
        // add the inflection direction
        A += mem48;
        phase1 = A;
	
        // set the inflection
        pitches[X] = A;

        do {
            // increment the position
            X++;
        
            // exit if the punctuation has been reached
            if (X == mem49) return;
        } while (pitches[X] == 255);
        A = phase1;
    } 
}

/*
    SAM's voice can be altered by changing the frequencies of the
    mouth formant (F1) and the throat formant (F2). Only the voiced
    phonemes (5-29 and 48-53) are altered.
*/
void SetMouthThroat(unsigned char mouth, unsigned char throat)
{
	unsigned char initialFrequency;
	unsigned char newFrequency = 0;
	//unsigned char mouth; //mem38880
	//unsigned char throat; //mem38881

	// mouth formants (F1) 5..29
	unsigned char mouthFormants5_29[30] = {
		0, 0, 0, 0, 0, 10,
		14, 19, 24, 27, 23, 21, 16, 20, 14, 18, 14, 18, 18,
		16, 13, 15, 11, 18, 14, 11, 9, 6, 6, 6};

	// throat formants (F2) 5..29
	unsigned char throatFormants5_29[30] = {
	255, 255,
	255, 255, 255, 84, 73, 67, 63, 40, 44, 31, 37, 45, 73, 49,
	36, 30, 51, 37, 29, 69, 24, 50, 30, 24, 83, 46, 54, 86};

	// there must be no zeros in this 2 tables
	// formant 1 frequencies (mouth) 48..53
	unsigned char mouthFormants48_53[6] = {19, 27, 21, 27, 18, 13};
       
	// formant 2 frequencies (throat) 48..53
	unsigned char throatFormants48_53[6] = {72, 39, 31, 43, 30, 34};

	unsigned char pos = 5; //mem39216
//pos38942:
	// recalculate formant frequencies 5..29 for the mouth (F1) and throat (F2)
	while(pos != 30)
	{
		// recalculate mouth frequency
		initialFrequency = mouthFormants5_29[pos];
		if (initialFrequency != 0) newFrequency = trans(mouth, initialFrequency);
		freq1data[pos] = newFrequency;
               
		// recalculate throat frequency
		initialFrequency = throatFormants5_29[pos];
		if(initialFrequency != 0) newFrequency = trans(throat, initialFrequency);
		freq2data[pos] = newFrequency;
		pos++;
	}

//pos39059:
	// recalculate formant frequencies 48..53
	pos = 48;
	Y = 0;
    while(pos != 54)
    {
		// recalculate F1 (mouth formant)
		initialFrequency = mouthFormants48_53[Y];
		newFrequency = trans(mouth, initialFrequency);
		freq1data[pos] = newFrequency;
           
		// recalculate F2 (throat formant)
		initialFrequency = throatFormants48_53[Y];
		newFrequency = trans(throat, initialFrequency);
		freq2data[pos] = newFrequency;
		Y++;
		pos++;
	}
}


//return = (mem39212*mem39213) >> 1
unsigned char trans(unsigned char mem39212, unsigned char mem39213)
{
	//pos39008:
	unsigned char carry;
	int temp;
	unsigned char mem39214, mem39215;
	A = 0;
	mem39215 = 0;
	mem39214 = 0;
	X = 8;
	do
	{
		carry = mem39212 & 1;
		mem39212 = mem39212 >> 1;
		if (carry != 0)
		{
			/*
						39018: LSR 39212
						39021: BCC 39033
						*/
			carry = 0;
			A = mem39215;
			temp = (int)A + (int)mem39213;
			A = A + mem39213;
			if (temp > 255) carry = 1;
			mem39215 = A;
		}
		temp = mem39215 & 1;
		mem39215 = (mem39215 >> 1) | (carry?128:0);
		carry = temp;
		//39033: ROR 39215
		X--;
	} while (X != 0);
	temp = mem39214 & 128;
	mem39214 = (mem39214 << 1) | (carry?1:0);
	carry = temp;
	temp = mem39215 & 128;
	mem39215 = (mem39215 << 1) | (carry?1:0);
	carry = temp;

	return mem39215;
}


