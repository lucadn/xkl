/*
$Log: synparwav.c,v $
Revision 1.2  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#include <stdio.h>
#include <math.h>
#include "synparwav.h"

/* This data file is included in PARWAVE.C */

#define N_SPDEF_PARS	13	/* Used only for debug printout */
#define N_VARIABLE_PARS	47

#define IMPULSIVE	1
#define NATURAL		2
#define CASCADE		0
#define PARALLEL	1

static int initsw;	/* Used to set speaker def on first call	*/

/* VARIABLES TO HOLD 11 SPEAKER DEFINITION FROM HOST:                   */

static int nspfr;	/* Number of samples in a parameter frame	*/
static int samrate;	/* Number of output samples per second		*/
static int nfcascade;	/* Number of formants in cascade vocal tract	*/
static int glsource;	/* 1->impulsive, 2->natural voicing source	*/
static int ranseed;	/* Seed specifying initial number for ran # gener */
static int sameburst;	/* Use same burst noise sequence if =1, rand if =0*/
static int cascparsw;	/* Cascade/parallel switch, 0=cascade, 1=parallel */
static int outsl;	/* Output waveform selector		  	*/
static int GainAV;	/* Overall gain, 60 dB is unity  0 to   80	*/
static int GainAH;	/* Overall gain, 60 dB is unity  0 to   80	*/
static int GainAF;	/* Overall gain, 60 dB is unity  0 to   80	*/
static int GainAI;      /* 93 Overall gain ?????? */

/* VARIABLES TO HOLD 45 TIME-VARYING INPUT PARAMETERS FROM HOST:	*/

static int F0hz10;  /*   Voicing fund freq in Hz,    0 to 5000	*/
static int AVdb  ;  /*      Amp of voicing in dB,    0 to   70  */
static int OQ    ;  /* Open quotient, in percent,    1 to   99  */
static int SQ    ;  /* Speed quotient, in percent, 100 to  500  */
static int TL    ;  /* Voicing spectral tilt in dB,  0 to   41  */
static int DP    ;  /* Double-pulsing, %	     0 to  100  */
static int FL    ;  /* Flutter, or slow variation F0,0 to   20  */
static int AH    ;  /*   Amp of aspiration in dB,    0 to   70  */
static int AF    ;  /*    Amp of frication in dB,    0 to   80  */
static int AI    ;  /*93 Amp of impulse, in dB       0 to   80  */
static int FSF   ;  /*93Formant spacing filter (0=off 1=on)     */


static int F1hz  ;  /*  First formant freq in Hz,  200 to 1300  */
static int B1hz  ;  /*    First formant bw in Hz,   40 to 1000  */
static int F2hz  ;  /* Second formant freq in Hz,  550 to 3000  */
static int B2hz  ;  /*   Second formant bw in Hz,   40 to 1000  */
static int F3hz  ;  /*  Third formant freq in Hz, 1200 to 4999  */
static int B3hz  ;  /*    Third formant bw in Hz,   40 to 1000  */
static int F4hz  ;  /* Fourth formant freq in Hz, 1200 to 4999  */
static int B4hz  ;  /*    Fourth formant bw in Hz,  40 to 1000  */
static int F5hz  ;  /*  Fifth formant freq in Hz, 1200 to 4999  */
static int B5hz  ;  /*    Fifth formant bw in Hz,   40 to 1000  */
static int F6hz  ;  /*  Sixth formant freq in Hz, 1200 to 4999  */
static int B6hz  ;  /*    Sixth formant bw in Hz,   40 to 2000  */
static int FNZhz ;  /*     Nasal zero freq in Hz,  248 to  528  */
static int BNZhz ;  /*       Nasal zero bw in Hz,   40 to 1000  */
static int FNPhz ;  /*     Nasal pole freq in Hz,  248 to  528  */
static int BNPhz ;  /*       Nasal pole bw in Hz,   40 to 1000  */
static int FTPhz ;  /*  Tracheal pole freq in Hz,  300 to 3000  */
static int BTPhz ;  /*    Tracheal pole bw in Hz,   40 to 1000  */
static int FTZhz ;  /*  Tracheal zero freq in Hz,  300 to 3000  */
static int BTZhz ;  /*    Tracheal zero bw in Hz,   40 to 1000  */

static int A1V   ;  /* Amp of par 1st formant in dB, 0 to   80  */
static int A2V   ;  /* Amp of par 2nd formant in dB, 0 to   80  */
static int A3V   ;  /* Amp of par 3rd formant in dB, 0 to   80  */
static int A4V   ;  /* Amp of par 4th formant in dB, 0 to   80  */
static int A2    ;  /* Amp of F2 frication in dB,    0 to   80  */
static int A3    ;  /* Amp of F3 frication in dB,    0 to   80  */
static int A4    ;  /* Amp of F4 frication in dB,    0 to   80  */
static int A5    ;  /* Amp of F5 frication in dB,    0 to   80  */
static int A6    ;  /* Amp of F6 (same as r6pa),     0 to   80  */
static int AB    ;  /* Amp of bypass fric. in dB,    0 to   80  */
static int AN    ;  /* Amp of par nasal pole in dB,  0 to   80  */
static int AT    ;  /* Amp of par tracheal pole in dB0 to   80  */
static int B1phz ;  /* Par. 1st formant bw in Hz,   40 to 1000  */
static int B2phz ;  /* Par. 2nd formant bw in Hz,   40 to 1000  */
static int B3phz ;  /* Par. 3rd formant bw in Hz,   40 to 1000  */
static int B4phz ;  /*  Par. 4th formant bw in Hz,  40 to 1000  */
static int B5phz ;  /* Par. 5th formant bw in Hz,   40 to 1000  */
static int B6phz ;  /* Par. 6th formant bw in Hz,   40 to 2000  */
static int dB1hz ;  /* Increment to B1hz during open phase of cycle */
static int dF1hz ;  /* Increment to F1hz during open phase of cycle */
static int F0next;  /* Value of f0 in next frame, used to interpolate f0 */

/* SAME VARIABLES CONVERTED TO LINEAR FLOATING POINT */
static float amp_parvF1;/* A1V converted to linear gain		    */
static float amp_parvF2;/* A2V converted to linear gain		    */
static float amp_parvF3;/* A3V converted to linear gain		    */
static float amp_parvF4;/* A4V converted to linear gain		    */
static float amp_parF2;	/* A2 converted to linear gain		    */
static float amp_parF3;	/* A3 converted to linear gain		    */
static float amp_parF4;	/* A4 converted to linear gain		    */
static float amp_parF5;	/* A5 converted to linear gain		    */
static float amp_parF6;	/* A6 converted to linear gain		    */
static float amp_bypas;	/* AB converted to linear gain		    */
static float amp_parNP; /* AN converted to linear gain		    */
static float amp_parTP;	/* AT converted to linear gain	    	    */
static float amp_voice;	/* AVdb converted to linear gain	    */
static float amp_aspir;	/* AH converted to linear gain              */
static float amp_imp  ; /* AI converted to linear gain   93         */
static float amp_frica;	/* AF converted to linear gain		    */
static float amp_gainAV;/* GV converted to linear gain		    */
static float amp_gainAH;/* GH converted to linear gain		    */
static float amp_gainAF;/* GF converted to linear gain		    */
static float amp_gainAI;/* GI converted to linear gain   93         */
static float flutter;

/* COUNTERS */

static int ns    ;  /* Number of samples into current frame         */
/*static int nper  ; debig reset this to 0 */
    int nper  ;  /*rrent loc in voicing period   40000 samp/s */
static int n4    ;  /* Counter of 4 samples in glottal source loop  */

/* COUNTER LIMITS */

static int T0    ;  /* Fundamental period in output samples times 4 */
static int nopen ;  /* Number of samples in open phase of period    */
static int nmod  ;  /* Position in period to begin noise amp. modul */

/* ALL-PURPOSE TEMPORARY VARIABLES */

static int temp    ;
static float temp1 ;

static float minus_pi_t;
static float two_pi_t;

/*static int F1last,F2last,F3last; debug reset to 0 */
int F1last;
int F2last;
int F3last;

static int FLPhz ;	/* Frequeny of glottal downsample low-pass filter */
static int BLPhz ;	/* Bandwidt of glottal downsample low-pass filter */

static int Ffant ;	/* Frequeny of LF glottal resonator */
static int BWfant ;	/* Bandwidth of LF glottal resonator */
static float Afant;	/* Amplitude of impulse exciting LF glottal resonator */

static int F1hzmod;	/* Increase in F1 during open phase of glottal cycle */
static int B1hzmod;	/* Increase in B1 during open phase of glottal cycle */

static int F0interp;	/* Interpolated value of f0 from current & next frame*/
static float glottalnoise;/* First diff. of frication noise, used for aspir  */
static float trachout;

static float anorm1;  /* Normalizing scale factor for F1 sudden change */
static float anorm2;  /* Normalizing scale factor for F2 sudden change */
static float anorm3;  /* Normalizing scale factor for F3 sudden change */
static float r;

static double arg7Hz;	/* Arguments for flutter (sum of sinusoids) of f0 */
static double arg5Hz;
static double arg3Hz;
static double darg7Hz;
static double darg5Hz;
static double darg3Hz;
static double radiansperframe;
static double arg;

/* VARIABLES THAT HAVE TO STICK AROUND FOR AWHILE, AND THUS "temp" */
/* IS NOT APPROPRIATE */

static short nrand ;  /* Varible used by random number generator      */
static int dpulse  ;  /* Double-pulsing, in quarter-sample units      */
			/* Can be same as temp1 */

static float a     ;  /* Makes waveshape of glottal pulse when open   */
static float b     ;  /* Makes waveshape of glottal pulse when open   */
static float voice ;  /* Current sample of voicing waveform           */
static float vwave ;  /* Ditto, but before multiplication by AVdb     */
static float noise ;  /* Output of random number generator            */
static float frics ;  /* Frication sound source                       */
static float aspiration; /* Aspiration sound source                   */
static float sourc ;  /* Sound source if all-parallel config used     */
static float casc_next_in;  /* Input to next used resonator of casc   */
static float out   ;  /* Output of cascade branch, also final output  */
/* 93 */
static float lastout; /* save previous out for filter   */
static float lstinput;  /* save last input to filter x[n]  */
static float previnput; /* save previous input to filter x[n-2] */
static float spacefix; /* pole or zero  used in spacefilt filter*/

static float rnzout;  /* Output of cascade nazal zero resonator	      */
static float glotout; /* Output of glottal sound source before AV&tilt*/
static float tiltout; /* Output of glottal sound source after AV&tilt */
static float par_glotout; /* Output of parallelglottal sound sourc    */
static float outbypas; /* Output signal from bypass path	      */

/* INTERNAL MEMORY FOR DIGITAL RESONATORS AND ANTIRESONATOR           */

 float r2p_1 ;  /* Last output sample from parallel 2nd formant */
 float r2p_2 ;  /* Second-previous output sample                */

 float r3p_1 ;  /* Last output sample from parallel 3rd formant */
 float r3p_2 ;  /* Second-previous output sample                */

 float r4p_1 ;  /* Last output sample from parallel 4th formant */
 float r4p_2 ;  /* Second-previous output sample                */

 float r5p_1 ;  /* Last output sample from parallel 5th formant */
 float r5p_2 ;  /* Second-previous output sample                */

 float r6p_1 ;  /* Last output sample from parallel 6th formant */
 float r6p_2 ;  /* Second-previous output sample                */


 float r1c_1 ;  /* Last output sample from cascade 1st formant  */
 float r1c_2 ;  /* Second-previous output sample                */

 float r2c_1 ;  /* Last output sample from cascade 2nd formant  */
 float r2c_2 ;  /* Second-previous output sample                */

 float r3c_1 ;  /* Last output sample from cascade 3rd formant  */
 float r3c_2 ;  /* Second-previous output sample                */

 float r4c_1 ;  /* Last output sample from cascade 4th formant  */
 float r4c_2 ;  /* Second-previous output sample                */

 float r5c_1 ;  /* Last output sample from cascade 5th formant  */
 float r5c_2 ;  /* Second-previous output sample                */

 float r6c_1 ;  /* Last output sample from cascade 6th formant  */
 float r6c_2 ;  /* Second-previous output sample                */

 float r7c_1 ;  /* Last output sample from cascade 7th formant  */
 float r7c_2 ;  /* Second-previous output sample                */

 float r8c_1;  /* Last output sample from cascade 8th formant  */
 float r8c_2;  /* Second-previous output sample                */

 float rnpc_1;  /* Last output sample from cascade nasal pole   */
 float rnpc_2;  /* Second-previous output sample                */

 float rnz_1 ;  /* Last output sample from cascade nasal zero   */
 float rnz_2 ;  /* Second-previous output sample                */

 float rztr1_1;  /* Last output sample from cascade tracheal pole */
 float rztr1_2;  /* Second-previous output sample                */

 float rptr1_1;  /* Last output sample from cascade tracheal zero */
 float rptr1_2;  /* Second-previous output sample                */

 float rgl_1 ;  /* Last output crit-damped glot low-pass filter */
 float rgl_2 ;  /* Second-previous output sample                */

 float rtlt_1;  /* Last output from TILT low-pass filter    */
 float rtlt_2;  /* Second-previous output sample                */

 float rlp_1 ;    /* Last output from downsamp low-pass filter */
 float rlp_2 ;    /* Second-previous output sample */
