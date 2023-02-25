/* $Log: lspgetf0.c,v $
 * Revision 1.2  1998/06/06 21:05:16  krishna
 * Added RCS header.
 * */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		            G E T F 0 . C		D.H. Klatt	 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*		On VAX, located in directory [klatt.klspec]		*/


/* EDIT HISTORY:
 * 000001 31-Jan-83 DK	Initial creation
 * 000002  6-Mar-84 DK	Fix serious bug in sorting of acceptable peaks
 * 000003  9-Mar-84 DK	Use comb filter to determine harmonic number of peaks
 * 000004 20-Jul-84 DK	Restrict range to 1500 Hz, select prominent harms, fix
 * 000005 24-Apr-86 DK	Add test to keep 'npeaks' within array size
 * 000006 14-Mar-88 DK	Return ten times f0 for better accuracy
 */
#include "lspgetf0.h"
#include <stdio.h>
#define UP	2
#define DOWN	1
#define MAXNPEAKS	50
static int fpeaks[MAXNPEAKS];

/*  Get estimate of fundamental frequency from dft mag spectrum */

int getf0(dftmag,npts,samrat) int dftmag[],npts,samrat; {

	int direction,npeaks,lasfltr,n;
	int dfpeaks[MAXNPEAKS],apeaks[MAXNPEAKS];
	int alastpeak,flastpeak,np,np1,mp,kp,dfp;
	int dfsum,dfmin,dfmax,dfn,nmax,nmaxa,ampmax;
	int da,da1,da2,df,npaccept;
	int dfsummax,dfnmax,dfnloc;
	char char1;	/* Used for debug only */

	direction = DOWN;
	npeaks = 0;
	lasfltr = dftmag[0];
/*    Look for local maxima only up to 1.5 kHz (was 3 kHz) */
	if (samrat > 0) {
	    nmax = (npts * 1500) / samrat;
/*	  Also find biggest amplitude in range from 0 to 900 Hz */
	    nmaxa = (npts * 900) / samrat;
	}
	else printf("In getf0(): somebody walked over samrat, set to zero");
	ampmax = 0;

/*    Begin search for set of local max freqs and ampls */
	for (n=1; n<nmax; n++) {
	    if (n < nmaxa) {
		if (dftmag[n] > ampmax)    ampmax = dftmag[n];
	    }
	    if (dftmag[n] > lasfltr) {
		direction = UP;
	    }
	    else if (dftmag[n] < lasfltr) {
		if ((direction == UP) && (npeaks < 49)) {
/*		  Interpolate the frequency of a peak by looking at
		  rel amp of previous and next filter outputs */
			da1 = dftmag[n-1] - dftmag[n-2];
			da2 = dftmag[n-1] - dftmag[n];
			if (da1 > da2) {
			    da = da1;
			}
			else {
			    da = da2;
			}
			if (da == 0) {
			    da = 1;
			}
			df = da1 - da2;
			df = (df * (samrat>>1)) / da;
			fpeaks[npeaks] = (((n-1) * samrat) + df) / npts;
			apeaks[npeaks] = lasfltr;
			if (npeaks < MAXNPEAKS-1)    npeaks++;
		}
		direction = DOWN;
	    }
	    lasfltr = dftmag[n];
	}

/*    Remove weaker of two peaks within minf0 Hz of each other */
#if 0
printf("\nInitial candidate set of %d peaks:\n", npeaks);
for (np=0; np<npeaks; np++) {
    printf("\t %d.   f = %4d   a = %3d\n", np+1, fpeaks[np], apeaks[np]);
}
#endif

	for (np=0; np<npeaks-1; np++) {

	    if (((np == 0) && (fpeaks[0] < 80))
	      || ((np > 0) && ((fpeaks[np] - fpeaks[np-1]) <= 50))) {
		if ((np > 0) && (apeaks[np] > apeaks[np-1])) np--;
		npeaks--;
		for (kp=np; kp<npeaks; kp++) {
		    fpeaks[kp] = fpeaks[kp+1];
		    apeaks[kp] = apeaks[kp+1];
		}
	    }
	}

/*    Remove initial weak peak before a strong one: */
	if ((apeaks[1] - apeaks[0]) > 200) {
	    npeaks--;
	    for (kp=0; kp<npeaks; kp++) {
		fpeaks[kp] = fpeaks[kp+1];
		apeaks[kp] = apeaks[kp+1];
	    }
	}

/*    Remove weak peak between two strong ones I: */
	for (np=1; np<npeaks-1; np++) {
/*	  See if a weak peak that should be ignorred */
	    if (((apeaks[np-1] - apeaks[np]) > 40)
	     && ((apeaks[np+1] - apeaks[np]) > 40)) {
		npeaks--;
		for (kp=np; kp<npeaks; kp++) {
		    fpeaks[kp] = fpeaks[kp+1];
		    apeaks[kp] = apeaks[kp+1];
		}
	    }
	}

/*    Remove weak peak between two strong ones II: */
	for (np=1; np<npeaks-1; np++) {
/*	  See if a weak peak that should be ignorred */
	    if ((apeaks[np-1] - apeaks[np]) > 80) {
		np1 = 1;
		while (((np+np1) < npeaks)
		  && (fpeaks[np+np1] < (fpeaks[np] + 400))) {
		    if ((apeaks[np+np1] - apeaks[np]) > 80) {
			npeaks--;
			for (kp=np; kp<npeaks; kp++) {
			    fpeaks[kp] = fpeaks[kp+1];
			    apeaks[kp] = apeaks[kp+1];
			}
			np--;
			goto brake1;
		    }
		    np1++;
		}
	    }
brake1:
	    np = np;
	}
#if 0
printf("\nFinal candidate set of %d peaks sent to comb filter:\n", npeaks);
for (np=0; np<npeaks; np++) {
    printf("\t %d.   f = %4d   a = %3d\n", np+1, fpeaks[np], apeaks[np]);
}
#endif

	if (npeaks > 1)        dfp = comb_filter(npeaks);
	else if (npeaks > 0)   dfp = fpeaks[0] * 10;
	else                   dfp = 0;
#if 0
printf("\nComb filter says f0 = %d\n", dfp);
printf("\nHit <CR> to continue:");
scanf("%c", &char1);
#endif

/*    Zero this estimate of f0 if little low-freq energy in spect */
	if (ampmax < 200) {
	    dfp = 0;
	}
/*    Or if f0 high and only a few peaks */
/* set npaccept to npeaks d. hall*/
   npaccept = npeaks;
	if ( (npaccept <= 2) && (dfp > 3000) )    dfp = 0;
	return(dfp);
}



