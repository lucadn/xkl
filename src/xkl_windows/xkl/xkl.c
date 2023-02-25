/* 
 * $Log: xkl.c,v $
 * Revision 1.17  2002-06-13 11:22:33-04  tiede
 * mods for labelling, cursor tracking; new menu entries; memory leaks plugged
 *
 * Revision 1.14  2000-11-02 15:59:16-05  tiede
 * fixed a problem with menu selection on Linux2 platforms
 * (SB_ListSelectedItemPosition broken)
 *
 * Revision 1.13  2000/09/15 19:57:09  tiede
 * label file mask changed:  *.lm ==> *.l* (*.label file support)
 *
 * Revision 1.12  2000/08/24 20:04:53  tiede
 * fix log prefix complaint
 *
 * Revision 1.11  2000/08/24 19:55:21  tiede
 * spectrogram now displayed in current device depth
 * de/activateMenuItem now applies to all waveforms, not just current
 * menu sensitivity set from currently open waveforms
 * support for reading/display of LAFF labels
 *
 * Revision 1.8  2000/06/08 03:05:51  tiede
 * byte-swapping behavior now inferred
 * .doc files get default name from .wav file in synthesis
 *
 * Revision 1.7  2000/04/20 19:13:56  xkldev
 * bogus change to demo RCS
 *
 * Revision 1.6  2000/04/20  05:22:18  krishna
 * Fixed warning messages.
 *
 * Revision 1.5  1999/05/20 19:45:03  vkapur
 * lock/unlock is fully functional.
 *   "lock"- provides a dialog to select which waveforms to be
 *           locked.  currently locked waveforms are highlighted.
 *   "unlock" - unlocks all waveforms.
 *
 * Revision 1.4  1999/02/18 22:05:27  vkapur
 * File selection box remembers current directory
 * "n" function allows for number of waveform to be pressed
 * all oops messages => use ShowOops
 * select text in systhesis dialog boxes to allow for
 *   continuous typing
 * after out-of-range box comes up from an input dialog,
 *   the input dialog remains for re-entering a new value
 * synthesis and save works-- calls up dialogs for both functions
 *
 * Revision 1.3  1998/07/17 08:55:39  krishna
 * Changed title for text window...
 *
 * Revision 1.2  1998/07/17 07:58:53  krishna
 * Made many changes, including but not limited to:
 *   - Modified to allow user to not open another window when they save a
 *     w->e .wav file.
 *   - Changed some of the menu item labels to be more understandable.
 *   - Change where xkl look for lib files by using -DXKLIBDIR=${LIBDIR}
 *     in Makefile.
 *   - Changed "D Delete .wav file" to "D Close .wav file" per Adrienne's
 *     request:  Also the command under File menu that is Delete the
 *     waveform might be clearer if it was Delete waveform from window or
 *     something that made it clear the original file would not be
 *     deleted completely.
 * */

/*********************************************************************
 *
 * Main program which gives the user access to klsyn, klspec,
 * recording and editing
 *
 *********************************************************************/

#include "../syn/syn_functions.h"    /* all X headers are in xinfo.h */
#include "../syn/synparwav.h"    /* all X headers are in xinfo.h */
#include "xspec_util.h"    /* all X headers are in xinfo.h */
#include "xklspec.h"    /* all X headers are in xinfo.h */
#include "resources.h"     /* X resource info */
#include "wavio.h"         /* Funcs for reading/writing wave files */
#include "wavaudio.h"      /* Funcs for recording/playing wave files */
#include <string.h>

/***********************************************************************
 *
 * X related widgets that everybody shares 
 ***********************************************************************/

Widget help_text;        /*the same for all toplevel shells*/
Widget param_sb,param_tl; /*list of spectrum parameters*/
Widget *param_buttons;   /*entries on parameter list menu*/
Widget param_menu;       /*widget that has HandleParamMenu as callback*/

Widget rec_sb,rec_tl;    /*list of record parameters*/
Widget *rec_buttons;     /*entries on record parameter list menu*/
Widget rec_menu;         /*widget that has HandleRecMenu as callback*/

Widget syntext;          /* window for displaying syn parameters */
Widget syn_sb,syn_tl;    /* list of synthesizer commands*/
Widget *syn_buttons;     /*entries on syn list menu*/
Widget syn_menu;         /*widget that has HandleSynMenu as callback*/
Widget syngraph;         /* drawing widget for synthesizer */

Widget wav_sb, wav_tl;           /*list Widget for choosing waveform*/
Widget wav_menu;         /*widget that has HandleWavlistMenu as callback*/
Widget *wav_buttons;     /*entries on syn list menu*/

Widget input_dialog;     /* general input */
Widget filesb_dialog;    /* file browser */
Widget cmdw, cmdtextw = NULL; /* separate application(for cut and paste)*/
Widget application;      /* parent of all the graphics */
Widget warning;          /* warning message box, overwrite? */
Widget warning2;         /* 2nd warning message box */
Widget oops;             /* notification of value out of range etc. */ 
PARAM sb_param;  /* used by parameter selection box and input dialog*/


/* 
 * 2 applications so text window is independent of graphics 
 */

XtAppContext app_context, text_app;

/***********************************************************************
 *
 *                        GLOBALS 
 *
 ***********************************************************************/

FILE *PS4_fp = NULL;      /* 4 spectra/pg postscript file */
FILE *PSfull_fp = NULL;   /* 1 spectra/pg postscript file */
FILE *D_fp = NULL;        /* test file for freq+amp */
int quadcount;            /* spectra count on 4/pg postscript file */
int PStype = PSNONE;

int lock = 0;             /* toggle move time on all waveforms with arrow */

char curdir[DIR_LENGTH];
char lastPar[10];         /* last parameter */

XmString current_dir; /* store last directory to look in (loading) */
XmString save_dir;    /* remember save dir */

XtWorkProcId playerID;  // work proc for audio output


/***********************************************************************
 *
 *              Synthesizer
 *
 ***********************************************************************/

#define VARRIED         'V'
#define FIXED_C       'C'
#define MAX_FRAMES      400
extern char firstname[];
GC syngc;
Pixmap synmap;
int synxr,synyb;
char synstr[200]; /* used to get info to syn callbacks */
int which_syntext;  /* 1 = 'p',  2 = 'd' */
char syn_tmp_name[500];

static String synlist[] = {
    "p       PRINT (screen) synthesis parameter default values",
    "c       CHANGE default value for a synthesis parameter",
    "d       CHANGE synthesis parameter time function",
    "G       GRAPH .ps varied pars as a function of time",
    "s       SYNTHESIZE waveform file, save everything",
    "S       SAVE parameters as .doc file (no synthesis)",
    "g       GRAPH .ps syngraph window",
    "v       PRINT (screen) varied parameters",
    "e       DONE"
};


/*
 * RECORDING
 */

#define numRecParams 2
enum {REC_DUR, REC_RATE};
paramStruct recParams[] = {
   {FLOAT, "d",   .5,     5,     2,    "Recording duration (sec)"},
   {FLOAT, "s", 2000, 48000, 16000,    "Sampling rate (Hz)"},
};

/*
 * reclist needed for creating menu.  Delete after better way of creating 
 * menu.
 */

static String reclist[] = {
    "d   .5     5.0    Recording duration (sec)",
    "s   8000   22000  Sampling rate (Hz)",
    "e                 DONE"
};

/* 
 * ALLSPECTROS contains all the inforamtion associated with each
 * different waveform, each of which is stored in a XSPECTRO
 * structure.  allspectro.list[0 - allspectro.count] is a list of
 * XSPECTRO pointers 
 */

ALLSPECTROS allspectros;

/* 
 * Parameters for spectra.
 *
 * Left column is the accelerator list for the selection
 * box that pops up when the user wants to change a parameter.
 * See add_spectro in xspec_util.c to set startup values.
 */

enum {SPEC_CB, SPEC_DFT, SPEC_SMOOTH, SPEC_LPC, SPEC_NUMLPC, SPEC_PREEMP, 
      SPEC_CB_BW_1, SPEC_CB_BW_1000, SPEC_SMOOTH_BW, SPEC_CB_CF1, 
      SPEC_CB_CF2, SPEC_CB_LOG, SPEC_GAIN, SPEC_WINDOW, SPEC_DFT_SIZE, 
      SPEC_K_AVG};

paramStruct specParams[] = {
  {FLOAT, "c", 1.0, 50.0, 25.7, "# ms in Hamming window for critical band spectrum"},
  {FLOAT, "d", 1.0, 50.0, 29.9, "# ms in Hamming window for DFT spectrum"},
  {FLOAT, "s", 1.0, 50.0, 25.6, "# ms in Hamming window for smoothed spectrum"},
  {FLOAT, "l", 1.0, 50.0, 25.6, "# ms in Hamming window for LPC spectrum"},
  {INT,   "n",   2,   20, 14,   "# of coefficients in analysis for LPC"},
  {INT,   "p",   0,  100, 0,    "First difference. Pre-emphasis if = 100"},
  {INT,   "b",  60,  500, 70,   "Bandwidth of CB filter at lowest frequency"},
  {INT,   "B",  60,  500, 180,  "Bandwidth of CB filter at 1000 Hz"},
  {INT,   "S",  40,  500, 300,  "Bandwidth of smoothed spectrum filter"},
  {INT,   "f", 100,  300, 100,  "Center frequency of first CB filter"},
  {INT,   "F", 120,  500, 124,  "Center frequency of second CB filter"},
  {INT,   "L", 500, 2000, 700,  "Freq at which CB goes from linear to log"},
  {INT,   "g",   0,  700, 450,  "Spectrum gain in dB times 10"},
  {INT,   "w",   0,    1, 1,    "Use window if =1, no windowing if =0"},
  {INT,   "D",  16, 4096, 512,  "Size of DFT (256/512 for others)"},
  {INT,   "k",   1,   50, 10,   "ms on either side of cursor to be averaged"},
};

static String paramlist[] = {

/*0*/  "c    1.0   50.   # ms in Hamming window for critical band spectrum (c)..",
/*1*/  "d    1.0   50.   # ms in Hamming window for DFT spectrum (d)..",
/*2*/  "s    1.0   50.   # ms in Hamming window for smoothed spectrum (s)..",
/*3*/  "l    1.0   50.   # ms in Hamming time window for LPC spectrum..",
/*4*/  "n      2    20   # of coefficients in analysis for LPC..",
/*5*/  "p      0   100   First difference. Pre-emphasis if = 100..",
/*6*/  "b     60   500   Bandwidth of CB filter at lowest frequency..",
/*7*/  "B     60   500   Bandwidth of CB filter at 1000 Hz..",
/*8*/  "S     40   500   Bandwidth of smoothed spectrum filter..",
/*9*/  "f    100   300   Center frequency of first CB filter..",
/*10*/ "F    120   500   Center frequency of second CB filter..",
/*11*/ "L    500  2000   Freq at which CB goes from linear to log..",
/*12*/ "g      0   700   Spectrum gain in dB times 10..",
/*13*/ "w      0     1   Use window if =1, no windowing if =0..",
/*14*/ "D     16  4096   Size of DFT (256/512 for others)..",
/*15*/ "k      1    50   ms on either side of cursor to be averaged (k)..",
/*16*/ "e                DONE"
};

/*
 * Min and max for spectra parameters.
 * See setparam() to change min and max of DFT size.
 */

static int paramin[] = {1,1,1,1,2,0,60,60,40,100,120,500,0,0,16,1};
static int paramax[] = {50,50,50,50,20,100,500,500,500,300,500,
			2000,700,1,4096,50};

/*
 * see makefbank 
 */

float cbskrt[SIZCBSKIRT] = {
1.00,  .998,  .995,  .984,  .972,  .952,  .931,  .907,  .883,  .849,
.814,  .775,  .735,  .699,  .662,  .618,  .573,  .529,  .484,  .447,
.410,  .370,  .329,  .292,  .255,  .238,  .221,  .206,  .191,  .179,
.167,  .157,  .146,  .137,  .128,  .120,  .112,  .106,  .099,  .093,
.087,  .082,  .077,  .073,  .069,  .065,  .061,  .058,  .055,  .052,
.049,  .047,  .044,  .042,  .040,  .038,  .036,  .035,  .033,  .032,
.030,  .029,  .027,  .026,  .025,  .024,  .023,  .022,  .021,  .021,
.020,  .019,  .018,  .018,  .017,  .017,  .016,  .016,  .015,  .015,
.014,  .014,  .013,  .013,  .012,  .012,  .0115, .0113, .011,  .0105,
.010,  .0098, .0095, .0093, .009,  .0088, .0085, .0083, .008,  .0077,
};


/*
 * MENU STUFF
 */

static XtCallbackProc menuprocs[] = {HandleFileMenu,HandleTimeMenu,
				     HandleSpectrumMenu,HandleAudioMenu,
				     HandleSynthesizeMenu,HandleHelpMenu};


static String menus[] = {"File", "Time", "Spectrum", "Audio", 
			 "Synthesize", "Help"};

/* 
 * Num of entries in each menu.  ATTENTION if entries array is
 * modified num_entries must also be modified.
 */

static int numPulldown = 6;
static int num_entries[] = {32,24,20,5,1,1};

/*
 * Menu items. If edited change int num_entries[].  The first character(s)
 * are the menu accelerators.  They must be unique in the entries list.  
 * Accelerators beginning with ! are special characters, e.g. !A 
 * corresponds to arrow keys, !C corresponds to Control.  So, for left 
 * arrow, one  specifies !AL. Only !AL, !AR, !AU, !AD work right now.
 */

static String entries[] = {

   /* FILE */

   "r Read new .wav file",
   "v Replace current .wav file",
   "D Close .wav file",
   "Z Read spectrum file",
   "z Save spectrum to file",
   "M Read parameter file",
   "m Save parameter file",
   "o Save w->e to .wav file",
   "O Save w->e to ASCII file",
   "",
   "y Read Label file",
   "x Delete all labels",
   "X Edit labels",
   "9 Make label @ cursor",
   "I Save labels",
   "",
   "G Open/close 1 spectra/pg PS file",
   "* Save window to PS file (G)",
   "",
   "g Open/close 4 spectra/pg PS file",
   "# Save spectrum to 4/pg PS file (g)",
   "",
   "N List freq. & BW in window",
   "V List spectrum values in window",
   "H Open/close amp. & freq. file",
   "h Save amp. & freq. to file (H)",
   "",
   "n Select waveform",
   "+ Forward one waveform",
   "- Backward one waveform",
   "",
   "q Quit",

   /* TIME MENU */ 

   "b Move back 10 ms",                 
   "f Move forward 10 ms",              
   "B Move back 50 ms",                 
   "F Move forward 50 ms",              
   "!AL Step left by width of window 1",  
   "!AR Step right by width of window 1",   
   "",                                  
   "t Move marker to time t",           
   "w Set start of selected region",    
   "e Set end of selected region",      
   "W Set w to beginning of file",      
   "E Set e to end of file",            
   "^ Set both W and E",                
   "1 Move marker to w",                
   "2 Move marker to e",                
   "J Show w->e in window 1",           
   "",                                  
   "@ Peak/valley picking on/off",
   "",
   "!AU Zoom in (window 0 & 1 only)",
   "!AD Zoom out (window 0 & 1 only)",
   "",                                  
   "L Lock cursor arrows",         
   "U Unlock cursor arrows",       

   /* SPECTRUM */

   "d DFT magnitude",
   "s Smoothed spectrum",
   "S Smoothed spectrum + DFT",
   "c Critical-band",
   "j Critical-band + DFT",
   "T c-b compute slope > 1kHz",
   "l Linear-prediction + DFT",
   "a Avg. DFT mag over interval",
   "A Avg. DFT mag ",
   "k Spectral avg. -kn/2 to kn/2",
   "C Change spectrum parameters",
   "",
   "i Include spectrogram",
   ", Lighten spectrogram",
   ". Darken spectrogram",
   "u Set msec in spectrogram",
   "8 Change spectrogram parameters",
   "K Recalculate spectrogram",
   "",
   "/ Peak picking on/off",

   /* AUDIO */

   "p Play between w->e",
   "P Play entire file",
   "0 Abort audio output",
   "R Record to .wav file",
   "Q Change record parameters",

   /* SYNTHESIZE */

   "Y Synthesize",

   /* HELP */

   "? help "
};

/* END of MENU */

/***********************************************************************
 *
 *                         MAIN PROGRAM 
 *
 ***********************************************************************/


int main(int argc, char** argv)
{
    String *fallback_resources;
    Arg args[20];  /* for initializing Widgets */
    int n;         /* for initializing Widgets */
    XSPECTRO *spectro ;   /* all wave info */       
    int i,s;
    Dimension cmdox,cmdoy;
    XtInputMask mask;
    extern char *small_resources[];
    int *goodnames; /* indices of argv that are valid files*/
    FILE *fp;
    int dogram; /*0 = no spectrgram*/
    int swap;   /* byte swap on sgi */
    int recmode; /* recording mode start without a .wav file */
    int batchgram;/* flag for -g mode, just create gram files */
    XEvent pevent;  /*used with peekevent*/
    char tmpname[800], tmpstr[400], *p;

    /*find display size */
    Display *display;
    char * display_name  = NULL;
    int screen_num;
    XFontStruct *font;
    int devwidth, devheight;
    int wchar, hchar;
    short swapTest = 1;

    /* 
     * Initailize record parameters 
     */

    recParams[REC_DUR].value = 2.0;         /* seconds */
    recParams[REC_RATE].value = 16000.0;    /* rate */

    /* 
     * Initailize some synthesizer stuff 
     */

    syntext = NULL; /* create it when it's called for */
    syngraph = NULL;
    syn_tl = NULL;
    synmap = 0;
    syngc = NULL; 
   /* record parameter window */
    rec_tl = NULL;

#ifdef _OBSOLETE  // now assume implicit -n if no filename specified
/* need wavefile name to start */
if( argc < 2 ) {
      do_usage(); /* exit with usage */  
    }
#endif

/* sort out comand line arguments */

     swap = 0; /* default reading .wav on DEC */ 
//#if defined(sgi) || defined(sun)
//    swap = 1;
//#endif
     recmode = 0;
     batchgram = 0;
     dogram = 0;  /* default to no spectrogram */
  /* check first to see if any command line options */

   allspectros.count = 0;
   /* store index of argv */
   goodnames = (int *)malloc( sizeof(int) * argc);

   if (argc < 2) {  /* no argument:  start in record/synthesis mode */
     recmode = 1;
     allspectros.count = 1;
   }

   s = 0;
   for (i = 1; i < argc; i++){
     if(argv[i][0] == '-'){
       switch (argv[i][1]) {
         case 'g': batchgram = 1; break; /* just generate .gram files*/
         case 's': swap = 1; break;      /* byte swap on sgi etc.*/
         /* if record mode, ignore all other command line args*/
         case 'n': recmode = 1; allspectros.count = 1; break;
         default:  printf("illegal or unsupported option\n"); break;
       }
     } else{
       /* argv[i] may include .wav or not */
       if(strcmp(&argv[i][strlen(argv[i]) - 3 ],"wav") != 0 )
            {sprintf(tmpname,"%s.wav",argv[i]);}
       else{strcpy(tmpname,argv[i]);}
       if( (fp = fopen(tmpname,"r")) ){
            goodnames[s] = i;
            allspectros.count ++;
            s++;
            fclose(fp);
	  }/* good name */
        else{
            fprintf(stderr,"ERROR attempting to open %s\n\n", argv[i]);
         }/* bad name */
       }/* wavenames */
    }/* all args */

/* infer byte-swapping on little-endian machines */
/* command line -s argument inverts inferred behavior all platforms */
   if (!(*((unsigned char *)(&swapTest)) == 1))
     swap = !swap;

/* no good file names */
if((allspectros.count < 1 && !recmode) || (recmode && batchgram) ){
      do_usage(); /* exit with usage */ 
   }

 /* allocate 1 xspectro */

if(recmode)
      allspectros.count = 1;

    getcwd(curdir, DIR_LENGTH);

/* just create .gram files from .wav files and exit */
// (obsolete)
//if(batchgram)
//    wavgram(allspectros.count,goodnames,argv,swap);


/* small_resources are defined in xspec_util.h because 
    of inexplicable overwriting of resources in VMS */

/* want to know display dimensions before choosing resources */

/*window sizing find max in pixels*/
if( (display = XOpenDisplay(display_name)) == (Display *)NULL){

  fprintf(stderr,"ERROR trying to open DISPLAY \n");
  exit(0);
}
    screen_num = DefaultScreen(display);
    devwidth = DisplayWidth(display,screen_num); 
    devheight = DisplayHeight(display,screen_num);

    font = XQueryFont(display,
		      XGContextFromGC(DefaultGC(display,screen_num))  );

    wchar = font->max_bounds.rbearing - font->min_bounds.lbearing;
    hchar = font->max_bounds.ascent + font->max_bounds.descent;

    XCloseDisplay(display); /* gets opened again later with AppInitialize */

    /* the parent of all graphics windows */

    n = 0;
    /*never see this one*/

    XtSetArg(args[n],XmNmappedWhenManaged,False); n++; 
    XtSetArg(args[n],XmNwidth,100); n++;
    XtSetArg(args[n],XmNheight,100); n++;
    XtSetArg(args[n], XmNx,100); n++;
    XtSetArg(args[n], XmNy,100); n++;

    /* deallocate widgets */
    XtSetArg(args[n], XmNinitialResourcesPersistent,False );n++;
    
    if(devheight < 800) fallback_resources = small_resources;
    else fallback_resources = big_resources;

    /* the parent of all graphics windows */


    application = XtAppInitialize(&app_context,"xkl",
				  NULL,0,
				  (&argc),
				  argv,
				  fallback_resources,
				  args, n);

   /* have a separate application for text window
     so that when the popup dialog kills input to
     menus the text window is still active for cut and paste
     maybe this could be a help text and not require a separate
     application
   */
      
    n = 0; 
      XtSetArg(args[n], XmNdeleteResponse,  XmDO_NOTHING );n++;
      sprintf(tmpstr, "Xkl v%s", VERSION);
      XtSetArg(args[n], XmNtitle, tmpstr);n++;
      cmdw = XtAppCreateShell("xkl_textwindow",NULL,
         applicationShellWidgetClass,XtDisplay(application), args, n);
      text_app = XtWidgetToApplicationContext(cmdw); 

    n = 0;
    XtSetArg(args[n],XmNx,&cmdox); n++;
    XtSetArg(args[n],XmNy,&cmdoy); n++;
    XtGetValues(cmdw ,args,n);


   /* get this on the screen so user knows something is happening */
   n = 0;
   XtSetArg(args[n],XmNeditable,False); n++;
   XtSetArg(args[n],XmNeditMode,XmMULTI_LINE_EDIT); n++;
    cmdtextw = XmCreateScrolledText(cmdw,"textw",args,n);
   XtAddCallback(cmdtextw,XmNfocusCallback,raiseinput,NULL);
   XtManageChild(cmdtextw);
   XtRealizeWidget(cmdw); /* text window */

     /* defaults may be overriden by command line options */

#ifdef VMS
        Setup_Hardware();
#endif  

/* Initialization to make filter skirts go down further (to 0.00062)*/

	for (s = 100; s<SIZCBSKIRT; s++) 
	    cbskrt[s] = 0.975 * cbskrt[s-1];
/* init cbskrt */
      


allspectros.list = 
     (XSPECTRO **) malloc( sizeof( XSPECTRO *) * allspectros.count);
       
for(s = 0; s < allspectros.count; s++){   
    allspectros.list[s] = (XSPECTRO *) malloc( sizeof(XSPECTRO) );
    spectro = allspectros.list[s];
    spectro->swap = swap;
    spectro->spectrogram = dogram;
    spectro->devwidth = devwidth;
    spectro->devheight = devheight;
    spectro->wchar = wchar;
    spectro->hchar = hchar;


    if(recmode) {
      strcpy(tmpname,"?");
    }/* don't open a .wav*/
    else {
      if(strcmp(&argv[goodnames[s]][strlen(argv[goodnames[s]]) - 3],
		"wav") != 0 )
	 
	sprintf(tmpname,"%s.wav", argv[goodnames[s]]);
      else sprintf(tmpname,"%s", argv[goodnames[s]]);
    }

    add_spectro(spectro,"xkl_defs.dat",tmpname); /* need to fix*/
    initime(spectro);
 }/* set up all waves*/
free(goodnames);

/* save path to executable (search loc for defaults60.con) */
 spectro = allspectros.list[0];
 strcpy(spectro->synDefPath, argv[0]);
 p = strrchr(spectro->synDefPath, '/');
 if (!p) p = spectro->synDefPath;
 *p = 0;

    /* generic popups shared by everyone */
    CreateHelpDialog(application);
    CreateParamDialog(application);
/*    CreateWavList(application); */
    CreateInputDialog(application);
    CreateFileSB(application);
    CreateWarning(application);
    CreateOops(application);

   /* have a separate application for text window
     so that when the popup dialog kills input to
     menus the text window is still active for cut and paste
     actually popping it up with no grab may have worked also
   */
 
  for(s = 0; s < allspectros.count; s++){
     spectro = allspectros.list[s];
     createwindows(spectro); 
     if(recmode){
	/* set XmNsensitive False all menu buttons except record*/
	/* iconify window 1 + 2*/
	recstart(spectro, 1);
     } /* startup with -r */

  } /* all waveforms */

    /*warning: getting the graphics context must be done after
    the application widget is realized */

   XtRealizeWidget(application);


   /* sgi puts the windows where it wants */
   XMoveWindow(XtDisplay(cmdw),
                     XtWindow(cmdw),
                     cmdox,cmdoy);

    /* incase some other program changed the colormaps */
    /* source for this function in StdCmap.c*/
   XmuAllStandardColormaps(XtDisplay(application) );


    sprintf(tmpstr, "First time users PLEASE READ HELP\n\nWelcome to Xkl v%s updated on %s.\n\n", 
	    VERSION, DATE);
    writetext(tmpstr);
    sprintf(tmpstr, "NEW FEATURES:  track cursor movement by holding down the Alt key;\nset a label by holding down Shift.\n\n");
    writetext(tmpstr);

 for(s = 0; s < allspectros.count; s++){
   spectro = allspectros.list[s];
   mapwindows(spectro);
   writcurpos(spectro);/* time in text window */
 }/* all waveforms */
 
   /* reset title after startup message */
// not any more (goes by too quickly)...
//    n=0;
//    XtSetArg(args[n], XmNtitle,"Xkl");n++;
//    XtSetValues( cmdw,args,n);

#ifdef WACKY_EVENT_LOOP
// avoid this to permit abort of audio-in-progress
    while(True) {

       /* 
	* because there are two applications can't use mainloop()
	* flush and block if no events but, leave event on queue 
	* don't understand why text_app isn't blocked
	* scrollbars didn't work properly with XPeekEvent
	*/

     XtAppPeekEvent(app_context,&pevent);

     if((mask = XtAppPending(app_context)) != 0) {
	if(pevent.type == UnmapNotify) undomap(&pevent);
	else if(pevent.type == MapNotify) domap(&pevent);
	XtAppProcessEvent(app_context, mask);
     }

     if ((mask = XtAppPending(text_app)) != 0)
	XtAppProcessEvent(text_app,mask);
  }
#else
    XtAppMainLoop(app_context);
#endif
}




/***************************************************/
/************* Utility Routines ********************/
/***************************************************/


/*************************************************************************
 *
 * void raiseinput(Widget w,XtPointer client_data,XtPointer call_data)
 *
 * Bring input dialog box to front, just in case it gets lost.
 *
 *************************************************************************/

void raiseinput(Widget w,XtPointer client_data,XtPointer call_data)
{
   if (XtIsManaged(input_dialog))
      XRaiseWindow(XtDisplay(XtParent(input_dialog)),
		   XtWindow(XtParent(input_dialog)));
   if (XtIsManaged(warning))
      XRaiseWindow(XtDisplay(XtParent(warning)),
		   XtWindow(XtParent(warning)) );
}

/*************************************************************************
 *
 *void raisewav(XSPECTRO *spectro)                      
 *
 * Bring all windows associated with the waveform to the front.
 *
 *************************************************************************/

void raisewav(XSPECTRO *spectro)
{
   int i;
   
  for(i = 0; i < NUM_WINDOWS; i++) {
     if (i == GRAM && !spectro->spectrogram) { /* no spectrogram window */ }
     else {
	if(spectro->normstate[i]) { /* not iconified */
	   XSync(XtDisplay(spectro->toplevel[i]),False);
	   XSync(XtDisplay(spectro->toplevel[i]),False);
	   XSync(XtDisplay(spectro->toplevel[i]),False);
	   XSync(XtDisplay(spectro->toplevel[i]),False);
	   XRaiseWindow(XtDisplay(spectro->toplevel[i]),
			XtWindow(spectro->toplevel[i]));
	}
     }
  }
   
  for(i = 0; i < NUM_WINDOWS; i++) {
     if(i == GRAM && !spectro->spectrogram) { /*no spectrogram window*/ }
     else {
	if(spectro->normstate[i]) {        /* not iconified */
	   XSetInputFocus(XtDisplay(spectro->toplevel[i]),
			  XtWindow(spectro->toplevel[i]),
			  RevertToParent, CurrentTime);
	}
     }
  }
   XRaiseWindow(XtDisplay(cmdw), XtWindow(cmdw));
}


/*
  ==========================================================================
  	UpdateCursor  - update cursor location
  ==========================================================================
*/

static void
UpdateCursor(spectro, index, x)
XSPECTRO *spectro;
int index;
int x;
{
	int i;	
	
	spectro->oldindex = spectro->saveindex;
	spectro->oldtime = spectro->savetime;
	findtime(spectro, index, x);

	if (spectro->oldindex != spectro->saveindex) {
		for (i=0; i<NUM_WINDOWS; i++)
			update(spectro->info[i], spectro, i);
	}
	
} /* UpdateCursor */

/*
  ==========================================================================
  	HandleMotion  - button down motion handler
  ==========================================================================
*/

static void
HandleMotion(
	Widget w, 
	XtPointer client_data, 
	XMotionEvent *mEvent,
	Boolean *continueToDispatch)
{ 
	XSPECTRO *spectro;
	int x;		 
	
	spectro = (XSPECTRO *) client_data;	
	UpdateCursor(spectro, spectro->clickedWinIndex, mEvent->x);
	
} /* HandleMotion */


/*
  ==========================================================================
  	dobutton  - window button click handler

    Current handling:
      Unmodified left - set position immediately
      Unmodified middle - squawk file
      Unmodified right - raise all windows associated with this waveform
      Shift left - set label using dialog for naming
      Shift middle - squawk w->e
      Shift right - set (unnamed) label immediately
      Alt left - track cursor while down, set position on release
      Alt middle - track cursor while down, play from mouseDn to mouseUp on release
      Alt right - track cursor, set position, raise windows on release
      Alt right is trapped by standard fvwm95 (push window behind)
      Shift+Alt left - track cursor, set label w/dialog on release
      Shift+Alt middle - (same as Alt middle)
      Shift+Alt right - track cursor, set unnamed label on release
    Click modifiers are ignored in spectrum window
  ==========================================================================
*/

