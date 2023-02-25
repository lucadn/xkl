/*
$Log: synparwav.c,v $
Revision 1.2  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#include <stdio.h>
#include <math.h>
#include "synparwav.h"

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
