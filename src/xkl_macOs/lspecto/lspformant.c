/* $Log: lspformant.c,v $
 * Revision 1.2  1998/06/06 21:05:16  krishna
 * Added RCS header.
 * */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		            G E T F O R M . C		D. Klatt	 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*		On VAX, located in directory [klatt.klspec]		*/


/* EDIT HISTORY:
 * 000001 31-Jan-84 DK	Initial creation
 * 000002 24-Mar-84 DK	Fix so hidden formant rejected if too close to peak
 * 000003 23-May-84 DK	Improved peak locating interpolation scheme
 * 000004 17-Apr-86 DK	Improve accuracy of freq estimate for hidden peaks
 */

#include "lspformant.h"

        extern int fltr[];
	int dfltr[257];			/* Spectral slope */
        extern int nfr[],nbw[];
	extern int sizwin;				    /* Set in KLSPEC */
	extern int noutchan;				    /* Set in KLSPEC */
	extern int srate[],nwave;	/* Sampling rate,      set in KLSPEC */

#define MAX_INDEX	7
	int nforfreq;			/* number of formant freqs found   */
	int forfreq[8],foramp[8];	/* nchan and ampl of max in fltr[] */
	int valleyfreq[8],valleyamp[8];	/* nchan and ampl of min in fltr[] */

	int nmax_in_d;			/* number of maxima in deriv       */
	int ncdmax[8],ampdmax[8];	/* nchan and ampl of max in deriv  */
	int ncdmin[8],ampdmin[8];	/* nchan and ampl of min in deriv  */

	int hftot;			/* number of hidden formants found */
	int hiddenf[8],hiddena[8];	/* freq and ampl of hidden formant */

	int down = 1;
	int up = 2;

