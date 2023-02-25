/***********************************************************************
 *
 * This program converts a MS .wav file to a Klatt .wav file
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
   char progName[256], *str, infile[256], outfile[256];
   int swap, l_end, numSamples, channels, bytesPerSample, i;
   short ck_end = 1, *data;
   FILE *ifp, *ofp;
   float samplingRate;

   
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

     strcpy(infile,argv[i]);
     printf("infile = %s\n", infile); 

     if (openWav(infile, &ifp, READ) == ERROR) {
       fprintf(stderr, "Error opening file %s\n", infile);

     }
     else {
       if (readMSWavHeader(ifp, infile, &samplingRate, &numSamples, &channels, 
			   &bytesPerSample, swap) == ERROR) {
	 fprintf(stderr, "Error reading waveform header in %s\n", infile);
	 printf("SR = %f.0, channels = %d, bps = %d\n", samplingRate, 
	      channels, bytesPerSample);closeWav(ifp);

       }
       else {

	 /* 
	  * Allocate memory for entire waveform
	  */
  
	 if ((data = (short *) calloc(numSamples, sizeof(short))) == NULL) {
	   fprintf(stderr, "Insufficient memory for %d samples\n", numSamples);
	   closeWav(ifp);
	   exit(0);
	 }
  
	 /* 
	  * Read wave data.  Check if number of samples read equals what is
	  * stated in header, i.e. numSamples, and tell user.
	  */

	 if (readWav(ifp, data, numSamples, swap) == ERROR) {
	   fprintf(stderr, "Error reading waveform from %s\n", infile);
	   closeWav(ifp);

	 }
	 else {
	   closeWav(ifp);

	   strcpy(outfile,infile); 
	   outfile[strlen(outfile) - 4 ]  = '\0';
  	   strcat(outfile, "_kl");  

	   if (putWavWaveform(outfile, data, samplingRate, numSamples, swap)
	       == ERROR) {
	     fprintf(stderr, "Error writing waveform to %s\n", outfile);
	     free(data);

	   }
	   else {
	     fprintf(stderr, 
		     "Klatt file written to %s (%d samples at %d Hz)\n", 
		     outfile, numSamples, (int) samplingRate);
	     free(data);
	   }
	 }
       }
     }
   }
exit(0);
}

void usage(char *name)
{
  fprintf(stderr, "\nUsage: %s [-h | --help] <files> ...\n", name);
  fprintf(stderr, "\t<files> are MS .wav files (.wav not needed).\n");
  fprintf(stderr, "\tThe output Klatt file names are formed by\n");
  fprintf(stderr, "\tby appending the string _kl to the root of the\n");
  fprintf(stderr, "\tinput file names.\n");
  fprintf(stderr, "e.g. %s in_ms_file1 in_ms_file2\n", name);
  fprintf(stderr, "\twill produce output files named in_ms_file1_kl.wav\n");
  fprintf(stderr, "\tand in_ms_file2_kl.wav\n");
  exit(0);
}