void 
dobutton(
	Widget w, 
	XtPointer client_data, 
	XtPointer call_data)
{ 
	XSPECTRO *spectro;
	int x, i, j, index;
	char str[300], tmp[100];
	XmDrawingAreaCallbackStruct *c_data;
	void InitLabel(), MakeLabel();		// xkl_label.c
	
	c_data = (XmDrawingAreaCallbackStruct *)call_data;
	
	spectro = (XSPECTRO *) client_data;	
	if (!spectro->iwave) return;
   
// which window was clicked
	for (index=0; index<NUM_WINDOWS; index++)
		if (spectro->draww[index] == w)  break;
	x = c_data->event->xbutton.x;       // clicked location

// handle spectrum window clicks (modifiers ignored)
	if (index==SPECTRUM) {
	    if (c_data->event->xbutton.type == ButtonPress) {	// ignore all but mouseDn events
		if (spectro->spectrum) {		// if valid			
			if (findspecindex(spectro, x)) {	// click within range
				spec_cursor(spectro);
				sprintf(str,"\n%s spectrum:\t%d Hz\t%.1f dB",
						spectro->wavename, spectro->specfreq, spectro->specamp);
				if (spectro->savspecamp != -1.0) {	// valid savfltr
					sprintf(tmp,"\nprevious time:\t%d Hz\t%.1f dB",
							spectro->savspecfreq,spectro->savspecamp);
					strcat(str,tmp);
				}
				strcat(str,"\n");
				writetext(str);
			}
		}
		switch (c_data->event->xbutton.button) {
			case 1:		// left button:    done
				break;
			case 2:		// middle button:  squawk (from mouseDn to mouseUp; full selection if same)
				spectro->segmode = 0;
				xklPlay(spectro, index);
				break;
			case 3:		// right button:   raise all windows associated with this waveform
				raisewav(spectro);
				XSetInputFocus(XtDisplay(spectro->toplevel[index]), XtWindow(spectro->toplevel[index]), RevertToNone, CurrentTime);
				break;
		}
	    }
	    return;
	}

// all others:  if alt key down track pointer motion until button release
	switch (c_data->event->xbutton.type) {
		case ButtonPress:
			UpdateCursor(spectro, index, x);
			spectro->clickedWinIndex = index;
			spectro->clickedWinTime = spectro->savetime;
			if (c_data->event->xbutton.state & Mod1Mask) {		// alt key down -- track pointer motion
			    XtAddEventHandler(w, ButtonMotionMask, FALSE, (XtEventHandler)HandleMotion, client_data);
			    break;
			} // else fall through
		case ButtonRelease:
		    if (c_data->event->xbutton.state & Mod1Mask) {
				XtRemoveEventHandler(w, ButtonMotionMask, FALSE, (XtEventHandler)HandleMotion, client_data);
				UpdateCursor(spectro, index, x);
			} else if (c_data->event->xbutton.type == ButtonRelease) {  // ignore release if not tracking motion
			    break;
			}
			if (spectro->localtimemax) {		// set offset to local min/max (Erika)
				findminmaxtime(spectro, !(c_data->event->xbutton.state & ShiftMask));
				for (i=0; i<NUM_WINDOWS; i++)
					update(spectro->info[i], spectro, i);
			}
			if (spectro->locked) {				// synchronized waveforms
				CalculateOffset(spectro->timeoffset);				
				for (i=0; i<allspectros.count; i++) {
					if (allspectros.list[i]->locked) {
						allspectros.list[i]->oldindex = allspectros.list[i]->saveindex;
						allspectros.list[i]->oldtime = allspectros.list[i]->savetime;
						allspectros.list[i]->savetime = spectro->savetime + allspectros.list[i]->timeoffset;         
						allspectros.list[i]->saveindex =  (allspectros.list[i]->savetime  * allspectros.list[i]->spers / 1000.0) + .5;
						validindex(allspectros.list[i]); 
						if (allspectros.list[i]->oldindex != allspectros.list[i]->saveindex) {
							for (j=0; j<NUM_WINDOWS; j++)
								update(allspectros.list[i]->info[j],allspectros.list[i],j);
							writcurpos(allspectros.list[i]);
						}
					}
				}
			} else {
				writcurpos(spectro);
			}
			switch (c_data->event->xbutton.button) {
				case 1:		// left button:    done
					if (!spectro->locked && (c_data->event->xbutton.state & ShiftMask))	// shift button down - label dialog
						InitLabel(application, spectro, -1);
					break;
				case 2:		// middle button:  squawk (from mouseDn to mouseUp; full selection if same)
					spectro->segmode = (spectro->clickedWinTime == spectro->savetime ? 0 : 2);
					xklPlay(spectro, index);
					break;
				case 3:		// right button: shift button down - make label immediately 
					if (!spectro->locked && (c_data->event->xbutton.state & ShiftMask)) {
						MakeLabel(spectro);
					} else {	// raise all windows associated with this waveform
						raisewav(spectro);
						XSetInputFocus(XtDisplay(spectro->toplevel[index]), XtWindow(spectro->toplevel[index]), RevertToNone, CurrentTime);
					}
					break;

			}
			break;
		default:
			break;
	}
	
} /* dobutton */


/**********************************************************************

********************   Menu Callback Procedures ***********************
 
**********************************************************************/



/**********************************************************************/
/*void HandleFileMenu(Widget w,XtPointer client_data,XtPointer call_data)*/
/**********************************************************************/

void HandleFileMenu(Widget w,XtPointer client_data, XtPointer call_data){

  int window, entry, offset;
  Widget *buttons;
  XSPECTRO *spectro;
  char str[300],mess[330],temp[200];
  Arg args[5];
  int n;
  XmString mstr;
  XmRowColumnCallbackStruct *c_data;
  char *new_filter;
  char extension[10];
  void LoadLabels(), SaveLabels(), KillLabels(), EditLabels(), InitLabel();	// xkl_label.c

  c_data = (XmRowColumnCallbackStruct *)call_data;
  spectro = (XSPECTRO *) client_data;

/*
 * Establish which pulldown menu
 */

  for (window = 0; window < numPulldown; window++)
    if (spectro->menu_bar[window] == XtParent(XtParent(w))) break;

 buttons = spectro->buttons[window];

/*
 * Find out offset based on num_entries globally defined array./
 * Offset in button array is total of previous num_entries in entry array.
 */

 offset = 0;
 for(entry = 0; entry < num_entries[0]; entry++)
   if (c_data->widget == buttons[offset + entry]) break;

 /* FILE 

0    "r Read new .wav file",
1    "v Replace current .wav file",
2    "D Close .wav file",
3    "Z Read spectrum file",
4    "z Write spectrum file",
5    "M Save parameter file",
6    "m Write parameter file",
7    "o Output w->e to .wav file",
8    "O Output w->e to ASCII file",
9    "",
10   "y Read Label file",
11   "x Delete labels",
12   "X Edit labels",
13   "9 Make label @ cursor",
14   "I Save labels", 
15    "",
16   "G Open/close 1 spectra/pg PS file",
17   "* Select spectrum (G)",
18   "",
19   "g Open/close 4 spectra/pg PS file",
20   "# Select spectrum (g)",
21   "",
22  "N List freq. & BW in window",
23  "V List spectrum values in window",
24   "H Open/close amp. & freq. file",
25   "h Select amp. & freq. (H)",
26   "",
27   "n Select current waveform",
28   "+ Forward one waveform",
29   "- Backward one waveform",
30   "",
31   "q Quit",
   */

 switch(entry) {

/** Callbacks: newwave, cancelfilesb  **/
 case 0:  
   /* r Read new .wav file*/
   XtAddCallback(filesb_dialog, XmNokCallback, newwave, (XtPointer)spectro);
   XtAddCallback(filesb_dialog, XmNcancelCallback, cancelfilesb, 
		 (XtPointer)spectro);

   /* make sure the file selection box is looking in the last directory
      you looked in */
   mstr = XmStringCreateLocalized("*.wav");
   XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(current_dir, mstr));
   XmStringFree(mstr);

   n=0;
   XtSetArg(args[n], XmNtitle, "New Waveform"); n++;
   XtSetValues(XtParent(filesb_dialog), args, n);
   opendialog(filesb_dialog);
   break;

/** Callbacks: swapwave, cancelfilesb  **/
 case 1:
  /* v Replave current .wav file */
   
   XtAddCallback(filesb_dialog,XmNokCallback,swapwave,(XtPointer)spectro);
   XtAddCallback(filesb_dialog,XmNcancelCallback,
		 cancelfilesb,(XtPointer)spectro);

   /* Remember last directory */
   mstr = XmStringCreateLocalized("*.wav");
   XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(current_dir, mstr));
   XmStringFree(mstr);

   n=0;
   XtSetArg(args[n], XmNtitle, "Replace Waveform"); n++;
   XtSetValues(XtParent(filesb_dialog), args, n);
   opendialog(filesb_dialog);
   break;

/** Callbacks: none  **/
 case 2:
  /* D Close .wav file */
   delete_spectro(spectro);
   break;

/** Callbacks: readspectrum, cancelfilesb  **/
 case 3:
   /* Z Read spectrum file */ 
   XtAddCallback(filesb_dialog, XmNokCallback, readspectrum,
		 (XtPointer)spectro);
   XtAddCallback(filesb_dialog, XmNcancelCallback, cancelfilesb,
		 (XtPointer)spectro);

   /* Remember last directory */
   mstr = XmStringCreateLocalized("*.spectrum");
   XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(current_dir, mstr));
   XmStringFree(mstr);
   n=0;
   XtSetArg(args[n], XmNtitle, spectro->wavename); n++;
   XtSetValues(XtParent(filesb_dialog), args, n);
   opendialog(filesb_dialog);
   break;
   
/** Callbacks: writespectrum, cancelinput  **/
 case 4:  /* z Write spectrum file */
     //   strcpy(str, spectro->wavename);
     //   sprintf(temp,"%d",(int)spectro->savetime);
     //   strcat(str, temp);

    sprintf(str, "%s%d", spectro->wavename, (int)spectro->savetime);
    mstr = XmStringCreateLocalized(str);
//    XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(save_dir, mstr));
    XmFileSelectionDoSearch(filesb_dialog, mstr);
    XmStringFree(mstr);

    n=0;
    XtSetArg(args[n], XmNtitle, "Save spectrum file as"); n++;
    XtSetValues(XtParent(filesb_dialog), args, n);
    XtAddCallback(filesb_dialog,XmNokCallback,writespectrum,(XtPointer)spectro);
    XtAddCallback(filesb_dialog,XmNcancelCallback,cancelfilesb,(XtPointer)spectro);
    opendialog(filesb_dialog);

#ifdef STUB
   XtSetArg(args[0], XmNvalue, str);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized("Filename: will append .spectrum");
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   XtAddCallback(input_dialog,XmNokCallback,writespectrum,
		 (XtPointer) spectro);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,
		 (XtPointer) spectro);
   opendialog(input_dialog);
#endif
   break;
   
/** Callbacks: readparams, cancelfilesb  **/
 case 5: /* M Read parameter file */
   XtAddCallback(filesb_dialog,XmNokCallback,readparams,
		 (XtPointer) spectro);
   XtAddCallback(filesb_dialog,XmNcancelCallback,cancelfilesb,
		 (XtPointer) spectro);
   
   mstr = XmStringCreateLocalized("*.param");
   XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(current_dir, mstr));
   XmStringFree(mstr);

   n=0;
   XtSetArg(args[n],XmNtitle,"Read .param File"); n++;
   XtSetValues(XtParent(filesb_dialog),args,n);
   opendialog(filesb_dialog);
   break;
   
/** Callbacks: writeparams, cancelinput  **/
   case 6:
  /* m Write parameters file */

    mstr = XmStringCreateLocalized(spectro->wavename);
//    XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(save_dir, mstr));
    XmFileSelectionDoSearch(filesb_dialog, mstr);
    XmStringFree(mstr);

    n=0;
    XtSetArg(args[n], XmNtitle, "Save params file as"); n++;
    XtSetValues(XtParent(filesb_dialog), args, n);
    XtAddCallback(filesb_dialog,XmNokCallback,writeparams,(XtPointer)spectro);
    XtAddCallback(filesb_dialog,XmNcancelCallback,cancelfilesb,(XtPointer)spectro);
    opendialog(filesb_dialog);

#ifdef STUB
   XtSetArg(args[0],XmNvalue,spectro->wavename);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized("Filename (will append .params)");
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   n=0;
   XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
   XtSetValues(XtParent(input_dialog),args,n);
   XtAddCallback(input_dialog,XmNokCallback,writeparams,
		 (XtPointer) spectro);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,
		 (XtPointer) spectro);
   opendialog(input_dialog);
#endif
   break;

/** Callbacks: segtowav, cancelinput  **/
   case 7:
   /* o Output w->e to .wav file */
   if(spectro->endmarker > spectro->startmarker){
    spectro->sampsplay = spectro->endmarker - spectro->startmarker + 1;
    spectro->startplay = &spectro->iwave[spectro->startmarker];
    mstr = XmStringCreateLocalized("*.wav");
//    XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(save_dir, mstr));
    XmFileSelectionDoSearch(filesb_dialog, mstr);
    XmStringFree(mstr);

    n=0;
    XtSetArg(args[n], XmNtitle, "Save .wav file as"); n++;
    XtSetValues(XtParent(filesb_dialog), args, n);
    XtAddCallback(filesb_dialog,XmNokCallback,segtowav,(XtPointer)spectro);
    XtAddCallback(filesb_dialog,XmNcancelCallback,cancelfilesb,(XtPointer)spectro);
    spectro->wwav = 1;
    opendialog(filesb_dialog);

#ifdef STUB
   XtSetArg(args[0],XmNvalue,spectro->wavename);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized("Filename (will append .wav)");
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   n=0;
   XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
   XtSetValues(XtParent(input_dialog),args,n);
   XtAddCallback(input_dialog,XmNokCallback,segtowav,(XtPointer)spectro);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,(XtPointer)spectro);
   spectro->wwav = 1;
   opendialog(input_dialog);
#endif
    }/* good segment */
   else{
   writetext("Beginning marker (w) => End marker (e).\n");
   }
  break;

/** Callbacks: writedata, cancelinput  **/
 case 8:
   /* o Output w->e to ASCII file*/
   /*write portion of wave that would be played*/
   if(spectro->endmarker > spectro->startmarker){
    spectro->sampsplay = spectro->endmarker - spectro->startmarker + 1;
    spectro->startplay = &spectro->iwave[spectro->startmarker]; 
    sprintf(str,"%s.data",spectro->wavename);
    mstr = XmStringCreateLocalized(str);
//    XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(save_dir, mstr));
    XmFileSelectionDoSearch(filesb_dialog, mstr);
    XmStringFree(mstr);

    n=0;
    XtSetArg(args[n], XmNtitle, "Save data file as"); n++;
    XtSetValues(XtParent(filesb_dialog), args, n);
    XtAddCallback(filesb_dialog,XmNokCallback,writedata,(XtPointer)spectro);
    XtAddCallback(filesb_dialog,XmNcancelCallback,cancelfilesb,(XtPointer)spectro);
    opendialog(filesb_dialog);

#ifdef STUB
   n = 0;
   XtSetArg(args[n],XmNvalue,str); n++;
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,n);
   mstr = XmStringCreateLocalized("name of data file:");
   n = 0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   n = 0;
   XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
   XtSetValues(XtParent(input_dialog),args,n);
   XtAddCallback(input_dialog,XmNokCallback,writedata,(XtPointer)spectro);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,(XtPointer)spectro);
   opendialog(input_dialog);
#endif
   }/* good segment */
   else{
   writetext("Beginning marker (w) => End marker (e).\n");
   }
   break;

/** Callbacks: none  **/
 case 9:
   /* Separator */
   break;

/** Callbacks: LoadLabels, cancelfilesb  **/
 case 10:
   /* y Read Label file  **/
     XtAddCallback(filesb_dialog, XmNokCallback, LoadLabels, (XtPointer)spectro);
     XtAddCallback(filesb_dialog, XmNcancelCallback, cancelfilesb, (XtPointer)spectro);
     mstr = XmStringCreateLocalized("*.l*");
     XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(current_dir, mstr));
     XmStringFree(mstr);
     n=0;
     XtSetArg(args[n],XmNtitle,"Read label File"); n++;
     XtSetValues(XtParent(filesb_dialog),args,n);
     opendialog(filesb_dialog);
     break;
   
/** Callbacks:  KillLabels, cancelwarning  **/
 case 11:
   /* x delete labels */
     if (spectro->labels) {
	 sprintf(mess,"Delete All Labels?");
	 mstr = XmStringCreateLocalized(mess);
	 n = 0;
	 XtSetArg(args[n], XmNmessageString, mstr); n++;
	 XtSetArg(args[n], XmNokLabelString, XmStringCreateLocalized("Yes")); n++;
	 XtSetArg(args[n], XmNcancelLabelString, XmStringCreateLocalized("Cancel")); n++;
	 XtSetValues(warning, args, n);
	 XmStringFree(mstr);
	 XtAddCallback(warning, XmNokCallback, KillLabels, spectro);
	 XtAddCallback(warning, XmNcancelCallback, cancelwarning, NULL);
	 opendialog(warning);
     }

/*
     while (spectro->labels) {
	 	lp = spectro->labels;
	 	spectro->labels = lp->next;
	 	free(lp);
     }
     for(n=0; n<3; n++)
	 	draw_cursor(spectro, n, spectro->info[n]);
*/
     break;
   
/** Callbacks:  none  **/
 case 12:
   /* X edit labels */
     if (spectro->labels)
	 EditLabels(application, spectro);
     break;
   
/** Callbacks:  none  **/
 case 13:
   /* 9 make label @ cursor */
     InitLabel(application, spectro, -1);
     break;
   
/** Callbacks:  SaveLabels, cancelfilesb  **/
 case 14:
    /* I save labels */
     XtAddCallback(filesb_dialog, XmNokCallback, SaveLabels, (XtPointer)spectro);
     XtAddCallback(filesb_dialog, XmNcancelCallback, cancelfilesb, (XtPointer)spectro);
     mstr = XmStringCreateLocalized("*.lbl");
     XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(current_dir, mstr));
     XmStringFree(mstr);
     n=0;
     XtSetArg(args[n],XmNtitle,"Save label File"); n++;
     XtSetValues(XtParent(filesb_dialog),args,n);
     opendialog(filesb_dialog);
     break;
   
/** Callbacks: none  **/
 case 15:
   /* Separator */
   break;

/** Callbacks: dopsfile, cancelfilesb  **/
 case 16:
   /* G Open/close 1 spectra/ window to ps */
 if(window == SPECTRUM && !spectro->spectrum && !spectro->savspectrum){
    writetext("No valid spectrum.\n");
 }
 else {
   if (PSfull_fp == NULL) { /* open up a ps file */ 
     spectro->ps_window =  -1;
     PStype = FULLPAGE;
     XtAddCallback(filesb_dialog,XmNokCallback,dopsfile,
		   (XtPointer) spectro);
     XtAddCallback(filesb_dialog,XmNcancelCallback,cancelfilesb,
		   (XtPointer) spectro);

     mstr = XmStringCreateLocalized("*.ps");
     XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(current_dir, mstr));
     XmStringFree(mstr);

     n = 0;
     XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
     XtSetValues(XtParent(filesb_dialog),args,n);
     opendialog(filesb_dialog);
   }
   else {
     /* going to close but ask first */
     writetext("PS file already open.\n");
     sprintf(mess,"CLOSE FULL PAGE POSTSCRIPT FILE?");
     mstr = XmStringCreateLocalized(mess);
     n = 0;
     XtSetArg(args[n], XmNmessageString, mstr); n++;
     XtSetValues(warning, args, n);
     XmStringFree(mstr);
     mstr = XmStringCreateLocalized("OK");
     XtSetArg(args[0], XmNokLabelString, mstr);
     XtSetValues(warning, args, 1);
     XtRemoveAllCallbacks(input_dialog, XmNokCallback);
     XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);  
     XtAddCallback(warning, XmNokCallback, closefullps, (XtPointer) spectro);
     XtAddCallback(warning, XmNcancelCallback, cancelwarning, NULL);
     opendialog(warning);
     }/* file was open ask if it should be closed */
 }
 break;

/** Callbacks: none  **/
 case 17:
   spectro->ps_window = window;
   if (PSfull_fp) writeps(spectro, PSfull_fp);
   break;

/** Callbacks: none  **/
 case 18: 
   break;

/** Callbacks: dopsfile/close4ps, cancelfilesb  **/
 case 19: /* * Select spectrum (G) */
   /*g Open/close 4 spectra/pg PS file*/
  if(window == SPECTRUM && !spectro->spectrum && !spectro->savspectrum){
   writetext("No valid spectrum.\n");
  }
  else{
     if(PS4_fp == NULL) {
       /*going to open a ps file*/

       PStype = FOURPAGE;
       spectro->ps_window =  -1;
       XtAddCallback(filesb_dialog,XmNokCallback,dopsfile,
		     (XtPointer) spectro);
       XtAddCallback(filesb_dialog,XmNcancelCallback,cancelfilesb,
		     (XtPointer) spectro); 

       mstr = XmStringCreateLocalized("*.ps");
       XmFileSelectionDoSearch(filesb_dialog, XmStringConcat(current_dir, mstr));
       XmStringFree(mstr);

       n = 0;
       XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
       XtSetValues(XtParent(filesb_dialog),args,n); 
       opendialog(filesb_dialog);
     }/* check first then open a file */
   else{
     /* going to close but ask first */
     writetext("PS file already open.\n");
     sprintf(mess,"CLOSE 4/page POSTSCRIPT FILE?");
     mstr = XmStringCreateLocalized(mess);
     n = 0;
     XtSetArg(args[n], XmNmessageString,mstr); n++;
     XtSetValues(warning,args,n);
     XmStringFree(mstr);
     mstr = XmStringCreateLocalized("OK");
     XtSetArg(args[0], XmNokLabelString,mstr);
     XtSetValues(warning,args,1);
     XtRemoveAllCallbacks(input_dialog, XmNokCallback);
     XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
     XtAddCallback(warning,XmNokCallback, close4ps, (XtPointer) spectro);
     XtAddCallback(warning,XmNcancelCallback,cancelwarning,NULL);
     opendialog(warning);
     }/* file was open ask if it should be closed */
   }/* good spectrum */

   break;

/** Callbacks: fourspectra  **/
 case 20: /* # Select spectrum (g) */
   if (PS4_fp) fourspectra(spectro, PS4_fp);
   break;

/** Callbacks: none  **/
 case 21: /* separator */
   break;

/** Callbacks: printfilters  **/
 case 22: /*N List freq & BW in window for 's','c','S','T' */

  if(spectro->option == 's' || spectro->option == 'S'||
     spectro->option == 'c' || spectro->option == 'T'||
     spectro->option == 'j') {

    if (!XtIsManaged(XtParent(XtParent(spectro->fbw_text))))
      XtPopup(XtParent(XtParent(spectro->fbw_text)),XtGrabNone);
    printfilters(spectro);
  }
  else writetext("Not applicable in this mode.");
  break;

/** Callbacks: showvalues  **/
 case 23: /* V List spectrum values in window */
   if (spectro->spectrum) {
     if(!XtIsManaged(XtParent(XtParent(spectro->fbw_text))))
       XtPopup(XtParent(XtParent(spectro->fbw_text)),XtGrabNone);
     showvalues(spectro);
   }
   else writetext("No spectrum data available.\n");
   break;

/** Callbacks: openjnl/closejnl, closeinput  **/
 case 24: /*H Open/close amp. & freq. file */

   /* open a journal file for storing waveform time and sample values and 
      spectrum freq + amp values when X is type current values will be 
      written to file
   */

     if(D_fp == NULL){
      sprintf(str,"%s.jnl",spectro->wavename);
      n = 0;
      XtSetArg(args[n],XmNvalue,str); n++;
       XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,n);
       mstr = XmStringCreateLocalized("name of data file:");
       n = 0;
       XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
       XtSetValues(input_dialog,args,n);
       XmStringFree(mstr);
       n = 0;
       XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
       XtSetValues(XtParent(input_dialog),args,n);
       XtAddCallback(input_dialog,XmNokCallback,openjnl,(XtPointer)spectro);
       XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,
	   (XtPointer)spectro);
       opendialog(input_dialog);

     }/* check first then open a file */

   else{
     /* going to close but ask first */
     writetext("Journal file already open.\n");
     sprintf(mess,"CLOSE JOURNAL FILE %s??",str);
     mstr = XmStringCreateLocalized(mess);
     n = 0;
     XtSetArg(args[n], XmNmessageString,mstr); n++;
     XtSetValues(warning,args,n);
     XmStringFree(mstr);
     mstr = XmStringCreateLocalized("OK");
     XtSetArg(args[0], XmNokLabelString,mstr);
     XtSetValues(warning,args,1);
     XtRemoveAllCallbacks(input_dialog, XmNokCallback);
     XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);  
     XtAddCallback(warning,XmNokCallback,closejnl,NULL);
     XtAddCallback(warning,XmNcancelCallback,cancelwarning,NULL);
     opendialog(warning);
     }/* file was open ask if it should be closed */
   break;

/** Callbacks: none  **/
 case 25:
   /*h Select amp. & freq. (H) */

   if(D_fp){

   fprintf(D_fp,"%16s%8c%8d%8.1f%8d%8.1f%12.1f%10d\n",
   spectro->wavename,spectro->option,
   spectro->specfreq,spectro->specamp,
   spectro->savspecfreq,spectro->savspecamp,
   spectro->savetime,
   (int)(spectro->iwave[spectro->saveindex] 
		 / 32767.0 * 9997.56 )	   );

   writetext("Wrote values to journal file.\n");
   fflush(D_fp);
   }/* file is open*/
   else{
   writetext("No journal file open.\n");
   }

  break;

/** Callbacks: none  **/
 case 26:
   /* separator */
   break;

/** Callbacks: dowav  **/
 case 27:
  /* n Select current waveform */
 /* pop windows associated with waveform */
 if (allspectros.count < 2) {  /* only one wave */
    break;
 }
 else if (allspectros.count == 2) {
    shiftCurrentWav(spectro, 1);
 }
 else {
   CreateWavList(application);
/*    dowavlist(); */

   XtAddCallback(wav_sb, XmNokCallback,dowav,(XtPointer)spectro);
   XtAddCallback(wav_menu,XmNentryCallback,HandleWavlistMenu,(XtPointer)spectro);
    open_tl_dialog(wav_tl);
 }
 break;

/** Callbacks: shiftCurrentWav  **/
 case 28:
  /* + Forward one waveform */
 shiftCurrentWav(spectro, 1);
 break;

/** Callbacks: shiftCurrentWav  **/
 case 29:
  /* - Backward	 one waveform */
 shiftCurrentWav(spectro, -1);
 break;

/** Callbacks: none  **/
 case 30:
   /* separator */
   break;

/** Callbacks: exitwarning  **/
  case 31:
  /* q Quit */
  /* close postscript files and exit*/

  exitwarning();
  break;

 }/* end switch */
  
}/* end HandleFileMenu */


/*********************************************************************
 *
 * void setfilesb(XSPECTRO *spectro)
 *
 * Set the file selection box based on *.wav.  Use if you don't want 
 * filter applied otherwise use XmFileSelectionDoSearch(w, dirmask)
 *
 *********************************************************************/
void setfilesb(XSPECTRO *spectro)
{
   char str[300];
   Arg args[3];

   strcpy(str,"*.wav");
   XtSetArg(args[0],XmNvalue,str);
   XtSetValues(XmFileSelectionBoxGetChild(filesb_dialog, XmDIALOG_FILTER_TEXT),
	       args,1);
   sprintf(str,"%s.wav",spectro->wavename);
   XtSetArg(args[0],XmNvalue,str);
   XtSetValues(XmFileSelectionBoxGetChild(filesb_dialog,XmDIALOG_TEXT),args,1);
}

/***************** Text Window Routines *********************/

/********************************************************************
 *
 * void writetext(char *str)                                
 * 
 * Write text, str, to the text window.  No '\n' is used.
 *
 ********************************************************************/

void writetext(char *str)
{
   int n;
   Arg args[1];

   XmTextInsert(cmdtextw, XmTextGetLastPosition(cmdtextw), str); 
   n = 0;
   XtSetArg(args[n], XmNcursorPosition, 
	    XmTextGetLastPosition(cmdtextw)); n++;
   XtSetValues(cmdtextw, args, n);
}


/*****************************************************************
 *
 * void  writewe( XSPECTRO *spectro)			 
 *
 * Write w->e duration to text window.
 *
 *****************************************************************/

void writewe(XSPECTRO *spectro) 
{
   float ms;
   char str[220];

   if (spectro->iwave == NULL) { return; }
   if (spectro->endmarker > spectro->startmarker) {
      ms = (float)(spectro->endmarker - spectro->startmarker) / 
	 spectro->spers * 1000.0;
      sprintf(str, "%s: w->e = %.1fms.\n", spectro->wavename, ms);
   }
   else sprintf(str, "%s: e <= w",spectro->wavename);
   writetext(str);
}


/*****************************************************************
 *
 * void  writecurpos(XSPECTRO *spectro)			 
 *
 * Write current cursor position to text window.
 *
 *****************************************************************/

void writcurpos(XSPECTRO *spectro)
{
   char str[550];

   if (spectro->iwave == NULL) return;  /* no waveform displayed */
   sprintf(str,"%s:\t%.1fms\t%d\n", spectro->wavename, spectro->savetime,
	   (int)(spectro->iwave[spectro->saveindex] / 32767.0 * 9997.56 ));
   writetext(str);
}



/**********************************************************************/
/*voidHandleTimeMenu(Widget w,XtPointer client_data,XtPointer call_data)*/
/**********************************************************************/

void HandleTimeMenu(Widget w,XtPointer client_data, XtPointer call_data){

/* use num_entries to find start of button array */
/* use w to find which window the call was made from*/

  int window, entry, offset, i,n;
  Widget *buttons;
  XSPECTRO *spectro;
  Arg args[5];
  char str[200];
  XmString mstr;
  XmRowColumnCallbackStruct *cbs;

  cbs = (XmRowColumnCallbackStruct *) call_data;
  spectro = (XSPECTRO *) client_data;

/* 
 * Establish which pulldown menu window
 */

 for(window = 0; window < numPulldown; window++)
   if(spectro->menu_bar[window] == XtParent(XtParent(w))) break;

 buttons = spectro->buttons[window];


 /* find out offset based on num_entries globally defined array*/
 /* offset in button array is total of previous num_entries in entry array*/

 offset = num_entries[0];
 for(entry = 0; entry < num_entries[1]; entry++)
   if(cbs->widget == buttons[offset + entry]) break;

   /* TIME MENU

0    "b Move back 10 ms",
1    "f Move forward 10 ms",
2    "B Move back 50 ms",
3    "F Move forward 50 ms",
4    "!AL Step right by window 1 width",
5    "!AR Step left by window 1 width",
6    "",
7    "t Move marker to time t",
8    "w Set start of selected region",
9    "e Set end of selected region",
10   "W Set w to beginning of file",
11   "E Set e to end of file",
12   "^ Set both W and E",
13   "1 Move marker to w",
14   "2 Move marker to e",
15   "J Show w->e in window 1",
16   "",                                  
17   "@ Peak/valley picking on/off",
18   "",
19   "!AU Zoom in (window 0 & 1 only)",
20   "!AD Zoom out (window 0 & 1 only)",
21   "",
22   "L Lock cursor arrows",
23   "U Unlock cursor arrows",	  

*/

 switch(entry) {

/** Callbacks: none **/
 case 0:
   /*b Move back 10 ms*/
   if(spectro->locked){
     for( i = 0; i < allspectros.count; i++)
       if (allspectros.list[i]->locked) {
       timestep(allspectros.list[i], -10);
     }/* all waveforms */
   }/* locked do all other waveforms too*/
   else
     timestep(spectro, -10);  /* just do current */
   break;

/** Callbacks: none **/
 case 1:
   /*f Move forward 10 ms*/
   if(spectro->locked){
     for( i = 0; i < allspectros.count; i++)
       if (allspectros.list[i]->locked) {
       timestep(allspectros.list[i], 10);
     }/* all waveforms */
   }/* locked do all other waveforms too*/
   else
     timestep(spectro, 10);
   break;

/** Callbacks: none **/
 case 2:
   /* B move back 50 ms*/
   if(spectro->locked){
     for( i = 0; i < allspectros.count; i++)
       if (allspectros.list[i]->locked) {
       timestep(allspectros.list[i], -50);
     }/* all waveforms */
   }/* locked do all other waveforms too*/
   else
     timestep(spectro, -50);
   break;

/** Callbacks: none **/
 case 3:
   /*F move forward 50 ms*/
   if(spectro->locked){
     for( i = 0; i < allspectros.count; i++)
       if (allspectros.list[i]->locked)
	 {
	   timestep(allspectros.list[i], 50);
	 }/* all waveforms */
   }/* locked do all other waveforms too*/
   else
     timestep(spectro, 50);
   break;

/** Callbacks: stepleft **/
 case 4:
   /*left arrow step left*/
   if(spectro->locked){
   for( i = 0; i < allspectros.count; i++)
     if (allspectros.list[i]->locked)
{
     if(allspectros.list[i] != spectro){
       stepleft(allspectros.list[i],window);
       }/* save the active spectro until last*/
     }/* all waveforms */
   }/* locked do all other waveforms too*/
   stepleft(spectro,window);

   break;

/** Callbacks: stepright **/
 case 5:
   /*right arrow step right*/
   if(spectro->locked)
     {
       for( i = 0; i < allspectros.count; i++)
	 if (allspectros.list[i]->locked)
	   {
	     if(allspectros.list[i] != spectro){
	       stepright(allspectros.list[i],window);
	 }/* save the active spectro until last*/
	   } /* all waveforms */
     }/* locked do all other waveforms too*/
   stepright(spectro,window);
   break;

/** Callbacks: none **/
 case 6:
   /* separator */
   break;

/** Callbacks: settime, cancelinput **/
 case 7:
   /*t Mover marker to time t */   
   sprintf(str,"%.1f",spectro->savetime);
   XtSetArg(args[0],XmNvalue,str);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);

   if (lock)
     mstr = XmStringCreateLocalized("set time for all locked waveforms:");
   else
     mstr = XmStringCreateLocalized("set time:");
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   if (spectro->locked)
     XtAddCallback(input_dialog,XmNokCallback, settime_lock, NULL);
   else
     XtAddCallback(input_dialog,XmNokCallback,settime,(XtPointer)spectro);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,(XtPointer)spectro);

   CalculateOffset(spectro->timeoffset);

   opendialog(input_dialog);

   /* Highlight the default text */
   HighlightInput (str, XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT));
   writetext("Waiting for input...\n");
  break;

