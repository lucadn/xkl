/* $Log: spectrum.c,v $
 * Revision 1.3  1998/06/06 05:21:40  krishna
 * Included spectrum.h.
 *
 * Revision 1.2  1998/06/06 05:15:54  krishna
 * Prettified text and added RCS header.
 * */

#include <math.h>
#include "spectrum.h"

/************************************************************************/
/*         pcode(s,npts,p,a,phi,pcerror,energy,ier)                     */
/************************************************************************/

void pcode(float *s,int npts, int p, float *a, float *phi, 
	   double *pcerror, double *energy, int *ier)
{
/*
C	LPCSUBS.FOR		D/.H. Klatt		5-Mar-84
C
C	To call these routines from a C program, all args must
C	 be passed as pointers
C
C
C	PCODE.FOR	D. KLATT	VERSION 1	12/20/78
C
C	Author:
C	 M. R. PORTINOFF		04.03.74
C
C	Purpose:
C		Compute linear prediction coefficients A(K) from
C		sequence S(N).
C
C	Usage:
C		CALL PCODE(S,NPTS,P,A,PHI,ERROR,ENERGY,IER)
C
C	Description of parameters:
C		S	- Floating-point array containing sequence
C			  of data samples.
C		NPTS	- Number of data samples in S.
C		P	- Order of predictor system, i.e., number of
C			  predictor coefficients (P is an integer).
C		A	- Floating-point array in which predictor
C			  coefficients are returned. The coeficients
C			  are determined for best fit of
C				s(n) = SUM [a(k)*s(n-k)].
C		PHI	- THE SCRATCH ARRAY PHI MUST CONTAIN AT LEAST
C			  P*(P+3)/2 REAL LOCATIONS. P=20 MEANS 230 LOCS.
C		ERROR	- Mean squared difference between the original
C			  sequence and the linear prediction estimate.
C		ENERGY	- Energy of given sequence computed as
C				ENERGY = SUM [s(n)*s(n)].
C		IER	- Error parameter returned by equation solving
C			  subroutine SRSOLV (see SRSOLV).
C
C	Remarks:
C		ITYPE	- Parameter specifying formulation of linear
C			  prediction algorithm to be used.
C			  ITYPE=0 - S(N) contains P+NPTS samples, the
C			    first P of which are used as initial
C				    conditions (Linear predictive coding
C				    formulation, c.f. Atal).
C			  ITYPE=1 - S(N) contains P+NPTS samples, the
C				    first P of which are ignored and
C				    treated as zeros (Inverse filtering
C				    formulation, c.f. Markel).
C			  ITYPE=2 - S(N) contains NPTS samples and the
C				    initial conditions are assumed to
C				    be zero (c.f. Markel).
C
C	Subroutines and functions required:
C		SRSOLV
C
C	References:
C		J.Makhoul, J.Wolf: BBN Report Number 2304.
C
C
*/
  static int n0,n1,nn,ipsi,itype,nnp1, nnj,nnk;
  static double dsum,phi00;
  static float eps;
  static int i, j, k, n, npdiag, idiag;
  static int nmk, ind, n0k, n0j;

  itype = 2;

  eps = 0.0;
  n0 = 0;
  ipsi = p * (p + 1) / 2 - 1;
  if(itype != 2)  { n0 = p; }
  nn = npts + n0;
  n0++;
  n1 = n0 ;
  nnp1 = nn + 1;

  dsum = 0.0;

  /* Compute correlation functions */

  i = 0;
n605: j = 0;
  k = i;
  i++;
  ipsi++;

/* Compute psi vector ( append to phi matrix) */

  dsum = 0.0;
  if(itype != 0) n1 = k + n0;
  for( n = n1; n <= nn; n++) {
    nmk = n - k;
    dsum = dsum + (double)(s[n - 1] * s[nmk - 1]);
  }/* number of points*/
  if(i == 1) phi00 = dsum;
  phi[ipsi - 1] = dsum;
  if (i > p) goto n630;

/* Compute elements of phi[j,k] but store them columnwise
   as phi[j,k] = phi(ind)                                */

  npdiag = p - i + 1;
  ind = k * (k - 1) / 2;
  for(idiag = 1; idiag <= npdiag; idiag++) {
    j++;
    k++;
    ind = ind + k;
    if (itype != 0) goto n620;
    nnj = nnp1 - j;
    nnk = nnp1 - k;
    n0j = n0 - j;
    n0k = n0 - k;
    dsum = dsum - (double)(s[nnj - 1] * s[nnk - 1]) +
      (double)(s[n0j - 1] * s[n0k - 1]);
n620: phi[ind - 1] = dsum;
  }/*npdiag*/
  goto n605;

n630: srsolv(phi,a,p,eps,ier);

/* Compute normalized error */

   n1 = p * (p + 1) / 2 + 1 ;
   nn = n1 + p - 1;
   dsum = phi00;
   for(n = n1; n <= nn; n++)
     dsum = dsum - (double)(phi[n - 1] * phi[n - 1]);
   *pcerror = dsum / phi00;
   *energy =  phi00;
}