/*static glotlast;*/      /* Previous value of glotout                   */
/* debug */
/*static float noiselast;*/ /* Used to first-difference random number for aspir */

/* COEFFICIENTS FOR DIGITAL RESONATORS AND ANTIRESONATOR */

static float r2pa  ;  /* Could be same as A2 if all integer impl.     */
static float r2pb  ;  /* "b" coefficient                              */
static float r2pc  ;  /* "c" coefficient                              */

static float r3pa  ;  /* Could be same as A3 if all integer impl.     */
static float r3pb  ;  /* "b" coefficient                              */
static float r3pc  ;  /* "c" coefficient                              */

static float r4pa  ;  /* Could be same as A4 if all integer impl.     */
static float r4pb  ;  /* "b" coefficient                              */
static float r4pc  ;  /* "c" coefficient                              */

static float r5pa  ;  /* Could be same as A5 if all integer impl.     */
static float r5pb  ;  /* "b" coefficient                              */
static float r5pc  ;  /* "c" coefficient                              */

static float r6pa  ;  /* Could be same as A6 if all integer impl.     */
static float r6pb  ;  /* "b" coefficient                              */
static float r6pc  ;  /* "c" coefficient                              */

static float r1ca  ;  /* Could be same as r1pa if all integer impl.   */
static float r1cb  ;  /* Could be same as r1pb if all integer impl.   */
static float r1cc  ;  /* Could be same as r1pc if all integer impl.   */

static float r2ca  ;   /* "a" coefficient for cascade 2nd formant     */
static float r2cb  ;   /* "b" coefficient                             */
static float r2cc  ;   /* "c" coefficient                             */

static float r3ca  ;   /* "a" coefficient for cascade 3rd formant     */
static float r3cb  ;   /* "b" coefficient                             */
static float r3cc  ;   /* "c" coefficient                             */

static float r4ca  ;   /* "a" coefficient for cascade 4th formant     */
static float r4cb  ;   /* "b" coefficient                             */
static float r4cc  ;   /* "c" coefficient (same as R4Cccoef)          */

static float r5ca  ;   /* "a" coefficient for cascade 5th formant     */
static float r5cb  ;   /* "b" coefficient                             */
static float r5cc  ;   /* "c" coefficient (same as R5Cccoef)          */

static float r6ca  ;   /* "a" coefficient for cascade 6th formant     */
static float r6cb  ;   /* "b" coefficient                             */
static float r6cc  ;   /* "c" coefficient			      */

static float r7ca  ;   /* "a" coefficient for cascade 7th formant     */
static float r7cb  ;   /* "b" coefficient                             */
static float r7cc  ;   /* "c" coefficient                             */

static float r8ca  ;   /* "a" coefficient for cascade 8th formant     */
static float r8cb  ;   /* "b" coefficient                             */
static float r8cc  ;   /* "c" coefficient                             */

static float rnpca ;   /* "a" coefficient for cascade nasal pole      */
static float rnpcb ;   /* "b" coefficient                             */
static float rnpcc ;   /* "c" coefficient                             */

static float rnza  ;   /* "a" coefficient for cascade nasal zero      */
static float rnzb  ;   /* "b" coefficient                             */
static float rnzc  ;   /* "c" coefficient                             */

static float rptr1a;   /* "a" coefficient for cascade tracheal pole   */
static float rptr1b;   /* "b" coefficient                             */
static float rptr1c;   /* "c" coefficient                             */

static float rztr1a;   /* "a" coefficient for cascade tracheal zero   */
static float rztr1b;   /* "b" coefficient                             */
static float rztr1c;   /* "c" coefficient                             */

static float rgla  ;   /* "a" coefficient for crit-damp glot filter   */
static float rglb  ;   /* "b" coefficient                             */
static float rglc  ;   /* "c" coefficient                             */

static float rtlta ;   /* "a" coefficient for TILT low-pass filter    */
static float rtltb ;   /* "b" coefficient                             */
static float rtltc ;   /* "c" coefficient                             */

static float rlpa  ;   /* "a" coefficient for downsam low-pass filter */
static float rlpb  ;   /* "b" coefficient                             */
static float rlpc  ;   /* "c" coefficient                             */

/* OUT static float decay ;  TLTdb converted to exponential time const   */
/* OUT static float onemd ;  in voicing one-pole low-pass filter         */
static int Ftilt,BWtilt;

/*static int dispt   ; */  /* Counter for display of point every 25 ms    */

/* CONSTANTS AND TABLES TO BE PUT IN ROM                              */

#define CASCADE_PARALLEL      1 /* Normal synthesizer config          */
#define ALL_PARALLEL          2 /* Only use parallel branch           */


/*
 * Constant B0 controls shape of glottal pulse as a function
 * of desired duration of open phase N0
 * (Note that N0 is specified in terms of 40,000 samples/sec of speech)
 *
 *    Assume voicing waveform V(t) has form: k1 t**2 - k2 t**3
 *
 *    If the radiation characterivative, a temporal derivative
 *      is folded in, and we go from continuous time to discrete
 *      integers n:  dV/dt = vwave[n]
 *                         = sum over i=1,2,...,n of { a - (i * b) }
 *                         = a n  -  b/2 n**2
 *
 *      where the  constants a and b control the detailed shape
 *      and amplitude of the voicing waveform over the open
 *      potion of the voicing cycle "nopen".
 *
 *    Let integral of dV/dt have no net dc flow --> a = (b * nopen) / 3
 *
 *    Let maximum of dUg(n)/dn be constant --> b = gain / (nopen * nopen)
 *      meaning as nopen gets bigger, V has bigger peak proportional to n
 *
 *    Thus, to generate the table below for 40 <= nopen < 800:
 *
 *	FILE *fopen(), *odev;
 *	int n;
 *		    odev = fopen("temp.c", "w");
 *		    fprintf(odev, "    float B0[760] = {\n\t");
 *		    for (n=40; n<nopenmax; n++) {
 *			b = 1920000. / (n * n);
 *			fprintf(odev, "%6.2f, ", b);
 *			if ((n-9) == ((n/10)*10))    fprintf(odev, "\n\t");
 *		    }
 *		    fprintf(odev, "    };\n\n");
 */
