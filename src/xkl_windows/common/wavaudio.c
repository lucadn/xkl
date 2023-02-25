/* $Log: wavaudio.c,v $
 * Revision 1.5  2001-09-14 15:58:49-04  tiede
 * play() given immediate return option (for klplay)
 *
 * Revision 1.4  1998/06/06 05:18:11  krishna
 * Added RCS header.
 * */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "wavaudio.h"
#include "wavio.h"

#define TRUE 1
#define FALSE 0

/*
 *   support for interruptible audio output
 */

#if ! defined(sgi)  // not supported on SGI boxen

static int audioFD = 0;
static int bufSize;
static int numSamps;
static short *waveLoc;
static char zeroBuf[NUM_ZEROS];

/* ***  PlayLoop  - work proc for play-in-progress  *** */

char PlayLoop(void *client_data)
{
    int numBytes;

    if (!audioFD) return TRUE;  // remove work proc if output aborted

    if (numSamps > bufSize) {
	numSamps -= bufSize;
	numBytes = bufSize * 2;
    } else {
	numBytes = numSamps * 2;
	numSamps = 0;
    }

// printf("writing %d samples\n", numSamps);

    if (write(audioFD, waveLoc, numBytes) < 0) {
	perror("Audio output aborted");
	close(audioFD);
	audioFD = 0;
	return TRUE;  // remove work proc on error
    }
    waveLoc += bufSize;
    if (numSamps > 0) return FALSE;  // continue looping

    write(audioFD, zeroBuf, NUM_ZEROS);

// leave device open so that we can trap interrupts
    // close(audioFD);
    // audioFD = 0;
    //if (ioctl(audioFD, SNDCTL_DSP_POST, 0) == -1) //Commented by LDN to compile under Mac Os
//	close(audioFD); //Commented by LDN to compile under Mac Os

    return TRUE;  // remove work proc
    
} /* PlayLoop */

#endif

/* ***  KILLPLAY  - halt audio-in-progress  *** */

void KillPlay()
{
  /*#if defined(sgi) //Commented by LDN to compile under Mac Os
    ;
#else
    if (audioFD) {
	ioctl(audioFD, SNDCTL_DSP_RESET, 0);
	close(audioFD);
	audioFD = 0;
    }
    #endif*/
  ;// Inserted by LDN to compile under Mac Os
}

/*************************************************************************
 *
 * int play(short *wave, float *samplingRate, int numSamples, int swap)
 *
 * Plays .wav wave file (*wave) to audio device at samplingRate, and 
 * whose length is numSamples.  swap defines whether to swap the bytes 
 * or not.  The actual sampling rate at which the sound was played at is
 * returned in samplingRate.  In addition, if there is an error, then play
 * returns -1.
 *
 * As of v2.82 initiates play parameters, returns 0 for work proc initialization
 * stupid event loop arrangement in xkl() blocks w/o work proc processing AARGH!
 * 
 * On sgi, to play .wav with sfplay :
 * -i byteorder little integer 16 2scomp chan 1 rate 16000 dataoff 512 end .wav
 *
 *************************************************************************/

