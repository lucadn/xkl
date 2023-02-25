/* $Log: wavdata.c,v $
 * Revision 1.3  1998/06/12 04:23:30  krishna
 * Changed from floats to shorts since the waveform is always a short.
 *
 * Revision 1.2  1998/06/06 05:19:13  krishna
 * Added RCS header.
 * */

#include <stdio.h>
#include "wavdata.h"

short findMaxValue(short *data, int length)
{
  register int i;
  register short max = -9999;

  for (i = 0; i < length; i++)
    if (*(data + i) > max) max = *(data + i);
  return(max);
}

short findMinValue(short *data, int length)
{
  register int i;
  register short min = 9999;

  for (i = 0; i < length; i++)
    if (*(data + i) < min) min = *(data + i);
  return(min);
}
