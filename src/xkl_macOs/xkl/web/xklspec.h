/* $Log: xklspec.h,v $
 * Revision 1.3  1998/06/12 03:52:17  krishna
 * Removed references to pcode and srsolv, which are now in
 * ../common/spectrum.h.
 *
 * Revision 1.2  1998/06/04 23:24:51  krishna
 * Added RCS header.
 * */

#ifndef XKLSPEC_H
#define XKLSPEC_H

#include <math.h>

/* DFT_SPEC is different in makefbank & quickspec*/

#define DFT_SPEC        0
#define AVG_SPEC        4
#define TWOPI           6.283185307

#define DFT_MAG		0
#define CRIT_BAND	1
#define SPECTROGRAM	2
#define LPC_SPECT	3
#define AVG_SPECT       4
#define DFT_MAG_NO_DB	4
#define SPECTO_NO_DFT	5
#define MAX_LPC_COEFS	20

/*
 * for getf0 
 */

#define UP	        2
#define DOWN	        1
#define MAXNPEAKS	50
static int fpeaks[MAXNPEAKS];

/*  
 * Minimum value allowed as input to log conversion
 * to prevent quantization noise problems 
 */

#define SUMMIN  .001

/*
 * Function prototypes
 */

int mgtodb();
void getavg();
void dft();               /* Dennis' DFT*/
void printfilters();
void makefbank();
void wave_pixmap();
void spectrum_pixmap();
void draw_spectrum();
void new_spectrum();
void stepleft();
void stepright();
void zoomin();
void zoomout();
void printspectrum();
void fourspectra();
void printwav();
void savdft();
void dofltr();
void copysav();

#endif  /* XKLSPEC_H */