int comb_filter(npks) int npks; {

/* Look for an f0 with most harmonics present, restrict */
/* search to range from f0=60 to f0=400 */

#define NBINS	65
    float f0_hypothesis,fmin_freq,fmax_freq;
    static float f0_hyp[NBINS];
    int nharm,bin,min_freq,max_freq,cntr_freq,max_cntr_freq;
    int max_bin,max_count_harm,harm_found;
    int sum_f,sum_nharm,f0_estimate;
    int np,freq_harm,count_harm;

/* Initialization */
    if (f0_hyp[0] == 0) {
	f0_hyp[0] = 400.;
	for (bin=1; bin<NBINS; bin++) {
/*	  Compute center freq of each bin */
	    f0_hyp[bin] = .97 * f0_hyp[bin-1];
	}
    }

    if (npks > 6)    npks = 6;
    max_count_harm = 0;
    for (bin=0; bin<NBINS; bin++) {
	fmin_freq = 0.97 * f0_hyp[bin];
	cntr_freq = f0_hyp[bin];		/* for debugging only */
	fmax_freq = 1.03 * f0_hyp[bin];
	count_harm = 0;
	sum_nharm = 0;
	sum_f = 0;
	for (nharm=1; nharm<=6; nharm++) {
	    min_freq = (int) (fmin_freq * (float) nharm);
	    max_freq = (int) (fmax_freq * (float) nharm);
	    if (min_freq < 2500) {
		harm_found = 0;
		for (np=0; np<npks; np++) {
		    freq_harm = fpeaks[np];
		    if ((freq_harm > min_freq) && (freq_harm < max_freq)) {
			if (harm_found == 0) {
			    harm_found++;
			    count_harm++;
			    sum_nharm += nharm;
			    sum_f += freq_harm;
			}
		    }
		}
	    }
	}
/*	printf("%d ", count_harm); */
	if (count_harm > max_count_harm) {
	    max_count_harm = count_harm;
	    max_bin = bin;
	    max_cntr_freq = cntr_freq;
	    f0_estimate = (sum_f * 10) / sum_nharm;	/* Ten times f0 */
	}
    }
/*  printf("\n	Best comb freq = %d, f0 = %d\n",
     max_cntr_freq, f0_estimate); */

/*    Zero this estimate of f0 if the distribution of f0 estimates */
/*    is rather scattered */
	if ((3*max_count_harm) < (2*npks)) {
/*	    printf("\nZero f0=%3d", f0_estimate); */
	    f0_estimate = 0;
	}
/*	else {
	    printf("\n     f0=%3d", f0_estimate);
	}
	printf("  nh=%d np=", max_count_harm);
	for (np=0; np<npks; np++) {
	    printf("%4d, ", fpeaks[np]);
	} */
	return(f0_estimate);
}
