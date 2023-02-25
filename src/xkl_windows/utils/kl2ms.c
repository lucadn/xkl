/***********************************************************************
 *
 * This program converts a Klatt .wav file to a MS .wav file
 *
 ***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wavio.h"
#include "getargs.h"

void usage(char *name);

int main(int argc, char **argv)
{
   FILE *wfp;
   short *data, ck_end = 1;
   float samplingRate;
   int numSamples, i, swap, l_end;
   char infile[128], outfile[128], progName[256], *str;


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
   if (argc < 2) usage(progName);


   /* Check if platform is little endian; if not, set swap to 1 */

   l_end = (*((char*) &ck_end) == 1);
   swap = ((l_end) ? 0 : 1);


   /* for each input file, open it and save the data in a MS .wav file */

   for (i = 1; i < argc; i++ ) {

     strcpy(infile, argv[i]);
     printf("infile = %s\n", infile); 
   
     data = getWavWaveform(infile, &samplingRate, &numSamples, swap);

     strcpy(outfile,infile); 
     outfile[strlen(outfile) - 4 ]  = '\0';
     strcat(outfile, "_ms");  

     if (openWav(outfile, &wfp, WRITE) == ERROR) {
       fprintf(stderr, "Error writing to file %s\n", outfile);
       /* exit(0); */
     }
     else {
       writeMSWavHeader(wfp, samplingRate, numSamples, 1, 2, swap);

       writeWav(wfp, data, numSamples, swap);

       closeWav(wfp);
       
       fprintf(stderr, "MS .wav file written to %s (%d samples at %d Hz).\n", 
	       outfile, numSamples, (int) samplingRate);
     }
   }
exit(0);
}

void usage(char *name)
{
  fprintf(stderr, "\nUsage: %s [-h | --help] <files> ...\n", name);
  fprintf(stderr, "\t<files> are Klatt .wav files (.wav not needed).\n");
  fprintf(stderr, "\tThe output MS .wav file names are formed by\n");
  fprintf(stderr, "\tby appending the string _ms to the root of the\n");
  fprintf(stderr, "\tinput file names.\n");
  fprintf(stderr, "e.g. %s in_kl_file1 in_kl_file2\n", name);
  fprintf(stderr, "\twill produce output files named in_kl_file1_ms.wav\n");
  fprintf(stderr, "\tand in_kl_file2_ms.wav\n");
  exit(0);
}