static float B0[760] = {
1200.00,1142.18,1088.44,1038.40, 991.74, 948.15, 907.37, 869.17, 833.33, 799.67,
 768.00, 738.18, 710.06, 683.52, 658.44, 634.71, 612.24, 590.95, 570.75, 551.57,
 533.33, 515.99, 499.48, 483.75, 468.75, 454.44, 440.77, 427.71, 415.22, 403.28,
 391.84, 380.88, 370.37, 360.29, 350.62, 341.33, 332.41, 323.83, 315.58, 307.64,
 300.00, 292.64, 285.54, 278.71, 272.11, 265.74, 259.60, 253.67, 247.93, 242.39,
 237.04, 231.86, 226.84, 221.99, 217.29, 212.74, 208.33, 204.06, 199.92, 195.90,

 192.00, 188.22, 184.54, 180.98, 177.51, 174.15, 170.88, 167.70, 164.61, 161.60,
 158.68, 155.83, 153.06, 150.36, 147.74, 145.18, 142.69, 140.26, 137.89, 135.58,
 133.33, 131.14, 129.00, 126.91, 124.87, 122.88, 120.94, 119.04, 117.19, 115.38,
 113.61, 111.88, 110.19, 108.54, 106.93, 105.35, 103.81, 102.30, 100.82,  99.37,
  97.96,  96.57,  95.22,  93.89,  92.59,  91.32,  90.07,  88.85,  87.66,  86.48,
  85.33,  84.21,  83.10,  82.02,  80.96,  79.92,  78.90,  77.89,  76.91,  75.95,
  75.00,  74.07,  73.16,  72.26,  71.39,  70.52,  69.68,  68.84,  68.03,  67.22,
  66.44,  65.66,  64.90,  64.15,  63.42,  62.69,  61.98,  61.29,  60.60,  59.92,
  59.26,  58.61,  57.96,  57.33,  56.71,  56.10,  55.50,  54.91,  54.32,  53.75,
  53.19,  52.63,  52.08,  51.55,  51.01,  50.49,  49.98,  49.47,  48.97,  48.48,

  48.00,  47.52,  47.05,  46.59,  46.14,  45.69,  45.24,  44.81,  44.38,  43.96,
  43.54,  43.13,  42.72,  42.32,  41.93,  41.54,  41.15,  40.77,  40.40,  40.03,
  39.67,  39.31,  38.96,  38.61,  38.27,  37.93,  37.59,  37.26,  36.93,  36.61,
  36.29,  35.98,  35.67,  35.37,  35.06,  34.77,  34.47,  34.18,  33.90,  33.61,
  33.33,  33.06,  32.78,  32.52,  32.25,  31.99,  31.73,  31.47,  31.22,  30.97,
  30.72,  30.48,  30.23,  30.00,  29.76,  29.53,  29.30,  29.07,  28.84,  28.62,
  28.40,  28.19,  27.97,  27.76,  27.55,  27.34,  27.14,  26.93,  26.73,  26.53,
  26.34,  26.14,  25.95,  25.76,  25.57,  25.39,  25.20,  25.02,  24.84,  24.67,
  24.49,  24.32,  24.14,  23.97,  23.80,  23.64,  23.47,  23.31,  23.15,  22.99,
  22.83,  22.67,  22.52,  22.36,  22.21,  22.06,  21.91,  21.77,  21.62,  21.48,

  21.33,  21.19,  21.05,  20.91,  20.78,  20.64,  20.50,  20.37,  20.24,  20.11,
  19.98,  19.85,  19.72,  19.60,  19.47,  19.35,  19.23,  19.11,  18.99,  18.87,
  18.75,  18.63,  18.52,  18.40,  18.29,  18.18,  18.07,  17.96,  17.85,  17.74,
  17.63,  17.52,  17.42,  17.31,  17.21,  17.11,  17.01,  16.91,  16.81,  16.71,
  16.61,  16.51,  16.42,  16.32,  16.22,  16.13,  16.04,  15.95,  15.85,  15.76,
  15.67,  15.58,  15.50,  15.41,  15.32,  15.24,  15.15,  15.06,  14.98,  14.90,
  14.81,  14.73,  14.65,  14.57,  14.49,  14.41,  14.33,  14.26,  14.18,  14.10,
  14.02,  13.95,  13.87,  13.80,  13.73,  13.65,  13.58,  13.51,  13.44,  13.37,
  13.30,  13.23,  13.16,  13.09,  13.02,  12.95,  12.89,  12.82,  12.75,  12.69,
  12.62,  12.56,  12.49,  12.43,  12.37,  12.31,  12.24,  12.18,  12.12,  12.06,

  12.00,  11.94,  11.88,  11.82,  11.76,  11.71,  11.65,  11.59,  11.53,  11.48,
  11.42,  11.37,  11.31,  11.26,  11.20,  11.15,  11.09,  11.04,  10.99,  10.94,
  10.88,  10.83,  10.78,  10.73,  10.68,  10.63,  10.58,  10.53,  10.48,  10.43,
  10.38,  10.34,  10.29,  10.24,  10.19,  10.15,  10.10,  10.05,  10.01,   9.96,
   9.92,   9.87,   9.83,   9.78,   9.74,   9.70,   9.65,   9.61,   9.57,   9.52,
   9.48,   9.44,   9.40,   9.36,   9.32,   9.27,   9.23,   9.19,   9.15,   9.11,
   9.07,   9.03,   9.00,   8.96,   8.92,   8.88,   8.84,   8.80,   8.77,   8.73,
   8.69,   8.65,   8.62,   8.58,   8.55,   8.51,   8.47,   8.44,   8.40,   8.37,
   8.33,   8.30,   8.26,   8.23,   8.20,   8.16,   8.13,   8.10,   8.06,   8.03,
   8.00,   7.96,   7.93,   7.90,   7.87,   7.84,   7.80,   7.77,   7.74,   7.71,

   7.68,   7.65,   7.62,   7.59,   7.56,   7.53,   7.50,   7.47,   7.44,   7.41,
   7.38,   7.35,   7.32,   7.30,   7.27,   7.24,   7.21,   7.18,   7.16,   7.13,
   7.10,   7.07,   7.05,   7.02,   6.99,   6.97,   6.94,   6.91,   6.89,   6.86,
   6.84,   6.81,   6.78,   6.76,   6.73,   6.71,   6.68,   6.66,   6.63,   6.61,
   6.58,   6.56,   6.54,   6.51,   6.49,   6.46,   6.44,   6.42,   6.39,   6.37,
   6.35,   6.32,   6.30,   6.28,   6.26,   6.23,   6.21,   6.19,   6.17,   6.14,
   6.12,   6.10,   6.08,   6.06,   6.04,   6.01,   5.99,   5.97,   5.95,   5.93,
   5.91,   5.89,   5.87,   5.85,   5.83,   5.81,   5.79,   5.77,   5.75,   5.73,
   5.71,   5.69,   5.67,   5.65,   5.63,   5.61,   5.59,   5.57,   5.55,   5.53,
   5.52,   5.50,   5.48,   5.46,   5.44,   5.42,   5.41,   5.39,   5.37,   5.35,

   5.33,   5.32,   5.30,   5.28,   5.26,   5.25,   5.23,   5.21,   5.19,   5.18,
   5.16,   5.14,   5.13,   5.11,   5.09,   5.08,   5.06,   5.04,   5.03,   5.01,
   4.99,   4.98,   4.96,   4.95,   4.93,   4.92,   4.90,   4.88,   4.87,   4.85,
   4.84,   4.82,   4.81,   4.79,   4.78,   4.76,   4.75,   4.73,   4.72,   4.70,
   4.69,   4.67,   4.66,   4.64,   4.63,   4.62,   4.60,   4.59,   4.57,   4.56,
   4.54,   4.53,   4.52,   4.50,   4.49,   4.48,   4.46,   4.45,   4.43,   4.42,
   4.41,   4.39,   4.38,   4.37,   4.35,   4.34,   4.33,   4.32,   4.30,   4.29,
   4.28,   4.26,   4.25,   4.24,   4.23,   4.21,   4.20,   4.19,   4.18,   4.16,
   4.15,   4.14,   4.13,   4.12,   4.10,   4.09,   4.08,   4.07,   4.06,   4.04,
   4.03,   4.02,   4.01,   4.00,   3.99,   3.97,   3.96,   3.95,   3.94,   3.93,

   3.92,   3.91,   3.90,   3.88,   3.87,   3.86,   3.85,   3.84,   3.83,   3.82,
   3.81,   3.80,   3.79,   3.78,   3.77,   3.76,   3.75,   3.73,   3.72,   3.71,
   3.70,   3.69,   3.68,   3.67,   3.66,   3.65,   3.64,   3.63,   3.62,   3.61,
   3.60,   3.59,   3.58,   3.57,   3.56,   3.55,   3.54,   3.53,   3.53,   3.52,
   3.51,   3.50,   3.49,   3.48,   3.47,   3.46,   3.45,   3.44,   3.43,   3.42,
   3.41,   3.40,   3.40,   3.39,   3.38,   3.37,   3.36,   3.35,   3.34,   3.33,
   3.32,   3.32,   3.31,   3.30,   3.29,   3.28,   3.27,   3.26,   3.26,   3.25,
   3.24,   3.23,   3.22,   3.21,   3.20,   3.20,   3.19,   3.18,   3.17,   3.16,
   3.16,   3.15,   3.14,   3.13,   3.12,   3.12,   3.11,   3.10,   3.09,   3.08,
   3.08,   3.07,   3.06,   3.05,   3.05,   3.04,   3.03,   3.02,   3.02,   3.01,
};


/*
 * Convertion table, db to linear, 87 dB --> 32767
 *                                 86 dB --> 29491 (1 dB down = 0.5**1/6)
 *                                 ...
 *                                 81 dB --> 16384 (6 dB down = 0.5)
 *                                 ...
 *                                  0 dB -->     0
 *
 * The just noticeable difference for a change in intensity of a vowel
 *   is approximately 1 dB.  Thus all amplitudes are quantized to 1 dB
 *   steps.
 */

static float amptable[88] = {
           0.0,    0.0,    0.0,    0.0,    0.0,
	   0.0,    0.0,    0.0,    0.0,    0.0,
	   0.0,    0.0,    0.0,  0.006,  0.007,
 	 0.008,  0.009,  0.010,  0.011,  0.013,
	 0.014,  0.016,  0.018,  0.020,  0.022,
	 0.025,  0.028,  0.032,  0.035,  0.040,
	 0.045,  0.051,  0.057,  0.064,  0.071,
	 0.080,  0.090,  0.101,  0.114,  0.128,
         0.142,  0.159,  0.179,  0.202,  0.227,
	 0.256,  0.284,  0.318,  0.359,  0.405,
	 0.455,  0.512,  0.568,  0.638,  0.719,
	 0.811,  0.911,  1.024,  1.137,  1.276,
	 1.438,  1.622,  1.823,  2.048,  2.273,
	 2.552,  2.875,  3.244,  3.645,  4.096,
	 4.547,  5.104,  5.751,  6.488,  7.291,
	 8.192,  9.093, 10.207, 11.502, 12.976,
	14.582, 16.384, 18.350, 20.644, 23.429,
	26.214, 29.491, 32.767
};




/* The following array converts tilt in dB at 3 kHz to BW of a resonator
   used to tilt down glottal spectrum, set F of resonator to .6 BW */

/* It is probably a good idea, at least for female voices, to set AH
   equal to TILT+36, truncating to a maximum of 56 */

/* Where the 36 is a speaker defining constant */
/* And maybe the 56 is too */

/* And maybe there should be constraints on legal combinations of values
   for OQ and TILT such that small OQ not compatable with large TILT */
/* Or OQ = f(TILT) */

	static int lineartilt[42] = {
		5000,	4350,	3790,	3330,	2930,
		2700,	2580,	2468,	2364,	2260,
		2157,	2045,	1925,	1806,	1687,
		1568,	1449,	1350,	1272,	1199,
		1133,	1071,	 1009,	 947,	 885,
		 833,	 781,	 729,	 677,	 625,
		 599,	 573,	 547,	 521,	 495,
		 469,	 442,	 416,	 390,	 364,
		 338,	 312
	};

/* Measured response of the 2-pole tilt resonator: 

	F	BW	A@250Hz	A@1kHz	A@2kHz	A@3kHz	A@4kHz
	3000	5000	42	42	43	44	44
	2250	3750	42	42	42	42	41
	1500	2500	42	42	41	39	36
	1125	1875	42	42	39	33	30
	 750	1250	42	41	32	27	23
	 567	 937	42	37	27	21	17
	 375	 625	42	32	21	15	11
	 283	 469	42	27	16	 9	 5
	 187	 312	40	22	10	 3	 0
*/

/* Convert SQ (speed quotient) into a negative bandwidth for Fant model */
/* of the glottal waveform (SS=3), these are bandwidths times 10 */

static int bwfanttab[42] = {
	  -0,  -6, -20, -40, -60,  -80,-104,-127,-153,-178,
	-201,-224,-247,-270,-292, -314,-336,-358,-379,-400,
	-421,-441,-462,-483,-504, -524,-545,-566,-587,-608,
	-627,-645,-663,-681,-699, -716,-733,-750,-766,-782,
	-796,-810
};

/* Convert SQ (speed quotient) into gain factor to make Ug(t) peak */
/* constant in Fant model (ss=3), fantgain[(SQ/10 - 10] */

static float fantgain[42] = {
	27.4, 26.3, 25.3, 24.3, 23.2,  22.1, 21.0, 20.0, 18.8, 17.6,
	16.1, 14.9, 13.8, 12.8, 11.7,  10.6, 9.81, 9.00, 8.12, 7.36,
	6.60, 6.05, 5.46, 4.92, 4.41,  3.94, 3.58, 3.14, 2.83, 2.49,
	2.24, 2.03, 1.83, 1.63, 1.48,  1.32, 1.19, 1.08, .982, .902,
	.832, .770
};

#if 0
/* CODE BELOW THIS POINT ONLY NEEDED IF CONVERT TO FLOATING-POINT CHIP IMPL. */

/*   costab = 8192 * cos(2 pi F / 10,000) where F=0,8,16,...,4996 */
/*    These numbers should be divided by 8192 */

float costab[] = {
/* f=0,... */
      8192., 8191., 8191., 8191., 8190., 8189., 8188., 8186., 8185., 8183.,
/* f=80,... */
      8181., 8179., 8177., 8174., 8171., 8168., 8165., 8162., 8158., 8154.,
/* f=160,... */
      8150., 8146., 8141., 8137., 8132., 8127., 8122., 8116., 8110., 8105.,
/* f=240,... */
      8099., 8092., 8086., 8079., 8072., 8065., 8058., 8050., 8043., 8035.,
/* f=320,... */
      8026., 8018., 8010., 8001., 7992., 7983., 7973., 7964., 7954., 7944.,
/* f=400,... */
      7934., 7924., 7913., 7903., 7892., 7880., 7869., 7858., 7846., 7834.,
/* f=480,... */
      7822., 7809., 7797., 7784., 7771., 7758., 7745., 7731., 7718., 7704.,
/* f=560,... */
      7690., 7675., 7661., 7646., 7631., 7616., 7601., 7586., 7570., 7554.,
/* f=640,... */
      7538., 7522., 7505., 7489., 7472., 7455., 7438., 7421., 7403., 7385.,
/* f=720,... */
      7367., 7349., 7331., 7313., 7294., 7275., 7256., 7237., 7218., 7198.,
/* f=800,... */
      7178., 7158., 7138., 7118., 7097., 7077., 7056., 7035., 7014., 6992.,
/* f=880,... */
      6971., 6949., 6927., 6905., 6883., 6861., 6838., 6815., 6792., 6769.,
/* f=960,... */
      6746., 6722., 6699., 6675., 6651., 6627., 6603., 6578., 6554., 6529.,
/* f=1040,... */
      6504., 6479., 6453., 6428., 6402., 6377., 6351., 6325., 6298., 6272.,
/* f=1120,... */
      6245., 6219., 6192., 6165., 6138., 6110., 6083., 6055., 6027., 5999.,
/* f=1200,... */
      5971., 5943., 5915., 5886., 5857., 5828., 5799., 5770., 5741., 5712.,
/* f=1280,... */
      5682., 5652., 5622., 5592., 5562., 5532., 5501., 5471., 5440., 5409.,
/* f=1360,... */
      5378., 5347., 5316., 5284., 5253., 5221., 5189., 5158., 5126., 5093.,
/* f=1440,... */
      5061., 5029., 4996., 4963., 4930., 4898., 4864., 4831., 4798., 4765.,
/* f=1520,... */
      4731., 4697., 4664., 4630., 4596., 4561., 4527., 4493., 4458., 4424.,
/* f=1600,... */
      4389., 4354., 4319., 4284., 4249., 4214., 4178., 4143., 4107., 4072.,
/* f=1680,... */
      4036., 4000., 3964., 3928., 3892., 3856., 3819., 3783., 3746., 3709.,
/* f=1760,... */
      3673., 3636., 3599., 3562., 3525., 3487., 3450., 3413., 3375., 3338.,
/* f=1840,... */
      3300., 3262., 3225., 3187., 3149., 3111., 3073., 3034., 2996., 2958.,
/* f=1920,... */
      2919., 2881., 2842., 2803., 2765., 2726., 2687., 2648., 2609., 2570.,
/* f=2000,... */
      2531., 2492., 2453., 2413., 2374., 2334., 2295., 2255., 2216., 2176.,
/* f=2080,... */
      2136., 2097., 2057., 2017., 1977., 1937., 1897., 1857., 1817., 1776.,
/* f=2160,... */
      1736., 1696., 1656., 1615., 1575., 1535., 1494., 1454., 1413., 1372.,
/* f=2240,... */
      1332., 1291., 1251., 1210., 1169., 1128., 1087., 1047., 1006.,  965.,
/* f=2320,... */
       924.,  883.,  842.,  801.,  760.,  719.,  678.,  637.,  596.,  555.,
/* f=2400,... */
       514.,  473.,  432.,  391.,  349.,  308.,  267.,  226.,  185.,  144.,
/* f=2480,... */
       102.,   61.,   20.,  -20.
};




