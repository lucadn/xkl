/* $Log: wavdata.h,v $
 * Revision 1.3  1998/07/17 00:12:15  krishna
 * Changed from floats to shorts since the waveform is always a short.
 *
 * Revision 1.2  1998/06/06 05:19:38  krishna
 * Added RCS header.
 * */

short findMaxValue(short *data, int length);
short findMinValue(short *data, int length);