/** Callbacks: draw_cursor **/
 case 8:
   /*w Set start of selected region*/
   spectro->oldstart = spectro->startmarker;
   spectro->startmarker = spectro->saveindex;

   if( spectro->oldstart != spectro->startmarker){
	for(i = 0; i < 3; i++){
	  draw_cursor(spectro,i,spectro->info[i]);
	 }
    writewe(spectro);
     }/* new start*/
   break;

/** Callbacks: draw_cursor **/
 case 9:
   /*e Set end of selected region */
   spectro->oldend = spectro->endmarker;
   spectro->endmarker = spectro->saveindex;
   if( spectro->oldend != spectro->endmarker){
       for(i = 0; i < 3; i++){
	  draw_cursor(spectro,i,spectro->info[i]);
	}
    writewe(spectro);
     }/* new end*/
   break;

/** Callbacks: draw_cursor **/
 case 10:
   /* "W Set w to beginning of file" */
   spectro->oldstart = spectro->startmarker;
   spectro->startmarker = 0;

   if( spectro->oldstart != spectro->startmarker){
	for(i = 0; i < 3; i++){
	  draw_cursor(spectro,i,spectro->info[i]);
	 }
    writewe(spectro);
     }/* new start*/
   break;

/** Callbacks: draw_cursor **/
 case 11:
   /* "E Set e to end of file" */
   spectro->oldend = spectro->endmarker;
   spectro->endmarker = spectro->totsamp;
   if( spectro->oldend != spectro->endmarker){
       for(i = 0; i < 3; i++){
	  draw_cursor(spectro,i,spectro->info[i]);
	}
    writewe(spectro);
     }/* new end*/
   break;

/** Callbacks: draw_cursor **/
 case 12:
   /* "^ Sets both W and E" */
   spectro->oldstart = spectro->startmarker;
   spectro->startmarker = 0;
   spectro->oldend = spectro->endmarker;
   spectro->endmarker = spectro->totsamp;

   if((spectro->oldstart != spectro->startmarker) || 
      (spectro->oldend != spectro->endmarker)) {
	for(i = 0; i < 3; i++){
	  draw_cursor(spectro,i,spectro->info[i]);
	 }
    writewe(spectro);
     }
   break;

/** Callbacks: update **/
 case 13:
   /*1 Move marker to w */
   spectro->oldindex = spectro->saveindex;
   spectro->oldtime = spectro->savetime;
   spectro->saveindex = spectro->startmarker;
   spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000;
   if( spectro->oldindex != spectro->saveindex){
       for(i = 0; i < NUM_WINDOWS; i++){
	   update(spectro->info[i], spectro, i);
	 }
     writcurpos(spectro);/* time in text window */
     }/* new index*/
   break;

/** Callbacks: update **/
 case 14:
   /*2 Move marker to e */
   spectro->oldindex = spectro->saveindex;
   spectro->oldtime = spectro->savetime;
   spectro->saveindex = spectro->endmarker;
   spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000.0;
   if( spectro->oldindex != spectro->saveindex){
	for(i = 0; i < NUM_WINDOWS; i++){
	  update(spectro->info[i],spectro,i);
	 }
     writcurpos(spectro);/* time in text window */
     } /* new index*/
   break;

/** Callbacks: wave_pixmap **/
 case 15:
  /*  "J Show w->e in window 1" */
   /*x-axis*/
   /* for now change window 1 reguardless of which window
      received the input   */
   window = 1;
   if(window == 1){
     /* expanded window use start and end  */
     /* set cursor to be in center */

     if(spectro->endmarker > spectro->startmarker ){
       if(spectro->saveindex > spectro->endmarker &&
	  spectro->saveindex < spectro->startmarker ){
     spectro->startindex[window] = spectro->startmarker;
     spectro->startime[window] = spectro->startindex[window] / 
				 spectro->spers * 1000;
     spectro->sampsused[window] =  spectro->endmarker - spectro->startmarker;

     wave_pixmap(spectro->info[window],spectro,1,window,NULL);
     update(spectro->info[window],spectro,window);
      }/* don't have to move cursor */
    else{
     /* cursor is outside of segment put it in the center */
     spectro->oldindex = spectro->saveindex;
     spectro->oldtime = spectro->savetime;
     spectro->startindex[window] = spectro->startmarker;
     spectro->startime[window] = spectro->startindex[window] / 
				 spectro->spers * 1000;
     spectro->sampsused[window] =  spectro->endmarker - spectro->startmarker;

     spectro->saveindex = spectro->sampsused[window] / 2 +
	  spectro->startindex[window] ;
     spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000.0; 

     wave_pixmap(spectro->info[window],spectro,1,window,NULL);
     update(spectro->info[window],spectro,window);

	for(i = 0; i < NUM_WINDOWS; i++){
	  if( i != window )
	       update(spectro->info[i],spectro,i);
	 }
       writcurpos(spectro);/* time in text window */
      }/* new cursor position */
    }/* end > start */
   }/* w1 */
   break;

/** Callbacks: none **/
 case 16:
  /* separator */
   break;

/** Callbacks: none **/
 case 17:  /* \ Peak/valley picking on/off */
   spectro->localtimemax = !spectro->localtimemax;
   if (spectro->localtimemax) {
     writetext("Peak/valley picking in time enabled.\n");
     writetext("Hold <Shift> down and click mouse button to find valley.\n");
  }
   else
     writetext("Peak/valley picking in time disabled.\n");
   break;

/** Callbacks: none **/
 case 18:
  /* separator */
   break;

/** Callbacks: zoomin **/
 case 19:
   /*up arrow zoom in*/
   if (spectro->locked) {
     for (i = 0; i < allspectros.count; i++)
      if (allspectros.list[i]->locked)
       {
       
       /* save the active spectro until last */
       if (allspectros.list[i] != spectro)
	 zoomin(allspectros.list[i],window);
       
     } /* all waveforms */
   } /* locked do all other waveforms too */
   zoomin(spectro,window);
   break;

/** Callbacks: zoomout **/
 case 20:
   /*down arrow zoom out*/
   if (spectro->locked) {
   for ( i = 0; i < allspectros.count; i++)
      if (allspectros.list[i]->locked) {
       if (allspectros.list[i] != spectro)
        /* save the active spectro until last*/
        zoomout(allspectros.list[i],window);
        
      }/* all waveforms */
   }/* locked do all other waveforms too*/
   zoomout(spectro,window);
   break;

/** Callbacks: none **/
 case 21:
  /* separator */
   break;

/** Callbacks: SelectLocked **/
 case 22:
   /*L lock cursor arrows */
   /* across selected waveforms works with arrows, f,b,F,B,t, mouse click */

   SelectLocked(); 

   break;

/** Callbacks: none **/
 case 23:
   /*U unlock cursor arrows */
   /* across all waveforms works with arrows only*/

   for ( i = 0; i < allspectros.count; i++) {
     allspectros.list[i]->locked = 0;
   }
   writetext("UNLOCKED.\n");
   break;

 }/* end switch */

}/* end HandleTimeMenu */


/**********************************************************************/
/*void	SelectLocked ()                                               */
/**********************************************************************/

void SelectLocked()
{
  Widget lock_dialog, pane, form, lock_list, ok_button, cancel_button, mainw;
  XmString *list;
  XmString label;
  int i, n;
  Arg args[5];
  n=0;


  lock_dialog = XtVaCreatePopupShell ("Lock", 
				      topLevelShellWidgetClass, application,
				      XmNdeleteResponse, XmDO_NOTHING,
				      NULL);

  mainw = XmCreateMainWindow(lock_dialog, "lock_mainw", args, n);
  XtManageChild(mainw);

  pane = XtVaCreateWidget ("lockpane",
			   xmPanedWindowWidgetClass, mainw,
			   XmNsashWidth, 1,
			   XmNsashHeight, 1,
			   NULL);

  form = XtVaCreateWidget ("lockform",
			   xmFormWidgetClass, pane, NULL);

  /* Construct the list */

  list = (XmString *) malloc( sizeof(XmString) * allspectros.count);

  for (i=0; i<allspectros.count; i++)
    list[i] = XmStringCreateLocalized (allspectros.list[i]->wavename);

  label = XmStringCreateLocalized ("Select waveforms to lock:");
  lock_list = XtVaCreateManagedWidget ("locklist",
				       xmListWidgetClass, form,
				       XmNitemCount, allspectros.count,
				       XmNitems, list,
				       XmNselectionPolicy, XmMULTIPLE_SELECT,
/*				       XmNmultipleSelectionCallback, toggleLock,*/
				       XmNlabelString, label,
				       XmNvisibleItemCount, 10,
				       NULL);

  XtManageChild(form);

  /* Set up action area */

  form = XtVaCreateWidget ("lockform2",
			   xmFormWidgetClass, pane,
			   XmNfractionBase, 5,
			   NULL);

  ok_button = XtVaCreateManagedWidget ("Ok",
				       xmPushButtonGadgetClass, form,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNbottomAttachment, XmATTACH_FORM,
				       XmNleftAttachment, XmATTACH_POSITION,
				       XmNleftPosition, 1,
				       XmNrightAttachment, XmATTACH_POSITION,
				       XmNrightPosition, 2,
				       NULL);
  XtAddCallback (ok_button, XmNactivateCallback, DoneLock, lock_list);

  cancel_button = XtVaCreateManagedWidget ("cancel",
				       xmPushButtonGadgetClass, form,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNbottomAttachment, XmATTACH_FORM,
				       XmNleftAttachment, XmATTACH_POSITION,
				       XmNleftPosition, 3,
				       XmNrightAttachment, XmATTACH_POSITION,
				       XmNrightPosition, 4,
				       NULL);

  XtAddCallback (cancel_button, XmNactivateCallback, CancelLock, lock_dialog);
  
  XtManageChild(lock_list);
  XtManageChild(form);
  XtManageChild(pane);
  XtManageChild(lock_dialog);

  /* Select all currently locked waveforms */
  /* set the temporary lock boolean array */

  for (i=0; i<allspectros.count; i++)
    if (allspectros.list[i]->locked)
      XmListSelectPos (lock_list, i+1, False);

  free(list);

}

/**********************************************************************/
/*void	DoneLock (Widget, XtPointer, XtPointer)                     */
/**********************************************************************/
void DoneLock(Widget w, XtPointer client_data, XtPointer call_data)
{
  int i, pos;
  Widget list = (Widget) client_data;
  int *lock_bool;
  int lock_count;
  char str[300];
  XSPECTRO *refspectro;  /* use as a reference for calculating initial offsets */

  XmListGetSelectedPos (list, &lock_bool, &lock_count);


  if (lock_count > 1)  /* lock more than one waveform */
    {
      /* initialize lock settings */
      for (i=0; i < allspectros.count; i++)
	allspectros.list[i]->locked = 0;

      /* Only update lock settings if the user presses "ok" */
      for (i=0; i < lock_count; i++)
	{
	  pos = lock_bool[i] - 1; 
	  allspectros.list[pos]->locked = 1;
	}

      writetext ("The following waveforms were LOCKED:\n");
      for (i=0; i < allspectros.count; i++)
	if (allspectros.list[i]->locked)
	  {
	    sprintf (str, "     %s\n", allspectros.list[i]->wavename);
	    writetext(str);
	  }

      /* arbitrarily set the ref. spectro to be the first selected spectro: */
      refspectro = allspectros.list[lock_bool[0]];

      /* Set the time offsets */
      for ( i = 0; i < allspectros.count; i++) {
	if (allspectros.list[i]->locked)
	  allspectros.list[i]->timeoffset =
	    allspectros.list[i]->savetime - refspectro->savetime;
      }

    }
  else
    writetext ("Must lock more than one waveform.\n");

  XtDestroyWidget (XtParent(XtParent(XtParent(XtParent(list)))));
  free(lock_bool);

}


/**********************************************************************/
/*void	CancelLock (Widget, XtPointer, XtPointer)                     */
/**********************************************************************/
void CancelLock(Widget w, XtPointer client_data, XtPointer call_data)
{

  Widget shell = (Widget) client_data;

  XtDestroyWidget (shell);

  writetext("No waveforms were locked.\n");
}


/**********************************************************************/
/*void	timestep (XSPECTRO *, float)                                  */
/**********************************************************************/
void timestep (XSPECTRO *spectro, float step)
{
  int i;

  spectro->oldindex = spectro->saveindex;
  spectro->oldtime = spectro->savetime;
  
  spectro->savetime = spectro->savetime + step;
  
  spectro->saveindex = spectro->savetime * spectro->spers / 1000.0 + .5;
  spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000.0;
  validindex(spectro);
  if( spectro->oldindex != spectro->saveindex){
    for(i = 0; i < NUM_WINDOWS; i++){
      update(spectro->info[i], spectro, i);
    }
    writcurpos(spectro);/* time in text window */
  }/* new index*/
}

/**********************************************************************/
/*void	CalculateOffset(float)                                   */
/**********************************************************************/
void CalculateOffset(float curoffset)
{
  int i;

  /* reorient offsets to be relative to current waveform */
  for ( i = 0; i < allspectros.count; i++)
    if (allspectros.list[i]->locked)
      {
	allspectros.list[i]->timeoffset += -curoffset;
      }
}


/**********************************************************************/
/*void	HandleSpectrumMenu(Widget w,XtPointer client_data,XtPointer call_data)*/
/**********************************************************************/

void HandleSpectrumMenu(Widget w,XtPointer client_data, XtPointer call_data){

/* use num_entries to find start of button array */
/* use w to find which window the call was made from*/

  int window, entry, offset, i, n;
  Widget *buttons;
  XSPECTRO *spectro;
  XmString mstr;
  char str[50];
  Arg args[5];
  XmRowColumnCallbackStruct *c_data;
  void InitSpecParams(), InitSGParams();	// xkl_dlog.c

  c_data = (XmRowColumnCallbackStruct *)call_data;
  spectro = (XSPECTRO *) client_data;

/*
 * Establish which pulldown menu 
 */


 for(window = 0; window < numPulldown; window++)
     if(spectro->menu_bar[window] == XtParent(XtParent(w))) break;

 buttons = spectro->buttons[window];


/*
 * Find out offset based on num_entries globally defined array./
 * Offset in button array is total of previous num_entries in entry array.
 */

 offset = num_entries[0] + num_entries[1] ;
 for (entry = 0; entry < num_entries[2]; entry++)
   if (c_data->widget == buttons[offset + entry]) break;

   /* SPECTRUM 

0    "d DFT magnitude",
1    "s Spectrogram-like",
2    "S Spectrogram-like + DFT",
3    "c Critical-band",
4    "j Critical-band + DFT",
5    "T c-b compute slope > 1kHz",
6    "l Linear-prediction + DFT",
7    "a Avg. DFT mag over interval",
8    "A Avg. DFT mag ",
9    "k Spectral avg. -kn/2 to kn/2",
10   "C Change spectrum parameters",
11   "",
12   "i Include spectrogram",
13   ", Lighten spectrogram",
14   ". Darken spectrogram",
15   "u Set msec in spectrogram",
16   "8 Change spectrogram parameters",
17   "K Recalculate spectrogram",
18   "",
19   "/ Peak picking on/off",

   */

 switch(entry) {

/** Callbacks: update **/
 case 0: /*d DFT magnitude*/
   if (spectro->option != 'd') {
     spectro->option = 'd';
     update(spectro->info[SPECTRUM],spectro,SPECTRUM);
   }
   break;

/** Callbacks: update **/
 case 1: /*s Spectrogram-like*/
   if (spectro->option != 's') {
       spectro->history = 0;
       spectro->option = 's';
       update(spectro->info[SPECTRUM],spectro,SPECTRUM);
   }
   break;

/** Callbacks: update **/
 case 2: /*S Spectrogram-like + DFT*/
   if (spectro->option != 'S') {
     spectro->option = 'S';
     update(spectro->info[SPECTRUM],spectro,SPECTRUM);
   }
   break;

/** Callbacks: update **/
 case 3: /*c Critical-band*/
   if (spectro->option != 'c') {
     spectro->option = 'c';
     spectro->history = 0;
     update(spectro->info[SPECTRUM],spectro,SPECTRUM);
   }
   break;

/** Callbacks: update **/
 case 4: /*j Critical-band + DFT*/
   if (spectro->option != 'j') {
     spectro->option = 'j';
     update(spectro->info[SPECTRUM],spectro,SPECTRUM);
   }
   break;

/** Callbacks: update **/
 case 5: /* T c-b compute slope > 1kHz */
   if (spectro->option != 'T') {
     spectro->option = 'T';
     update(spectro->info[SPECTRUM],spectro,SPECTRUM);
   }
   break;

/** Callbacks: update **/
 case 6: /* l Linear-prediction + DFT*/
   if (spectro->option != 'l') {
     spectro->option = 'l';
     update(spectro->info[SPECTRUM],spectro,SPECTRUM);
   }
   break;

/** Callbacks: aval, cancelinput **/
 case 7:
   /*a avg. DFT mag over interval*/
   /* don't save averaging option */
   if(spectro->option != 'a' && spectro->option != 'A' &&
      spectro->option != 'k') 
	   spectro->savoption = spectro->option;
   spectro->option = 'a';
   spectro->avgcount = 0; 
   sprintf(str,"%.1f",spectro->savetime);
   XtSetArg(args[0],XmNvalue,str);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);

   mstr = XmStringCreateLocalized("start time:");
   XtSetArg(args[0],XmNselectionLabelString,mstr);
   XtSetValues(input_dialog,args,1);
   XmStringFree(mstr);
   XtSetArg(args[0],XmNtitle,spectro->wavename);
   XtSetValues(XtParent(input_dialog),args,1);
   XtAddCallback(input_dialog,XmNokCallback,aval,(XtPointer)spectro);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,(XtPointer)spectro);
   writetext("Waiting for input...\n");
   opendialog(input_dialog); 
   HighlightInput (str, XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT));

   break;

/** Callbacks: avgval, cancelavg **/
 case 8:
   /*A Avg. DFT mag*/
   /* input up to AVGLIMIT spectral times */
   /* don't save averaging option */
   if(spectro->option != 'a' && spectro->option != 'A' &&
      spectro->option != 'k') 
   spectro->savoption = spectro->option;
   spectro->option = 'A';
   spectro->avgcount = 0;
   XtManageChild(
       XmFileSelectionBoxGetChild(input_dialog,XmDIALOG_HELP_BUTTON) );
   sprintf(str,"%.1f",spectro->savetime);
   XtSetArg(args[0],XmNvalue,str);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized("time 1:");
   XtSetArg(args[0],XmNselectionLabelString,mstr);
   XtSetValues(input_dialog,args,1);
   XmStringFree(mstr);
   XtSetArg(args[0],XmNtitle,spectro->wavename);
   XtSetValues(XtParent(input_dialog),args,1);
   XtAddCallback(input_dialog,XmNokCallback,avgval,(XtPointer)spectro);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelavg,(XtPointer)spectro);
   XtAddCallback(input_dialog,XmNhelpCallback,doneavg,(XtPointer)spectro);
   writetext("Waiting for input...\n");
   opendialog(input_dialog);
   HighlightInput (str, XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT));

   /* 3 buttons don't need traversal */
   break;

/** Callbacks: getavg **/
 case 9:
   /*k spectral avg. -kn/2 to kn/2*/
   /* no input required */
   /* don't save averaging option */
   if (spectro->option != 'a' && spectro->option != 'A' &&
      spectro->option != 'k') spectro->savoption = spectro->option;
   spectro->option = 'k';
   getavg(spectro);
   break;

/** Callbacks: doparamsall, doparam, HandleParamMenu **/
 case 10:
   /* C Change spectrum parameters */
   /*make sure the correct spectro pointer gets to the menu callback
    and the selection box callback since there is only one selection box*/
#ifdef _OBSOLETE  // superseded in v3
   XtSetArg(args[0],XmNtitle,spectro->wavename);
   XtSetValues(param_tl, args,1);

   /* set flag so spectro knows that it has the parameter
      widget managed  */

  for( i = 0; i < allspectros.count; i++){
     if(allspectros.list[i] == spectro){
	  spectro->param_active = 1;
	}
     else{
	 spectro->param_active = 0; 
       }
   }

   XtRemoveAllCallbacks(param_sb,XmNokCallback);
   XtRemoveAllCallbacks(param_sb,XmNhelpCallback);
   XtRemoveAllCallbacks(param_menu,XmNentryCallback);
   XtAddCallback(param_sb, XmNokCallback,doparam,(XtPointer)spectro);
   XtAddCallback(param_sb, XmNhelpCallback,doparamsAll,(XtPointer)spectro);
   XtAddCallback(param_menu,XmNentryCallback,HandleParamMenu,(XtPointer)spectro);

   dolist(spectro); /* update values */
   open_tl_dialog(param_tl);
#endif
   InitSpecParams(application, spectro);
   break;


/** Callbacks: none **/
 case 11:
  /* separator */
   break;

/** Callbacks: addgram **/
 case 12:
  /* i include spectrogram */
  if(!spectro->spectrogram) { addgram(spectro); }
   break;

/** Callbacks: remapgray **/
 case 13:
   /* , Lighten spectrogram */
   if(spectro->spectrogram){
     spectro->maxmag += 2;
     spectro->minmag += 2;
     remapgray(spectro);
   }
   break;

/** Callbacks: remapgray **/
 case 14:
   /* . Spectrogram darker */
   if(spectro->spectrogram){
    spectro->maxmag += -2;
    spectro->minmag += -2;
    remapgray(spectro);
   }
   break;

/** Callbacks: setms, cancelinput **/
 case 15:
 /* u Set ms in spectrogram */
 if(spectro->spectrogram){
    sprintf(str,"%.1f",spectro->specms);
    XtSetArg(args[0],XmNvalue,str);
    XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
    mstr = XmStringCreateLocalized("msec in spectrogram:");
    XtSetArg(args[0],XmNselectionLabelString,mstr);
    XtSetValues(input_dialog,args,1);
    XmStringFree(mstr);
    n=0;
    XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
    XtSetValues(XtParent(input_dialog),args,n); 
    XtAddCallback(input_dialog,XmNokCallback,setms,(XtPointer)spectro);
    XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,(XtPointer)spectro);
    opendialog(input_dialog);
  }/* spectrogram exists */
 break; 

/** Callbacks: none **/
 case 16:
   /* 8 Change spectrogram parameters */
   InitSGParams(application, spectro);
   break;

/** Callbacks: findsdelta, findypix, calculate_spectra, win_size, writegram **/
 case 17:
 /* K check for xkl_defs.dat file and recalculate spectrogram*/
   if(spectro->spectrogram) {

     if (!read_data(spectro)) set_defaults(spectro);
     
     spectro->savemsdelta = spectro->msdelta;
     spectro->savewinms = spectro->winms; 

    findsdelta(spectro);
    findypix(spectro);
    calculate_spectra(spectro);
    win_size(spectro);
    writegram(spectro);

    XDestroyImage(spectro->xim);
    spectro->xim = NULL;

    /* assume for now that this deallocates spectro->pix */   

    if (!resizegram(spectro))
      update(spectro->info[GRAM],spectro,GRAM); 
   }
   break;

/** Callbacks: none **/
 case 18: /* separator */
   break;

/** Callbacks: none **/
 case 19: /* / toggle peak picking EC 2/27/97 */
   spectro->localfreqmax = !spectro->localfreqmax;
   if (spectro->localfreqmax)
     writetext("Peak picking in frequency enabled.\n");
   else
     writetext("Peak picking in frequency disabled.\n");
   break;

 } /* end switch */

} /* end HandleSpectrumMenu */



/**********************************************************************/
/*voidHandleAudioMenu(Widget w,XtPointer client_data,XtPointer call_data)*/
/**********************************************************************/

void HandleAudioMenu(Widget w,XtPointer client_data, XtPointer call_data){

/* use num_entries to find start of button array */
/* use w to find which window the call was made from*/

  int window, entry, offset;
  Widget *buttons;
  XSPECTRO *spectro;
  XmRowColumnCallbackStruct *c_data;

  c_data = (XmRowColumnCallbackStruct *) call_data;
  spectro = (XSPECTRO *) client_data;

/*
 * Establish which pulldown menu
 */

  for (window = 0; window < NUM_WINDOWS; window++)
    if (spectro->menu_bar[window] == XtParent(XtParent(w))) break;

  buttons = spectro->buttons[window];
 
/*
 * Find out offset based on num_entries globally defined array./
 * Offset in button array is total of previous num_entries in entry array.
 */

  offset = num_entries[0] + num_entries[1] + num_entries[2];
  for (entry = 0; entry < num_entries[3]; entry++)
    if (c_data->widget == buttons[offset + entry]) break;

   /* TIME MENU

0   "p Play between w->e",
1   "P Play entire window",
2   "0 Abort audio output",
3   "R Record to .wav file",
4   "Q Change record parameters",

   */

 switch (entry) {

/** Callbacks: xklPlay **/
 case 0: /* p Play between w->e */
   spectro->segmode = 1;
   xklPlay(spectro, window);
   break;

/** Callbacks: xklPlay **/
 case 1: /* P Play entire window */
   spectro->segmode = 0;
   xklPlay(spectro, window);
  break;

  /** Callbacks: xklPlay **/
 case 2: /* 0 Abort audio in progress */
     KillPlay();
     break;

/** Callbacks: xklRecord **/
 case 3: /* R Record to .wav file */
   xklRecord(spectro);
   char tmp[256];
   // sprintf(tmp, "Out from xklRecord!\n");
  //writetext(tmp);
   break; 

/** Callbacks: CreateRecDialog, listrec, open_tl_dialog **/
 case 4: /* Q Change record parameters */
   /* changes global variables, okcallback added in CreateRecDialog */ 
   if (!rec_tl) CreateRecDialog(application);
   listrec(); /* update values */
   open_tl_dialog(rec_tl);
   break;
 }
}


/**********************************************************************
 *
 * void HandleHelpMenu(Widget w,XtPointer client_data,
 *		    XmRowColumnCallbackStruct *call_data)
 *
 **********************************************************************/

void HandleHelpMenu(Widget w, XtPointer client_data, XtPointer call_data)
{
   XtPopup(XtParent(XtParent(help_text)), XtGrabNone);
}



/**********************************************************************
 *
 * void HandleParamMenu(Widget w,XtPointer client_data,
 *		    XtPointer call_data)
 *
 * param_buttons is a global since all spectros share the parameter 
 * Widget.
 *
 **********************************************************************/

void HandleParamMenu(Widget w,XtPointer client_data, XtPointer call_data)
{

   XSPECTRO *spectro;
   int entry;
   XmRowColumnCallbackStruct *c_data;

   c_data = (XmRowColumnCallbackStruct *) call_data;
   spectro = (XSPECTRO *) client_data;
   
   for (entry = 0; entry < XtNumber(paramlist); entry++)
      if (c_data->widget == param_buttons[entry]) break;
   
   /* 
    * Note: paramlist is one more that spectro->params, e = DONE.
    */

/** Callbacks: inputparams, doneparam **/   
   if (entry == XtNumber(paramlist) - 1)  doneparam(w, client_data, call_data);    
   else inputparams(spectro,entry); 
}



/**********************************************************************
 *
 * void HandleWavlistMenu(Widget w,XtPointer client_data,
 *		    XtPointer call_data)
 *
 * wav_buttons is a global since all spectros share the parameter 
 * Widget.
 *
 **********************************************************************/

void HandleWavlistMenu(Widget w,XtPointer client_data, XtPointer call_data)
{

   XSPECTRO *spectro;
   int entry;
   XmRowColumnCallbackStruct *c_data;

   c_data = (XmRowColumnCallbackStruct *) call_data;
   spectro = (XSPECTRO *) client_data;
   
   for (entry = 0; entry < allspectros.count; entry++)
      if (c_data->widget == wav_buttons[entry]) break;
   
   /* 
    * Note: paramlist is one more that spectro->params, e = DONE.
    */

/** Callbacks: FindWav, cancelwavlist **/   
   FindWav(spectro, entry);

   cancelwavlist(w, NULL, NULL); /* clean up */
}


/**********************************************************************/
/*void	HandleRecMenu(Widget w,XtPointer client_data,XtPointer call_data)*/
/**********************************************************************/

void HandleRecMenu(Widget w,XtPointer client_data, XtPointer call_data)
{

/* rec_buttons is a global since all spectros share the Rec Widget*/

  int entry;
  XmRowColumnCallbackStruct *c_data;

  c_data = (XmRowColumnCallbackStruct *)call_data;

  for (entry = 0; entry < numRecParams + 1; entry++)
    if (c_data->widget == rec_buttons[entry]) break;

/** Callbacks: donerec, inputrec **/

  if (entry == numRecParams) donerec(w, client_data, call_data);   /* e = DONE: Last entry */
  else inputrec(entry);
 
}


/******************* Menu Creation Procedures ************************/

/************************************************************************/
/* Widget CreateOneMenu(parent,name,labels,num_labels,buttons,is_toggle)*/
/************************************************************************/

