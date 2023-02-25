/*
 * $Log: klplay.c,v $
 * Revision 1.7  2001-09-14 16:14:12-04  tiede
 * interface to play() changed (immediate play & return)
 *
 * Revision 1.6  1998/06/09 00:48:36  krishna
 * Changed argument processing based on changes to functions in
 * getargs.c.  Rewrote usage, and added -p argument, which allows for
 * pausing between playing wavfiles.
 *
 * Revision 1.5  1998/06/06 22:51:22  krishna
 * Added RCS log.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "wavio.h"
#include "wavaudio.h"
#include "getargs.h"

#define PAUSE_DURATION 2  /* Pause duration in seconds */
#define IMMEDIATE 1       /* play & return */

int main(int argc, char **argv);
void usage(char *name);


int main(int argc, char **argv)
{
   float samplingRate, rate;
   int numSamples;
   int i, Pause;
   short *wave;
   char fname[256], progName[256], *str;
   
   strcpy(progName, argv[0]);

   if (IsArg("-h", &argc, argv) || IsArg("-H", &argc, argv) || 
       IsArg("-help", &argc, argv) || IsArg("--help", &argc, argv) ||
       IsArg("-?", &argc, argv))
     usage(progName);

   Pause = IsArg("-p", &argc, argv);

   if ((str = RemainingArgs(argc, argv)) != NULL) {
     fprintf(stderr, "Unknown or duplicate argument: %s\n", str);
     usage(progName);
   }
   if (argc < 2) usage(progName);

   /*
    * For each wavefile to play, read the file, and then play it out.
    */

   for (i = 1; i < argc; i++) {
     strcpy(fname, argv[i]);
     wave = (short *) getWavWaveform(fname, &samplingRate, 
				     &numSamples, SWAP);
     rate = samplingRate;

     play(wave, &rate, numSamples, SWAP, IMMEDIATE);

     fprintf(stdout, "Playing %s at %d Hz (requested SR: %d Hz)", 
	     fname, (int) rate, (int) samplingRate);
     fprintf(stdout, " for %3.1f sec.\n", (int) numSamples/samplingRate);

     free(wave);
     if ((i != (argc - 1)) && Pause)  /* If not last file, pause */
       sleep(PAUSE_DURATION);
   }
}

void usage(char *name)
{
  fprintf(stderr, "\nUsage: %s [-h | --help] [-p] <wav1> <wav2> ...\n", name);
  fprintf(stderr, "\t-p specifies that the program should pause %d seconds", 
	  PAUSE_DURATION);
  fprintf(stderr, "\n\t  between playing wave files.\n");
  fprintf(stderr, "\t<wav1>, <wav2> are the Klatt .wav file ");
  fprintf(stderr, "(.wav not needed).\n");
  fprintf(stderr, "e.g. %s hello\n", name);
  exit(1);
}
