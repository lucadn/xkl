/* $Log: xspec_util.h,v $
 * Revision 1.5  1999/02/18 22:29:51  vkapur
 * changed procedure declarations to be explicit
 *   i.e., declare arguments and types explicitly
 *
 * Revision 1.4  1998/07/17 08:07:22  krishna
 * Added RCS header, and added some prototypes, etc.
 * */

#ifndef XSPEC_UTIL_H
#define XSPEC_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include "xinfo.h"
#include "../../common/wavio.h"
#include "help.h"

#define VERSION "2.5"   /* xkl version */
#define DATE "2/17/99"   /* Date released */
#define DIR_LENGTH 600           /* Length in chars of current directory */

/* Current working directory */

extern XmString current_dir; /* store last directory to look in */


/*
 * Some globals used for the spectrum graphics.
 */

#define SPEC_OX .1
#define SPEC_XR .75
#define SPEC_DB 80.0

/* 
 * Spectrum params array indices. See xkl.c.
 */

#define WC 0
#define WD 1
#define WS 2
#define WL 3
#define NC 4
#define FD 5
#define B0 6
#define B1 7
#define BS 8
#define F1 9
#define F2 10
#define FL 11
#define SG 12
#define NW 13
#define SD 14
#define KN 15

/* 
 * Postscript types
 */

#define PSNONE -1
#define FULLPAGE 0
#define FOURPAGE 1

extern FILE *PS4_fp;
extern FILE *PSfull_fp;
extern int quadcount;    /* Spectrum counter for 4/pg postscript */

/*
 * Misc globals
 */


#define NUM_WINDOWS 4     /* Number of windows for each waveform */
#define GRAM       2      /* index of spectrogam */
#define SPECTRUM   3      /* index of drawing widget for spectrum */

extern int lock;                 /* specifies if wave files are locked */

extern char curdir[DIR_LENGTH];  /* current working directory */

#define MELFAC 2595.0375  /* = 1000./log10(1. + 1000. / 700. ) */

#define MINSIZE  450  /* minimum width of window for menu_bar*/

#define AVGLIMIT 100 /* max number of spectra used in 'A' */

#define NPMAX 32000
#define NFMAX  257
#define SIZCBSKIRT 200
extern float cbskrt[200];

extern char cmdstr[800];

typedef struct PARAM {
   int min;
   int max;
   int index; /* position on selection box list = index of spectro->params[] */
} PARAM;

/* Parameter structure */

typedef struct {
   int type;     /* int or float */
   char *key;    /* key to type */
   float min;    /* minimum value */
   float max;    /* maximum value */
   float value;  /* current value */
   char *desc;   /* Description   */
} paramStruct;

enum {FLOAT, INT};



extern PARAM sb_param;  

#define XAM(a,b)    (((a) > (b)) ? (a) : (b)) /* for dft library */

/* 
 * The spectro stucture has been modified to accomodate the XKL
 * program.  The structure has all the information needed to draw the
 * two waveform windows, the spectrum window and the spectrogram
 * window. The stucture stores the information about each drawing area
 * widget used in an array of "INFO" stuctures: info[0] has
 * information about the spectrum, info[1] the large waveform window,
 * info[2] the small waveform window and info[3] has information about
 * the spectrogram drawing area widget
 */