Widget CreateOneMenu(Widget parent, char *name, char *labels[], int num_labels,
		     Widget *buttons, Boolean is_toggle, int shl)
    /* parent is the menu bar  */
    /* shl is the flag for whether the cascade buttons have labels*/
{
 
  Widget shell,casc;
  Arg	 args[10];
  int	 row;
  XmString  buffer,mstr;
  char	 acc[50], key;
  char	 acc_text[10];
  
   /* create a menu pane, parent should be a menu bar */
   shell = XmCreatePulldownMenu(parent,name,NULL,0);

   /* enable tear-off menus (broken) */
   // XtVaSetValues(shell, XmNtearOffModel, XmTEAR_OFF_ENABLED, NULL);

   /*create buttons */
  for (row = 0; row < num_labels; row++) {

     if (strlen(labels[row]) == 0) {  /* Separator */
	buttons[row] = XmCreateSeparatorGadget(shell, "msep", NULL, 0);
     }
     else { 
	if (labels[row][0] == '!') { /* an escaped char,e.g. arrow key */
	   if (labels[row][1] == 'A') {
	      if (labels[row][2] == 'L') { /* left arrow key */
		 buffer = XmStringCreateLocalized(&labels[row][3]);
		 XtSetArg(args[0], XmNlabelString, buffer);

		 sprintf(acc, " <Key>Left");
		 mstr = XmStringCreateLocalized("left ->");
	      }
	      if (labels[row][2] == 'R') { /* right arrow key */
		 buffer = XmStringCreateLocalized(&labels[row][3]);
		 XtSetArg(args[0], XmNlabelString, buffer);

		 sprintf(acc, " <Key>Right");
		 mstr = XmStringCreateLocalized("right ->");
	      }
	      if (labels[row][2] == 'U') { /* up arrow key */
		 buffer = XmStringCreateLocalized(&labels[row][3]);
		 XtSetArg(args[0], XmNlabelString, buffer);

		 sprintf(acc, " <Key>Up");
		 mstr = XmStringCreateLocalized("up ->");
	      }
	      if (labels[row][2] == 'D') { /* down arrow key */
		 buffer = XmStringCreateLocalized(&labels[row][3]);
		 XtSetArg(args[0], XmNlabelString, buffer);

		 sprintf(acc, " <Key>Down");
		 mstr = XmStringCreateLocalized("down ->");
	      }
	   } /* Arrow key */
	}
/* Normal chars */

	else {
	   buffer = XmStringCreateLocalized(&labels[row][1]);
	   XtSetArg(args[0], XmNlabelString, buffer);
	   
	   key = labels[row][0];

	   if((int)key > 64 && (int)key < 91){ /* Uppercase */
	      sprintf( acc, " Shift<Key>%c", key); 
	   }
	   else if (key == ' ') { strcpy(acc, " <Key>space"); }
	   else if (key == '<') { strcpy(acc, " Shift<Key>less"); }
	   else if (key == '>') { strcpy(acc, " Shift<Key>greater");}
	   else if (key == '+') { strcpy(acc, " Shift<Key>plus"); }
	   else if (key == '-') { strcpy(acc, " <Key>minus"); }
	   else if (key == '^') { strcpy(acc, " Shift<Key>asciicircum"); }
	   else if (key == '#') { strcpy(acc, " Shift<Key>numbersign"); }
	   else if (key == '*') { strcpy(acc, " Shift<Key>asterisk"); }
	   else if (key == '.') { strcpy(acc, " <Key>period"); }
	   else if (key == ',') { strcpy(acc, " <Key>comma"); }
	   else if (key == '/') { strcpy(acc, " <Key>slash"); }
	   else if (key == '@') { strcpy(acc, " Shift<Key>at"); }
	   else if (key == '?') { strcpy(acc, " Shift<Key>question"); }
	   else { sprintf( acc, " <Key>%c", key); }

	   sprintf(acc_text, "%c", key);
	   mstr = XmStringCreateLocalized(acc_text);
	}
     
	XtSetArg(args[1], XmNaccelerator, acc);
	XtSetArg(args[2], XmNacceleratorText, mstr);

	if (is_toggle) {
	   XtSetArg(args[3], XmNset, True);
	   buttons[row] = XmCreateToggleButtonGadget(shell,"mtoggle", args, 4);
	} /* toggle */
	else {
	   buttons[row] = XmCreatePushButtonGadget(shell,"mbutton", args, 3); 
	}/* Push */
	XmStringFree(buffer);
	XmStringFree(mstr);
     }/* button	  */		
  }

  XtManageChildren(buttons,num_labels);
  
   /* cascade button */
  if(shl){
  buffer = XmStringCreateLocalized(name);
  XtSetArg(args[0],XmNlabelString,buffer);
  XtSetArg(args[1],XmNmnemonic,*name);
  XtSetArg(args[2],XmNsubMenuId,shell);
  casc = XtCreateManagedWidget("",
	     xmCascadeButtonWidgetClass,parent,args,3);
  XmStringFree(buffer);
  }/*full sized menu bar with labels*/
 
  else{
  XtSetArg(args[0],XmNmnemonic,*name);
  XtSetArg(args[1],XmNsubMenuId,shell);
  casc = XtCreateManagedWidget("",
	     xmCascadeButtonWidgetClass,parent,args,2);

 }/*no labels on menu bar to save space*/

 if(strcmp(name,"Help") == 0){
       XtSetArg(args[0],XmNmenuHelpWidget,casc);
       XtSetValues(parent,args,1);
     }

   return(shell);
}



/*************************************************************************/
/*    void CreateMenus(parent, client_data , Widget **buttons,*menu_bar) */
/*************************************************************************/

void CreateMenus(Widget  parent, XSPECTRO *spectro, 
      Widget **buttons, Widget *menu_bar, int shl){

/* parent is mainwindow	 */
/* shl is a flag, 1 = full sized menu bar, 0 = smaller one without labels*/

  Widget  menu, *but;
  int	  i;
  Arg	  args[5];
  int	  entrycnt;  /* placement in entries array */
  int	  totbutt;  /* for allocating buttons on menus*/


  /*menus,entries and num_entries are static arrays defined in xkl.c*/

  
  totbutt = 0;
    for(i = 0; i < XtNumber(menus); i++)
	   totbutt += num_entries[i];

  *buttons  = (Widget *) malloc( sizeof(Widget) * totbutt);
  but = *buttons;

  *menu_bar = XmCreateSimpleMenuBar(parent, "bar", args, 0);
  XtManageChild(*menu_bar);

  /* create file menu */

  entrycnt = 0;

  for(i = 0; i < XtNumber(menus); i++){
  
    menu = CreateOneMenu(*menu_bar,menus[i],
		       &entries[entrycnt],num_entries[i],
		       &but[entrycnt],	False, shl );
					     
    XtAddCallback (menu, XmNentryCallback,menuprocs[i],(XtPointer)spectro );
  
    entrycnt +=  num_entries[i];

  }/* all menus */
    
 }/* end CreateMenus */



/******************* Dialog Creation Routines **********************/


/*********************************************************************
 *
 *	 void CreateHelpDialog(Widget parent)		     
 *
 *********************************************************************/

void CreateHelpDialog(Widget parent)
{
   int n = 0;
   Arg args[5];
   Widget help_dialog;
       
   XtSetArg(args[n],XmNdeleteResponse, XmUNMAP); n++;
   help_dialog = XtCreatePopupShell("XKLHELP",topLevelShellWidgetClass,
				    parent,args,n);
   CreateHelpText(help_dialog);
}

/****************************************************************
 *
 *    void CreateHelpText(Widget parent)		
 *
 ****************************************************************/
void CreateHelpText(Widget parent) 
{
   int i;
   int textlen;

   createtext(parent,&help_text);
   textlen = XtNumber(help_string);

   for(i = 0; i < textlen; i++)	
      XmTextInsert(help_text,XmTextGetLastPosition(help_text),
		   help_string[i]);
}


/****************************************************************/
/*     void createtext(Widget parent, Widget *textw)		*/
/****************************************************************/

void createtext(Widget parent,Widget *textw)
{
  Arg   args[10];
  int n = 0;
  Widget h_sb,v_sb;

  XtSetArg(args[n],XmNeditable,False);	n++;
  XtSetArg(args[n],XmNeditMode,XmMULTI_LINE_EDIT); n++;
  
  XtSetArg(args[n],XmNheight,800); n++;
  XtSetArg(args[n],XmNwidth,600); n++;
  
  *textw = XmCreateScrolledText(parent,"textw",args,n);
  XtManageChild(*textw);
  
  
  XtSetArg(args[0],XmNverticalScrollBar,&v_sb);
  XtSetArg(args[1],XmNhorizontalScrollBar,&h_sb);
  XtGetValues(XtParent(*textw),args,2);
  
  if(v_sb != NULL) XmAddTabGroup(v_sb);
  if(h_sb != NULL) XmAddTabGroup(h_sb);  
}



/********************** Resizing Routines **************************/

/**************************************************************/
/*   void doresize(Widget ,XtPointer, XtPointer)	*/
/**************************************************************/

void doresize(Widget w,XtPointer client_data, XtPointer call_data){ 

  Dimension width, height;
  Arg args[5];
  int i,n;
  XSPECTRO *spectro;
  INFO *info;
  XmAnyCallbackStruct *c_data;

  c_data= (XmAnyCallbackStruct *) call_data;
  spectro = (XSPECTRO *) client_data;

     for(i = 0; i < NUM_WINDOWS; i++)
	if(spectro->draww[i] == w) break;
     
      info = spectro->info[i]; 

/*xr must = width of spectrogram, window may be wider*/


      if (i != GRAM ) {
	n = 0;
	XtSetArg(args[n], XmNwidth, &width); n++;
	XtSetArg(args[n], XmNheight, &height); n++;
	XtGetValues(w,args,n);
	spectro->xr[i] = width;
	spectro->yb[i] = height;
      }
      
      if(info->pixmap) { 
	if(i == GRAM ) {
      /*assume size is known since user can't resize*/
	  width = spectro->xr[GRAM];
	  height = spectro->yb[GRAM]; 
	}
	else { 
	  make_pixmap(info,spectro,i);
	  if(i == SPECTRUM) draw_spectrum(spectro,1,NULL);
	}
	
	spurious(info,width,height);
	
      } /* wait until there's a gc */ 

}/* doresize */



/************************************************************/
/* spurious(INFO *info,Dimension width, Dimension height)   */
/************************************************************/
void spurious(INFO *info, Dimension width, Dimension height)
{
int n,num,app,first;
XtInputMask mask;
XEvent event;
/* app_context is global, context for graphics */

/*needs the width and height of the drawwing widget
  that is being resized
*/

   /* deal with extra expose events DEC motif 1.1 generated by*/
   /* resize, may not need this with future releases*/
   /* if the window manager provides a single full sized expose*/
   /* this may be omitted */

    XSync(info->display,False);
    num = XPending(info->display);
    first = 1;
    for(n = 0; n < num; n++){
      mask = XtAppPending(app_context);
      XNextEvent(info->display,&event);
      if(event.type == Expose && mask != 0 &&
	 event.xexpose.window == info->win){
	  if(first){
	    event.xexpose.x = 0;
	    event.xexpose.y = 0;
	    event.xexpose.width = width;
	    event.xexpose.height = height;
	    event.xexpose.count = 0;
	    XPutBackEvent(info->display,&event);
	    if((mask = XtAppPending(app_context)) != 0){
	     XtAppProcessEvent(app_context,mask);
	     XFlush(XtDisplay(application)); 
	     XSync(info->display,False);
	     num = XPending(info->display) + n + 1;
	      }
	     first = 0;
	    }/*one complete redraw*/
	  else{
	    event.xexpose.x = 0;
	    event.xexpose.y = 0;
	    event.xexpose.width = 2;
	    event.xexpose.height = 2;
	    event.xexpose.count = 1;
	    XPutBackEvent(info->display,&event);
	    if((mask = XtAppPending(app_context)) != 0){
	     XtAppProcessEvent(app_context,mask);
	     XFlush(XtDisplay(application));	  
	     }
	  }/*little exposes to keep things straight on queue*/
       }/* expose of resized window*/
   
   else{

      XPutBackEvent(info->display,&event);
       app = 0;

       if((mask = XtAppPending(app_context)) != 0){
	   XtAppProcessEvent(app_context,mask);
	   XFlush(XtDisplay(application));	
	   app++;
	 }
       if(app == 0){
	if((mask = XtAppPending(text_app))!= 0 ){
	     XtAppProcessEvent(text_app,mask);
	     XFlush(XtDisplay(cmdw));
	  } 
       } 
    }
 
}/* all events on queue*/

  if(first){
	    event.type = Expose;
	    event.xexpose.window = info->win;
	    event.xexpose.display = info->display;
	    event.xexpose.serial = event.xany.serial + 1;
	    event.xexpose.send_event = True;
	    event.xexpose.x = 0;
	    event.xexpose.y = 0;
	    event.xexpose.width = width;
	    event.xexpose.height = height;
	    event.xexpose.count = 0;
	    XSendEvent( info->display,info->win,False,Expose,&event);	      
	  }/* there wasn't an expose so send one */

    }	 /* end of spurious expose work around */




/****************************************************************/
/*	      resizegram(XSPECTRO *spectro)			 */
/****************************************************************/
int resizegram(XSPECTRO *spectro){
Arg args[5];
int n;
Dimension width;
INFO *info;
Dimension gramw,gramh;
int index;
       
   index = GRAM;

   info = spectro->info[GRAM];

   make_pixmap(info,spectro,index);

   XClearWindow(info->display,info->win);

   if(spectro->xr[GRAM] <= MINSIZE)
       width = MINSIZE;
   else
       width = spectro->xr[GRAM];

   n = 0;
   XtSetArg(args[n],XmNwidth,&gramw); n++;
   XtSetArg(args[n], XmNheight,&gramh);n++;
   XtGetValues(spectro->toplevel[GRAM] ,args,n);
   if( gramw != width ||
       gramh != spectro->yb[GRAM]+spectro->menu_height){
     XResizeWindow(XtDisplay(spectro->toplevel[GRAM]),
		   XtWindow(spectro->toplevel[GRAM]),width,
		   spectro->yb[GRAM] + spectro->menu_height );
     return(1); /* generated resize event */
     
   }
   return(0); /* didn't change size of window */ 
}


/*********************** MenuItem Callbacks ************************/

/**********************************************************************/
/*    void dorec(Widget w, XtPointer client_data, XtPointercall_data)	*/
/**********************************************************************/

void dorec(Widget w, XtPointer client_data, XtPointer call_data) 
{
  int whichparam;
  whichparam = SB_ListSelectedItemPosition(w);
  if (!whichparam) return;

  if(whichparam == numRecParams+1 )  donerec(w, client_data, call_data);
  else inputrec(whichparam - 1);
}

/**********************************************************************/
/*    void doparam(Widget w, XtPointer client_data, XtPointercall_data)	  */
/**********************************************************************/

void doparam(Widget w, XtPointer client_data, XtPointer call_data)
{
  int whichparam, numParams = XtNumber(paramlist);
  XSPECTRO *spectro;

  spectro = (XSPECTRO *)client_data;

  whichparam = SB_ListSelectedItemPosition(w);
  if (!whichparam) return;

  /* selection box list starts at 1, deal with e DONE entry */

  if (whichparam == XtNumber(paramlist)) doneparam(w, client_data, call_data);
  else inputparams(spectro, whichparam - 1);
}

/**********************************************************************/
/* void doparamsAll(Widget w, XtPointer client_data, XtPointer)	  */
/**********************************************************************/

void doparamsAll(Widget w, XtPointer client_data, XtPointer call_data)
{
  int whichparam, i, numParams = XtNumber(paramlist);
  XSPECTRO *spectro;

  spectro = (XSPECTRO *)client_data;

  whichparam = SB_ListSelectedItemPosition(w);
  if (!whichparam) return;

  /* selection box list starts at 1, deal with e DONE entry */

  if (whichparam == XtNumber(paramlist)) doneparam(w, client_data, call_data);
  else for (i = 0; i < allspectros.count; i++)
    inputparams(allspectros.list[i], whichparam - 1);
}


/*********************************************************************/
/*			 inputrec(int index);			     */
/*********************************************************************/
void inputrec(int index)
{
  char str[100], t_str[100];
  Arg args[3];
  int n;
  XmString mstr;

   n = 0;
   sprintf(t_str,"%.1f",recParams[index].value);
   XtSetArg(args[n],XmNvalue,t_str); n++;
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,n);

   
   sprintf(str,"%s: (%.1f - %.1f)",
	   recParams[index].key, recParams[index].min, recParams[index].max);
   mstr = XmStringCreateLocalized(str);
   XtSetArg(args[0],XmNselectionLabelString,mstr);
   XtSetValues(input_dialog,args,1);
   XmStringFree(mstr);


   XtAddCallback(input_dialog,XmNokCallback,setrec,(XtPointer)index);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,NULL);

  HighlightInput (t_str, XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT));
   opendialog(input_dialog);

}

/*********************************************************************
 * writeParam()	
 *********************************************************************/

void writeParam(paramStruct param,  char *str)
{
   sprintf(str, "%s  %10.1f  %10.1f  %10.1f     %s",
	   param.key, param.min, param.max, param.value, param.desc);

}

/*********************************************************************
 *
 * void listrec()				     
 *
 * Prints the list of parameters for recording, and allows user
 * to change them.
 *
 ********************************************************************/

void listrec()
{
   Arg args[5];
   XmString *list;
   int i, n;
   char temp[200];

   /*
    * Create list
    */

   list = (XmString *) malloc( sizeof(XmString) * (numRecParams + 1));
   for (i = 0; i < numRecParams; i++) {
      writeParam(recParams[i], temp);
      list[i] = XmStringCreateLtoR(temp, XmSTRING_DEFAULT_CHARSET);
   }
   /* 
    * Add DONE to end 
    */
   
   sprintf(temp,"e                      Done");
   list[i] = XmStringCreateLtoR(temp, XmSTRING_DEFAULT_CHARSET);

   /*
    * Set widget
    */
      
   n = 0;
   XtSetArg(args[n], XmNlistItems, list); n++;
   XtSetArg(args[n], XmNlistItemCount, numRecParams + 1); n++;
   XtSetValues(rec_sb, args, n);
   for(i = 0; i < numRecParams; i++) XmStringFree(list[i]);
   free(list);
}

/*********************************************************************/
/*		  dolist(XSPECTRO *spectro)			     */
/*********************************************************************/
void dolist(XSPECTRO *spectro){
Arg args[5];
XmString *list;
int listcount,i,n;
char temp[200];


/* load the current values into parameter selection box */
/* note paramlist is one more than spectro->params, because
   the DONE entry isn't a parameter */

    listcount = XtNumber(paramlist) ;
    list = (XmString *) malloc( sizeof(XmString) * listcount);
    for(i = 0; i < listcount - 1; i++){
	if(i < 4){
	/* print hamming window size in ms, use float */
	sprintf(temp,"%s  %.1f",paramlist[i],spectro->hamm_in_ms[i]);
	}
	else{
	    sprintf(temp,"%s  %d",paramlist[i],spectro->params[i]);
	}
	list[i] = XmStringCreateLtoR(temp,XmSTRING_DEFAULT_CHARSET);
      }

/* 
 * print out DONE command with out a parameter value
 */

	sprintf(temp,"%s ",paramlist[i]);
	list[i] = XmStringCreateLtoR(temp,XmSTRING_DEFAULT_CHARSET);

    n = 0;
    XtSetArg(args[n], XmNlistItems, list );n++;
    XtSetArg(args[n], XmNlistItemCount,listcount);n++;
    XtSetValues(param_sb,args,n);
    for(i = 0; i < listcount; i++)
	  XmStringFree(list[i]);
    free(list);

}/* dolist */

/***********************************************************************/
/*	setrec(Widget w,XtPointer client_data,XtPointer call_data)     */
/***********************************************************************/
void setrec(Widget w,XtPointer client_data,XtPointer call_data){
/* check against min and max and set rec_param */
char *str,mess[50];
int n;
float newfloat;
Arg args[3];
int index;
XmString mstr;

index = (int) client_data;

   n = 0;
   XtSetArg(args[n],XmNvalue,&str); n++;
   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,n);
   newfloat = atof(str);
   
   if(newfloat >= recParams[index].min && newfloat <= recParams[index].max){
       recParams[index].value = newfloat;
       listrec();
       writetext("received input\n");
       
     }/* changed value */
   else{
	 /*out of range*/
	  sprintf(mess,"value out of range: %.1f",newfloat );
	  ShowOops(mess);
	  writetext("Value out of range.\n");
	  return;
	}/*out of range*/

   listrec();

   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
   XtUnmanageChild(input_dialog);


}/* end setrec */

/***********************************************************************/
/*    setparam(Widget w,XtPointer client_data,XtPointer call_data)	   */
/***********************************************************************/
void setparam(Widget w,XtPointer client_data,XtPointer call_data){
XSPECTRO *spectro;
char *str, mess[50];
int newvalue,n;
float newfloat;
Arg args[3];
XmString mstr;

   spectro = (XSPECTRO *)client_data;

   n = 0;
   XtSetArg(args[n],XmNvalue,&str); n++;
   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,n);

   /* first 4 params are float convert to number of samples here */

   if(sb_param.index == WC || sb_param.index == WD ||
      sb_param.index == WS || sb_param.index == WL){
     /*hamm in ms*/
     newfloat = atof(str);
     if(newfloat >= (float)sb_param.min && newfloat <= (float)sb_param.max){
       spectro->hamm_in_ms[sb_param.index] = newfloat;
       spectro->params[sb_param.index] = 
	      newfloat * spectro->spers / 1000.00 + .5;

       if(sb_param.index == WD && 
	spectro->params[sb_param.index] > spectro->params[SD] ){
       /* may need to change dft size if it is less than # of samples*/ 
	   if(newvalue > 16 && newvalue <= 32)
		 {spectro->params[SD] = 32;}
	   else if( newvalue > 32 && newvalue <= 64)
		 {spectro->params[SD] = 64;} 
	   else if( newvalue > 64 && newvalue <= 128)
		 {spectro->params[SD] = 128;} 
	   else if( newvalue > 128 && newvalue <= 256)
	     {spectro->params[SD] = 256;} 
	   else	 if( newvalue > 256 && newvalue <= 512)
	     {spectro->params[SD] = 512;} 
	   else	 if( newvalue > 512 && newvalue <= 1024)
	     {spectro->params[SD] = 1024;} 
	   else	 if( newvalue > 1024 && newvalue <= 2048)
	     {spectro->params[SD] = 2048;} 
	   else	 if( newvalue > 2048 && newvalue <= 4096)
	     {spectro->params[SD] = 4096;} 
	   else	 
	     {spectro->params[SD] = 4096;} 

      }/* if wd changed check dft size */

    }/* changed value */
    else{ 
    /* just a message no callbacks */
    sprintf(mess,"value out of range: %.1f\n",newfloat );
    ShowOops (mess);
    return;
/*    n = 0;
    XtSetArg(args[n], XmNmessageString,mstr); n++;
    mstr = XmStringCreateLocalized(mess);
    XtSetValues(oops,args,n);
    opendialog(oops);
    XmStringFree(mstr); */

    }/* value did change */
   }/*float value, hamm_in_ms*/
   else{
   newvalue = atoi(str);

      /* check to make sure DFT size is power of 2 */
      /* and that it is >= number of samples */
      if(sb_param.index == SD){
       if(newvalue == 16  || newvalue == 32  || newvalue == 64	||
	  newvalue == 128 || newvalue == 256 || newvalue == 512 || 
	  newvalue == 1024 || newvalue == 2048 || newvalue == 4096 &&
	  newvalue >= spectro->params[WD])
		 {spectro->params[sb_param.index] = newvalue;}
       }/* DFT size */

      else if(newvalue >= sb_param.min && newvalue <= sb_param.max){
	 spectro->params[sb_param.index] = newvalue;
       }
      else{
	 /*out of range*/
	  /* just a message no callbacks */
	  sprintf(mess,"value out of range: %d\n",newvalue );
	  ShowOops(mess);
	  return;

      }/* value	 didn't change */
   
   }/* change an integer paramenter */

   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
   XtUnmanageChild(input_dialog);

   dolist(spectro);
   writetext( "Received input\n");
  
}/* end setparam */

/***********************************************************************/
/*    donerec(Widget w,XtPointer client_data,XtPointer call_data)	 */
/***********************************************************************/

void donerec(Widget w, XtPointer client_data, XtPointer call_data) 
{
 XtUnmanageChild(rec_tl);
}


/***********************************************************************/
/*    doneparam(Widget w,XtPointer client_data,XtPointer call_data)	 */
/***********************************************************************/
void doneparam(Widget w,XtPointer client_data,XtPointer call_data) 
{
int i;

 XtUnmanageChild(param_tl);

/* keep track of whether spectrum parameter widget is active */
/* so whenever a waveform is deleted the parameter isn't left hanging*/
for( i = 0; i < allspectros.count; i++)
   allspectros.list[i]->param_active = 0;

}/* end doneparam*/

/************************************************************************/
/*		  CreateRecDialog(Widget parent)			*/
/************************************************************************/
void CreateRecDialog(Widget parent)	
{
Arg args[5];
Widget mainw, menu_bar;
int listcount,n;
XmString items_label;


/* popup list, used for changing spectrum parameters */
   n = 0;	
   XtSetArg(args[n], XmNdeleteResponse, XmUNMAP); n++;
   rec_tl = XtCreatePopupShell("record parameters",topLevelShellWidgetClass,
				    application,args,n);


    n = 0;
    mainw = XmCreateMainWindow(rec_tl, "mainw", args, n);
    XtManageChild(mainw);

    /* XmStringCreateLocalized includes linefeed*/
     n = 0;
    items_label =
    XmStringCreateLocalized(
    "SYM     MIN       MAX    VALUE    DESCRIPTION");
    XtSetArg(args[n], XmNlistLabelString, items_label );n++; 
    XtSetArg(args[n], XmNokLabelString,XmStringCreateLocalized("change")  );n++; 
    XtSetArg(args[n], XmNcancelLabelString,XmStringCreateLocalized("done")  );n++;
    /* otherewise crashes when nothing is selected and change is clicked*/
    XtSetArg(args[n], XmNmustMatch,True);n++;


   /* global widget used by all spectro and all windows */
    rec_sb = XmCreateSelectionBox(mainw,"recw",args,n);
    XtAddCallback(rec_sb,XmNcancelCallback,donerec,NULL);
    XtAddCallback(rec_sb,XmNokCallback,dorec,NULL);

    XtManageChild(rec_sb);

  XtUnmanageChild(XmSelectionBoxGetChild(rec_sb,XmDIALOG_HELP_BUTTON) );
  XtUnmanageChild(XmSelectionBoxGetChild(rec_sb,XmDIALOG_SELECTION_LABEL) );
  XtUnmanageChild(XmSelectionBoxGetChild(rec_sb,XmDIALOG_TEXT) );

  /* set up menu_bar so the user can just hit a key to change a param*/
  /*rec_buttons is global pointer to all entries on param menu*/
  listcount = numRecParams+1;
  rec_buttons  = (Widget *) malloc( sizeof(Widget) * listcount );

  menu_bar = XmCreateSimpleMenuBar(mainw,"rbar",args,0);
  XtManageChild(menu_bar);

 rec_menu = CreateOneMenu(menu_bar,"Parameters",
		       reclist, listcount,
		       rec_buttons, False, 1);

  /* client_data doesn't change so add callback here */
  XtAddCallback(rec_menu,XmNentryCallback,HandleRecMenu,NULL);

  XmMainWindowSetAreas( mainw, menu_bar, NULL, NULL, NULL, rec_sb);

 }/* end CreateRecDialog*/


/************************************************************************/
/*	   void CreateParamDialog(Widget parent)			*/
/************************************************************************/
void CreateParamDialog(Widget parent)	{

int n;
Arg args[5];
Widget mainw, menu_bar;
int listcount;
XmString items_label;

/* popup list, used for changing spectrum parameters */
  n = 0;
  /* can't get close and exit to just close the window 
  * XmUNMAP,XmDO_NOTHING*/
   XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
   param_tl = XtCreatePopupShell("spectrum parameters",
	      topLevelShellWidgetClass, application,args,n);
   
    n = 0;
    mainw = XmCreateMainWindow(param_tl, "mainw", args, n);
    XtManageChild(mainw);

    /* XmStringCreateLocalized includes linefeed*/
     n = 0;
    items_label =
    XmStringCreateLocalized(
    "SYM MIN  MAX    DESCRIPTION			       VALUE");
    XtSetArg(args[n], XmNlistLabelString, items_label );n++; 
    XtSetArg(args[n], XmNokLabelString,XmStringCreateLocalized("Change for current\nwaveform (default)")  );n++; 
    XtSetArg(args[n], XmNcancelLabelString,XmStringCreateLocalized("Done")  );n++;
    XtSetArg(args[n], XmNhelpLabelString,XmStringCreateLocalized("Change for all\nwaveforms")  );n++;
    /* otherewise crashes when nothing is selected and change is clicked*/
    XtSetArg(args[n], XmNmustMatch,True);n++;


   /* global widget used by all spectro and all windows */
    param_sb = XmCreateSelectionBox(mainw,"parw",args,n);
    XtAddCallback(param_sb,XmNcancelCallback,doneparam,NULL);

    XtManageChild (param_sb);

  XtUnmanageChild(XmSelectionBoxGetChild(param_sb,XmDIALOG_SELECTION_LABEL) );
  XtUnmanageChild(XmSelectionBoxGetChild(param_sb,XmDIALOG_TEXT) );

  /* set up menu_bar so the user can just hit a key to change a param*/
  /*param_buttons is global pointer to all entries on param menu*/
  listcount = XtNumber(paramlist);
  param_buttons	 = (Widget *) malloc( sizeof(Widget) * listcount );

  menu_bar = XmCreateSimpleMenuBar(mainw,"pbar",args,0);
  XtManageChild(menu_bar);

  param_menu = CreateOneMenu(menu_bar,"Parameters",
		       paramlist,listcount,
		       param_buttons, False, 1 );

  /* Callbacks are in HandleOptionsMenu (C change record parameters)*/

  XmMainWindowSetAreas( mainw, menu_bar, NULL, NULL, NULL, param_sb);

 }/* end CreateParamDialog*/



/************************************************************************/
/*	       void CreateWavList(Widget parent)			*/
/************************************************************************/
void CreateWavList(Widget parent)  {

int n, i, j;
Arg args[5];
XmString *list;
String *wavlist;
char str[20];
Widget mainw, menu_bar;
int listcount;
XmString items_label;
char tmpstr[200];

/* list for selecting a waveform*/
/*
   n = 0;
   XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);n++;	 
   XtSetArg(args[n], XmNdeleteResponse, XmUNMAP); n++;
   XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
   XtSetArg(args[n], XmNmustMatch,True);n++;
   wav_sb = XmCreateSelectionDialog(parent,"Wavs",args,n);
   
  XtAddCallback(wav_sb,XmNcancelCallback,cancelwavlist,NULL);

  XtUnmanageChild(XmSelectionBoxGetChild(wav_sb,XmDIALOG_HELP_BUTTON) );
  XtUnmanageChild(XmSelectionBoxGetChild(wav_sb,XmDIALOG_APPLY_BUTTON) );
  XtUnmanageChild(XmSelectionBoxGetChild(wav_sb,XmDIALOG_SELECTION_LABEL) );
  XtUnmanageChild(XmSelectionBoxGetChild(wav_sb,XmDIALOG_TEXT) );
*/


/* popup list, used for changing spectrum parameters */
  n = 0;
  /* can't get close and exit to just close the window 
  * XmUNMAP,XmDO_NOTHING*/
   XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
   wav_tl = XtCreatePopupShell("wavlist",
	      topLevelShellWidgetClass, parent,args,n);

   
    n = 0;
    mainw = XmCreateMainWindow(wav_tl, "mainw_wav", args, n);
    XtManageChild(mainw);

    list = (XmString *) malloc( sizeof(XmString) * allspectros.count);
    wavlist = (String *) malloc( sizeof(String) * allspectros.count);

      for(i = 0; i < allspectros.count; i++)
	{
	  if (i < 10)
	    sprintf (str, "%d  ", i);  /* Label waveform with number */
	  else   /* Label each waveform with a char after 10 */
	    sprintf (str, "%c  ", (char) ((int) 'a' + i-10)); 

	  strcpy (tmpstr, allspectros.list[i]->wavename);
	  wavlist[i] = strcat(str, tmpstr);  

	  list[i] = XmStringCreateLocalized (wavlist[i]);
	}
/* Convert back into char string */
for (i=0; i<allspectros.count; i++)
  XmStringGetLtoR (list[i], "", &wavlist[i]);

    /* XmStringCreateLocalized includes linefeed*/
     n = 0;
    items_label = XmStringCreateLocalized("Press Number or Select Waveform:");
    XtSetArg(args[n], XmNlistLabelString, items_label );n++; 
    XtSetArg(args[n], XmNokLabelString,XmStringCreateLocalized("Change current waveform")  );n++; 
    /* otherewise crashes when nothing is selected and change is clicked*/
    XtSetArg(args[n], XmNmustMatch,True);n++;
    XtSetArg(args[n], XmNlistItems, list );n++;
    XtSetArg(args[n], XmNlistItemCount, allspectros.count);n++;

   /* global widget used by all spectro and all windows */
    wav_sb = XmCreateSelectionBox(mainw,"wavw",args,n);
    XtAddCallback(wav_sb,XmNcancelCallback,cancelwavlist,NULL);

    XtManageChild (wav_sb);

  XtUnmanageChild(XmSelectionBoxGetChild(wav_sb,XmDIALOG_SELECTION_LABEL) );
  XtUnmanageChild(XmSelectionBoxGetChild(wav_sb,XmDIALOG_TEXT) );
  XtUnmanageChild(XmSelectionBoxGetChild(wav_sb,XmDIALOG_HELP_BUTTON) );
  XtUnmanageChild(XmSelectionBoxGetChild(wav_sb,XmDIALOG_APPLY_BUTTON) );

  /* set up menu_bar so the user can just hit a key to change a param*/
  /* wav_buttons is global pointer to all entries on wav menu*/
  listcount = allspectros.count;
  wav_buttons	 = (Widget *) malloc( sizeof(Widget) * listcount );

  menu_bar = XmCreateSimpleMenuBar(mainw,"bar_wav",args,0);
  XtManageChild(menu_bar);

  wav_menu = CreateOneMenu(menu_bar,"WavList",
			   wavlist,listcount,
			   wav_buttons, False, 1 );

  XmMainWindowSetAreas( mainw, menu_bar, NULL, NULL, NULL, wav_sb);

 /* Clean up memory */
 for(i = 0; i < listcount; i++)
   XmStringFree(list[i]);
 free(list);
}/* end CreateWavList*/


