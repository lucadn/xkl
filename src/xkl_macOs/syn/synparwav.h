/*
$Log: synparwav.h,v $
Revision 1.2  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#ifndef SYNPARWAV_H
#define SYNPARWAV_H
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                    P A R W A V T A B . H                            */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */



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
void setabc(int,int,float *,float *,float *);
void setzeroabc(int,int,float *,float *,float *);
void parwav(short *);
void gethost();
void gen_noise();
void setR1(int,int);
void pitch_synch_par_reset();
void resonlp();
void resonglot();
void resontilt();
void spacefilt();
void resontz();
void resontp();
void resonnz();
void resonnp();
void resonc8();
void resonc7();
void resonc6();
void resonc5();
void resonc4();
void resonc3();
void resonc2();
void resonc1();
void reson6p();
void reson5p();
void reson4p();
void reson3p();
void reson2p();
void no_rad_char(float);
void getmax(int,int*);
int truncate_dk(int);
void overload_warning(int);
float dBconvert(int);
void pr_pars();
void reset_data();

#endif /*SYNPARWAV_H*/