/*************************************************************************/
/*                  SRSOLV(A,X,N,EPS,IER)                                */
/*************************************************************************/
void srsolv(float *a, float *x, int n, float eps, int *ier)
{
/*     
C
C
C	SRSOLV.FOR	D. KLATT	VERSION 1	12/20/78
C
C	Author:
C		M.R. Portnoff
C		(Adapted from IBM SSP/V3  matrix factoring subroutine)
C
C	Puropse:
C		Solve a symmetric, positive-definite set of linear
C		algebfactorization looraic equations by the square-rooting method.
C
C	Usage:
C		CALL SRSOLV(A,X,N,EPS,IER)
C
C	Description of parameters:
C		A	- 1-dim. array containing the upper-triangular
C			  part of the given symmetric, positive-definite
C			  coefficient matrix augmented by the constant 
C			  vector. The elements are stored columnwise
C			  in this array. Upon return, this array
C			  contains the elements of the upper-triangular
C			  square-root factor of the original coefficient
C			  matrix augmented by the auxillary vector.
C		X	- Array in which solution vector is returned.
C		N	- Number of rows or columns in the coefficient
C			  matrix. That is, the number of equations in
C			  the system.
C		EPS	- A (positive) input constant which is used as 
C			  relative tolerance for test on loss of 
C			  significance.
C		IER	- Resulting error parameter coded as follows:
C			  IER=0  - No error
C			  IER=-1 - No result because of wrong input
C			           parameter N or because some radicand
C			           is nonpositive (i.e., matrix A is not
C			           positive definite, possibly due to 
C			           loss of significance).
C			  IER= K - Warning which indicates loss of
C			           significance. The radicand formed
C			           at factorization step K+1 was still
C			           positive but no longer greater than
C			           ABS(EPS*A(K+1,K+1)).
C
C	Remarks:
C		The upper-triangular part of the given symmetric
C		matrix augmented by the constant vector is stored
C		columnwise in the N*(N+1)/2 + N  element array A( ).
C		The square-root factor of A augmented by the  auxiliary 
C		vector is returned in this array.
C
C	Subroutines and functions required:
C		None.
C
C	Method:
C		Solution is found using the square-rooting method
C		of Cholesky. The coefficient matrix is factored as the
C		product of two triangular matrices, where the left-hand
C		factor is the transpose of the returned right-hand
C		factor.Once factored the solution follows by recursion
C		(see Faddeeva: Computational Methods of Linear Algebra).
C
*/
  static double dpiv,dsum;
  static int k, i, l, lend;
  static int kpiv, ind, j, jy, lanf;
  static int lind,kstart,np1;
  static double tol;

  dsum = 0.0;

  /* test on wrong input parameter n*/
  if (n - 1  < 0) { 
    *ier = -1; 
    return;
  }
  *ier = 0;
  np1 = n + 1;

  /*  initialize diagonal loop */    

  kpiv = 0;
  for(k = 1; k <= n; k++) {
    kpiv = kpiv + k;
    ind =  kpiv;
    lend = k - 1;

    /* calculate tolerance */
    
    tol = fabs(eps * a[kpiv - 1]);

    /* start factorization loop over kth row */

    for (i = k; i <= np1; i++) {
      dsum = 0.0;
      if (lend > 0) { /* start inner loop */
	for (l = 1;  l <= lend; l++) {
	  lanf = kpiv - l;
	  lind = ind - l;
	  dsum = dsum + (double)(a[lanf - 1] * a[lind - 1]);
	}
      }

      /* transform element  a[ind] */

      dsum = (double)a[ind - 1] - dsum;

      if (i - k == 0) {
	if ((float)dsum - tol <= 0.0) {
	  if(dsum <= 0.0) { 
	    *ier = -1; 
	    return;
	  }
	  if ((*ier) <= 0) *ier = k - 1;
	  /* compute pivot element */
	}
	dpiv  = sqrt(dsum);
	a[kpiv - 1] = dpiv;
	dpiv = 1.0 / dpiv;
	goto n11;
      }
      a[ind - 1] = dsum * dpiv;
n11:  {ind  = ind + i;}
    }/*factorization loop*/
  }/* end of diagonal  loop */

  ind = n * (n + 1) / 2 ;
  jy = ind  + n;
  x[n - 1] = (double)a[jy - 1] / (double)a[ind - 1];

/* solve for x[j] if n > 1 */

     if (n == 1) return;
     k = n;
     for(i = 2; i <= n; i++) {
       j = n - i + 1;
       jy = jy - 1;
       ind = j * (j + 1) / 2;    
       dsum = (double)a[jy - 1];
       dpiv = (double)a[ind - 1];
       kstart = j + 1;
       for(k = kstart; k <= n; k++){
	 ind = ind + k - 1;
	 dsum = dsum - (double)(a[ind - 1] * x[k - 1]);
       }/* k */
       x[j - 1] = dsum / dpiv;
     }
}