/*	Log table for F/8 in Hz */
/*	 logtab = 100 * log10(F) where F=0,8,16,...,5000 */

short logtab[] = {
/* f=0,... */
         0,    90,   120,   138,   150,   160,   168,   174,   180,   185,
/* f=80,... */
       190,   194,   198,   201,   204,   207,   210,   213,   215,   218,
/* f=160,... */
       220,   222,   224,   226,   228,   230,   231,   233,   235,   236,
/* f=240,... */
       238,   239,   240,   242,   243,   244,   245,   247,   248,   249,
/* f=320,... */
       250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
/* f=400,... */
       260,   261,   261,   262,   263,   264,   265,   265,   266,   267,
/* f=480,... */
       268,   268,   269,   270,   270,   271,   272,   272,   273,   274,
/* f=560,... */
       274,   275,   276,   276,   277,   277,   278,   278,   279,   280,
/* f=640,... */
       280,   281,   281,   282,   282,   283,   283,   284,   284,   285,
/* f=720,... */
       285,   286,   286,   287,   287,   288,   288,   288,   289,   289,
/* f=800,... */
       290,   290,   291,   291,   292,   292,   292,   293,   293,   294,
/* f=880,... */
       294,   294,   295,   295,   295,   296,   296,   297,   297,   297,
/* f=960,... */
       298,   298,   298,   299,   299,   300,   300,   300,   301,   301,
/* f=1040,... */
       301,   302,   302,   302,   303,   303,   303,   303,   304,   304,
/* f=1120,... */
       304,   305,   305,   305,   306,   306,   306,   307,   307,   307,
/* f=1200,... */
       307,   308,   308,   308,   309,   309,   309,   309,   310,   310,
/* f=1280,... */
       310,   310,   311,   311,   311,   312,   312,   312,   312,   313,
/* f=1360,... */
       313,   313,   313,   314,   314,   314,   314,   315,   315,   315,
/* f=1440,... */
       315,   316,   316,   316,   316,   317,   317,   317,   317,   317,
/* f=1520,... */
       318,   318,   318,   318,   319,   319,   319,   319,   319,   320,
/* f=1600,... */
       320,   320,   320,   321,   321,   321,   321,   321,   322,   322,
/* f=1680,... */
       322,   322,   322,   323,   323,   323,   323,   323,   324,   324,
/* f=1760,... */
       324,   324,   324,   325,   325,   325,   325,   325,   326,   326,
/* f=1840,... */
       326,   326,   326,   327,   327,   327,   327,   327,   327,   328,
/* f=1920,... */
       328,   328,   328,   328,   329,   329,   329,   329,   329,   329,
/* f=2000,... */
       330,   330,   330,   330,   330,   330,   331,   331,   331,   331,
/* f=2080,... */
       331,   331,   332,   332,   332,   332,   332,   332,   333,   333,
/* f=2160,... */
       333,   333,   333,   333,   334,   334,   334,   334,   334,   334,
/* f=2240,... */
       335,   335,   335,   335,   335,   335,   335,   336,   336,   336,
/* f=2320,... */
       336,   336,   336,   336,   337,   337,   337,   337,   337,   337,
/* f=2400,... */
       338,   338,   338,   338,   338,   338,   338,   339,   339,   339,
/* f=2480,... */
       339,   339,   339,   339,   340,   340,   340,   340,   340,   340,
/* f=2560,... */
       340,   340,   341,   341,   341,   341,   341,   341,   341,   342,
/* f=2640,... */
       342,   342,   342,   342,   342,   342,   342,   343,   343,   343,
/* f=2720,... */
       343,   343,   343,   343,   343,   344,   344,   344,   344,   344,
/* f=2800,... */
       344,   344,   344,   345,   345,   345,   345,   345,   345,   345,
/* f=2880,... */
       345,   346,   346,   346,   346,   346,   346,   346,   346,   347,
/* f=2960,... */
       347,   347,   347,   347,   347,   347,   347,   347,   348,   348,
/* f=3040,... */
       348,   348,   348,   348,   348,   348,   348,   349,   349,   349,
/* f=3120,... */
       349,   349,   349,   349,   349,   349,   350,   350,   350,   350,
/* f=3200,... */
       350,   350,   350,   350,   350,   351,   351,   351,   351,   351,
/* f=3280,... */
       351,   351,   351,   351,   352,   352,   352,   352,   352,   352,
/* f=3360,... */
       352,   352,   352,   352,   353,   353,   353,   353,   353,   353,
/* f=3440,... */
       353,   353,   353,   353,   354,   354,   354,   354,   354,   354,
/* f=3520,... */
       354,   354,   354,   354,   355,   355,   355,   355,   355,   355,
/* f=3600,... */
       355,   355,   355,   355,   356,   356,   356,   356,   356,   356,
/* f=3680,... */
       356,   356,   356,   356,   356,   357,   357,   357,   357,   357,
/* f=3760,... */
       357,   357,   357,   357,   357,   357,   358,   358,   358,   358,
/* f=3840,... */
       358,   358,   358,   358,   358,   358,   358,   359,   359,   359,
/* f=3920,... */
       359,   359,   359,   359,   359,   359,   359,   359,   360,   360,
/* f=4000,... */
       360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
/* f=4080,... */
       361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
/* f=4160,... */
       361,   361,   362,   362,   362,   362,   362,   362,   362,   362,
/* f=4240,... */
       362,   362,   362,   362,   363,   363,   363,   363,   363,   363,
/* f=4320,... */
       363,   363,   363,   363,   363,   363,   364,   364,   364,   364,
/* f=4400,... */
       364,   364,   364,   364,   364,   364,   364,   364,   364,   365,
/* f=4480,... */
       365,   365,   365,   365,   365,   365,   365,   365,   365,   365,
/* f=4560,... */
       365,   365,   366,   366,   366,   366,   366,   366,   366,   366,
/* f=4640,... */
       366,   366,   366,   366,   366,   367,   367,   367,   367,   367,
/* f=4720,... */
       367,   367,   367,   367,   367,   367,   367,   367,   367,   368,
/* f=4800,... */
       368,   368,   368,   368,   368,   368,   368,   368,   368,   368,
/* f=4880,... */
       368,   368,   368,   369,   369,   369,   369,   369,   369,   369,
/* f=4960,... */
       369,   369,   369,   369,   369,   369,   369,   370,   370,   370,
};


/* Inverse of log table */

/* loginv[] = 32768 * exp(n * -.0231) where n=0,1,2,...,199 */
/*  i.e. loginv[0] is 32767, i.e. 1.0 for frac1mul() */
/*  and. loginv[30] = 0.5 of 32768 */
/*  note that logtab[] above increases by 30 for each doubling of F */
/*  These numbers should be divided by 32768. */

float loginv[] = {
/* arg=0,... */
     32767.,32019.,31288.,30574.,29875.,29193.,28527.,27875.,27239.,26617.,
/* arg=10,... */
     26009.,25415.,24834.,24267.,23713.,23172.,22643.,22125.,21620.,21127.,
/* arg=20,... */
     20644.,20173.,19712.,19262.,18822.,18392.,17972.,17562.,17161.,16769.,
/* arg=30,... */
     16386.,16012.,15646.,15289.,14940.,14598.,14265.,13939.,13621.,13310.,
/* arg=40,... */
     13006.,12709.,12419.,12135.,11858.,11587.,11323.,11064.,10811.,10565.,
/* arg=50,... */
     10323.,10088., 9857., 9632., 9412., 9197., 8987., 8782., 8581., 8385.,
/* arg=60,... */
      8194., 8007., 7824., 7645., 7471., 7300., 7133., 6970., 6811., 6656.,
/* arg=70,... */
      6504., 6355., 6210., 6068., 5930., 5794., 5662., 5533., 5406., 5283.,
/* arg=80,... */
      5162., 5044., 4929., 4817., 4707., 4599., 4494., 4391., 4291., 4193.,
/* arg=90,... */
      4097., 4004., 3912., 3823., 3736., 3650., 3567., 3485., 3406., 3328.,
/* arg=100,... */
      3252., 3178., 3105., 3034., 2965., 2897., 2831., 2766., 2703., 2642.,
/* arg=110,... */
      2581., 2522., 2465., 2408., 2353., 2300., 2247., 2196., 2146., 2097.,
/* arg=120,... */
      2049., 2002., 1956., 1912., 1868., 1825., 1783., 1743., 1703., 1664.,
/* arg=130,... */
      1626., 1589., 1553., 1517., 1482., 1449., 1416., 1383., 1352., 1321.,
/* arg=140,... */
      1291., 1261., 1232., 1204., 1177., 1150., 1123., 1098., 1073., 1048.,
/* arg=150,... */
      1024., 1001.,  978.,  956.,  934.,  912.,  892.,  871.,  851.,  832.,
/* arg=160,... */
       813.,  794.,  776.,  758.,  741.,  724.,  708.,  691.,  676.,  660.,
/* arg=170,... */
       645.,  630.,  616.,  602.,  588.,  575.,  562.,  549.,  536.,  524.,
/* arg=180,... */
       512.,  500.,  489.,  478.,  467.,  456.,  446.,  435.,  425.,  416.,
/* arg=190,... */
       406.,  397.,  388.,  379.,  370.,  362.,  354.,  346.,  338.,  330.
};

#endif /* if chip */
/* need to look at these more carefully */
float noiseinlast, lastin; /* debug see gen_noise */
float glotlast; /* Previous value of glotout                   */
float noiselast; /* Used to first-difference random number for aspir */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                        P A R W A V E . C                            */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*   Copyright            1988                    by Dennis H. Klatt
 *
 *      Klatt synthesizer
 *         Modified version of synthesizer described in
 *         J. Acoust. Soc. Am., Mar. 1980. -- new voicing
 *         source, floating-point chip implementation.
 *
 * Edit history
 *   0001 18-Jan-88 DK  Initial creation from PARWAVE.C;285
 *   0002  5-Feb-88 DK	Invert waveform so positive means flow out of mouth
 *   0003 12-Feb-88 DK	os=xx for derivative of glottal wave, invert glottal wave
 *   0004 15-Feb-88 DK	Replace random number generator
 *   0005 22-Feb-88 DK	Add Liljencrants-Fant model of voice source (ss=3)
 *   0006 24-Feb-88 DK	Make tilt filter use critical damping f=.375 bw
 *   0007 10-Sep-88 DK	Make compatable with KLSYN88 documentation & default600
 *			 TO BE DONE: Change all tables to floating point, get
 *	         	  rid of divides for floating-point chip real time impl.
 *           Feb-93 NM  Modified to allow impulse
 *           May-94 DH  Removed graphics            
 */


