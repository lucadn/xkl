/*
$Log: synscrip.c,v $
Revision 1.2  1998/06/09 00:45:41  krishna
Added RCS header.
 */


/* P O S T S C R I P . C		D. H. KLATT			 */

/*	Copyright 1987 by Dennis H. Klatt */

/* EDIT HISTORY *
 * 0001  8-Jul-87 DK	Initial creation in directory [klatt.klspec]
 * 0002 21-Jul-87 DK	Add post_box()
 * 0003 14-Aug-87 DK	Add post_setblack()
 * 0004 27-Aug-87 DK	Make thinest line more visible
 * 0005 11-Sep-87 DK	Add post_offset()
 * 0006  5-Mar-88 DK	Convert to floating point for better resolution
 * 0007 10-Mar-88 DK	Add TypeFont = /Courier
 * 0008 11-Mar-88 DK	Make faintest and darkest line widths wider
 */

#include <stdio.h>
#include "synscrip.h"



/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Open file								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_open(arg1) char *arg1; {			/* Turn on graphics mode */
	if ((dpost = fopen(arg1, "w")) == NULL) {
	    printf("ERROR: Can't open Postscript laser printer file %s\n",
	     arg1);
	    /*exit(1);*/
            return(0);
     
	}
	else {
/*	  Set default font and size */
	    fprintf(dpost,"%c!PS\n",'%');
	    fprintf(dpost,
	    "/MainFont\n  /Helvetica findfont 16 scalefont def\n");
	    fprintf(dpost,
	    "/SmallFont\n  /Helvetica findfont 12 scalefont def\n");
	    fprintf(dpost,
	    "/TypeFont\n  /Courier findfont 12 scalefont def\n");
	    fprintf(dpost,
	    "MainFont setfont\n");
/*	  Select default line width */
	    post_lwidth(2);			     /* Medium (1-point) width */
/*	  Waveform looks better with rounded corners */
	    fprintf(dpost,"1 setlinejoin\n");
	    newpath = 0;
	    lastact = OPEN;
          return(1);
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Close file								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_close() {
	post_newpage();
	fclose(dpost);
	lastact = CLOSE;
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Print Contents of Current Page					*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_newpage() {
	do_stroke();
	fprintf(dpost,"showpage\n");		/* Force page output */
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Line width								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_lwidth(arg) int arg; {		/* weak=1, mid=2, max=3 */

	float lwidth;

	if (arg == 1)  lwidth = 0.3;
	else if (arg == 2)  lwidth = 1.0;
	else           lwidth = 2.0;
	do_stroke();
	fprintf(dpost,"%3.1f setlinewidth\n", lwidth);
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Character size							*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_csize(arg1) int arg1; {	/* Set character size to arg1/72 of an inch */
	do_stroke();
	fprintf(dpost, "/Helvetica findfont %d scalefont\nsetfont\n", arg1);
	return(0);
}

int post_mainfont() {	/* Set character size to 16/72 of an inch */
	do_stroke();
	fprintf(dpost, "MainFont setfont\n");
	return(0);
}

int post_smallfont() {	/* Set character size to 12/72 of an inch */
	do_stroke();
	fprintf(dpost, "SmallFont setfont\n");
	return(0);
}

int post_typefont() {	/* Set character size to 12/72 of an inch, Courier */
	do_stroke();
	fprintf(dpost, "TypeFont setfont\n");
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Offset plot to make room for 3-hole punch				*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_offset(sixtyths) int sixtyths; {

	do_stroke();
	fprintf(dpost, "%d 0 translate\n", sixtyths);
	return(0);
}



/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Define plotting area						*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_space(arg1,arg2,arg3,arg4)
 int arg1,arg2,arg3,arg4; {				/* Pretend plotting */

	do_stroke();
	xscale = POSTXMAX / (float) (arg3 - arg1);	/* area is from lower */
	yscale = POSTYMAX / (float) (arg4 - arg2);	/* left <arg1,arg2> */
	xhome = arg1;					/* to upper right */
	yhome = arg2;					/* <arg3,arg4> */
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Draw a box								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_box(arg1,arg2,arg3,arg4,arg5)
 int arg1;	/* xmin */
 int arg2;	/* ymin */
 int arg3;	/* xmax */
 int arg4;	/* ymax */
 int arg5;	/* Grayness, 0=white, 8=black */
 {
	float gray;

	post_point(arg1,arg2);	/* Define a box */
	post_cont(arg1,arg4);
	post_cont(arg3,arg4);
	post_cont(arg3,arg2);
	fprintf(dpost, "closepath\n");
	gray = (8.0 - arg5) * 0.125;	/* Set grayness, 0=black 1=white */
	fprintf(dpost, "%4.2f setgray fill\n", gray);
	return(0);
}

int post_setblack() {
	fprintf(dpost, "0 setgray\n");
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Move, ready to draw text only					*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_move(arg1,arg2) int arg1,arg2; {	/* Move to <arg1,arg2>, do not plot */
	do_stroke();
	xactual = mapx(arg1);
	yactual = mapy(arg2);
	fprintf(dpost, "%4.2f %4.2f moveto\n", xactual, yactual);
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Move, ready to draw lines only					*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_point(arg1,arg2) int arg1,arg2; {	/* Move to <arg1,arg2> and display */
	do_stroke();
	do_newpath();
	xactual = mapx(arg1);
	yactual = mapy(arg2);
	fprintf(dpost, " %4.2f %4.2f moveto\n", xactual, yactual);
	lastact = POINT;
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Incremental draw line						*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_cont(arg1,arg2) int arg1,arg2; {	/* Draw line from current location */

	do_newpath();
/*    Do a moveto if beginning of newpath */
	if (lastact == NEWPATH) {
	    fprintf(dpost, " %4.2f %4.2f moveto\n", xactual, yactual);
	}
	dxactual = -xactual;
	dyactual = -yactual;
	xactual = mapx(arg1);		/*  to <arg1,arg2> */
	yactual = mapy(arg2);
	dxactual += xactual;
	dyactual += yactual;
/*    Make sure to move at least a little bit (DANGEROUS) */
	if ((dxactual+dyactual) == 0) {
	    xactual++;
	    dxactual++;
	}
	fprintf(dpost, " %4.2f %4.2f rlineto\n", dxactual, dyactual);
	lastact = CONT;
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Print text								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int post_text(direction, format, a,b,c,d,e,f,g,h)	/* Print a text string at cur loc */
    int direction;	/* Orientation in degrees, 0=horizontal, 90=vertical */
    char *format;
    int a,b,c,d,e,f,g,h;
{
	char charsave;
	int n;

	do_stroke();
	if (direction != lastrotate) {
	    fprintf(dpost,"%d rotate\n", direction);
	    lastrotate = direction;
	}

	sprintf(flabel, format, a,b,c,d,e,f,g,h);
/*    Count characters in flabel[] (laser printer can't handle > 80 chars) */
	n = 0;
	while (flabel[n++] != '\0');
	if (n > 70) {
	    charsave = flabel[70];
	    flabel[70] = '\0';
	    fprintf(dpost,"(%s) show\n", flabel);
	    flabel[70] = charsave;
	    fprintf(dpost,"(%s) show\n", &flabel[70]);
	}
	else	fprintf(dpost,"(%s) show\n", flabel);
/*    Due to assumption that only text is rotated (true of klspec, false of */
/*    Postscript, return to zero orientation after each rotate (thus cumulative */
/*    rotates won't work here */
	if (direction != 0) {
	    fprintf(dpost,"%d rotate\n", -direction);
	    lastrotate = 0;
	}
	lastact = TEXT;
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Start drawing new path						*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int do_newpath() {
	if (newpath == 0) {
	    fprintf(dpost,"newpath\n");
	    newpath = 1;
	    lastact = NEWPATH;
	}
	return(0);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   End of path, actually draw it					*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

int do_stroke() {
	if (newpath == 1) {
	    fprintf(dpost,"stroke\n");
	    newpath = 0;
	    lastact = STROKE;
	}
	return(0);
}