typedef struct XSPECTRO{
/*******************things from quickspec ****************/
        int type_spec;               /* flag for type of spectrum */

        char option;                 /* which combination of spectra
                                        are displayed*/
        char savoption;              /* used to store mode when
                                        doing a, A or k averaging*/
        int history;                 /* whether the last spectrum
                                        is displayed */
        float histime;               /* time of .spectrum file*/



         int oldsizwin, oldwintype;/* does a new hamming window need
                                      to be calculated */
	 int sizfft;		/*Set here, max{usersizfft, sizwin} */

	 int sizwin,savsizwin;	/* # pts in time weighting window    */

	 int nsdftmag;		/* Set in KLSPEC */

/*    Arrays from MAKEFBANK.C */

/* Defines shape of pseudo-Gaus filt */
        float cbskrt[SIZCBSKIRT];
        int nfr[4096];	        /* Center freq (Hz) of output filters */
        int savnfr[4096];	/* Center freq (Hz) of output filters */

	short nbw[NFMAX];	/* Bandwidths (Hz) of output filters */
	short pweight[NPMAX];	/* Weights of indiv. filter coefs    */
	int sizcbskirt;		/* # pts in cbskrt[]		     */
        int nchan;		/* cb		     */
        short nstart[NFMAX],ntot[NFMAX]; /*First sample contributing to filt*/
	int logoffset;
	float attenpersamp;


/*    From GETFORM.C */

	int dfltr[257];			/* Spectral slope */

#define MAX_INDEX	7
	int nforfreq;			/* number of formant freqs found   */
	int forfreq[16],foramp[16];	/* nchan and ampl of max in fltr[] */
	int valleyfreq[8],valleyamp[8];	/* nchan and ampl of min in fltr[] */

	int nmax_in_d;			/* number of maxima in deriv       */
	int ncdmax[8],ampdmax[8];	/* nchan and ampl of max in deriv  */
	int ncdmin[8],ampdmin[8];	/* nchan and ampl of min in deriv  */

	int hftot;			/* number of hidden formants found */
	int hiddenf[8],hiddena[8];	/* freq and ampl of hidden formant */



/* spectrum stuff */

        float hamm[4096];
        float dftmag[4096];
        int fltr[514]; /* spectrum */ /* store RMS in last element */
	int savfltr[514]; /* last spectrum or dft depending on mode */   
        int lenfltr,lensavfltr;
        int fd,savfd; /* spectrum,
                       save the first difference value for c and s */
        int spectrum,savspectrum; /*flag for whether to draw fltr
                                       and savfltr*/
        int savclip; /* in case savfltr needs to be clipped(8k on 5k) */
        float xfltr[256],yfltr[256],xsav[256],ysav[256];  /* postscript
                                          version of fltr and savfltr */ 
        int nxxmin,nxxmax;   /* used for 'a' and 'k' 
                                start and stop in msec*/
        int avgtimes[AVGLIMIT];    /* array of spectra centers
                               for 'A' in msec*/
        int avgcount;        /* how many spectra in 'A'*/
	double denergy;


int params[16];            /* wc,wd,ws..etc. */

float hamm_in_ms[4];     /* store sizwin in ms, convert to samples
                         in setparam and store in params[0-4]*/

/***********************spectrogram stuff****************/

Dimension menu_height;               /*used for sizing gram toplevel*/

int spectrogram;                     /* spectrogram flag whether or not 
                                       spectrogram widget is managed*/
int param_active;                    /* is the spectrum paramter selection
                                        box active for this widget */
int segmode;                         /* whether w to e or the waveform
                                       displayed in the window is played */
short *startplay;                    /* used for playing or writing section 
                                        of waveform*/
long sampsplay;                      /* number of samples in section
                                        that is played*/
int swap;                            /* if running on sgi swap bytes 
                                       in .wav*/
XImage *xim;                         /* ximage of entire spectrogram*/

char *pix;                           /* data for xim */

float *allmag;                       /* dft temporary storage*/ 

int xpix,ypix;                       /* scale factor in x and y */

int devwidth, devheight;             /* device width and height  */

Dimension ox[4],oy[4];               /* device coord of window origins */

Dimension cmdox,cmdoy;               /* device coord of cmd window origin*/

int wxmax,wymax;                     /*spectrogram size  */

int numcol;                          /* shades of grey + white */

float cinc;                          /* change of intensity
                                        between shades*/
int tickspace;                       /*spacing so tick marks
                                         don't have to be redrawn*/
int axisdist,wchar,hchar;            /*axis offset from spectrogram,
                                        character height and width   */
Dimension xr[4],yb[4];               /*window boundaries */

Dimension cmdxr,cmdyb;               /*boundaries of command window*/

int sec;                             /* seconds or milliseconds flag*/

Widget fbw_text;                   /* scrolled text window for displaying
                                        freq + bw */
Widget draww[4];                         /* the drawing area widget
                                         where the spectrogram is drawn*/
int xgram;                           /* total of pixels in x
                                         stored in .gram file*/
INFO *info[4];                           /*information used for draw in
                                        X, gc etc.*/
Widget *buttons[6];                    /* all the buttons on the menus
                                         allocated in create menus*/
Widget toplevel[4],mainw[4],menu_bar[4];  /* things to make a toplevel
                                        shell for each spectrogram*/ 
int   normstate[4];                    /* state of toplevel windows
                                         1 = normal, 0 = icon*/
char  *allgvs;                          /* all the grey indices */

int  *posti, postcount;                  /*postsript and rle stuff*/

int ps_window;                           /*which window is requesting
                                           postscript output*/
int quadcount;                            /* used for putting 4
                                          spectra on a page */ 
float gscale;                            /*postscript font scale*/

float Gscale;                            /* postscript font scale
                                            4 spectra/page*/
int wwav;                                /* (segtowav)write .. 1:play seg
                                           2:record  3:syn*/
int donew;                               /* display recorded file in
                                          current display or new one*/
float maxmag,minmag;                     /* minmag maps to white,
                                            maxmag maps to darkest gray */
short *iwave;                            /*totsamp samples must convert
                                            (float)(iwave[p]>>4)*/
int totsamp;                             /*total number of 
                                             samples in iwave file */
int  slice;                              /*length of each FFT snapshot, 
                                             default 64 samples*/
float winms;                             /*window in msec
                                            winsamps*1000/spers */
float savewinms;                         /* save for file swaps */

int winsamps;                            /*number of samples padded
                                         with 0's if needed passed to fft*/
int numav;                               /*number of FFTs to 
                                            average*/
int sdelta;                              /* number of new samples in each
                                            spectra*/
float msdelta;                           /*number of new ms in each
                                            spectrum*/
float savemsdelta;                       /* save for file swaps*/

float specms;                            /*suggested number of ms in
                                              spectrogram*/
int ovlap;                               /*amount of overlap between 
                                            each FFT*/
int insize;                              /*number of samples in wav file 
                                            doesn't have to be power of 2*/
int specsize;                            /*number of samples in display 
                                            doesn't have to be power of 2*/
int sampsused[3];                          /* the number of samples actually
                                          displayed in window*/
int  estsize;                            /*(slice-ovlap)*(numav-1)
                                              + slice*/
int numests;                             /* insize/estsize (totsamp)*/

int xmaxests;                            /* number of estimates in window*/

float  spers;                             /* samples per second 
                                             used in wave file*/
float savspers;                          /* samples per second
                                            in .fltr file */
int startmarker, endmarker;             /* sample indices(same for all
                                          windows*/
int oldstart, oldend;                   /* used to erase markers*/

float startime[3];                          /*starting time of window
                                                         in msec*/
int startindex[3];                        /*starting index of data in 
					   window*/
int t0;                                 /*time offset in image coord(spectra)*/

char fname[200];                          /* preferrence dat file name */

char wavename[200];                       /* waveform name */

char savname[200];                       /* name of wavform for
                                          read and write spectrum*/
char wavefile[200];                       /* directory and file name */

char segname[200];                       /* new file name for seg,
                                             syn and record  */
char grampath[200];                     /* directory for .gram file*/

int fdifcoef;                         /* first difference coefficient
                                         between 0 and 100 (spectrogram)*/ 
float wavemin, wavemax;               /* waveform min and max value */

int step;                             /* waveform increment */

int spratio;                        /* aspect ratio of freq to ms*/

float wvfactor[2];                       /*for zooming in on waveform*/

float savetime,oldtime;            /*time from beginning of file
                                  in ms where middle mouse is clicked*/

int saveindex,oldindex;                /* index from beginning of file
                                   where left mouse is clicked(all windows)*/

int specfreq, savspecfreq;         /* freq val at cursor location in 
                                      spectrum window  */

int spec_cursor;                  /* position of cursor in spectrum*/

float savspecamp, specamp;         /* amp val at cursor location in 
                                      spectrum window  */

int defserr;                       /* inappropriate value in defs.dat */

char tmp_name[500];               /*storage for a file name used for
                                    overwrite popup */

int localtimemax;		 /* shall we lock onto local max in time? EC*/

int localfreqmax;		 /* shall we lock onto local max in freq? EC*/
} XSPECTRO;