void getform() {

	int nch,lasfltr,lasderiv,direction,da,da1,da2,nhf;
	long nsum;
	char garbage;
	int nchb,nchp,ff,df;


/*	  Pass 1: Compute spectral slope */
	    for (nch=2; nch<noutchan-2; nch++) {
/*	      Approximate slope by function involving 5 consecutive channels */
		dfltr[nch] = fltr[nch+2]+fltr[nch+1] -fltr[nch-1] -fltr[nch-2];
	    }
/*	  Pass 2: Find slope max, slope min; half-way between is a formant */
/*	   If max and min of same sign, this is a potential hidden formant */
	    direction = down;
	    nmax_in_d = 0;
	    lasderiv = dfltr[2];
	    hftot = 0;
	    for (nch=3; nch<noutchan-2; nch++) {
	        if (dfltr[nch] > lasderiv) {
		    if ( (direction == down) && (nmax_in_d > 0) ) {
/*		      Found max and next min in deriv, see if hidden formant */
			if ((hiddenf[hftot] = testhidden()) > 0) {
			    hiddena[hftot] =
			     fltr[(hiddenf[hftot]*256)/srate[nwave]];
			    if (hftot < MAX_INDEX)   hftot++;
			}
		    }
		    direction = up;
	        }
	        else if (dfltr[nch] < lasderiv) {
		    if ((nmax_in_d > 0) && (direction == down)) {
/*		      On way down to valley */
			ampdmin[nmax_in_d-1] = dfltr[nch];
			ncdmin[nmax_in_d-1] = nch;
		    }
		    if ((direction == up) && (nmax_in_d < 7)) {
			ampdmin[nmax_in_d] = dfltr[nch];
			ncdmin[nmax_in_d] = nch;
/*		      Peak in deriv found, compute chan # and value of deriv */
			ampdmax[nmax_in_d] = lasderiv;
			ncdmax[nmax_in_d] = nch-1;
			if (nmax_in_d < MAX_INDEX)   nmax_in_d++;
		    }
		    direction = down;
	        }
	        lasderiv = dfltr[nch];
	    }
		
/*	  Pass 3: Find actual spectral peaks, fold hidden peaks into array */
	    direction = down;
	    nforfreq = 0;
	    nhf = 0;
	    lasfltr = fltr[0];
	    for (nch=1; nch<noutchan; nch++) {
	        if (fltr[nch] > lasfltr) {
		    direction = up;
	        }
	        else if (fltr[nch] < lasfltr) {
		    if ((nforfreq > 0) && (direction == down)) {
/*		      On way down to valley */
			valleyamp[nforfreq-1] = fltr[nch];
		    }
		    if ((direction == up) && (nforfreq < 6)) {
			valleyamp[nforfreq] = fltr[nch];

/*		    Peak found, compute frequency */
/*		      Step 1: Work back to filter defining beginning of peak */
			for (nchb=nch-2; nchb>0; nchb--) {
			    if (fltr[nchb] < fltr[nch-1])    break;
			}
/*		      Step 2: Compute nearest filter of peak */
			nchp = (nchb + nch) >> 1;
			ff = nfr[nchp];
/*		      Step 3: Add half a filter if plateau has even # filters */
			if (nchp < ((nchb + nch + 1) >> 1)) {
			    ff = (nfr[nchp] + nfr[nchp+1]) >> 1;
			}
/*		      Step 4: Compute interpolation factor */
		        da1 = fltr[nchp] - fltr[nchb];
		        da2 = fltr[nchp] - fltr[nch];
		        da = da1 + da1;
			if (da2 > da1)    da = da2 + da2;
			nsum = ((da1-da2) * (nfr[nch-1] - nfr[nch-2]));
			df = nsum / da;
			ff += df;

			while ((nhf < hftot) && (hiddenf[nhf] < ff)
			&& (nforfreq < MAX_INDEX)) {
/*			  Hidden formant should be inserted here */
			    foramp[nforfreq] = hiddena[nhf];
			    forfreq[nforfreq++] = -hiddenf[nhf++];
			}

			foramp[nforfreq] = lasfltr;
			forfreq[nforfreq] = ff;
			if (nforfreq < MAX_INDEX)    nforfreq++;
		    }
		    direction = down;
	        }
	        lasfltr = fltr[nch];
	    }
	    while ((nhf < hftot) && (nforfreq < MAX_INDEX)) {
/*	      Hidden formant should be inserted here */
		foramp[nforfreq] = hiddena[nhf];
		forfreq[nforfreq++] = -hiddenf[nhf++];	/* minus indic. hidd */
	    }
	    zap_insig_peak();
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		        T E S T H I D D E N				 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define NCH_OF_PEAK_IN_DERIV	ncdmax[nmax_in_d-1]
#define AMP_OF_PEAK_IN_DERIV	ampdmax[nmax_in_d-1]
#define NCH_OF_VALLEY_IN_DERIV	ncdmin[nmax_in_d-1]
#define AMP_OF_VALLEY_IN_DERIV	ampdmin[nmax_in_d-1]

/* Found loc max in deriv followed by loc min in deriv, see if hidden formant */

int testhidden() {

	int nchpeak;	/* channel number of potential hidden peak */
	int amphidpeak;	/* amplitude of hidden peak = (max+min)/2 in deriv */
	int delamp,delslope,nchvar,f,a,denom,foffset;

/*    A real peak is where derivative passes down through zero. */
/*    A hidden peak is where local max and succeeding local min of derivative */
/*    both are positive, or both are negative */
	if ((AMP_OF_PEAK_IN_DERIV <= 0) 
	 || (AMP_OF_VALLEY_IN_DERIV >= 0)) {

/*	   OUT printf("ap=%d av=%d fp=%d fv=%d\n",
	     AMP_OF_PEAK_IN_DERIV, AMP_OF_VALLEY_IN_DERIV, 
	     NCH_OF_PEAK_IN_DERIV, NCH_OF_VALLEY_IN_DERIV); END OUT */

/*	  See if diff in slope not simply noise */
	    delslope = AMP_OF_PEAK_IN_DERIV - AMP_OF_VALLEY_IN_DERIV;
	    if (delslope < 25) {
		return(-1);
	    }

/*	  Tentative channel hidden peak = mean of peak & valley locs in deriv */
	    nchpeak = (NCH_OF_PEAK_IN_DERIV + NCH_OF_VALLEY_IN_DERIV) >> 1;

/*	  Find amplitude of nearest formant peak (local spectral max) */
	    if (AMP_OF_PEAK_IN_DERIV < 0) {
/*	      Nearest peak is previous peak, find it */
		nchvar = nchpeak;
		while ( (fltr[nchvar-1] >= fltr[nchvar]) && (nchvar > 0) ) {
		    nchvar--;
		}
		delamp = fltr[nchvar] - fltr[nchpeak];
		foffset = -80;
	    }
	    else {
/*	      Nearest peak is next peak, find it */
		nchvar = nchpeak;
		while ( (fltr[nchvar+1] >= fltr[nchvar]) && (nchvar < 127) ) {
		    nchvar++;
		}
		delamp = fltr[nchvar] - fltr[nchpeak];
		foffset = 120;	/* hidden formant is actually between min
				   and next max */
	    }

/*	  See if amp of hidden formant not too weak relative to near peak */
	    if (delamp < 80) {
/*	      Exact location of hidden peak is halfway from max to min in deriv */
		amphidpeak = (AMP_OF_PEAK_IN_DERIV+AMP_OF_VALLEY_IN_DERIV)>>1;
		nchpeak = NCH_OF_PEAK_IN_DERIV;
		if (dfltr[nchpeak] > amphidpeak) {
		    while ( ((dfltr[nchpeak] - amphidpeak) >= 0)
		     && (nchpeak <= NCH_OF_VALLEY_IN_DERIV) ) {
			nchpeak++;
		    }
		    nchpeak--;
		}
		a = dfltr[nchpeak] - amphidpeak;
		denom = dfltr[nchpeak] - dfltr[nchpeak+1];
		if (denom > 0) {
		    f = nfr[nchpeak]
			 + ((a*(nfr[nchpeak+1]-nfr[nchpeak]))/denom);
		}
		else {
		    f = nfr[nchpeak];
		}

/* OUT		printf("dfltr[%d]=%d, amphidpeak=%d, dfltr[nchpeak+1]=%d\n",
		 nchpeak, dfltr[nchpeak], amphidpeak, dfltr[nchpeak+1]);
		printf("f=%d = %d + (%d * (%d - %d)) / %d)\n",
		 f, nfr[nchpeak], a, nfr[nchpeak+1], nfr[nchpeak], denom);
		printf("\tT=%3d, ds=%3d, da=%3d, hidden formant at f=%d\n",
		 time_in_ms, delslope, delamp, f);
   END OUT */
		return(f+foffset);
	    }
	}
	return(-1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		        Z A P - I N S I G - P E A K			 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*	  Eliminate insignificant peaks (last one always significant) */
/*	  Must be down 6 dB from peaks on either side		      */
/*	  And valley on either side must be less than 2 dB	      */

/*	  Also eliminate weaker of two formant peaks within 210 Hz of */
/*	  each other, unless not enough formants in F1-F3 range	      */

void zap_insig_peak() {

	int nf,n,xxx,fcur,fnext;

	for (nf=0; nf<nforfreq-1; nf++) {

	    if ((nf == 0) || (foramp[nf-1] > foramp[nf]+60)) {
		if (foramp[nf+1] > foramp[nf]+60) {
		    if ((nf == 0) || (valleyamp[nf-1] > foramp[nf]-20)) {
			if (valleyamp[nf] > foramp[nf]-20) {
/*			    printf("\n\tT=%d, zap weak peak at P%d=%d",
			     time_in_ms, nf+1, forfreq[nf]); */
			    for (n=nf; n<nforfreq; n++) {
				forfreq[n] = forfreq[n+1];
				foramp[n] = foramp[n+1];
			    }
			    nforfreq--;
			}
		    }
		}
	    }

/*	  Zap weaker of 2 close peaks (hidden peak is alway weaker) */
	    if (nf < nforfreq-1) {
		fcur = forfreq[nf];
		if (fcur < 0)    fcur = -fcur;

		fnext = forfreq[nf+1];
		if (fnext < 0)    fnext = -fnext;

		if ((fnext - fcur) < 210) {

		    if (foramp[nf] > foramp[nf+1])    xxx = 1;
		    else    xxx = 0;

		    for (n=nf+xxx; n<nforfreq-1; n++) {
			forfreq[n] = forfreq[n+1];
			foramp[n] = foramp[n+1];
		    }
		    nforfreq--;
		    nf--;
		}
	    }

	}
}

