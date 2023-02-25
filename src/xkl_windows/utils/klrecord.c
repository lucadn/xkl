/*
 * $Log: klrecord.c,v $
 * Revision 1.4  1998/06/09 00:38:53  krishna
 * Changed argument processing based on changes to functions in
 * getargs.c.
 *
 * Revision 1.3  1998/06/06 22:51:22  krishna
 * Added RCS log.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wavio.h"
#include "wavaudio.h"
#include "getargs.h"

#define DEFAULT_SAMPLING_RATE 10000 /* Hz */
#define DEFAULT_DURATION 5.0    /* Seconds */

void usage(char *name);
int main(int argc, char **argv);

int main(int argc, char **argv)
{
   float samplingRate, rate;
   float duration;
   int numSamples;
   short *wave = NULL;
   char fname[256], progName[256], dummy, *str;
   char answer[100];
   FILE *fp;

   strcpy(progName, argv[0]);

   if (IsArg("-h", &argc, argv) || IsArg("-H", &argc, argv) || 
       IsArg("-help", &argc, argv) || IsArg("--help", &argc, argv) ||
       IsArg("-?", &argc, argv))
     usage(progName);

   /*
    * Get command line parameters for sampling rate and duration.
    */

   samplingRate = checkIntArg("-f", &argc, argv, DEFAULT_SAMPLING_RATE);
   duration = checkFloatArg("-d", &argc, argv, DEFAULT_DURATION);


   if ((str = RemainingArgs(argc, argv)) != NULL) {
     fprintf(stderr, "Unknown or duplicate argument: %s\n", str);
     usage(progName);
   }

   /* 
    * Check for wave file name
    */

   if (argc != 2) usage(progName);
   strcpy(fname, argv[1]);

   rate = samplingRate; /* Requested rate not necessarily actual rate */

   fprintf(stdout, "Hit a key to start recording for %4.3f seconds...", 
	   duration);
   dummy = getchar();

   /*
    * Record wave file
    */

   if ((wave = record(&rate, duration, &numSamples, SWAP)) == NULL) {
     fprintf(stderr, "Error trying to record from audio device\n");
     exit(-1);
   }
   checkWavName(fname);
   if (putWavWaveform(fname, wave, rate, numSamples, SWAP) == ERROR)
     fprintf(stderr, "Error writing wavefile %s\n", fname);

   fprintf(stdout, 
   "\nRecorded %s at %d Hz (requested %d Hz) for %4.3f sec (%d samples).\n", 
	   fname, (int) rate, (int) samplingRate, duration, numSamples);

   if (wave != NULL) free(wave);
}

void usage(char *name)
{
  fprintf(stderr, "\nUsage: %s [-h | --help] [-f <rate>] [-d <duration>] <wavefile>\n", name);
  fprintf(stderr, "\t<rate> is the sampling rate in Hz.\n");
  fprintf(stderr, "\t<duration> is the duration in seconds.\n");
  fprintf(stderr, "\t<wavefile> is the file containing the recording (.wav not needed).\n");
  fprintf(stderr, "\tDefault sampling rate is %d Hz, default duration is %4.3f seconds.\n",  DEFAULT_SAMPLING_RATE, DEFAULT_DURATION);
  fprintf(stderr, "e.g. : %s -d 3 word\n", name);
  exit(0);
}
