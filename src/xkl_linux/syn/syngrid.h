#ifndef SYNGRID_H
#define SYNGRID_H
#define XPOSTSIZE	600
#define SCALE	100

	int xminpost,yminpost;	/* Lower left corner of plot	*/

	//extern char toupper();
	extern char *getenv();
//	extern int strlen();
/*
	extern int	post_space();
	extern int	post_move();
	extern int	post_point();
	extern int	post_cont();
	extern int	post_smallfont();
	extern int	post_mainfont();
	extern int	post_setblack();
	extern int	post_lwidth();
	extern int	post_text();
	extern int	post_newpage();
*/
	static int nfreqchan,xinc,xpostop,yposmax,xsize,frmax,curpage;
	static int npts_frame,nframe_sec,npts_tick,delfreq;
	static int i,j,t,f,xpos,ypos,ypos0,ypos1,ypos2,xpage;
	static char klpath[200],label[200];
int postspectgrid(char *,int,int,char *,char *,int,int,int);
int post_labelplot(char *,char *);
int post_grid(int,int,char *,int);
int plefttick(int,int);
int pleftlabel(int,int,int);
int prighttick(int,int);
int pbottomtick(int,int);
int ptoptick(int,int);
int post_freq_plot(int*,int,int,int);
int post_par_plot(int,int,int);
#endif /*SYNGRID_H*/