typedef struct {
  float real;
  float imag;
} COMPLEX;

typedef struct ALLSPECTROS {
  XSPECTRO **list;
  int count;
} ALLSPECTROS;

extern ALLSPECTROS allspectros;       /* global list of all waves */

/* 
 * Widgets that everybody shares 
 */

extern Widget warning; 
extern Widget help_dialog;
extern Widget param_sb, param_tl;
extern Widget *param_buttons;
extern Widget param_menu;
extern Widget input_dialog; 
extern Widget filesb_dialog;
extern Widget cmdw, cmdtextw;
extern Widget application; 

/***********************************************************************
 *
 *                   FUNCTION DEFINITIONS
 *
 ***********************************************************************/

/* 
 * Functions in xkl.c 
 */

void setms();
void writedata();
void dowritedata();


/**** New function definitions for xkl.c *****/

void raiseinput (Widget, XtPointer, XtPointer);
void raisewav(XSPECTRO *);
void dobutton(Widget,XtPointer, XtPointer);         /* button click stuff*/
void HandleFileMenu(Widget,XtPointer, XtPointer);
void setfilesb(XSPECTRO *);
void writetext(char *);               /* write a line of text to text window */
void writewe(XSPECTRO *);
void writcurpos(XSPECTRO *);
void HandleTimeMenu(Widget, XtPointer, XtPointer);
void HandleHelpMenu(Widget, XtPointer, XtPointer);
void HandleAudioMenu(Widget,XtPointer, XtPointer);
void HandleParamMenu(Widget,XtPointer, XtPointer);
void HandleRecMenu(Widget,XtPointer, XtPointer);
void HandleSpectrumMenu(Widget,XtPointer, XtPointer);
void HandleWavlistMenu(Widget,XtPointer, XtPointer);
Widget CreateOneMenu (Widget, char *, char **, int, Widget *, Boolean, int);         /* menupane */
void CreateMenus (Widget, XSPECTRO *, Widget **, Widget *, int);           /* menubar  */
void CreateHelpDialog(Widget);
void CreateHelpText(Widget);
void createtext(Widget, Widget *);
void doresize(Widget,XtPointer, XtPointer);      /* resize window */
void spurious (INFO *, Dimension, Dimension);    /* deal with extraneous resize events */
int resizegram(XSPECTRO *);
void dorec(Widget, XtPointer, XtPointer);
void doparam(Widget, XtPointer, XtPointer);
void doparamsAll(Widget, XtPointer, XtPointer);
void inputrec(int);
void writeParam(paramStruct, char *);
void listrec();
void dolist (XSPECTRO *);
void setrec(Widget, XtPointer, XtPointer);
void setparam(Widget, XtPointer, XtPointer);
void donerec(Widget, XtPointer, XtPointer);
void doneparam(Widget, XtPointer, XtPointer);
void CreateRecDialog(Widget);
void CreateParamDialog(Widget);
void CreateWavList(Widget);
void dowavlist();
void dowav(Widget, XtPointer, XtPointer);
void FindWav(XSPECTRO *, int);
void shiftCurrentWav(XSPECTRO *, int);
void cancelwavlist(Widget, XtPointer, XtPointer);
void CreateInputDialog(Widget);
void CreateFileSB(Widget);
void CreateWarning(Widget);
void CreateOops(Widget);
void wavgram(int, int *, char **, int);
void xklPlay(XSPECTRO *, int);
void xklRecord(XSPECTRO *);
void do_usage();
void doraise(Widget, XtPointer, XVisibilityEvent *, Boolean *); /* if input dialog is obscured raise it */
void undomap(XEvent *);
void domap(XEvent *);                      /* set flag when window is iconified */                    
void dopsfile(Widget, XtPointer, XtPointer);   /* get postscript file name */
void dooverwriteps(Widget, XtPointer, XtPointer);
void openps(XSPECTRO *);                    /* get postscript file name */  
void activateMenuItem(XSPECTRO *, char);
void deActivateMenuItem(XSPECTRO *, char);
void writeps(XSPECTRO *, FILE *);
void avgval(Widget, XtPointer, XtPointer);  /* get input for 'A' averaging */
void cancelavg(Widget, XtPointer, XtPointer);
void doneavg(Widget, XtPointer, XtPointer);
void aval(Widget, XtPointer, XtPointer);    /* get input for 'a' averaging */
void readparams(Widget, XtPointer, XtPointer);  /* read a spectrum ,params file */
void writeparams(Widget, XtPointer, XtPointer);
void dowriteparams(Widget, XtPointer, XtPointer);
void cancelfilesb(Widget, XtPointer, XtPointer);
void cancelinput(Widget, XtPointer, XtPointer);
void cancelwarning(Widget, XtPointer, XtPointer);
void showvalues (XSPECTRO *);              /* show spectrum valules in help type window*/
void setwvfact(Widget, XtPointer, XtPointer);
void segtowav(Widget, XtPointer, XtPointer);
void writewarn(Widget, XtPointer, XtPointer);
int doreplace (XSPECTRO *);
void recstart(XSPECTRO *, int);
void recquery (XSPECTRO *);
void recswap(Widget, XtPointer, XtPointer);
void doswap(Widget, XtPointer, XtPointer);
void recnew(Widget, XtPointer, XtPointer);
void cancelrec(Widget, XtPointer, XtPointer);
void opendialog (Widget);
void open_tl_dialog (Widget);
void exitwarning();
void doexit();
void ShowOops (char *);
void closefullps(Widget, XtPointer, XtPointer);  /* close fullpage postscript file */
void close4ps(Widget, XtPointer, XtPointer);     /* close 4/page postscript file*/
void closejnl();
void openjnl(Widget, XtPointer, XtPointer);
void dooverwritejnl(Widget, XtPointer, XtPointer);

