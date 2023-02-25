/*
$Log: syn.h,v $
Revision 1.4  1998/07/16 22:59:29  krishna
Removed full pathname for default file, so the user can specify where
the file will be located in the Makefile.

Revision 1.3  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#ifndef SYN_H
#define SYN_H

/*IMPORTANT make sure this path is correct*/
static char defaultFile[] = "default60.con";

static char kldate[] =
 {"KLSYN93   Version 2.1 98/07/17 KG (original program by D.H. Klatt)"};

#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "wavio.h"

/* added to convert from streamline filetype 4/93 NM*/
#ifdef VMS
#include rms
struct FAB fab;
struct RAB rab;
#endif


#ifdef sgi
#define SWAP 1
#else
#define SWAP 0
#endif


/*	K L S Y N . H			D. H. Klatt			*/
#define NPAR             64  /*93*/
#define MAX_VAR_PARS     63  /*93*/
#define LOC_F0INTERP	 48
#define MAX_FRAMES      400
#define FIXED_S           'C'
#define VARIABLE        'v'
#define VARRIED         'V'
#define NOVICE          'n'
#define VT100		'd'
#define VT125           't'
#define VS100		's'
#define TTRUE           't'
#define GRID_ON		3
#define GRID_OFF	2
#define ERASE_PAR	1
#define OVERLAY_PAR	0

/* For VT125 terminal plotting */

#define CHAR_V_SIZE_VT125	20
#define CHAR_H_SIZE_VT125	10
#define MIN_INTENSITY_VT125	1
#define MID_INTENSITY_VT125	2
#define MAX_INTENSITY_VT125	3
#define XMAX_VT125		768
#define YMAX_VT125		480


#ifndef False
#define False 0
#endif
#ifndef True
#define True  1
#endif

float dBconvert();
float DBtoLIN();
void printstuff();
void helpa();
void initconfig(char *); 
void clearpar();
void copystring(char *,char *);
void makefilenames();
void readdocfile();   
void helpr();
char get_request();
void actonrequest(char);
void endgraph();
void putconfig(FILE *); 
int get_par_name();
void settypicalpar();
void drawparam();
int graphallpars();
void askforname(); 
int synthesize();
void savedocfile();
void prpars(FILE *);
void skip_line(FILE *);
void readheader();
int get_digits(FILE *); 
char get_nonwhite_char(FILE *);
void setspdef();
int get_value(int);
void prparnames(FILE *);
void initpars();
void begingraph();
void gettimval(int*, int*);
void eraseparline();
void drawparline();
void draw_line();
int get_time();
int getpval(); 
void setpval(); 
int checklimits();
void post_constants();
int post_variables();
void putforfreq(char *);
int relevent(char,char,char);
int conval(char *);
int findtext(char,char);
int findnp(char *);
int decodparam();
void mergestring(char *, char *, char *);

        FILE *fconf, *odev, *odoc;
        int pdata[MAX_VAR_PARS][MAX_FRAMES];  /* Parameter data [np][nf] */
        char firstname[200];
	char dname[200];	      /* .doc configuration file name */
	char wname[200];	      /* .wav waveform file name */
	char pname[200];	      /* .ps postscript laser plot file name */
        char string[40];		/* Temporary string buffer */
        /*93*/
 static char definitionGI[]=" Overall gain scale factor for AI, in dB ";
 static char definitionAI[]=" Amp of impulse, in dB ";
 static char definitionFSF[]=" Formant Spacing Filter (1=on, 0=off) ";
        /**/
	char param[5];
        char request;
        /*char request;*/
	char command;
	char sym1,sym2,sym3;
        int syn_i;
        /*int i,n,npar1,lastnf,lastval;*/
        int n,npar1,lastnf,lastval;
	int nsamtot,nskip;
	short *synwave;
        int warningsw; 
	static int nc,samrat,nsperframe;
	int ms25,dispt;


/*    State Variables */
        char user;      /* If == NOVICE, print everything */
        char graphout;  	/* = {VT100, VT125, VS100} */
	int plotsw;	/* Set to 1 if plot has been made on VT125 screen */
        char ipsw;      /* TRUE if default pars inserted in par tracks */
        int np;         /* Current parameter */
        int nf;         /* Current time frame */
        int val;        /* Current value of current par for cur frame */

/*    Configuration definition Arrays */
/*              First entry in config must be desired utterance dur in ms */
/*              Second entry must be the desired number of ms per frame */
/*              (from these numbers are computed NSAMP_PER_FRAME & nframes) */
        int npars;              /* Number of parameters in config definition */
	int totdur;		/* Utterance duration in msec */
	int ms_frame;		/* Number of msec in a frame */
        int nframes;            /* # frames (Dur in ms divided by framedur */
        int nvar;               /* Number of variable parameters in config */
        char cv[NPAR];          /* Constant/variable switch for each param */
        char symb[NPAR][4];     /* 2 or 3 char symbol for parameter */
        int maxval[NPAR];       /* Maximaum values for all parameters */
        int minval[NPAR];       /* Minimum values for all parameters */
        int defval[NPAR];       /* Default values for all parameters */
	char definition[NPAR][51];  /* Text definition of each parameter */
        int dispars[NPAR]; /* Flags for other pars to be displayed (NOT USED) */
/*    Variables known to parwave.c */
        int pars[NPAR];		/* Whenever NPAR changed, change code below */
				/*  that sets f0next, and change all of the */
				/*  pars[xx] = statements in PARWAVEF.C     */
	int sigmx;

/* 93   Speaker Definition	    0  1  2  3  4  5  6  7  8  9 10 11 12*/
        int spkrdef[13];	/* du ui sr nf ss rs sb cp os gv gh gf gi*/
/*				     (ui is in samples) */



	/*static KEYIO keyiospace;*/
	static int ttype,*ptext;
	int forfreq[16],nforfreq;	/* Formant freq estimates */
	int time_in_ms; 	/* Time in msec corresp. to cursor position */

	static int ngrids,tgridmax,timetrunc;
	static char symdigit[5] = {'1', '2', '3', '4', '5' };       


#endif /*SYN_H*/


