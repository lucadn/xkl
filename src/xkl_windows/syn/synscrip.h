
#include <stdio.h>
#ifndef SYNSCR_H
#define SYNSCR_H

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