/*********************************************************************/
/*		  void dowavlist()				     */
/*********************************************************************/
void dowavlist(){
  XmString *list;
  Arg args[5];
  int i,n;

    list = (XmString *) malloc( sizeof(XmString) * allspectros.count);
    for(i = 0; i < allspectros.count; i++){
	list[i] = XmStringCreateLtoR(
	allspectros.list[i]->wavename,XmSTRING_DEFAULT_CHARSET);
      }

    n = 0;
    XtSetArg(args[n], XmNlistItems, list );n++;
    XtSetArg(args[n], XmNlistItemCount, allspectros.count);n++;
    XtSetValues(wav_sb,args,n);
    for(i = 0; i < allspectros.count; i++)
	  XmStringFree(list[i]);
    free(list);

}/* end dowavlist */


/**********************************************************************/
/*    void dowav(Widget w, XtPointer client_data, XtPointercall_data)	  */
/**********************************************************************/

void dowav(Widget w, XtPointer client_data, XtPointer call_data) 
{
 int whichparam;
 XSPECTRO *spectro, *oldspectro;

 /*expose from popup causes exposed window to end up on top */
 oldspectro = (XSPECTRO *)client_data;
 XSync(XtDisplay(oldspectro->toplevel[0]),False);
 XSync(XtDisplay(oldspectro->toplevel[0]),False);
 XSync(XtDisplay(oldspectro->toplevel[0]),False);
 XSync(XtDisplay(oldspectro->toplevel[0]),False);

 /*client_data is NULL */
 whichparam = SB_ListSelectedItemPosition(w) - 1;

 FindWav(spectro, whichparam);

 cancelwavlist(w, NULL, NULL);
}/* end dowav */


/*******************************************************************
******  FindWav (XSPECTRO *, int)  ***************************
********************************************************************/

void FindWav (XSPECTRO *spectro, int whichparam)
{

 /* raise selected waveform's windows */ 
 spectro = allspectros.list[whichparam];
 raisewav(spectro);
 raisewav(spectro);
 XtRemoveAllCallbacks(wav_sb, XmNokCallback);

}


/*******************************************************************
******  shiftCurrentWav (XSPECTRO, int)  ***************************
********************************************************************/

void shiftCurrentWav(XSPECTRO *spectro, int step)
{
   register int i, current, next;

   for (i = 0; i < allspectros.count; i++) 
      if (allspectros.list[i] == spectro) {
	 current = i; break;
      }

   next = (current + step) % allspectros.count;
   if (next < 0) next += allspectros.count;

   spectro = allspectros.list[next];
   raisewav(spectro);
   raisewav(spectro);
}

 
/**********************************************************************/
/*void cancelwavlist(Widget w, XtPointer client_data, XtPointer call_data)*/
/**********************************************************************/
void cancelwavlist (Widget w, XtPointer client_data, XtPointer call_data)
{
 XtUnmanageChild(wav_tl);
 XtRemoveAllCallbacks(wav_sb, XmNokCallback);
 XtRemoveAllCallbacks(wav_menu,XmNentryCallback);
}

/**********************************************************************/
/*	      void CreateInputDialog(Widget parent);		      */
/* global widget used by everyone */
/**********************************************************************/
void CreateInputDialog(Widget parent)
{
  Arg args[6];
  int n;

  n = 0;

  /* kill all interaction except with text application */

  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);n++;
  XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;/* SGI */
  XtSetArg(args[n], XmNautoUnmanage, False); n++;

/* used when getting input for averaging */

  XtSetArg(args[n], XmNhelpLabelString,XmStringCreateLocalized("Done do it"));n++; 
  input_dialog = XmCreatePromptDialog(parent,"input",args,n);

  XtUnmanageChild(XmSelectionBoxGetChild(input_dialog, 
					 XmDIALOG_HELP_BUTTON));

/* eliminate close from menu, pg. 12-15 osf prog. guide*/
/* in order to insure all callbacks are removed */

  n = 0; 
  XtSetArg(args[n], XmNmwmFunctions,33); n++;   /*VMS*/
  XtSetValues(XtParent(input_dialog), args, n);

  XtAddEventHandler(input_dialog, VisibilityChangeMask,
		    True, (XtEventHandler) doraise, NULL);

  /* use XtAddCallback for specific parameters*/
}

/**********************************************************************/
/*		   void CreateFileSB(Widget parent);		      */
/* global widget used by everyone                                     */
/**********************************************************************/

void CreateFileSB(Widget parent)
{
  Arg args[5];
  int n;

  n = 0;

  XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++; /* SGI */
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL); n++;

  filesb_dialog = XmCreateFileSelectionDialog(parent, "filesb", args, n);
  
  XtUnmanageChild(XmFileSelectionBoxGetChild(filesb_dialog,
					     XmDIALOG_HELP_BUTTON));

  /* 
   * eliminate close from menu, pg. 12-15 osf prog. guide
   * in order to insure all callbacks are removed 
   */

  n = 0; 
  XtSetArg(args[n], XmNmwmFunctions, 33); n++; /*VMS*/
  XtSetValues(XtParent(input_dialog), args, n);

  /* use manage and unmanage, use XtAddCallback for specific parameters*/

}

/****************************************************************/
/*		   CreateWarning(Widget parent)			*/
/****************************************************************/

void CreateWarning(Widget parent)
{
  Arg args[7];
  int n;
  XmString ts[2];

  ts[0] = XmStringCreateLocalized("New");
  ts[1] = XmStringCreateLocalized("ATTENTION");

  n = 0;
  XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY);n++;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);n++;
  XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;/* SGI */
  XtSetArg(args[n], XmNautoUnmanage, False); n++;
  XtSetArg(args[n], XmNhelpLabelString, ts[0]); n++;
  XtSetArg(args[n], XmNdialogTitle, ts[1]); n++;
  /*question = XmCreateQuestionDialog(parent,"question",args,n);*/
  /* don't like the question mark pixmap */
  warning = XmCreateWarningDialog(parent,"warning",args,n);
  warning2 = XmCreateWarningDialog(parent,"warning2",args,n);
  
  XtAddEventHandler(warning,VisibilityChangeMask,
		    True, (XtEventHandler) doraise,NULL);
  XtAddEventHandler(warning2,VisibilityChangeMask,
		    True, (XtEventHandler) doraise,NULL);
  n = 0;
  XtSetArg(args[n], XmNshowAsDefault,(Dimension)2);n++; 
  XtSetValues(XmMessageBoxGetChild(warning,XmDIALOG_OK_BUTTON),args,n);
  XtSetValues(XmMessageBoxGetChild(warning2,XmDIALOG_OK_BUTTON),args,n);
  
  XtUnmanageChild(XmMessageBoxGetChild(warning,XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(warning2,XmDIALOG_HELP_BUTTON));

  XmStringFree(ts[0]);
  XmStringFree(ts[1]);
}

/****************************************************************/
/*		   CreateOops(Widget parent)			*/
/****************************************************************/

void CreateOops(Widget parent)
{
  int n;
  Arg args[7];
  XmString ts;

  ts = XmStringCreateLocalized("OOPS");
  n = 0;
  XtSetArg(args[n], XmNresizePolicy, XmRESIZE_ANY);n++;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);n++;
  XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;/* SGI */
  XtSetArg(args[n], XmNdialogTitle, ts); n++;
  
  oops = XmCreateWarningDialog(parent,"OOPS",args,n);
  
  XtAddEventHandler(oops,VisibilityChangeMask,
		    True, (XtEventHandler) doraise, NULL);
  
  XtUnmanageChild(XmMessageBoxGetChild(oops,XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(oops,XmDIALOG_CANCEL_BUTTON));

  XmStringFree(ts);
}


/********************************************************************
 *
 * void wavgram(numwavs,goodnames,argv)
 *
 ********************************************************************/

void wavgram(int numwavs, int *goodnames, char **argv, int swap)
{
  int s;
  XSPECTRO *spectro;

  spectro = (XSPECTRO *) malloc( sizeof(XSPECTRO));
  spectro->spectrogram = 1;
  spectro->swap = swap;
  spectro->devwidth = 1024;
  spectro->devheight = 768;
  spectro->hchar = 13;
  spectro->wchar = 13;

  for(s = 0; s < numwavs; s++){
    add_spectro(spectro,"xkl_defs.dat",argv[goodnames[s]]);
    delete_wave(spectro);
  }
  exit(1);
}

/*************************************************************************
 *
 * void xklPlay(spectro, window)
 *
 *************************************************************************/

void xklPlay(XSPECTRO *spectro, int window)
{
  char tmp[256];
  float rate, h, t;

  findseg(spectro, window);

  /*
   * Set sampling rate.  Play will return actual sampling rate
   */

  rate = spectro->spers;
  //  play(spectro->startplay, &rate, spectro->sampsplay, spectro->swap);
  //  return;  // work procs not working with XtAppPeekEvent()

// set up for audio output
  switch (play(spectro->startplay, &rate, spectro->sampsplay, spectro->swap, 0)) {

  case 0:  // success, install work proc for play loop
// printf("install\n");
      if (spectro->spers != rate) {
		switch (spectro->segmode) {
			case 0:
				sprintf(tmp, "Playing file at %g Hz\n", rate);
				break;
			case 1:
				sprintf(tmp, "Playing w->e at %g Hz\n", rate);
				break;
			case 2:
				if (spectro->clickedWinTime > spectro->savetime) {
					h = spectro->savetime;
					t = spectro->clickedWinTime;
				} else {
					h = spectro->clickedWinTime;
					t = spectro->savetime;
				}
				sprintf(tmp, "Playing from %.1f to %.1f at %g Hz\n", h, t, rate);
				break;
		}
	    writetext(tmp);
      }
      playerID = XtAppAddWorkProc(app_context, PlayLoop, NULL);
      break;

  case -1: // error, exit
// printf("error\n");
      KillPlay();
      break;

  }

}

/*************************************************************************
 *
 * void xklRecord(spectro)
 *
 *************************************************************************/

void xklRecord(XSPECTRO *spectro)
{
#if defined(sgi)
  XmString mstr;
  int n;
  Arg args[5];

   /*
    * in this case assume recording is finshed
    * do this when recording is terminated
    * set flag so segtowav knows what to write 
    */

   spectro->wwav = 2;

   XtSetArg(args[0],XmNvalue,spectro->wavename);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized("filename: will append .wav");
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   n=0;
   XtSetArg(args[n],XmNtitle,spectro->wavename); n++;
   XtSetValues(XtParent(input_dialog),args,n);
   XtAddCallback(input_dialog,XmNokCallback,segtowav,(XtPointer)spectro);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,(XtPointer)spectro);
   opendialog(input_dialog);
#else
  XmString mstr;
  int n;
  Arg args[5];

  char tmp[256];
  //short *wave;
  //float rate;
  //int nsamps;

   XSync(XtDisplay(cmdw),False); /* Flush queue for more veridical recording */

  /*
   * Set sampling rate.  Record will return actual sampling rate
   */
  //rate=recParams[REC_RATE].value;
  //rate = spectro->spers;
  //spectro->iwave = record(&rate, recParams[REC_DUR].value,&nsamps, spectro->swap);
  //spectro->spers = rate;
  //spectro->sampsplay = nsamps;
  //sprintf(tmp, "After record!\n");
  //fprintf(stderr,"%s",tmp);
  /*
   * Tell user what the sampling rate was.
   */

  /*
   * Store waveform into a file.
   */

  //spectro->wwav = 2;
  XtSetArg(args[0], XmNvalue, spectro->wavename);
  XtSetValues(XmSelectionBoxGetChild(input_dialog, XmDIALOG_TEXT), args, 1);
  mstr = XmStringCreateLocalized("Filename: will append .wav");
  n=0;
  XtSetArg(args[n], XmNselectionLabelString, mstr); n++;
  XtSetValues(input_dialog, args, n);
  XmStringFree(mstr);
  n=0;
  XtSetArg(args[n], XmNtitle, spectro->wavename); n++;
  XtSetValues(XtParent(input_dialog), args, n);
  //XtAddCallback(input_dialog, XmNokCallback, segtowav, (XtPointer) spectro);
  XtAddCallback(input_dialog, XmNokCallback, rectofile, (XtPointer) spectro);
  XtAddCallback(input_dialog, XmNcancelCallback, cancelinput, (XtPointer) spectro);
  opendialog(input_dialog);
  /* sprintf(tmp, "Dialog open!\n");
  writetext(tmp);*/
#endif
}



/*************************************************************************
 *
 * void do_usage()						 
 *
 *************************************************************************/

void do_usage()
{
 fprintf(stderr,"    usage:  xkl [-n] [-s] [-g] [filename(s)])\n");
 fprintf(stderr,"	     -s (swab) to swap .wav bytes on big endian \n");
 fprintf(stderr,"	     -n no .wav file, startup in record/synthesize mode\n");
 fprintf(stderr,"	     -g batch creation of .gram files from .wav\n");
 exit(0);
}

/************************************************************************
 *
 * void doraise(w, unused, event, continue_to_dispatch)		 
 *
 * Raise input or warning dialog if it gets obscured.
 *
 *************************************************************************/

void doraise(Widget w, XtPointer unused, XVisibilityEvent *event, 
	     Boolean *continue_to_dispatch)
{
   if (event->state == VisibilityFullyObscured)
      XRaiseWindow(XtDisplay(XtParent(w)), XtWindow(XtParent(w)));
   *continue_to_dispatch = True;
}



/*************************************************************************/
/*	     void undomap(XEvent *pevent)				 */
/*************************************************************************/

void undomap(XEvent *pevent) {

/* 
 * This is called when a graphics window is iconified,a flag is set so 
 * the window is not updated while it is iconified 
 */

  int i,s;
  /* find which window */
  for (s =0; s < allspectros.count; s++) {
    for (i=0; i < NUM_WINDOWS; i++) {
      if(pevent->xunmap.window == XtWindow(allspectros.list[s]->toplevel[i]) ){
	allspectros.list[s]->normstate[i] = 0;
	/* get out of here */
	i = NUM_WINDOWS; s = allspectros.count;
      }
    }
  }
} /* end undomap */



/*************************************************************************/
/*		     void domap(XEvent *pevent)				 */
/*************************************************************************/
void domap(XEvent *pevent){
/* this is called when window is normalized
after being an icon, the window is updated and a flag is 
set so other updates are processed
*/

  int i,s;

  /* find which window */
  for(s =0; s < allspectros.count; s++){
    for(i=0; i < NUM_WINDOWS; i++){
      if(pevent->xmap.window ==
	 XtWindow(allspectros.list[s]->toplevel[i]) ){
	if(allspectros.list[s]->normstate[i] != 1){
	if( i == SPECTRUM)
	  allspectros.list[s]->history = 0;
	allspectros.list[s]->normstate[i] = 1;
	update(allspectros.list[s]->info[i],
	       allspectros.list[s], i); 
	}     
	/* get out of here */
	i = NUM_WINDOWS; s = allspectros.count;
      }
    }
  }
} /* end domap*/


/******************* PostScript Routines *********************/


/******************************************************************
 *
 * void dopsfile(Widget w, XtPointer client_data, XtPointer call_data)
 *
 *
 * Get a name for a postscript file.  PStype is a parameter that
 * specifies if it is a 1 / page or a 4 / page PS file.
 *
 ******************************************************************/

void dopsfile(Widget w, XtPointer client_data,
	      XtPointer call_data)
{
   char *temp, mess[550], ps_name[500];  
   XSPECTRO *spectro;
   XmString mstr;
   int n;
   Arg args[5];
   FILE *fp;
   XmFileSelectionBoxCallbackStruct *c_data = 
     (XmFileSelectionBoxCallbackStruct *) call_data;
   
   /*
    * Check to see if file already exists or if file can be opened
    * for writing.
    */
   
   spectro = (XSPECTRO *) client_data;

   XtUnmanageChild(filesb_dialog);
   XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);
   mstr = XmStringCopy(c_data->value);
   
   XmStringGetLtoR(mstr,XmSTRING_DEFAULT_CHARSET,&temp);
   XmStringFree(mstr);
   strcpy(ps_name,temp); XtFree(temp);
   
   if (!(fp = fopen(ps_name,"r"))) {  /* File does not exist */

      if (!(fp = fopen(ps_name,"w"))) { /* Can't write to file */
	 sprintf(mess,"Can't open:%s.\n",ps_name);
	 ShowOops(mess);
	 return; 
      }
      
      fclose(fp); /* No overwrite, file can be written  */
      strcpy(spectro->tmp_name, ps_name);
      openps(spectro); 
   } 
  else {   /* the file did exist so popup overwrite query*/
     
     fclose(fp);     /* was already opened for read */
     
     if (!(fp = fopen(ps_name,"w"))){ /* Can't write to file */
	sprintf(mess,"Can't open:%s.\n",ps_name);
	ShowOops(mess);
	return; 
     }
     
     fclose(fp);   /* was opened for write, and file existed */
     
     /* 
      * File already exists so popup overwrite query 
      */
     
     strcpy(spectro->tmp_name,ps_name);
     sprintf(mess,"OVERWRITE %s?",ps_name);
     mstr = XmStringCreateLocalized(mess);
     n = 0;
     XtSetArg(args[n], XmNmessageString,mstr); n++;
     XtSetValues(warning,args,n);
     XmStringFree(mstr);
     mstr = XmStringCreateLocalized("OK");
     XtSetArg(args[0], XmNokLabelString,mstr);
     XtSetValues(warning,args,1);
     XmStringFree(mstr);
     XtRemoveAllCallbacks(input_dialog, XmNokCallback);
     XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);  
     XtAddCallback(warning,XmNokCallback,dooverwriteps,(XtPointer)spectro);
     XtAddCallback(warning,XmNcancelCallback,cancelwarning,NULL);
     opendialog(warning);
  }

   current_dir = XmStringCopy (c_data->dir);
}

/******************************************************************
 *
 * void dooverwriteps(Widget w, XtPointer client_data, XtPointer call_data)
 *
 *
 * Callback for overwrite of PS dialog box
 *
 ******************************************************************/

void dooverwriteps(Widget w, XtPointer client_data, XtPointer call_data)
{
   XSPECTRO *spectro;
   
   XtUnmanageChild(warning);
   spectro = (XSPECTRO *) client_data;
   
   XtRemoveAllCallbacks(warning, XmNokCallback);
   XtRemoveAllCallbacks(warning, XmNcancelCallback);
   
   openps(spectro);
}

/******************************************************************
 *
 * void openps(XSPECTRO *spectro)
 *
 *
 * Open a postscript file.  All checking for file overwite, ability
 * to write file, etc. has already been done.  PStype is a parameter
 * specifying what type of PS file it is: 1/page or 4/page.
 *
 ******************************************************************/

void openps(XSPECTRO *spectro)
{
   char text[100];
   char mess[630];

   /*
    * After opening Postscript file, turn on menu item to save to file 
    */

   if (PStype == FULLPAGE) {
      PSfull_fp = fopen(spectro->tmp_name, "w");
      fprintf(PSfull_fp, "%c!PS\n",'%');
      strcpy(text, "full page");
      activateMenuItem(spectro, '*');
   }
   else {
      PS4_fp = fopen(spectro->tmp_name, "w");
      fprintf(PS4_fp,"%c!PS\n",'%');
      quadcount = 0;
      strcpy(text, "4/page");
      activateMenuItem(spectro, '#');
   }
   
   PStype = PSNONE;
   sprintf(mess,"Opened %s postscript file: %s.\n",text, spectro->tmp_name);
   writetext(mess);

}



/******************************************************************
 *
 * void writeps(XSPECTRO *spectro, FILE *fp)
 *
 *
 * Write the postscript file based on window type. The PS file should
 * be open.
 *
 ******************************************************************/

void writeps(XSPECTRO *spectro, FILE *fp)
{
   char mess[700], type[100];
   
   if(spectro->ps_window == GRAM) {
      postgvs(spectro, fp);
      strcpy(type, "spectrogram");
   }
   else if (spectro->ps_window == SPECTRUM) {
      printspectrum(spectro, fp);
      strcpy(type, "spectrum");
   }
   else{
      printwav(spectro,spectro->ps_window, fp);
      strcpy(type, "waveform");
   }
   sprintf(mess,"Full-size %s copied to postscript file:\n\t%s.\n",
	   type, spectro->tmp_name);
   writetext(mess);
}



/******************************************************************
 *
 * void activateMenuItem(XSPECTRO *spectro, char accelerator)
 *
 *
 * Turn a menu item on (assuming it has been turned off).  The item
 * is determined by looking at the entries array.
 *
 ******************************************************************/

void activateMenuItem(XSPECTRO *spectro, char accelerator)
{
  register int i, j, s, n, count;
  Arg args[1];

  for (i = 0; i < NUM_WINDOWS; i++){ /* all windows */
    count = 0;
    for (s = 0; s < numPulldown; s++) { /* all menus */
      for (n = 0; n < num_entries[s]; n++) { /* all entries */
	if (entries[count][0] == accelerator) {
/* apply to all spectros (support multi-waveform saves to same PS file) */
	  for (j=0; j<allspectros.count; j++)
	    XtSetSensitive(allspectros.list[j]->buttons[i][count], True);
	}
	count++;
      }
    }
  }
}

/******************************************************************
 *
 * void deActivateMenuItem(XSPECTRO *spectro, char accelerator)
 *
 *
 * Turn a menu item off (assuming it has been turned on).  The item
 * is determined by looking at the entries array.
 *
 ******************************************************************/

void deActivateMenuItem(XSPECTRO *spectro, char accelerator)
{
  register int i, j, s, n, count;
  Arg args[1];

  for (i = 0; i < NUM_WINDOWS; i++){ /* all windows */
    count = 0;
    for (s = 0; s < numPulldown; s++) { /* all menus */
      for (n = 0; n < num_entries[s]; n++) { /* all entries */
	if (entries[count][0] == accelerator) {
/* apply to all spectros */
	  for (j=0; j<allspectros.count; j++)
	    XtSetSensitive(allspectros.list[j]->buttons[i][count], False);
	}
	count++;
      }
    }
  }
}



/**************************************************************/
/* avgval(Widget w,XtPointer client_data,XtPointer call_data)   */
/**************************************************************/
void avgval(Widget w,XtPointer client_data, XtPointer call_data){

/* receive values from avg_dialog for spectral averaging */

XSPECTRO *spectro;
Arg args[3];
char t_str[500], str[500];
char *temp;
XmString mstr;


   spectro = (XSPECTRO *) client_data;

   XtSetArg(args[0],XmNvalue,&temp);
   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   strcpy(t_str,temp);

  /* if exits in file and there are enough samples for dft */
  

   spectro->avgtimes[spectro->avgcount] = (int)(atof(t_str) + .5);

  /*just for 'A' */
   spectro->avgcount++;
   if(spectro->avgcount ==  AVGLIMIT){
     spectro->avgcount--;
     XtUnmanageChild(input_dialog);
     XtUnmanageChild(
       XmFileSelectionBoxGetChild(input_dialog,XmDIALOG_HELP_BUTTON) );
 
     XtRemoveAllCallbacks(input_dialog, XmNokCallback);
     XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
     XtRemoveAllCallbacks(input_dialog, XmNhelpCallback);
     getavg(spectro);            

  }/* reached limit calculate avg. */

  else{
   sprintf(str,"time %d",spectro->avgcount + 1);
   mstr = XmStringCreateLocalized(str);
   XtSetArg(args[0],XmNselectionLabelString,mstr);
   XtSetValues(input_dialog,args,1);
   XmStringFree(mstr);
   HighlightInput (t_str, XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT));
 }/* get another time */
}/* end avgval */



/********************************************************************/
/*   cancelavg(Widget w, XtPointer client_data, XtPointer call_data)   */
/********************************************************************/

void  cancelavg(Widget w, XtPointer client_data, XtPointer call_data){
/* used to cancel input dialog that uses 3 buttons instead of 2 */
   XtUnmanageChild(input_dialog);
   XtUnmanageChild(
       XmFileSelectionBoxGetChild(input_dialog,XmDIALOG_HELP_BUTTON) );
   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
   XtRemoveAllCallbacks(input_dialog, XmNhelpCallback);

 writetext("cancel \n");
}/* end cancelavg*/



/********************************************************************/
/*   doneavg(Widget w, XtPointer client_data, XtPointer call_data)   */
/********************************************************************/

void doneavg(Widget w, XtPointer client_data, XtPointer call_data)
{
  XSPECTRO *spectro;

  /* done button is going to be unmanaged so make */
  /* another button active */
  XmProcessTraversal(input_dialog,XmTRAVERSE_HOME);

  spectro = (XSPECTRO *) client_data;
  XtUnmanageChild(input_dialog);
  XtUnmanageChild(XmFileSelectionBoxGetChild(input_dialog,
					     XmDIALOG_HELP_BUTTON));
  XtRemoveAllCallbacks(input_dialog, XmNokCallback);
  XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
  XtRemoveAllCallbacks(input_dialog, XmNhelpCallback);
  
  if(spectro->avgcount >= 1) {
    spectro->avgcount --;
    getavg(spectro);   
  }
}/* end doneavg*/



/**************************************************************/
/* aval(Widget w,XtPointer client_data,XtPointer call_data)   */
/**************************************************************/

void aval(Widget w,XtPointer client_data, XtPointer call_data)
{

/* receive values from avg_dialog for spectral averaging */

  XSPECTRO *spectro;
  Arg args[3];
  char str[500];
  char *temp;
  XmString mstr;

 /* get two time values for 'a' averaging */

  spectro = (XSPECTRO *) client_data;
  
  XtSetArg(args[0],XmNvalue,&temp);
  XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
  strcpy(str,temp);

  if (spectro->avgcount == 1){
    spectro->avgtimes[1] = (int)(atof(str) + .5); 
    XtUnmanageChild(input_dialog);     
    XtRemoveAllCallbacks(input_dialog, XmNokCallback);
    XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
    getavg(spectro);            
  }
  else if(spectro->avgcount == 0){
    spectro->avgtimes[0] = (int)(atof(str) + .5);
    mstr = XmStringCreateLocalized("end time:");
    XtSetArg(args[0],XmNselectionLabelString,mstr);
    XtSetValues(input_dialog,args,1);
    XmStringFree(mstr);
    spectro->avgcount++;
    HighlightInput (str, XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT));
  }
} /* end aval */



/*******************************************************************/
/*void  readparams(Widget w, XtPointer client_data, XtPointer call_data) */
/*******************************************************************/

void  readparams(Widget w, XtPointer client_data, XtPointer call_data){
  /* add a new wave form and update allspectros*/
  Arg args[3];
  char *temp,mess[550],str[500];
  XSPECTRO *spectro;
  int i;
  FILE *fp;
  int numparams;
  
/* read a file of spectro->params  that has been stored */

  XmFileSelectionBoxCallbackStruct *c_data =  ( XmFileSelectionBoxCallbackStruct *) call_data; 
   XtUnmanageChild(filesb_dialog);

   spectro = (XSPECTRO *) client_data;

   XtSetArg(args[0],XmNvalue,&temp);
   XtGetValues(XmFileSelectionBoxGetChild(filesb_dialog,XmDIALOG_TEXT),args,1);

   strcpy(str,temp);
   XtFree(temp);
  
   XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);

 
#ifdef VMS
    for(i = strlen(str) - 1; i >= 0; i-- )
         if(str[i] == ';') break;

         str[i] = '\000';
#endif
   
  if(!(fp = fopen(str,"r")) ||
     (strcmp(&str[strlen(str) - 6 ],"params") != 0)){
       sprintf(mess,"can't open: %s\n",str);
       ShowOops(mess);
       return ;   
     }/* can't find file */


     numparams = XtNumber(spectro->params);
    /* read first 4 floats */
      for(i = 0; i < 4; i++){
       if(!fscanf(fp,"%f\n",&spectro->hamm_in_ms[i])){
            writetext("error reading file\n");
            return;
	  }/* can't read value */
       spectro->params[i] = 
            spectro->hamm_in_ms[i]  * spectro->spers / 1000.00 + .5;
       }
      for(i = 4; i <  numparams; i++){

         if(!fscanf(fp,"%d\n",&spectro->params[i])){
         writetext("error reading file\n");
         return;
	  } /* can't read value */ 
       }
   fclose(fp);
   sprintf(mess,"read: %s\n",str);
   writetext(mess);
   dolist(spectro);/* update parameter list */

  current_dir = XmStringCopy (c_data->dir);

}/* end readparams */



/********************************************************************/
/*void  writeparams(Widget w, XtPointer client_data, XtPointer call_data) */
/********************************************************************/

void  writeparams(Widget w, XtPointer client_data, XtPointer call_data){
/* add a new wave form and update allspectros*/
Arg args[3];
char *temp,fname[520],mess[550],str[500];
XSPECTRO *spectro;
int i,n,numparams;
FILE *fp;
XmString mstr;
extern XmString save_dir;
XmFileSelectionBoxCallbackStruct *c_data = (XmFileSelectionBoxCallbackStruct *) call_data;

//   XtUnmanageChild(input_dialog);
   XtUnmanageChild(filesb_dialog);
   spectro = (XSPECTRO *) client_data;
   save_dir = XmStringCopy(c_data->dir);

   XtSetArg(args[0],XmNvalue,&temp);
//   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   XtGetValues(XmFileSelectionBoxGetChild(filesb_dialog,XmDIALOG_TEXT),args,1);
   strcpy(str,temp);
   XtFree(temp);

//   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
//   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);

 
  sprintf(fname,"%s.params",str);


  if(!(fp = fopen(fname,"r"))   ){
      /* can write with impunity */



    if(!(fp = fopen(fname,"w")) ){
       sprintf(mess,"can't open: %s\n",fname);
       ShowOops(mess);
       return ;   
     }/* can't open file */


    else{
   /* if changes are made, change dowriteparams as well*/
      numparams = XtNumber(spectro->params);
      for( i = 0; i < 4; i ++)
           {fprintf(fp,"%f \n",spectro->hamm_in_ms[i]); }
      for( i = 4; i < numparams; i ++)
           {fprintf(fp,"%d \n",spectro->params[i]); }
      fclose(fp);
      sprintf(mess,"wrote: %s\n",fname);
      writetext(mess);
    }/*wrote file*/

  }/* don't have to worry about overwrite */


  /* the file did exist so popup overwrite query*/
  else{
 /* was opened for read */
  fclose(fp);
    if(!(fp = fopen(fname,"w")) ){
       sprintf(mess,"can't open: %s\n",fname);
       ShowOops(mess);
       return ;   
     }/* can't open file */


   else{
    /* was opened for write */
   fclose(fp);   
   strcpy(spectro->tmp_name,fname);
   sprintf(mess,"OVERWRITE %s ??",fname);
   mstr = XmStringCreateLocalized(mess);
   n = 0;
   XtSetArg(args[n], XmNmessageString,mstr); n++;
   XtSetValues(warning,args,n);
   XmStringFree(mstr);
   mstr = XmStringCreateLocalized("OK");
   XtSetArg(args[0], XmNokLabelString,mstr);
   XtSetValues(warning,args,1);
   XmStringFree(mstr);
   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);  
   XtAddCallback(warning,XmNokCallback,dowriteparams,(XtPointer)spectro);
   XtAddCallback(warning,XmNcancelCallback,cancelwarning,NULL);
   opendialog(warning);
   }/* popup warning window */
  }/*could read file check for write or overwrite */
}/*end  writeparams*/



/********************************************************************/
/*void  dowriteparams(Widget w, XtPointer client_data, XtPointer call_data) */
/********************************************************************/

