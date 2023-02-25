/* $Log: lspspec.c,v $
 * Revision 1.2  1998/06/06 21:05:16  krishna
 * Added RCS header.
 * */


#include <math.h>
#include <stdio.h>
#include "spectrum.h"
#include "lspspec.h"
#include "lspfbank.h"
#include "lspdft.h"
#include "lspgetf0.h"
#include "lspformant.h"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		L G E T S P E C . C			D. H. Klatt	 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*		On VAX, located in directory [klatt.specto]		*/

/* Old version of getspec.c, used by LSPECTO.C				*/

/* EDIT HISTORY:
 * 0001 21-Jul-87 DK	Initial creation from oldgetspe.c 9-Sep-86
 * 0002  1-Jan-88 DK	Fix bug in mgtodb(), extend range
 * 0003  2-Mar-88 DK	Move all postscript stuff to postgrid.c
 */


/* 1. IF "type_spec" IS EQUAL TO 0, COMPUTE DFT MAGNITUDE SPECTRUM */
/*    Input is sizwin-point integer waveform chunk in iwave[] */
/*      Do first difference preemphasis			      */
/*      Multiply by a Hamming window			      */
/*      Compute 512-point dft, put mag in array dft[256]      */
/*        where output is dB-times-10			      */
/*        and where dft[0] contains dc componet		      */
/*	and estimate fundamental frequency		      */

/* 3. IF "type_spec" IS EQUAL TO 2, COMPUTE SPECTROGRAM SPECTRUM   */
/*    Input is sizwin-point integer waveform chunk in iwave[] */
/*      Do first difference preemphasis			      */
/*      Multiply by a Hamming window			      */
/*      Compute 256-point dft, put mag in array dft[128]      */
/*      Perform weighted sum of dft samples to		      */
/*      place nchan critical-band spectrum in fltr[nchan]     */
/*        where output is dB-times-10			      */
/*        and total energy is in fltr[nchan]		      */
/*	and estimate formant frequency positions	      */

/* 4. IF "type_spec" IS EQUAL TO 3, COMPUTE LPC SPECTRUM      */
/*    Input is sizwin-point integer waveform chunk in iwave[] */
/*      Do first difference preemphasis			      */
/*      Multiply by a Hamming window			      */
/*	Call lpc() to get coefficents			      */
/*	Pad coefficient array with zeros		      */
/*      Compute 256-point dft, put mag in array dft[128]      */
/*	and estimate formant frequency positions	      */
/*		NOTE THAT NFR[] MUST BE SET FOR THIS TO WORK  */

/*      nfr[nchan] contains filter center frequencies, in Hz  */
/*      nbw[nchan] contains filter bandwidths, in Hz	      */


#define DFT_MAG		0
#define SPECTROGRAM	2
#define LPC_SPECT	3
#define MAX_LPC_COEFS	20

/*    Minimum value allowed as input to log conversion */
/*     to prevent quantization noise problems */
#define SUMMIN  .001

        float hamm[513]; /*513*/
        float dftmag[1028]; /*514*/
        int fltr[258];   /*258*/

	extern int firstdifsw;		/* Part of config def, set in KLSPEC */
	extern int sizwin;				    /* Set in KLSPEC */
	extern int noutchan;				    /* Set in KLSPEC */
	extern int srate[],nwave;	/* Sampling rate,      set in KLSPEC */
	extern int time_in_ms;		/* Time at window cent set in KLSPEC */

