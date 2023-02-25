/* $Log: lspscrip.c,v $
 * Revision 1.3  1998/06/06 21:05:16  krishna
 * Added RCS header.
 * */


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
 */

#include <stdio.h>
#include <stdlib.h>
#include "lspscrip.h"


FILE *dpost,*fopen();


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Open file								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_open(char *arg1){			/* Turn on graphics mode */
	if ((dpost = fopen(arg1, "w")) == NULL) { 
	    printf("ERROR: Can't open Postscript laser printer file %s\n",
	     arg1);
	    exit(1);
	}
	else {
/* Debug Timer: */
/*	fprintf (dpost,"/time usertime def\n");
	fprintf (dpost,"/elapsed 20 string def\n");		*/

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
/*	  Waveform looks better with rounded corners */
	    fprintf(dpost,"1 setlinejoin\n");
	    newpath = 0;
	    lastact = OPEN;
	  fprintf(dpost,"%%Command abbreviations:\n");
	  fprintf(dpost,"/n /newpath   load def\n");
	  fprintf(dpost,"/l /rlineto   load def\n");
	  fprintf(dpost,"/m /moveto    load def\n");
	  fprintf(dpost,"/c /closepath load def\n");
	  fprintf(dpost,"/s /stroke    load def\n");
	  fprintf(dpost,"/g /setgray   load def\n");
	  fprintf(dpost,"/r /grestore  load def\n");
	  fprintf(dpost,"/t /translate load def\n");
	  fprintf(dpost,"/ro /rotate   load def\n");
	  fprintf(dpost,"/w /setlinewidth load def\n");

	  fprintf(dpost,"/setF{ \n");
	  fprintf(dpost,"    currentscreen \n");
	  fprintf(dpost,"    4 -2 roll pop \n");
	  fprintf(dpost,"    3 1 roll setscreen \n");
	  fprintf(dpost,"    } bind def \n");
	  fprintf(dpost,"75 setF  %% set screen to 75 lpi \n");
	  fprintf(dpost,
	  "/IM {  %% Scanline image routine, arg: scanlength in samples\n");
	  fprintf(dpost,"   dup /DataString exch 2 idiv string def\n");
	  fprintf(dpost,"   1 4 [1 0 0 1 0 0]\n");
	  fprintf(dpost,"   {currentfile DataString readhexstring pop\n");
	  fprintf(dpost,"   }bind image\n");
	  fprintf(dpost,"}bind def\n");

/* Box args: linewidth width height LLx LLy */
	  fprintf(dpost,"/Box %% args: linewidth width height LLx LLy\n");
	  fprintf(dpost,"{  newpath moveto\n");
	  fprintf(dpost,"   1 index 0 rlineto\n");
	  fprintf(dpost,"   0 exch rlineto\n");
	  fprintf(dpost,"   neg 0 rlineto\n");
	  fprintf(dpost,"   closepath\n");
	  fprintf(dpost,"   gsave setlinewidth stroke grestore\n");
	  fprintf(dpost,"}bind def\n");

/*	  Select default line width */
	    post_lwidth(2);			     /* Medium (1-point) width */
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Close file								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_close() {
	post_newpage();
	fclose(dpost);
	lastact = CLOSE;
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Print Contents of Current Page					*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_newpage() {
	if (iscanline != 0)
	{   iscanline = 0;
	    last_line = 0;
	    last_b2 = 127;
	}
	do_stroke();

/*	fprintf(dpost,"10 10 moveto\n");
	fprintf(dpost,"(Printing time = ) show\n");
	fprintf(dpost,"usertime time sub 1000 div elapsed cvs show\n");
	fprintf(dpost,"( seconds) show\n");  */

	fprintf(dpost,"showpage\n");		/* Force page output */
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Line width								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_lwidth(arg) int arg; {		/* weak=1, mid=2, max=3 */

	float lwidth;

	if (arg == 1)  lwidth = 0.2;
	else if (arg == 2)  lwidth = 1.0;
	else           lwidth = 1.6;
	do_stroke();
	fprintf(dpost,"%3.1f w\n", lwidth);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Character size							*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_csize(arg1) int arg1; {	/* Set character size to arg1/72 of an inch */
	do_stroke();
	fprintf(dpost, "/Helvetica findfont %d scalefont\nsetfont\n", arg1);
}

void post_mainfont() {	/* Set character size to 16/72 of an inch */
	do_stroke();
	fprintf(dpost, "MainFont setfont\n");
}

void post_smallfont() {	/* Set character size to 12/72 of an inch */
	do_stroke();
	fprintf(dpost, "SmallFont setfont\n");
}

void post_typefont() {	/* Set character size to 12/72 of an inch, Courier */
	do_stroke();
	fprintf(dpost, "TypeFont setfont\n");
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Offset plot to make room for 3-hole punch				*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_offset(sixtyths) int sixtyths; {

	do_stroke();
	fprintf(dpost, "%d 0 t\n", sixtyths);
}



/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Define plotting area						*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_space(arg1,arg2,arg3,arg4)
 int arg1,arg2,arg3,arg4; {				/* Pretend plotting */

	do_stroke();
	xscale = POSTXMAX / (float) (arg3 - arg1);	/* area is from lower */
	yscale = POSTYMAX / (float) (arg4 - arg2);	/* left <arg1,arg2> */
	xhome = arg1;					/* to upper right */
	yhome = arg2;					/* <arg3,arg4> */
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Draw a box								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_box(box_linewidth,box_wid,box_hyt,LLx,LLy)
 int box_linewidth,box_wid,box_hyt,LLx,LLy;
 { float lwidth;
	if      (box_linewidth == 1)  lwidth = 0.2;
	else if (box_linewidth == 2)  lwidth = 1.0;
	else			      lwidth = 1.6;
	box_wid = box_wid * xscale;
	box_hyt = box_hyt * yscale;
	LLx = mapx(LLx);
	LLy = mapy(LLy);
	fprintf(dpost,"%3.1f %d %d %d %d Box\n",
		lwidth,box_wid,box_hyt,LLx,LLy);
 }


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   post_setblack*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_setblack() {
	fprintf(dpost, "0 g\n");
}
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Move, ready to draw text only					*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_move(arg1,arg2) int arg1,arg2; {	/* Move to <arg1,arg2>, do not plot */
	do_stroke();
	xactual = mapx(arg1);
	yactual = mapy(arg2);
	fprintf(dpost, "%4.2f %4.2f m\n", xactual, yactual);
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Move, ready to draw lines only					*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_point(arg1,arg2) int arg1,arg2; {	/* Move to <arg1,arg2> and display */
	do_stroke();
	do_newpath();
	xactual = mapx(arg1);
	yactual = mapy(arg2);
	fprintf(dpost, " %4.2f %4.2f m\n", xactual, yactual);
	lastact = POINT;
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Incremental draw line						*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void post_cont(arg1,arg2) int arg1,arg2; {	/* Draw line from current location */

	do_newpath();
/*    Do a moveto if beginning of newpath */
	if (lastact == NEWPATH) {
	    fprintf(dpost, " %4.2f %4.2f m\n", xactual, yactual);
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
	fprintf(dpost, " %4.2f %4.2f l\n", dxactual, dyactual);
	lastact = CONT;
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Print text								*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*     direction:	Orientation in degrees, 0=horizontal, 90=vertical */
void post_text(int direction, char * format, int a,int b,int c,int d,int e,int f,int g,int h)	/* Print a text string at cur loc */
{
	char charsave;
	int n;

	do_stroke();
	if (direction != lastrotate) {
	    fprintf(dpost,"%d ro\n", direction);
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
	    fprintf(dpost,"%d ro\n", -direction);
	    lastrotate = 0;
	}
	lastact = TEXT;
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   Start drawing new path						*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void do_newpath() {
	if (newpath == 0) {
	    fprintf(dpost,"n\n");
	    newpath = 1;
	    lastact = NEWPATH;
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*   End of path, actually draw it					*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

void do_stroke() {
	if (newpath == 1) {
	    fprintf(dpost,"s\n");
	    newpath = 0;
	    lastact = STROKE;
	}
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		       P O S T _ S C A N _ L I N E								 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*    Plot scan line of gray scale values, using postcript Image  */

void post_scan_line (float scan_wid,int LLx,int LLy,int URx,int *outspec)
{
	int i;
	int maxlevel = 15;
	float scan_len;
/*	int minrun = 8;	*/	/* minimum white space to skip */
	int minrun = 16;	/* minimum white space to skip */
/*	int minrun = 32; */	/* minimum white space to skip */
/*	int minrun = 64; */	/* minimum white space to skip */
/*	int minrun = 128;*/	/* minimum white space to skip */
	int b1;
	int b2;
/* Initialize position & scale before 1st scanline
   Assumption: There is no graphics to disturb path or scale
   between scan lines !!!
*/
	if (iscanline == 0)
	{   LLx = mapx(LLx);
	    LLy = mapy(LLy);
	    URx = mapx(URx);
	    scan_wid = scan_wid * yscale;
	    scan_len = (URx - LLx + 1)/128.;		/* size in current coordinates, 
						   	   scaled for image length */
	    fprintf (dpost,"gsave %d %d t\n",LLx,LLy);	/* translate to start */
	    fprintf (dpost,"%f %f scale\n",scan_len,scan_wid);
	}

	b1 = -10;
	b2 = 1000;
	for (i=0;i<=127;i++)
	{   if (outspec[i] < maxlevel)		/* if not white */
	    {	if (b2 == i-1) b2=i;		/* continue gray? */
		else
		{   if ((i-b2+1) >= minrun)	/* if not, was white long enough to skip? */
		    {	scan_seg (b1,b2,outspec);   	/* yes: print last gray seg */
			b1=b2=i;			/* and begin new segment */
		    }
		    else if (b1 < 0)			/* no, and this is 1st gray: */
		    {   b1 = i;				/* begin new gray segment */
			b2=i;
		    }
		    else b2=i;				/* no & not first:  */
		}					/* continue current segment */
	    }
	}
	if (b1 >= 0) scan_seg(b1,b2,outspec);		/* print any unprinted seg */
	iscanline++;
}

void scan_seg (b1,b2,outspec)
int b1,b2,outspec[];
{
  int n,dx,dy,i,j;
	n = b2-b1+1;
	if (n%2 != 0)		/* segment must be even */
	{ n++;
	  if (b2 < 127) b2++;
	  else b1--;
	}
	dx = last_b2 - b2;		/* samples counted in reverse direction 
					  from x */
	dy = iscanline - last_line;
	fprintf (dpost,"%d %d t %d IM\n",dx,dy,n);	/* translate, then image 
							   scanline n long */
	last_line = iscanline;
	last_b2 = b2;

	j = 0;
	for (i=b2;i>=b1;i--)
	{   fprintf (dpost, "%1x",outspec[i]);	/* Hex, forced 1-digit (for 4 bit)  */
/*	    fprintf (dpost,"%02x",outspec[i]);*/	/* Hex, forced 2-digit (for 8 bit)  */
	    j++;
/* 	    if (j == 32)*/			/* line break */
	    if (j == 64)			/* line break */
	    {	fprintf (dpost,"\n");
	    	j = 0;
	    }
	}
	if (j != 0) fprintf (dpost,"\n");
}


void post_amp_plot(int npage,float start)
{ extern int xminpost,yminpost,amps[];
  extern int ispec;
  int i,j,j1,j2,k,x,xmin,ymin;
  float ywid,sx,xscale10;
#define XMINAMPF	729	/* Lower corner of amplitude plot in specto */
	xmin = mapx(XMINAMPF);
	ymin = mapy(yminpost);
	ywid = .6 * yscale;
	fprintf(dpost,"grestore gsave\n");	/* back to neutral from scan coords*/
	fprintf(dpost,"%d %d t\n",xmin,ymin); /* origin to lower left of amp */
/*	if (npage == 1) isummx = amps[0]; */
	summy = start*ywid;
	xscale10 = -xscale/10.;
					
/*	for (i=0;i<=iscanline-1;i=i+100)*/	/* plot by hundreds*/
/*	{   if (i != 0) j1 = i; else j1 = 1; */	/* bad news if only 1 point */
					
	for (i=1;i<=iscanline-1;i=i+100)	/* plot by hundreds */
	{   j1 = i;				/* bad news if only 1 point */
	    j2 = i+100-1;
	    if (j2 > iscanline-1) j2=iscanline-1;

	    sx = (xscale10 * amps[j1-1]);
	    fprintf(dpost,"%.2f %.2f m\n",sx,summy); /* move to previous point*/
	    fprintf(dpost,"[\n");			/* begin rlineto array*/

	    k = 0;
	    for (j=j1;j<=j2;j++)
	    {   x = amps[j]-amps[j-1];
		sx = (xscale10 * x);
		fprintf(dpost,"%.2f ",sx);
		summy += ywid;
		if (k++ >10)
		{   fprintf(dpost,"\n");
		    k = 0;
		}
	    }
	    fprintf(dpost,"] {%.2f rlineto} forall stroke\n",ywid);
	}
	fprintf(dpost,"grestore\n");

	ispec = 0;
}