extern int spkrdef[],pars[];      
extern char symb[][4];	  
extern int sigmx;                
extern float DBtoLIN();			
extern float impulsive_source();	
extern float natural_source();		
extern float fant_source();
	
/*extern float fantgain[];*/

static int disptcum;			/* Cum # samples, used for debugging */

/* CONVERT NEXT FRAME OF PARAMETER DATA TO A WAVEFORM CHUNK
 * Synthesize nspfr samples of waveform and store in jwave[]. */

void parwav(jwave) short *jwave; {

/* Initialize synthesizer and get specification for current speech
   frame from host microcomputer */

    gethost();		/* 1. (see subroutine 1 listed later in this file) */


/* MAIN LOOP, for each output sample of current frame: */

    for (ns=0; ns<nspfr; ns++) {

/*    Get random number for aspiration and frication noise */

        gen_noise();		/* 2. Output variable is 'noise' */

/*    Amplitude modulate noise (reduce noise amplitude during semi-closed
 *    portion of glottal period) if voicing simultaneously present.
 *    nmod=T0 if voiceless, nmod=nopen if voiced  */
        if (nper > nmod) {
            noise *= 0.5;
        }

/*    Compute frication noise sample */
/* 93 add impulse to fricatoin       */

       frics = amp_frica * noise + amp_imp*500000*(ns==0);

/*  Compute voicing waveform */
/*    (run glottal source simulation at 4 times normal sample rate to minimize
 *    quantization noise in period of female voice) */

	for (n4=0; n4<4; n4++) {

/*	  Use impulsive glottal source */
	    if (glsource == IMPULSIVE) {
		voice = impulsive_source();	/* 3. output is 'voice' */
	    }

/*	  Or use a more-natural-shaped source waveform with excitation
	  occurring both upon opening and upon closure, strongest at closure */
	    else if (glsource == NATURAL) {
		voice = natural_source();	/* 4. output is 'voice' */
	    }

/*	  Or use modified Liljencrants-Fant source waveform */
	    else {
		voice = fant_source();		/* 4. output is 'voice' */
	    }

/*	  Modify F1 and BW1 pitch synchrounously */
	    if (nper == nopen) {	/* CASE I: Glottis closes */
		if ((dF1hz+dB1hz) > 0) {
		    setR1(F1hz,B1hz);		/* 5. */
		}
	    }
	    if (nper == T0) {	/* CASE II: Glottis opens */
		if ((dF1hz+dB1hz) > 0) {
		    setR1(F1hz+dF1hz,B1hz+dB1hz);
		}

/*	      Reset period when counter 'nper' reaches T0 */
		nper = 0;
/*	      Reset certain pars pitch synchronously */
		pitch_synch_par_reset();	/* 6. */
	    }

/*	  Low-pass filter voicing waveform before downsampling from 4*samrate
 *	  to samrate samples/sec.  Resonator f=.095*samrate, bw=.063*samrate */

	    if ((outsl == 0) || (outsl > 4))
		resonlp();			/* 8. in=voice, out=voice */

/*	  Increment counter that keeps track of 4*samrate samples/sec */
            nper++;
	}

/*    Set amplitude of voicing waveform sample */

        glotout = amp_voice * voice;

/*    Tilt spectrum of voicing source down by soft low-pass filtering, amount
 *    of tilt determined by TL which determines additional dB down at 3 kHz */

	resontilt();			/* 8. in=glotout, out=tiltout */

/*    Compute aspiration sample and add to voicing source */

	glottalnoise = noise - noiselast;	/* Use first diff, atten. lows */
	noiselast = noise;
        aspiration = amp_aspir * glottalnoise;
	glotout = tiltout + aspiration;


/* 93 */
	lstinput = glotout;
	spacefilt();              /* filter wrt formant spacing */
	previnput = lstinput;
	lastout = glotout;



/*  CASCADE VOCAL TRACT: excited by laryngeal sources.  Tracheal zero and pole,
 *    nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1 */

	if (cascparsw == CASCADE) {

/*	  Tracheal resonance-antiresonance system modeled by one pole pair */
/*	  and one zero pair, default T(f) = 1.0 */

	    trachout = glotout;
	    resontz();			/* 9. in=glotout, out=trachout */
	    resontp();			/* 9. in=trachout, out=trachout   */

/*	  Nasal resonance-antiresonance system also modelied by one pole pair */
/*	  and one zero pair, default T(f) = 1.0 */

	    resonnz();			/* 9. in=trachout, out=rnzout   */
	    resonnp();			/* 9. in=rnzout, out=rnpc_1 */
	    casc_next_in =  rnpc_1;

	    if (nfcascade >= 8) {
		resonc8();		/* 9. Do not use unless samrat=16000 */
		casc_next_in = r8c_1;
	    }

	    if (nfcascade >= 7) {
		resonc7();		/* 9. Do not use unless samrat=16000 */
		casc_next_in = r7c_1;
	    }

	    if (nfcascade >= 6) {
		resonc6();		/* 9. Do not use unless long vocal */
		casc_next_in = r6c_1;	/*     tract or samrat increased */
	    }

	    if (nfcascade >= 5) {
		resonc5();		/* 9. Normal choice for male voice */
		casc_next_in = r5c_1;
	    }

	    if (nfcascade >= 4) {
		resonc4();		/* 9. Normal choice for female voice */
		casc_next_in = r4c_1;
	    }

	    if (nfcascade >= 3) {
		resonc3();		/* 9. Normal choice for small child */
		casc_next_in = r3c_1;
	    }

	    if (nfcascade >= 2) {
		resonc2();		/* 9. */
		casc_next_in = r2c_1;
	    }

	    resonc1();			/* 9. */
	    out = r1c_1;
	}

/*    Debugging options, select which signal to send to D/A, normal=0 */
	if ((outsl > 0) && (outsl < 13)) {
	    if (outsl ==  1) {
		out =  tiltout * -0.5;	/* was voice * -0.02 */
	    }
	    if (outsl ==  2)	out = aspiration;
	    if (outsl ==  3)	out = frics;
	    if (outsl ==  4)	out = glotout * 3.6;
	    if (outsl ==  5)	out = trachout;
	    if (outsl ==  6)	out = rnzout;
	    if (outsl ==  7)	out = rnpc_1;
	    if (outsl ==  8)	out = r5c_1;
	    if (outsl ==  9)	out = r4c_1;
	    if (outsl == 10)	out = r3c_1;
	    if (outsl == 11)	out = r2c_1;
	    if (outsl == 12)	out = r1c_1;
	    if (outsl <= 3) {
	        no_rad_char(out);	/* 11. Cancel radiation char */
	    }
	    goto skip;
	}


/*    PARALLEL VOCAL TRACT: Step 1, excite R1 and RNP by voicing waveform */

	if (cascparsw == PARALLEL) {

	    out = 0;
	    casc_next_in = glotout;	/* Source is voicing plus aspiration */
	    resonc1();			/* 9. in=casc_next_in, out=r1c_1 */
	    out += r1c_1;

	    rnzout = glotout;		/* Source is voicing plus aspiration */
	    resonnp();			/* 9. in=rnzout, out=rnpc_1 */
	    out += rnpc_1;		/* Add in phase with R1 to boost lows */

/*	  Sound source for other vowel resonators is 1st diff of voicing */

	    casc_next_in = glotout - glotlast;
	    glotlast = glotout;

	    resonc4();			/* 9. in=casc_next_in, out=r4c_1 */
            out -= r4c_1;	/* Alternating phases to approx cascade T(f) */

	    resonc3();			/* 9. in=casc_next_in, out=r3c_1 */
            out += r3c_1;

	    resonc2();			/* 9. in=casc_next_in, out=r2c_1 */
            out -= r2c_1;

	    trachout = casc_next_in;	/* Tracheal pole pair */
	    resontp();			/* 9. in=trachout, out=trachout */
	    out += trachout;
	}

/*  Standard parallel vocal tract for fricatives
 *  Formants F6,F5,F4,F3,F2, bypass path, outputs added with alternating sign */

/*    Sound sourc for wide-bandwidth parallel resonators is frication */
        sourc = frics;

	reson6p();		/* 10. in=sourc, out=r6p_1 */
        out -= r6p_1;

	reson5p();		/* 10. in=sourc, out=r5p_1 */
        out += r5p_1;

	reson4p();		/* 10. in=sourc, out=r4p_1 */
        out -= r4p_1;

	reson3p();		/* 10. in=sourc, out=r3p_1 */
        out += r3p_1;

	reson2p();		/* 10. in=sourc, out=r2p_1 */
        out -= r2p_1;		/* Out of phase with R1 */

        outbypas = amp_bypas * sourc;
        out += outbypas;	/* Out of phase with R6 */

	if (outsl > 12) {
		if (outsl == 13)	out = r6p_1;
		if (outsl == 14)	out = r5p_1;
		if (outsl == 15)	out = r4p_1;
		if (outsl == 16)	out = r3p_1;
		if (outsl == 17)	out = r2p_1;
		if (outsl == 18)	out = r1c_1;
		if (outsl == 19)	out = rnpc_1;
		if (outsl == 20)	out = outbypas;
	}

skip:	temp = out;			/* Convert back to integer */
        getmax(temp,&sigmx);		/* 12. See if overload */
	*jwave++ = truncate_dk(temp);	/* 13. Truncate if exceeds 16 bits */
    }
}

/* ******************************************************************** */
/*                                                                      */
/*           formant spacing filter (new 2/93)                          */
/* ******************************************************************** */

void spacefilt(){


/* pars[17]= F4  pars[49]=FSF   xxx */

     if ((pars[17]/3.5-1000)>=20) {   /* we should add a POLE */
           spacefix= 5760-110*(pars[17]/3.5-1000)+0.64*(pars[17]/3.5-1000)*(pars[17]/3.5-1000);/*y=a+bx+cx^2 best curve to equal spacings 1000,1020...1100*/
       if (spacefix <0) spacefix=100;
           glotout = pars[49]*(exp(-6.28*spacefix/10000)*lastout +(1-exp(-6.28*spacefix/10000))*previnput) + (pars[49]!=1)*glotout;
            }

        else if (1000-(pars[17]/3.5)>=20) {  /* add a ZERO */
	   spacefix =2.77-3.8e-2*(1000-pars[17]/3.5)+1.1e-4*(1000-pars[17]/3.5)*(1000-pars[17]/3.5);/*y=a+bx+cx^2 best curve to equal spacings 900,920..1000*/ 
             if (spacefix <0.1) spacefix=0.1;
           glotout=pars[49]*(1/(1+.2*exp(-spacefix)))*(-(.2+exp(-spacefix))*lastout + (.2+1+.2*exp(-spacefix)+exp(-spacefix))/(1+.2)*(previnput+.2*glotout))+(pars[49]!=1)*glotout;
            }

         } 

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         1.        S U B R O U T I N E   G E T H O S T               */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Get variable parameters from host computer,
 *   initially also get constant speaker-defining pars */

