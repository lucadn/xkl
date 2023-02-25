/*
 * $Log: xinfo.c,v $
 * Revision 1.6  2000-08-24 15:49:11-04  tiede
 * use named grays
 *
 * Revision 1.5  2000/06/08 03:21:06  tiede
 * use XAllocNamedColor
 *
 * Revision 1.4  1999/05/20 19:50:31  vkapur
 * no changes made
 *
 * Revision 1.3  1999/02/18  22:13:11  vkapur
 * none
 *
 * Revision 1.2  1998/07/17 08:04:20  krishna
 * Added RCS header.
 *
 */

/***********************************************************************
 *
 * Routines for all x widgets used for graphics
 *
 **********************************************************************/

#include "xinfo.h"

/***********************************************************************
 *
 * setupcmap(Widget draww, INFO *info) 
 *
 * Set up a gray map of 8 shades of gray to be used with all xkl tools.
 * Each widget used for graphics has an info structure that accesses the
 * colormap through color array:
 *  color[0].pixel = white
 *  color[1].pixel = black
 *  color[2-9].pixel = light gray to dark gray 
 *
 * Warning: getting the graphics context must be done after
 * the toplevel widget is realized.
 *
 ***********************************************************************/

void setupcmap(Widget draww, INFO *info)
{
  char str[20];
  int i, val;
  XColor exact;

  /* 
   * Some of these things could have been global
   * but this seems a little neater if somewhat redundant
   */

  info->display = XtDisplay(draww);
  info->screen = XtScreen(draww);
  info->screen_num = DefaultScreen(info->display);
  info->win = XtWindow(draww);
  info->gc = info->screen->default_gc;

  info->cmap = DefaultColormapOfScreen(info->screen); 
   
/*    XParseColor(info->display,info->cmap,"white",&(info->color[0])); */
/*    XAllocColor(info->display,info->cmap,&(info->color[0])); */
/*    XParseColor(info->display,info->cmap,"black",&(info->color[1])); */
/*    XAllocColor(info->display,info->cmap,&(info->color[1])); */

  XAllocNamedColor(info->display, info->cmap, "white", &(info->color[0]), &exact);
  XAllocNamedColor(info->display, info->cmap, "black", &(info->color[1]), &exact);
   
  XSetBackground(info->display, info->gc, info->color[0].pixel);
  
  for (i = 2; i < 10; i++) {  /* grays */

    /*
     * Changed from "gray10, gray20, etc. to numbers due to
     * Linux not recognizing these color variables (found by Erika)
     */

    /* val = (256 * (100 - i * 10)) / 100; */
    /* sprintf(str,"#%2x%2x%2x", val, val, val); */
    
    sprintf(str,"gray%d", 100 - i * 10);
/*      XParseColor(info->display,info->cmap,str,&(info->color[i])); */
/*      XAllocColor(info->display,info->cmap,&(info->color[i])); */
    XAllocNamedColor(info->display, info->cmap, str, &(info->color[i]), &exact);
  }
 
  XSetGraphicsExposures(info->display, info->gc, False);
}













