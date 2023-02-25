/*
$Log: syngrid.c,v $
Revision 1.2  1998/06/09 00:45:41  krishna
Added RCS header.
 */



/* [klatt.klsyn]	P O S T G R I D . C	     D.H. Klatt	*/

/*	Make a grid for plotting synthesis parameters
 *	 on Postscript laser printer/plotter */

/*	This program is used by KLSYN,
 *	 modified version of [klatt.specto]postgrid.c, borrowed 3-Sep-87 */

#include "syngrid.h"
#include "synscrip.h"
#include <ctype.h>
#include <string.h>


int postspectgrid(name,pagenumber,freqmax,pdate,xlabel,xsiz,xtop,tmsec)
char *name; int pagenumber,freqmax; char *pdate,*xlabel; int xsiz,xtop,tmsec;
{
	
	xsize = xsiz;
	xpostop = xtop;
	xminpost = xpostop + xsize;
	frmax = freqmax;
	xpage = 0;
	xpostop += xpage;
	if ((pagenumber != curpage) && (pagenumber != 1)) {

		post_newpage();
	}
	curpage = pagenumber;
		
	post_setblack();

	post_labelplot(name,pdate);

	post_grid(pagenumber,freqmax,xlabel,tmsec);

	return(0);
}



int post_labelplot(name,pdate)
 char *name; char *pdate; {

klpath[0] = '\0';

/*    Define coordinates (xmin, ymin, xmax, ymax) */
	post_space(0, 0, XPOSTSIZE*SCALE, 792*SCALE);

/*    Find current directory */
        
#ifdef VMS
       strcpy(klpath,getenv("PATH")); 
#else

	strcpy(klpath,getenv("HOST"));
        strcat(klpath,":");
	strcat(klpath,getenv("PWD"));

#endif

/*    Raise case and then print path name */
	i = 0; j = 0;
	/*while (klpath[++j] != '[');*/
	while (klpath[j] != '\0') {
	    label[i] = toupper(klpath[j]);
	    i++; j++;
	}
	label[i] = '\0';
	post_move((xpage+100)*SCALE,64*SCALE);
	post_text(90, label,0,0,0,0,0,0,0,0);

/*    Raise case and then print input name */
	i = 0;
	while (name[i] != '\0') {
	    label[i] = toupper(name[i]);
	    i++;
	}
	label[i] = '\0';
	post_move((xpage+100)*SCALE,(760-(10*strlen(label)))*SCALE);
	post_text(90, label,0,0,0,0,0,0,0,0);

/*    Raise case and then print date plot made */
	i = 0; j = 0;
	while (pdate[++j] != ' ');
	while (pdate[j] != '\n') {
	    label[i] = toupper(pdate[j]); 
/*	  Skip over time portion of date */
	    if (label[i] == ':') {
		i -= 3;
		j += 5;
	    }
	    i++; j++;
	  }
	label[i] = '\0';
	post_move((xpage+100)*SCALE,370*SCALE);
	post_text(90, label,0,0,0,0,0,0,0,0);
	return(0);
 }



int post_grid(pagenumber,freqmax,flabel,tms)
 int pagenumber,freqmax; char *flabel; int tms; {

	yminpost = 125;
	yposmax = yminpost + (60 * ((tms + 99) / 100)) + 5;
/* OUT	if (yposmax <= 695)    yposmax += 60; */	/* WAS yposmax = 755; */
	npts_frame = 3;
	nframe_sec = 200;
	npts_tick = npts_frame * nframe_sec / 10;	/* Tick every 100 ms */
printf("\n\tStill ok ...\n");
/*    Draw box */
	post_point((xpostop-10)*SCALE,(yminpost-10)*SCALE);
	post_cont((xpostop-10)*SCALE,(yposmax+5)*SCALE);
	post_cont((xpostop+xsize+10)*SCALE,(yposmax+5)*SCALE);
	post_cont((xpostop+xsize+10)*SCALE,(yminpost-10)*SCALE);
	post_cont((xpostop-10)*SCALE,(yminpost-10)*SCALE);
printf("\n\tStill ok ...\n");
/*    Label y axis */
	if       (freqmax <  100)   delfreq = 10;
	 else if (freqmax <= 200)   delfreq = 20;
	 else if (freqmax <= 500)   delfreq = 50;
	 else if (freqmax <= 1000)  delfreq = 100;
	 else if (freqmax <= 2000)  delfreq = 200;
	 else if (freqmax <= 5000)  delfreq = 500;
	 else                       delfreq = 1000;
	for (f=0; f<= freqmax; f+=delfreq) {
		xpos = xpostop + xsize;
		xpos -= (f * xsize) / freqmax;
		plefttick(xpos,yminpost);
/*	      Label every other tick */
		if (((f / (2*delfreq))*(2*delfreq)) == f) {
		    pleftlabel(f,xpos+5,yminpost-2);
		}
		prighttick(xpos,yposmax);
	}
	xpos = xpostop + (xsize>>1) + 28;
	ypos = yminpost - 50;
	if (flabel[1] == 'O') {
	    xpos += 50;	/* FORMANT FREQS (Hz) */
	    ypos = yminpost - 54;
	}
	post_move(xpos*SCALE,ypos*SCALE);
	post_text(180, flabel,0,0,0,0,0,0,0,0);

/*    Label time axis, assume 3 pts per 5 ms */
	for (ypos=yminpost; ypos<=yposmax; ypos+=npts_tick) {
	    t = ((ypos-yminpost) * 1000) / (npts_frame * nframe_sec);
	    t += 1000 * (pagenumber - 1);
	    pbottomtick(ypos,t);
	    ptoptick(ypos,t);
	}
	post_move((xpostop+xsize+44)*SCALE,(((yminpost+yposmax)>>1)-46)*SCALE);
	post_text(90, "TIME (MS)",0,0,0,0,0,0,0,0);
		return(0);
}