/* Synthesis */
void HandleSynthesizeMenu(Widget, XtPointer, XtPointer);
void HandleSynMenu(Widget, XtPointer, XtPointer);
void dosyn(Widget, XtPointer, XtPointer);
void inputsyn (XSPECTRO *, int);
void synpar(Widget, XtPointer, XtPointer);
void CreateSynDialog (Widget);
void donesyn(Widget, XtPointer, XtPointer);
void synps(Widget, XtPointer, XtPointer);
void writesynps();
void dowritesynps(Widget, XtPointer, XtPointer);
void printsyn (char *);
void writedoc(Widget, XtPointer, XtPointer);
void warndoc(Widget, XtPointer, XtPointer);
void syndef(Widget, XtPointer, XtPointer);
void syndoc(Widget,XtPointer, XtPointer);
void rdoc(Widget,XtPointer, XtPointer);
void showdoc();
void showvaried();
void drawsyn (FILE *);
void syn_pixmap();
void resizesyn(Widget, XtPointer, XtPointer);
void expsyn(Widget, XtPointer, XtPointer);
void inputparams (XSPECTRO *, int);
void vartime(Widget, XtPointer, XtPointer);
void setvar(Widget, XtPointer, XtPointer);
void setcon(Widget, XtPointer, XtPointer);
void ShowDocDialog();

