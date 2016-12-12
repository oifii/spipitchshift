////////////////////////////////////////////////////////////////
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
//
//
//2012june23, creation from callsmbpitchshift.cpp for 
//				replacing miniaiff library by libsndfile library
//
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////


/* *********************************************************************************

	EXAMPLE main() for smbPitchShift using MiniAiff to handle sound file i/o
	(c) 2003-2009 S. M. Bernsee, http://www.dspdimension.com
	
	IMPORTANT: requires miniAIFF to be included in the project

********************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*
#include "MiniAiff.h"
*/
#include "sndfile.h"
#include "sndfile.hh"

//do, do#, re, re#, mi, fa, fa#, sol, sol#, la, la#, si
void smbPitchShift(float pitchShift, long numSampsToProcess, long fftFrameSize, long osamp, float sampleRate, float *indata, float *outdata);


int main(int argc, char *argv[])
{
	char infilename[2048] = {"b4-mono.wav"};
	char outfilename[2048] = {"out.wav"};
	//long semitones = 3;	// shift up by 3 semitones
	long semitones = 12;	// shift up by 3 semitones
	if(argc>1)
	{
		strcpy(infilename, argv[1]);
	}
	if(argc>2)
	{
		semitones = atol(argv[2]); //semitone shift: -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1 or 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
	}

	long numChannels = 1;
	long bufferLengthFrames = 8192;
	int ret = 0;
	

	//float **data = mAiffAllocateAudioBuffer(numChannels, bufferLengthFrames);
	float* pData = (float*)malloc(numChannels*bufferLengthFrames*sizeof(float));
	if(pData==NULL)
	{
		printf("\n");
		return -1;
	}
	//long semitones = 3;	// shift up by 3 semitones
	//long semitones = -3; // shift down by 3 semitones
	float pitchShift = pow(2., semitones/12.);	// convert semitones to factor
	
	//char inFileName[] = "voice.aif";
	//char outFileName[] = "out2.aif";
	//char inFileName[] = "voice.wav";
	//char inFileName[] = "a#4.wav";

	//printf("Running MiniAiff version %s\n(C) S.M.Bernsee - http://www.dspdimension.com\n\n", mAiffGetVersion());
	printf("Running libsndfile version\n(C) , stephane.poirier@nakedsoftware.org\n\n");

	//open input file
	//SndfileHandle myInputSndFileHandle(inFileName, SFM_READ, SF_FORMAT_AIFF | SF_FORMAT_PCM_16, numChannels, 44100);
	SndfileHandle myInputSndFileHandle(infilename, SFM_READ, SF_FORMAT_WAV | SF_FORMAT_PCM_16, numChannels, 44100);
	if(!myInputSndFileHandle)
	{
		printf("! Error opening input file %s ! File may be busy\n", infilename);
		exit(-1);
	}

	// Create output file
	printf("Creating output\n");
	//ret = mAiffInitFile(outFileName, 44100. /* sample rate */, 16 /* bits */, numChannels /* mono */);
	//if (ret < 0)
	//SndfileHandle myOutputSndFileHandle(outFileName, SFM_WRITE, SF_FORMAT_AIFF | SF_FORMAT_PCM_16, numChannels, 44100);
	SndfileHandle myOutputSndFileHandle(outfilename, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, numChannels, 44100);
	if(!myOutputSndFileHandle)
	{
		printf("! Error creating output file %s ! File may be busy\n", outfilename);
		exit(-1);
	}
	
	//unsigned long inPosition=0;
	for(;;)
	{
	
		// Read from input file
		printf("Reading from input: %s\n", infilename);
		//ret = mAiffReadData(inFileName, data, inPosition, bufferLengthFrames, numChannels);
		ret = myInputSndFileHandle.read(pData, bufferLengthFrames);
		if (ret < 0) 
		{
			printf("! Error reading input file %s - error #%d\n! Wrong format or filename\n", infilename, ret);
			exit(-1);
		} else if (ret == 0) break;		// end of file reached
		
		// Increment the read position
		//inPosition += bufferLengthFrames;

		// --------------------------------- call smbPitchShift() ---------------------------------
		//smbPitchShift(pitchShift, bufferLengthFrames, 2048, 4, mAiffGetSampleRate(inFileName), data[0], data[0]);
		smbPitchShift(pitchShift, bufferLengthFrames, 2048, 4, myInputSndFileHandle.samplerate(), pData, pData);
		// ----------------------------------------------------------------------------------------

		// write processed data to output file
		printf("Writing to output: %s\n", outfilename);
		//ret = mAiffWriteData(outFileName, data, bufferLengthFrames, numChannels);
		ret = myOutputSndFileHandle.write(pData, bufferLengthFrames);
		if (ret < 0) 
		{
			printf("! Error writing output file %s - error #%d\n! Not enough space?\n", outfilename, ret);
			exit(-1);
		}
	}

	/*
	mAiffDeallocateAudioBuffer(data, numChannels);
	*/
	if(pData) free(pData);

	//done, write an empty file named sps_ok.txt in order to tell other applications this process is terminated. 
	//note, prior to launching this process, other applications should be deleting this sps_ok.txt file and wait for it to appear. 
	FILE* pFILE=fopen("sps_ok.txt", "w");
	if(pFILE!=NULL)
	{
		fclose(pFILE);
	}
	return 0;
}

