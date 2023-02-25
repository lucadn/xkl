/*
 * $Log: xinfo.h,v $
 * Revision 1.3  1998/07/17 08:05:16  krishna
 * Added RCS header
 *
 */

#ifndef XINFO_H
#define XINFO_H

#include <stdio.h>
#include <math.h>
#include <time.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/keysymdef.h>
#include <X11/Xmu/StdCmap.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleBG.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrollBar.h>
#include <Xm/SelectioB.h>
#include <X11/keysymdef.h>
#include <Xm/SelectioBP.h>
#include <Xm/FileSB.h>
#include <Xm/DialogS.h>

/*
 * All X windows used for graphics need one of these
 */ 

typedef struct INFO { 
    Screen *screen;
    int screen_num;
    Display *display;
    GC gc;
    Window win;
    Colormap cmap;
    XColor color[10];   /* use 2-9 until .freq changes */
    Pixmap pixmap;      /* background of every grapics window */
    XFontStruct *font;
 } INFO;

#endif /* XINFO_H */






