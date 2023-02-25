/*
$Log: syn.c,v $
Revision 1.5  2001-09-14 16:09:40-04  tiede
read default60.con from executable directory

Revision 1.4  1999/06/19 04:30:03  krishna
Vishal added a check for printing out dB.

Revision 1.3  1998/07/16 22:58:30  krishna
- Change where syn looks for lib files by using -DXKLIBDIR=${LIBDIR}
in Makefile.

Revision 1.2  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#include "syn.h"
#include "synscrip.h"
#include <stdlib.h>
#include <string.h>
#include "syngrid.h"
#include "synparwav.h"
char when[100];
time_t tvec;

/* EDIT HISTORY:
 * 0001 20-Jan-88 DK	Initial creation from KLSYN.C;270
 * 0002 18-Feb-88 DK	Ready to install on system
 * 0003 22-Feb-88 DK	Add LF glottal source model to PARWAVE.C, 'G' -> 'L'
 * 0004 10-Mar-88 DK	Remove all calls to lxy
 * 0005  6-Apr-88 DK	Add -b 'batch-job' disable Keyio so command files work
 * 0006 19-Aug-88 DK	Change name to KLSYN88
 * 0007  6-Sep-88 DK	Allow 3-char names for parameters
 *      24-Feb-93 NM    Allow impulse in parallel tract KLSYN93
 *         May-94 DH    Took out all graphics/write .wav on big endian
 */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      A C T - O N - R E Q U E S T                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void printstuff(){

      printf("\n\n%s\n\n", kldate);

}/* end printstuff */
void actonrequest(char request) {

	endgraph();	/*  5. Clear screen if graphic data present */
	if (request == 'q') {
            printf(
	    "\tExit program, saving nothing.  Confirm [CR] or save pars [S] ");
            command = get_request();	/* 18. */
            if ((command != 'S')&&(command != 's')) {
                exit(1);
            }
            request = 'S';
        }

        if ((request == 'P') || (request == 'p')) {
            putconfig(stdout);		/* 3. Print configuration */
        }

        else if ((request == 'C') ||  (request == 'c')) {
            if ((np = get_par_name()) >= 0) {		/* 19. */
                settypicalpar();	/* 4. Set typical value for parameter */
            }
        }

        else if (request == 'd') {
            drawparam();		/* 5. change time varying parameters */
        }


        else if ((request == 'G') || (request == 'L')) {
            graphallpars();		/* 7. Postscrip plot of pars */
        }

	else if ((request == 's') || (request == 'S')) {
	    askforname();		/* 11. */
          if (n!=0) {  /* 93 if non-empty file name */
	    if (request == 's') {
	        synthesize();		/* 10. Synthesize a waveform */
	      
	      }

#ifdef VMS
/* 93 make sure file not saved as streamline */
fab= cc$rms_fab;
fab.fab$b_bks=4;
fab.fab$l_fna=dname;
fab.fab$b_rat= FAB$M_CR;
fab.fab$b_rfm= FAB$C_FIX;
fab.fab$b_shr= FAB$M_NIL;	        
rab=cc$rms_rab;
rab.rab$l_fab= &fab;
#endif


            savedocfile();


	    exit(1);		/* Quit so synthesizer init next time used */
	  }/* n!= 0 */
	}/* S or s */

        else {
            helpr();            /* 28. Print command menu */
        }
      }/* acton_request */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      0.              S A V E - D O C - F I L E                        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void savedocfile(){

/*	  Save a .doc file */

	    if ((odoc = fopen(dname,"w")) != NULL) {
		fprintf(odoc,
		 "\t\t\tSynthesis specification for file:    '%s'\n", wname);
		fprintf(odoc, "\n%s\n", kldate);

		if (sigmx <= 0)
		  sigmx = 1;  /* from synthesize function */
	        fprintf(odoc,
"\n\tMax output signal (overload if greater than 0.0 dB) is  %3.1f dB  \n",
		 dBconvert(sigmx));

                 if (nsamtot==0 ) nsamtot=nframes*nsperframe;  /* corrected
                        3/93 to change nsamtot if got here through quit,save */
		fprintf(odoc,
		 "\tTotal number of waveform samples = %d\n\n", nsamtot);
		putconfig(odoc);	/*  3. Output synth config & defaults */
		fprintf(odoc, "\n");
		prpars(odoc);		/* 17. Output variable par time funcs */
		fprintf(odoc, "\n\n");
	        fclose(odoc);
	        printf(
		 "\tSynthesis config and varied pars saved in file  '%s'\n",
		 dname);
	    }
	    else {
	        printf("\tERROR: Can't open output file '%s'\n", dname);
	    }

}/* end savedocfile */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      1.              R E A D - D O C - F I L E                        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void readdocfile()   {

	int nc;

/* If there is a doc file with first name, open it and skip over beginning */
	if((fconf = fopen(dname,"r")) != NULL){
		skip_line(fconf); /* 24. Skip "synthesis spec ... " line */
		skip_line(fconf); /* Skip blank line */
		skip_line(fconf); /* Skip version number identification */
		skip_line(fconf);
		skip_line(fconf); /* Skip "Max output" line */ 
		skip_line(fconf); /* Skip "number of samples" line */
		skip_line(fconf);

	}
/*    Otherwise, abort */
	else {
		printf("\tConfig/params file '%s' does not exist, abort\n",
		 dname);
		exit(1);
	}

	printf("    Reading config file '%s' ...\n", dname) ;

	readheader();		/*  2. Read default values into arrays */

/*    Check how many parameters are varied */
	nvar = 0 ;
	for(n=0; n<npars; n++) {
		if (cv[n] == VARRIED)  nvar++ ;
		if ((n < 12) && (cv[n] != FIXED_S)) {
		    printf(
	    "\n\nERROR: File '%s' incompatable with new KLSYN93, use KLSYN\n\n",
		     dname);
		    exit(1);
		}
	}
	printf("\t%d parameters are time varying\n", nvar) ;

/*    Set all params to config values */
	skip_line(fconf);	/* 24. Finish last line */

	for(np=0; np<npars; np++)
		clearpar();	/* 12. Enter default values into par time funcs */
	ipsw = TTRUE ;
/*   If any parameters were varied, read rest of file and fill in values */
	if (nvar != 0) {
		skip_line(fconf);	/* 24. Skip four blank lines */
		skip_line(fconf);
		skip_line(fconf);
		skip_line(fconf);
		skip_line(fconf);	/* Skip "Varied params" line */

		/* fill in values for varied parameters */
		for(nf=0; nf<nframes; nf++)  {
		    get_digits(fconf);			/* 22. Skip time */
		    for(np=0; np<npars; np++)  {
			if(cv[np] == VARRIED) 
			    pdata[np][nf] = get_digits(fconf);	/* 22. */
		    }
		}
	}
	fclose(fconf);
      }



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      2.       R E A D - D E F A U L T - C O N F I G - F I L E         */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void initconfig(char *synDefPath) 
{
  char filename[400];

  /*
   * XKLDIR specified by user in the Makefile, passed in 
   * using -DXKLDIR on gcc.
   */
  	printf("\n  Reading default config file \n");
  if (synDefPath)
  {
    sprintf(filename, "%s/%s", synDefPath, defaultFile);
    //	printf("\n  synDefPath not null\n");
    }
  else
  {
     // 	printf("\n  synDefPath null\n");
       strcpy(filename, defaultFile);
    }

		//printf("\n  Reading default config file pippo ...\n");
  /*    Open default configuration file */

	if ((fconf = fopen(filename, "r")) == NULL) {
	  printf("\tERROR: Default configuration file '%s' does not exist\n",
		 filename);
	  exit(1);
        }
	//printf("\n  Reading default config file %s ...\n", filename);
	/*    Read header of a config file into various arrays */
        readheader();		/*  2. */
   //printf("\n  File reading  complete\n");
        fclose(fconf);
        	
}

