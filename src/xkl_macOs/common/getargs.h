/* $Log: getargs.h,v $
 * Revision 1.7  1998/06/09 00:33:16  krishna
 * Added remainingArgs, which checks for args starting with a "-" in
 * argv.  Changed ERROR to -9999.
 *
 * Revision 1.6  1998/06/07 22:19:03  krishna
 * Changed prototype -- made argc into *argc so can return remaining
 * number for arguments.
 *
 * Revision 1.5  1998/06/06 05:07:16  krishna
 * Added RCS header.
 * */

int IsArg(char *str, int *argc, char *argv[]);
int checkIntArg(char *name, int *argc, char **argv, int default_value);
float checkFloatArg(char *name, int *argc, char **argv, float default_value);
char *RemainingArgs(int argc, char *argv[]);

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef ERROR
#define ERROR -9999
#endif