void dowriteparams(Widget w,XtPointer client_data, XtPointer call_data)
{
  XSPECTRO *spectro;
  int i, numparams;
  FILE *fp;
  char mess[520];
  
  XtUnmanageChild(warning);
  spectro = (XSPECTRO *) client_data;

  XtRemoveAllCallbacks(warning, XmNokCallback);
  XtRemoveAllCallbacks(warning, XmNcancelCallback);


  /* file has already been checked so go ahead and open it */
  fp = fopen(spectro->tmp_name,"w");

  numparams = XtNumber(spectro->params);
  for( i = 0; i < 4; i ++) {
    fprintf(fp,"%f \n",spectro->hamm_in_ms[i]); 
  }
  for( i = 4; i < numparams; i ++) {
    fprintf(fp,"%d \n",spectro->params[i]); 
  }
  fclose(fp);
  sprintf(mess,"wrote: %s\n",spectro->tmp_name);
  writetext(mess);
} /* end dowriteparams */



/********************************************************************
 *
 * cancelfilesb(Widget w,XtPointer client_data,XtPointer call_data)
 *
 * Cancel file browser.
 *
 ********************************************************************/

void  cancelfilesb(Widget w, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild(filesb_dialog);
  XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
  XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);

  writetext("Cancel file.\n");
}


/********************************************************************
 *
 * cancelinput(Widget w,XtPointer client_data,XtPointer call_data)
 *
 * Cancel input dialog box.
 *
 ********************************************************************/

void cancelinput(Widget w,XtPointer client_data,XtPointer call_data)
{
  XtUnmanageChild(input_dialog);
  XtRemoveAllCallbacks(input_dialog, XmNokCallback);
  XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
  
  writetext("Cancel input.\n");
} /* end cancelinput */



/********************************************************************
 *
 * cancelwarning(Widget w,XtPointer client_data,XtPointer call_data)
 *
 * Cancel warning message box.
 *
 ********************************************************************/

void cancelwarning(Widget w,XtPointer client_data,XtPointer call_data)
{
  XtUnmanageChild(warning);
  XtRemoveAllCallbacks(warning, XmNokCallback);
  XtRemoveAllCallbacks(warning, XmNcancelCallback);
  XtRemoveAllCallbacks(warning, XmNhelpCallback);
  XtUnmanageChild(XmMessageBoxGetChild(warning,XmDIALOG_HELP_BUTTON));

  XtUnmanageChild(warning2);
  XtRemoveAllCallbacks(warning2, XmNokCallback);
  XtRemoveAllCallbacks(warning2, XmNcancelCallback);
  XtRemoveAllCallbacks(warning2, XmNhelpCallback);
  XtUnmanageChild(XmMessageBoxGetChild(warning2,XmDIALOG_HELP_BUTTON));

  writetext("cancel \n");
} /* end cancelwarning */



/*************************************************************
 *
 * void showvalues(XSPECTRO *spectro)               
 *
 * Display current spectrum data in help type text window.
 *
 *************************************************************/              

void showvalues(XSPECTRO *spectro)
{
  
  int n3,n;
  char str[5000];
  char temp[200];
  Arg args[1];

  sprintf(str,"\t\t%s SPECTRUM VALUES at %.1f",
	  spectro->wavename, spectro->savetime);

  sprintf(temp," (%.1fms Hamming window):\n\n",
	  (float)spectro->sizwin/spectro->spers * 1000.0);
  strcat(str,temp);

  /*
   * List out formant info 
   */

  if (spectro->option == 's' || spectro->option == 'S' ||
     spectro->option == 'l' || spectro->option == 'j'||
     spectro->option == 'c' || spectro->option == 'T'){
    strcat(str,"\t\t\tFormants:\n\n\t\t\tFREQ\tAMP\n");              

    for (n = 0; n < spectro->nforfreq; n++) {
     
      if(spectro->forfreq[n]< 0)
	sprintf(temp, "\t\t\t*%d\t%d\n",-spectro->forfreq[n],
		(int)(spectro->foramp[n]/10) );
      else
	sprintf(temp, "\t\t\t%d\t%d\n",spectro->forfreq[n],
		(int)(spectro->foramp[n]/10) );       
      strcat(str,temp);
     }
  }

  /*
   * List out spectral values
   */

  strcat(str,"\n\n\t\t\tSpectrum:\n\n\tn\tdB\tn\tdB\tn\tdB\n");
    
  n3 = (spectro->lenfltr+2)/3;
  for (n=0; n <n3; n++) {
    sprintf(temp,"\t%3d\t%4.1f  ", n+1, (float) spectro->fltr[n]/10.0);
    strcat(str,temp);
    
    sprintf(temp,"\t%3d\t%4.1f  ", n+n3+1, (float) spectro->fltr[n+n3]/10.0);
    strcat(str,temp);
    
    if ((n+n3+n3) < spectro->lenfltr) {
      sprintf(temp,"\t%3d\t%4.1f\n", n+n3+n3+1, 
	      (float)spectro->fltr[n+n3+n3]/10.0);
      strcat(str,temp);
    }
    else strcat(str,"\n");
  }
  
  strcat(str,"\n");
  n = 0;
  XtSetArg(args[n],XmNvalue,str); n++;
  XtSetValues(spectro->fbw_text,args, n);
}



/**************************************************************/
/* setwvfact(Widget w,XtPointer client_data,XtPointer call_data)   */
/**************************************************************/

void setwvfact(Widget w,XtPointer client_data, XtPointer call_data){

XSPECTRO *spectro;
Arg args[3];
int window;
char str[500];
char *temp;
float newfloat;

   XtUnmanageChild(input_dialog);
   spectro = (XSPECTRO *) client_data;

   XtSetArg(args[0],XmNvalue,&temp);
   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   strcpy(str,temp);

/* for now only window 1 is changed */
/* need to pass window information on from HandleViewingMenu*/
/* if you want to change either window */

   window = 1;

   newfloat = (float)atof(str);
   if(0.0 < newfloat && newfloat <= 1.0){
    spectro->wvfactor[window] = newfloat;

     wave_pixmap(spectro->info[window],spectro,1,window,NULL);
     update(spectro->info[window],spectro,window);
        
   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);

   sprintf(str,
   "%s:y-axis scale factor = %.1f\n",spectro->wavename,spectro->wvfactor[1]);
   writetext(str);
   }/* value in range*/

  else
    {writetext("value out of range\n");}

}/* end setwvfact*/



/***************************************************************/
/*  segtowav(Widget w,XtPointer client_data, XtPointer call_data)  */
/***************************************************************/

void segtowav(Widget w,XtPointer client_data, XtPointer call_data){
/*writes a .wav file generated by an edit('n'), */
/*synthesize  or record ,  spectro->wwav = 1,2or3 respectively*/

XSPECTRO *spectro;
Arg args[3];
int n;
char str[500];
char tmp[500];
char *temp;
char *tmp2;
FILE *fp;
XmString mstr;
int nowarn = 0;
XmFileSelectionBoxCallbackStruct *c_data = (XmFileSelectionBoxCallbackStruct *) call_data;

   spectro = (XSPECTRO *) client_data;
if(spectro->wwav<3) 
	XtUnmanageChild(input_dialog);
else
   XtUnmanageChild(filesb_dialog);

/*   if(spectro!=NULL)
   {
    	sprintf(tmp,"spectro ok\n");
  	  	fprintf(stderr,"%s",tmp);
   }
      if(temp!=NULL)
{
		sprintf(tmp,"temp ok\n");
  	  	fprintf(stderr,"%s",tmp);
}
else
{
		sprintf(tmp,"temp null\n");
  	  	fprintf(stderr,"%s",tmp);
}
   if(c_data->dir!=NULL)
{
		sprintf(tmp,"dir ok\n");
  	  	fprintf(stderr,"%s",tmp);
  	  	XmStringGetLtoR(c_data->dir,XmSTRING_DEFAULT_CHARSET,&tmp2);
  	   fprintf(stderr,"%s",tmp2);
}
else
{
		sprintf(tmp,"dir null\n");
  	  	fprintf(stderr,"%s",tmp);
}*/
   save_dir = XmStringCopy(c_data->dir);
 
   XtSetArg(args[0],XmNvalue,&temp);

//   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   XtGetValues(XmFileSelectionBoxGetChild(filesb_dialog,XmDIALOG_TEXT),args,1);


   strcpy(str, temp);
   XtFree(temp);
   fprintf(stderr,"%s",str);
   if (!strchr(str, '.'))
       strcat(str,".wav");
   strcpy(spectro->segname, str); 
   if(spectro->wwav<3) 
   { XtRemoveAllCallbacks(input_dialog, XmNokCallback);
	XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
	}
	else
{  XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);
}  
 
   /* file does not exist */
   /*   i.e., no overwrite */
   if( !(fp = fopen(spectro->segname,"r")) || nowarn ){
     if((fp = fopen(spectro->segname,"w")) ){

       switch(spectro->wwav) {
       case 1:
       case 2: 

	  writeWavHeader(fp, spectro->spers, spectro->sampsplay, 
			 spectro->swap);


	  writeWav(fp, spectro->startplay, spectro->sampsplay, 
			 spectro->swap);

	  closeWav(fp); 
        break;

       case 3:
       
	  /*write synthesized .wav */
	  strcpy(firstname,spectro->segname);
	  firstname[strlen(firstname) - 4] = '\000';
	  makefilenames();
	  reset_data(); synthesize(); 

	  /* Continue-- save to doc file */
//	  ShowDocDialog(firstname);
	  strcpy(str, firstname);
	  writedoc(str);

	  break;

       }/* end switch */

      sprintf(str,"wrote %s\n",spectro->segname);
      writetext(str);
      if((fp = fopen(spectro->segname,"r")) ){
	fclose(fp);
	if(spectro->iwave == NULL){
	  /* syn/recmode no current waveform*/
	  strcpy(spectro->wavefile,spectro->segname);
	  swapstuff(spectro);
	}/* recmode no current waveform*/
	else{
	  recquery(spectro);
	}/* ask replace or new */
      }
      else{
	sprintf(str,"can't read %s\n",spectro->segname);
       /* just a message no callbacks */
	ShowOops(str);
       }

   }/* can open file */

     else{
     /* can't open file */
       sprintf(str,"can't open %s\n",spectro->segname);
       ShowOops(str);
     }
   }/* file does not exist */

   /* pop up overwrite query */
   else{
     /* first close file */
     fclose(fp);
     sprintf(str,"OVERWRITE %s ??",spectro->segname);
     mstr = XmStringCreateLocalized(str);
     n = 0;
     XtSetArg(args[n], XmNmessageString,mstr); n++;
     XtSetArg(args[n], XmNokLabelString,XmStringCreateLocalized("OK")  ); n++;
     XtSetArg(args[n], XmNcancelLabelString, XmStringCreateLocalized("Cancel")); n++;
     XtSetValues(warning,args,n);
     XmStringFree(mstr);
     XtAddCallback(warning,XmNokCallback,writewarn,(XtPointer)spectro);
     XtAddCallback(warning,XmNcancelCallback,
		   cancelwarning,(XtPointer)spectro);
     opendialog(warning);
   }

}/* end segtowav */


/***************************************************************/
/*  rectofile(Widget w,XtPointer client_data, XtPointer call_data)  */
/***************************************************************/

void rectofile(Widget w,XtPointer client_data, XtPointer call_data){
/*writes a .wav file generated by an edit('n'), */
/*synthesize  or record ,  spectro->wwav = 1,2or3 respectively*/

Arg args[3];
int n;
char str[500];
char str_kl[500];
char str2[520];
char tmp[500];
char *temp;
char *tmp2;
FILE *fp;
XmString mstr;
float rate;
int nowarn = 0;
sprintf(tmp,"In rectofile\n");
fprintf(stderr,"%s",tmp);
XmFileSelectionBoxCallbackStruct *c_data = (XmFileSelectionBoxCallbackStruct *) call_data;

	XtUnmanageChild(input_dialog);
 
   XtSetArg(args[0],XmNvalue,&temp);

XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   //XtGetValues(XmFileSelectionBoxGetChild(filesb_dialog,XmDIALOG_TEXT),args,1);

   strcpy(str, temp);
   strcpy(str_kl, temp);
   strcat(str_kl,"_kl.wav");
   XtFree(temp);
   fprintf(stderr,"%s",str);
   if (!strchr(str, '.'))
       strcat(str,".wav");
	XtRemoveAllCallbacks(input_dialog, XmNokCallback);
	XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);

 
   /* file does not exist */
   /*   i.e., no overwrite */
   if( !(fp = fopen(str,"r")) || nowarn ){
     if((fp = fopen(str,"w")) ){
     	rate=recParams[REC_RATE].value;
     	sprintf(tmp, "Recording for %.1f seconds at %f!\n", recParams[REC_DUR].value, rate);
  		writetext(tmp);
		recordToFile(&rate, recParams[REC_DUR].value, str);

      sprintf(str2,"wrote %s\n",str);
      writetext(str2);
      if((fp = fopen(str,"r")) ){
	fclose(fp);
      }
      else{
	sprintf(str2,"can't read %s\n",str);
       /* just a message no callbacks */
	ShowOops(str2);
       }

   }/* can open file */

     else{
     /* can't open file */
       sprintf(str2,"can't open %s\n",str);
       ShowOops(str2);
     }
        LoadNewWaveform((XSPECTRO *)client_data, str_kl);
   }/* file does not exist */

   /* pop up overwrite query */
   else{
     /* first close file */
     fclose(fp);
     sprintf(str2,"%s exists, giving up\n",str);
       ShowOops(str2);
   }

}/* end rectofile */

/****************************************************************/
/*  writewarn(Widget w,XtPointer client_data, XtPointer call_data)  */
/****************************************************************/

void writewarn(Widget w,XtPointer client_data, XtPointer call_data){
XSPECTRO *spectro;
char str[330],temp[300];
FILE *fp;

/* this is the callback for an overwrite warning */
/* used for either write seg to .wav, record or syn */
/* user wants to overwite an exicsting .wav file */

   XtUnmanageChild(warning);
   XtRemoveAllCallbacks(warning, XmNokCallback);
   XtRemoveAllCallbacks(warning, XmNcancelCallback);


   spectro = (XSPECTRO *) client_data;



  switch(spectro->wwav){
     case 1:
     /* write seg to .wav */
       putWavWaveform(spectro->segname, spectro->startplay, 
		      spectro->spers, spectro->sampsplay, spectro->swap);
       break;

   case 2:
      /* record on sgi, convert aif to .wav and read in */
       system("/usr/sbin/sfconvert /tmp/rec.aif /tmp/rec.raw -o byte little");
       /* debug need to change rawtowav directory */
       /* strip .wav from spectro->segname */
       strcpy(temp,spectro->segname);
       temp[strlen(temp) - 4] = '\000';
       sprintf(str,"rawtowav /tmp/rec.raw %s 16000",temp);
       system(str);

   break;

   case 3:
       /*write synthesized .wav */
       if ((fp = fopen(spectro->segname,"w")) ){
	 strcpy(firstname,spectro->segname);
	 firstname[strlen(firstname) - 4] = '\000';
	 makefilenames();
	 reset_data();
	 synthesize();
       }
       else{writetext("can't write file\n");}

       /* Continue-- save to doc file */
       //       ShowDocDialog(firstname);
       strcpy(temp, firstname);
       writedoc(temp);
   break;

 }/* end switch */


       sprintf(str,"wrote %s\n",spectro->segname);
       writetext(str);
       strcpy(str,spectro->segname);
       str[strlen(str) - 4] = '\000';
       strip_path(str,temp);

       /* remove old gram file */
       sprintf(str,"rm -f %s.gram",temp);
       system(str);

       if((fp = fopen(spectro->segname,"r")) ){
          fclose(fp);
             if(spectro->iwave == NULL){
                 /* recmode, empty display */
                   strcpy(spectro->wavefile,spectro->segname);
                   swapstuff(spectro);
		 }
              else{
                 /* if this .wav is displayed replace it*/
                 if(doreplace(spectro)){
		 }
		 else{
                 recquery(spectro);
	         }
                 
	        }/* replace all waveform displays of same name*/
	}/*  can open file for reading*/
       else{
       sprintf(str,"can't read %s\n",spectro->segname);
       writetext(str);
       }

}/* end writewarn */



/************************************************************/
/*           ShowDocDialog()                                */
/************************************************************/

static void writedoc0(Widget w,XtPointer client_data, XtPointer call_data)
{
  Arg args[1];
  char *temp, str[200];

   XtUnmanageChild(input_dialog);
 
   XtSetArg(args[0],XmNvalue,&temp);
   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   strcpy(str,temp);
   writedoc(str);
}

void ShowDocDialog(char *defName)
{
  Arg args[5];
  int n;
  XmString mstr;

  /* Continue-- ask for doc file and save */
  XtSetArg(args[0],XmNvalue,defName);
  XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
  mstr = XmStringCreateLocalized("filename: will append .doc");
  n=0;
  XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
  XtSetValues(input_dialog,args,n);
  XmStringFree(mstr);
  n=0;
  XtSetArg(args[n],XmNtitle,"write .doc"); n++;
  XtSetValues(XtParent(input_dialog),args,n);
  
  XtAddCallback(input_dialog,XmNokCallback,writedoc0,NULL);
  XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,NULL);
  opendialog(input_dialog);
  writetext("Waiting for input...\n");

}

/************************************************************/
/*           doreplace(XSPECTRO *spectro)                   */
/************************************************************/

int doreplace(XSPECTRO *spectro)
{
   int i, didreplace;
   char temp[200];
   /*in the case that a file has been overwritten replace
     the current display of that file with the new file*/

   strcpy(temp,spectro->segname);
   temp[strlen(temp) - 4] = '\000';

   didreplace = 0;
   for( i = 0; i < allspectros.count; i++){
      if(strcmp(allspectros.list[i]->wavename, temp) == 0) {
	 delete_wave(spectro);
	 strcpy(allspectros.list[i]->wavefile,spectro->segname);
	 swapstuff(allspectros.list[i]);
	 raisewav(allspectros.list[i]);
	 didreplace = 1;
      }
   } /* see if the file is already displayed */
   return(didreplace);
}



/****************************************************************
 *
 * void recstart(XSPECTRO *spectro, int on)            
 *
 * When starting up in record mode only, record menu entry is sensitive, 
 * so is read file and synthesize all other entires are not, 
 * if on = 0, a recorded .wav has been read in and menus should be
 * activated.
 *
 ****************************************************************/

void recstart(XSPECTRO *spectro, int on)
{
  int i, n, s, count;
  Arg args[1];

  if (on) {
    for(i = 0; i < NUM_WINDOWS; i++){ /* all windows */
      count = 0;
      for(s = 0; s < numPulldown; s++) { /* all menus */
	for(n = 0; n < num_entries[s]; n++) { /* all entries */
	  if(entries[count][0] != 'Z' &&
	     entries[count][0] != 'q' &&
	     entries[count][0] != 'r' &&
	     entries[count][0] != 'Y' &&
	     entries[count][0] != '?' &&
	     entries[count][0] != 'v' &&
	     entries[count][0] != 'R' ){
	    XtSetArg(args[0],XmNsensitive,False);
	    XtSetValues(spectro->buttons[i][count],args,1);
	  }
	  count++;     
	}
      }
    }
    
    /* iconify window 1 + 2 */
    
    XtSetArg(args[0],XmNiconic,True);
    XtSetValues(spectro->toplevel[1],args,1);
    spectro->normstate[1] = 0;
    XtSetArg(args[0],XmNiconic,True);
    XtSetValues(spectro->toplevel[SPECTRUM],args,1);
    spectro->normstate[SPECTRUM] = 0;
  }
  else {  /*  activate menu entries */

    
    XtSetArg(args[0],XmNiconic,False);
    XtSetValues(spectro->toplevel[1],args,1);
    spectro->normstate[1] = 1;
    XtSetArg(args[0],XmNiconic,False);
    XtSetValues(spectro->toplevel[SPECTRUM],args,1);
    spectro->normstate[SPECTRUM] = 1;

    for(i = 0; i < NUM_WINDOWS; i++) { /* all windows */
      count = 0;
      for(s = 0; s < numPulldown; s++) { /* all menus */
	for(n = 0; n < num_entries[s]; n++) { /* all entries */
	  switch (entries[count][0]) {
	    case 'u':
	    case ',':
	    case '.':
            case 'K':
              XtSetSensitive(spectro->buttons[i][count], spectro->spectrogram);
        	break;
            case '#':
              XtSetSensitive(spectro->buttons[i][count], (PS4_fp != NULL));
              break;
            case '*':
              XtSetSensitive(spectro->buttons[i][count], (PSfull_fp != NULL));
	      break;
	    default:
	      XtSetSensitive(spectro->buttons[i][count], True);
	      break;
	  }
	  count++;     
	}
      }
    }
  }
}



/****************************************************************/
/*            void recquery(XSPECTRO *spectro)                  */
/****************************************************************/

void recquery(XSPECTRO *spectro){
Arg args[4];
XmString mstr;
char str[300];

/* ask whether recorded .wav uses newwave or ƒ -
   replace or new
*/

   sprintf(str,"Use Current or New display for %s ??",spectro->segname);
   mstr = XmStringCreateLocalized(str);
   XtSetArg(args[0], XmNmessageString,mstr);
   XtSetValues(warning,args,1);
   XmStringFree(mstr);  


   XtSetArg(args[0], XmNhelpLabelString,XmStringCreateLocalized("New")  );
   XtSetArg(args[1], XmNokLabelString,XmStringCreateLocalized("Current")  );
   XtSetArg(args[2], XmNcancelLabelString,
	    XmStringCreateLocalized("Don't display\nwaveform"));
   XtSetValues(warning, args, 3);

   XtManageChild(XmMessageBoxGetChild(warning,XmDIALOG_HELP_BUTTON));
   XtManageChild(XmMessageBoxGetChild(warning,XmDIALOG_CANCEL_BUTTON));

 
   XtAddCallback(warning,XmNokCallback, recswap, (XtPointer)spectro);
   XtAddCallback(warning,XmNhelpCallback, recnew, (XtPointer)spectro);
   XtAddCallback(warning,XmNcancelCallback,
                           cancelrec,(XtPointer)spectro);
   opendialog(warning);
   /* 3 buttons don't need traversal */

 }/* end recquery*/


/****************************************************************/
/*  recswap(Widget w,XtPointer client_data, XtPointer call_data)    */
/****************************************************************/
void recswap(Widget w,XtPointer client_data, XtPointer call_data){
XSPECTRO *spectro;

/* after recorded .aif has been converted to .wav 
   replace current waveform with it
   when the play routines are done there will be a data pointer 
*/
   
   XtUnmanageChild(warning);

   spectro = (XSPECTRO *)client_data;

if(allspectros.count == 1){

   /*no choice to be made*/
   delete_wave(spectro);
   strcpy(spectro->wavefile,spectro->segname);
   swapstuff(spectro);
 }
else{
/* see which waveform the user wants replaced */ 
 dowavlist();
 XtAddCallback(wav_sb,XmNokCallback,doswap,(XtPointer)spectro);
 XtManageChild(wav_sb);
}


   XtRemoveAllCallbacks(warning, XmNokCallback);
   XtRemoveAllCallbacks(warning, XmNcancelCallback);
   XtRemoveAllCallbacks(warning, XmNhelpCallback);
   XtUnmanageChild(
     XmMessageBoxGetChild(warning,XmDIALOG_HELP_BUTTON) );

}/* end recswap*/


/**********************************************************************/
/*    void doswap(Widget w, XtPointer client_data, XtPointercall_data)     */
/**********************************************************************/

 void doswap(Widget w, XtPointer client_data, XtPointer call_data) {
 int whichparam;
 XSPECTRO *spectro, *oldspectro;


/* not used */
 oldspectro = (XSPECTRO *)client_data;

 /*client_data is NULL */
 /* raise selected waveform's windows */ 
 whichparam = SB_ListSelectedItemPosition(w) - 1;
 spectro = allspectros.list[whichparam];

   delete_wave(spectro);
   strcpy(spectro->wavefile,oldspectro->segname);
   swapstuff(spectro);
   raisewav(spectro);

XtRemoveAllCallbacks(wav_sb, XmNokCallback);

}/* end doswap */


/**************************************************************/
/*  recnew(Widget w,XtPointer client_data, XtPointer call_data)  */
/**************************************************************/

void recnew(Widget w,XtPointer client_data, XtPointer call_data){
XSPECTRO *spectro;

/* after recorded .aif has been converted to .wav 
   replace current waveform with it
   when the play routines are done there will be a data pointer 
*/
   /* this button is going away so make another one active */
   XmProcessTraversal(warning, XmTRAVERSE_HOME);

   XtUnmanageChild(warning);
   spectro = (XSPECTRO *) client_data;

   LoadNewWaveform(spectro,spectro->segname);

   XtRemoveAllCallbacks(warning, XmNokCallback);
   XtRemoveAllCallbacks(warning, XmNcancelCallback);
   XtRemoveAllCallbacks(warning, XmNhelpCallback);
   XtUnmanageChild(
     XmMessageBoxGetChild(warning,XmDIALOG_HELP_BUTTON) );
}/* end recnew*/


/********************************************************************/
/*    cancelrec(Widget w,XtPointer client_data,XtPointer call_data)    */
/********************************************************************/
void cancelrec(Widget w,XtPointer client_data,XtPointer call_data){
 
  Arg args[1];
/* cancel warning message box */
/*NOTE: free recorded data pointer here*/

   XtUnmanageChild(warning);
   XtRemoveAllCallbacks(warning, XmNokCallback);
   XtRemoveAllCallbacks(warning, XmNcancelCallback);
   XtRemoveAllCallbacks(warning, XmNhelpCallback);

   XtSetArg(args[0], XmNcancelLabelString, XmStringCreateLocalized("Cancel"));
   XtSetValues(warning, args, 1);

   writetext("cancel \n");
   XtUnmanageChild(
     XmMessageBoxGetChild(warning,XmDIALOG_HELP_BUTTON) );
}

/******************************************************************/
/* void opendialog(Widget dialog)*/
/******************************************************************/
 
void opendialog(Widget dialog)
{

  Display *display;
  Window root_return, child_return;
  Arg args[4];
  Dimension dialogWidth, dialogHeight, dialogBorderWidth;
  int displayWidth, displayHeight;
  int winX, winY, n;
  int dialogX, dialogY;
  unsigned int mask_return;
  char str[100];

  /* 
   * Move the window to the location of the cursor(pointer)
   * so the filebrowser has the input focus whether or not the 
   * window mananger has the pointer set to implicit or explicit 

   * First, find the size of the terminal's display
   */

  display = XtDisplay(XtParent(dialog));
  displayWidth = DisplayWidth(display, DefaultScreen(display));
  displayHeight = DisplayHeight(display, DefaultScreen(display));

  /*
   * Next, find out where the cursor is
   */
  
  XQueryPointer(display,
		RootWindowOfScreen(XtScreen(XtParent(dialog))),
		&root_return, &child_return,
		&dialogX, &dialogY,
		&winX, &winY, &mask_return);

  XtManageChild(dialog);

  /* 
   * Find out size of dialog box
   */

  n = 0;
  XtSetArg(args[n], XmNwidth, &dialogWidth); n++;
  XtSetArg(args[n], XmNheight, &dialogHeight); n++;
  XtSetArg(args[n], XmNborderWidth, &dialogBorderWidth); n++;
  XtGetValues(dialog, args, n);


  /*
   * Place dialog box so that it does not go off screen...for both x & y
   */

  if ((dialogX +(int) dialogWidth) > displayWidth) 
    winX = displayWidth - (int) dialogWidth - dialogBorderWidth-10;
  else if ((dialogX - (int) dialogWidth/2) < 0) 
    winX = 0;
  else winX = dialogX - (int) dialogWidth/2;
  
  if ((dialogY +(int) dialogHeight) > displayHeight) 
    winY = displayHeight - (int) dialogHeight - (int) dialogBorderWidth-30;
  else if ((dialogY - (int) dialogHeight/2) < 0) 
    winY = 0;
  else winY = dialogY - (int) dialogHeight/2;

  XMoveWindow(XtDisplay(XtParent(dialog)), 
	      XtWindow(XtParent(dialog)), winX, winY);

// try to give it focus
  XmProcessTraversal(dialog, XmTRAVERSE_CURRENT);

} /* end opendialog */

/******************************************************************/
/* void open_tl_dialog(Widget dialog) */
/******************************************************************/
void open_tl_dialog(Widget dialog){

Display *display;
Window root_return, child_return;
Arg args[4];
Dimension dialogWidth, dialogHeight, dialogBorderWidth;
int displayWidth, displayHeight;
int winX, winY, n;
int dialogX, dialogY;
unsigned int mask_return;
char str[100];

/* 
 * Find the size of the terminal's display
 */

display = XtDisplay(XtParent(dialog));
displayWidth = DisplayWidth(display, DefaultScreen(display));
displayHeight = DisplayHeight(display, DefaultScreen(display));


/* move the window to the location of the cursor(pointer)
so the filebrowser has the input focus whether or not the 
window mananger has the pointer set to implicit or explicit*/

XQueryPointer(XtDisplay(XtParent(dialog)),
	      RootWindowOfScreen(XtScreen(XtParent(dialog))),
	      &root_return ,&child_return,
	      &dialogX,&dialogY,
	      &winX,&winY,&mask_return);
XtManageChild(dialog); 

/* 
 * Find out size of dialog box
 */

n = 0;
XtSetArg(args[n], XmNwidth, &dialogWidth); n++;
XtSetArg(args[n], XmNheight, &dialogHeight); n++;
XtSetArg(args[n], XmNborderWidth, &dialogBorderWidth); n++;
XtGetValues(dialog, args, n);

/* 
 * If it is a dialog box which has the user enter text, highlight 
 * the selection.
 */

if (XtIsManaged(input_dialog)) {
   strcpy(str, XmTextGetString(XmSelectionBoxGetChild(dialog,XmDIALOG_TEXT)));
   if (strlen(str) != 0)
      XmTextSetSelection(XmSelectionBoxGetChild(dialog,XmDIALOG_TEXT), 0, 
			 strlen(str), 0);
}

/*
 * Place dialog box so that it does not go off screen...for both x & y
 */

if ((dialogX +(int) dialogWidth) > displayWidth) 
   winX = displayWidth - (int) dialogWidth - dialogBorderWidth-10;
else if ((dialogX - (int) dialogWidth/2) < 0) 
   winX = 0;
else winX = dialogX - (int) dialogWidth/2;

if ((dialogY +(int) dialogHeight) > displayHeight) 
   winY = displayHeight - (int) dialogHeight - (int) dialogBorderWidth-30;
else if ((dialogY - (int) dialogHeight/2) < 0) 
   winY = 0;
else winY = dialogY - (int) dialogHeight/2;

XMoveWindow(XtDisplay(dialog), XtWindow(dialog),winX, winY);

}/* end open_tl_dialog*/


/********************************************************************
 *
 * void exitwarning()
 *
 * User wants to quit, so pop up dialog box and ask if they really want
 * to quit.
 *
 ********************************************************************/

void exitwarning()
{
  char mess[500];
  int n;
  Arg args[3];
  XmString mstr;

  sprintf(mess,"EXIT PROGRAM ??");
  mstr = XmStringCreateLocalized(mess);
     n = 0;
     XtSetArg(args[n], XmNmessageString, mstr); n++;
     XtSetArg(args[n], XmNokLabelString, XmStringCreateLocalized("OK")); n++;
     XtSetArg(args[n], XmNcancelLabelString, XmStringCreateLocalized("Cancel")); n++;
     XtSetValues(warning,args,n);
     XmStringFree(mstr);
  XtRemoveAllCallbacks(input_dialog, XmNokCallback);
  XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);  
  XtAddCallback(warning, XmNokCallback,doexit, NULL);
  XtAddCallback(warning, XmNcancelCallback, cancelwarning, NULL);
  opendialog(warning);
}

/********************************************************************
 *
 * void doexit()
 *
 * Close up files if they are open, and then exit.
 *
 ********************************************************************/

