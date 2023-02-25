/*
$Log: synparwav.h,v $
Revision 1.2  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#ifndef SYNPARWAV_H
#define SYNPARWAV_H
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                    P A R W A V T A B . H                            */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */



void setabc(int,int,float *,float *,float *);
void setzeroabc(int,int,float *,float *,float *);
void parwav(short *);
void gethost();
void gen_noise();
void setR1(int,int);
void pitch_synch_par_reset();
void resonlp();
void resonglot();
void resontilt();
void spacefilt();
void resontz();
void resontp();
void resonnz();
void resonnp();
void resonc8();
void resonc7();
void resonc6();
void resonc5();
void resonc4();
void resonc3();
void resonc2();
void resonc1();
void reson6p();
void reson5p();
void reson4p();
void reson3p();
void reson2p();
void no_rad_char(float);
void getmax(int,int*);
int truncate_dk(int);
void overload_warning(int);
float dBconvert(int);
void pr_pars();
void reset_data();

#endif /*SYNPARWAV_H*/