int plefttick(x,y) int x,y; {

	post_point(x*SCALE,(y-5)*SCALE);
	post_cont(x*SCALE,(y-10)*SCALE);		/* Draw tick mark */
		return(0);
}


int pleftlabel(value,x,y) int value,x,y; {

	int yoffset;

	if (value < 10)        yoffset = 19;
	else if (value < 100)  yoffset = 28;
	else if (value < 1000) yoffset = 37;
	else                   yoffset = 46;
	post_move((x+1)*SCALE,(y-yoffset)*SCALE);
	sprintf(label, "%d", value);
	post_text(90, label,0,0,0,0,0,0,0,0);
		return(0);
}


int prighttick(x,y) int x,y; {

	post_point(x*SCALE,y*SCALE);
	post_cont(x*SCALE,(y+5)*SCALE);		/* Draw tick mark */
		return(0);
}


int pbottomtick(y,time) int time,y; {

	post_point((xpostop+xsize+5)*SCALE,y*SCALE);
	post_cont((xpostop+xsize+10)*SCALE,y*SCALE);
	post_move((xpostop+xsize+27)*SCALE,(y-18)*SCALE);
	sprintf(label, "%4d", time);
	if (y < 740)	post_text(90, label,0,0,0,0,0,0,0,0);
		return(0);
}


int ptoptick(y,time) int time,y; {
	post_point((xpostop-5)*SCALE,y*SCALE);
	post_cont((xpostop-10)*SCALE,y*SCALE);
		return(0);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		         P O S T - F R E Q - P L O T			 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Take a list of formant frequencies, and plot them */
/*   Plot faintly if negative */

int post_freq_plot(infreq, ninfreq, time_ms, ngrid)
	int infreq[];		/* Input formant frequency list */
	int ninfreq;		/* Number of input formant freqs */
	int time_ms;		/* Time in ms of window center, 0 to 1000 */
	int ngrid;		/* Number of the grid on which to put spect */
{

	extern int xminpost;	/* Position of f=0 in postscript plot */
	extern int yminpost;	/* Position of t=0 in postscript plot */
	extern int srate[],nwave;
	int n,m,xloc,yloc,faintsw;

	for (n=0; n<ninfreq; n++) {
	    m = infreq[n];
	    faintsw = 0;
	    if (m < 0) {
		faintsw++;
		m = -m;
	    }

/*        Assume time axis of 3 points per 5 ms */
	    yloc = (yminpost*SCALE) + ((time_ms * 6 * SCALE) / 10);
/*	  Assume freq axis of 0 to srate[nwave]/2 kHz is xsize points */
	    xloc = (xminpost*SCALE) - ((m * xsize * SCALE) / frmax);
	    if (faintsw == 0) {		/* Draw dark dot */
		post_lwidth(3);
		post_point(xloc-SCALE,yloc);
		post_cont(xloc+(2*SCALE),yloc);
	    }
	    else {
		post_lwidth(2);	/* Draw light dot */
		post_point(xloc-SCALE,yloc);
		post_cont(xloc+SCALE,yloc);
	    }
	}
		return(0);
}



int post_par_plot(value, time_ms, ngrid) int value,time_ms,ngrid; {

	extern int xminpost,yminpost;
	int xloc,yloc,faintsw;

	faintsw = 0;
	if (value <= 0) {  /* Neg number is a flag, zero should be faint too */
	    faintsw++;
	    value = -value;
	}

/*    Assume time axis of 3 points per 5 ms */
	yloc = (yminpost*SCALE) + ((time_ms * 6 * SCALE) / 10);
/*    Assume freq axis of 0 to srate[nwave]/2 kHz is xsize points */
	xloc = (xminpost*SCALE) - ((value * xsize * SCALE) / frmax);
	if (faintsw == 0) {		/* Draw dark dot */
	    post_lwidth(3);
	    post_point(xloc-SCALE,yloc);
	    post_cont(xloc+(2*SCALE),yloc);
	}
	else {
	    post_lwidth(2);	/* Draw light dot */
	    post_point(xloc-SCALE,yloc);
	    post_cont(xloc+SCALE,yloc);
	}
		return(0);
}
