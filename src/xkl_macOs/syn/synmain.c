/*
$Log: synmain.c,v $
Revision 1.2  1998/06/09 00:45:41  krishna
Added RCS header.
 */
#include "syn.h"
#include <stdlib.h>
#include <string.h>
#define VT100             'd'

/* HISTORY:
 *
 * 0004 10-Mar-88 DK	Remove all calls to lxy
 * 0005  6-Apr-88 DK	Add -b 'batch-job' disable Keyio so command files work
 * 0006 19-Aug-88 DK	Change name to KLSYN88
 * 0007  6-Sep-88 DK	Allow 3-char names for parameters
 *      24-Feb-93 NM    Allow impulse in parallel tract KLSYN93
 *         May-94 DH    Took out all graphics/write .wav on big endian
*/

int main(int argc, char **argv) 
{

   char tmpname[500];
   extern char graphout; 
   extern int warningsw;
   extern char request; /*syn.h*/
   extern int syn_i;
   extern char firstname[];


   graphout = VT100; warningsw = 0;

/* Print version number */

  printstuff();

  if (argc <= 1) {	/* Must have name of .doc file or -d*/
    helpa();
    exit(1);
  }

/*    Decode arguments */
  for (syn_i=1; syn_i<argc; syn_i++) {
      if (argv[syn_i][0] == '?') {
         helpa();		/* 27. Print argument options */
         exit(1);
      }/* usage*/

  else if (argv[syn_i][0] == '-' && argv[syn_i][1] == 'd') {
  	printf("\n  Reading default config file \n");
     initconfig(NULL);	/*  2. Read defaults from .con file */
     clearpar();	/* 12. Put defaults in time functions */
  }      

  else {
   if(strcmp(&argv[syn_i][strlen(argv[syn_i]) - 3 ],"doc") == 0 )
                    {argv[syn_i][strlen(argv[syn_i]) - 4 ]  = '\0';}
            copystring(&argv[syn_i][0],firstname);	/* 26. */
            makefilenames();			/* 11. */
	    readdocfile();			/*  1. */
	  }
  }/* done Decode arguments*/

/* Set up ability to peek at char typed without waiting for carret */

  helpr();

  /* Main loop */
  while (1) {
   request = get_request();	
   actonrequest(request);	
  }
}
