/*
$Log: syn.h,v $
Revision 1.4  1998/07/16 22:59:29  krishna
Removed full pathname for default file, so the user can specify where
the file will be located in the Makefile.

Revision 1.3  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#ifndef SYN_FUNC_H
#define SYN_FUNC_H

#include <stdio.h>

float dBconvert();
float DBtoLIN();
void printstuff();
void helpa();
void initconfig(char *); 
void clearpar();
void copystring(char *,char *);
void makefilenames();
void readdocfile();   
void helpr();
char get_request();
void actonrequest(char);
void endgraph();
void putconfig(FILE *); 
int get_par_name();
void settypicalpar();
void drawparam();
int graphallpars();
void askforname(); 
int synthesize();
void savedocfile();
void prpars(FILE *);
void skip_line(FILE *);
void readheader();
int get_digits(FILE *); 
char get_nonwhite_char(FILE *);
void setspdef();
int get_value(int);
void prparnames(FILE *);
void initpars();
void begingraph();
void gettimval(int*, int*);
void eraseparline();
void drawparline();
void draw_line();
int get_time();
int getpval(); 
void setpval(); 
int checklimits();
void post_constants();
int post_variables();
void putforfreq(char *);
int relevent(char,char,char);
int conval(char *);
int findtext(char,char);
int findnp(char *);
int decodparam();
void mergestring(char *, char *, char *);

#endif /*SYN_FUNC_H*/


