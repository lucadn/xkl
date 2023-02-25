/*
 * $Log: concat.c,v $
 * Revision 1.6  2001-09-14 16:13:16-04  tiede
 * clear sample accumulator initially (sample overrun fix)
 *
 * Revision 1.5  1998/06/09 00:35:50  krishna
 * Changed argument processing based on changes to functions in
 * getargs.c.  Rewrote usage, and added a print statement about which
 * file was being read in.
 *
 * Revision 1.4  1998/06/06 22:51:22  krishna
 * Added RCS log.
 *
 */

/*********************************************************************
 *
 *                             CONCAT.C
 *
 * Read in (at least) two waveform files, concatenate them, and 
 * write to output file.
 *
 * EDIT HISTORY:
 * 1.0 2-Jan-85, DK	Initial creation
 * 1.1 4/94, DH(?), edited to use getWavWaveform -- this used to use in
 * modules the kl library, .wav produced with with the old version of 
 * concat have a 504 byte header instead of the expected 512.
 * 1.2 10/30/97 KKG  Cleaned up code...deleted unused variables, etc.
 * Included a check for the same sampling rates across input waveform
 * files.  Included function calls to functions in wave_util.c.
 * 1.3 4/15/98 KKG  Added check for existing output file, and increased 
 * MAXWAVEFORMS to 100.
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wavio.h"
#include "getargs.h"

/*
 *  Prototypes
 */

int main(int argc, char **argv);
void usage(char *name);
void freeWaves(short **wavlist, int nwaves);


/*
 * Globals
 */

#define MAXWAVEFORMS 100

/***********************************************************************
 *                              MAIN
 ***********************************************************************/

int main(int argc, char **argv)
{
  FILE *fp;                     /* Output file pointer */
  char fname[500];              /* Temporary storage of filename */
  short *wavlist[MAXWAVEFORMS];   /* Pointers to all waveform data */
  short *data;                  /* Temp pointer to data */
  int nwtot[MAXWAVEFORMS];	/* # of samples in each waveform */
  int numWaves;		        /* Number of current waveform file */
  float curRate, samplingRate;  /* sampling rates */
  int i, nw;                    /* Counter */
  char progName[256];           /* Current program name */
  char *str;
  char answer[100];
   
  strcpy(progName, argv[0]); /* Program name */

  /*
   * Check for help
   */

  if (IsArg("-h", &argc, argv) || IsArg("-H", &argc, argv) || 
      IsArg("-help", &argc, argv) || IsArg("--help", &argc, argv) ||
      IsArg("-?", &argc, argv))
    usage(progName);

  if ((str = RemainingArgs(argc, argv)) != NULL) {
    fprintf(stderr, "Unknown or duplicate argument: %s\n", str);
    usage(progName);
  }
  if (argc < 4) usage(progName); /* Must be at least 3 args */

  numWaves = -1;
  for (i = 1; i < argc; i++) {  /* Main loop over args. */

    strcpy(fname, argv[i]);
    numWaves++;

    /*
     * Check if we are overflowing our buffers.
     */

    if (numWaves == MAXWAVEFORMS - 1) {
      fprintf(stderr, 
	      "\tError: Program can't take more than %d input waveforms.\n", 
	      MAXWAVEFORMS - 1);
      usage(progName);
    }

    /* 
     * If not last argument, read and load the waveform
     */

    if (i < argc - 1) {
      wavlist[numWaves] = getWavWaveform(fname, &curRate, 
					 &(nwtot[numWaves]), SWAP);

      fprintf(stdout, "Reading %s (SR = %.0f Hz; %d samples)...\n", fname, curRate, nwtot[numWaves]);
      if (wavlist[numWaves] == NULL) {
	  fprintf(stderr, "Error: Could not read waveform %s.\n", fname);
	  fprintf(stderr, "**** SKIPPING FILE %s in concat\n", fname);
	  free(wavlist[numWaves]);
	  wavlist[numWaves] = NULL;
      }

      /*
       * First waveform sets Master sampling rate.  Otherwise, check 
       * verify sampling rate is same as Master.
       */

      if (i == 1) samplingRate = curRate;  
      else if (samplingRate != curRate) {
	fprintf(stderr, 
		"Error: Sampling rate for %s isn't same as other files.\n", 
		fname);
	fprintf(stderr, "**** SKIPING FILE %s ****\n", fname);
	free(wavlist[numWaves]);
	wavlist[numWaves] = NULL;
      }
    }
    else {

    /* 
     * The last arg. So, write out concatenated wavefiles. First, 
     * check if output file exists.
     */

      checkWavName(fname);

      /* Now write out the output file */

      if (openWav(fname, &fp, WRITE) == ERROR) {
	fprintf(stderr, "Error: Problem opening the output wave file %s.\n", 
		fname);
	freeWaves(wavlist, numWaves);
	exit(-1);
      }
      nwtot[numWaves] = 0;   // samp overrun bugfix
      for (nw = 0; nw < numWaves; nw++) nwtot[numWaves] += nwtot[nw];

      if (writeWavHeader(fp, (int) samplingRate, 
			 nwtot[numWaves], SWAP) == ERROR) {
	fprintf(stderr, 
		"Error: Problem writing header to output wave file %s.\n", 
		fname);
	freeWaves(wavlist, numWaves);
	exit(-1);
      }

      for (nw = 0; nw < numWaves; nw++) {
	data = (short *) wavlist[nw];
	if (writeWav(fp, data, nwtot[nw], SWAP) == ERROR) {
	  fprintf(stderr,
		  "Error: Problem writing header to output wave file %s.\n", 
		  fname);
	  freeWaves(wavlist, numWaves);
	  exit(-1);
	}
      }
      closeWav(fp);
      fprintf(stderr, 
	      "\nConcatenated waveform saved in %s (%d samples at %d Hz).\n\n",
	      fname, nwtot[numWaves], (int) samplingRate);
    }
  }
}

void freeWaves(short **wavlist, int nwaves) {
  register int nw;

  for (nw = 0; nw < nwaves; nw++) 
    if (wavlist[nw] != NULL) free(wavlist[nw]);
}

/***********************************************************************
 *                              Usage()
 ***********************************************************************/

void usage(char *name)
{
  fprintf(stderr, "\nUsage: %s [-h | --help] <wav1> <wav2> [<wav3> ...] <Outfile>\n", name);
  fprintf(stderr, "\t<wav1>, <wav2>, <wav3> are the input, Klatt .wav files\n");
  fprintf(stderr, "\t\tto be concatenated.\n");
  fprintf(stderr, "\tFiles must have the same sampling rate. If a file\n");
  fprintf(stderr, "\t\thas a different sampling rate it is excluded.\n");
  fprintf(stderr, "\t<Outfile> is the output waveform file.\n");
  fprintf(stderr, "\tThe .wav extension is optional for all filenames.\n");
  fprintf(stderr, "e.g., %s ba li bali\n", name);
  exit(1);
}

