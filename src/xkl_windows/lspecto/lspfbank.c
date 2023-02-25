/* $Log: lspfbank.c,v $
 * Revision 1.2  1998/06/06 21:05:16  krishna
 * Added RCS header.
 * */


#include <math.h>
/*	M A K E F B A N K . C		D.H. Klatt	*/

/*	Make a critical-band filter bank, or spectrogram filter bank,
 *	 given values for the first seven parameters listed here */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "lspfbank.h"
#define SHORT int	/* Setting to 'short' causes compiler errors */

/*  FROM KLSPEC.C */
/*    Samples per second  of current waveform is in srate[nwave] */
	extern int srate[],nwave;

/*    Number of dft magnitude samples */
	extern int nsdftmag;
/*    Number of output channels (or dft magnitude samples) */
	extern int noutchan;

/*    Center frequency of first filter and second filter */
	extern int ffilt1;
	extern int ffilt2;
/*    All other filter center frequencies, plus the total number
      of filters are determined by ffilt1,ffilt2,srate[nwave] */

/*    Filter bandwidth at 1000 Hz */
/*    and filter bandwidth assymtote as center freq approaches zero */
	extern int bw1000;
	extern int bw0;
/*    Bandwidth of spectrgram filters */
	extern int bwspecto;
/*    Frequency at which critical-band scale goes from linear to log */
	extern int flinlog;


/*    Number of filters in critical-band filter bank (set by this routine
      based on srate[nwave],ffilt1,ffilt2 */
	SHORT nchan;

/*    Filter skirt attenuation vs frequency (down 3 dB in 6.4 dft mag
      samples (Gaussian shape vs linear freq, see Patterson 1976, JASA
      59, 640-654, Fig. 3).  Note skirt truncated when down 22 dB. */
#define SIZCBSKIRT	200
	int sizcbskirt = SIZCBSKIRT;
	float cbskrt[SIZCBSKIRT] = {
	 1.000,  .998,  .995,  .984,  .972,  .952,  .931,  .907,  .883,  .849,
	  .814,  .775,  .735,  .699,  .662,  .618,  .573,  .529,  .484,  .447,
	  .410,  .370,  .329,  .292,  .255,  .238,  .221,  .206,  .191,  .179,
	  .167,  .157,  .146,  .137,  .128,  .120,  .112,  .106,  .099,  .093,
	  .087,  .082,  .077,  .073,  .069,  .065,  .061,  .058,  .055,  .052,
	  .049,  .047,  .044,  .042,  .040,  .038,  .036,  .035,  .033,  .032,
	  .030,  .029,  .027,  .026,  .025,  .024,  .023,  .022,  .021,  .021,
	  .020,  .019,  .018,  .018,  .017,  .017,  .016,  .016,  .015,  .015,
	  .014,  .014,  .013,  .013,  .012,  .012,  .0115, .0113, .011,  .0105,
	  .010,  .0098, .0095, .0093, .009,  .0088, .0085, .0083, .008,  .0077
	};

/*    Replaces all arrays formerly in data file CBDATA.C */
#define CRIT_BAND	1
#define SPECTROGRAM	2
#define LPC_SPECT	3
#define AVG_SPECT	4
#define DFT_SPECT	4	/* Same as AVG_SPECT, i.e. just set nfr[] */
#define NPMAX 32000 /*12000*/
#define NFMAX 257      /*130*/
	SHORT pweight[NPMAX];
	SHORT nbw[NFMAX],nfr[NFMAX+NFMAX],nstart[NFMAX],ntot[NFMAX];


