/*
$Log: syn.h,v $
Revision 1.4  1998/07/16 22:59:29  krishna
Removed full pathname for default file, so the user can specify where
the file will be located in the Makefile.

Revision 1.3  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#ifndef LSPCRIP_H
#define LSPCRIP_H


#define POSTXMAX		576
#define POSTYMAX		756

#define OPEN	0
#define CLOSE	-1
#define POINT	1
#define CONT	2
#define TEXT	3
#define NEWPATH	4
#define STROKE	5

/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */
/*       Postscript Laser printer formatter PLOT ROUTINES		*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * */

#define mapx(x) ((x - xhome) * xscale)
#define mapy(y) ((y - yhome) * yscale)

static int isummx;
static float summy;
static int iscanline = 0;
static int last_line;
static int last_b2 = 127;
static float xscale,yscale, xhome,yhome;
static float xactual,yactual,dxactual,dyactual;
static int lastrotate,newpath,lastact;
static char flabel[80];	/* Needed to format the string sent to label() */
void post_open(char *);
void post_close();
void post_newpage();
void post_lwidth(int);
void post_csize(int);
void post_mainfont();
void post_smallfont();
void post_typefont();
void post_offset(int);
void post_space(int,int,int,int);
void post_box(int,int,int,int,int);
void post_setblack();
void post_move(int,int);
void post_point(int,int);
void post_cont(int,int);
void post_text(int, char *, int,int,int,int,int,int,int,int);
void do_newpath();
void do_stroke();
void post_scan_line (float,int,int,int,int *);
void scan_seg (int,int,int*);
void post_amp_plot(int,float);
#endif /*LSPCRIP_H*/


