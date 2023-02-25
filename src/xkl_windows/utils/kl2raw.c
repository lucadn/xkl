/*
 * $Log: kl2raw.c,v $
 * Revision 1.5  1998/06/09 00:36:38  krishna
 * Changed argument processing based on changes to functions in
 * getargs.c.
 *
 * Revision 1.4  1998/06/06 22:51:22  krishna
 * Added RCS log.
 *
 */

/***********************************************************************
 *
 * This program writes out a raw soundfile that is in 16 bit signed
 * little endian format.  
 *
 * On sgi the sf description of the raw .wav format is:
 * -i byteorder little integer 16 2scomp chan 1
 *
 * 
 * KKG 11/04/97, v.1.2
 * - Commented, cleaned up. 
 * - Incorporated wave_util.c functions.
 *
 ***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wavio.h"
#include "getargs.h"

int main(int argc, char **argv);
void usage(char *name);

int main(int argc, char **argv)
{
   FILE *wfp;
   short *data;
   float samplingRate;
   int numSamples;
   char iname[128], oname[128], progName[256], *str;
   int i;

  /* 
   * Check for help or ?, and if right number of args
   */

   
   strcpy(progName, argv[0]);

   if (IsArg("-h", &argc, argv) || IsArg("-H", &argc, argv) || 
       IsArg("-help", &argc, argv) || IsArg("--help", &argc, argv) ||
       IsArg("-?", &argc, argv))
     usage(progName);

   if ((str = RemainingArgs(argc, argv)) != NULL) {
     fprintf(stderr, "Unknown or duplicate argument: %s\n", str);
     usage(progName);
   }
   if (argc != 3) usage(progName);

   strcpy(iname, argv[1]);
   strcpy(oname, argv[2]);
   
   data = getWavWaveform(iname, &samplingRate, &numSamples, SWAP);
   if (!(wfp = fopen(oname, "w"))) { 
      fprintf(stderr, "Error: Could not write output file, %s.\n", oname);
      exit(0);
   }

   writeWav(wfp, data, numSamples, SWAP);
   closeWav(wfp);
   fprintf(stderr, "Raw file written to %s (%d samples at %d Hz).\n", 
	     oname, numSamples, (int) samplingRate);
}

void usage(char *name)
{
  fprintf(stderr, "\nUsage: %s [-h | --help] <infile> <outfile>\n", name);
  fprintf(stderr, "\t<infile> is the Klatt .wav file (.wav not needed).\n");
  fprintf(stderr, "\t<outfile> is a headerless, single channel 16-bit\n");
  fprintf(stderr,"\t\tsigned (2scomp), little endian binary file.\n");
  fprintf(stderr,"e.g. %s in out.raw\n", name);
  exit(0);
}