void HighlightInput (char *, Widget);


/* 
 * xspec_util.c
 */

void spec_cursor(XSPECTRO *);
void expdraww();                /* draw just exposed*/
void postgvs();
int  readfreq();
void writefreq();
void startup();
void set_defaults();
int  read_data();
void add_spectro(XSPECTRO *, char *, char *);
void spectro_pixmap();
void make_pixmap();
void win_size();
void findsdelta();
void findypix();
void calculate_spectra();
void draw_spectrogram();
void draw_cursor();
void draw_axes();
void findxryb();
void findtime();
void findminmaxtime();
void eraseline();
void erasechar();
void xline();
void gramoffset();
void remapgray();
void update();
void settime();
void swapwave();
void wave_stuff();
void canceltime();
void cancelswap();
void delete_wave();
void delete_spectro();
void createwindows();
void mapwindows();
void newwave();
void do_newwave();
void cancelnew();
void validindex();
void draw_hamm();
void swapstuff();
void LoadNewWaveform(XSPECTRO *, char *);
void ChangeSpectrumOption(XSPECTRO *, char);
void SetCurrentTime (XSPECTRO *, float);

/* 
 * xklspec.c 
 */

void readspectrum();
void cancelread();
void writespectrum();
void dowritespectrum();
void cancelwrite();
void writegram();

/*
 * function prototypes for dft and inverse dft functions 
 */

void fft(COMPLEX *,int);
void ifft(COMPLEX *,int);
void spec_dft(COMPLEX *,COMPLEX *,int);
void idft(COMPLEX *,COMPLEX *,int);
void rfft(float *,COMPLEX *,int);
void ham(COMPLEX *,int);
void han(COMPLEX *,int);
void triang(COMPLEX *,int);
void black(COMPLEX *,int);
void harris(COMPLEX *,int);
int ilog2(unsigned int);

#endif /*XSPEC_UTIL_H*/
