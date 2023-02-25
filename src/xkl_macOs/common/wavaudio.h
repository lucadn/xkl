/* $Log: wavaudio.h,v $
 * Revision 1.4  2001-09-14 16:01:42-04  tiede
 * play() given immediate return option (for klplay)
 *
 * Revision 1.3  1998/06/06 05:18:42  krishna
 * Added RCS header.
 * */

#if defined(linux)
#include <linux/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#define DSP_DEVICE "/dev/dsp"
#endif

#define RECORD  0
#define PLAY    1

#define MONO 0
#define STEREO 1

#define NUM_ZEROS 20  /* number of zeros written to prevent clicks */

int play(short *wave, float *samplingRate, int numSamples, int swap, int immed);
short *record(float *samplingRate, float duration, int *numSamples, int swap);
void recordToFile(float *samplingRate, float duration, char *fileName);
char PlayLoop(void *);
void KillPlay();

