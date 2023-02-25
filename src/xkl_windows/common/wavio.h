#ifndef _WAVIO_H_
#define _WAVIO_H_
/* $Log: wavio.h,v $
 * Revision 1.6  2001/01/23  17:52:40  hanson
 * In declaration of MSWavHeaderStruct changed variables of type
 * long to type int.
 * Added variable fname to declaration of function readMSWavHeader.
 *
 * Revision 1.5  2000/04/20 05:35:55  krishna
 * Added MS wav and AIFF read/write functions.
 *
 * Revision 1.4  1998/06/06 05:20:41  krishna
 * Added RCS header.
 * */

#define WAVE_VERSION 2     /* Version of .wav format */

#define READ  "r"     /* Read flag for openWav */
#define WRITE "w"     /* Write flag for openWav */
#define APPEND "w+"   /* Append flag for openWav */

#ifndef ERROR
#define ERROR -1
#endif

#ifdef sgi
#define SWAP 1
#else
#define SWAP 0
#endif

typedef struct {
  char riffString[4];
  int riffChunkLength;
  char waveString[4];
  char fmtString[4];
  int fmtChunkLength;
  short waveFormat;
  short numChannels;
  int samplingRate;
  int numBytesPerSecond;
  short blockAlign;
  short bitsPerSample;
  char dataString[4];
  int dataLength;
} MSWavHeaderStruct;

typedef struct {
  char formString[4];
  long formChunkLength;
  char aiffString[4];
  char commString[4];
  long commChunkLength;
  short numChannels;
  long numFrames;
  short numBits;
  double samplingRate;
  char ssndString[4];
  long ssndChunkLength;
  long offset;
  long blockSize;
} AIFFHeaderStruct;

void checkWavName(char *filename);

short *getWavWaveform(char *fname, float *samplingRate, int *numSamples, 
		      int swap);
int putWavWaveform(char *fname, short *wave, float samplingRate, 
		    int numSamples, int swap);

int openWav(char *fname, FILE **fp, char *readWriteFlag);
int closeWav(FILE *fp);

int readWavHeader(FILE *fp, float *samplingRate, int *numSamples, 
		   int *version, int swap);
int readMSWavHeader(FILE *fp, char* fname, float *samplingRate, int *numSamples, 
		   int *channels, int *bytesPerSample, int swap);
int readAIFFHeader(FILE *fp, float *samplingRate, int *numSamples, 
		   int *numChannels, int *bytesPerSample, int swap);


int writeWavHeader(FILE *fp, float samplingRate, int numSamples, int swap);
int writeMSWavHeader(FILE *fp, float samplingRate, int numSamples, 
		   int numChannels, int bytesPerSample, int swap);
int writeAIFFHeader(FILE *fp, float samplingRate, int numSamples, 
		   int numChannels, int bytesPerSample, int swap);


int readWav(FILE *fp, short *wav, int numSamples, int swap);
int writeWav(FILE *fp, short *wav, int numSamples, int swap);


int swapInt(int value);
short swapShort(short value);

#endif
