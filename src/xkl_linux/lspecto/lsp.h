/*
$Log: syn.h,v $
Revision 1.4  1998/07/16 22:59:29  krishna
Removed full pathname for default file, so the user can specify where
the file will be located in the Makefile.

Revision 1.3  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#ifndef LSP_H
#define LSP_H


char date[] = 
{ "LSPECTO   U.T. Version 1.0  Dec-88  D.H. Klatt (U.T. Revision by G.D.Lame)"};

/*IMPORTANT: make sure this directory is correct!!*/
char confile[] = { "lspecto.con" };
char conname[400];

char malef0[] = { "LSPECTO: 30-ms Hamming window every 5 ms, harmonic sieve f0 extraction, no data smoothing."};
char femf0[] = { "LSPECTO: 23-ms Hamming window every 5 ms, harmonic sieve f0 extraction, no data smoothing."};
char frmnts[] = {"            LSPECTO: 25-ms Hamming window every 5 ms, 14-pole linear prediction, peak picking."};
char frmntsf[] = {"            LSPECTO: 20-ms Hamming window every 5 ms, 12-pole linear prediction, peak picking."};
char speco[] = {"                   LSPECTO: 256-pt DFT, smart AGC            6.4-ms Hamming window every 1 ms"};  /*Moved to avoid punch holes*/
char wavplt[] = {"LSPECTO: Waveform plot of every point, expanded to fill available vertical range."};

#include <stdio.h>
#include <math.h>
#include <time.h>

#define DFT_SPEC	 0	/* Spectral rep used for f0 estimation */
#define SPECTO_SPEC	 2	/* Spectral rep used for broadband spectrogram */
#define LPC_SPEC	 3	/* Spectral rep used when formants estimated */

        FILE *fopen(), *wdev;		/* Output .doc or .tem file */
	FILE *idev;			/* Input .con file */
	FILE *tdev;			/* Times-of-analysis file, if exists */
        short *iwave,*p;		/* Pointer to curr pos in waveform */
	int srate[2],nwave;		/* Waveform sampling rate srate[nwave] */
	int nwtot;			/* # samples in waveform */
	static int nwtotsave;
	static int nwstart;
	int amp,ampmax;			/* Used to maximize waveform signal */
	float floatamp,scalefac;	/*  prior to specto analysis */
	float start;			/* for post_amp_plot */

	char frstname[300];		/* First name of waveform and specto */
	char wname[305];		/* Full name of waveform file */
	char outname[305];		/* Full name of output specto file */
	char proutname[305];		/* Full name of output specto file */
	char timename[305];		/* Full name of time voice/fric/sil info */
	char junk[400];			/* General purpuse char buffer */
        char *pstring;		/* Buffer for stuff typed by user */

	extern int nwtotr;	/* # samples in waveform (from READWAVE.C) */
	int samper;	/* Number of 1 usec counts between samples */
	extern int fltr[];	/* Output spectral representation of getspec() */
	extern int nchan;	/* Number of output channels in spectrum */
	extern int forfreq[],nforfreq;	/* Formant freq estimates (or f0) */
	extern int foramp[];

	short *savfor;			/* Pointer into saved formant freqs */
	short nsfcum, nsfcummax;	/* # saved formant data points */
	int time_in_samp;	/* Time in samples coresp to cursor position */
	int time_in_ms; 	/* Time in msec corresp. to cursor position */
	int incr;	/* # of samples in waveform increment (5 or 3.3 ms) */
	static int incrms;	/* # ms (approx) in each waveform increment */
	char listsw;		/* Enable use of LXY22 if arg = -l */
	char prswitch;		/* Print f0 if '0', formants if 'f', both */
				/*  if 's', broadband specto if 'n' */
	short prwavesw;		/* Set to 1 or 2 if waveform to be plotted */
	char normswitch;	/* Normalize wave to const peak amp if 'n' */
	int specttype = SPECTO_SPEC; /* Default is pseudo-spectrogram */
	int sizwin,sizhalfwin,halfwin;	/* Size of time weighting window */
        char command,request,sym1,sym2,phonetype,curphonetype;
        int i,n,nsec,n5,ntick;
	static int tstart;




#define SIZEWINDFT	299
#define SIZEWINSPECTO	 64
#define NUMBERLPCOEFS	 14
int size_win_dft= SIZEWINDFT;	/* Size Hamming window used for DFT f0 anal */
int size_win_specto=SIZEWINSPECTO; /* Size non-zero part of Hamming window */
				/*  for specto analy (does 256-point dft) */
int nlpcoefs = NUMBERLPCOEFS;	/* LPC number of coefs */
int firstdifsw =	 1;	/* Do first diff preemphasis if =1, not if=0 */
int bw1000 = 	       250;	/* CB filter bandwidth at 1000 Hz */
int bw0    =	       210;	/* CB filters minimum bandwidth */
int bwspecto =	       300;	/* Spectrogram filters bandwidth */
int ffilt1 =	       200;	/* CB filter 1 center freq */
int ffilt2 =	       248;	/* CB filter 2 center freq */
int flinlog =	       700;	/* Freq at which cb goes from linear to log */
int noutchan =	       128;	/* Number of pseudo-specto filters */
int nsdftmag = 	       128;	/* Number of dft mag samples for dft anal */
static int nfcascade =   5;	/* Number of formants in 5 kHz for synth .doc */

/*    Variables needed by GETSPEC.C */
        float hamwin[512];	/* Hamming time weighting window array */


	static char when[200];	/* Put today's date here */
	static char *descrip;	/* Put descriptive text here */
	static long tvec;	/* Time since 1970 */
	static int goprint;
	extern void post_open();
	extern void post_close();
	extern void post_amp_plot(int npage,float start);


#ifdef sgi
#define SWAP 1
#else
#define SWAP 0
#endif
void get_wave();
void get_wave_name();
void readline();
void mergestring(char *, char *, char *);
void helpa();
void listformants(); 
#endif /*LSP_H*/