int play(short *wave, float *samplingRate, int numSamples, int swap, int immed)
{

#if defined(sgi)
  int rate;    /* sampling rate */
  FILE *rawfp; /* file for sfplay */
  static char rawname[] = "/tmp/raw.xkl";
  static char cvtraw[] = "/tmp/cvt.xkl";
  char playcom[300], *rawfile;


  if (swap) {
    rawfp = fopen(rawname, "w");
    fwrite(wave, sizeof(short), (size_t) numSamples, rawfp);
    fclose(rawfp);
    rate = (int) *samplingRate;
    rawfile = rawname; /* if no sampling rate conversion needed */

  /* 
   * For now just convert to 8000,11025 or 16000 the hardware
   * can play files sampled up to 48k
   */
  
  if (*samplingRate != 8000.0 && *samplingRate != 11025.0 &&
     *samplingRate != 16000.0 &&  *samplingRate != 22050.0 &&
     *samplingRate != 32000.0 &&  *samplingRate != 44100.0 &&
     *samplingRate != 48000.0) {
    
    if (*samplingRate < 8000.0) rate = 8000;
    else if (*samplingRate < 11025.0) rate = 11025;
    else if (*samplingRate < 16000.0) rate = 16000; 
    else if (*samplingRate < 22050.0) rate = 22050; 
    else if (*samplingRate < 32000.0) rate = 32000; 
    else if (*samplingRate < 44100.0) rate = 44100; 
    else rate = 48000;
  }

    /* 
     * /usr/sbin/sfconvert  there is an esps version, so use sgi version
     */
    
    sprintf(playcom, "/usr/sbin/sfconvert %s %s -i int 16 2scomp chan 1 rate %f end rate %d", rawname, cvtraw, *samplingRate, rate);
    system(playcom);
      
    sprintf(playcom, "rm -f %s", rawname);  
    system(playcom);
    rawfile = cvtraw;
    
    sprintf(playcom, "sfplay -i int 16 2scomp chan 1 rate %d end %s", 
	    rate, rawfile);
    system(playcom);
    
    sprintf(playcom, "rm -f %s", rawfile); 
    system(playcom);
    *samplingRate = rate;
  }
#elif defined(linux)
  int rate; /* sampling rate */
  int availFormats, format;
  int stereo = MONO;
  short *waveLocation;
  int i, len, samplesWritten;

// if audioFD is non-zero assume play in progress; abort it

  if (audioFD) KillPlay();

  /*
   * Open up the audio device
   */

  if ((audioFD = open(DSP_DEVICE, O_WRONLY, 0)) == -1) {
      perror("Error opening audio device.");
      audioFD = 0;
      return(-1);
  }

  /* 
   * Get supported formats and check if formats are compatible with
   * what we want. See if it supports 16 bit sound.  If not, setup for
   * 8bit sound.
   */

  if (ioctl(audioFD, SNDCTL_DSP_GETFMTS, &availFormats) == -1) {
    perror("Error: Can't get soundfile formats.");
    close(audioFD);
    audioFD = 0;
    return(-1);
  }
  
  if (swap) format = AFMT_S16_BE;
  else format = AFMT_S16_LE;

  /*
   * KKG: Need to change code to check for 8 bit and change code to
   * play 8 bit 
   */

  if (! (availFormats & format)) {
      perror("Error:  Don't know how to support format.");
      close(audioFD);
      audioFD = 0;
      return(-1);
  }

  /*
   * Set format, stereo, and sampling rate
   */

  if (ioctl(audioFD, SNDCTL_DSP_SETFMT, &format) == -1) {
    perror("Error:  Can't set soundfile format.");
    close(audioFD);
    audioFD = 0;
    return(-1);
  }

  if (ioctl(audioFD, SNDCTL_DSP_STEREO, &stereo) == -1) {
    perror("Error:  Can only play mono files.");
    close(audioFD);
    audioFD = 0;
    return(-1);
  }

  rate = (int) *samplingRate;

  if (ioctl(audioFD, SNDCTL_DSP_SPEED, &rate) == -1) {
    perror("Error: Can't set sampling rate.");
    close(audioFD);
    audioFD = 0;
    return(-1);
  }
  *samplingRate = rate;  // rate actually used

  /*
   *  Get block size
   */

  ioctl(audioFD, SNDCTL_DSP_GETBLKSIZE, &bufSize);
  if (bufSize < 2 || bufSize > 65536) {
    fprintf (stderr, "Invalid audio buffer size (%d)\n", bufSize);
    close(audioFD);
    audioFD = 0;
    return(-1);
  }

  /*
   * Write zeros (to prevent clicks), and write data to device
   */

//  for (i = 0; i < NUM_ZEROS; i++) zeroBuf[i] = 0;
  if (write(audioFD, zeroBuf, NUM_ZEROS) < 0) {
    perror("AUDIO WRITE");
    close(audioFD);
    audioFD = 0;
    return(-1);
  }

// play & return mode (i.e. klplay)
  if (immed) {
    i = 0; /* Counter of number of samples written */

    while (i < numSamples) {

    /* 
     * If we have less than the NUM_BUF samples, only do that amount 
     *  else do a block of BufSize. 
     */

      if ((i + bufSize) > numSamples) samplesWritten = numSamples - i;
      else samplesWritten = bufSize;

    /* 
     * Write the data to the device 
     */

      waveLocation = (wave + i);
      if ((len = write(audioFD, waveLocation, 2*samplesWritten)) == -1) {
        perror("Audio write");
        close(audioFD);
        return(-1);
      }
      i += len/2;
    }

  /*
   * Write zeros (to prevent clicks), and write data to device
   */

    if ((len = write(audioFD, zeroBuf, NUM_ZEROS)) == -1) {
      perror("Audio write");
      close(audioFD);
      return(-1);
    }
    close(audioFD);
    return(0);

// init play loop
  } else {
    numSamps = numSamples;   // samples left to play
    waveLoc = wave;          // buffer location
    return 0;                // success - install work proc (PlayLoop)
  }
  // while (!PlayLoop(0)) ;  // till I figure out how to use work procs with XtAppPeekEvent()
  // return 0;
  
  #endif

  int rate;    /* sampling rate */
  FILE *rawfp; /* file for sfplay */
  static char rawname[] = "/tmp/kltemp.xkl";
  static char klname[] = "./kltemp.wav";
    static char msname[] = "./kltemp_ms.wav";
  static char cvtraw[] = "/tmp/cvt.xkl";
  char playcom[300], *rawfile;


  
    rawfp = fopen(rawname, "w");
    fwrite(wave, sizeof(short), (size_t) numSamples, rawfp);
    fclose(rawfp);
    rate = (int) *samplingRate;
    rawfile = rawname; /* if no sampling rate conversion needed */

  /* 
   * For now just convert to 8000,11025 or 16000 the hardware
   * can play files sampled up to 48k
   */
  


    /* 
     * /usr/sbin/sfconvert  there is an esps version, so use sgi version
     */
        sprintf(playcom, "../utils/raw2kl -f %d %s %s", rate, rawfile, klname);
            system(playcom);
    sprintf(playcom, "../utils/kl2ms %s", klname);
            system(playcom);
    sprintf(playcom, "play %s", msname);
    system(playcom);
    
sprintf(playcom, "rm -f %s", rawfile); 
    system(playcom);
    sprintf(playcom, "rm -f %s", klname); 
    system(playcom);
    sprintf(playcom, "rm -f %s", msname); 
    system(playcom);
return(0);
}