void makefbank(type_of_filters) int type_of_filters; {

    static int oldsamrat,oldnsdftmag,oldbw1000,oldbw0,oldffilt1,oldffilt2;
    static int oldbwspecto,oldflinlog,initskrt;
    static int current_filters;
    int np,i1,i3,nsmin,n1000,nfmax,sizcbskrt;
    float x2,xfmax,fcenter,bwcenter,bwscal,bwscale,xbw1000,xsdftmag,linlogfreq;
    double a,b,c,d,temp1,temp2;


/* Initialization to make filter skirts go down further (to 0.00062) */
    if (initskrt == 0) {
	initskrt++;
	for (np=100; np<SIZCBSKIRT; np++) {
	    cbskrt[np] = 0.975 * cbskrt[np-1];
	}
/*	printf("\nMax down of cbskrt[] is %f  ", cbskrt[SIZCBSKIRT-1]); */
    }

/* See if any defining properties of the filters have changed */
    if ((srate[nwave] != oldsamrat)
     || (nsdftmag != oldnsdftmag)
     || (((bw1000 != oldbw1000)
     ||   (bw0 != oldbw0)
     ||   (flinlog != oldflinlog)
     ||   (ffilt1 != oldffilt1)
     ||   (ffilt2 != oldffilt2)) && (type_of_filters == CRIT_BAND) )
     || ((bwspecto != oldbwspecto) && (type_of_filters == SPECTROGRAM) )
     || (current_filters != type_of_filters)) {

	oldsamrat = srate[nwave];
	oldnsdftmag = nsdftmag;
	oldbw1000 = bw1000;
	oldbw0 = bw0;
	oldflinlog = flinlog;
	oldffilt1 = ffilt1;
	oldffilt2 = ffilt2;
	oldbwspecto = bwspecto;
	current_filters = type_of_filters;

/*    Highest frequency in input dft magnitude array */
	nfmax = srate[nwave] >> 1;
	xfmax = nfmax;
	xsdftmag = nsdftmag;

	if (type_of_filters == CRIT_BAND) {
/*	    printf("\tRebuilding critical-band filter bank ..."); */
	}
	else if (type_of_filters == SPECTROGRAM) {
/*	    printf("\tRebuilding spectrogram-based filter bank ..."); */
	}
	if ((type_of_filters == LPC_SPECT) || (type_of_filters == AVG_SPECT)) {
/*	    printf("\tCompute freq of each dft mag sample (nfr[%d])",
	     noutchan); */
/*	  Reset nfr[] */
	    i3 = noutchan>>1;	/* For rounding up */
	    for (i1=0; i1<noutchan; i1++) {
		nfr[i1] = ((i1 * nfmax) + i3) / noutchan;
	    }
	    return;
	}

	if (type_of_filters == CRIT_BAND) {
/*	  Do not include samples below 80 Hz */
	    nsmin = ((80*nsdftmag)/nfmax) + 1;
/*	  Determine center frequencies of filters, use frequency spacing of
 *	  equal increments in log(1+(f/flinlog)) */
	    linlogfreq = (float) flinlog;
	    temp1 = 1. + (ffilt1/linlogfreq);
	    temp1 = log(temp1);
	    temp2 = 1. + (ffilt2/linlogfreq);
	    temp2 = log(temp2);
	    a = temp2 - temp1;
	    b = (temp1/a) - 1.;
	    c = (xsdftmag*linlogfreq)/xfmax;
	}
	else {
	    nsmin = 1;
	}
	n1000 = (1000*nsdftmag)/nfmax;
	xbw1000 = bw1000;
	xbw1000 = xbw1000/n1000;
	bwscal = (360.*xfmax)/xsdftmag;		/* 360 is a magic number */

	np = 0;
	nchan = 0;
	while (nchan < NFMAX) {

	    if (type_of_filters == SPECTROGRAM) {
/*	      Center frequency in Hz of filter i */
		fcenter = nchan;
/*	      Bandwidth in Hz of filter i */
		bwcenter = bwspecto;
	    }

	    else {
/*	      Center frequency in Hz of filter i */
		d = a*(b+nchan+1);
		fcenter = c * (exp(d) - 1.);

/*	      Bandwidth in Hz of filter i */
		bwcenter = xbw1000*fcenter;
		if (bwcenter < bw0) {
		    bwcenter = bw0;
	        }
	    }
/*	  See if done */
	    if (fcenter >= xsdftmag) {
		break;
	    }

/*	  Center frequency and bandwidth in Hz of filter i */
	    nfr[nchan] = ((fcenter*xfmax) / xsdftmag) + .5;
	    nbw[nchan] = bwcenter;

/*	  Find weights for each filter */
	    nstart[nchan] = 0;
	    ntot[nchan] = 0;
	    bwscale = bwscal/(10.*nbw[nchan]);
	    for (i1=nsmin; i1<nsdftmag; i1++) {
/*	      Let x2 be difference between i1 and filter center frequency */
		x2 = fcenter - i1;
		if (x2 < 0) {
		    x2 = -x2;
		    sizcbskrt = SIZCBSKIRT;	/* Go down 22 dB on low side */
		}
		else sizcbskrt = SIZCBSKIRT>>1;	/* Go down 32 dB on high side */
		i3 = x2 * bwscale;
		if (i3 < sizcbskrt) {
/*		  Remember which was first dft sample to be included */
		    if (nstart[nchan] == 0) {
			nstart[nchan] = i1;
		    }
/*		  Remember pointer to weight for this dft sample */
		    pweight[np++] = i3;
		    if (np >= NPMAX) {
			fprintf(stderr,
			 "\nFATAL ERROR in MAKECBFB.C: 'NPMAX' exceeded\n");
			fprintf(stderr,
			 "\t(while working on filter %d).  ", nchan);
			fprintf(stderr,
			 "\tToo many filters or bw's too wide.\n");
			fprintf(stderr,
			 "\tRedimension NPMAX and recompile if necessary.\n");
			exit(0);
		    }
/*		  Increment counter of number of dft samples in sum */
		    ntot[nchan]++;
		}
	    }
	    nchan++;
	}
	sizcbskrt = SIZCBSKIRT;
/*    NCHAN is now set to be the total number of filter channels */
/*	printf(" done.\n\n"); */
    }
}

/*
 * 
 */

void printfilters() 
{
  int n3, n;

  printf("\t\t\tFILTER CHACTERISTICS:\n\n");
  printf("\t     n\t FREQ\t BW\t     n\t FREQ\t BW\t     n\t FREQ\t BW\n");
  n3 = (nchan+2)/3;
  for (n=0; n<n3; n++) {
    printf("\t    %2d\t %4d\t%4d", n+1, nfr[n], nbw[n]);
    printf("\t    %2d\t %4d\t%4d", n+n3+1, nfr[n+n3], nbw[n+n3]);
    if ((n+n3+n3) < nchan) {
      printf("\t    %2d\t %4d\t%4d\n",
	     n+n3+n3+1, nfr[n+n3+n3], nbw[n+n3+n3]);
    }
  }
  printf("\n");
}
