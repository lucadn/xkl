
#include <stdio.h>
#ifndef SYNSCR_H
#define SYNSCR_H
/* Should be 612 by 792 */
#define POSTXMAX		600
#define POSTYMAX		792

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

static float xscale,yscale, xhome,yhome;
static float xactual,yactual,dxactual,dyactual;
static int lastrotate,newpath,lastact;
static char flabel[200]; /* Needed to format the string sent to label() */
FILE *dpost,*fopen();
int post_open(char *);
int post_close();
int post_newpage();
int post_lwidth(int);
int post_csize(int);
int post_mainfont();
int post_smallfont();
int post_typefont();
int post_offset(int);
int post_space(int,int,int,int);
int post_box(int,int,int,int,int);
int post_setblack();
int post_move(int,int);
int post_point(int,int);
int post_cont(int,int);
int post_text(int, char*, int,int,int,int,int,int,int,int);
int do_newpath();
int do_stroke();
#endif /*SYNSCR_H*/