/*************************************************************************
 *
 * int record(short *wave, float *samplingRate, float duration, int swap)
 *
 * Record a .wav wave file (*wave) from audio device at samplingRate, and 
 * whose length is duration (in seconds). swap defines whether to swap 
 * the bytes or not.  The actual sampling rate at which the sound was 
 * recorded at is returned in samplingRate.  record allocates the memory
 * for wave.  In addition, record returns the number of samples read, or 
 * if there is an error, then record returns -1.
 * 
 *************************************************************************/
 /* This function is left here for historical reasons, but is not used anymore. 
 It was replaced by the recordToFile() function (see later in the file) that uses sox to record
 a waveform on disk*/
 
short *record(float *samplingRate, float duration, int *nsamps, int swap)
{
#if defined(sgi)
  char str[256];
  int rate;
  short *wave;

  rate = (int) *samplingRate;
  sprintf(str,"recordaiff -n 1 -s 16 -r %d -t %.1f /tmp/rec.aif",
	  (int)*samplingRate, duration);
  system(str);

  system("/usr/sbin/sfconvert /tmp/rec.aif /tmp/rec.raw -o byte little");
  /* debug need to change rawtowav directory */
  sprintf(str,"rawtowav -f rate /tmp/rec.raw /tmp/rec.wav");
  system(str);
  strcpy(str, "/tmp/rec.wav");
  wave = (short *) getWavWaveform(str, samplingRate, nsamps, swap);
  *nsamps = (int) (duration * *samplingRate);
  return(wave);
#elif defined(linux)
  int audioFD;
  int rate; /* sampling rate */
  int availFormats, format;
  int stereo = MONO;
  short *waveLocation;
  int i, len, samplesRead, bufSize;
  short *wave = NULL;
  int numSamples;

  /*
   * Open up the audio device
   */

  if ((audioFD = open(DSP_DEVICE, O_RDONLY, 0)) == -1) {
      perror("Error opening audio device");
      return(NULL);
  }
  /* 
   * Get supported formats and check if formats are compatible with
   * what we want. See if it supports 16 bit sound.  If not, setup for
   * 8bit sound.
   */

  if (ioctl(audioFD, SNDCTL_DSP_GETFMTS, &availFormats) == -1) {
    perror("Error: Can't get soundfile formats.");
    close(audioFD);
    return(NULL);
  }
  
  if (swap) format = AFMT_S16_BE;
  else format = AFMT_S16_LE;

  /*
   * KKG: Need to change code to check for 8 bit and change code to
   * play 8 bit 
   */

  if (! (availFormats & format)) {
      perror("Error:  Don't know how to support format.");
      close(audioFD);
      return(NULL);
  }

  /*
   * Set format, stereo, and sampling rate
   */

  if (ioctl(audioFD, SNDCTL_DSP_SETFMT, &format) == -1) {
    perror("Error:  Can't set soundfile format.");
    close(audioFD);
    return(NULL);
  }

  if (ioctl(audioFD, SNDCTL_DSP_STEREO, &stereo) == -1) {
    perror("Error:  Can only play mono files.");
    close(audioFD);
    return(NULL);
  }

  rate = (int) *samplingRate;

  if (ioctl(audioFD, SNDCTL_DSP_SPEED, &rate) == -1) {
    perror("Error: Can't set sampling rate.");
    close(audioFD);
    return(NULL);
  }

  /*
   *  Get block size
   */

  ioctl(audioFD, SNDCTL_DSP_GETBLKSIZE, &bufSize);
  if (bufSize < 2 || bufSize > 65536) {
    fprintf (stderr, "Invalid audio buffers size %d\n", bufSize);
    close(audioFD);
    return(NULL);
  }
  /*
   * Allocate initial buffer for wave.
   */

  numSamples = (int) (duration * rate);
  wave = (short *) calloc(numSamples, sizeof(short));
  
  i = 0; /* Counter of number of samples read */

  while (i < numSamples) {

    /* 
     * If we have less than the NUM_BUF samples, only do that amount 
     *  else do a block of BufSize. 
     */

    if ((i + bufSize) > numSamples) samplesRead = numSamples - i;
    else samplesRead = bufSize;

    /* 
     * Read the data from the device 
     */

    waveLocation = (wave + i);
    if ((len = read(audioFD, waveLocation, 2*samplesRead)) == -1) {
      perror("Audio write");
      close(audioFD);
      return(NULL);
    }
    i += len/2;
  }

  close(audioFD);

  *samplingRate = rate;
  *nsamps = i;
  return(wave);
#endif
 float rate = *samplingRate;   /* sampling rate */
 int nsamples=0;
 short *wave;
 static char msname1[] = "./kltemp_ms_orig.wav";
 static char msname2[] = "./kltemp_ms_10k_long.wav";
 static char msname3[] = "./kltemp_ms_10k.wav";
 static char msname4[] = "./kltemp_ms_10k_16b.wav";
 static char msname5[] = "./kltemp.wav";
 static char klname[] = "./kltemp_kl.wav";
 char reccom[300];
 int duration_int=(int) duration;
 duration_int=duration_int+1;
 sprintf(reccom,"sox -d --clobber --buffer $((%1.0f*%d)) %s trim 0 %1.0f",rate, duration_int, msname1, duration);
 fprintf(stderr,"%s",reccom);
 system(reccom);
 sprintf(reccom,"sox -r %1.0f %s %s speed 2.75625",rate, msname1, msname3);
 system(reccom);
 //sprintf(reccom,"sox %s %s trim 0 %1.0f",msname2, msname3, duration);
 //system(reccom);
 sprintf(reccom,"sox %s -b 16 %s dither -s",msname3, msname4);
 system(reccom);
 sprintf(reccom,"sox %s %s remix 1", msname4, msname5);
 system(reccom);
 
 sprintf(reccom, "../utils/ms2kl %s", msname5);
 system(reccom);
 wave = (short *) getWavWaveform(klname, samplingRate, nsamps, swap);

    sprintf(reccom, "rm -f %s", msname1); 
    system(reccom);
    sprintf(reccom, "rm -f %s", msname2); 
    system(reccom);
    sprintf(reccom, "rm -f %s", msname3); 
    system(reccom);
    sprintf(reccom, "rm -f %s", msname4); 
    system(reccom);
    sprintf(reccom, "rm -f %s", msname5); 
    system(reccom);
    //sprintf(reccom, "rm -f %s", klname); 
    //system(reccom);
	return(wave);
}
/*************************************************************************
 *
 * int recordToFile(float *samplingRate, float duration, char *fileName)
 *
 * Record a .wav wave file (*wave) from audio device at samplingRate, and 
 * whose length is duration (in seconds) using the sox utility. The file is saved in 
 * both Klatt and Microsoft .wav formats.
 * 
 *************************************************************************/
