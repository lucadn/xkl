/* $Log: lsp.c,v $
 * Revision 1.6  1998/07/16 22:56:10  krishna
 * Deleted a extraneous \n in defining the default filename.
 *
 * Revision 1.5  1998/06/12 05:00:43  krishna
 * Changed how lspecto looks for library file, lspecto.con.  It is now
 * based on a variable specified in the Makefile.
 *
 * Revision 1.4  1998/06/06 21:05:16  krishna
 * Added RCS header.
 * */


#include <stdio.h>
#include <time.h>
#include "wavio.h"
#include "lsp.h"
#include "lspgrid.h"
#include "lspscrip.h"
#include "lspspec.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* dk$disk[klatt.lspecto] P S P E C T O . C             D. H. K L A T T
 *    LASER PRINTER (POSTSCRIPT) version of				
 *    PSEUDO-SPECTROGRAM Maker -- resides in [klatt.lspecto]		
 *	On VAX [klatt.lspecto]pspecto.c.37 modified 16-Sep-88 for  
 *	installation as system program,  type "@pspecto" to compile/load
 */

/* EDIT HISTORY:
 * 0001 21-Jul-87 DK	Initial creation from version 18-Mar-87 of specto.c
 * 0002 14-Aug-87 DK	Fix bug in spectrogram maker if waveform > 1 sec
 * 0003 11-Sep-87 DK	Offset spectrogram & f0 ps plot to allow 3-hole punch
 * 0004  1-Jan-88 DK	Fix bug in mgtodb() of LSPECTO.C, extend range
 * 0005 29-Feb-88 DK	Separate plot of f0 and amp
 * 0006  7-Mar-88 DK	Plot 100-ms waveform chunks, improve resolution
 * 0007 10-Mar-88 DK	Add -pff option, do formants every 5 ms
 * 0008 11-Mar-88 DK	Add -syn option, do formants and f0 into .doc file
 * 0009 14-Mar-88 DK	For -syn option, improve resolution of f0 to 0.1 Hz
 * 0010  8-Jun-88 Dk	Add -l option, list formants or f0 only if this arg added
 *-----------------------------------------------------------------------------*
 * 0011 26-Apr-91 SMH   Output correct .WAV file name in .DOC file.
 */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                              M A I N                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int main(int argc, char *argv[])
{
  struct tm *time_structure ;
  long time_val ;

  static char *month[12] = { "Jan", "Feb", "Mar", "Apr",
			     "May", "Jun", "Jul", "Aug",
			     "Sep", "Oct", "Nov", "Dec" } ;

  static int nw, ypos, nlines_page;
  static int ngrids,tgridmax,timetrunc,amplitude,xxsw,incrsw;
  static int ampspec;
  int d;

  if (argc <= 1) {	/* do usage*/
    helpa();
    exit(1);
  }

/*    Print version number of this program */
	printf("\n\n%s\n", date);
	incrsw = 1;

/*    Select buffer zero of 10 waveform buffers (KLSPEC uses all, we need 1) */
	nwave = 0;

/*    Default print option is none */
	prswitch = 'n';
	descrip = speco;

/*    Decode arguments */
        for (i=1; i<argc; i++) {
            if (argv[i][0] == '?') {
                helpa();	/* List arg options for user */
                exit(1);
            }
	    if (argv[i][0] == '-') {
/*	      "-f0" OPTION */
		if ((argv[i][1] == 'f') || (argv[i][1] == 'F')) {
		    prswitch = '0';
		    descrip = malef0;
		    if ((argv[i][3] == 'f') || (argv[i][3] == 'F')) {
			size_win_dft = (SIZEWINDFT*3)>>2;	/* Female */
			descrip = femf0;
		    }
		    printf("\n\tPlot and list fund freq and ampl");
		    printf(" in dB, use %d ms window.\n",
		      (size_win_dft+5)/10);
		}
/*	      "-pw" and "-pf" OPTIONS */
		else if ((argv[i][1] == 'p') || (argv[i][1] == 'P')) {
		    if ((argv[i][2] == 'w') || (argv[i][2] == 'W')) {
			prwavesw = 1;
			printf("\n\tPlot entire waveform.\n");
			if ((argv[i][3] == 'i') || (argv[i][3] == 'I')) {
			    prwavesw = 2;
			    printf("\t (inverted).\n");
			}
		    }
		    else if ((argv[i][2] == 'f') || (argv[i][2] == 'F')) {
			prswitch = 'f';
			specttype = LPC_SPEC;
			descrip = frmnts;
			size_win_specto = 256;	/* Use long window for LPC */
			if ((argv[i][3] == 'f') || (argv[i][3] == 'F')) {
			    descrip = frmntsf;
			    size_win_specto = 200; /* Use shorter window and */
			    nlpcoefs = 12;	   /* fewer poles for female */
			}
			printf("\n\tPlot and list formants based on peaks");
			printf(" in %d-pole lpc spectra.\n", nlpcoefs);
		    }
		    else {
			printf("\n\tError, this option not available.\n");
		    }
		}
/*	      "-syn" OPTION */
		else if ((argv[i][1] == 's') || (argv[i][1] == 'S')) {
		    if ((argv[i][2] == 'y') || (argv[i][2] == 'Y')) {
/*		      Do formant analysis first */
			prswitch = 's';
			specttype = LPC_SPEC;
			descrip = frmnts;
			size_win_specto = 256;	/* Use long window for LPC */
			if ((argv[i][4] == 'f') || (argv[i][4] == 'F')) {
			    descrip = frmntsf;
			    nfcascade = 4;
			    size_win_specto = 200; /* Use shorter window and */
			    nlpcoefs = 12;	   /* fewer poles for female */
			}
			printf("\n\tPlot and list formants based on peaks");
			printf(" in %d-pole lpc spectra.\n", nlpcoefs);
		    }
		}
/*	      "-l" OPTION */
		else if ((argv[i][1] == 'l') || (argv[i][1] == 'L')) {
		    listsw = 'l';	/* list formants or f0 on LXY22 */
		}
/*	      "-n" OPTION */
		else if ((argv[i][1] == 'n') || (argv[i][1] == 'N')) {
		    if ((argv[i][2] == 'n') || (argv[i][2] == 'N')) {
			normswitch = 'n';
			printf("\n\tDo not normalize wave to max peak amp.\n");
		    }
		}
/*	      "-digits" option */
		else {
		    size_win_specto = atoi(&argv[i][1]);
		    if ((size_win_specto < 64) || (size_win_specto > 512)) {
			printf(
			"ERROR: window must be between 64 and 512 points\n");
			exit(1);
		    }
		    printf("\n\tUse analysis window duration = %d samples\n",
		     size_win_specto);
		}
	    }
/*	  "firstname" OPTION */
	    else {
		pstring = frstname;
        	if (pstring[0] != '\0') {
		    printf("Too many waveform name arguments\n");
		    exit(1);
        	}


                /* accept filename or filename.wav */
                if(strcmp(&argv[i][strlen(argv[i]) - 3 ],"wav") == 0 )
                        {argv[i][strlen(argv[i]) - 4 ]  = '\0';}
            
                sprintf(wname, "%s.wav", argv[i] );
/* write into directory the user is running lspecto from */
/* remove path*/

    for(d = strlen(argv[i]) - 1; d >= 0; d --)
         if(argv[i][d] == '/')  break;

   strcpy(frstname,&argv[i][d+1]);
   get_wave();	/* Get waveform from disk, put in core buffer */
	    }
        }

/*    Request file name if no name given in arg list */
	if (frstname[0] == '\0') {
	    get_wave_name();
/*	  Read all of waveform into iwave[], set samper,nwtotr to srate,nwtot */
	    get_wave();
        }

/*    Amplitude normalize waveform so largest sample fills up bits avail. */
	if (normswitch != 'n') {
	    printf("\n\tFirst normalize waveform: max peak amp= ");
	    ampmax = 0;
	    p = iwave;
/*	  Find largest waveform sample */
	    for (n=0; n<nwtot; n++) {
		amp = *p++;
		if (amp < 0)    amp = -amp;
		if (amp > ampmax)    ampmax = amp;
	    }
	    if (ampmax > 0) {
		printf("%d, set to 32767\n", ampmax);
		p = iwave;
		scalefac = 32767. / ampmax;
		for (n=0; n<nwtot; n++) {
		    floatamp = *p * scalefac;
		    *p++ = floatamp;
		}
	    }
	    else {
		printf("Error, waveform consists entirely of silence.\n");
		exit(1);
	    }
	}

	time(&tvec);			/* Get time re 1970, to go on plots */
	strcpy(when, ctime(&tvec) );		/* Convert time to date */

/*    Plot waveform and quit */
	if (prwavesw > 0) {

/*	  Open output file of vectors for waveform plot on Laser plotter */
	    mergestring(frstname,".ps_pw",outname);
	    post_open(outname);
	    printf("\n\tPrint dot every 100 ms to reassure user:\n\t");

/*	  Plot in 1000-point chunks (usually 100-ms each) */
	    incr = 1000;

/*	  For three lines per page, set ypos=160, delta_ypos=210 */
/*	  For  four lines per page, set ypos=120, delta_ypos=165 */
	    nlines_page = 4;
	    nwtotsave = nwtot;
	    nwstart = 0;
	    while (nwtotsave > 0) {
		ypos = 150;
		nwtot = 4000;		/* Do a page consisting of 400 ms */
		if (nwtot > nwtotsave)    nwtot = nwtotsave;   /* or less */
	        for (nw=0; nw<nwtot; nw+=incr) {
		    if ((nwtot-nw) < 1000)    incr = nwtot - nw;
/*	          Plot waveform at current ypos position */
		    post_wave_plot(&iwave[nw+nwstart],ypos,incr);
/*	          Make a new page every time ypos set to .le. 160 */
		    post_wave_axis(wname,ypos,incr,when,nlines_page,wavplt);
		    ypos += 165;
		    if (ypos >= 780)    ypos = 150;
		    printf(".");
		}
		nwstart += nwtot;
		nwtotsave -= nwtot;
		if (nwtotsave > 0)    post_newpage();
	    }
	    post_close();
	    printf("\n");
	    exit(1);
	}

/*    Open output file for listing of f0 or formants */
	if (prswitch != 'n') {
	    if (prswitch == 's') {
		mergestring(frstname,".doc",proutname);
		if ((wdev = fopen(proutname, "w")) == NULL) {
		    printf("ERROR: Can't open output file\n");
		    exit(1);
		}
	    }
	    else if (listsw == 'l') {
		mergestring(frstname,".tem",proutname);
		if ((wdev = fopen(proutname, "w")) == NULL) {
		    printf("ERROR: Can't open output file\n");
		    exit(1);
		}
	    }


/*	  Look to see if a times-of-analysis file exists */
	    mergestring(frstname,".tim",timename);
	    if ((tdev = fopen(timename, "r")) == NULL) {
/* OUT		printf("\tTimes of voicing-fric-sil file %s does not exist\n",
		 timename); END OUT */
		tstart = 500000;
		curphonetype = 'v';
	    }
/*	  It exists, read first type/time */
	    else {
		printf(
		"\n\tOpen file %s giving times of begin voicing,fric,sil\n",
		 timename);
		if (fscanf(tdev, "%c%5d\n", &phonetype, &tstart) == EOF) {
		    tstart = 500000;
		    curphonetype = 'v';
		    printf("\tUnexpected format error in %s\n", timename);
		}
		else {
		    curphonetype = 's';
		    printf(
		    "\tAssume silence until t=%d ms, then start phonetype=%c\n",
		     tstart, phonetype);
		}
	    }
	}

/*    Ask for core array to put formant estimates in */
/*    Need 2000 bytes (1000 short words) for each second of speech */
	nsfcummax = nwtot/10;	/* Maximum number of formant estimates */
	if ( (savfor = (short *)malloc((nwtot+5) / 5)) == 0) {	/* In bytes !!! */
	    printf("No room for core storage of formant freq trajectories\n");
	    exit(1);
	}

/*    Compute number of samples in 3.33 ms if doing broadband spectogram */
/*	incr = 3334 / samper;
	incrms = 3;		*/
/* Try every msec: */
	incr = 1000 / samper;
	incrms = 1;

/*    Make header for xxxx.tem output file listing f0 or formant data */
	if (((prswitch == 'f') || (prswitch == '0')) && (listsw == 'l')) {
	    fprintf(wdev, "%s\n", date);
	    fprintf(wdev, "\tWaveform file name:\t\t\t %s\n", frstname);
	    fprintf(wdev, "\t%s", when);
	}
	else if (prswitch == 's') {
/*	  Read in config file, change duration, write out as .doc file */


	  /*
	   * XKLDIR specified by user in the Makefile, passed in 
	   * using -DXKLDIR on cc.
	   */

	  sprintf(conname, "%s/%s", XKLDIR, confile);
	  if ((idev = fopen(conname, "r")) == NULL){
		printf("Can't open file %s, abort\n", conname);
		exit(1);
	    }

/* Throw away first three lines of default .CON file. */

	    for (n=0; n<3; n++)
	       readline();

/* Output information about input .WAV file and current date to .DOC file. */

	    time( &time_val ) ;
	    time_structure = localtime( &time_val ) ;

	    fprintf(wdev, "%s \n\n", date);
	    fprintf(wdev, "Created:  %s %d, 19%d\t ",
			   month[time_structure->tm_mon],
			   time_structure->tm_mday,
			   time_structure->tm_year ) ;

	    fprintf(wdev, "\tSynthesis specification for file: %s\n", frstname);

	    for (n; n<13; n++) {
		readline();
		fprintf(wdev, "%s\n", junk);
	    }
/* Parameter line 1: DU -- duration: */
	    readline();
/*	    sprintf(&junk[20], "%4d", (nwtot/50)*5);	
	    junk[20+4] = ' ';				*/
	    for (i=0;((junk[i] == ' ') || (junk[i] == '\t')) && (i<121);
                        i++){}		/* count leading spaces */
	    sprintf(&junk[i+12], "%5d", (nwtot/50)*5);	/* g.lame */
	    junk[i+12+5] = ' ';	

/*	    sprintf(&junk[i+18], "%5d", (nwtot/50)*5);
	    junk[i+18+5] = ' ';	*/
/*klsyn88 wouldn't use longer dur when its initial value was set shorter ?*/

	    fprintf(wdev, "%s\n", junk);

	    for (n=0; n<2; n++) {
		readline();
		fprintf(wdev, "%s\n", junk);
	    }
/* Param line 4: NF -- number of formants: */
	    readline();
	    if (descrip == frmntsf) {
                for (i=0;((junk[i] == ' ') || (junk[i] == '\t')) && (i<121);
                        i++){}		/* count leading spaces */
		junk[i+16] = '4';		/* nfcascade */
/*		junk[23] = '4';	*/	/* nfcascade */
	    }
	    fprintf(wdev, "%s\n", junk);
	    for (n=0; n<57; n++) {
		readline();
/* Param line 4+28 -- F5: 	*/
		if ((descrip == frmntsf) && (n == 27)) {
                    for (i=0;((junk[i] == ' ') || (junk[i] == '\t')) && (i<121);
                        i++){}		/* count leading spaces */
		    junk[i+5] = 'v';	/* Do not vary F5 if female */
/*		    junk[11] = 'v';*/	/* Do not vary F5 if female */
		}
		fprintf(wdev, "%s\n", junk);
	    }

/* 3 additional skipped lines needed by KLSYN88  -- g.lame */
	    fprintf(wdev, "\n\n\nVaried Parameters:\n");
	}
/*    Compute number of samples in 5 msec for f0 or formant analysis */
	if (prswitch != 'n') {
	    incr = 5000 / samper;
	    incrms = 5;
	}

/* Main loop, do spectrogram or formant plot first */
    if (prswitch != '0') {
	if (prswitch == 'n') {
	    printf("\n\tBegin pseudo-spectrogram generation\n");
/*	  Open output file of vectors for spectrogram on Laser plotter */
	    mergestring(frstname,".ps",outname);
	    post_open(outname);
	}
	else {
/*	  Open output file of vectors for formant freq plot on Laser plotter */
	    mergestring(frstname,".ps_pf",outname);
	    post_open(outname);
/*	  Adjust window size for non-standard sampling rate */
	    if (samper != 100) {
		nlpcoefs = (nlpcoefs * 100) / samper;
		if (nlpcoefs > 20)    nlpcoefs = 20;
		printf("\tSet nlpcoefs = %d for formant freq est.\n", nlpcoefs);
	    }
	}
	if (prswitch == 'f' && listsw == 'l') {
	    fprintf(wdev,"\nTIME\t  F1\t  F2\t  F3\t  F4\t  F5\n");
	    fprintf(wdev, "(ms)\t (Hz)\t (Hz)\t (Hz)\t (Hz)\t (Hz)\n");
	}
	else if (prswitch == 's') {
	    if (descrip == frmntsf) {
		fprintf(wdev, "time   f0   av   F1   F2   F3   F4\n");
	    }
	    else {
		fprintf(wdev, "time   f0   av   F1   F2   F3   F4   F5\n");
	    }
	}
	printf(
	 "\n\tPrint dot every 20 ms, <CR> every second, to reassure user:\n\t");
/*    Adjust window size for non-standard sampling rate */
	if (samper != 100) {
	    size_win_specto = (size_win_specto * 100) / samper;
	    if (size_win_specto > 256)    size_win_specto = 256;
	    printf("\tSet size_win_specto = %d samples\n", size_win_specto);
	}
	sizwin = size_win_specto;
	sizhalfwin = sizwin >> 1;
	time_in_samp = 0;
/*    Set time to corresp approximately to center of first window */
	time_in_ms = 0;
	ngrids = 0;
	tgridmax = 0;
	timetrunc = 1000;
	start = (float) timetrunc / (float) incrms;
/* This start business doesn't seem to be quite right
   (it should be corrected for sizhalfwin in spectral displays)
   but seems to agree with the way things are done. gl */
	for (nw=0; nw<nwtot; nw+=incr) {
	    if ( (nw >= sizhalfwin) && (nw <= nwtot-sizhalfwin) ) {
/*	      Window waveform chunk, take DFT, convert to 128 300-Hz bw filters */
		getspec(&iwave[nw-sizhalfwin],specttype);
		amplitude = fltr[noutchan]/10;
		ampspec = fltr[noutchan];
/*	      Flag a formant peak if amplitude weak */
		for (i=1; i<nforfreq-1; i++) {
/*		  A formant weaker in amp than adjecent higher and lower
		  frequency formants should be made faint */
		    if ((foramp[i] < foramp[i-1])
		     && (foramp[i] < foramp[i+1])) {
			if (forfreq[i] > 0) {
			    forfreq[i] = -forfreq[i];
			}
		    }
		}
/*	      If strongest peak in F1 region is weak, make display faint */
		ampmax = 0;
		for (i=0; i<nforfreq; i++) {
		    if ((forfreq[i] > 280) && (forfreq[i] < 1000)) {
			if (foramp[i] > ampmax)    ampmax = foramp[i];
		    }
		}
		if (ampmax < 260) {
		    for (i=0; i<nforfreq; i++) {
			if (forfreq[i] > 0) {
			    forfreq[i] = -forfreq[i];
			}
		    }
		}
/*	      Do postscript plot grid */
		if (time_in_ms > tgridmax) {
		    if (prswitch == 'n' && ngrids>0) 
			     post_amp_plot(ngrids,start);
		    spectgrid(wname,++ngrids,srate[nwave]>>1,when,descrip);
		    tgridmax += 1000;
		    timetrunc -= 1000;
		    start = (float) timetrunc / (float) incrms;
		}
/*	      Plot formants */
		if ((prswitch == 'f') || (prswitch == 's')) {
/*		  Process input command file of times to plot (if it exists) */
/*		  If command file doesnt exist, tstart=50000, skip this code */
		    while (time_in_ms >= tstart) {
			printf("\tBegin phonetype = %c at t = %d\n\t",
			 phonetype, tstart);
			curphonetype = phonetype;
			if (fscanf(tdev, "%c%5d\n", &phonetype, &tstart) == EOF) {
			    tstart = 500000;
/*			  Last time should begin a silence interval */
			    if (phonetype != 's') {
				printf("\tUnexpected format error in %s\n", timename);
			    }
			    break;
			}
		    }
/*		  Plot formant frequencies */
		    if (curphonetype == 'v') {
			post_freq_plot(forfreq, nforfreq,amplitude, timetrunc);
		    }
		}
/*	      Normal broadband spectrogram plot */
		else {
		    post_spec_plot(fltr, ampspec, timetrunc);
		}
	    }
	    else {
		nforfreq = 0;
	    }

/*	  Move ahead by 3,3,4 ms in plot so that keep synch to 10 ms */
	    if (xxsw == 1) {
		timetrunc += incrms;
		if (incrms == 3)    xxsw = 2;
	    }
	    else if (xxsw == 2) {
		timetrunc += incrms+1;
		if (incrms == 3)    xxsw = 0;
	    }
	    else {
		timetrunc += incrms;
		if (incrms == 3)    xxsw = 1;
	    }

/*	  Put formant values into a file name.tem */
	    listformants();
	    time_in_samp += incr;
	    time_in_ms += incrms;
/*	  Compensate for rounding error in incr */
	    if ((incrms == 3) && (++incrsw == 3)) {
/*	      Round up to be nominally correct at 10 ms increments */
		time_in_ms++;
/*	      Round up to be 100 samples per 10 ms, i.e. 33,33,34 */
		if (incr == 33)    nw++;
		incrsw = 0;
	    }
/*	  Print dot every 20 ms */
	    if (n5++ >= 6) {
		n5 = 0;
		printf(".");
	    }
/*	  Print <cr> every second */
	    if (nsec++ >= 299) {
		nsec = 0;
		printf("\n\t");
	    }
	}
	if (prswitch == 'n') post_amp_plot(ngrids,start);
	if ((prswitch == 'f') && (listsw == 'l')) {
	    printf(
      "\n\n\tListing of formant values is in .tem file,");
	    printf("\n\t Wrote postscript file.\n");
	    fprintf(wdev,
	     "\n---------------------\n* A negative value indicates that the");
	    fprintf(wdev,
	     " formant peak is weaker than adjacent peaks.\n");
	}
	else printf("\n\n");
    }

/* Do f0 analysis as second step in -syn procedure */
    if (prswitch == 's') {
	if (descrip == frmntsf) {
	    size_win_dft = (SIZEWINDFT*3)>>2;  /* Female */
	    descrip = femf0;
	}
	else    descrip = malef0;
	printf("\n\tPlot and list fund freq and ampl");
	printf(" in dB, use %d ms window.\n", (size_win_dft+5)/10);
    }

/* F0 analysis and print formant freqs at end of file */
    if ((prswitch == '0') || (prswitch == 's')) {
	nsfcum = 0;
/*    Open output file of vectors for f0 plot on Laser plotter */
	if (prswitch == '0') {
	    mergestring(frstname,".ps_f0",outname);
	    post_open(outname);
	}
	else {
	    post_newpage();
	}
	printf(
	 "\n\tPrint dot every 25 ms, <CR> every second, to reassure user:\n\t");
	if ((prswitch == '0') && (listsw == 'l')) {
	    fprintf(wdev, "\nTIME      Amp       F0\n");
	    fprintf(wdev, "(ms)      (dB)     (Hz)\n");
	}
/*    Adjust window size for non-standard sampling rate */
	if (samper != 100) {
	    size_win_dft = (SIZEWINDFT * 100) / samper;
	    if (size_win_dft > 512)    size_win_dft = 512;
	    printf("\tSet size_win_dft = %d samples\n", size_win_dft);
	}
	sizwin = size_win_dft;
	sizhalfwin = sizwin >> 1;
	noutchan = 256;
	nsfcum = 0;
	n5 = 0;
	nsec = 0;
	time_in_ms = 0;
	ngrids = 0;
	tgridmax = 0;
	timetrunc = 1000;
	for (nw=0; nw<nwtot; nw+=incr) {
/*	  See if time exceeds tstart, which is set to 5000000 if no */
/*	  times of analysis file */
	    while (time_in_ms >= tstart) {
		printf("\tBegin phonetype = %c at t = %d\n\t",
		 phonetype, tstart);
		curphonetype = phonetype;
		if (fscanf(tdev, "%c%5d\n", &phonetype, &tstart) == EOF) {
		    tstart = 500000;
/*		  Last time should begin a silence interval */
		    if (phonetype != 's') {
			printf("\tUnexpected format error in %s\n", timename);
		    }
		    break;
		}
	    }
/*	  Enable Postscript plot of f0 */
	    if (curphonetype == 'v') {
		goprint = 1;
		if ((listsw == 'l') || (prswitch == 's')) {
		    fprintf(wdev, "%4d", time_in_ms);
		}
	    }
	    else    goprint = 0;
	    if ( (nw >= sizhalfwin) && (nw <= nwtot-sizhalfwin) ) {
		getspec(&iwave[nw-sizhalfwin],DFT_SPEC);
		amplitude = fltr[noutchan]/10;
/*	      Do postscript plot stuff */
		if (time_in_ms > tgridmax) {
		    post_f0_grid(wname,++ngrids,400,when,descrip);
		    tgridmax += 1000;
		    timetrunc -= 1000;
		}
		if (curphonetype == 'v') {
		    post_f0_plot((forfreq[0]+5)/10, amplitude, timetrunc);
		}
		if ((goprint == 1) && ((listsw == 'l') || (prswitch == 's'))) {
		    if (prswitch == '0') {
			fprintf(wdev, "     %4d", amplitude);
			fprintf(wdev, "     %4d     ", (forfreq[0]+5)/10);
			fprintf(wdev, "\n");
		    }
		    else {
			fprintf(wdev, "%5d%5d", forfreq[0], amplitude);
			for (n=0; n<nfcascade; n++) {
			    if (savfor[nsfcum+n] < 0) {
				savfor[nsfcum+n] = -savfor[nsfcum+n];
			    }
			    fprintf(wdev, "%5d", savfor[nsfcum+n]);
			}
			fprintf(wdev, "\n");
		    }
		}
	    }
	    else {
		amplitude = 0;
		forfreq[0] = 0;			/* Clamp f0 to zero */
		if ((goprint == 1) && ((listsw == 'l') || (prswitch == 's'))) {
		    if (prswitch == '0') {
			fprintf(wdev, "     %4d", amplitude);
			fprintf(wdev, "     %4d     ", forfreq[0]);
			fprintf(wdev, "\n");
		    }
		    else {
			fprintf(wdev, "%5d%5d", forfreq[0], amplitude);
			for (n=0; n<nfcascade; n++) {
			    if (savfor[nsfcum+n] < 0) {
				savfor[nsfcum+n] = -savfor[nsfcum+n];
			    }
			    fprintf(wdev, "%5d", savfor[nsfcum+n]);
			}
			fprintf(wdev, "\n");
		    }
		}
	    }
	    timetrunc += 5;
	    nsfcum += 5;
	    time_in_ms += 5;
	    if (n5++ >= 4) {
		n5 = 0;
		printf(".");
	    }
	    if (nsec++ >= 199) {
		nsec = 0;
		printf("\n\t");
	    }
	}
	printf("\n\t Wrote postscript file.\n");
    }
    printf("\n");
    post_close();
}