void readheader() {

        skip_line(fconf);	/* 24. */
        skip_line(fconf);	/* 24. Ignore file header "Current config" */
        npars = get_digits(fconf);    /* 22. Number of pars in config file */
        skip_line(fconf);
        skip_line(fconf);
        skip_line(fconf);
        skip_line(fconf);	/* and ignore header "SYM  V/C ..." */


       for (n=0; n<(npars+(npars<63)*(63-npars)); n++) { 
	 /* added 2/93 to read old .doc files */
	 if ((npars==60)&&(n==12)) {
	   symb[n][0] = 'G';
	   symb[n][1] = 'I';
	   symb[n][2] = '\0';
	   cv[n] = 'C';
	   minval[n] = 0;
	   defval[n] = 60;
	   maxval[n] = 80;
	   syn_i=0;
	      while ((definition[n][syn_i]=definitionGI[syn_i]) !='\0')
                syn_i++;
	   n++;
           }


/*	  Read 2 or 3 char symbol for each synthesis parameter */
	    nc = 0;
	    symb[n][0] = get_nonwhite_char(fconf);
	    do {
		nc++;
		fscanf(fconf, "%c", &symb[n][nc]);
	    } while ((symb[n][nc] != ' ') && (symb[n][nc] != '\0')
	     && (nc < 3));
/*	  Make symbols all caps */
	    symb[n][0] = toupper(symb[n][0]);
	    symb[n][1] = toupper(symb[n][1]);
	    symb[n][2] = toupper(symb[n][2]);
	    symb[n][nc] = '\0';
            cv[n] =    get_nonwhite_char(fconf);
            minval[n] = get_digits(fconf);		/* 22. */
            defval[n] = get_digits(fconf);
            maxval[n] = get_digits(fconf);
/*	  Read text description of each synthesis parameter */
	    nc = -1;
	    do {
		nc++;
		fscanf(fconf, "%c", &definition[n][nc]);
	    } while ((definition[n][nc] != '\n') && (nc < 50));
/*	  See if description is too long to be displayed on screen */
	    if (definition[n][nc] != '\n') {
		printf(
		"\nERROR: Description of parameter %s is too long, remainder is: ",
		&symb[n][0]);
		do {
		    printf("%c", definition[n][nc]);
		    fscanf(fconf, "%c", &definition[n][nc]);
		} while (definition[n][nc] != '\n');
	    }
	    definition[n][nc] = '\0';

/* added 2/93 to read old .doc files */
      if ((npars==60)&&(n==60)){      
	n++;  /*  n now = 61 */
	symb[n][0] = 'A';
	symb[n][1] = 'I';
	symb[n][2] = '\0';
	cv[n] = 'v';
	minval[n] = 0;
	defval[n] = 0;
	maxval[n] = 80;
	syn_i=0;
	   while ((definition[n][syn_i]=definitionAI[syn_i]) !='\0')
                syn_i++;
	}


     if ((npars<63)&&(n==61)){
        n++; /* n now = 62 */
        npars=63;   /* we've added three new parameters GI ,AI and FSF */
	symb[n][0] = 'F';
	symb[n][1] = 'S';
	symb[n][2] = 'F';
	symb[n][3] = '\0';
	cv[n] = 'v';
	minval[n] = 0;
	defval[n] = 0;
	maxval[n] = 1;
	syn_i=0;
	while ((definition[n][syn_i]=definitionFSF[syn_i]) !='\0')
                syn_i++;
          }

       }
        setspdef();		/*  2. Set speaker-defining constants */
}


