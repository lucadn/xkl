/*
 * $Log: klgain.c,v $
 * Revision 1.2  1998/06/12 04:46:22  krishna
 * Changed argument processing based on changes to functions in
 * getargs.c.
 *
 * Revision 1.1  1998/06/06 22:12:47  krishna
 * Initial revision
 *
 */

/***********************************************************************
 *
 * KLGAIN
 * 
 * This program adjusts the gain in a .wav to the MAX_RANGE * max
 * possible amplitude or by a factor that the user specifies.
 *
 * Currently the maximum is 75% of the max.  This was decided based on
 * VAX users' knowledge of recording and clipping.
 * 
 *   klgain [-h | --help] [-g <Gain>] [-m] <wav1> <wav2> <wav3> ...
 * 
 *   -g <Gain> is the gain (multiplication) factor.
 * 
 *   If -m is specified, then the waveforms are scaled to 75% of the
 *   maximum range.  See below for choice of 75%.
 * 
 *  <wav1> <wav2> are the files containing the original waveform (.wav not
 *  needed)
 * 
 *  If both -m and -g are specified, then the -g option is ignored.
 * 
 *  The (scaled) output wav files will have "_scaled" appended to them.
 *  So, if you type 'klgain good', the output file is 'good_scaled.wav'.
 * 
 *  75% of maximum value was chosen to match the amplitude of the waveform
 *  on the VAXstations.  When recording on the VAXstations, if the
 *  amplitude of the waveform is greater than 7500 (75% of maximum value
 *  on VAX) then the waveform gets clipped Therefore, the waveform is only
 *  scaled to 75% tto be consistent across both the VAXes and the UNIX
 *  platforms.
 * 
 * KKG 5/18/98, v.1.0
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>

#include "wavio.h"
#include "wavdata.h"
#include "getargs.h"

#define MAX_RANGE .75     /* See note at top */

#define DEFAULT_GAIN 999  /* Default gain factor, used for checking arg */
#define MAX_SHORT 32767   /* Maximum value that a short can take on */

int main(int argc, char **argv);
void usage(char *name);

int main(int argc, char **argv)
{
  char iname[256], oname[270], progName[256], *str;
  float samplingRate;
  int numSamples;
  int IsSwap, swap;
  short *data;
  int i, j;

  int setMax = 0;
  float max, min, absmax, scale, maxscale;
  float gain;

  /* 
   * Check for help or ?, and if right number of args
   */
   
   strcpy(progName, argv[0]);

   if (IsArg("-h", &argc, argv) || IsArg("-H", &argc, argv) || 
       IsArg("-help", &argc, argv) || IsArg("--help", &argc, argv) ||
       IsArg("-?", &argc, argv))
     usage(progName);

   /*
    * Get command line parameters for gain or for setting to max value.
    * Check if either one of command line args set.
    */

   if (! IsArg("-m", &argc, argv)) { /* If -m not specified */
     gain = checkFloatArg("-g", &argc, argv, DEFAULT_GAIN);
     if (gain == DEFAULT_GAIN) {
       fprintf(stderr, "\nPlease specify either -g <Gain> or -m option.\n");
       usage(progName);
     }
   }
   else setMax = 1;

   if ((str = RemainingArgs(argc, argv)) != NULL) {
     fprintf(stderr, "Unknown or duplicate argument: %s\n", str);
     usage(progName);
   }

   if (argc < 2) usage(progName);

   for (j = 1; j < argc; j++) {

     /* 
      * Get input and output file names.  The output file name has 
      * "_scaled" appended to the basename
      */

     strcpy(iname, argv[j]);
     fprintf(stdout, "\nProcessing %s...\n", iname);
  
     if (strcmp(&iname[strlen(iname) - 3],"wav") == 0)
       iname[strlen(iname) - 4]  = '\0';
     sprintf(oname, "%s_scaled", iname);

     /* Open input wave, and get header info, and data */

     data = getWavWaveform(iname, &samplingRate, &numSamples, swap);

     if (data == NULL) {
       fprintf(stderr, "Error reading input waveform.\n");
       continue;
     }

     /*
      * Find min and max value, and the largest of min and max
      */

     max = (float) abs(findMaxValue(data, numSamples));
     min = (float) abs(findMinValue(data, numSamples));
     absmax = ((max > min) ? max : min);
     maxscale = (MAX_SHORT * MAX_RANGE)/absmax;  /* Max. scale possible */

     /*
      * Scale waveform based on absolute maximum if user specified -max;
      * else scale based on what user specified, checking to make sure
      */
 
     if (setMax) {
       scale = maxscale;
       fprintf(stdout, "Scaling to max. (gain = %.3f).\n", maxscale);
     }
     else if (gain != 1.0) { /* User specified */

       if (fabs(gain) > maxscale) { /* Check if scale within range */
	 scale = maxscale;
	 fprintf(stdout, "Scaling by %.3f will lead to clipping. ", gain);
	 fprintf(stdout, "Scaling to max (gain = %.3f).\n", 
		 maxscale);
       }
       else {
	 scale = gain;
	 fprintf(stdout, "Scaling by %.3f.\n", scale);
       }
     }

     /*
      * Scale data
      */

     for (i = 0; i < numSamples; i++) *(data + i) *= scale;

     /*
      * Write out scaled waveform
      */

     if (putWavWaveform(oname, data, samplingRate, numSamples, swap)) {
       fprintf(stderr, "Error writing scaled waveform.\n");
       if (data != NULL) free(data);
       continue;
     }
     fprintf(stdout, "Scaled waveform written to %s.\n", oname);

     if (data != NULL) free(data);
   }
}

void usage(char *name)
{
  fprintf(stderr, "\nUsage: %s [-h | --help] [-g <Gain>] [-m] <wav1> <wav2> ...\n", 
	  name);
  fprintf(stderr, "\n\t-g <Gain> is the gain (multiplication) factor.\n");

  fprintf(stderr, "\n\tIf -m is specified, then the waveforms are scaled to %.0f%s of\n", (int) 100*MAX_RANGE, "%");
  fprintf(stderr, "\tthe maximum range.  See below for choice of %.0f%s.\n", 100*MAX_RANGE, "%");

  fprintf(stderr, "\n\t<wav1> <wav2> are the files containing the original waveform\n\t(.wav not needed).\n");

  fprintf(stderr, "\n\tIf both -m and -g are specified, then the -g option is ignored.\n");

  fprintf(stderr, "\n\tThe (scaled) output wav files will have \"_scaled\" appended to them.\n");
  fprintf(stderr, "\tIf you type \'%s good\', the output file is \'good_scaled.wav\'.\n\n", name);

  fprintf(stderr, "\t%0.f%s of maximum value was chosen to match the amplitude of\n\t", 100*MAX_RANGE, "%");
  fprintf(stderr, "the waveform on the VAXstations.  When recording on the\n\t");
  fprintf(stderr, "VAXstations, if the amplitude of the waveform is greater\n\t");
  fprintf(stderr, "than %.0f (%.0f%s of maximum value on VAX) then the waveform\n\tgets clipped.  ", 10000*MAX_RANGE, 100*MAX_RANGE, "%");
  fprintf(stderr, "Therefore, the waveform is only scaled to %.0f%s\n\tto be consistent across ", 100*MAX_RANGE, "%");
  fprintf(stderr, "both the VAX and the UNIX platforms.\n\n");
  exit(1);
}