void doexit()
{
  XtUnmanageChild(warning);
  XtRemoveAllCallbacks(warning, XmNokCallback);
  XtRemoveAllCallbacks(warning, XmNcancelCallback);

  KillPlay();  // close audio device

  if (PS4_fp) {
    quadcount = 4;
    fourspectra(allspectros.list[0], PS4_fp);
    fclose(PS4_fp);
    PS4_fp = NULL;
  }

  if (PSfull_fp) {
    fclose(PSfull_fp);
    PS4_fp = NULL;
    PSfull_fp = NULL;
  }

  if (D_fp) {
    fclose(D_fp);
    D_fp = NULL;
  }
  exit(0);
}

/********************************************************************
 *
 * void ShowOops(char *message)
 *
 * Genereic widget that says oops, and a message.
 *
 ********************************************************************/

void ShowOops(char *message)
{
  Arg args[5];
  int n;
  XmString mstr;

  writetext(message);
  mstr = XmStringCreateLocalized(message);
  n = 0;
  XtSetArg(args[n], XmNmessageString,mstr); n++;

  XtSetValues(oops,args,n);
  opendialog(oops);  /* just a message no callbacks */
  XmStringFree(mstr);
}

/********************************************************************
 *
 * void closefullps()
 *
 * Close full page postscript file.
 *
 ********************************************************************/

void closefullps(Widget w, XtPointer client_data, XtPointer call_data)
{
  XSPECTRO *spectro;

  XtUnmanageChild(warning);
  XtRemoveAllCallbacks(warning, XmNokCallback);
  XtRemoveAllCallbacks(warning, XmNcancelCallback);

  spectro = (XSPECTRO *) client_data;

  fclose(PSfull_fp);
  PSfull_fp = NULL;
  deActivateMenuItem(spectro, '*');
  writetext("Closed full page postscript file.\n");
}


/********************************************************************
 *
 * void close4ps()
 *
 * Close 4/page postscript file.
 *
 ********************************************************************/

void close4ps(Widget w,XtPointer client_data,XtPointer call_data)
{
  XSPECTRO *spectro;

  XtUnmanageChild(warning);
  XtRemoveAllCallbacks(warning, XmNokCallback);
  XtRemoveAllCallbacks(warning, XmNcancelCallback);
  spectro = (XSPECTRO *) client_data;

  if (quadcount != 0) fprintf(PS4_fp,"showpage grestore grestore\n");
  fclose(PS4_fp);
  PS4_fp = NULL;
  deActivateMenuItem(spectro, '#');
  writetext("Closed 4/page postscript file.\n");
}

/********************************************************************
 *
 * void closejnl()
 *
 * Close journal file.
 *
 ********************************************************************/

void closejnl()
{
  XtUnmanageChild(warning);
  XtRemoveAllCallbacks(warning, XmNokCallback);
  XtRemoveAllCallbacks(warning, XmNcancelCallback);

  fclose(D_fp);
  D_fp = NULL;
  writetext("closed journal file.\n");
}

/******************************************************************
 *
 * void openjnl(Widget w, XtPointer client_data, XtPointer call_data)
 *
 *
 * Callback for opening journal dialog box.
 *
 ******************************************************************/

void openjnl(Widget w, XtPointer client_data, XtPointer call_data)
{
  XSPECTRO *spectro;
  Arg args[3];
  int n;
  char fname[300];
  char *temp, mess[350];
  FILE *fp;
  XmString mstr;

  XtUnmanageChild(input_dialog);
  spectro = (XSPECTRO *) client_data;

  XtSetArg(args[0],XmNvalue,&temp);
  XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
  strcpy(fname, temp);

  XtRemoveAllCallbacks(input_dialog, XmNokCallback);
  XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);

  if (!(fp = fopen(fname,"r"))) { /* check if file exists */
    if (!(fp = fopen(fname,"w"))) { /* check if can write to file */
      sprintf(mess,"Can't open journal file: %s\n",fname);
      ShowOops(mess);
      return; 
    }
    else{ /* if changes are made here change dowritejnl as well*/
      D_fp = fp;
      fprintf(D_fp,"%16s%8s%8s%8s%8s%8s%12s%10s\n", "Name","Option","Freq", 
	      "Amp","Hist","Hist","Time","Val");
      fprintf(D_fp,"%16s%8s%8s%8s%8s%8s%12s%10s\n", " "," ","Hz", 
	      "dB","Hz","dB","ms","");
      sprintf(mess,"Opened journal file: %s.\n",fname);
      writetext(mess);
    }
  }
  else { 
    fclose(fp); /* was opened for read, so close */
    if (!(fp = fopen(fname,"w"))) { /* check if we can write */
      sprintf(mess,"Can't open journal file: %s\n",fname);
      ShowOops(mess);
      return;
    }/* can't open file */
    else{ /* was opened for write, do we want to overwrite? */
      fclose(fp);   
      strcpy(spectro->tmp_name,fname);
      sprintf(mess,"OVERWRITE %s ??",fname);
      mstr = XmStringCreateLocalized(mess);
      n = 0;
      XtSetArg(args[n], XmNmessageString,mstr); n++;
      XtSetValues(warning,args,n);
      XmStringFree(mstr);
      mstr = XmStringCreateLocalized("OK");
      XtSetArg(args[0], XmNokLabelString,mstr);
      XtSetValues(warning,args,1);
      XmStringFree(mstr);
      XtRemoveAllCallbacks(input_dialog, XmNokCallback);
      XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);  
      XtAddCallback(warning,XmNokCallback,dooverwritejnl,(XtPointer)spectro);
      XtAddCallback(warning,XmNcancelCallback,cancelwarning,NULL);
      opendialog(warning);
    }
  }
}

/******************************************************************
 *
 * void dooverwritejnl(Widget w, XtPointer client_data, XtPointer call_data)
 *
 *
 * Callback for overwrite of journal dialog box.
 *
 ******************************************************************/

void dooverwritejnl(Widget w,XtPointer client_data, XtPointer call_data)
{
  char mess[550];
  XSPECTRO *spectro;

  XtUnmanageChild(warning);
  spectro = (XSPECTRO *) client_data;

  XtRemoveAllCallbacks(warning, XmNokCallback);
  XtRemoveAllCallbacks(warning, XmNcancelCallback);

  D_fp = fopen(spectro->tmp_name,"w");
  fprintf(D_fp,"%16s%8s%8s%8s%8s%8s%12s%10s\n", "Name","Option","Freq", 
	  "Amp","Hist","Hist","Time","Val");
  fprintf(D_fp,"%16s%8s%8s%8s%8s%8s%12s%10s\n", " "," ","Hz", 
	  "dB","Hz","dB","ms","");
  sprintf(mess,"Opened journal file: %s.\n",spectro->tmp_name);
  writetext(mess);
}

/*======================================================================
  ======================================================================
  ======================================================================
                                SYNTHESIS
  ======================================================================
  ======================================================================
  ======================================================================*/

/**********************************************************************/
/*voidHandleSynthesizeMenu(Widget w,XtPointer client_data,XtPointer call_data)*/
/**********************************************************************/
void HandleSynthesizeMenu(Widget w,XtPointer client_data, XtPointer call_data){

/* use num_entries to find start of button array */
/* use w to find which window the call was made from*/

  int window, entry, offset, n;
  Widget *buttons;
  XSPECTRO *spectro;
  Arg args[5];
  char str[200];
  XmString mstr;
  XmRowColumnCallbackStruct *c_data;

  c_data = (XmRowColumnCallbackStruct *) call_data;
  spectro = (XSPECTRO *) client_data;


/*
 * Establish which pulldown menu
 */

  for (window = 0; window < numPulldown; window++)
    if (spectro->menu_bar[window] == XtParent(XtParent(w))) break;

 buttons = spectro->buttons[window];

/*
 * Find out offset based on num_entries globally defined array./
 * Offset in button array is total of previous num_entries in entry array.
 */

 offset = num_entries[0] + num_entries[1] + num_entries[2] + num_entries[3];
 for (entry = 0; entry < num_entries[4]; entry++)
   if (c_data->widget == buttons[offset + entry]) break;

   /* SYNTHESIZE MENU

0   "Y Synthesize",

   */

 switch(entry) {
 case 0:
   /* Y Synthesize */
   /* popup attention dialog and see whether to
   use defaults or read .doc file */
   if (syn_tl == NULL)
     CreateSynDialog(application);
   reset_data();

   if (!XtIsManaged(syn_tl) ) {
     sprintf(str,"read .doc file or use defaults");
     mstr = XmStringCreateLocalized(str);
     n = 0;
     XtSetArg(args[n], XmNmessageString,mstr); n++;
     XtSetValues(warning,args,n);
     XmStringFree(mstr);
     XtSetArg(args[0], XmNokLabelString,XmStringCreateLocalized("defaults")  );
     XtSetArg(args[1], XmNhelpLabelString,XmStringCreateLocalized(".doc file")  );
     XtSetValues(warning,args,2);
     XtAddCallback(warning,XmNokCallback,syndef, NULL);
     XtAddCallback(warning,XmNcancelCallback,
		   cancelwarning,(XtPointer)spectro);
     XtManageChild(XmMessageBoxGetChild(warning,XmDIALOG_HELP_BUTTON) );
     XtAddCallback(warning,XmNhelpCallback,syndoc,NULL);
     opendialog(warning); /* 3 buttons don't need traversal */
   }
   /* if syn command window is open don't do anything */ 
   break;
 }/* end switch */
}/* end HandleSynthesizeMenu */



/**********************************************************************/
/*void	HandleSynMenu(Widget w,XtPointer client_data,XtPointer call_data)*/
/**********************************************************************/

void HandleSynMenu(Widget w,XtPointer client_data,XtPointer call_data){

/*syn_buttons is a global since all spectros share the Syn Widget*/

  XSPECTRO *spectro;
  int entry;
  XmRowColumnCallbackStruct *c_data;
  
  c_data = (XmRowColumnCallbackStruct *)call_data;
  
  
  /* an arbitrary spectro structure is associated with the
     synthesizer for compatibility with other functions
     like segtowav
     */
  spectro = allspectros.list[0];
  
  for(entry = 0; entry < XtNumber(synlist); entry++)
    if(c_data->widget == syn_buttons[entry])
      break;

  if(entry == XtNumber(synlist) - 1){
    donesyn(w, client_data, call_data);
  }
  else{
    inputsyn(spectro,entry ); 
  }
  
}/* end HandleSynMenu */



/**********************************************************************/
/*    void dosyn(Widget w, XtPointer client_data, XtPointercall_data)	*/
/**********************************************************************/

 void dosyn(Widget w, XtPointer client_data, XtPointer call_data) {
 int whichparam;
 XSPECTRO *spectro;

 /* basically the synthesizer widget is not associated with a 
    particular waveform, but since it use some functions like
    segtowav that require an XSPECTRO pointer spectro is set
    to the first spectro onthe list
 */

 spectro = allspectros.list[0];

 whichparam = SB_ListSelectedItemPosition(w);


/* selection box list starts at 1*/
 /* deal with e DONE command */
  if(whichparam	 == XtNumber(synlist)){
     donesyn(w, client_data, call_data);
 }
  else{
    inputsyn(spectro,whichparam - 1);
  }


 }/* end dosyn*/


void ShowInputDialog(Arg *args)
{


}

/*********************************************************************/
/*	       inputsyn(XSPECTRO *spectro,int index);		     */
/*********************************************************************/

void inputsyn(XSPECTRO *spectro,int index){
Arg args[5];
int n;
XmString mstr;
 char nullName[] = {""};

  switch(index){
  case 0:
/*    p	      PRINT set of synthesis parameter default values */
   showdoc();
  break;
 case 1:
/*    c	      CHANGE default value for a synthesis parameter */

   XtSetArg(args[0],XmNvalue,"");
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized ("Par:");
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   n=0;
   XtSetArg(args[n],XmNtitle,"syn: constant"); n++;
   XtSetValues(XtParent(input_dialog),args,n);
   synstr[0] = 'c';
   XtAddCallback(input_dialog,XmNokCallback,synpar,NULL);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,NULL);
   opendialog(input_dialog);

   writetext("Waiting for input...\n");	    

  break;

 case 2:
/*     d       CHANGE synthesis parameter time function */


   XtSetArg(args[0],XmNvalue,"");
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized("Par:");
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetArg(args[n],XmNcancelLabelString, XmStringCreateLocalized ("Done")); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);

   n=0;
   XtSetValues(XtParent(input_dialog),args,n);
   XtSetArg(args[n],XmNtitle,"syn: varied"); n++;
   XtSetValues(XtParent(input_dialog),args,n);
   synstr[0] = 'd';
   XtAddCallback(input_dialog,XmNokCallback,synpar,NULL);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,NULL);

   opendialog(input_dialog);

   writetext("Waiting for input...\n"); 

 break;
 case 3:
 /* G GRAPH on Laser printer all varied pars as a function of time*/
   synstr[0] = 'G';
   XtAddCallback(filesb_dialog, XmNokCallback, synps, NULL);
   XtAddCallback(filesb_dialog, XmNcancelCallback, cancelfilesb, NULL);
   mstr = XmStringCreateLocalized("*.ps");
   XmFileSelectionDoSearch(filesb_dialog, mstr);
   XmStringFree(mstr);

   n = 0;
   XtSetArg(args[n],XmNtitle,"syn"); n++;
   XtSetValues(XtParent(filesb_dialog),args,n);
   opendialog(filesb_dialog);
   writetext("Waiting for file name:\n");

 break;
 case 4:
/* s  SYNTHESIZE waveform file, save everything*/
#ifdef STUB
   XtSetArg(args[0],XmNvalue,"");
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized("filename: will append .wav");
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   n=0;
   XtSetArg(args[n],XmNtitle,"synthesize"); n++;
   XtSetValues(XtParent(input_dialog),args,n);
   XtAddCallback(input_dialog,XmNokCallback,segtowav,(XtPointer)spectro);
   XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,(XtPointer)spectro);
   spectro->wwav = 3; /*shares .wav overwrite check function */
   opendialog(input_dialog);
#endif
   mstr = XmStringCreateLocalized("*.wav");
   XmFileSelectionDoSearch(filesb_dialog,mstr);
   XmStringFree(mstr);
    n=0;
    XtSetArg(args[n], XmNtitle, "Save synthesized .wav file as"); n++;
    XtSetValues(XtParent(filesb_dialog), args, n);
    XtAddCallback(filesb_dialog,XmNokCallback,segtowav,(XtPointer)spectro);
    XtAddCallback(filesb_dialog,XmNcancelCallback,cancelfilesb,(XtPointer)spectro);
    spectro->wwav = 3; /*shares .wav overwrite check function */
    opendialog(filesb_dialog);
   
   writetext("Waiting for input...\n");

 break;

 case 5:
/*	   S	   SAVE current parameters as .doc file (no synthesis)*/
   ShowDocDialog(nullName);
   break;
 case 6:
  /* postscript file of syngraph contents */

   synstr[0] = 'g';
   XtAddCallback(filesb_dialog, XmNokCallback, synps, NULL);
   XtAddCallback(filesb_dialog, XmNcancelCallback, cancelfilesb, NULL);
   mstr = XmStringCreateLocalized("*.ps");
   XmFileSelectionDoSearch(filesb_dialog,mstr);
   XmStringFree(mstr);
   n = 0;
   XtSetArg(args[n],XmNtitle,"syn"); n++;
   XtSetValues(XtParent(filesb_dialog),args,n);
   opendialog(filesb_dialog);
   writetext("Waiting for file name:\n");
   break;

   case 7:
/* Show all varied parameters */
   synstr[0] = 'v';
   showvaried();
   break;

   
   }/* end switch*/

}/* end inputsyn*/



/***************************************************************/
/*  synpar(Widget w,XtPointer client_data, XtPointer call_data)	 */
/***************************************************************/

void synpar(Widget w,XtPointer client_data, XtPointer call_data){
/* see if input was a valid constant and add callback
   to get value
*/
Arg args[5];
int i,n;
char str[300];
char *temp;
XmString mstr;
char symbls[4];
Widget synmain;
int begin;
/*syn globals*/
extern int defval[],minval[],maxval[],np,nvar;
extern char cv[];
//extern float AFval[];

 
   XtSetArg(args[0],XmNvalue,&temp);
   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   strcpy(str,temp);
   /*check first 3 char against symb*/

     symbls[0] = '\0';
     symbls[1] = '\0';
     symbls[2] = '\0';
      for(i = 0; i < 3; i++){
       if(str[i] != '\n')
	    symbls[i] = toupper(str[i]);
      }
      symbls[i] = 0;
      strcpy(lastPar, symbls);
      if ((np = findnp(symbls)) == -1) {	       
		sprintf(str, "%s is not a legal parameter\n",
		symbls);
		writetext(str);
		XtRemoveAllCallbacks(input_dialog, XmNokCallback);
		cancelinput(w,NULL,NULL);
		return;
	    }

   XtRemoveAllCallbacks(input_dialog, XmNokCallback);


  /* 'c' command */
  if( synstr[0] == 'c'){

      if (cv[np] == VARRIED ) {
       writetext("parameter already varied\n");
       cancelinput(w,NULL,NULL);
       return;
     }

//      if (lastPar[0] == 'A' && lastPar[2] == 'F') {
//	  sprintf(str, "%g", AFval[lastPar[1]-'0'+2]);
//      } else {
	  sprintf(str, "%d",defval[np]); 
//      }
   XtSetArg(args[0],XmNvalue,str);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);

   HighlightInput (str, XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT));
   sprintf(str, "%s: min:%d max:%d",symbls,minval[np],maxval[np]); 
   mstr = XmStringCreateLocalized(str);
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   XtAddCallback(input_dialog,XmNokCallback,setcon,NULL);
    }/*'c'*/

  /* 'd' command */
 else if( synstr[0] == 'd')
   {
     
     if (cv[np] == FIXED_C) {
       writetext("not a variable param\n");
       cancelinput(w,NULL,NULL);
       return;
     }
     else {
       /* maybe this should be after a range check */
       if (cv[np] != VARRIED) {
	 nvar++;
	 cv[np] = VARRIED;
       }
     }/* not fixed */
     
     /* create graph window */
     if( syngraph == NULL )
       {
	 /* Use delete response = DO_NOTHING since the window goes
	    away permanently if closed once */
	 n = 0;
	 XtSetArg(args[n],XmNdeleteResponse, XmDO_NOTHING);n++;

	 synmain = XtCreatePopupShell("syngraph",topLevelShellWidgetClass,
				   cmdw,args,n);
      
	 
	 synxr = 400; synyb = 400;
	 n = 0;
	 XtSetArg(args[n], XmNwidth, synxr);n++;
	 XtSetArg(args[n], XmNheight,synyb);n++;
	 syngraph = XmCreateDrawingArea(synmain,"graph", args, n);
	 XtAddCallback(
		       syngraph,	         /* widget */
		       XmNexposeCallback,  /* callback name */
		       expsyn,	         /* callback procedure */
		       NULL		 /* client data */
		       );
	 XtAddCallback(
		       syngraph,	     /* widget */
		       XmNresizeCallback, /* callback name */
		       resizesyn,	     /* callback procedure */
		       NULL		     /* client data */
		       );
      
	 XtManageChild(syngraph);
	 syngc = XtScreen(syngraph)->default_gc;
       }
     
     if(!XtIsManaged(XtParent(syngraph)))
       {
	 XtManageChild(XtParent(syngraph));
	 syn_pixmap();
	 drawsyn(NULL);
	 
	 /* make sure input_dialog still has focus */
	 XtUnmanageChild(input_dialog);
	 opendialog(input_dialog);
	 XSync(XtDisplay(input_dialog),False);
       }
     
     else{
       syn_pixmap();
       drawsyn(NULL);
       XCopyArea(XtDisplay(syngraph),synmap,
		 XtWindow(syngraph),syngc,
		 0,0,synxr,synyb,0,0);
     }
     
     XtSetArg(args[0],XmNvalue,"");
     XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
     mstr = XmStringCreateLocalized("Time:");
     n=0;
     XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
     XtSetValues(input_dialog,args,n);
     XmStringFree(mstr);
     begin = 1;
     XtAddCallback(input_dialog,XmNokCallback,vartime,(XtPointer)begin);
   }/*'d'*/

}/* synpar */



/************************************************************************/
/*	   void CreateSynDialog(Widget parent)			      */
/************************************************************************/

void CreateSynDialog(Widget parent)  {

int n;
Arg args[5];
Widget mainw, menu_bar;
int i,listcount;
XmString items_label,*list,done_label;

/* popup list, used for interacting with synthesizer */
   n = 0;	
   XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
   XtSetArg(args[n], XmNsaveUnder, True); n++;
   syn_tl = XtCreatePopupShell("syn",topLevelShellWidgetClass,
				    application,args,n);


    n = 0;
    mainw = XmCreateMainWindow(syn_tl, "mainw", args, n);
    XtManageChild(mainw);

    /* XmStringCreateLocalized includes linefeed*/
     n = 0;
    items_label =
    XmStringCreateLocalized("Klatt Synthesizer Commands:");
    XtSetArg(args[n], XmNlistLabelString, items_label );n++; 
    done_label = XmStringCreateLocalized("done");
    XtSetArg(args[n], XmNcancelLabelString, done_label);n++;
    /* otherwise crashes when nothing is selected and change is clicked*/
    XtSetArg(args[n], XmNmustMatch,True);n++;


   /* global widget used by all spectro and all windows */
    syn_sb = XmCreateSelectionBox(mainw,"synw",args,n);

    XmStringFree(items_label);
    XmStringFree(done_label);
    XtAddCallback(syn_sb,XmNcancelCallback,donesyn,NULL);
    XtAddCallback(syn_sb, XmNokCallback,dosyn,NULL);
  

    XtManageChild (syn_sb);

  XtUnmanageChild(XmSelectionBoxGetChild(syn_sb,XmDIALOG_HELP_BUTTON) );
  XtUnmanageChild(XmSelectionBoxGetChild(syn_sb,XmDIALOG_SELECTION_LABEL) );
  XtUnmanageChild(XmSelectionBoxGetChild(syn_sb,XmDIALOG_TEXT) );

  /* set up menu_bar so the user can just hit a key to change a param*/
  /*param_buttons is global pointer to all entries on param menu*/
  listcount = XtNumber(synlist);
  syn_buttons  = (Widget *) malloc( sizeof(Widget) * listcount );

  menu_bar = XmCreateSimpleMenuBar(mainw,"sbar",args,0);
  XtManageChild(menu_bar);

  syn_menu = CreateOneMenu(menu_bar,"Commands",
		       synlist,listcount,
		       syn_buttons, False, 1 );

   XtAddCallback(syn_menu,XmNentryCallback,HandleSynMenu,NULL);

   XmMainWindowSetAreas( mainw, menu_bar, NULL, NULL, NULL, syn_sb);


   /* load Commands list into syn selection box */

    listcount = XtNumber(synlist);
    list = (XmString *) malloc( sizeof(XmString) * listcount);
    for(i = 0; i < listcount; i++){
	list[i] = XmStringCreateLtoR(synlist[i],XmSTRING_DEFAULT_CHARSET);
      }

    n = 0;
    XtSetArg(args[n], XmNlistItems, list );n++;
    XtSetArg(args[n], XmNlistItemCount,listcount);n++;
    XtSetValues(syn_sb,args,n);
    for(i = 0; i < listcount; i++)
	  XmStringFree(list[i]);
    free(list);

}/*CreateSynDialog*/


/***********************************************************************/
/*    donesyn(Widget w,XtPointer client_data,XtPointer call_data)      */
/***********************************************************************/
void donesyn(Widget w,XtPointer client_data,XtPointer call_data) {

/* unmanage list of synthesizer commands*/

if(syngraph != NULL){
 if(XtIsManaged(XtParent(syngraph)))
	XtUnmanageChild(XtParent(syngraph));
}
if(syntext != NULL){
 if(XtIsManaged(XtParent(syntext)))
	 XtPopdown(XtParent(XtParent(syntext)));
}

 XtUnmanageChild(syn_tl);


}/* end donesyn*/


/******************************************************************/
/*void  synps(Widget w, XtPointer client_data, XtPointer call_data) */
/******************************************************************/
void synps(Widget w, XtPointer client_data, XtPointer call_data)
{
  /* get a name for a post script file */
  char *temp, mess[550], ps_name[500];  
  XmString mstr;
  FILE *fp;
  Arg args[3];
  int n;
  
  XmFileSelectionBoxCallbackStruct *c_data = (XmFileSelectionBoxCallbackStruct *)call_data;
  
  XtUnmanageChild(filesb_dialog);
  
  XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
  XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);
  
  XtSetArg(args[0], XmNvalue, &temp);
  XtGetValues(XmFileSelectionBoxGetChild(filesb_dialog, XmDIALOG_TEXT),
	      args, 1);
  
  strcpy(ps_name,temp); 
  XtFree(temp);
  
  if(!(fp = fopen(ps_name,"r"))   ){
    /* can write with impunity */
    
    
    if(!(fp = fopen(ps_name,"w"))   ){
      sprintf(mess,"can't open %s\n",ps_name);
      ShowOops(mess);
      return; 
    }/* can't write to this file don't do anything */


    /* close fp, it get's opened again later*/
    fclose(fp);
    strcpy(syn_tmp_name,ps_name);
    writesynps();
  }/* don't have to worry about overwrite */
  
  else{
    /* was opened for read */
    fclose(fp);
    if(!(fp = fopen(ps_name,"w"))   ){
      sprintf(mess,"can't open:%s\n",ps_name);
      ShowOops(mess);
      return; 
    }/* can't write to this file don't do anything */
    
    
    /* was opened for write */
    fclose(fp);   
    strcpy(syn_tmp_name,ps_name);
    sprintf(mess,"OVERWRITE %s ??",ps_name);
    mstr = XmStringCreateLocalized(mess);
    n = 0;
    XtSetArg(args[n], XmNmessageString,mstr); n++;
    XtSetValues(warning,args,n);
    XmStringFree(mstr);
    mstr = XmStringCreateLocalized("OK");
    XtSetArg(args[0], XmNokLabelString,mstr);
    XtSetValues(warning,args,1);
    XmStringFree(mstr);
    XtRemoveAllCallbacks(input_dialog, XmNokCallback);
    XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);  
    XtAddCallback(warning,XmNokCallback,dowritesynps,NULL);
    XtAddCallback(warning,XmNcancelCallback,cancelwarning,NULL);
    opendialog(warning);
    
  }/* file already exists so popup overwrite query */
  
}/*end synps*/



/************/
void writesynps(){
  char mess[500];
  XmString mstr;
  int n,i;
  Arg args[5];
  
  /* called from synps and overwrite warning */
  
  if(synstr[0] =='G'){
    
#ifdef VMS
    
    for(i = strlen(syn_tmp_name) - 1; i >= 0; i --)
         if(syn_tmp_name[i] == ';') break;

         syn_tmp_name[i] = '\000';
#endif
    /* strip off file spec */
    for(i = strlen(syn_tmp_name) - 1; i >= 0; i --)
         if(syn_tmp_name[i] == '.') break;
         syn_tmp_name[i] = '\000';

   strcpy(firstname,syn_tmp_name);
   makefilenames();
   graphallpars();
   sprintf(mess,"Wrote %s.\n",syn_tmp_name);
   writetext(mess); 
  }/*G*/
  else if(synstr[0] == 'g'){
  
    if(syngraph == NULL){
      sprintf(mess,"Syngraph window not active!");
      /* just a message no callbacks */

      ShowOops(mess);
      writetext(mess);
      return;
    }

    printsyn(syn_tmp_name);
    sprintf(mess,"wrote %s\n",syn_tmp_name);
    writetext(mess); 
  }/*g*/

 }/* end writesynps*/

/************************/


void dowritesynps(Widget w,XtPointer client_data, XtPointer call_data){
  /*callback from psfile overwrite warning */

  XtUnmanageChild(warning);

   XtRemoveAllCallbacks(warning, XmNokCallback);
   XtRemoveAllCallbacks(warning, XmNcancelCallback);

  writesynps();

}/* end dowritesynps*/


/*******************************************************************/
/*                    printsyn(char *ps_name)                      */
/*******************************************************************/
/* create a postscript file of syngraph window */

void printsyn(char *ps_name){

  float scale, tmpscale;
  time_t curtime;
  char datestr[50];
  FILE *ps_fp;


  /* 8 1/2 X 11 = 612 by 792  */
  /* leave a 1 in. border all around */
  /* need to make sure the BoundingBox */
  /* takes into account landscape orientation*/
  /* after 90 deg rotation, origin displaced 72 in y */


  scale = 1.0;
  /* do xr + yb fit on page (1 inch border, 792 x 612)*/
  if( synxr > 648  && synyb > 468){

    scale = (float)648 /(float)synxr;
    tmpscale = (float)468 / (float)synyb;
       if(tmpscale < scale){
           scale = tmpscale;
        }

  }/* check to see which scale prevails */
 else {
    if(synxr > 648 ){
      scale = (float)648 / (float)synxr ;
    }  
    if(synyb  > 468){
     scale = (float)468 / (float)synyb ;
    }
  }

  /* already checked to see if file could be opened in synps*/
  ps_fp = fopen(ps_name, "w");

  fprintf(ps_fp,"%c!PS\n",'%');
  fprintf(ps_fp,"%c%cBoundingBox: 72 72 %d %d\n",'%','%',
  (int)( synyb * scale) + 72, (int)( synxr * scale) + 72); 
  fprintf(ps_fp,"gsave\n");
  fprintf(ps_fp,"/d {def} def \n");
  fprintf(ps_fp,"/o {show} d /m {moveto} d\n");
  fprintf(ps_fp,"/k {lineto stroke} d \n");

/* put date and filename outside of bounding box */  
  fprintf(ps_fp,
    "/Times-Roman findfont 13 scalefont setfont\n");
  fprintf(ps_fp,"72 72 translate 90 rotate\n");
  time(&curtime);
  strcpy(datestr,ctime(&curtime) ); datestr[24] = ' ';
  fprintf(ps_fp, 
       "0 36 m (%s %s) o\n",datestr, ps_name);
  fprintf(ps_fp,"grestore gsave\n");
/* date and filename*/

/* may not always scale char width properly but should work in most cases */
  fprintf(ps_fp,
       "/Times-Roman findfont %d scalefont setfont\n",
         allspectros.list[0]->hchar);
  fprintf(ps_fp,"%f %d translate 90 rotate\n", 
              synyb * scale + 72,  72); 
  fprintf(ps_fp,"%f %f scale\n",scale,scale);

  drawsyn(ps_fp);

  fprintf(ps_fp,"\n showpage grestore\n");
  fclose(ps_fp);
 } /* end printsyn */


/***************************************************************/
/*  writedoc                                                   */
/***************************************************************/
//void writedoc(Widget w,XtPointer client_data, XtPointer call_data){
void writedoc(char *fname) {
/* write a .doc file, put up a warning if one already exists */
Arg args[3];
/* globals from syn */
char *temp,str[200],mess[200];
FILE *fp;
XmString mstr;
int nowarn,n;


#ifdef VMS
  nowarn = 1;
#else
  nowarn = 0;
#endif

//   XtUnmanageChild(input_dialog);
 
//   XtSetArg(args[0],XmNvalue,&temp);
//   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
//   strcpy(str,temp);

//   sprintf(mess,"%s.doc",str);    
  sprintf(mess, "%s.doc", fname);

   /* file does not exist */
   if( !(fp = fopen(mess,"r")) || nowarn ){
       if((fp = fopen(mess,"w")) ){
          fclose(fp);
          strcpy(firstname,fname);
          makefilenames();
	  savedocfile();
         }/* write it */
       else{
        sprintf(mess,"can't open %s.doc\n",fname);
        writetext(mess);
      }
     }/* no warning */
  /* file already exists */
  else{
   sprintf(mess,"OVERWRITE %s.doc ??",fname);
   mstr = XmStringCreateLocalized(mess);
   n = 0;
   XtSetArg(args[n], XmNmessageString,mstr); n++;
   XtSetValues(warning2,args,n);
   XmStringFree(mstr);
   mstr = XmStringCreateLocalized("OK");
   XtSetArg(args[0], XmNokLabelString,mstr);
   XtSetValues(warning2,args,1);
   strcpy(synstr,fname);
   XtRemoveAllCallbacks(warning2, XmNokCallback);
   XtRemoveAllCallbacks(warning2, XmNcancelCallback);  
   XtAddCallback(warning2,XmNokCallback,warndoc,NULL);
   XtAddCallback(warning2,XmNcancelCallback,cancelwarning,NULL);
   opendialog(warning2);
 }/* put up warning*/

}/* writedoc */


