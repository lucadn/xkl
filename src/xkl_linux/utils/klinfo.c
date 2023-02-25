/*
 * $Log: klinfo.c,v $
 * Revision 1.8  1998/06/12 04:22:59  krishna
 * Changed argument processing based on changes to functions in
 * getargs.c.  Added flags to print out only certain quantities,
 * e.g. sampling rate.  Rewrote usage.
 *
 * Revision 1.7  1998/06/06 22:51:22  krishna
 * Added RCS log.
 *
 */

/***********************************************************************
 *
 * This program prints out info about a .wav file based on the header.
 * 
 * KKG 5/18/98, v.1.1
 * Added max and min waveform value.
 * KKG 11/04/97, v.1.0
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "wavio.h"
#include "getargs.h"
#include "wavdata.h"

int main(int argc, char **argv);
void usage(char *name);

int main(int argc, char **argv)
{
   FILE *fp;
   float samplingRate;
   int numSamples, numsamps;
   int i, version;
   char iname[256], progName[256];
   short max, min;
   short *wave;
   char *str;
   int printFrequency, printVersion, printEndian, printNumSamples;
   int swap = SWAP;

  /* 
   * Check for help or ?, and if right number of args
   */
   
   strcpy(progName, argv[0]);

   if (IsArg("-h", &argc, argv) || IsArg("-H", &argc, argv) ||
       IsArg("-help", &argc, argv) || IsArg("--help", &argc, argv) ||
       IsArg("-?", &argc, argv))
     usage(progName);

   printFrequency = IsArg("-f", &argc, argv);
   printNumSamples = IsArg("-n", &argc, argv);
   printVersion = IsArg("-v", &argc, argv);
   printEndian = IsArg("-e", &argc, argv);
   if ((str = RemainingArgs(argc, argv)) != NULL) {
     fprintf(stderr, "Unknown or duplicate argument: %s\n", str);
     usage(progName);
   }
   if (argc < 2) usage(progName);

   /*
    * Loop thru files
    */

   for (i = 1; i < argc; i++) {
     strcpy(iname, argv[i]);

     /* Open wave, and get header info */

     if (openWav(iname, &fp, READ) == ERROR) {
       fprintf(stderr, "No wavefile %s...skipping file\n", iname);
       continue;
     }
     if (readWavHeader(fp, &samplingRate, &numSamples, 
		       &version, swap) == ERROR) {
       fprintf(stderr, "Error reading header from %s...skipping file\n", iname);
       fclose(fp);
       continue;
     }

     if (printFrequency || printVersion || printEndian || printNumSamples) {
       if (printFrequency) fprintf(stdout, "%d\n",(int) samplingRate);
       else if (printNumSamples) fprintf(stdout, "%d\n", numSamples);
       else if (printVersion) fprintf(stdout, "%d\n", version);
       else if (printEndian) fprintf(stdout, "%d\n", swap);

       fclose(fp);
       continue;
     }

     /* 
      * Allocate memory for entire waveform
      */
  
     if ((wave = (short *) calloc(numSamples, sizeof(short))) == NULL) {
       fprintf(stderr, "Insufficient memory for %d samples\n", numSamples);
       closeWav(fp);
       continue;
     }
  
     /* 
      * Read wave data.  Check if number of samples read equals what is
      * stated in header, i.e. numSamples, and tell user.
      */

     numsamps = readWav(fp, wave, numSamples, swap);
     if (numsamps == ERROR) {
       fprintf(stderr, "Error reading waveform from %s\n", iname);
       closeWav(fp);
       continue;
     }

     if (numsamps != numSamples) {
       fprintf(stderr, "Read '%s' sampled at %d samp/sec\n", iname, (int)
	       samplingRate);
       fprintf(stderr, "\n\t\tHeader indicates %d samples...", numSamples);
       fprintf(stderr, "but can only read %d samples.\n", numsamps);
       numSamples = numsamps;
     } 


     max = findMaxValue(wave, numSamples);
     min = findMinValue(wave, numSamples);

     /* Write out info */

     fprintf(stdout, "\nWave info for %s:\n\n", iname);
     fprintf(stdout, "\tSampling rate: %d\n", (int) samplingRate);
     fprintf(stdout, "\tNumber of samples: %d\n", numSamples);
     fprintf(stdout, "\tHeader version: %d\n", version);
     fprintf(stdout, "\tUsing big endian format: %s\n", 
	     (swap == 1)? "Yes" : "No");
     fprintf(stdout, "\tMinimum value: %d\n", min);
     fprintf(stdout, "\tMaximum value: %d\n", max);
     fprintf(stdout, "\n");

     if (wave != NULL) free(wave);
     closeWav(fp);
   }
}

void usage(char *name)
{
  fprintf(stderr, "\nUsage: %s [-h | --help] [-f | -n | -v | -e] <wav1> <wav2> ...\n", 
	  name);
  fprintf(stderr, "\t-f prints out only the sampling frequency.\n");
  fprintf(stderr, "\t-n prints out only the number of samples.\n");
  fprintf(stderr, "\t-v prints out only the header version number.\n");
  fprintf(stderr, "\t-e prints out only the endian type.\n");
  fprintf(stderr, "\t<wav1> <wav2> are the .wav file (.wav not needed).\n");
  fprintf(stderr, "e.g. %s bad\n", name);
  exit(1);
}