/* List up to 5 formant freq estimates */

void listformants() {

	if (curphonetype == 'v') {
	    goprint = 1;
	    if ((prswitch == 'f') && ((listsw == 'l') || (prswitch == 's'))) {
		fprintf(wdev, "%4d", time_in_ms);
	    }
	}
	else    goprint = 0;
	for (n=0; n<5; n++) {
	    if (n < nforfreq) {
		savfor[nsfcum+n] = forfreq[n];
	    }
	    else {
		if (nsfcum > 4) {
		    savfor[nsfcum+n] = savfor[nsfcum+n-5];	/* use prev. */
		}
		else {
		    savfor[nsfcum+n] = 500 + (n * 1000);
		}
	    }
	    if ((goprint == 1) && (n < nforfreq)
	     && (prswitch == 'f') && ((listsw == 'l') || (prswitch == 's'))) {
		fprintf(wdev, "\t%5d ", savfor[nsfcum+n]);
	    }
	}
	if ((goprint == 1) && (prswitch == 'f') && (listsw == 'l')) {
	    fprintf(wdev, "\n");
	}
	nsfcum += 5;
	if (nsfcum > nsfcummax) {
/*
	    printf("\n\tERROR in SPECTO.C: Array savfor[] space exceeded\n");
*/
	    nsfcum -= 5;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                        G E T _ W A V E	                         */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void get_wave() 
{
  float samp;

  printf("\n\tBegin reading file %s ...", wname);

  iwave = (short *)getWavWaveform(wname, &samp, &nwtot, SWAP);
  srate[nwave] = (int) samp;
  samper = (int)(1000000 / samp + .5);
  printf("  %d samples @ %d samp/sec\n", nwtot, srate[nwave]);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                          G E T - W A V E - N A M E                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void get_wave_name() 
{
  int n,d;
  char temp[100];

try1:	    printf("\n\tFirst name of input waveform file: ");
	    pstring = frstname;
	    n = 0;
	    for (n=0; n<80; n++) {
		scanf("%c", &sym1);
		if (sym1 != '\n') {
		    *pstring++ = sym1;
		}
		else {
		    if (n == 0) {
			printf("\tTry again.  ");
			goto try1;
		    }
		    break;
		}
	    }
	    if (n > 75) {
		printf("Too many chars in first name (75 max), try again.\n");
		goto try1;
	    }
	    *pstring = '\0';


                /* accept filename or filename.wav */
                if(strcmp(&frstname[strlen(frstname) - 3 ],"wav") == 0 )
                        {frstname[strlen(frstname) - 4 ]  = '\0';}
            
                sprintf(wname, "%s.wav", frstname);
                strcpy(temp,frstname);

/* write into directory the user is running lspecto from */
/* remove path*/
    for(d = strlen(temp) - 1; d >= 0; d --)
         if(temp[d] == '/')  break;

   strcpy(frstname,&temp[d+1]);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                          R E A D L I N E	                         */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void readline() 
{
  int j = -1;
  do {
    if (j < 120)    j++;
    fscanf(idev, "%c", &junk[j]);
  } while (junk[j] != '\n');
  junk[j] = '\0';
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                          M E R G E - S T R I N G S                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void mergestring(char *fname, char *sname, char *oname)
{
        int nfirst;

loopn:  n = 0;

        while ((n < 80) && (fname[n] != '\0')) {
            oname[n] = fname[n];
            n++;
        }
        nfirst = n;
        while ((n < 80) && (sname[n-nfirst] != '\0')) {
            oname[n] = sname[n-nfirst];
            n++;
        }
        if (n < 80) {
            oname[n] = '\0';
        }
        else {
illnam:     printf("\n\tERROR: Illegal name '%s'\n",
              frstname);
	    exit(1);
        }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                  H E L P - L I S T - A R G U M E N T S                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void helpa() {

        printf("\n    Required arguments for LSPECTO:\n");
        printf("\tname\tFirst name of waveform to be analyzed\n");

        printf("\n    Optional arguments (separated by spaces):\n");
	printf("\t-pw\tRequest plot of waveform\n");
	printf("\t-pwi\tRequest plot of inverted waveform\n");
	printf("\t-f0\tRequest plot of fund freq and energy in dB\n");
	printf("\t-f0f\tRequest plot of fund freq (female) and energy in dB\n");
	printf(
	 "\t-pf\tRequest plot of formants based on maxima in lpc spectra\n");
	printf(
	 "\t-pff\tRequest plot of formants (female) based on maxima in lpc spectra\n");
	printf("\t-l\tAlso list fund freq or formant values in .tem file\n");
	printf("\t-nn\tRequest no normalization of wave to max peak amp\n");
	printf("\t-___\tUse analysis window duration of ___ samples\n");
	printf("\t-syn\tProduce parameter .doc file for input to KLSYN88\n");
	printf("\t-synf\tSame as -syn but for female voice (only 4 formants)\n");
        printf("\n");
}