/***************************************************************/
/*  warndoc(Widget w,XtPointer client_data, XtPointer call_data)  */
/***************************************************************/
void warndoc(Widget w,XtPointer client_data, XtPointer call_data){
char mess[250];
FILE *fp;

   XtUnmanageChild(warning2);


      sprintf(mess,"%s.doc",synstr);
      if((fp = fopen(mess,"w")) ){
       fclose(fp);
       strcpy(firstname,synstr);
       makefilenames();
       savedocfile();
     }
     else{
       sprintf(mess,"can't open %s.doc\n",synstr);
       writetext(mess);
      }

   XtRemoveAllCallbacks(warning2, XmNokCallback);
   XtRemoveAllCallbacks(warning2, XmNcancelCallback);

}/* end warndoc */


/****************************************************************/
/*  syndef(Widget w,XtPointer client_data, XtPointer call_data)    */
/****************************************************************/
void syndef(Widget w,XtPointer client_data, XtPointer call_data){


   XSPECTRO *spectro;
   spectro = allspectros.list[0];

/*
  read defaults for synthesizer
*/

   XtUnmanageChild(warning);

   /* use defaults */
   initconfig(spectro->synDefPath);
   clearpar();
   initpars();

   XtRemoveAllCallbacks(warning, XmNokCallback);
   XtRemoveAllCallbacks(warning, XmNcancelCallback);
   XtRemoveAllCallbacks(warning, XmNhelpCallback);
   XtUnmanageChild(
     XmMessageBoxGetChild(warning,XmDIALOG_HELP_BUTTON) );

   /* put up syn command window */
   /* commands handled in dosyn*/

   open_tl_dialog(syn_tl);

}/* end syndef*/

/****************************************************************/
/*  syndoc(Widget w,XtPointer client_data, XtPointer call_data)     */
/****************************************************************/
void syndoc(Widget w,XtPointer client_data, XtPointer call_data){
int n;
Arg args[3];
XmString mstr;

   /* this button is going away so make another one active */
   XmProcessTraversal(warning, XmTRAVERSE_HOME);
   XtUnmanageChild(warning);

   /* put up file browser file browser callback read .doc file*/

   XtAddCallback(filesb_dialog,XmNokCallback,rdoc,NULL);
   XtAddCallback(filesb_dialog,XmNcancelCallback,cancelfilesb,NULL);
   mstr = XmStringCreateLocalized("*.doc");
   XmFileSelectionDoSearch(filesb_dialog,mstr);
   XmStringFree(mstr);
   n=0;
   XtSetArg(args[n],XmNtitle,"syn"); n++;
   XtSetValues(XtParent(filesb_dialog),args,n);
   opendialog(filesb_dialog);

   XtRemoveAllCallbacks(warning, XmNokCallback);
   XtRemoveAllCallbacks(warning, XmNcancelCallback);
   XtRemoveAllCallbacks(warning, XmNhelpCallback);
   XtUnmanageChild(
     XmMessageBoxGetChild(warning,XmDIALOG_HELP_BUTTON) );

}/* end syndoc*/

/****************************************************************/
/*    rdoc(Widget w,XtPointer client_data, XtPointer call_data)     */
/****************************************************************/
void rdoc(Widget w,XtPointer client_data, XtPointer call_data){ 
Arg args[3];
char *temp,mess[320],str[300];
FILE *fp;
//extern int defval[];       /* Default values for all parameters */
//extern float AFval[];
//int n;

   fp = NULL;


   XtUnmanageChild(filesb_dialog);
 
   XtSetArg(args[0],XmNvalue,&temp);
   XtGetValues(XmFileSelectionBoxGetChild(filesb_dialog,XmDIALOG_TEXT),args,1);

   strcpy(str,temp);
   XtFree(temp);
  
   XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);

 
#ifdef VMS
    for(i = strlen(str) - 1; i >= 0; i-- )
         if(str[i] == ';') break;

         str[i] = '\000';
#endif

   
  if(  !(fp = fopen(str,"r")) ||

       (strcmp(&str[strlen(str) - 3 ],"doc") != 0 &&
        strcmp(&str[strlen(str) - 3 ],"DOC") != 0 ) ){
       sprintf(mess,"can't open: %s\n",str);
       writetext(mess);
       return ;   
     }/* can't find file */
  if(fp)
      fclose(fp);

  /* strip .doc */
  str[strlen(str) - 4] = '\000';/* remove .doc*/

   /* firstname of doc. file */
   strcpy(firstname,str);
   makefilenames();
   readdocfile();

   // load FP equivalents
   //   for (n=44; n<49; n++)
   //       AFval[n-44] = defval[n];

   sprintf(mess,"read %s.doc\n",firstname);
   writetext(mess);   


   open_tl_dialog(syn_tl);

}/* end rdoc*/


/************************************************************/
/*                         showdoc()                        */
/************************************************************/              
void showdoc() {

int n;
char str[5000];
char temp[3500];
Arg args[1];
char symspace;
Widget syndialog;
extern char cv[];
extern char symb[][4];     /* 2 or 3 char symbol for parameter */
extern int maxval[];       /* Maximaum values for all parameters */
extern int minval[];       /* Minimum values for all parameters */
extern int defval[];       /* Default values for all parameters */
extern	char definition[][51];
extern int npars;
//extern float AFval[];

/* should switch this to line by line like showvaried */

/* if the text widget is null create one */
if (syntext == NULL){

  XtSetArg(args[0],XmNdeleteResponse, XmUNMAP); 
  syndialog = XtCreatePopupShell("SYN",topLevelShellWidgetClass,
                                    cmdw,args,1);
  createtext(syndialog,&syntext);


}/* no text widget yet */


   /* see if text widget is already managed */
   if(!XtIsManaged(XtParent(XtParent(syntext))))
           {XtPopup(XtParent(XtParent(syntext)),XtGrabNone);}

/* display synthesizer configuration*/
strcpy(str, "\n       KLATT SYNTHESIZER\n");
sprintf(temp, "       CURRENT CONFIGURATION:    (%2d parameters)\n",npars);
strcat(str,temp);
strcat(str, "   SYM V/C  MIN   VAL   MAX  DESCRIPTION\n");
strcat(str, "-------------------------------------------------\n");

        for (n=0; n<npars; n++) {
	    sprintf(temp, "%2d. ", n+1);
	    strcat(str,temp);

	    if (symb[n][2] == '\0')  {symspace = ' ';}
	    else {symspace = symb[n][2];}

#ifdef _HACK_TO_SUPPORT_FP_AF_PARAMS
	    if (n>=44 && n <=48) {
		sprintf(temp, "%c%c%c  %c %4d %5g %5d %s\n",
			symb[n][0], symb[n][1], symspace, cv[n], minval[n],
			AFval[n-44], maxval[n], &definition[n][0]);
	    } else {
		sprintf(temp, "%c%c%c  %c %4d %5d %5d %s\n",
			symb[n][0], symb[n][1], symspace, cv[n], minval[n],
			defval[n], maxval[n], &definition[n][0]);
	    }
#else
            sprintf(temp, "%c%c%c  %c %4d %5d %5d %s\n",
              symb[n][0], symb[n][1], symspace, cv[n], minval[n],
	      defval[n], maxval[n], &definition[n][0]);
#endif
            strcat(str,temp);

          }/* all parameters */
         
  strcat(str,"\n");

  n = 0;
  XtSetArg(args[n],XmNvalue,str); n++;
  /* maybe this should be fallback so people can change something*/
  XtSetValues(syntext,args,n);


 /* set flag so this can be updated if constant value is changed*/
    which_syntext = 1; 

}/*showdoc*/


/*******************************************************/
/*                showvaried()                         */
/*******************************************************/
void showvaried()
{
  char str[200],temp[200];
  Widget syndialog;
  Arg args[2];
  int i, n;
  extern char cv[];
  extern char symb[][4];     /* 2 or 3 char symbol for parameter */
  extern int npars,nvar;
  extern int pdata[][MAX_FRAMES],nframes,ms_frame;
  
  
  /* if the text widget is null create one */
  if (syntext == NULL){
    
    XtSetArg(args[0],XmNdeleteResponse, XmUNMAP); 
    syndialog = XtCreatePopupShell("SYN",topLevelShellWidgetClass,
				   cmdw,args,1);
    createtext(syndialog,&syntext);
    
    
  }/* no text widget yet */
  
  
  /* see if text widget is already managed */
  if(!XtIsManaged(XtParent(XtParent(syntext))))
    {XtPopup(XtParent(XtParent(syntext)),XtGrabNone);}
  
  
  /* clear everything first */
  n = 0;
  XtSetArg(args[n],XmNvalue," "); n++;
  /* maybe this should be fallback so people can change something*/
  XtSetValues(syntext,args,n);


/* put labels across the top */
   XSetForeground(XtDisplay (syntext), XtScreen(syntext)->default_gc,
                  WhitePixel(XtDisplay(syntext),
			     DefaultScreen(XtDisplay(syntext))));

         sprintf(str, "\n\nVaried Parameters:\n");
         XmTextInsert(syntext,XmTextGetLastPosition(syntext),str);

	if (nvar == 0) {
	   sprintf(str, "    none");
           XmTextInsert(syntext,XmTextGetLastPosition(syntext),str);
	}
	else {
	    sprintf(str, "time");
            for (n=0; n<npars; n++) {
                if (cv[n] == VARRIED) {
                    sprintf(temp, "   %s", &symb[n][0]);
                    strcat(str,temp);
		}
            }
        }
        strcat(str, "\n");
        XmTextInsert(syntext,XmTextGetLastPosition(syntext),str);



   if (nvar == 0) {
        sprintf(str, "\tNo parameters are varied\n");
        XmTextInsert(syntext,XmTextGetLastPosition(syntext),str);
    }
    else {
        for (n=0; n<nframes; n++) {
	    sprintf(str, "%4d", ms_frame * n); 
            for (i=0; i<npars; i++) {
                if (cv[i] == VARRIED) {
                    sprintf(temp, "%5d", pdata[i][n]);
                    strcat(str,temp);
                }
            }
          strcat(str, "\n");
          XmTextInsert(syntext,XmTextGetLastPosition(syntext),str);
	  }
        sprintf(str, "\n");
        XmTextInsert(syntext,XmTextGetLastPosition(syntext),str);
      }

 /* set flag so this can be updated if constant value is changed*/
    which_syntext = 2; 


 }/* end showvaried */


/*****************************************************************/
/*                     drawsyn(FILE *postfp)                     */
/*****************************************************************/
void drawsyn(FILE *postfp) {
  /*draw to synmap(screen) if fps in null else to postscript file*/
  /* the synthesizer uses globals declared in the "syn" modules */
  /* and a few X globals declared in this module */
  
  extern int pdata[][MAX_FRAMES],nframes,np, ms_frame,maxval[],spkrdef[];
  extern char symb[][4];
  int allnps[7]; /* store np's of formants */
  int graphmax;
  float xratio, yratio, xinc;
  Display *display;
  int i, count;
  float ms,totms;
  XPoint *points;
  int yps;
  int d,dsets;
  static char mstxt[] = "ms";
  char str[10];
  float yval;
  int tmax,tstep;
  int do_formants;
  
/*  all graphs start at 0 on y-axis regardless of minval */
  
  XSPECTRO *spectro;  /* use the first spectro structure on the list*/
  /*  to get access to char width and height etc. */
  int yt,xt;
  int x,x2,y,y2; /* borders */
  int x1,y1,y0; /* offset for data */
  float xmax,ymax; 
  int xpdata;

  spectro = allspectros.list[0];

  /* it's ok to use spectro functions that use char height */
  /* etc. that aren't tied to  window size*/

  y2 = synyb - spectro->hchar;
  x2 = synxr - 4 * spectro->wchar;
  y = yoffset(spectro);
  x = xoffset(spectro) - spectro->axisdist;

  xmax = x2 - x - spectro->axisdist ;
  ymax = y2 - y - spectro->axisdist - 1;

  x1 =  xoffset(spectro);
  y1 =  y2 - spectro->axisdist - 1;


  /* flip y back again for postscript */
  yps = synyb;

  display = XtDisplay(syngraph);


if(postfp == NULL){
/* clear the pixmap */
   XSetForeground(display,syngc,
                  WhitePixel(display,DefaultScreen(display))  );

   XFillRectangle(display,synmap,
     syngc, 0,0,
     synxr,synyb);

   XSetForeground(display,syngc,
                  BlackPixel(display,DefaultScreen(display))  );
 }/* clear pixmap*/


  /* draw borders */
  if(postfp != NULL){
  fprintf(postfp, "%d %d m %d %d k\n", x, yps - y, x, yps - y2);
  fprintf(postfp, "%d %d m %d %d k\n", x, yps - y, x2, yps - y);
  fprintf(postfp, "%d %d m %d %d k\n", x2, yps - y, x2, yps - y2);
  fprintf(postfp, "%d %d m %d %d k\n", x, yps - y2, x2, yps - y2);
   }
  else{
   XDrawLine(display,synmap, syngc,x,y,x,y2);

   XDrawLine(display,synmap, syngc,x,y,x2,y);

   XDrawLine(display,synmap, syngc,x2,y,x2,y2);

   XDrawLine(display,synmap, syngc,x,y2,x2,y2);
   }

/* flag for drawing all formants on same graph */
do_formants = 0;

/* do F1,F2,F3,F4,F5,F6 together if any one of these is selected */
 if (symb[np][0] == 'F' && 
       (symb[np][1] == '1' || symb[np][1]  == '2' ||
        symb[np][1] == '3' || symb[np][1]  == '4' ||
        symb[np][1] == '5' || symb[np][1]  == '6')   ){
         
     
            for(i = 0; i < spkrdef[findnp("NF")]; i++){
                     sprintf(str,"F%d",i+1);
                     allnps[i] = findnp(str);
		   }/* get ready to draw all formants */
            graphmax = maxval[findnp("F0")];
            dsets = i;
          
	}/* is it a formant */

else{
  allnps[0] = np;
  graphmax = maxval[np] ;
  dsets = 1;
}

/*label  ms*/
  x =  synxr - 3 * spectro->wchar + 1; /* far left of window */
  y = synyb - 2; 
    if(postfp != NULL){
    fprintf(postfp, "%d %d m (%s) o\n", x, yps - y, mstxt);
    }
    else{
     XDrawString(display,synmap,syngc,
           x,y,mstxt,strlen(mstxt)  );
   }

/*  label  y axis  no units just name of parameter */

   x =  synxr - 3 * spectro->wchar;
   y = 2 * spectro->hchar ;



  /* might have several labels for formants */
for(d =  dsets - 1; d >= 0; d--){
  if(postfp != NULL){
    fprintf(postfp, "%d %d m (%s) o\n", x, yps - y,symb[allnps[d] ] );
    }
    else{
   XDrawString(display,synmap,syngc,
           x,y,symb[allnps[d] ] ,strlen(symb[allnps[d] ])   );
   }
    y += 2 * spectro->hchar;

  }/* all labels */


for(d = 0; d < dsets; d++){
/* data points */


  points = (XPoint *)malloc(sizeof(XPoint) * nframes * 2);


  totms = nframes * ms_frame;
  /* need to make space for labels */
  xratio = xmax / totms;
  yratio = ymax / (float)graphmax;


  count = 0;
  xpdata = 0;

  for(i = 0; i < nframes; i++){
  /* each member of pdata becomes two points */
    
    points[count].x = xratio * (float)xpdata + .5 + x1;
    points[count].y = y1 - (int)(yratio * (float)pdata[allnps[d]][i] + .5);
    count++;
    xpdata += ms_frame;
    points[count].x = xratio * (float)xpdata + .5 + x1;
    points[count].y = y1 - (int)(yratio * (float)pdata[allnps[d]][i] + .5);
    count++;
  
  }/* all points */ 

/*xinc = xratio * ms_frame;*/
  xinc = xmax / (float)nframes;


if(postfp){

  fprintf(postfp," .5 setlinewidth\n");
  fprintf(postfp,"/str 20 string def\n");


/* drawline for first interval */
 fprintf(postfp,
    "%.1f %.1f m\n",(float)points[0].x, (float)(yps - points[0].y));

  fprintf(postfp,
    "%.1f %.1f k\n",(float)points[0].x + xinc, (float)(yps - points[0].y));

  fprintf(postfp,
    "%.1f %.1f m\n",(float)points[0].x + xinc, (float)(yps - points[0].y));

  fprintf(postfp,"/x %.1f def\n",  (float)points[0].x + xinc   );


  /* now loop through and draw 2 lines per interval */
  fprintf(postfp,
  "%d{currentfile str readline pop cvr dup x exch k dup x exch m\n",
                (nframes - 1 ) );
  fprintf(postfp,"x %f add /x exch def\n",xinc);
  fprintf(postfp,"dup x exch k x exch m}repeat\n");

    /* every two y points are the same */
    for(i = 2; i <= count ; i += 2){
      fprintf(postfp,"%.1f\n",(float)(yps - points[i].y));
    }

  
  fprintf(postfp," 1.0 setlinewidth\n");

}/* write to postcsript file */


else{
   /* draw on screen */

   XDrawLines(display,synmap,
         syngc, points,count,CoordModeOrigin);


 }/* draw on screen */

  free(points);


}/* all sets of data (all formants) */


/* draw y tick marks no units */

  /*yratio = ymax / (float)graphmax;*/

   tstep = (float)spectro->hchar * 1.5 * (float)graphmax / (float)ymax ;
   if(graphmax  > 4000)
              {      tstep =  tstep + 500 -  (tstep % 500);   }
    else if( graphmax > 800)
              {       tstep =  tstep + 100 -  (tstep % 100);   }
    else if( graphmax > 150)
              {       tstep =  tstep + 50 -  (tstep % 50);   }           
    else if( graphmax > 50)
              {       tstep =  tstep + 5 -  (tstep % 5);   }           
    else
      {tstep =  tstep + 1 - (tstep % 1);}

/* always start at 0*/
     yval = 0.0;
     tmax = graphmax / tstep;
     x2 = xoffset(spectro);
     x = x2 - spectro->axisdist; 
     y0 = synyb - spectro->hchar - spectro->axisdist - 1;
     y = y0 - (int)(yratio * yval + .5);
     yt = y + spectro->hchar/ 2.0;
     for( i = 0; i <= tmax; i++){
     sprintf(str,"%d",(int)yval );

     xt = x - (spectro->wchar * strlen(str));

    if(postfp != NULL){

     fprintf(postfp, "%d %d m %d %d k\n",x , yps - y, x2, yps - y);

     fprintf(postfp, "%d %d m (%s) o\n",xt, yps - yt, str);
     }
    else{

    XDrawLine(display,synmap, syngc,x,y,x2,y);

    XDrawString(display,synmap,syngc,
           xt,yt,str,strlen(str));
    }
  
    yval += tstep;
    y = y0 - (int)(yratio * yval+ .5);
    yt = y + spectro->hchar/ 2.0; 
   }
/* end  y tick marks */



/* now draw x tick marks */
/* see what the longest number is */

      sprintf(str,"%d",(int)totms);

      tstep = (float)spectro->wchar *  (float)strlen(str) * totms / xmax ;
   if(  totms > 150.0)
              {       tstep =  tstep + 50 -  (tstep % 50);   }

    else if( totms> 80.0)
              {       tstep =  tstep + 10 -  (tstep % 10);   }
           

    else if( totms > 50.0)
              {       tstep =  tstep + 5 -  (tstep % 5);   }
           

      else{
      tstep =  tstep + 1 - (tstep % 1);
      }

y =  synyb - spectro->hchar - 1;
y2 = synyb - spectro->hchar - 
    spectro->axisdist - 1 + spectro->tickspace;
yt = synyb - 2;

  tmax = totms / tstep ;

  ms = 0.0;
  x = xoffset(spectro);

  for( i = 0; i <= tmax; i++){

      sprintf(str,"%d",(int)ms );

    xt = x - spectro->wchar * strlen(str)  / 2 + 3;

    if(xt + spectro->wchar * strlen(str) / 2 + 5 <
         synxr - spectro->wchar * 3){

    if(postfp != NULL){
     fprintf(postfp, "%d %d m %d %d k\n",x , yps - y, x, yps - y2);
     fprintf(postfp, "%d %d m (%s) o\n",xt, yps - yt, str);
     }
     else{
       XDrawLine(display,synmap, syngc,x,y,x,y2);
       XDrawString(display,synmap,syngc,
           xt,yt,str,strlen(str));
      }
     } /*don't draw into ms label*/ 
    ms += tstep; 
    x = ms * xratio + .5 + xoffset(spectro);
   }  /* x tick marks */

}/* end drawsyn */


/*****************************************************************/
/*                  syn_pixmap()                                 */
/*****************************************************************/
void syn_pixmap() {
  /* used for startup and resize */
  Display *display;

  display = XtDisplay(syngraph);

  if(synmap) XFreePixmap(display, synmap);

   /* create pixmap */
   synmap = XCreatePixmap(display,
        XtWindow(syngraph),synxr,synyb,
        DefaultDepth(display,DefaultScreen(display) ));


   XSetForeground(display,syngc,
                  WhitePixel(display,DefaultScreen(display))  );

   XFillRectangle(display,synmap,
     syngc, 0,0,
     synxr,synyb); 

 }/* end syn_pixmap */


/**************************************************************/
/*   void resizesyn(Widget ,XtPointer, XmAnyCallbackStruct *)    */
/**************************************************************/

void resizesyn(Widget w,XtPointer client_data,
                 XtPointer call_data){ 

XmAnyCallbackStruct *c_data = ( XmAnyCallbackStruct *) call_data;

int n;
Arg args[3];
Dimension width, height;

    /* get width and height of window */
    n = 0;
    XtSetArg(args[n], XmNwidth, &width); n++;
    XtSetArg(args[n], XmNheight, &height); n++;
    XtGetValues(w,args,n);
    synxr = width;    synyb = height;
   /* left out the Dec Motif "spurious" expose workaround*/
   /* since there is just the pixmap and no drawing on top*/
   /* it is not visible  and hopefully the problem will be*/
   /* fixed in later versions*/

  syn_pixmap();
  drawsyn(NULL);   

  /* resize should cause complete exp. but
     doesn't on dec 5000
  */
   XCopyArea(XtDisplay(syngraph),synmap,
            XtWindow(syngraph),syngc,
            0,0,synxr,synyb,0,0);

  
}/* end resizesyn */


/******************************************************************/
/* void expsyn(Widget w, XtPointer client_data, XtPointer call_data)*/
/******************************************************************/
 
void expsyn(Widget w, XtPointer client_data,
             XtPointer call_data){ 

XmDrawingAreaCallbackStruct *c_data = (XmDrawingAreaCallbackStruct *) call_data;

int x,y,width,height;

if(synmap){
   x = c_data->event->xexpose.x;  y = c_data->event->xexpose.y;
   width = c_data->event->xexpose.width;
   height = c_data->event->xexpose.height;

 

   XCopyArea(XtDisplay(syngraph),synmap,
            XtWindow(syngraph),syngc,
            x ,y,width,height,x,y);

 }/* there is a pixmap */
}/* end expsyn */


/*********************************************************************/
/*	       inputparams(XSPECTRO *spectro,int index);	      */
/*********************************************************************/
void inputparams(XSPECTRO *spectro, int index) 
{
  char str[100], t_str[100];
  Arg args[3];
  int n;
  XmString mstr;

/* 
 * param is a global used in the input dialog callback
 * and shared by all the spectros
 *
 * this function adds callbacks to the input dialog
 * widget to change the selected parameter	    
 */

/* change parameters used in spectrum */

  sb_param.min = paramin[index];
  sb_param.max = paramax[index];
  sb_param.index = index;
  
  if(sb_param.index < 4){
    sprintf(t_str,"%.1f",spectro->hamm_in_ms[sb_param.index]);
  }
  else{
    sprintf(t_str,"%d",spectro->params[sb_param.index]);
  }
  n = 0;
  XtSetArg(args[n],XmNvalue,t_str); n++;
  XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,n);
  
  sprintf(str,"%c: (%d - %d)",paramlist[index][0], 
	  sb_param.min, sb_param.max);
  mstr = XmStringCreateLocalized(str);
  XtSetArg(args[0], XmNselectionLabelString, mstr);
  XtSetValues(input_dialog,args,1);
  XmStringFree(mstr);
  
  XtAddCallback(input_dialog,XmNokCallback,setparam, 
		(XtPointer) spectro);
  XtAddCallback(input_dialog,XmNhelpCallback,setparam,
		(XtPointer) spectro);
  XtAddCallback(input_dialog,XmNcancelCallback,cancelinput,
		(XtPointer) spectro);
  
  HighlightInput (t_str, XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT));
  
  opendialog(input_dialog);
}/* end inputparams */



/***************************************************************/
/*  vartime(Widget w,XtPointer client_data, XtPointer call_data)  */
/***************************************************************/
void vartime(Widget w,XtPointer client_data, XtPointer call_data){
  /* get a value for a synthesizer time varying param */
  Arg args[3];
  int time;
  char str[300];
  char input_str[100];
  char *temp;
  XmString mstr;
  int n;
  int begin;
  /*syn globals*/
  extern int pdata[][MAX_FRAMES],np,ms_frame,nframes;
  extern int val,lastval,nf,lastnf;
  extern char symb[][4];
  
  XtSetArg(args[0],XmNvalue,&temp);	      
  XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
  strcpy (str, "");
  strcpy(str,temp);
  time = atoi(str);
  
  
  /* np = impluse */

  if(np == 61){
    if(time < 0 || time >= ms_frame * nframes){
      writetext(" invalid time \n");
       return;
    }	
  }
  else if(time < 0 || time > ms_frame * nframes){
    writetext(" invalid time \n");
    return;
  }
  
  
   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
  
  
  /* if impulse, just do single time step */
  
  /* impulse is special case */ 
  if (np == 61){
    sprintf(str,"%s:Time:%d Val:",symb[np],time );
    XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
    mstr = XmStringCreateLocalized(str);
    n=0;
    XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
    XtSetValues(input_dialog,args,n);
    XmStringFree(mstr);
  }
  /* all cases besides impulse */
  else{
    begin = (int )client_data;  /* Is this the first point ? */
    
    if(begin)
      {
	sprintf(str,"%s:Time:%d Val:",symb[np],time);
	XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
	mstr = XmStringCreateLocalized(str);
	n=0;
	XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
	XtSetValues(input_dialog,args,n);
	XmStringFree(mstr);
      }/* start of a sequence */
    
    else
      {
	lastnf = nf;
	lastval = val;
	sprintf(str,"%s:Time:%d Val:%d-Time:%d Val:",symb[np],
		nf * ms_frame ,pdata[np][lastnf],time);
	XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);

	mstr = XmStringCreateLocalized(str);
	n=0;
	XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
	XtSetValues(input_dialog,args,n);

	XmStringFree(mstr);
      }
    
  }
  
  nf = time / ms_frame;
  sprintf(str, "%d",pdata[np][nf]); 
  XtSetArg(args[0],XmNvalue, str); 
  XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
  XtAddCallback(input_dialog,XmNokCallback,setvar,(XtPointer)begin);

  HighlightInput (input_str, XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT));

}/* end vartime*/


/*************** void HighlightInput() *****************/
/*** Highlight the default text in an input box ***/

void HighlightInput(char *input_str, Widget text)
{
  strcpy(input_str, XmTextGetString(text));
  if (strlen(input_str) != 0)
    XmTextSetSelection(text, 0, strlen(input_str), 0);
	 
}  /* end highlight input */
  



/***************************************************************/
/*  setvar(Widget w,XtPointer client_data, XtPointer call_data)	 */
/***************************************************************/
void setvar(Widget w,XtPointer client_data, XtPointer call_data){
/* get a value for a synthesizer time varying param */
  Arg args[3];
  int newval;
  char str[300],mess[50];
  char *temp;
  int begin;
  int n;
  XmString mstr;

  /*syn globals*/
  extern int minval[],maxval[];
  extern int np,ms_frame,nf,pdata[][MAX_FRAMES];
  extern int val,lastval,nf,lastnf;
  extern char symb[][4];

  begin = (int)client_data;

  XtSetArg(args[0],XmNvalue,&temp);	      
  XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
  strcpy(str,temp);
  newval = atoi(str);

/********************************************************************/
  /* check range */
   val = newval;
  
   if(val < minval[np] || val > maxval[np]){
     sprintf(str,"valid range is %d to %d\n", minval[np], maxval[np]);
     printf("valid range is %d to %d\n", minval[np], maxval[np]);

     writetext(str);

     /*out of range*/
     sprintf(mess,"value out of range: %d",val);
     mstr = XmStringCreateLocalized(mess); 
     /* just a message no callbacks */

     ShowOops(mess);  
     XmStringFree(mstr);
     return;
   }

   pdata[np][nf] = val;
   XtRemoveAllCallbacks(input_dialog, XmNokCallback);

/* impulse special case */
/* should use symbol */
if(np == 61){
   sprintf(str,"%s:Time:",symb[np]);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized(str);
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   XtSetArg(args[0],XmNvalue,"");
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);

   /* just a single time step */    
   lastval = val;
   lastnf = nf;
   nf++;
      if(syntext){
      if(XtIsManaged(XtParent(syntext)) &&
	  which_syntext == 1)
	   {showvaried();}
    }
}

else{
  /* get another time and value */
   sprintf(str,"%s:Time:%dVal:%d-Time:",symb[np],nf * ms_frame,val);
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   mstr = XmStringCreateLocalized(str);
   n=0;
   XtSetArg(args[n],XmNselectionLabelString,mstr); n++;
   XtSetValues(input_dialog,args,n);
   XmStringFree(mstr);
   XtSetArg(args[0],XmNvalue,"");
   XtSetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);

 }/* all other cases */


if(begin && np != 61){

   begin = 0;
   

}/*begin*/

else if(np == 61 || !begin){

/* drawline and get another time and value */
  draw_line();
  syn_pixmap();
  drawsyn(NULL);
  XCopyArea(XtDisplay(syngraph),synmap,
	    XtWindow(syngraph),syngc,
	    0,0,synxr,synyb,0,0);
  
     if(syntext){
      if(XtIsManaged(XtParent(syntext)) &&
	  which_syntext == 1)
	   {showvaried();}
    }
}

   XtAddCallback(input_dialog,XmNokCallback,vartime,(XtPointer)begin);
 

}/* end setvar*/


/***************************************************************/
/*  setcon(Widget w,XtPointer client_data, XtPointer call_data)	 */
/***************************************************************/
void setcon(Widget w,XtPointer client_data, XtPointer call_data){
/* get a value for a synthesizer constant */

Arg args[3];
int newval,n;
XmString mstr;
char str[300],mess[5];
char *temp;
float fVal;
/*syn globals*/
extern int defval[],minval[],maxval[],np,val;
//extern float AFval[];

   XtSetArg(args[0],XmNvalue,&temp); 
   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   strcpy(str,temp);

   newval = atoi(str);

   if(newval < minval[np] || newval > maxval[np]){
    sprintf(str,"Warning: %s valid range is %d to %d\n", lastPar, minval[np], maxval[np]);
    writetext(str);

    /*out of range*/
    //    sprintf(mess,"value out of range: %d",newval );
    //    ShowOops (mess);
    /* just a message no callbacks */
    //return; /* don't remove input dialog yet */
   }
   //   else {

// hack to read FP values for A*F parameters

   //   if (lastPar[0] == 'A' && lastPar[2] == 'F') {
   //       fVal = atof(str);
   //       AFval[lastPar[1]-'0'+2] = fVal;
   //       defval[np] = newval; val = newval; setspdef();
   //       sprintf(str,"%s set to %g\n", lastPar, fVal);

   //   } else {

     defval[np] = newval; val = newval; setspdef();
     sprintf(str,"%s set to %d\n", lastPar, newval);

   //   }

     writetext(str);
      /* may be displaying incorrect info
	 don't know whether syntext is displaying constant or varied */


     if(syntext){
      if(XtIsManaged(XtParent(syntext)) &&
	  which_syntext == 1)
	   {showdoc();}
       }


     //    }

   /* if > max query about override */
  XtRemoveAllCallbacks(input_dialog, XmNokCallback);
  XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
  XtUnmanageChild(input_dialog);


}/* end setcon */


