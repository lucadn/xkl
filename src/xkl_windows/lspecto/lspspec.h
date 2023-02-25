/*
$Log: syn.h,v $
Revision 1.4  1998/07/16 22:59:29  krishna
Removed full pathname for default file, so the user can specify where
the file will be located in the Makefile.

Revision 1.3  1998/06/09 00:45:41  krishna
Added RCS header.
 */


#ifndef LSPSPEC_H
#define LSPSPEC_H

void getspec(short *iwave,int type_spec);
int mgtodb(long nsum);
void mkwnd(float *hamm,int wsize,int power2);
#endif /*LSPGRID_H*/