void recordToFile(float *samplingRate, float duration, char *fileName)
{
 float rate = *samplingRate;   /* sampling rate */
 float speedCorrection;
 float rateOrig;
 FILE * fp;
 int nsamples=0;
 short *wave;
 char *rateStr;
 static char msname1[] = "./kltemp_ms_orig.wav";
 static char msname3[] = "./kltemp_ms_10k.wav";
 static char msname4[] = "./kltemp_ms_10k_16b.wav";
 char reccom[300];
 int duration_int=(int) duration;
 duration_int=duration_int+1;
 sprintf(reccom,"sox -d --clobber --buffer $((%1.0f*%d)) %s trim 0 %1.0f",rate, duration_int, msname1, duration);
 fprintf(stderr,"%s\n",reccom);
 system(reccom);

 //sprintf(reccom,"export RATE=$(sox --i %s | grep 'Sample Rate'| grep -o -E '[0-9]+')",msname1);
 sprintf(reccom,"sox --i %s | grep 'Sample Rate'| grep -o -E '[0-9]+' >> rateValue.txt",msname1);
 fprintf(stderr,"%s\n",reccom);
 system(reccom);
 
 //rateStr=getenv("RATE");
 fp=fopen("rateValue.txt","r");
 //fprintf(stderr,"%s\n",rateStr);
 //sscanf(rateStr,"%f",&rateOrig);
 fscanf(fp,"%f",&rateOrig);
 fclose(fp);
 speedCorrection=rateOrig/rate;
 sprintf(reccom,"%f %f %f\n",rateOrig,rate,speedCorrection);
 fprintf(stderr,"%s",reccom);
 
 sprintf(reccom,"sox -r %1.0f %s %s speed %f",rate, msname1, msname3,speedCorrection);
fprintf(stderr,"%s\n",reccom);
 system(reccom);
 //sprintf(reccom,"sox %s %s trim 0 %1.0f",msname2, msname3, duration);
 //system(reccom);
 sprintf(reccom,"sox %s -b 16 %s dither -s",msname3, msname4);
 system(reccom);
 sprintf(reccom,"sox %s %s remix 1", msname4, fileName);
 system(reccom);
 
 sprintf(reccom, "../utils/ms2kl %s", fileName);
 system(reccom);
 //wave = (short *) getWavWaveform(klname, samplingRate, nsamps, swap);

    sprintf(reccom, "rm -f %s", msname1); 
    system(reccom);
    //sprintf(reccom, "rm -f %s", msname2); 
    //system(reccom);
    sprintf(reccom, "rm -f %s", msname3); 
    system(reccom);
    sprintf(reccom, "rm -f %s", msname4); 
    system(reccom);
    sprintf(reccom, "rm -f rateValue.txt"); 
    system(reccom);
    //sprintf(reccom, "rm -f %s", msname5); 
    //system(reccom);
    //sprintf(reccom, "rm -f %s", klname); 
    //system(reccom);
	//return(wave);
}