/* $Log: spectrum.h,v $
 * Revision 1.2  1998/06/06 05:17:26  krishna
 * Added prototype for srsolv and RCS header.
 * */

/*
 * Find lpc.
 */

void pcode(float *s,int npts, int p, float *a, float *phi, 
	   double *pcerror, double *energy, int *ier);
void srsolv(float *a, float *x, int n, float eps, int *ier);