/*    Arrays from MAKEFBANK.C */
        extern int nfr[],nbw[];
	extern int pweight[];
        extern float cbskrt[];
        extern int nchan;
        extern int nstart[],ntot[];
	static int logoffset;
	static int sizfft;		/* Set here depends on arg type_spec */
	extern int halfwin;		/* If=1, use only 1st half of window */
	extern int nforfreq,forfreq[];	/* from GETFORM.C		     */
	double denergy;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		             G E T S P E C				 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void getspec(short *iwave,int type_spec) {

/*    Arrays needed for lpc analysis */
	extern int nlpcoefs;	/* Number of linear prediction coeficients */
	float scratch[(MAX_LPC_COEFS * (MAX_LPC_COEFS+3) ) >> 1];/* Use dftmag*/
	double error, energy;
	int ier,iwlast;

        static int initsw;

        int temp,pdft,pdftend,nch,pw,db,dbinc;
        long nsum;
        float sum;


/*    Initialize (make Hamming window) */

        if (initsw != sizwin) {
            initsw = sizwin;
            mkwnd(hamm,sizwin,sizfft);
	}

/*    Select fft size */

	if (type_spec == SPECTROGRAM) {
	    sizfft = 256;	/* Fold waveform if window > 256 */
	    makefbank(SPECTROGRAM);
	}

	else if (type_spec == DFT_MAG) {
	    sizfft = 512;	/* Assume window > 256 in general */
	}

	else if (type_spec == LPC_SPECT) {
	    makefbank(LPC_SPECT);	/* Actually just set nfr[] */
	    sizfft = 256;	/* Truncate waveform if window > 256 */
/*	  Multiply waveform times window, do first difference, use dftmag[] */
	    iwlast = 0;
	    for (nch=0; nch<sizfft; nch++) {
		if (firstdifsw == 1) {
		    dftmag[nch] = hamm[nch] * (float) (iwave[nch] - iwlast);
		    iwlast = (iwave[nch]*28)>>5;
		}
		else {
		    dftmag[nch] = hamm[nch] * (float) iwave[nch];
		}
	    }
/*	  Call Fortran lpc analysis subroutine, coefs in dftmag[256]... */
	    pcode(dftmag,sizwin,nlpcoefs,&dftmag[256],scratch,&error,&energy,
	     &ier);
/*	  Move coefs down 1, make negative */
	    for (nch=nlpcoefs-1; nch>=0; nch--) {
		dftmag[nch+257] = -dftmag[nch+256];
	    }
	    dftmag[256] = 1.0;
	    if (ier != 0) {
		printf("Loss of significance, ier=%d\n", ier);
	    }
/*	  Pad acoef with zeros for dft */
	    for (nch=nlpcoefs+257; nch<=512; nch++) {
		dftmag[nch] = 0.;
	    }
/*	  Call dft routine with firstdifsw=-1 to indicate lpc */
	    dft(iwave,&dftmag[256],dftmag,sizwin,sizfft,-1);
	}


/*    Compute magnitude spectrum */
	if (type_spec != LPC_SPECT) {
/*	  Hack to make first diff coef be .85 for all spectral first diffs */
	    dft(iwave,hamm,dftmag,sizwin,sizfft,85*firstdifsw);
	}

/*  DFT now done in all cases, next create filters */

/*    If DFT magnitude spectrum, just convert to dB */
	if (type_spec == DFT_MAG) {
/*	  Consider each dft sample as an output filter */
	    for (nch=0; nch<noutchan; nch++) {
		nsum = dftmag[nch] * 20000.;	/* Scaled so smaller than cb */
		fltr[nch] = mgtodb(nsum);
	    }
	}

/*    If LPC: invert, multiply by energy, and convert to dB */
	else if (type_spec == LPC_SPECT) {
/*	  Multiply coefs by square root of energy */
	    denergy = energy * error;
	    if (denergy > 0.0) {
		logoffset = -1450 + 100. * log10(denergy);
		fltr[noutchan] = 1130 + logoffset;
	    }
	    else logoffset = -1450;
/*	  Consider each dft sample as an output filter */
	    for (nch=0; nch<noutchan; nch++) {
		denergy = dftmag[nch];
		if (denergy > 0.) {
		    fltr[nch] = logoffset - 100. * log10(denergy);
		}
		else fltr[nch] = 0;
		if (fltr[nch] < 0)    fltr[nch] = 0;
	    }
	}

/*    Sum dft samples to create Critical-band or Spectrogram filter outputs */
	else {

            pw = 0;
	    for (nch=0; nch<noutchan; nch++) {
                pdft = nstart[nch];
                pdftend = pdft + ntot[nch];
                sum = SUMMIN;
/*            Compute weighted sum of relevent dft samples */
                while (pdft < pdftend) {
                    sum += cbskrt[pweight[pw++]] * dftmag[pdft++];
                }

/*            Convert to long integer */
                sum = sum * 18000.;	/* Scaled so bigger than dft */
		nsum = sum;
		fltr[nch] = mgtodb(nsum);
/*	      Look for maximum filter output in this frame */
                if (fltr[nch] > fltr[noutchan+1]) {
		    fltr[noutchan+1] = fltr[nch];
		}
	    }
	}

	if (type_spec == DFT_MAG) {
/*	  Estimate f0, put in formant freq array for display */
	    forfreq[0] = getf0(fltr,sizfft,srate[nwave]);
	    nforfreq = 1;
	}

	else {
/*        ESTIMATE FORMANT FREQUENCY POSITIONS, put in forfreq[nforfreq] */
	    getform();
	}


/*    Compute overall intensity and place in fltr[noutchan] */
	if (type_spec != LPC_SPECT) {
	    sum = SUMMIN;
	    for (pdft=0; pdft < sizfft>>1; pdft++) {
        	sum = sum + dftmag[pdft];
	    }
	    nsum = sum * 10000.;	/* Scale factor so synth av=60 about right */
	    fltr[noutchan] = mgtodb(nsum);
	}
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		             M G T O D B				 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Convert long integer to db */

    int dbtable[65] = {
         0,  1,  2,  3,  5,  6,  7,  9, 10, 11,
        12, 13, 14, 16, 17, 18, 19, 20, 21, 22,
        23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 37, 38, 39, 40, 41,
        42, 43, 43, 44, 45, 46, 47, 47, 48, 49,
        50, 50, 51, 52, 53, 53, 54, 55, 56, 56,
        57, 58, 58, 59, 60};

int mgtodb(long nsum) 
{

	int db,temp,dbinc;

/*    Convert nsum to db (decibels-times-10) */
/*    If nsum is 03777777777L, db=660 */
/*    If nsum is halved, db is reduced by 60 */
        db = 1440;
        if (nsum <= 0L) {
	  if (nsum < 0L) {
	    printf("Error in mgtodb of LGETSPE,");
	    printf(" nsum=%d is negative, set to 0\n", (int) nsum);
	  }
	  return(0);
	}
	if (nsum > 03777777777L) {
	    printf ("Error in mgtodb of LGETSPE, nsum=%d too big, truncate\n",
	     (int) nsum);
	    nsum = 03777777777L;
        }
        while (nsum < 03777777777L) {
            nsum = nsum + nsum;
            db = db - 60;
        }
        temp = (nsum >> 23) - 077;
	if (temp > 64) {
	    temp = 64;
	    printf ("Error in mgtodb of LGETSPE, temp>64, truncate\n");
	}
        dbinc = dbtable[temp];
        db = db + dbinc;
	db = db >> 1;		    /* Routine expects mag, we have mag sq */
	if (db < 0) db = 0;
	return (db);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		              M K W N D 				 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*    Make window */

#define TWOPI   6.283185307


void mkwnd(float *hamm,int wsize,int power2) {

	float xi, incr;
	int i,limit;

/*	printf(
	 "\tMake Hamming window; width at base = %d points, pad with %d zeros\n",
	  wsize, power2-wsize); */
	xi = 0.;
	incr = TWOPI/(wsize);
	limit = wsize;
	if (halfwin == 1)    limit = limit >> 1;

/*    Compute non-raised cosine */
	for(i=0;i<limit;i++) {
		hamm[i] = (1. -  cos(xi));
		xi += incr;
	}

/*    Pad with zeros if window duration less than power2 points */
	while (i < power2) {
	    hamm[i++] = 0.;
	}
}



/*  Program used to generate entries for dbtable[] *
C       MAKEDB.FOR      D.H. KLATT
C
        DIMENSION NDB(65)
        DO 100 N=1,65
        ARG=1.+(FLOAT(N-1)/64.)
        NDB(N)=IFIX(200.*ALOG10(ARG))
        WRITE(5,90) N,NDB(N)
90      FORMAT (' ',I2,'.  ',I5)
100     CONTINUE
        STOP
        END
  */