void setspdef() {			/* Change DU, UI, or SR */


/*    Read all speaker-def constants into array in order expected by parwave.c */
	spkrdef[0] =  defval[findnp("DU")];
	spkrdef[1] =  defval[findnp("UI")];
	spkrdef[2] =  defval[findnp("SR")];
	spkrdef[3] =  defval[findnp("NF")];
	spkrdef[4] =  defval[findnp("SS")];
	spkrdef[5] =  defval[findnp("RS")];
	spkrdef[6] =  defval[findnp("SB")];
	spkrdef[7] =  defval[findnp("CP")];
	spkrdef[8] =  defval[findnp("OS")];
	spkrdef[9] =  defval[findnp("GV")];
	spkrdef[10] = defval[findnp("GH")];
	spkrdef[11] = defval[findnp("GF")];
        spkrdef[12] = defval[findnp("GI")];

/*    Set number of frames in utterance, and number of samples per frame */
	samrat =   defval[findnp("SR")];
	nforfreq = defval[findnp("NF")];
	totdur   = defval[findnp("DU")];
	ms_frame = defval[findnp("UI")];
/*    Redefine UI to be in samples */
        spkrdef[1] = (ms_frame * samrat) / 1000;
	nsperframe = spkrdef[1];
        nframes = (totdur + ms_frame - 1) / ms_frame;	/* Round up */
        if (nframes > MAX_FRAMES) {
            printf("\n\tToo many frames for parameter buffer,");
            printf(" MAX_FRAMES=%d frames,\n", MAX_FRAMES);
            nframes = MAX_FRAMES;
	    printf(
	 "\t  duration truncated.  (Increase 'ui' or recompile with larger\n");
	    printf(
	     "\t   MAX_FRAMES before setting 'du' greater than %d ms).\n\n",
	     nframes*ms_frame);
        }
        totdur  = nframes * ms_frame;

        printf("\tUtterance duration = %d ms,  %d ms per frame\n",
          totdur, ms_frame);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      3.                P U T - C O N F I G - F I L E                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void putconfig(dev) FILE *dev; {

	char symspace;

	if (dev == stdout) {
	    fprintf(dev, "\n  CURRENT CONFIGURATION:    (%2d parameters)\n",
		 npars);
	}
	else {
	    fprintf(dev, "\n  CURRENT CONFIGURATION:\n");
	    fprintf(dev, "    %2d parameters\n\n", npars);
        }
	if (dev != stdout) {
	   fprintf(dev, "  ");
	}
	fprintf(dev, "SYM V/C  MIN   VAL   MAX  DESCRIPTION\n");
        fprintf(dev, "----------------------------------------------\n");
        for (n=0; n<npars; n++) {
	    if (dev == stdout) {
		fprintf(dev, "%2d. ", n+1);
	    }
	    else {
		fprintf(dev, "  ");
	    }
	    if (symb[n][2] == '\0')  symspace = ' ';
	    else symspace = symb[n][2];
            fprintf(dev, "%c%c%c  %c %4d %5d %5d %s",
              symb[n][0], symb[n][1], symspace, cv[n], minval[n],
	      defval[n], maxval[n], &definition[n][0]);
            fprintf(dev, "\n");
	    if ((n == 19) && (dev == stdout)) {
		printf(
		"\nTo see additional synthesis control parameters, hit return");
		command = get_request();
	        fprintf(dev, "\nSYM V/C  MIN   VAL   MAX  DESCRIPTION\n");
        	fprintf(dev,
		 "----------------------------------------------\n");
	    }
	    if ((n == 39) && (dev == stdout)) {
		printf(
		"\nTo see additional synthesis control parameters, hit return");
		command = get_request();
	        fprintf(dev, "\nSYM V/C  MIN   VAL   MAX  DESCRIPTION\n");
        	fprintf(dev,
		 " ----------------------------------------------\n");
	    }
	  }
        fprintf(dev, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      4.                  S E T - T Y P I C A L - P A R                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void settypicalpar() {

   if (cv[np] == VARRIED) {
     printf("\tThis parameter already varied, change in default has no effect\n");
	    return;
	}
	printf("\tChange value of %s (min = %d, max = %d) to\n", 
	 &symb[np][0], minval[np], maxval[np]);
	if ((val = get_value(defval[np])) >= 0) {		/* 21. */
            defval[np] = val;
	}
	clearpar();			/* 12. Put new value in time function */

	if (cv[np] == FIXED_S) {
	    setspdef();        /*  2. Reset speaker-defining attributes */
	}
        if ((user == NOVICE) || (val != defval[np])) {
            putconfig(stdout);		/*  3. */
        }
      }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      5.                  D R A W - P A R A M E T E R                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void drawparam() {

	int nfsw;

        if ((graphout == VT100) && (nvar > 0)) {
            prparnames(stdout);		/* 17. Print names of varied paramters */
        }

/*    Initialize paramter buffer if not done yet */
        if (ipsw != TTRUE) {
            initpars();				/* 12. */
        }

/*    Select paramter (loop here until user types 'q' or <CR> */
/* debug while */
        if ((np = get_par_name()) >= 0) {	/* 19. */

            if (cv[np] == FIXED_S) {
                printf("\tERROR: '%s' Not a variable param", &symb[np][0]);
                printf(" try again\n");
            }
            else {
                if (cv[np] != VARRIED) {
                    nvar++;
                    cv[np] = VARRIED;
                }

		if ((graphout == VT100) && (warningsw == 0)) {
		    printf(
		     "\tNo graphics capability on default VT100 terminal\n");
		    warningsw = 1;
		}

/*	      Clear screen, make grid, enable draw on VT-125 */
		begingraph();		/*  5. */

/*	      Read very first point (i.e. do not draw line from previous point */
/* only read if not impulse (plotting single point only makes sense if impulse)*/
		if (np!=61){
                         gettimval(&nf,&val);	/*  5. */
		       }
		else   {
                 nf=0;              /* insure next loop is executed */
                /* value of zero entry */
		 val=pdata[np][nf]; /* retain current value*/
	       }
                if (nf >= 0) {

/*                Set switches to cause setting of value of initial point */
		    nfsw = nf;
                    lastnf = nf;
/*                Now draw a bunch of straight lines until time = <CR> */
		    while (1) {
                        lastval = val;
			gettimval(&nf,&val);	/*  5. */
			if (nf < 0) {
			    if (nfsw != -1) {	/* Set single point upon exit */
	                	pdata[np][nfsw] = lastval;
			    }
			    break;
			}

/* if entered time for impulse, make it last only 1 sample  */

                if (np==61)  {
                     lastnf=nf;
                   }



/*		      In order to graph new line segment, first erase old one */
			eraseparline();		/*  5. */

/*		      Change values in data array */
			if (nfsw != -1) {
/*			  Must wait until erase before setting first point */
	                    pdata[np][nfsw] = lastval;
			    nfsw = -1;
			}
                        draw_line();		/* 13. */
                        lastnf = nf;

/*		      Draw new line segment on graphical teminal */
			drawparline();		/*  5. */

                    }
		}
            }
        }
        /*if ((graphout == VT100) && (nvar > 0)) {*/
        if (nvar > 0) {
	    prpars(stdout);	/* 17. List values vs time for varied pars */
	}
      }/* end drawparam */


/* Clear screen, draw axes, plot current param track */
void begingraph() {

	if (graphout == VT125) {
	    /*erase_text();*/  /* VT125PLOT.C Erase any text currently on screen */
	    /*set_scroll(21,24);*/	/* "   Text will appear as 3 lines at bottom */
	    /*vt125_par_plot(np,nframes,-1,GRID_OFF);	*//* VT125PAR.C (nf=-1) */
	    plotsw = 1;
	}
      }/* end begingraph()*/


/* Obtain <time,value> from terminal */
void gettimval(gtime,gvalue) int *gtime,*gvalue; {

/*	  Algorithm for VT-100 and VT125 */
	    if ((graphout == VT125) || (graphout == VT100)) {
/*            Ask for time in msec, see if legal time */
time1:                *gtime = get_time();		/* 20. */
                if (*gtime >= 0) {

/*                Ask for paramter value, see if legal value */
                    *gvalue = get_value( getpval());	/* 21.(15.) */
/*		  Typing a value less than -1 is request to correct time spec */
		    if (*gvalue < -1)    goto time1;
		  }
	    }
	  }


/* Erase from plot that part of line that will be replaced by new line seg */
/* i.e. for par `np', erase nframes, starting at frame `nf' */
void eraseparline() {
	if (graphout == VT125) {
	    /*vt125_par_plot(np,nframes,nf,ERASE_PAR);*/	/* VT125PAR.C */
	}
      }


/* Draw new line segment on graphical teminal */
/* i.e. for par `np', plot nframes, starting at frame `nf' */
void drawparline() {
	if (graphout == VT125) {
	    /*vt125_par_plot(np,nframes,nf,OVERLAY_PAR);*/	/* VT125PAR.C */
	}
      }

/* Clear graphical device of any plotted material if new request received */
/*  at top level */
void endgraph() {
	if ((graphout == VT125) && (plotsw == 1)) {
	/*    erase_plot();*/	/* VT125PLOT.C  Erase plotted material */
	/*    erase_text();*/   /* VT125PLOT.C  Erase textual material */
	/*    set_scroll(1,24);*/	/* VT125PLOT.C  Return to full screen  */
	    plotsw = 0;
	}
      }



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      6.                 G R A P H - P A R A M E T E R                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void graphparam() {

	if (graphout == VT125) {
	    if ( (np = get_par_name()) >= 0) {	/* 19. */
		begingraph();			/*  5. */
		printf("\tHit <CR> when LA-50 power switch on and paper inserted");
		printf("\n\t   (or 'a' for abort command): ");
		command = get_request();	/* 18. Wait for <CR> or 'a' */
		if (command == 'a') 
		   printf("\tAborted\n");
		else
			printf("\tWait until LA-50 printout completed before issuing next command.\n");
		/*la50_plot();	*/		/* xx. Transfer plot to LA-50 */
	    }
	}
	else {
	    printf("\tERROR: Can only graph to LA50 if 'klsyn93 -vt125'\n");
	}

      }



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      7.                  G R A P H - A L L - P A R S                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	static int nform;	/* Counter of number of formants in plot list */
	static int plotrange,plotsize,plottop;
        char plotname[30];
	static int ms_alt,nforfreqx;

int graphallpars() {

/*    See what day it is (will appear on plots) */
	time(&tvec); 	/* SYSTEM SUBROUTINE  Get curr time */
	strcpy(when, ctime(&tvec) );		/* SYSTEM   "   Convert time to text */

/*    Open output file of vectors */
	if (pname[0] == '\0')    askforname();	    /* 11. */
      if(post_open(pname)) {	/* POSTSCRIP.C */
/*    List values of all synthesis constants in a table printout */
	post_constants();		/* 8. */
/*    Plot all varying synthesis parameters except formants */
	post_variables();
/*    Plot formants together */
	printf("\n\tPlot all formant freqs ...\n");
	time_in_ms = 0;
	ngrids = 0;
	tgridmax = 0;
	timetrunc = 1000;
	for (nf=0; nf<nframes; nf++) {

/*	  Plot grid the first time needed */
	    if (time_in_ms >= tgridmax) {
      postspectgrid(firstname,(++ngrids),(samrat>>1),
		when, "FORMANT  FREQS (HZ)", 256, 200, totdur);
		tgridmax += 1000;
		timetrunc -= 1000;
	    }
/*	  Plot a set of formant values */
	    nform=0;
	    putforfreq("F1");		/*  7. */
	    putforfreq("F2");
	    putforfreq("F3");
	    putforfreq("F4");
	    putforfreq("F5");

/*	  Plot nasal pole/zero if not on top of each other */
	    nforfreqx = nform;
	    putforfreq("FNP");	/* First assume will be plotted */
	    putforfreq("FNZ");
/*	  Only plot nasal pole and zero during frames when not equal */
	    if ((forfreq[nforfreqx] != forfreq[nforfreqx+1])
	      && (forfreq[nforfreqx] != -forfreq[nforfreqx+1])) {
		if (forfreq[nforfreqx+1] > 0) {
/*		  Make zero always faint */
		    forfreq[nforfreqx+1] = -forfreq[nforfreqx+1];
		}
		if (cv[np] == VARRIED) {  /* Is fz variable */
		    nforfreqx += 2;
		}
	      }
	 post_freq_plot(forfreq, nforfreqx, timetrunc, ngrids); /* POSTGRID */
	 timetrunc += ms_frame;
	 time_in_ms += ms_frame;
	    if (ms_frame == 6) {	/* Assume really 6.4 */
		if (ms_alt == 0) {
		    ms_alt = 1;
		}
		else {
		    ms_alt = 0;
		    timetrunc++;
		    time_in_ms++;
		}
	    }
	}
	post_close();
	/*printf(
	"\n\n\tDone, use system command lpr %s to print on laser printer.\n",
	 pname);*/


         }/* could open postscript file */
     /* printf("poop\n");*/
      return(0);
      }


void putforfreq(psymb) char *psymb; {

	np = findnp(psymb);
/*    Put formant value in special array */
	forfreq[nform] = pdata[np][nf];
/*    If parallel formant amplitude is off/weak, make formant plot weak */
	np = findnp("A1V");
	if ((np < 0) || ((pdata[np][nf] < 30)
	  && (defval[findnp("CP")] == 1))) {	/* In parallel mode */
	    if (forfreq[nform] > 0) {
		forfreq[nform] = - forfreq[nform];
	    }
	}
/*    See if 'av' is off, if so see if 'af' also off */
	np = findnp("AV");
	if (pdata[np][nf] < 30) {
	    np = findnp("AF");
	    if (pdata[np][nf] < 30) {
/*	      Make faint if not excited */
		if (forfreq[nform] > 0) {
		    forfreq[nform] = - forfreq[nform];
		}
	    }
	}
	nform++;
      }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      8.       P O S T S C R I P T - P R I N T - C O N S T A N T S	 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void post_constants() {

	int ypos;
	char tex[500];

	    post_space(0,0,612,792);
	    post_typefont();


	    printf("\n\tPrint list of relevent synthesis constants...");
            sprintf(tex,
" TABLE 1. PARAMETERS SET TO A CONSTANT IN SYNTHESIZER CONFIGURATION, %s.\n",
	     firstname);
	    ypos = 700;
	    post_move(36,ypos);
	    post_text(0, tex,0,0,0,0,0,0,0,0);

            sprintf(tex, "     SYM  VALUE    DESCRIPTION\n");
	    ypos -= 30;
	    post_move(60,ypos);
	    post_text(0, tex,0,0,0,0,0,0,0,0);

            sprintf(tex,
	     "     ____________________________________________________________\n");
	    ypos -= 5;
	    post_move(60,ypos);
	    post_text(0, tex,0,0,0,0,0,0,0,0);

	    ypos -= 5;
	    syn_i = 0;
	    for (n=0; n<npars; n++) {
		if ((cv[n] == 'C') || (cv[n] == 'v')) {
		    if (relevent(symb[n][0],symb[n][1],symb[n][2])) { /* 8. */
	        	sprintf(tex, "      %s  %5d   %s\n",
			 &symb[n][0], defval[n], &definition[n][0]);
			ypos -= 20;
			post_move(60,ypos);
			post_text(0, tex,0,0,0,0,0,0,0,0);
			if (syn_i++ == 26) {
			    post_newpage();
			    ypos = 700;
			}
		    }
		}
	    }
	    post_newpage();
	    post_mainfont();
      }


/* Return 1 if should be printed, zero if not */

int relevent(char c1, char c2, char c3){

	int answer,nftemp,aftemp,cptemp;
	char symtemp[4];

	symtemp[0] = c1;
	symtemp[1] = c2;
	symtemp[2] = c3;
	answer = 1;	/* Assume relevent to print this par */
	nftemp = conval("NF");		/*  8. */
	aftemp = conval("AF");
	cptemp = conval("CP");
/*    Don't print speed quotient unless using Fant source */
	if ((c1 == 'S') && (c2 == 'Q')) {
	    if (conval("SS") != 3)    answer = 0;
	}
/*    Don't print formant f or bw if resonator is not excited at all */
	if ((c1 == 'B') || (c1 == 'F')) {
	    if (((c2 == '6') && (nftemp < 6))
	     || ((c2 == '5') && (nftemp < 5))
	     || ((c2 == '4') && (nftemp < 4)))    answer = 0;
	    if ((c1 == 'F') && (aftemp != 0))    answer = 1;
	}
/*    Don't print nasal pole/zero f & bw if they always cancel one another */
	if (((c1 == 'B') || (c1 == 'F'))
	 && (c2 == 'N')) {
	    if ((conval("FNP") == conval("FNZ"))
	     && (conval("BNP") == conval("BNZ")))   answer = 0;
	}
/*    Don't print tracheal pole/zero freq and bw if coupling is zero */
	if ((conval("FTP") == conval("FTZ"))
	 && (conval("BTP") == conval("BTZ"))) {
	    if (((c1 == 'F') || (c1 == 'B'))
	     && (c2 == 'T'))   answer = 0;
	}
/*    Don't print parallel formant amplitudes if af and cp are zero */
	if (c1 == 'A') {
	    if ((cptemp == 0) && (c3 == 'V')) {
		if ((c2 == '1') || (c2 == '2')
		 || (c2 == '3') || (c2 == '4'))  answer = 0;
	    }
	    if (cptemp == 0) {
		if ((c2 == 'N') || (c2 == 'T'))  answer = 0;
	    }
	    if ((aftemp == 0) && (c3 != 'V')) {
		if ((c2 == '2') || (c2 == '3') || (c2 == '4'))  answer = 0;
		if ((c2 == '5') || (c2 == '6') || (c2 == 'B'))  answer = 0;
	    }
	}
/*    Don't print parallel formant bandwidths if af is zero */
	if ((c1 == 'B') && (c3 == 'F')) {
	    if (aftemp == 0) {
		if ((c2 == '2') || (c2 == '3') || (c2 == '4'))  answer = 0;
		if ((c2 == '5') || (c2 == '6'))  answer = 0;
	    }
	}
/*    Don't print random seed and same burst if af and ah are zero */
	if ((aftemp == 0) && (conval("AH") == 0)) {
	    if (((c1 == 'R') && (c2 == 'S')) || ((c1 == 'S') && (c2 == 'B'))) {
		answer = 0;
	    }
	}
/*    Don't print gains unless differ from defaults */
	if (c1 == 'G') {
	    if ((c2 == 'V') && (conval(symtemp) == 60))    answer = 0;
	    if ((c2 == 'H') && (conval(symtemp) == 60))    answer = 0;
	    if ((c2 == 'F') && (conval(symtemp) == 60))    answer = 0;
	}
/*    Don't print 'ui', 'sr' and 'nf' unless differ from defaults */
	if ((c1 == 'U') && (c2 == 'I')) {
	    if (conval(symtemp) == 5)    answer = 0;
	}
	if ((c1 == 'S') && (c2 == 'R')) {
	    if (conval(symtemp) == 10000)    answer = 0;
	}
	if ((c1 == 'N') && (c2 == 'F')) {
	    if (conval(symtemp) == 5)    answer = 0;
	}
/*    Never print 'os' */
	if ((c1 == 'O') && (c2 == 'S'))   answer = 0;
/*    Don't print delta F1 and delta b1 if zero */
	if (conval(symtemp) == 0) {
	    if (((c1 == 'D') && (c2 == 'F'))
	     || ((c1 == 'D') && (c2 == 'B'))
	     || ((c1 == 'D') && (c2 == 'I'))
	     || ((c1 == 'F') && (c2 == 'L')))   answer = 0;
	}
	return(answer);
      }


/* Return default value if constant, return -ntemp if not */

int conval(symtemp) char *symtemp; {

	int ntemp;

	ntemp = findnp(symtemp);				/* 14. */
	if ((cv[ntemp] != 'C') && (cv[ntemp] != 'v')) {
	    return(-ntemp);
	}
	return(defval[ntemp]);
      }



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*	9.     P O S T S C R I P T - P L O T - V A R I A B L E S	 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int post_variables() {

	int npsav;
	char sym1sav,sym2sav;

	for (n=0; n<npars; n++) {

/*	  Plot all varied parameters except formants, which are done separately */
	    if ((cv[n] == 'V') && ((symb[n][0] != 'F')
	      || ((symb[n][1] < '1') && (symb[n][1] > '5')))) {
		sym1 = symb[n][0];
		sym2 = symb[n][1];/*	      Do not plot nasal pole-zero here, superimposed on formant plot */
		if ((sym1 == 'f') && ((sym2 == 'p')
		 || (sym2 == 'z')))    goto skipp;
		np = findnp(&symb[n][0]);			/* 14. */
		time_in_ms = 0;
		ngrids = 0;
		tgridmax = 0;
		timetrunc = 1000;
		plottop = 200;
		plotname[0] = toupper(sym1);
		plotname[1] = toupper(sym2);
		plotname[2] = ' ';	
		plotname[3] = '(';	
		plotname[6] = ')';	
		plotname[7] = '\0';	
		if (findtext('d','B')) {
		    plotname[4] = 'd';
		    plotname[5] = 'B';
		}
		else if (findtext('H','z')) {
		    plotname[4] = 'H';
		    plotname[5] = 'z';
		}
		else {
		    plotname[4] = ' ';
		    plotname[5] = ' ';	/* Should be '%', but Postscript objects */
		}
/*	      Find best plotting range, look for max val of par over time */
		plotrange = 1;
		for (nf=0; nf<nframes; nf++) {
		    if (plotrange < pdata[np][nf])   plotrange=pdata[np][nf];
		}
/*	      Convert f0 to Hz */
		if ((sym1 == 'f') && (sym2 == '0'))    plotrange /= 10;
		if (plotrange < 100) {
		    plotrange += 10;
		    plotrange = (plotrange/10)*10;
		    plotsize = 150;
		}
		else if (plotrange < 1000) {
		    plotrange += 100;
		    plotrange = (plotrange/100)*100;
		    plotsize = 200;
		}
		else {
		    plotrange += 1000;
		    plotrange = (plotrange/1000)*1000;
		    plotsize = 250;
		}
		printf("\n\tPlot %s ...", plotname);

		for (nf=0; nf<nframes; nf++) {

/*		  Plot grid every 1000 msec */
		    if (time_in_ms >= tgridmax) {
			postspectgrid(firstname,++ngrids,plotrange,when,
			 plotname, plotsize, plottop, totdur);	/* POSTGRID.C */
			tgridmax += 1000;
			timetrunc -= 1000;
		    }
/*		  Get value from data array */
		    val = pdata[np][nf];
/*		  F0 is in Hz times 10, convert to Hz */
		    if ((sym1 == 'f') && (sym2 == '0'))    val /= 10;
/*		  Make plotted point faint if AV = 0 */
		    npsav = np;
		    np = findnp("AV");			/* 14. */
		    if (pdata[np][nf] == 0) {
			val = -val;	/* Minus sign is a flag */
		    }
		    np = npsav;
/*		  Call plotter */
		    post_par_plot(val, timetrunc, ngrids);	/* xx. */
		    timetrunc += ms_frame;
		    time_in_ms += ms_frame;
		    if (ms_frame == 6) {	/* Assume really 6.4 */
			if (ms_alt == 0) {
			    ms_alt = 1;
			}
			else {
			    ms_alt = 0;
			    timetrunc++;
			    time_in_ms++;
			}
		    }
		}
		post_newpage();
skipp:		nf = nf;
	    }
	}
       /*printf("why doesn't this work\n");*/
       return(1);
      }/* end post variables */


/* Look for 2-letter sequence c1,c2 in the description of a control parameter */

int findtext(char c1,char c2){

	int nx;
	char cx;

	nx = 0;
	while ((cx= definition[n][nx++]) != '\0') {
	    if ((cx == c1) && (definition[n][nx] == c2))    return(1);
	}
	return(0);
      }



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     10.            S Y N T H E S I Z E - W A V E F O R M              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int synthesize() {

  /*	char *malloc();  */
	int samper;
        FILE *fp;

        printf("\tSynthesize waveform.\n");

	nsamtot = nframes * nsperframe;
	printf("\tnframes: %d nsperframe: %d nsamtot: %d.\n", nframes, nsperframe,nsamtot);
/*    Allocate core area to put synthetic waveform */
	synwave = (short *)malloc(nsamtot*2);
	if (!synwave) {
	    printf("ERROR in Klsyn.c: no room in system core for wave\n");
	    return(0);
	}

        if (ipsw != TTRUE) {
            initpars();         /* Stick default values in param tracks */
        }

/*    Reasure user */
        printf(
	 "\n\n\tPrint dot every 25 ms during synthesis to reassure user:\n\t");
	ms25 = (25 * samrat) / 1000;	/* Number samps in 25 ms */

/*    Main loop, for each frame of parameter data */
	nsamtot = 0;
        for (nf=0; nf<nframes; nf++) {

/*        Move pars into output parameter buffer */
	    nskip = 0;
            for (np=0; np<npars; np++) {
		if (cv[np] == FIXED_S) {
		    nskip++;
		}
                else if (cv[np] == VARRIED) {
                    pars[np-nskip] = pdata[np][nf];  /* Skip FIXED pars */
/* PRINT ORDERING OF PARS */
		    /*if (nf == 1)    printf("\tpars[%d] is %s\n", np-nskip,
			&symb[np][0]);*/
		  }
                else {
                    pars[np-nskip] = defval[np];

		    /*if (nf == 1)    printf("\tpars[%d] is %s\n", np-nskip,
			&symb[np][0]);*/

		  }
/*	      Move next value of f0, f0next into last par 
		if ((symb[np][0] == 'f') && (symb[np][1] == '0')) {
	            if (cv[np] == VARRIED) {
			pars[LOC_F0INTERP] = pdata[np][nf+1];
		      }
		    else    {pars[LOC_F0INTERP] = defval[np];}
		}

*/
	      }/* npars */




/*        Compute waveform chunk from paramter data */
            parwav(&synwave[nsamtot]);		/* in PARWAVEF.C */
	    nsamtot += nsperframe;
/*    Print a dot on screen every 25 ms to indicate program not dead */
	    if (dispt >= ms25) {
	        dispt -= ms25;
	        printf( ".");
	    }
	    dispt += nsperframe;
	}

	if (sigmx <= 0)    sigmx = 1;
        printf(
"\n\n\tMax output signal (overload if greater than 0.0 dB) is  %3.1f dB  \n",
	 dBconvert(sigmx));					/* in PARWAVEF.C. */
	printf(
	 "\tTotal number of waveform samples = %d\n\n", nsamtot);

/*    Save array in output file */
	samper = 1000000 / samrat;
        if (putWavWaveform(wname, synwave, (float) samrat, nsamtot, SWAP) 
	    == -1)
	  printf("Error writing wavefile %s\n", wname);
	else
	  printf("\tWaveform saved in file '%s'\n", wname);

      /* free synwave */
      if(synwave != NULL) free(synwave);
      	    return(1);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     11.                   A S K - F O R - N A M E                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void askforname() {

        char garb;

        printf("\n\tFirst name for synthesis output file");
        printf(" (20 chars max): ");
        for (n=0; n<23; n++) {
            scanf("%c", &garb);
            if (garb != '\n') {
                firstname[n] = garb;
            }
            else {
                break;
            }
        }
        firstname[n] = '\0';
	if (n == 0) {
	  printf("\n\tERROR: Name has no chars, NO FILE WAS CREATED\n");

	}
	if (n > 20) {
	    firstname[20] = '\0';	/* Max # chars is 20 */
	    printf("\n\tERROR: Name too long, truncated\n");
       
	}
        if (n!=0) makefilenames();
      }


void makefilenames() {
int p;
char temp[200];

   /* read from specified directory */
      sprintf(dname,"%s.doc",firstname);

    /* now strip path, write to directory user is in */
#ifdef VMS

    for(p = strlen(firstname) - 1; p >= 0; p --)
         if(firstname[p] == ']')  break;

   strcpy(temp,&firstname[p+1]);

#else
/* assume unix*/

    for(p = strlen(firstname) - 1; p >= 0; p --)
         if(firstname[p] == '/')  break;

   strcpy(temp,&firstname[p+1]);
#endif

      strcpy(firstname,temp);
      sprintf(wname,"%s.wav",firstname);		/* 25. */
      sprintf(pname,"%s.ps" ,firstname);
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     12.        I N I T - P A R A M E T E R - T R A C K S              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void initpars() {	/* Put default value in all parameter time functions */

        ipsw = TTRUE;
        nvar = 0;
        for (np=0; np<npars; np++) {
            if (cv[np] != FIXED_S) {
                if (cv[np] == VARRIED) {
                    cv[np] = VARIABLE;
                }
                clearpar();			/* 12. */
            }
        }
/*      printf("\n\tDefault values placed in variable param buffers\n\n"); */
      }


void clearpar() {	/* Put default value in parameter = np time function */

        for (nf=0; nf<nframes; nf++) {
            val = defval[np];
            setpval();			/* 16. Put value in time function */
        }
      }



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*      13.                  D R A W - L I N E                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void draw_line() {

        float dval,dnf;
        int nf1,nf2,val1,nfinc;

/*    See if line to be drawn forward in time or backward */
        if (nf > lastnf) {
            nfinc = 1;
        }
        else {
            nfinc = -1;
        }

/*    Linear interpolation */
        nf1 = lastnf;
        nf2 = nf;
        val1 = lastval;
        dval = (val - lastval) * 10;
	if ((dval>0)&&(val*lastval>1)) dval +=9;/*93 don't round between 0 and 1*/
        dnf = (nf2 - nf1) * 10;
        for (nf=nf1; nf!= nf2; nf+=nfinc) {
            val = val1 + ((dval*(nf-nf1))/dnf);
            pdata[np][nf] = val;
        }
        nf = nf2;
        val = val1 + dval/10;
        pdata[np][nf] = val;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     14.                  D E C O D E - P A R A M                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int findnp(syms) char *syms; {

        int nxx;
	char *syms1;

        nxx = 0;
        for (nxx=0; nxx<npars; nxx++) {
	    syms1 = syms;
            if (*syms1++ == symb[nxx][0]) {
		if (*syms1++ == symb[nxx][1]) {
		    if (*syms1 == symb[nxx][2]) {
	                return(nxx);
		    }
		}
            }
        }
	printf(
	"\n\tERROR in findnp() of KLSYN93: can't find par named '%s'\n", syms);
        return(-1);
}

int decodparam() {		/* Find np corresponding to <symb[np][]> */

        int nxx;

        nxx = 0;
        for (nxx=0; nxx<npars; nxx++) {
            if ((sym1 == symb[nxx][0]) && ( sym2 == symb[nxx][1])) {
                return(nxx);
            }
        }
        return(-1);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     15.                      G E T P V A L                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int getpval() {

        if (np >= MAX_VAR_PARS) {
            printf("  ERROR: np=%d > max=%d in getpval(), return 0",
              np, MAX_VAR_PARS);
            return(0);
        }
        if (nf >= MAX_FRAMES) {
            printf("  ERROR: Time frame nf=%d > max=%d in getvpar(), return 0",
              nf, MAX_FRAMES);
            return(0);
        }
	if (cv[np] != 'V')    return(defval[np]);
        return(pdata[np][nf]);
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     16.                       S E T P V A L                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void setpval() {	/* 16. Put val in time function np at frame nf */

        if (np >= MAX_VAR_PARS) {
            printf("  ERROR: np=%d > max=%d in setpval(), ignore request\n",
              np, MAX_VAR_PARS);
        }
        else if (nf >= MAX_FRAMES) {
            printf("  ERROR: Time frame nf=%d > max=%d in setvpar(), ignored\n",
              nf, MAX_FRAMES);
        }
        else {
            pdata[np][nf] = val;
	  
	  }
      }




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     17.         P R I N T - P A R A M E T E R - T R A C K S           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void prpars(dev) FILE *dev; {

    if (nvar == 0) {
        fprintf(dev, "\tNo parameters are varied\n");
    }
    else {
	prparnames(dev);		/* 17. */
        for (n=0; n<nframes; n++) {
	    fprintf(dev, "%4d", ms_frame*n);
            for (npar1=0; npar1<npars; npar1++) {
                if (cv[npar1] == VARRIED) {
                    fprintf(dev, "%5d", pdata[npar1][n]);
                }
            }
            fprintf(dev, "\n");
        }
        fprintf(dev, "\n");
    }
}

void prparnames(dev) FILE *dev; {

        fprintf(dev, "\n\nVaried Parameters:\n");
	if (nvar == 0) {
	    fprintf(dev, "    none");
	}
	else {
	    fprintf(dev, "time");
            for (npar1=0; npar1<npars; npar1++) {
                if (cv[npar1] == VARRIED) {
                    fprintf(dev, "   %s", &symb[npar1][0]);
		}
            }
        }
        fprintf(dev, "\n");
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     18.                     G E T - R E Q U E S T                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

char get_request() {

        char req, garb;

        printf("> ");


	    scanf("%c", &req);
            if(req != '\n'){
                   garb = 's';
                  while(garb != '\n'){scanf("%c",&garb);}
		 }
	printf("%c\n", req);
        return(req);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     19.                     G E T - P A R - N A M E                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int get_par_name() {

    char garb,symbls[4];

    np = -1;
    while (np == -1) {
        if (user == NOVICE) {
            printf("\n    2-char name for desired param (or <CR> to quit): ");
        }
        else {
            printf("\n    Par: ");
        }

        symbls[0] = '\0';
        symbls[1] = '\0';
        symbls[2] = '\0';
        scanf("%c", &garb);
        if (garb != '\n') {
            symbls[0] = toupper(garb);
            scanf("%c", &garb);
            if (garb != '\n') {
                symbls[1] = toupper(garb);
                scanf("%c", &garb);
                if (garb != '\n') {
		    symbls[2] = toupper(garb);
                    while (garb != '\n') {
                       scanf("%c", &garb);
                    }
                }
             }
	  }

/*    Delete these lines when decodparam() is deleted */
	sym1 = symbls[0];
	sym2 = symbls[1];
	sym3 = symbls[2];

        if ((symbls[0] == '\0') || (symbls[0] == 'q')) {
            return(-2);         /* Request to quit */
        }
        if ((np = findnp(symbls)) == -1) {		/* 14. */
	    if (symbls[0] != '?') {
	        printf("\tERROR: '%s' is not a legal parameter\n",
		 symbls);
	    }
        }
    }
    return(np);
  }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     20.                      G E T - T I M E                          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int get_time() {

        int nms;

	if (user == NOVICE) {
repett:     printf("\n\tTime in msec (limits are 0 to %d), or <CR> to quit: ",
              totdur);
        }
        else {
            printf("\tTime:  ");
        }
        nms = get_digits(stdin);		/* 22. Time in msec */
        if (nms < 0) {
            if (nms == -1) {
                return(-1);                     /* Request to quit */
            }
	    if (nms != -2) {
		printf("\t  ERROR in requested value\n");
	    }
            goto repett;
        }
        nms = nms / ms_frame;  /* Convert to number of frames */
	if (nms == nframes) {
	    nms--;		/* Assume user meant last possible frame */
	}
        else if (nms > nframes) {
            printf("\t  ERROR in requested value\n");
            goto repett;
        }
        return(nms);
      }



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     21.                     G E T - V A L U E                         */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int get_value(arg) int arg; {

        if (user == NOVICE) {
gv1:      printf("\tValue (was %d, limits are %d to %d): ",
             arg, minval[np], maxval[np]);
        }
        else {
            printf("\tValue (was %d): ", arg);
        }
        val = get_digits(stdin);		/* 22. */
	if (val < -1) {
	    printf(
	 "\t  ERROR: Negative values are meaningless, retype time and value\n");
	    return(val);
	}
	val = checklimits();			/* 21. */
	if (val < 0) {
	   goto gv1;	/* checklimits sets to -2 if user doesn't override */
	}
	return(val);
}



int checklimits() {

ch1:
        if (val > maxval[np]) {
            printf("\t  ERROR: %d is bigger than maxval[%s] = %d\n",
              val, &symb[np][0], maxval[np]);
            printf("\t  Override this suggested maximum [y] [n] ");
            command = get_request();		/* 18. */
            if (command == 'y') {
                maxval[np] = val;
		printf("\t\tmaxval[%s] is now %d\n", 
		 &symb[np][0], maxval[np]);
            }
            else {
		printf("\t  Please type requested value again\n");
		return(-2);
            }
        }
        if (val < minval[np]) {
	    if (val == -1) {
		val = getpval();		/* 15. */
		printf("\t  Use current value == %d\n", val);
		goto ch1;
	    }
            printf("\t  ERROR: %d is smaller than minval[%s] = %d\n",
              val, &symb[np][0], minval[np]);
            printf("\t  Override this suggested minimum [y] [n] ");
            command = get_request();
            if (command == 'y') {
                minval[np] = val;
		printf("\t\tminval[%s] is now %d\n", 
		  &symb[np][0], minval[np]);
            }
	    else {
		printf("\t  Please type requested value again\n");
		return(-2);
	    }
        }
        return(val);
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     22.                     G E T - D I G I T S                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int get_digits(dev) FILE *dev; {

        int dig,nn;
        char digitarray[9],c;

/*    Strip off leading white space */
        do {
            fscanf(dev, "%c", &c);
        } while ((c == ' ') || (c == '\t'));
        if (c == '\n') {
            return(-1);
        }

/*    Read digits and place in array digitarray[] */
        nn = 0;
        while ((c == '-') || ((c >= '0') && (c <= '9'))) {
            if (nn >= 8) {
                printf("\t  ERROR: Digit string too long, value set to 0\n");
                return(0);
            }
            digitarray[nn++] = c;
            fscanf(dev, "%c", &c);
        }
        if ((c != ' ') && (c != '\t') && (c != '\n')) {
            skip_line(stdin);               /* Gobble up CR */
            if (c == 'q') {
                return(-1);
            }
	    if (c == '?') {
		return(-2);
	    }
            printf("\t  ERROR: Alphabetic char '%c' in a digit string\n", c);
	    return(-2);
        }
        digitarray[nn] = '\0';

/*    Convert from ascii string to integer */
        dig = atoi(digitarray);
        return(dig);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     23.               G E T - N O N W H I T E - C H A R               */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

char get_nonwhite_char(dev) FILE *dev; {

        char c;

/*    Strip off leading whitespace */
        do {
            fscanf(dev, "%c", &c);
        } while ((c == ' ') || (c == '\t') || (c == '\n'));
        return(c);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     24.                      S K I P - L I N E                        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void skip_line(dev) FILE *dev; {

        char c;

        do {
            fscanf(dev, "%c", &c);
        } while (c != '\n');
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     25.                    M E R G E - S T R I N G S                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void mergestring(char *fname, char *sname, char *oname){

        int nfirst;

/*loopn:*/  n = 0;
/*    See if name specified, if not, use 'default name' */
        if (fname[0] == '\0')    goto illnam;
        while ((n < 40) && (fname[n] != '\0')) {
            oname[n] = fname[n];
            n++;
        }
        nfirst = n;
        while ((n < 40) && (sname[n-nfirst] != '\0')) {
            oname[n] = sname[n-nfirst];
            n++;
        }
        if (n < 40) {
            oname[n] = '\0';
        }
        else {
illnam: 	oname[39] = '\0';
	printf(
 "\n\tERROR: More than 40 chars in '%s' in mergestring(), KLSYN93.C\n",
              firstname);
	    exit(1);
        }
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     26.                    C O P Y - S T R I N G                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void copystring(istring,ostring) char istring[],ostring[]; {

        n = 0;
        do {
            ostring[n] = istring[n];
        } while (istring[n++] != '\0');
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     27.             H E L P - L I S T - A R G U M E N T S                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void helpa() {
        printf("\n    Text interface only\n");
        printf("\n    Required arguments for KLSYN93:\n");
        printf("\n    Name of '.doc' file to be read\n");
        printf("\n                 or\n");
        printf("\n    -d  start with DEFAULT parameters\n");
        printf("\n");
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*     28.             H E L P - A C T - O N - R E Q U E S T             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void helpr() {

        printf("\n    Legal KLSYN93 commands:\n");
        printf("\tp\tPRINT set of synthesis parameter default values\n");
        printf("\tc\tCHANGE default value for a synthesis parameter\n");
        printf("\n\td\tCHANGE synthesis parameter time function\n");
	printf("\tG\tGRAPH on Laser printer all varied pars as a function of time\n");
        printf("\n\ts\tSYNTHESIZE waveform file, save everything, quit");
        printf("\n\tS\tSAVE current parameters as .doc file (no synthesis), quit");
        printf("\n\tq\tQUIT program, save nothing\n");
        printf("\t?\tHELP, please list legal KLSYN93 commands\n");
        printf("\n");
}




