void gethost() {

/*  Initialize speaker definition */

        if (initsw == 0) {
	    initsw = 1;

/*        Read Speaker Definition from Host */
            nspfr     = spkrdef[ 1];    /* # of samples per frame of pars  */
	    samrate   = spkrdef[ 2];	/* Sampling rate		   */
	    nfcascade = spkrdef[ 3];	/* Number formants in cascade tract*/
	    glsource  = spkrdef[ 4];	/* 1->impulsive, 2->natural voicing*/
            ranseed   = spkrdef[ 5];    /* Seed for ran num gen		   */
	    sameburst = spkrdef[ 6];	/* Use same burst noise spectrum   */
	    cascparsw = spkrdef[ 7];	/* cascade or parallel vowel synth */
	    outsl     = spkrdef[ 8];	/* Select which waveform to output */
	    GainAV    = spkrdef[ 9];	/* Gain scale factor for AV	   */
	    GainAH    = spkrdef[10];	/* Gain scale factor for AH	   */
	    GainAF    = spkrdef[11];	/* Gain scale factor for AF	   */
/*93*/
	    GainAI    = spkrdef[12];    /* Gain scale factor for AI        */

/*	  Global gain scale factors, balance voicing, aspiration & frication */
	    amp_gainAV = DBtoLIN(GainAV) * 0.012;
	    amp_gainAH = DBtoLIN(GainAH);
	    amp_gainAF = DBtoLIN(GainAF);
   /*93*/   amp_gainAI = DBtoLIN(GainAI);
/*	  Variables for setabc() calculations */
	    minus_pi_t = -3.14159 / samrate;
	    two_pi_t = -2. * minus_pi_t;
/*	  Fixed downsampling filter */
	    FLPhz = (950 * samrate) / 10000;
	    BLPhz = (630 * samrate) / 10000;
	    setabc(FLPhz,BLPhz,&rlpa,&rlpb,&rlpc);		/* 7a. */
/*	  Variables for flutter calculation */
	    radiansperframe = (2.0 * 3.14159 * nspfr) / samrate;
	    darg3Hz = 12.7 * radiansperframe;	/* WAS 2.9 */
	    darg5Hz = 4.7 * radiansperframe;
	    darg7Hz = 7.1 * radiansperframe;
/*	  Initialize random # generator */
	    nrand = ranseed;	/* was srand(ranseed); */

        }

/*    Read  speech frame definition into temp store
 *    and move some parameters into active use immediately
 *    (voice-excited ones are updated pitch synchronously
 *    to avoid waveform glitches). */

        F0hz10 = pars[ 0];
        AVdb   = pars[ 1];
        OQ     = pars[ 2];	/* Open quotient in percent of T0 */
        SQ     = pars[ 3];	/* Speed quotient in percent, Fant source only*/
	TL     = pars[ 4];
	FL     = pars[ 5];	/* Slow random variation in f0, tenths of Hz */
        DP     = pars[ 6];	/* Double-pulsing of alternating periods, in % */
        AH     = pars[ 7];
        AF     = pars[ 8];
        F1hz   = pars[ 9];
        B1hz   = pars[10];
        dF1hz  = pars[11];	/* F1 increment during open phase of cycle */
        dB1hz  = pars[12];	/* B1 increment during open phase of cycle */
        F2hz   = pars[13];
        B2hz   = pars[14];
        F3hz   = pars[15];
        B3hz   = pars[16];
        F4hz   = pars[17];
        B4hz   = pars[18];
        F5hz   = pars[19];
        B5hz   = pars[20];
        F6hz   = pars[21];
        B6hz   = pars[22];
        FNPhz  = pars[23];
        BNPhz  = pars[24];
        FNZhz  = pars[25];
        BNZhz  = pars[26];
	FTPhz  = pars[27];	/* Freq of tracheal pole pair */
	BTPhz  = pars[28];	/* Bw of tracheal pole pair */
	FTZhz  = pars[29];	/* Freq of tracheal zero pair */
	BTZhz  = pars[30];	/* Bw of tracheal zero pair */

        A2     = pars[31];
        A3     = pars[32];
        A4     = pars[33];
        A5     = pars[34];
        A6     = pars[35];
        AB     = pars[36];

        B2phz  = pars[37];
        B3phz  = pars[38];
        B4phz  = pars[39];
        B5phz  = pars[40];
        B6phz  = pars[41];
        AN     = pars[42];
        A1V    = pars[43];
	A2V    = pars[44];
	A3V    = pars[45];
	A4V    = pars[46];
        AT     = pars[47];
/*93*/
	AI     = pars[48];
	FSF    = pars[49];

	F0next = pars[50];	/* Value of F0hz10 in next frame, for interp. */      

/*    Convert from dB to linear scale factor */
/*     (amp_voice is done pitch-synchronously) */

        amp_aspir = DBtoLIN(AH) * 0.025 * amp_gainAH;	/* 7c. */
/*93*/
        amp_imp   = DBtoLIN(AI) * 0.25 * amp_gainAI; /* why 0.25?? */

        amp_frica = DBtoLIN(AF) * 0.25 * amp_gainAF;
	amp_parF2 = DBtoLIN(A2) * 0.17;
	amp_parF3 = DBtoLIN(A3) * 0.075;
	amp_parF4 = DBtoLIN(A4) * 0.04;
	amp_parF5 = DBtoLIN(A5) * 0.025;
	amp_parF6 = DBtoLIN(A6) * 0.022;
	amp_bypas = DBtoLIN(AB) * 0.112;
	amp_parvF1 = DBtoLIN(A1V) * 0.900;   /* Scale factors so that 60 dB is */
	amp_parvF2 = DBtoLIN(A2V) * 0.340;   /* comparable to level if cascade */
	amp_parvF3 = DBtoLIN(A3V) * 0.135;   /* vowel synth with F1=500,F2=1500 */
	amp_parvF4 = DBtoLIN(A4V) * 0.090;
	amp_parNP = DBtoLIN(AN) * 0.900;
	amp_parTP = DBtoLIN(AT) * 0.200;

	flutter = (FL * F0hz10) / 2500;		/* Convert to floating point */
	arg3Hz += darg3Hz;	/* Cosine arg for pseudo-random flutter */
	if (arg3Hz >= 6.28318)    arg3Hz -= 6.28318;
	arg5Hz += darg5Hz;	/* Cosine arg for pseudo-random flutter */
	if (arg5Hz >= 6.28318)    arg5Hz -= 6.28318;
	arg7Hz += darg7Hz;	/* Cosine arg for pseudo-random flutter */
	if (arg7Hz >= 6.28318)    arg7Hz -= 6.28318;

/*    Reset noise generator to same seed value before burst */
/*    i.e. when fric and asp noise sources off because of silence closure */
/* 93 added AI*/
	if ((sameburst == 1) && ((AF + AH + AI) == 0)) {
	    nrand = ranseed;	/* was srand(ranseed); */
	}

/*    Set coefficients of variable cascade resonators */

	if (nfcascade >= 8)    setabc(7500,600,&r8ca,&r8cb,&r8cc);	/* 7b. */
	if (nfcascade >= 7)    setabc(6500,500,&r7ca,&r7cb,&r7cc);
	if (nfcascade >= 6)    setabc(F6hz,B6hz,&r6ca,&r6cb,&r6cc);
        setabc(F5hz,B5hz,&r5ca,&r5cb,&r5cc);
        setabc(F4hz,B4hz,&r4ca,&r4cb,&r4cc);
        setabc(F3hz,B3hz,&r3ca,&r3cb,&r3cc);
        setabc(F2hz,B2hz,&r2ca,&r2cb,&r2cc);

/*    Adjust memory variables to compensate for sudden change to F3 */

	if ((F3last != 0) && (F3hz < F3last)) {
	    anorm3 = F3hz / anorm3;	/* Use logtab[] and loginv[] */
	    r3c_1 = r3c_1 * anorm3;
	    r3c_2 = r3c_2 * anorm3;
	}
	F3last = F3hz;
	anorm3 = F3hz;		/* Save to use next time in denom of divide */

/*    Adjust memory variables to compensate for sudden change to F2 */

	if ((F2last != 0) && (F2hz < F2last)) {
	    anorm2 = F2hz / anorm2;	/* Use logtab[] and loginv[] */
	    r2c_1 = r2c_1 * anorm2;
	    r2c_2 = r2c_2 * anorm2;
	}
	F2last = F2hz;
	anorm2 = F2hz;		/* Save to use next time in denom of divide */

/*    R1 is set pitch synchronously if user specifies non-zero dF1hz or dB1hz */

	if ((dF1hz+dB1hz) == 0) {
	    setR1(F1hz,B1hz);				/* 5. */
	}

/*    Set coeficients of nasal resonator and zero antiresonator */

	setabc(FNPhz,BNPhz,&rnpca,&rnpcb,&rnpcc);	/* 7a. */
	setzeroabc(FNZhz,BNZhz,&rnza,&rnzb,&rnzc);	/* 7b. */

/*    Set coeficients of tracheal resonator and antiresonator */

	setabc(FTPhz,BTPhz,&rptr1a,&rptr1b,&rptr1c);
	setzeroabc(FTZhz,BTZhz,&rztr1a,&rztr1b,&rztr1c);

/*    Set coefficients of parallel resonators, and amplitude of outputs */

        setabc(F2hz,B2phz,&r2pa,&r2pb,&r2pc);
	r2pa *= amp_parF2;
        setabc(F3hz,B3phz,&r3pa,&r3pb,&r3pc);
	r3pa *= amp_parF3;
        setabc(F4hz,B4phz,&r4pa,&r4pb,&r4pc);
	r4pa *= amp_parF4;
        setabc(F5hz,B5phz,&r5pa,&r5pb,&r5pc);
	r5pa *= amp_parF5;
	setabc(F6hz,B6phz,&r6pa,&r6pb,&r6pc);
	r6pa *= amp_parF6;

/*    Also modify certain cascade resonator gains if vowel synth by parallel */

	if (cascparsw == PARALLEL) {
	    rnpca  *= amp_parNP;
	    rptr1a *= amp_parTP;
	    r2ca   *= amp_parvF2;	/* r1ca is adjusted in setR1() */
	    r3ca   *= amp_parvF3;
	    r4ca   *= amp_parvF4;
	    r5ca    = 0.;	    /* Not excited by voicing if parallel */
	    r6ca    = 0.;
	    r7ca    = 0.;
	    r8ca    = 0.;
	}

/*    Initialize pitch-sychrounous variables */
	if (initsw == 1) {
	    initsw = 2;
	    pitch_synch_par_reset();			/* 6. */
	}

        disptcum += nspfr;	/* Cum time in samples for debugging only */
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*        2.          S U B R O U T I N E   G E N - N O I S E          */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Random number generator (return a number between -8191 and +8191) */
/*     16-bit version of 32-bit algorithm for Vax Unix (ranmul=1103515245) */
/*     random repeats every 65535 samples, noise repeats every 16384 samples */
/*     Runs of positive and negative numbers follow expected statistics, */
/*     so spectrum should be flat */
/*     For more details, see Knuth "Semi-Numerical Algorithms" */

void gen_noise() {

/*	static float noiseinlast;  */
        extern float noiseinlast;
	float frand;


	nrand = (nrand * 20077) + 12345;  /* was  nrand = (rand()>>17) -8191; */
	frand = nrand >> 2;		  /* was  noise = nrand; */

/*    Tilt down noise spectrum at high freqs by soft high-pass		  */
/*    filter having a zero at 5 kHz maxfre, and a gain of 4.0, i.e.	  */
/*    output = 4. * (0.25 * input)  +  (0.75 * lastinput)		  */

	noise = frand + (0.75 * noiseinlast);
	noiseinlast = frand;

/* TEMPORARY */
	noise = frand;
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         3.            I M P U L S I V E - S O U R C E	       */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

float doublet[] = { 0., 490000000., -490000000. };

float impulsive_source() {

	if ((nper < 3) && (F0hz10 > 0)) {
	    vwave = doublet[nper];		/* nper=1 upon first entry */
	}
	else {
	    vwave = 0.;
	}

/*    Low-pass filter the differenciated impulse with a critically-damped
      second-order filter, time constant proportional to nopen */

	resonglot();				/* 9. */
	return(rgl_1);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*          4.              N A T U R A L - S O U R C E		       */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*  Vwave is the differentiated glottal flow waveform, there is a weak
    spectral zero around 800 Hz, magic constants a,b reset pitch-synch */

float natural_source() {

/*    See if glottis open */

        if ((nper < nopen) && (F0hz10 > 0)) {
	    a -= b;
            vwave += a;
	    return(vwave);
        }

/*    Glottis closed */

        else {
            vwave = 0.;
	    return(0.);
	}
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*        4a.      S U B R O U T I N E   F A N T - S O U R C E         */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

float fant_source() {

/*    Input is an impulse */

	if ((nper == 1) && (F0hz10 > 0)) {
	    vwave = Afant;
	}
	else {
	    vwave = 0.;
	}

/*    Filter the impulse with an under-damped
      second-order filter, frequency proportional to SQ */

/*    See if glottis open */

        if (nper < nopen) {
	    resonglot();				/* 9. */
	    return(rgl_1);
        }

/*    Glottis closed */

        else {
            vwave = 0.;
	    return(0.);
	}
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         5.           S U B R O U T I N E   S E T R 1                */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Impliment pitch-synch change to F1,B1 so both rise when glottis open */

void setR1(Fx,Bx)
 int Fx;	/* Desired F1 value */
 int Bx;	/* Desired B1 value */
{

	    setabc(Fx,Bx,&r1ca,&r1cb,&r1cc);			/* 7a. */

/*	  Adjust memory variables to have proper levels for a given sudden
          change to F1hz.
          Approximate r1c_n' = r1c_n * sqrt(r1ca/r1calast)
	  by r1c_n' = r1c_n * (F1hz/F1hzlast) */

	    if ((F1last != 0) && (Fx < F1last)) {
		anorm1 = Fx / anorm1;	/* Use logtab[] and loginv[]: */
					/* logfx = logtab[Fx>>3];  do always */
					/* anorm1 = loginv[logfx-logfxlast]; */
					/* logfxlast = logfx;	   do always */
/*	      For reasons that I don't understand, amplitude compensation
              only needed when a formant goes down in frequency */
		r1c_1 = r1c_1 * anorm1;
		r1c_2 = r1c_2 * anorm1;
	    }
	    F1last = Fx;	/* For print only */
	    anorm1 = Fx;	/* Save to use next time in denom of divide */

/*	  Impose A1 amplitude if using parallel config for vowel synthesis */

	    if (cascparsw == PARALLEL) {
		r1ca *= amp_parvF1;
	    }

}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         6.         P I T C H _ S Y N C _ P A R _ R E S E T          */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Reset selected parameters pitch-synchronously */

void pitch_synch_par_reset() {

	int inc;
	float incfl;

	if (F0hz10 > 0) {

/*	  Interpolate f0 with previous value */
/*	  Tnis calculation is unnecessary in a chip implementation */

	    if (F0next > 0) {
		F0interp = (((nspfr - ns) * F0hz10) + (ns * F0next)) / nspfr;
	    }
	    else F0interp = F0hz10;

/*	  Add in controlled amount of flutter */

	    F0interp += (flutter * (cos(arg3Hz) + cos(arg5Hz) + cos(arg7Hz)));
	    T0 = (40 * samrate) / F0interp;	 /* Period in samp*4 */

/*	  Duration of period before amplitude modulation */

            nmod = T0;
            if (AVdb > 0) {
        	nmod = nopen;
            }

/*	  Amplitude of voicing */

	    amp_voice = DBtoLIN(AVdb) * amp_gainAV;

/*	  Set open phase of glottal period, where  40 <= open phase < T0 */

	    nopen = (T0 * OQ) / 100;
	    if (nopen >= (T0-1)) {
		nopen = T0 - 2;
	    }
            if (nopen < 40) {
		 nopen = 40;	/* F0 max = 1000 Hz */
		printf(
		"Warning: minimum glottal open period is 1.0 ms, truncated\n");
	    }

/*	  Reset a & b, which determine shape of "natural" glottal waveform */

            if (glsource == NATURAL) {
		if (nopen > 799) {	/* B0[] table only goes up to 20 ms */
		    nopen = 799;
		}
		b = B0[nopen-40];
		a = (b * nopen) * .333;
	    }

/*	  Reset width of "impulsive" glottal pulse, resonator bandwidth is */
/*	  inversely proportional to nopen */

	    else if (glsource == IMPULSIVE) {
		temp = samrate / nopen;
	        setabc(0,temp,&rgla,&rglb,&rglc);		/* 7a. */
/*	      Make gain at F1 about constant */
		temp1 = nopen *.00833;
		rgla *= temp1 * temp1;
	    }

/*	  Reset LF model: resonator freq and bw, zero memory variables */

	    else {
/*	      Set freq equal to one over rise time of Ug(t) */
		Ffant = (10 * samrate * (SQ + 100)) / (2 * SQ * nopen);
		temp = SQ / 10;
/*	      Set gain of Fant impulse to make Ug(t) peak invariant with SQ */
/*	      and so that u'g(t) neg peak constant with change to OQ */
		Afant = fantgain[temp-10];
		incfl = fantgain[temp-9] - Afant;
		Afant += ((float)(SQ - (10 * temp)) * incfl * 0.1);
		Afant *= 220000.;
/*	      And so that U'g(t) neg peak is constant with change to OQ */
		Afant = (Afant * (float) nopen) * .005;	      /* was / 200. */
/*	      Resonator bandwidth is a function of SQ, then scaled by nopen */
		BWfant = bwfanttab[temp-10];
		inc = bwfanttab[temp-9] - BWfant;
		temp = SQ - (10 * temp);
		BWfant += (((inc * temp) + 5) / 10);
/*	      Scale bandwidth up and down with freq */
		BWfant = (BWfant * 200) / nopen;
	        setabc(Ffant,BWfant,&rgla,&rglb,&rglc);		/* 7a. */
		rgl_1 = 0.;
		rgl_2 = 0.;
	    }

/*	  Double-pulsing in % of duration of closed phase of glottal period */

	    if (DP == 0)    dpulse = 0;

	    else {
		if (dpulse > 0) {	/* Open phase is at end of period */
		    dpulse = -dpulse;	/* Delay every other pulse */
		}
		else {
		    temp = T0 - nopen - 16;  /* Dur of closed phase in samples*4 */
		    if (temp < 1)    temp = 1;
		    dpulse = (temp * DP) / 100;	/* Delay every other pulse */
/*		  Reduce amplitude and tilt spectrum of this delayed pulse */
		    amp_voice = (amp_voice * (temp - dpulse)) / temp;
		    TL += ((25 * dpulse) / temp);
		}

/*	      Add double-pulsing delay to voicing period */
		T0 = T0 - dpulse;
	    }

/*	  Set one-pole low-pass filter that tilts glottal source */

	    if (TL < 0)    TL = 0;
	    if (TL > 41)   TL = 41;
            BWtilt = lineartilt[TL];		/* Array in PARWAVTABF.H */
	    Ftilt = (3 * BWtilt) >> 3;		/* Times 0.375, 1/e */
	    setabc(Ftilt,BWtilt,&rtlta,&rtltb,&rtltc);		/* 8a */
/*	  Adjust gain to reflect unity-gain pivot point at 300 Hz */
	    if (TL > 10) {
		rtlta *= (1.0 + (.001 * (TL - 10) * (TL - 10)));
	    }
	}

/*    F0 is currently zero, do not make voicing waveform */

	else {
	    T0 = 4;			/* Default for f0 undefined */
	    amp_voice = 0.;
	    nmod = T0;
	    a = 0.;
	    b = 0.;
	}
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         7a.          S U B R O U T I N E   S E T A B C              */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*      Convert formant freqencies and bandwidth into
 *      resonator difference equation constants */
/* f:                      Frequency of resonator in Hz 
   bw:                     Bandwidth of resonator in Hz 
*acoef, *bcoef, *ccoef: 	output coefficients              */
void setabc(int f,int bw,float *acoef,float *bcoef,float *ccoef)
{

	float r;
	double arg,exp(),cos();


/*    Let r  =  exp(-pi bw t) */

	arg = minus_pi_t * bw;
	if (bw < 0)    arg *= 0.1;	/* Need accuracy to tenth of Hz */
	r = exp(arg);			/*  for Fant glottal model. */

/*    To get rid of transcendentals for chip implementation, use code: */
/*	r = 1.0 - (.000314 * bw);		*/ /* actually 3.14/samrate */
/*	if (r < 0.)    r = 0.;			*/
/*    Validity of approximation:		*/
/*     Requested bw   Resultant bw  % error	*/
/*	  50		  50		 0	*/
/*	 100		 100		 0	*/
/*	 200		 206		 3	*/
/*	 400		 423		 5	*/
/*	 800		 894		11	*/
/*	1600		2164		35	*/
/*	2500		4711		88	*/
/*	4100		infinite		*/


/*    Let c  =  -r**2 */

	*ccoef = -(r * r);

/*    Let b = r * 2*cos(2 pi f t) */

	arg = two_pi_t * f;
	if (bw <= 0)    arg *= 0.1;	/* Need accuracy to tenth of Hz */
	*bcoef = r * cos(arg) * 2.;	/*  for Fant glottal model. */

/*    To get rid of transcendentals for chip implementation, use code:	*/
/*	index = arg * 198.97;		*/ 	/*    10000 / (16 * pi)	*/
/*	if (index < 2500/8) {						*/
/*	    *bcoef = r * twocostab[index];  */	/*    index = f/8	*/
/*	}								*/
/*	else {								*/
/*	    *bcoef = - r * twocostab[(5000/8) - index];			*/
/*	}								*/
/*    Validity of approximation:					*/
/*     Accurate to 8 Hz, i.e. excellent approx except when f is low.	*/
/*     At f=200, max error is 4%, which is about the JND */

/*    Let a = 1.0 - b - c */

        *acoef = 1.0 - *bcoef - *ccoef;

/*    Debugging printout *
      printf("f=%4d bw=%3d acoef=%8.5f bcoef=%8.5f ccoef=%8.5f\n",
          f, bw, *acoef, *bcoef, *ccoef);
*/
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*        7b.        S U B R O U T I N E   S E T Z E R O A B C         */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*      Convert formant freqencies and bandwidth into
 *      anti-resonator difference equation constants */
/* f:                      Frequency of resonator in Hz 
   bw:                     Bandwidth of resonator in Hz 
*acoef, *bcoef, *ccoef: 	output coefficients              */
void setzeroabc(int f,int bw,float *acoef,float *bcoef,float *ccoef)
{

	extern double exp(),cos();

/*    First compute ordinary resonator coefficients */
/*    Let r  =  exp(-pi bw t) */
/*    To get rid of transcendentals for chip implementation, see above: */

	arg = minus_pi_t * bw;
	r = exp(arg);

/*    Let c  =  -r**2 */

	*ccoef = -(r * r);

/*    Let b = r * 2*cos(2 pi f t) */
/*    To get rid of transcendentals for chip implementation, see above: */

	arg = two_pi_t * f;
	*bcoef = r * cos(arg) * 2.;

/*    Let a = 1.0 - b - c */

        *acoef = 1. - *bcoef - *ccoef;

/*    Now convert to antiresonator coefficients (a'=1/a, b'=-b/a, c'=-c/a) */
/*    It would be desirable to turn these divides into tables for chip impl. */

	*acoef = 1.0 / *acoef;
	*ccoef *= -*acoef;
	*bcoef *= -*acoef;

/*    Debugging printout *
      printf("fz=%3d bw=%3d acoef=%8.5f bcoef=%8.5f ccoef=%8.5f\n",
          f, bw, *acoef, *bcoef, *ccoef);
*/
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         7c.          S U B R O U T I N E   D B t o L I N            */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*      Convert from decibels to a linear scale factor
 */

float DBtoLIN(dB) int dB; {

/*    Check limits on argument (can be removed in final product) */

        if (dB < 0) {
            printf("ERROR in DBtoLIN, try to compute amptable[%d]\n", dB);
            return(0);
        }
        return(amptable[dB]);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*          8.          L O W - P A S S - R E S O N A T O R S          */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Critically-damped Low-Pass Resonator of Impulsive Glottal Source */

void resonglot() {

	register int temp3,temp4;

	temp4 = rglc * rgl_2;	       /*   (ccoef * old2)     */
        rgl_2 = rgl_1;

	temp3 = rglb * rgl_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = rgla * vwave;          /* + (acoef * input)    */
	rgl_1 = temp4 + temp3;
}


/* Low-Pass filter for tilting glottal spectrum */

void resontilt() {

	register int temp3,temp4;

	temp4 = rtltc * rtlt_2;	       /*   (ccoef * old2)     */
        rtlt_2 = rtlt_1;

	temp3 = rtltb * rtlt_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = rtlta * glotout;          /* + (acoef * input)    */
	rtlt_1 = temp4 + temp3;
	tiltout = rtlt_1;
}


/* Low-Pass Downsampling Resonator of Glottal Source */

void resonlp() {

	register int temp3,temp4;

	temp4 = rlpc * rlp_2;	       /*   (ccoef * old2)     */
        rlp_2 = rlp_1;

	temp3 = rlpb * rlp_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = rlpa * voice;          /* + (acoef * input)    */
	rlp_1 = temp4 + temp3;
	voice = rlp_1;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*          9.            C A S C A D E - R E S O N A T O R S          */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* First Antiresonator of Tracheal Tract:
 *  Output = (rnza * input) + (rnzb * oldin1) + (rnzc * oldin2) */

void resontz() {

	register int temp3,temp4;

	temp4 = rztr1c * rztr1_2;	       /*   (ccoef * oldin2)   */
        rztr1_2 = rztr1_1;

	temp3 = rztr1b * rztr1_1;          /* + (bcoef * oldin1)   */
        temp4 += temp3;

	temp3 = rztr1a * trachout;            /* + (acoef * input)    */
	rztr1_1 = trachout;
	trachout = temp4 + temp3;
}

/* First tracheal pole pair */

void resontp() {

	register int temp3,temp4;

	temp4 = rptr1c * rptr1_2;	       /*   (ccoef * old2)     */
        rptr1_2 = rptr1_1;

	temp3 = rptr1b * rptr1_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = rptr1a * trachout;          /* + (acoef * input)    */
	rptr1_1 = temp4 + temp3;
	trachout = rptr1_1;
}

/* Nasal Antiresonator of Cascade Vocal Tract:
 *  Output = (rnza * input) + (rnzb * oldin1) + (rnzc * oldin2) */

void resonnz() {

	register int temp3,temp4;

	temp4 = rnzc * rnz_2;	       /*   (ccoef * oldin2)   */
        rnz_2 = rnz_1;

	temp3 = rnzb * rnz_1;          /* + (bcoef * oldin1)   */
        temp4 += temp3;

	temp3 = rnza * trachout;            /* + (acoef * input)    */
	rnz_1 = trachout;
	rnzout = temp4 + temp3;
}

/* Nasal Resonator of Cascade Vocal Tract */

void resonnp() {

	register int temp3,temp4;

	temp4 = rnpcc * rnpc_2;	       /*   (ccoef * old2)     */
        rnpc_2 = rnpc_1;

	temp3 = rnpcb * rnpc_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = rnpca * rnzout;          /* + (acoef * input)    */
	rnpc_1 = temp4 + temp3;
}

/* Eighth cascaded Formant */

void resonc8() {

	register int temp3,temp4;

	temp4 = r8cc * r8c_2;	       /*   (ccoef * old2)     */
        r8c_2 = r8c_1;

	temp3 = r8cb * r8c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = r8ca * casc_next_in;   /* + (acoef * input)    */
	r8c_1 = temp4 + temp3;
}

/* Seventh cascaded Formant */

void resonc7() {

	register int temp3,temp4;

	temp4 = r7cc * r7c_2;	       /*   (ccoef * old2)     */
        r7c_2 = r7c_1;

	temp3 = r7cb * r7c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = r7ca * casc_next_in;   /* + (acoef * input)    */
	r7c_1 = temp4 + temp3;
}

/* Sixth cascaded Formant */

void resonc6() {

	register int temp3,temp4;

	temp4 = r6cc * r6c_2;	       /*   (ccoef * old2)     */
        r6c_2 = r6c_1;

	temp3 = r6cb * r6c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = r6ca * casc_next_in;   /* + (acoef * input)    */
	r6c_1 = temp4 + temp3;
}

/* Fifth Formant */

void resonc5() {

	register int temp3,temp4;

	temp4 = r5cc * r5c_2;	       /*   (ccoef * old2)     */
        r5c_2 = r5c_1;

	temp3 = r5cb * r5c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = r5ca * casc_next_in;   /* + (acoef * input)    */
	r5c_1 = temp4 + temp3;
}

/* Fourth Formant */

void resonc4() {

	register int temp3,temp4;

	temp4 = r4cc * r4c_2;	       /*   (ccoef * old2)     */
        r4c_2 = r4c_1;

	temp3 = r4cb * r4c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = r4ca * casc_next_in;   /* + (acoef * input)    */
	r4c_1 = temp4 + temp3;
}

/* Third Formant */

void resonc3() {

	register int temp3,temp4;

	temp4 = r3cc * r3c_2;	       /*   (ccoef * old2)     */
        r3c_2 = r3c_1;

	temp3 = r3cb * r3c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = r3ca * casc_next_in;   /* + (acoef * input)    */
	r3c_1 = temp4 + temp3;
}

/* Second Formant */

void resonc2() {

	register int temp3,temp4;

	temp4 = r2cc * r2c_2;	       /*   (ccoef * old2)     */
        r2c_2 = r2c_1;

	temp3 = r2cb * r2c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = r2ca * casc_next_in;   /* + (acoef * input)    */
	r2c_1 = temp4 + temp3;
}

/* First Formant of Cascade Vocal Tract */

void resonc1() {

	register int temp3,temp4;

	temp4 = r1cc * r1c_2;	       /*   (ccoef * old2)     */
        r1c_2 = r1c_1;

	temp3 = r1cb * r1c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

	temp3 = r1ca * casc_next_in;   /* + (acoef * input)    */
	r1c_1 = temp4 + temp3;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         10.         P A R A L L E L -  R E S O N A T O R S          */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*   Output = (acoef * input) + (bcoef * old1) + (ccoef * old2); */

/* Sixth Formant of Parallel Vocal Tract */

void reson6p() {

	register int temp3,temp4;

	temp4 = r6pc * r6p_2;
	r6p_2 = r6p_1;

	temp3 = r6pb * r6p_1;
	temp4 += temp3;

	temp3 = r6pa * sourc;
	r6p_1 = temp4 + temp3;
}

/* Fifth Formant of Parallel Vocal Tract */

void reson5p() {

	register int temp3,temp4;

	temp4 = r5pc * r5p_2;
	r5p_2 = r5p_1;

	temp3 = r5pb * r5p_1;
	temp4 += temp3;

	temp3 = r5pa * sourc;
	r5p_1 = temp4 + temp3;
}

/* Fourth Formant of Parallel Vocal Tract */

void reson4p() {

	register int temp3,temp4;

	temp4 = r4pc * r4p_2;
	r4p_2 = r4p_1;

	temp3 = r4pb * r4p_1;
	temp4 += temp3;

	temp3 = r4pa * sourc;
	r4p_1 = temp4 + temp3;
}

/* Third Formant of Parallel Vocal Tract */

void reson3p() {

	register int temp3,temp4;

	temp4 = r3pc * r3p_2;
	r3p_2 = r3p_1;

	temp3 = r3pb * r3p_1;
	temp4 += temp3;

	temp3 = r3pa * sourc;
	r3p_1 = temp4 + temp3;
}

/* Second Formant of Parallel Vocal Tract */

void reson2p() {

	register int temp3,temp4;

	temp4 = r2pc * r2p_2;
	r2p_2 = r2p_1;

	temp3 = r2pb * r2p_1;
	temp4 += temp3;

	temp3 = r2pa * sourc;
	r2p_1 = temp4 + temp3;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*          11.                N O - R A D - C H A R           	       */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define ACOEF		0.001	/* was .005 */
#define BCOEF		(1.0 - ACOEF)	/* Slight decay to remove dc */
#define GCOEF		(0.8/ACOEF)	/* Gain constant */

void no_rad_char(float in) {

/*	static float lastin; */
        extern float lastin;

	out = (ACOEF * in) + (BCOEF * lastin);
	lastin = out;
	out = -GCOEF * out;	/* Scale up to make visible */
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         12.           S U B R O U T I N E   G E T M A X             */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Find absolute maximum of arg1 & arg2, save in arg2 */

void getmax(arg1,arg2) int arg1; int *arg2; {

        if (arg1 < 0) {
            arg1 = -arg1;
	}

        if (arg1 > *arg2) {
            *arg2 = arg1;
	}
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         13.         S U B R O U T I N E   T R U N C A T E           */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Truncate arg to fit into 16-bit word */

int truncate_dk(arg) int arg; {

	if (arg < -32768) {
	    overload_warning(-arg);
	    arg = -32768;
	}
	if (arg >  32767) {
	    overload_warning(arg);
	    arg =  32767;
	}
	return(arg);
}

void overload_warning(arg) int arg; {

    static int warnsw;
    extern float dBconvert();

    if (warnsw == 0) {
	warnsw++;
	printf("\n* * * WARNING: ");
	printf(
	 " Signal at output of synthesizer (+%3.1f dB) exceeds 0 dB\n",
	 dBconvert(arg));
	printf("    at synthesis time = %d ms\n",
	 (disptcum*1000)/samrate);
	printf(" Output waveform will be TRUNCATED\n");

	pr_pars();
    }
}

float dBconvert(arg) int arg; {

	double x,log10();
	float db;

	x = arg / 32767.;
	x = log10(x);
	db = 20. * x;
	return(db);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*        14.          S U B R O U T I N E   P R - P A R S             */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void pr_pars() {

	int m4,m;	/* only used for debug printout */

	m4 = 0;
	printf("\n  Speaker-defining Constants:\n");
	for (m=0; m<N_SPDEF_PARS; m++) {
	    printf("    %s %5d\t", &symb[m][0], spkrdef[m]);
	    if (++m4 >= 4) {
		m4 = 0;
		printf("\n");
	    }
	}
	if (m4 != 0) {
	    printf("\n");
	}

	m4 = 0;
	printf("  Par values for this frame:\n");
 	for (m=0; m<N_VARIABLE_PARS; m++) {
	    printf("    %s %5d\t", &symb[N_SPDEF_PARS+m][0], pars[m]);
	    if (++m4 >= 4) {
		m4 = 0;
		printf("\n");
	    }
	}
	if (m4 != 0) {
	    printf("\n");
	}
}

/***************************************************/
/* in original version these were assume to be zero*/
extern float noiseinlast;
extern float lastin;
extern float r1c_1,r1c_2;
extern int nper,np;

void reset_data(){
np = 0;
nper = 0;
F1last = 0;
F2last = 0;
F3last = 0;
noiseinlast = 0.0;
lastin = 0.0;
noiselast = 0.0;
glotlast = 0.0;
/* do all resonators */

 r2p_1 = 0.0;  
 r2p_2 = 0.0;  
          
 r3p_1 = 0.0;  
 r3p_2 = 0.0;  
          
 r4p_1 = 0.0;  
 r4p_2 = 0.0;  
          
 r5p_1 = 0.0;  
 r5p_2 = 0.0;  
          
 r6p_1 = 0.0;  
 r6p_2 = 0.0;  
          
 r1c_1 = 0.0;
 r1c_2 = 0.0;
          
 r2c_1 = 0.0;  
 r2c_2 = 0.0;  
          
 r3c_1 = 0.0;  
 r3c_2 = 0.0;  
          
 r4c_1 = 0.0;  
 r4c_2 = 0.0;  
          
 r5c_1 = 0.0;  
 r5c_2 = 0.0;  
          
 r6c_1 = 0.0;  
 r6c_2 = 0.0;  
          
 r7c_1 = 0.0;  
 r7c_2 = 0.0;  
          
 r8c_1= 0.0; 
 r8c_2= 0.0; 
          
 rnpc_1= 0.0;  
 rnpc_2= 0.0;  
          
 rnz_1 = 0.0;  
 rnz_2 = 0.0;  
          
 rztr1_1= 0.0; 
 rztr1_2= 0.0; 
          
 rptr1_1= 0.0; 
 rptr1_2= 0.0; 
          
 rgl_1 = 0.0;  
 rgl_2 = 0.0;  
          
 rtlt_1= 0.0;  
 rtlt_2= 0.0;  
          
 rlp_1 = 0.0;  
 rlp_2 = 0.0;  

}/* end reset_data*/
