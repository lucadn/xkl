#ifndef SYNGRID_H
#define SYNGRID_H
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