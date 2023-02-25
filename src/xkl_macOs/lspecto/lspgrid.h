/*
$Log: syn.h,v $
Revision 1.4  1998/07/16 22:59:29  krishna
Removed full pathname for default file, so the user can specify where
the file will be located in the Makefile.

Revision 1.3  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#ifndef LSPGRID_H
#define LSPGRID_H


#define XMINAMP	650		/* Lower corner of amplitude plot in f0 */
#define XMINAMPF	729	/* Lower corner of amplitude plot in specto */
#define XMINF0	480		/* Lower corner of f0 plot */
#define XMINFORM	576	/* Lower corner of formant or specto plot */

	//extern char toupper();
	extern char *getenv();
	//extern int strlen();

	extern void	post_space();
	extern void	post_move();
	extern void	post_point();
	extern void	post_cont();
	extern void	post_smallfont();
	extern void	post_mainfont();
	extern void	post_setblack();
	extern void	post_lwidth();
	extern void	post_text();

	
	void spectgrid(char *,int ,int ,char *,char *);
	void post_wave_plot(short *, int ,int );
	void post_wave_axis(char *, int, int, char *, int, char *);
	void post_freq_plot(int *, int, int, int);
	void post_spec_plot(int *, int, int);
	void post_grid(char *,int ,int ,char *,char);
	void post_f0_grid(char *,int, int,char *,char *);
	void post_f0_plot(int, int, int);
	void post_amp_axis(int ,int);
	void lefttick(int,int);
	void leftlabel(int,int,int);
	void righttick(int,int);
	void bottomtick(int,int);
	void toptick(int,int);
#endif /*LSPGRID_H*/


