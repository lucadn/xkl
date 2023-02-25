/* $Log: getargs.c,v $
 * Revision 1.4  1998/06/09 00:32:27  krishna
 * Added remainingArgs, which checks for args starting with a "-" in
 * argv.  Also, modified arg processing, so that arg (and its parameter)
 * are removed from argv and argc.
 *
 * Revision 1.3  1998/06/06 05:06:57  krishna
 * Added RCS header.
 * */

#include <stdio.h>
#include <string.h>
#include "getargs.h"

/***********************************************************************
 * 
 * This function checks if the argument name is present in argv.  If it 
 * does exist, then return TRUE, and remove name from argv, and decrements 
 * argc.  If name does not exist, then the function returns FALSE.
 *
 **********************************************************************/

int IsArg(char *name, int *argc, char *argv[])
{
  register int i, j;

  for (i = 1; i < *argc; i++) 

    /* 
     * If Arg name present, remove arg from argv, and decrement argc 
     */

    if (strcmp(argv[i], name) == 0) {
      for (j = i; j < *argc - 1; j++) argv[j] = argv[j + 1];
      *argc -= 1;
      return(TRUE);
    }
  return(FALSE);
}

/***********************************************************************
 * 
 * This function checks if the argument name is present in argv.  If
 * it does exist, then return the integer following name, removes name
 * and the integer from argv, and decrement argc by 2.  If str does
 * not exist, then the function returns the default value.
 *
 * Suppose function called with:
 * checkIntArg("-i", &argc, argv, 999);
 * 
 * If "-i 2" is specified on command line, then the function returns
 * 2.  If "-i" was not specified on command line, then the function
 * returns 999.
 *
 **********************************************************************/

int checkIntArg(char *name, int *argc, char **argv, int defaultValue)
{
   register int i, location = 0;
   int num = ERROR;

   for (i = 1; i < *argc; i++)
     if (strcmp(argv[i], name) == 0) location = i;

   /* 
    * If the argument is found, then read the integer value into
    * num, and then remove arg and the integer from argv, and decrement
    * argc 
    */

   if (location > 0) {
     if ((location + 1) >= *argc) {
       fprintf(stderr, "Integer not specified for '%s'\n", argv[location]);
       for (i = location; i < *argc - 1; i++) /* Remove arg and int */
	 argv[i] = argv[i + 1];
       *argc -= 1; /* Decrement argc */
       return(defaultValue);
     }
     sscanf(argv[location + 1], "%d", &num); /* Read int */
     for (i = location; i < *argc - 2; i++) /* Remove arg and int */
       argv[i] = argv[i + 2];
     *argc -= 2; /* Decrement argc */

     return(num); /* Return the integer value */
   }
   return(defaultValue);  /* Return the default value */
}


/***********************************************************************
 * 
 * 
 * This function checks if the argument name is present in argv.  If
 * it does exist, then return the float following name, removes name
 * and the float from argv, and decrement argc by 2.  If name does
 * not exist, then the function returns the default value.
 *
 * Suppose function called with:
 * checkFloatArg("-f", &argc, argv, 999);
 * 
 * If "-f 2.4" is specified on command line, then the function returns
 * 2.4.  If "-f" was not specified on command line, then the function
 * returns 999.0.
 *
 **********************************************************************/

float checkFloatArg(char *name, int *argc, char **argv, float defaultValue)
{
   register int i, location = 0;
   float num = ERROR;

   for (i = 1; i < *argc; i++)
     if (strcmp(argv[i], name) == 0) location = i;

   /* 
    * If the argument is found, then read the integer value into
    * num, and then remove arg and the integer from argv, and decrement
    * argc 
    */

   if (location > 0) {
     if ((location + 1) >= *argc) {
       fprintf(stderr, "Float not specified for '%s'\n", argv[location]);
       for (i = location; i < *argc - 1; i++) /* Remove arg and float */
	 argv[i] = argv[i + 1];
       *argc -= 1; /* Decrement argc */
       return(defaultValue);
     }
      sscanf(argv[location + 1], "%f", &num); /* Read float */
      for (i = location; i < *argc - 2; i++) /* Remove arg and float */
	argv[i] = argv[i + 2];
      *argc -= 2; /* Decrement argc */

      return(num); /* Return the integer value */
   }
   return(defaultValue);  /* Return the default value */
}

/***********************************************************************
 * 
 * This function checks if there are arguments that start with a "-",
 * e.g. "-f", present in argv.  If they do exist, then the function
 * return argument.  If no arguments starting with a "-" exist, then
 * the function returns NULL.  This is mainly to check if the user
 * specified an argument that we were not expecting.
 *
 **********************************************************************/

char *RemainingArgs(int argc, char *argv[])
{
  register int i, j, found = FALSE;

  for (i = 1; i < argc; i++)
    if (argv[i][0] == '-')  /* Argument starts with a - */
      return(argv[i]);
  return(NULL);
}
