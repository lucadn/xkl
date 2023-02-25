/*
 * $Log: raw2kl.c,v $
 * Revision 1.5  1998/06/09 00:37:53  krishna
 * Changed argument processing based on changes to functions in
 * getargs.c.  Modified usage.
 *
 * Revision 1.4  1998/06/06 22:51:22  krishna
 * Added RCS log.
 *
 */

/***********************************************************************
 *
 * This program reads a raw soundfile that is in 16 bit signed
 * little endian format.  The user enters the sampling rate as a 
 * command line argument and the program writes out the data with
 * a .wav header that contains the following:
 *  - 32 bit int that is the number of usec between sample
 *  - 32 bit int that is the number of samples
 *  - 32 bit int that is the version number 1 in this case
 *  - the remainder of the 512 byte header is filled with zeros
 *
 * On sgi the sf description of the raw .wav format is:
 * -i byteorder little integer 16 2scomp chan 1
 *
 * 
 * KKG 2/25/97, v.1.1
 * - Commented
 * - Made more clean.
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wavio.h"
#include "getargs.h"

int main(int argc, char **argv);
void usage(char *name);

#define CHUNK 10000  /* Chunk of data to read */

int main(int argc, char **argv)
{
   char data[CHUNK];
   int i;
   float samplingRate;
   int count, numread, numargs;
   FILE *rfp, *wfp;
   char iname[256], oname[256], progName[256], *str;

/* 
 * Need file name and sampling rate.  If not provided, exit.
 * Also, Check for help or ?, and if right number of args
 */

   strcpy(progName, argv[0]);

   if (IsArg("-h", &argc, argv) || IsArg("-H", &argc, argv) || 
       IsArg("-help", &argc, argv) || IsArg("--help", &argc, argv) ||
       IsArg("-?", &argc, argv))
     usage(progName);

   samplingRate = checkIntArg("-f", &argc, argv, -1);

   if ((str = RemainingArgs(argc, argv)) != NULL) {
     fprintf(stderr, "Unknown or duplicate argument: %s\n", str);
     usage(progName);
   }

   if ((samplingRate == -1) || (argc != 3)) {
     printf("\nNOTE NEW USAGE -- sampling rate is specified differently now.\n");
     usage(progName);
   }
   
   strcpy(iname, argv[1]);
   strcpy(oname, argv[2]);

   openWav(oname, &wfp, WRITE);

   /* 
    *Can read from input and write to output file. So,  write out all
    * zero header right now and fill it in later after reading all the
    * data.
    */

   for (i = 0; i < 512; i++) data[i] = 0;
   fwrite(data, sizeof(char), 512, wfp);

   if (!(rfp = fopen(iname, "r"))) { 
      fprintf(stderr, "Error: Could not read input file, %s.\n", iname);
      exit(0);
   }

   /* 
    * Read all the data from the input file and write to output file
    */

   count = 0; /* Number of shorts read */

   while((numread = fread(data, sizeof(char), CHUNK, rfp)) == CHUNK) {
      fwrite(data, sizeof(char), numread, wfp);
      count += numread;
   } 
   fwrite(data, sizeof(char), numread, wfp);

   count += numread;   /* Add last numread */
   count = count >> 1; /* Number of shorts  (2 bytes = 1 short) */
   fclose(rfp);

   fseek(wfp, 0, 0); /* rewind */
   writeWavHeader(wfp, samplingRate, count, SWAP);
   closeWav(wfp);


   fprintf(stderr, "Read %s....Wrote %d samples at %d Hz to %s.\n\n",
	   iname, count, (int) samplingRate, oname);
}

void usage(char *name)
{
   fprintf(stderr, "\nUsage: %s [-h | --help] -f <sampling rate> <infile> <outfile>\n", name);
   fprintf(stderr, "\t<infile> must be a headerless, single channel 16-bit\n");
   fprintf(stderr, "\t\tsigned (2scomp), little endian data.\n");
   fprintf(stderr, "\tFor <outfile>, .wav is optional.\n");
   fprintf(stderr, "\t<sampling rate> should be an integer.\n");
   fprintf(stderr,"e.g. %s -f 11000 in.raw out\n", name);
   exit(0);
} 
