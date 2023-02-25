/*
 * $Log: xspec_util.c,v $
 * Revision 1.12  2002-06-13 11:13:32-04  tiede
 * support mouseDn:mouseUp squawk; lmfile moved to xkl_label
 *
 * Revision 1.10  2000-09-15 16:19:31-04  tiede
 * spectrogram now displayed correctly regardless of screen depth
 * mods to support LEX *.label files
 *
 * Revision 1.9  2000/08/24 20:15:46  tiede
 * count adjustment in addtolist()
 * use DefaultDepth() for bitspp in spectro_pixmap()
 * LAFF label support:  lmfile()
 *
 * Revision 1.8  2000/06/08 03:28:42  tiede
 * stub writegram (don't write gram files)
 *
 * Revision 1.7  2000/04/20 20:00:34  xkldev
 * Removed printf statements that echoed tick marks (from Vishal?)
 *
 * Revision 1.6  1999/05/20  19:47:00  vkapur
 * revisions to support lock feature-- i.e., lock a subset of
 *   opened waveforms.
 *
 * Revision 1.5  1999/02/18 22:27:57  vkapur
 * showoops -> ShowOops
 * funtionality for remembering current directory in callbacks
 *   for file sel. box dialogs
 *
 * Revision 1.4  1998/07/17 08:06:44  krishna
 * Added RCS header.
 * */

/*
Power spectral estimation using the FFT(fast discrete Fourier transform)

The power spectral estimation code is by Kimble and Embree.  
An array (1/2 number of samples in FFT-neg freq are thrown out)
of magnitudes in db X 10 is calculated and these magnitudes
map to a gray scale. The frequencies are then displayed
versus time in this greyscale. The frequencies are established
by the number of samples in a FFT displayed as 1/2 that number 
delineations on the y axis in kHz (range of freq. from sampling rate)
*/

#include "xspec_util.h"
#include "xinfo.h"
#include "xklspec.h"

extern Widget warning;
extern FILE *PS4_fp;
extern int quadcount;
extern int lock;

/****************************************************************/
/*        mstox(XSPECTRO *spectro, int window, float ms)         */
/****************************************************************/

int mstox(XSPECTRO *spectro, int window, float ms)
{
   float fx,st;
   int x;

   fx = (ms - spectro->startime[window]) * 
      (float)findxmax(spectro,window)   /
	 ((float)spectro->sampsused[window] / spectro->spers * 1000.0); 
   x = fx + .5;
   x += xoffset(spectro);

   return(x);
}

/*********************************************************************/
/*void expdraww(Widget w, XtPointer client_data, XtPointer call_data)*/
/*********************************************************************/
 
void expdraww(Widget w, XtPointer client_data,XtPointer call_data){ 

/* redrawing for an expose event only copy what's necessary */

XSPECTRO *spectro;
INFO *info;
int x,y,width,height;
static int oldx,oldy,oldw,oldh;
int i;
XmDrawingAreaCallbackStruct *c_data;


   c_data = (XmDrawingAreaCallbackStruct *)call_data;
   

   /* need to redraw cursor */
   spectro = (XSPECTRO *)client_data;

  /* establish which window by checking which widget */
  /* needs work */
  for( i = 0; i < NUM_WINDOWS; i++)
     if (spectro->draww[i]  == w) break;

info = spectro->info[i];

   /*
   x = c_data->event->xexpose.x;  y = c_data->event->xexpose.y;
   width = c_data->event->xexpose.width;
   height = c_data->event->xexpose.height;

   
   XCopyArea(info->display,info->pixmap,
            info->win,info->gc,
            x ,y,width,height,x,y);
   */
   /* the draw cursor routine copies the whole pixmap */
   /*wait until all segments of pixmap are redrawn*/
 if(c_data->event->xexpose.count == 0){

    if( i == SPECTRUM)
       {spec_cursor(spectro);   }  /* Used to have 2 arguments */
    else
       {draw_cursor(spectro,i,info);}

  }/*wait until all segments of pixmap are redrawn*/

 }/*expdraww*/

/******************************************************************/
/* draw without expanding zeros used in postscript to save space  */

/*
   lastindex = -1;

   t = spectro->t0;  fq = spectro->wymax - 1;
   for(i = 0; i < spectro->postcount; i++){
      if(spectro->posti[i] > 10){
          tmp = spectro->posti[i] - 10;
          t += tmp / spectro->wymax; 
          fq = fq - (tmp % spectro->wymax);
             if( fq < 0){
               fq = spectro->wymax + fq;
               t++;
	     }
        }
     else{

            if(lastindex != spectro->posti[i]){
                 XSetForeground(spectro->info->display,
                                spectro->info->gc,
				spectro->info->color[spectro->posti[i]].pixel);
				lastindex = spectro->posti[i];
				}
				XDrawPoint(spectro->info->display,
				spectro->info->win,
				spectro->info->gc,t,fq);
				fq = fq - 1;
				if( fq < 0){
				fq = spectro->wymax + fq;
				t++;
				} finished one spectra 
				}draw 
				
				}all ints 
				*/
/*******************************************************************/

/*******************************************************************/
/* postgvs(Widget w, XtPointer client_data, XtPointer call_data)   */
/*******************************************************************/
/* this writes a poscript file with basically the resolution*/
/* that you see on the screen it puts encoded data in the file*/
/* that is in the .gram format */
/*******************************************************************/
void  postgvs(XSPECTRO *spectro, FILE *fp)
{
  int numperl, i, totgvs, starti, numzeros, numb;
  float scale, tmpscale;
  time_t curtime;
  char datestr[26];
  
  totgvs = spectro->wxmax * spectro->wymax;
  starti = spectro->t0 * spectro->wymax;
  spectro->posti = (int *) malloc( sizeof(int) * totgvs);
  spectro->postcount = 0;
  
  for( i = starti; i < totgvs + starti; i++){
    if((int)spectro->allgvs[i] == 0){
      numzeros = 0;
      while(((int)spectro->allgvs[i] == 0) && (numzeros < 245)
	    && (i < totgvs + starti )){
	numzeros++;
	i++;
      }
      spectro->posti[spectro->postcount] = numzeros + 10;
      spectro->postcount++;
      i = i - 1;
    }
    else{
      spectro->posti[spectro->postcount] = (int)spectro->allgvs[i];
      spectro->postcount++;
    }
  }
 
  /*if you want to know what's happening between currentfile*/
  /*and repeat see code above */ 
  
  scale = 1.0;

   /* do xr + yb fit on page (1 inch border, 792 x 612)*/
  if( spectro->xr[GRAM] > 648  && spectro->yb[GRAM] > 468){

    scale = (float)648 /(float)spectro->xr[GRAM] ;
    tmpscale = (float)468 / (float)spectro->yb[GRAM] ;
       if(tmpscale < scale){
           scale = tmpscale;
        }

  }/* check to see which scale prevails */
 else {
    if(spectro->xr[GRAM] > 648 ){
      scale = (float)648 / (float)spectro->xr[GRAM];
    }  
    if(spectro->yb[GRAM] > 468){
     scale = (float)468 / (float)spectro->yb[GRAM] ;
    }
  }

  fprintf(fp,"%c%cBoundingBox: 72 72 %d %d\n",'%','%',
  (int)(spectro->yb[GRAM] * scale) + 144,
  (int)(spectro->xr[GRAM] * scale) + 72); 
  fprintf(fp,"gsave\n");
  fprintf(fp,"/d {def} def /e {exch} def /p {pop} def\n");
  fprintf(fp,"/s {sub} d /a {add} d\n");
  fprintf(fp,"/o {show} d /m {moveto} d\n");
  fprintf(fp,"/k {lineto stroke} d\n");
  fprintf(fp,"/b {moveto -.5 0 rmoveto 1.0 0 rlineto stroke} d\n"); 
  fprintf(fp,"/g [0 0 .7 .6 .5 .4 .3 .2 .1 .0] d\n");
  fprintf(fp,"/w %d d /h %d d\n",spectro->xgram,spectro->wymax);
  fprintf(fp,"/t 0 d /f 0 d /l -1 d\n");

/* put date and filename outside of bounding box */  

  fprintf(fp,"/Times-Roman findfont 13 scalefont setfont\n");
  fprintf(fp,"144 96 translate 90 rotate\n");

  time(&curtime);
  strcpy(datestr,ctime(&curtime) ); datestr[24] = ' ';
  fprintf(fp, "0 42 m (%s) o\n",datestr);
  fprintf(fp, "0 28 m (File: %s) o\n", spectro->wavefile);
  fprintf(fp, "0 14 m (Current Dir: %s) o\n", curdir);
  fprintf(fp, "0 0 m (SF = %d Hz, %.1fms Hamming window every %.1fms, %d pt. DFT) o\n",
    (int)spectro->spers,spectro->winms,spectro->msdelta,spectro->slice);
  fprintf(fp,"grestore gsave\n");
/* end date and filename*/

 /* axes etc. */
/* debug */
/* fprintf(fp,
  "/Times-Roman findfont %d scalefont setfont\n",spectro->hchar);*/
 fprintf(fp,"%f 72 translate 90 rotate\n", 
         (spectro->yb[GRAM] - 1) * scale + 144);
 fprintf(fp,"%f %f scale\n",scale,scale);
 fprintf(fp,
  "/Times-Roman findfont %d scalefont setfont\n",spectro->hchar);
 draw_axes(spectro,spectro->info[GRAM],GRAM,fp);
 fprintf(fp,"grestore gsave\n");
 /* end axes */


  /* offset spectrogram */  
  fprintf(fp,"%f %f translate 90 rotate\n", 
  (spectro->wymax + yoffset(spectro) - 1) * scale + 144 ,
  72 + (xoffset(spectro) ) * scale); 
  fprintf(fp,"%f %f scale\n",scale,scale);
  fprintf(fp,"/str 1 string def\n");  
  fprintf(fp,"%d{",spectro->postcount);
  /* store each gray value or rle white in two ascii char*/
  /* that are read two at a time and stored as 1-255 in*/
  /* the array str when accessed as a single element (0 get)*/
  /* the integer value of the byte is used */
  fprintf(fp,
  "currentfile str readhexstring p 0 get dup 10 gt { 10 s /n e d n h idiv\n");
  /* include linefeeds in byte count */
  numb = spectro->postcount * 2 + spectro->postcount / 35 + 1;
  fprintf(fp,"%c%cBeginData: %d Hex Bytes\n",'%','%',numb);
  fprintf(fp,"t a /t e d n h mod f e a /f e d f h ge\n"); 
  fprintf(fp,"{/f f h s d /t t 1 a d} if}\n");
  fprintf(fp,"{g e get setgray t f b /f f 1 a d f h ge \n");
  fprintf(fp,"{/f f h s d /t t 1 a d} if} ifelse} repeat\n");
  
  numperl = 0;
  for(i = 0; i < spectro->postcount; i++){
    if(numperl == 35){
      fprintf(fp,"\n");/* need / at end of line */
      numperl = 0;
    }
    if(spectro->posti[i] > 15)
      {fprintf(fp,"%x",spectro->posti[i]);}
    else
      {fprintf(fp,"0%x",spectro->posti[i]);}
    numperl++;
   }
 /* add a newline */

 fprintf(fp,"\n%c%cEndData\n",'%','%');
 fprintf(fp,"showpage grestore grestore\n");
free(spectro->posti);
}

/*******************************************************************/
/*                       readgram(XSPECTRO *spectro)                 */
/*******************************************************************/
int readgram(XSPECTRO *spectro){
int spsize[2];
FILE *fp;
int gvsindex,i,totgvs;
unsigned char c, dim[4];
char filename[220], mess[300];


  /* return 0 if error reading file, 1 if ok */

  sprintf(filename,"%s.gram",spectro->wavename);
  /* error checking */
  if(!(fp = fopen(filename,"r")))
       return(0);  /* no file*/

 if(!fscanf(fp,"%d %d %f %f %d %d %f %d %d %f %f\n", &spectro->xgram,
      &spectro->wymax,&spectro->spers,&spectro->winms,
      &spectro->slice,&spectro->numav,&spectro->msdelta,
      &spectro->xpix,&spectro->ypix,&spectro->maxmag,&spectro->minmag)){

      if(cmdtextw )
      {writetext("error reading file\n");}
      else{printf("error reading file\n");}
      return(0);
    }/* error reading values */

  spectro->allgvs = (char *) malloc( sizeof(char) *
                 spectro->xgram * spectro->wymax);

  gvsindex = 0;
  totgvs = spectro->xgram * spectro->wymax;


  /* this format is much like the .gram format but it the integer*/
  /* values are written in ascii so 255 representing 245 continuous*/
  /* white pixels is written with 3 char instead of 1 byte*/


  /* this will eventually be stored instead of allgvs */
  /* over allocate */


/* expand the white space to make pixmap */
  while(gvsindex < totgvs){
     c = getc(fp);
     /*       if( (int)c < 0 ){
          if(cmdtextw )
          {writetext("error reading file\n");}
          else{printf("error reading file\n");}
          return(0);
	}*//* read error */
     if( (int)c < 10){
     spectro->allgvs[gvsindex] = (int)c;
     gvsindex ++;
   }/*gray*/
     else{
        for(i = 0; i < (int)c - 10; i++){
        if(gvsindex < totgvs){
        spectro->allgvs[gvsindex] = 0;
        gvsindex ++;
        }
      }

   }/*white*/
 }/*whole image*/


fclose(fp);

/* use for sizing window in ms later */
findsdelta(spectro);


sprintf(mess,"found %s\n",filename);
if(cmdtextw )
      {writetext(mess);}
else{printf("%s",mess);}


return(1);   /* .gram file exists no need to do dfts */
}/* end readgram */



/********************************************************************/
/*                   startup(spectro,name,wvname)                   */
/********************************************************************/
/* store the preferences file name and the wave file name */
/* read the preferences from the file or use defaults*/

void startup(spectro,name,wvname)
XSPECTRO *spectro;
char *name,*wvname;

{

  if(wvname != 0){
  strcpy(spectro->wavefile, wvname);
  }

  strcpy(spectro->fname, name);
/* if there is a .dat file read it */
/* should check home directory also */
  if(!read_data(spectro))
     set_defaults(spectro);


   spectro->savemsdelta = spectro->msdelta ;
   spectro->savewinms = spectro->winms ; 

/* no cursor position yet */ 
spectro->saveindex = -1;

/*as this number increases x get shorter relative to y*/
spectro->spratio = 650;

spectro->numcol = 9;

spectro->cinc = (spectro->maxmag - spectro->minmag) /
                (spectro->numcol - 2);

/* assume approx 80 pixels per inch*/

spectro->tickspace = 2;
spectro->axisdist  = 8;

/* spectro->xr and spectro->yb used for sizing window */
/* have to determine wxmax and wymax first*/ 

/* time offset in pixels*/
spectro->t0 = 0;
 
}/*end startup*/
/***************************************************************/
/*               set_defaults(spectro)                         */
/***************************************************************/
/* if couldn't find defs file or there are illegal values in*/
/*the file use these values*/


void set_defaults(spectro)
XSPECTRO *spectro;
{
     spectro->winms = 6.4;
     spectro->slice = 128;
     spectro->numav = 3;
     spectro->msdelta = 1.0;
     spectro->specms = 1300.0;
     spectro->minmag = 5.0;
     spectro->maxmag = 25.0;
     spectro->xpix = 1;
     spectro->fdifcoef = 100;
     spectro->sec = 0;

}/*end set_defaults*/
/***************************************************************/
/*               read_data(spectro)                            */
/***************************************************************/
/*read values from defaults  file and do some error checking*/
/*if one the values in the file is illegal the read is aborted*/
/*and the defaults in he code are used check log for notification*/

int read_data(spectro)
XSPECTRO *spectro;
{
FILE *fp;

       spectro->defserr = 0;

if((fp = fopen(spectro->fname,"r"))){

    if(!fscanf(fp,"%f",&spectro->winms)){
         spectro->defserr = 1;
         return(0);
       }
     while(getc(fp) != '\n'){ } /* ignore comment */

     if(!fscanf(fp,"%d",&spectro->slice)){
         spectro->defserr = 1;
         return(0);
       }
     if(spectro->slice != 16 &&
        spectro->slice != 32 &&
        spectro->slice != 64 && 
        spectro->slice != 128 && 
        spectro->slice != 256 &&
        spectro->slice != 512) { 
         spectro->defserr = 1;
         return(0);
       }
     while(getc(fp) != '\n'){ }

     if(!fscanf(fp,"%d",&spectro->numav)){
         spectro->defserr = 1;
         return(0);
       }
     if(spectro->numav < 1){
         spectro->defserr = 1;
         return(0);
       }
     while(getc(fp) != '\n'){ }
   
     if(!fscanf(fp,"%f",&spectro->msdelta)){
         spectro->defserr = 1;
         return(0);
       }
     if(spectro->msdelta <= 0 ){
         spectro->defserr = 1;
         return(0);
       }
     while(getc(fp) != '\n'){ }

    if(!fscanf(fp,"%f",&spectro->specms)){
         spectro->defserr = 1;  
         return(0);
       }
     while(getc(fp) != '\n'){ }

    if(!fscanf(fp,"%f",&spectro->minmag)){
         spectro->defserr = 1;
         return(0);
       }
     while(getc(fp) != '\n'){ }

    if(!fscanf(fp,"%f",&spectro->maxmag)){
         spectro->defserr = 1;
         return(0);
       }
     if(spectro->minmag >= spectro->maxmag){
         spectro->defserr = 1;
         return(0);
       }
     while(getc(fp) != '\n'){ }

    if(!fscanf(fp,"%d",&spectro->xpix)){
         spectro->defserr = 1;
         return(0);
       }
     while(getc(fp) != '\n'){ }


    if(!fscanf(fp,"%d",&spectro->fdifcoef)){
         spectro->defserr = 1;
         return(0);
       }
    if(spectro->fdifcoef < 0 || spectro->fdifcoef > 100){
         spectro->defserr = 1;
         return(0);
    }
     while(getc(fp) != '\n'){ }


   if(!fscanf(fp,"%d",&spectro->sec)){
         spectro->defserr = 1;
         return(0);
       }
   if(spectro->sec != 0 &&
      spectro->sec != 1){
         spectro->defserr = 1;
         return(0);
       }



    fclose(fp);
    return(1);
	 }

  else{
    return(0);
  }

}/*end read_data*/

/****************************************************************/
/*  add_spectro(XSPECTRO *spectro,char *defname,char *wavefile)  */
/****************************************************************/
void add_spectro(XSPECTRO *spectro,char *defname,char *wavefile)
{
   char str[500];
   int p;

   /*things that are dynamically allocated*/
   for(p = 0; p < NUM_WINDOWS; p++)
      spectro->info[p] = (INFO *) malloc( sizeof(INFO) );

spectro->allmag = NULL;
spectro->allgvs = NULL;
spectro->posti = NULL;
spectro->xim = NULL;
spectro->pix = NULL;
spectro->iwave = NULL;

spectro->locked = 0;

/* used to set sizwin */
spectro->hamm_in_ms[0] = 25.7;/*wc*/ 
spectro->hamm_in_ms[1] = 29.9;/*wd*/ /*f0*/
spectro->hamm_in_ms[2] = 25.6;/*ws*/
spectro->hamm_in_ms[3] = 25.6;/*wl*/

/* set these when sampling rate is known */
/*  hamm in samples */
/*spectro->params[0] = 257; wc*/ 
/*spectro->params[1] = 299; wd*/
/*spectro->params[2] = 256; ws*/
/*spectro->params[3] = 256; wl*/

spectro->params[4] = 14;  /*nc*/ /* # of coeff for LPC*/
spectro->params[5] = 0; /*fd*/ /* firstdifsw */
spectro->params[6] = 70;  /*b0*/
spectro->params[7] = 180; /*b1*/
spectro->params[8] = 300; /*bs*/
spectro->params[9] = 100; /*f1*/
spectro->params[10] = 124;/*f2*/
spectro->params[11] = 700;/*fl*/
spectro->params[12] = 450;/*sg*/
spectro->params[13] = 1;  /*nw*/
spectro->params[14] = 512;/*sd*/
spectro->params[15] = 10;/*kn*/

spectro->localtimemax = 0; /* EC 2/27/97 */
spectro->localfreqmax = 0; /* EC 2/27/97 */

spectro->option = 'S'; 
spectro->oldwintype = 0; /* cause window to be calulated first time */
spectro->oldsizwin = 0;

/*spectro->quadcount = 4; when 0 <= quadcount <=3 put 4 spectra on page*/

spectro->history = 0;

spectro->segmode = 1; /* play w to e*/

spectro->param_active = 0; /* widget for changing params not active*/

spectro->labels = NULL;  // init labels list

for(p = 0; p < NUM_WINDOWS; p++) spectro->info[p]->pixmap = 0;

startup(spectro,defname,wavefile);

wave_stuff(spectro);


}/* end  add_spectro*/
/*****************************************************************/
/*           strip_path(char *in, char *out)                     */
/*****************************************************************/
void strip_path(char *in, char *out){
/* make sure the two strings are allocated */

int p;

#ifdef VMS
    for(p = strlen(in) - 1; p >= 0; p --)
         if(in[p] == ']')  break;

   strcpy(out,&in[p+1]);

#else
/* assume unix*/

    for(p = strlen(in) - 1; p >= 0; p --)
         if(in[p] == '/')  break;

   strcpy(out,&in[p+1]);
#endif



}/* end strip_path */


/*****************************************************************/
/*           void wave_stuff(XSPECTRO *spectro);                   */
/******************************************************************/
void wave_stuff(XSPECTRO *spectro){
int p;
char str[500];
float datatmp;

/* this function is called to update wavefrom information
   associated with windows, (replace and read wave) */


/* set insize to totsamp so dfts done on whole file once */
/* and find xpix and ypix */
/* wavefile includes .wav spec */
/* remove path and .wav  from wavefile*/

  if(spectro->wavefile[0] =='?'){

  /* not going to open .wav file*/
  /* iwave will be NULL; */
  }


  else{
    strcpy(str,spectro->wavefile);
    str[strlen(str) - 4] = '\000';/* remove .wav*/

  strip_path(str, spectro->wavename);


  spectro->iwave = getWavWaveform(spectro->wavefile,
                &(spectro->spers),&(spectro->totsamp), spectro->swap);
  
  spectro->insize = spectro->totsamp;

  spectro->wavemax = 0.0;
  spectro->wavemin = 0.0; 

  spectro->wvfactor[0] = 1.0; /* scale in y for waveform window 0*/
  spectro->wvfactor[1] = 1.0; /* scale in y for waveform window 1*/

/* divide by 16 for spectrogram
and find min and values used for drawing waveform
*/

  for(p = 0; p < spectro->insize; p++){
      datatmp = (float) (spectro->iwave[p] >> 4);
         if(datatmp > spectro->wavemax)
                   {spectro->wavemax = datatmp;}
         if(datatmp < spectro->wavemin)
                   {spectro->wavemin = datatmp;}
    }

/* wavemin is negative*/
if(spectro->wavemin < 0 && spectro->wavemax > 0){
 if((spectro->wavemin * spectro->wavemin) >
    (spectro->wavemax * spectro->wavemax)){
      spectro->wavemax = - spectro->wavemin;
     }
 else if (spectro->wavemin == spectro->wavemax) {
      spectro->wavemin = 6000;
      spectro->wavemin = - spectro->wavemax;
 }
 else {
      spectro->wavemin = - spectro->wavemax;
    }
}

   /* find out how many samples to use in spectrum hamming
      window now that sampling rate is known               */
  for(p = 0; p < 4; p++){
       spectro->params[p] = 
            spectro->hamm_in_ms[p]  * spectro->spers / 1000.00 + .5;

  }

      /* the shifting may move to wave_util */
}/* opened a .wav file */

/* assume for now that startup is always no spectrogram */
if(spectro->spectrogram){
 if(readgram(spectro) == 0){
    findsdelta(spectro);

    findypix(spectro);

    calculate_spectra(spectro);

    writegram(spectro); 
    
     }/* calculate and save */
  /* see how many ms to display on screen */
  win_size(spectro);
 }/* read or generate spectrogram */
else{
/* make up some dimensions for spectrogram window */
spectro->xr[GRAM] = 100;
spectro->yb[GRAM] = 100;
}/* not going to draw a spectrogram but a
    widget will still be created for it and
    it may be managed later
*/



}/* wave_stuff */
/*************************************************************/
/*                    addtolist()                            */
/*************************************************************/
void addtolist(){
/* uses the global list of spectros, allspectros  */
/* adds a new spectro structure to the list*/

XSPECTRO *spectro;
XSPECTRO **tmplist;
int i;

  tmplist = (XSPECTRO **) malloc( sizeof( XSPECTRO *) * (allspectros.count));

   for( i = 0; i < allspectros.count; i++)
     tmplist[i] = allspectros.list[i]; 
  
  free(allspectros.list);
  allspectros.list = 
        (XSPECTRO **) malloc( sizeof( XSPECTRO *) * allspectros.count+1);

     for( i = 0; i < allspectros.count; i++)
           allspectros.list[i] = tmplist[i];
     allspectros.list[i] = (XSPECTRO *) malloc( sizeof(XSPECTRO) ); 
     allspectros.count = allspectros.count + 1;

}/* end addtolist */
/******************************************************************/
/*             createwindows(XSPECTRO *spectro);                  */
/******************************************************************/
void createwindows(XSPECTRO *spectro){
/* create all graphics windows for a waveform */
/* add_spectro needs to be called first */
  int i,n;
  Arg args[15],mharg[1];
  char title[200], name[15];
  Widget fbwdialog;

  /* create a text window for f+bw output*/
  /* for now the parent is the text window application so that
     this window can remain active for cut and paste*/

  n = 0;
       
  XtSetArg(args[n],XmNdeleteResponse, XmUNMAP); n++;
  fbwdialog = XtCreatePopupShell("XKL_VALUES",topLevelShellWidgetClass,
                                    cmdw,args,n);
  createtext(fbwdialog,&spectro->fbw_text);
  
  for(i = 0; i < NUM_WINDOWS; i++){
     sprintf(name,"window_%d",i);

     n = 0;
     XtSetArg(args[n], XmNallowShellResize,True);n++;

     if (i == GRAM) { /* spectrogram */

      /* don't know menu_height yet, but resize will fix that later*/
      /* guess for now  */

	/* get menu_height from window_0 */
	XtSetArg(mharg[0],XmNheight,&spectro->menu_height);
	XtGetValues(XtParent(spectro->buttons[0]),mharg,1);
	spectro->menu_height += 10; /* space between menu and draw widget*/

	XtSetArg(args[n], XmNwidth, spectro->xr[i] );n++;
	XtSetArg(args[n], XmNheight,spectro->yb[i]+spectro->menu_height);n++;
	/* plus menu_height */
	/* eliminate resize and maximize from frame pg.12-15 osf prog. guide*/
	XtSetArg(args[n], XmNmwmDecorations,69);n++;/* 1 + 4 + 64 */
	/* eliminate resize and maximize from menu */
	XtSetArg(args[n], XmNmwmFunctions,19);n++;/* 1 + 2 + 16 */
     }/* spectrogram */
     
     XtSetArg(args[n], XmNdeleteResponse,  XmDO_NOTHING );n++;
     
     /* so menu_bar doesn't change size*/
     XtSetArg(args[n], XmNminWidth,MINSIZE);n++;
     /* OSF programmer's reference 1-75  for widget deallocation(/
	XtSetArg(args[n], XmNinitialResourcesPersistent,False );n++;*/
	/* toplevel shell */
     spectro->toplevel[i] = XtCreatePopupShell(name, topLevelShellWidgetClass, 
					       application, args, n);
       
     n = 0;
     
     spectro->mainw[i] = XmCreateMainWindow(spectro->toplevel[i],
					    "main", args, n);
     XtManageChild(spectro->mainw[i]);
     
     CreateMenus(spectro->mainw[i],spectro,&spectro->buttons[i],
		 &spectro->menu_bar[i],1);

     /* find out window size and origin before sgi puts it in the 
          wrong place */
     
     n = 0;
     if(i != GRAM){
	/* spectrogram sizing controlled by ms */
	XtSetArg(args[n],XmNwidth,&spectro->xr[i]); n++;
	XtSetArg(args[n], XmNheight,&spectro->yb[i]);n++;
     }
     
     XtSetArg(args[n],XmNx,&spectro->ox[i]); n++;
     XtSetArg(args[n], XmNy,&spectro->oy[i]);n++;


     XtGetValues(spectro->toplevel[i] ,args,n);

       /* find height of drawing area (window height - menu height)*/
     if(i != GRAM)
	spectro->yb[i] = spectro->yb[i] - spectro->menu_height;
     
     /* set the resources for draw widget */
     n = 0;

     XtSetArg(args[n], XmNwidth, spectro->xr[i]);n++;
     XtSetArg(args[n], XmNheight,spectro->yb[i]);n++;
     spectro->draww[i] = XmCreateDrawingArea(spectro->mainw[i],
					     "draww", args, n);

     XtAddCallback(
		   spectro->draww[i],  /* widget */
		   XmNexposeCallback,  /* callback name */
		   expdraww,           /* callback procedure */
		   (XtPointer) spectro   /* client data */
		   );


     XtAddCallback(
		   spectro->draww[i], /* widget */
		   XmNinputCallback,  /* callback name */
		   dobutton,         /* callback procedure */
		   (XtPointer) spectro     /* client data */
		   );

     XtAddCallback(
		   spectro->draww[i], /* widget */
		   XmNresizeCallback,  /* callback name */
		   doresize,         /* callback procedure */
		   (XtPointer) spectro     /* client data */
		   );

     XtManageChild( spectro->draww[i] );
     XmMainWindowSetAreas(spectro->mainw[i],spectro->menu_bar[i],NULL,
                          NULL,NULL,spectro->draww[i]); 
     

    
     spectro->normstate[i] = 1; /* start off with normal state */
     /* may have problems if people have .Xdefaults with different*/
     /* initial state */
     
  }/* create all widgets for one waveform */
  
  dotitles(spectro);
  recstart(spectro,0);
  /* make appropriate menu entries sensitve */

}/* end createwindows*/
/***************************************************************/
/*             mapwindows(XSPECTRO *spectro)                    */
/***************************************************************/
void  mapwindows(XSPECTRO *spectro)
{
   /* application must be realized */
   int i,n;
   Arg args[3];
   INFO *info;
   Dimension gramw,gramh;
   XWindowAttributes windowatt;
   
   for(i = 0; i < NUM_WINDOWS; i++){
      if(i == GRAM && !spectro->spectrogram){/* no spectrogram*/}
      else {
      XtManageChild(spectro->toplevel[i]);
      
      
      /* overrides interactive placement */
      XMoveWindow(XtDisplay(spectro->toplevel[i]),
		  XtWindow(spectro->toplevel[i]),
		  spectro->ox[i],spectro->oy[i]);
      
      setupcmap(spectro->draww[i],spectro->info[i]);   
   }
      /* override resizing of spectrogram by interactive placement */
      if(i == GRAM && spectro->spectrogram){
	 resizegram(spectro);
      }/* GRAM */
      
      else if(i != GRAM){
	 make_pixmap(spectro->info[i],spectro,i);
      }
      
      info = spectro->info[i];
      
      
      /*assume for now that structure notify is selected for toplevel*/
      /*since spectrogram window wasn't managed the following code crashed*/
      
      /*
	 StructureNotifyMask:
	 when selected iconify sends UnmapNotify(18)
	 and normalize sends MapNotify(19)
	 */
/*
   
   XGetWindowAttributes(XtDisplay(spectro->toplevel[i]),
   XtWindow(spectro->toplevel[i]), &windowatt);
   
   XSelectInput(XtDisplay(spectro->toplevel[i]),
   XtWindow(spectro->toplevel[i]) , 
   (windowatt.your_event_mask) | StructureNotifyMask);
   
   */
   }/* set up all pixmaps */
   
   
   new_spectrum(spectro);
   draw_spectrum(spectro, 0, NULL);/* this draws onto the pixmap */ 
   
}/* end  mapwindows */


/****************************************************************/
/*                delete_spectro(XSPECTRO *spectro)              */
/****************************************************************/
void delete_spectro(XSPECTRO *spectro){
int i,n;
XSPECTRO **tmplist;


  if(allspectros.count > 1 ){

  /* unmanage any popups that may have spectro as client_data*/
  /* for now all others grab */
   if(spectro->param_active)
      { XtUnmanageChild(param_tl);  } 

 tmplist = (XSPECTRO **) malloc( sizeof( XSPECTRO *) * (allspectros.count- 1));

  /* update the allspectros list */
  n = 0;
  for( i = 0; i < allspectros.count; i++){
     if(allspectros.list[i] != spectro){
     tmplist[n] = allspectros.list[i];
     n++;  
     }     
    }

  allspectros.count = allspectros.count - 1;
  free(allspectros.list);
  allspectros.list = 
       (XSPECTRO **) malloc( sizeof( XSPECTRO *) * allspectros.count);
  for( i = 0; i < allspectros.count; i++)
      allspectros.list[i] = tmplist[i];

  free(tmplist);

  delete_wave(spectro);

  for(i = 0; i < NUM_WINDOWS; i++){

       XtRemoveAllCallbacks(
                  spectro->toplevel[i], /* widget */
                  XmNdestroyCallback  /* callback name */
                  );

     XtDestroyWidget(spectro->toplevel[i]);
     if(spectro->info[i]->pixmap) 
     {XFreePixmap(XtDisplay(spectro->toplevel[i]),spectro->info[i]->pixmap);
      }
     if(spectro->info[i])
        {free(spectro->info[i]); spectro->info[i] = NULL;}
     }
    XtDestroyWidget(XtParent(XtParent(spectro->fbw_text)));

  

  free(spectro);

 }/* more than one wave file */

else{   
  exitwarning();
  }/* if only one wave file exit program */
 
}/* end delete_spectro*/
/****************************************************************/
/*              delete_wave(XSPECTRO *spectro)                  */
/****************************************************************/
void delete_wave(XSPECTRO *spectro) {
  int i;
  LABEL *lp;

/* free all wave form related memory on spectro structure */
/* leave widgets alive*/

/* iwave is allocated by wave_util */
  if(spectro->allmag)
      free(spectro->allmag);
  if(spectro->allgvs)
      free(spectro->allgvs);
  if(spectro->posti)
      free(spectro->posti);
  if(spectro->iwave)
      free(spectro->iwave);
  if(spectro->xim)     
       XDestroyImage(spectro->xim);
  /*  if(spectro->pix)
      free(spectro->pix); */
  while (spectro->labels) {
      lp = spectro->labels;
      spectro->labels = spectro->labels->next;
      free(lp);
  }

spectro->allmag = NULL;
spectro->allgvs = NULL;
spectro->posti = NULL;
spectro->xim = NULL;
spectro->pix = NULL;
spectro->iwave = NULL;


}/* end delete_wave */
/****************************************************************/
/*                 win_size(XSPECTRO *spectro)                   */
/****************************************************************/
void win_size(XSPECTRO *spectro){


if(spectro->specms > (float)spectro->totsamp / spectro->spers * 1000.0 ){
     spectro->specsize = spectro->totsamp;
     spectro->specms = spectro->specsize / spectro->spers * 1000;
   }
else
    {spectro->specsize = spectro->specms * spectro->spers / 1000;}

   /* calculate how many estimates will be displayed in window */

    spectro->xmaxests = (spectro->specsize - spectro->winsamps) /
                   ((spectro->winsamps - spectro->ovlap) * spectro->numav);


    spectro->wxmax = spectro->xpix  * (spectro->xmaxests ) ;

    /* pixmap is limited */
     if(spectro->wxmax > spectro->devwidth - 100){
           spectro->wxmax = spectro->devwidth - 100;


      spectro->xmaxests = spectro->wxmax / spectro->xpix;

      spectro->specsize = spectro->xmaxests *
     ((spectro->winsamps - spectro->ovlap) * spectro->numav) + 
      spectro->winsamps;

       spectro->specms = spectro->specsize / spectro->spers * 1000;
      
      }/* spectrogram wider that screen */
   

 
    spectro->sampsused[GRAM] = spectro->winsamps + spectro->xmaxests  *
                   ((spectro->winsamps - spectro->ovlap) * spectro->numav);

    findxryb(spectro);

  
}/* end find_win_size*/
/***************************************************************/
/*             findxryb(XSPECTRO *spectro)                      */
/***************************************************************/
void findxryb(XSPECTRO *spectro) {
/* determine drawing area width and height for spectrogram */
/* controlled by number ms */

/* 6 char before y axis, 4 char on the right */
spectro->xr[GRAM] = spectro->wxmax + spectro->wchar * 10 +
                 spectro->axisdist;
/* added one more hchar here for time label */
spectro->yb[GRAM] = spectro->wymax + 
           3 * spectro->hchar  + spectro->axisdist;

}/* end  findxryb */

/***************************************************************/
/*                  findsdelta(spectro)                       */
/***************************************************************/
void findsdelta(spectro)
XSPECTRO *spectro;
{
/*find number of samples for Hamming window from window in ms*/

 spectro->winsamps = spectro->winms * spectro->spers / 1000;
 /* default to fft size if window is larger*/
 if(spectro->winsamps > spectro->slice){
     spectro->winsamps = spectro->slice;
     spectro->winms = (float)spectro->slice * 1000.0 /spectro->spers;
   }

 spectro->sdelta = spectro->msdelta * spectro->spers / 1000;

 if((spectro->winsamps - spectro->sdelta) > 0)
     spectro->ovlap = spectro->winsamps - spectro->sdelta;
 else
   spectro->ovlap = 11;

}/* end findsdelta */
/**************************************************************/
/*                   findypix(XSPECTRO *spectro)               */
/**************************************************************/

void  findypix(XSPECTRO *spectro){

/*called before calculation */
/* set up ypix based on xpix only */
/* change xpix if it forces ypix off screen*/
/* this code tries to preserve a standard(lspecto) aspect ratio
lspecto displays 650 ms in the length of the y axis
*/

/* calculate all dfts */

spectro->specsize = spectro->totsamp;  

spectro->xmaxests = (spectro->specsize - spectro->winsamps) /
                   ((spectro->winsamps - spectro->ovlap) * spectro->numav);


spectro->sampsused[GRAM] = spectro->winsamps + spectro->xmaxests  *
                   ((spectro->winsamps - spectro->ovlap) * spectro->numav);



spectro->wxmax = spectro->xmaxests * spectro->xpix;

spectro->wymax = spectro->wxmax * spectro->spratio / 
      (spectro->sampsused[GRAM] / spectro->spers * 1000);

/*make sure it is less than devheight*/
if(spectro->wymax > spectro->devheight ){
      spectro->wymax = spectro->devheight ;
      spectro->ypix = spectro->wymax / (spectro->slice / 2);

     /* change xpix */
      spectro->wxmax = (spectro->sampsused[GRAM] / spectro->spers * 1000) /
      spectro->spratio  * spectro->wymax;

   }
else{
  spectro->ypix = spectro->wymax / (spectro->slice / 2);
  }

}/* findypix */


/***************************************************************/ 
/*                  calculate_spectra(spectro)                 */
/***************************************************************/
void calculate_spectra(spectro)
XSPECTRO *spectro;

{
  int        i, j, m, k,l, index;
  float      a,*mag; 
  double     tempflt;   
  COMPLEX    *samp;
  float      t,fq,xinc,yinc;
  unsigned long gv;    /* gray value index*/
  float fd;  /* convert the fdifcoef to float between 0 and 1*/


  int f,fdj,fdk;
  int halfslice, halfwin;
  float xri, xii; 
  char str[800];
  int gvsindex,totgvs;
  int y,fmax,inc;

if(cmdtextw)
      {writetext("calculating...\n"); XSync(XtDisplay(cmdtextw),False);}
else{printf("calculating...\n");}

 halfslice = spectro->slice >> 1;
 halfwin = spectro->winsamps / 2;

/*  Calculate the number of samples in each estimation and
      the number of estimations in the data set */

 spectro->estsize = (spectro->winsamps - spectro->ovlap) *
       (spectro->numav-1) + spectro->winsamps;

 
 spectro->numests = (spectro->insize - spectro->winsamps)/
                  ((spectro->winsamps - spectro->ovlap) * spectro->numav);

 t = 0.0;  /* for drawing only */

/* pre-emphasis*/

  fd = 0.01 * (float) spectro->fdifcoef;


  /*allocate space for all the spectra (1/2 slice * numests) */
  if(spectro->allmag)
      free(spectro->allmag);
  spectro->allmag = (float *) malloc(sizeof(float) *
                    spectro->slice / 2 * spectro->numests);
 
  mag = (float *) malloc(sizeof(float) * spectro->slice);
  samp = (COMPLEX *) malloc(sizeof(COMPLEX) * spectro->slice);
  if (!mag || !samp){
    printf("\n  Memory allocation error.\n");
    exit(1);
  }


/* store gray values */ 
  totgvs = spectro->slice / 2.0 * spectro->numests *
           spectro->xpix * spectro->ypix;

    if(spectro->allgvs)
      free(spectro->allgvs);
  /* make background zeros */
#ifdef _REPLACED_WITH_CALLOC
  spectro->allgvs = (char *) malloc(totgvs);
/***debug**/
  for(i=0; i < totgvs; i++)
       spectro->allgvs[i] = (char)0;
#endif
  spectro->allgvs = (char *)calloc(sizeof(char),totgvs);

  gvsindex = 0;


/* put #points in DFT in window title 
sprintf(str,"%s, %.1fms win.",spectro->wavename,spectro->winms);
*/

  for (i=0; i<spectro->numests; i++){

   for (k=0; k<spectro->slice; k++) {mag[k] = 0;}

    for (j=0; j<spectro->numav; j++){

        if(fd > 0.0){
            /*Use first difference preemphasis by D.Klatt*/
            index = i*(spectro->winsamps-spectro->ovlap)*spectro->numav +
            j*(spectro->winsamps-spectro->ovlap);
                if(index == 0){
                  for( f = 0; f < spectro->winsamps; f++){
                    samp[f].real = (float)(spectro->iwave[f] >> 4);
                    samp[f].imag = 0.0;
	           }
	        }/*index = 0*/

            else{
            k = 0;

            for( f = 0; f < halfwin; f++){                
            fdj = f + f ;
            fdk = fdj + 1;

                xri = (float)(spectro->iwave[index + fdj] >> 4)
                 - fd * (float)(spectro->iwave[index + fdj - 1] >> 4);

                xii = (float)(spectro->iwave[index + fdk] >> 4) 
                 - fd *  (float)(spectro->iwave[index + fdj] >> 4);

	    samp[k].real = xri; samp[k].imag = 0.0;
	    samp[k+1].real = xii; samp[k + 1].imag = 0.0;
	    k+= 2;
	  }
	}/*don't use iwave[-1]*/
      }/* preemphasis */


     else{
        for (k=0; k<spectro->winsamps; k++){
       
          /* index = i*spectro->estsize +*/ 

          /*debug dhall*/
          index = i*(spectro->winsamps-spectro->ovlap)*spectro->numav +
            j*(spectro->winsamps-spectro->ovlap) + k;

          samp[k].real = (float)(spectro->iwave[index] >> 4);
          samp[k].imag = 0;
	}

      }/* no preemphasis */


      if(spectro->slice > spectro->winsamps){
           /*pad with zeros like lspecto*/
      for( k = spectro->winsamps; k < spectro->slice; k++){
          samp[k].real = 0.0;
          samp[k].imag = 0.0;
        }
    }/* padding */


      ham(samp,spectro->winsamps);


      m = ilog2(spectro->slice);
      fft(samp,m);

      a = (float) spectro->slice*spectro->slice;
      for (k=0; k<spectro->slice; k++){
        tempflt  = samp[k].real * samp[k].real;
        tempflt += samp[k].imag * samp[k].imag;
        tempflt = tempflt/a;
        mag[k] += tempflt;
        
      }
    }   
    fq = 0.0; /* for drawing */


/* Take log after averaging the magnitudes.  */
    for (k=0; k<spectro->slice/2; k++){
      mag[k] = mag[k]/spectro->numav;
      mag[k] = 10*log10(XAM(mag[k],1.e-14));
      
      /* store all spectra in one array*/
      spectro->allmag[i*spectro->slice/2+k] = mag[k];
      
      spectro->allmag[i * spectro->slice / 2 + k] = mag[k];
      
      /*DRAWING STUFF*/
      
      if(k > 0 && i > 0){
	yinc = (spectro->allmag[(i-1)*spectro->slice/2 + k ] -
		spectro->allmag[(i-1)*spectro->slice/2 + k - 1]) /
                  (float)(spectro->ypix );
	
	xinc = (spectro->allmag[i*spectro->slice/2 + k - 1] -
		spectro->allmag[(i - 1)*spectro->slice/2 + k - 1]) /
                  (float)(spectro->xpix );
	
	
	for(j = 0; j < spectro->xpix; j++){
	  for(l =0; l < spectro->ypix; l++){
	    
	    
	    
	    if((spectro->allmag[(i-1)*spectro->slice/2 + k - 1] + j * xinc + l * yinc)
	       > spectro->minmag) {
	      
	      
	      if((spectro->allmag[(i-1)*spectro->slice/2 + k - 1] + j * xinc + l * yinc)
		 >= spectro->maxmag)
		{ gv = spectro->numcol;}
	      
	      else { gv = (spectro->allmag[(i-1)*spectro->slice/2 + k - 1]
			   - spectro->minmag + j * xinc + l * yinc) / spectro->cinc + 2;
		     
		   }
	      
	      gvsindex = (int)(t) * spectro->ypix
		* halfslice + (int)fq;
	      spectro->allgvs[gvsindex] = (char)gv;
	      
	    }/* only draw if not white */
	    fq += 1.0;
	  }/*ypix freq*/
	  t+= 1.0;
	  fq -= (float)spectro->ypix;
	}/*xpix time*/
	t -= (float)spectro->xpix;
	fq += (float)spectro->ypix;
      }/*wait till second spectrum to draw*/
    }/*k*/
   if(i > 0 )
     t += (float)spectro->xpix;
 }/* end numests */
  
  /*  free(mag); free(samp); */
  
  spectro->xgram = spectro->numests * spectro->xpix;
  spectro->wymax = spectro->slice / 2 * spectro->ypix;
  

}/*end calculate_spectra*/
/******************************************************************/
/*                    draw_spectrogram(spectro)                   */
/******************************************************************/
void draw_spectrogram(spectro)
XSPECTRO *spectro;
{
  int        gv,gvsindex,totgvs;
  int        i, j, k, l,halfslice; 
  float      fq,xinc,yinc,t;/* for interpolation */

 halfslice = spectro->slice >> 1;
/* store gray values */ 

  if(spectro->allgvs) {free(spectro->allgvs);}

  /* make background zeros */
  spectro->allgvs = (char *) calloc(
           (size_t)(spectro->xgram * spectro->wymax),sizeof(char)  );

  gvsindex = 0;
  totgvs = spectro->xgram * spectro->wymax;

  t = 0.0; 

  for( i = 0; i < spectro->numests - 1; i++){
    fq = 0.0;
    
     for(k = 0; k < spectro->slice/2 - 1; k++){

      yinc = (spectro->allmag[i*spectro->slice/2 +k+1] -
               spectro->allmag[i*spectro->slice/2 +k]) /
                  (float)(spectro->ypix );


      xinc = (spectro->allmag[(i+1)*spectro->slice/2 +k] - 
               spectro->allmag[i*spectro->slice/2 +k]) / 
                  (float)(spectro->xpix );


    for(j = 0; j < spectro->xpix; j++){
       for(l =0; l < spectro->ypix; l++){

       if((spectro->allmag[i*spectro->slice/2 +k] + j * xinc + l * yinc) 
                        > spectro->minmag) {

       if ((spectro->allmag[i*spectro->slice/2 +k] + j * xinc + l * yinc) 
                        >= spectro->maxmag) 
              { gv = spectro->numcol;}

       else {gv = (spectro->allmag[i*spectro->slice/2 +k] 
             - spectro->minmag + j * xinc + l * yinc) / spectro->cinc + 2;}


               gvsindex = (int)(t) * spectro->ypix
                                   * halfslice + (int)fq;
               spectro->allgvs[gvsindex] = (char)gv;


       }/* only draw if not white */
       fq += 1.0;
      }/*ypix freq*/
      t+= 1.0;
      fq -= (float)spectro->ypix;
     }/*npix time*/
     t -= (float)spectro->xpix;
     fq += (float)spectro->ypix;
    }/*k*/
   t += (float)spectro->xpix;

  }/* end numest */
}/*draw_spectrogram*/

/***************************************************************/
/*                 remapgray(XSPECTRO *spectro)                 */
/***************************************************************/   
void remapgray(XSPECTRO *spectro){
INFO *info;

     info = spectro->info[GRAM];

   if(spectro->allmag == NULL){
      findsdelta(spectro);
      findypix(spectro);
      calculate_spectra(spectro);
      win_size(spectro);
     }/* used .gram file to start with */
   else{
     draw_spectrogram(spectro);
      }
    writegram(spectro);
    XDestroyImage(spectro->xim);
    /* XDestroyImage doesn't set it to NULL*/
    /* assume for now that this deallocates spectro->pix */ 
    spectro->xim = NULL;

    make_pixmap(info,spectro,GRAM);

    draw_cursor(spectro,GRAM,info);

 }/* end remapgray */

/****************************************************************/
/*           void writegram(XSPECTRO *spectro)                  */
/****************************************************************/
void writegram(XSPECTRO *spectro){

  FILE *fp;
  char c;
  int totgvs,i;
  int gvsindex;
  int numzeros;
  char str[200],mess[300];
 
  /* stub (don't write gram file) */
  if (1) return;

  /*store .gram  file */
  sprintf(str,"%s.gram",spectro->wavename);


  if(!(fp = fopen(str,"w")) ){
     sprintf(mess,"can't open %s\n",str);
     if(cmdtextw )
         {ShowOops(mess);}
      else{printf("%s",mess);}
    return;
   }


  fprintf(fp,"%d %d %.2f %.2f %d %d %.2f %d %d %.2f %.2f\n", spectro->xgram,
      spectro->wymax,spectro->spers,(float)spectro->winms,
      spectro->slice,spectro->numav,spectro->msdelta,
      spectro->xpix,spectro->ypix,spectro->maxmag,spectro->minmag);


 /* The spectrogram image is stored so that it can be transferred*/
 /* in ascii mode.  The white space is run length encoded with up*/
 /* to 245 white pixels stored in a byte.  If a byte is greater  */
 /* than 10 then it represents it's value -10 white pixels otherwise*/
 /* the byte represents the gray scale value of a single pixel    */
 /* the gray values range from 2 thru 9 (light to dark)           */

  totgvs = spectro->xgram * spectro->wymax;

  gvsindex = 0;


 for (i=0; i < totgvs; i++) {
  if ((int)spectro->allgvs[i] == 0) { /* white space */
    numzeros = 0;
    while ((i < totgvs) && ((int)spectro->allgvs[i] == 0) 
	   && (numzeros < 245)) {
      numzeros++;
      i++;
    }
   c = (char) (numzeros + 10);
   fputc(c, fp);
   i = i - 1;
  }
 else {
   c = spectro->allgvs[i];
   fputc(c,fp);
 }
}
 fclose(fp);
}

/****************************************************************/
/*       make_pixmap(INFO *info,XSPECTRO *spectro,int index)     */
/****************************************************************/      
void make_pixmap(INFO *info,XSPECTRO *spectro,int index){

   if(info->pixmap){ XFreePixmap(info->display,  info->pixmap);}


   /* create pixmap */
   info->pixmap = XCreatePixmap(info->display,
        info->win,spectro->xr[index],spectro->yb[index],
        DefaultDepth(info->display,info->screen_num) );

   if(spectro->iwave == NULL){ 
   /*just clear the pixmaps no waveform to draw*/

   XSetForeground(info->display,info->gc,
                  WhitePixel(info->display,info->screen_num)  );

   XFillRectangle(info->display,info->pixmap,
     info->gc, 0,0,
     spectro->xr[index],spectro->yb[index]);


   return;
   }

   switch(index){

   case 0:
   case 1:
   wave_pixmap(info, spectro, 1, index,NULL);
   break;

   case 2:
   spectro_pixmap(info, spectro, index,NULL);
   break;
  

  case 3:
   spectrum_pixmap(info,spectro,NULL);
   break;

   }/*switch*/

 }/* make_pixmap */


/*****************************************************************
 *
 * void spec_cursor(XSPECTRO *spectro)
 *
 * Update spectrum cursor when there is a new cursor location, and
 * it is a valid location.
 *
 *****************************************************************/

void spec_cursor(XSPECTRO *spectro)
{
  INFO *info;
  int grdyb, grdoy;
  int grdxr;
  float  halfsrate;
  int pointx;
  static int prevLen = 0;
  char str[80],tmp[80];


  info = spectro->info[SPECTRUM];
  XCopyArea(info->display,info->pixmap, info->win, info->gc,
            0, 0, spectro->xr[SPECTRUM], spectro->yb[SPECTRUM], 0, 0);
  
  if(spectro->specamp != -1) { /* valid cursor location */
    grdoy = 0;
    grdyb = (float)spectro->yb[SPECTRUM] - 2 * spectro->hchar - 1;
      
    /* draw cursor */
    
    XSetLineAttributes(info->display,info->gc,1,LineOnOffDash,
		       CapNotLast,JoinMiter);
    XDrawLine(info->display, info->win,
	      info->gc,spectro->spec_cursor, grdoy,spectro->spec_cursor,
	      grdyb);
    XSetLineAttributes(info->display,info->gc,1,LineSolid,
		       CapNotLast,JoinMiter); 

    /* 
     * Write out frequency and amplitude of cursor in spectrum window 
     */

    grdxr = (int) (float)spectro->xr[SPECTRUM] * SPEC_XR;
    grdyb = (int) (float)spectro->yb[SPECTRUM] - 20;

   /* 
    * Clear text area 
    */

    XSetForeground(info->display,info->gc,
		   WhitePixel(info->display,info->screen_num));
	  
    XFillRectangle(info->display,info->win,
		   info->gc, grdxr+10, grdyb-50, 105, 50);

    /*
     * Write out text
     */

    XSetForeground(info->display,info->gc,
		   BlackPixel(info->display,info->screen_num));

    sprintf(str, "%6d Hz %3.1f dB", (int) spectro->specfreq, spectro->specamp);
    XDrawString(info->display, info->win, info->gc,
		grdxr + spectro->wchar, grdyb - 2*spectro->hchar, 
		str, strlen(str)); 

    if (spectro->savspecamp != -1.0) {
      sprintf(str,"%6d Hz %3.1f dB", (int) spectro->savspecfreq,
	      spectro->savspecamp);
      XDrawString(info->display, info->win, info->gc,
		  grdxr + spectro->wchar, grdyb - spectro->hchar, 
		  str, strlen(str)); 
    }
  }
}

/*****************************************************************
 *
 * void update(INFO *info,XSPECTRO *spectro,int index)
 *
 * Update spectrum when there is a new cursor location.
 *
 *****************************************************************/

void update(INFO *info, XSPECTRO *spectro, int index)
{
  if (spectro->normstate[index]) { /* window is not iconified */    
    if (index == SPECTRUM) {
      new_spectrum(spectro); 
      draw_spectrum(spectro, 0, NULL); /* this draws onto the pixmap */
      spec_cursor(spectro);
    }
    else {
      draw_cursor(spectro,index,info);
    }
  } 
}

/*****************************************************************/
/*  spectro_pixmap(INFO *info,XSPECTRO *spectro,int index);       */
/*****************************************************************/
void spectro_pixmap(INFO *info,XSPECTRO *spectro,int index){

int i,t,fq;
int fmax, j, y;
float inc, fy;
int bytesperline,n,l,bitspp;


/* just do this once  */
if(spectro->xim == NULL){
  /*
  assume spectro->pix is deallocated by XDestroyImage
  if(spectro->pix)
    {free(spectro->pix); }
  */


/* don't want gridlines stored in allgvs because */
/* it would expand postscript file, for now*/
/* the gridlines are drawn on top of the image on screen*/
/* go through and put gridlines in */

/*
   fmax = spectro->spers/2000.0;
   inc = (float)spectro->wymax/(spectro->spers/2000.0);

   fy = 0; y = fy;

  for( i = 0; i <= fmax; i++){
       for( j = 0; j < spectro->xgram; j++){
         if((int)spectro->allgvs[y + j * spectro->wymax] == 0){
            spectro->allgvs[y + j * spectro->wymax] = 1;
	   }
       }
       fy += inc;
       y = fy + .5;
     }
*/
/*gridlines*/


 if(DefaultDepth(info->display,info->screen_num) == 1){

  spectro->pix  = (char *) malloc( sizeof(char) * 
                  (spectro->xgram/8 + 1) * spectro->wymax);


/* change scan order from spectrum by spectrum to 0,0 upper left scan in x */
  i = 0;    
  for(fq = spectro->wymax - 1; fq >= 0; fq += -1){
   l = 0;
   for(t =  0; t < spectro->xgram; t+= 8){
     spectro->pix[i] = (char)0;
     for(n = 0; n < 8; n++){
          if( l < spectro->xgram){
          spectro->pix[i] +=
         (char) (info->color[(int)spectro->allgvs[(t + n) *
                             spectro->wymax + fq]].pixel) << n;
	  }
          l++;
          }/*8 bits*/
         i++;
       }/* t */
  }/* fq */
  

  bytesperline = spectro->xgram / 8; 
  if(spectro->xgram % 8)  bytesperline++;
  bitspp = 1;

  spectro->xim =  XCreateImage(info->display,
           DefaultVisual(info->display,info->screen_num),
           DefaultDepth(info->display,info->screen_num),
           ZPixmap,0,spectro->pix,(unsigned int)spectro->xgram,
           (unsigned int)spectro->wymax, 8, bytesperline );

  spectro->xim->bits_per_pixel = bitspp;


}/* monochrome */
        
 else{
   /* this may have to be changed for workstations
   that have 16 or 32 bits_per_pixel   */

#ifdef BLUESCREENIE
  spectro->pix  = (char *) malloc( sizeof(char) * 
                  spectro->xgram * spectro->wymax);

/* change scan order from spectrum by spectrum to 0,0 upper left scan in x */
  i = 0;    
  for(fq = spectro->wymax - 1; fq >= 0; fq += -1){
   for(t =  0; t < spectro->xgram; t++){
     spectro->pix[i] = (char)
     (info->color[(int)spectro->allgvs[t * spectro->wymax + fq]].pixel);
      i++;
   }
  }/* fq */
  bytesperline = spectro->xgram;
  /*    bitspp = 8; */
  bitspp = DefaultDepth(info->display,info->screen_num);
#endif

    spectro->xim = XCreateImage( info->display, 
				 DefaultVisual(info->display,info->screen_num),
				 DefaultDepth(info->display,info->screen_num),
				 ZPixmap,
				 0,
				 (char *)NULL,
				 spectro->xgram,
				 spectro->wymax,
				 XBitmapPad(info->display),
				 0 );

    spectro->pix = (char *)malloc(sizeof(char) * spectro->xim->bytes_per_line * spectro->wymax);
    spectro->xim->data = spectro->pix;

    for (fq=0; fq<spectro->wymax; fq++) {
	for (t=0; t<spectro->xgram; t++) {
	    XPutPixel(spectro->xim, t, fq, 
		      info->color[(int)spectro->allgvs[(t+1)*spectro->wymax-fq]].pixel);
	}
    }

 }/* more than one bit plane */

}/* first time copy onto xim*/


   /* clear */

   XSetForeground(info->display,info->gc,
                  WhitePixel(info->display,info->screen_num)  );

   XFillRectangle(info->display,info->pixmap,
     info->gc, 0,0,
     spectro->xr[index],spectro->yb[index]); 


  /* spectro->t0 is time offset for spectrogram */


   XPutImage(info->display,info->pixmap,
	     info->gc,spectro->xim,
/* src */    spectro->t0,0,
/* dest*/    xoffset(spectro),yoffset(spectro),
/* w,h */    spectro->wxmax,spectro->wymax);
   
   /* draw axes onto pixmap */
   draw_axes(spectro,info,GRAM,NULL);


}/* end spectro_pixmap*/

/*****************************************************************
 *
 * int xoffset(XSPECTRO *spectro) 
 *
 * Space for x axis.
 *
 *****************************************************************/

int xoffset(XSPECTRO *spectro) 
{
  return(spectro->wchar * 6 + spectro->axisdist);
}

/*****************************************************************
 *
 * int yoffset(XSPECTRO *spectro) 
 *
 * Space for start and end marker labels.
 *
 ******************************************************************/

int yoffset(XSPECTRO *spectro)
{
  return(spectro->hchar);
}

/*****************************************************************/
/*               findtime(spectro,index,x)                       */
/*****************************************************************/
void findtime(XSPECTRO *spectro,int index,int x){ 

/* find iwave index of x value returned from button click*/
/* index = which window */


/* xoffset + 1\2 the largest window for the spectrum */ 
  if( x >= xoffset(spectro) && x <= rborder(spectro,index)){
     spectro->savetime = (float)(x -  xoffset(spectro)) / 
     (float)findxmax(spectro,index)  * 
     (float)spectro->sampsused[index] / spectro->spers * 1000.0 +
     (float)spectro->startime[index];

     spectro->saveindex = spectro->savetime * spectro->spers / 1000.00 + .5;
     /* make savetime the closest sample */     
     spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000;

     validindex(spectro);

   }/* valid range */
}/* end findtime */


/*****************************************************************
void findminmaxtime(XSPECTRO *spectro, int max);
Synopsis: Moves the current position to the local
   maximum (if max != 0) or minimum (if max == 0)
*****************************************************************/

void findminmaxtime(XSPECTRO *spectro, int max) 
{
  int waveindex;
  double left, right, mid;

  waveindex = spectro->saveindex;
  do {
    
    mid = spectro->iwave[waveindex];
    
    if (waveindex == 0) left = 0.0;
    else left = spectro->iwave[waveindex-1];
    
    if (waveindex == spectro->totsamp-1) right = 0.0;
    else right = spectro->iwave[waveindex+1];
    
    if (max) {
      if (mid < right) waveindex++;
      else if (mid < left) waveindex--;
      else break; /* LOCAL MAX FOUND */
    } else {
      if (mid > right) waveindex++;
      else if (mid > left) waveindex--;
      else break; /* LOCAL MIN FOUND */
    }
  } while (1);
  spectro->saveindex = waveindex;
  spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000;
}

/*****************************************************************/
int findspecindex(XSPECTRO *spectro,int x){
  /* don't use this function unless there is a valid
     spectrum */

/* xoffset + 1\2 the largest window for the spectrum */
/* I guess this means check ham window for DFT spectro etc*/
 
 float grdox,grdxr;
 float halfsrate,savhalf,tempfr;
 int i, freqindex,drawsav;
 int freqrange;
 float diff, tempdiff;
 int *histnfr;
 double left, right, mid;

 /* .1 and .75 should be globals somewhere */  
  grdox = spectro->xr[SPECTRUM] * SPEC_OX;
  grdxr = spectro->xr[SPECTRUM] * SPEC_XR;


if( x >= (int)grdox  && x <= (int)grdxr){


   /* in most cases draw fltr and savfltr */
  drawsav = 1;
  if(spectro->option == 'd' ||
     ((spectro->option == 'c' || spectro->option == 's') &&
              !spectro->history )     ){
       /* just draw spectro->fltr */
       drawsav = 0;
  }


  /* fltr case so assume all freqs map to a mag */

    halfsrate = spectro->spers / 2.0; 

  if( 	  spectro->option == 'c' || spectro->option == 'j'||
          spectro->option == 'T' || spectro->option == 'l'){

      tempfr = ((float)x -  grdox) * halfsrate / (grdxr - grdox);

	/* find nfr index for this freq value */
       
        diff = fabsf(tempfr - (float)spectro->nfr[0]);
        freqindex = 0;
	 for(i = 1; i < spectro->lenfltr; i++){
           tempdiff = fabsf(tempfr - (float)spectro->nfr[i]);
           if(tempdiff < diff){
              diff = tempdiff;
              freqindex = i;
	   }/* closer value */

	 }/* all nfr values */


    spectro->specfreq = spectro->nfr[freqindex];
    spectro->specamp = spectro->fltr[freqindex]/10.0;


    /* store x location for cursor */
   spectro->spec_cursor = spectro->specfreq / halfsrate *
   (grdxr - grdox) + .5 +  grdox;


  }/* going to check nfr because the map of freq to
           amp is obscure */

  /* find freq from linear map */
  else{
     freqindex = (int)(   ((float)x -  grdox) * (float)spectro->lenfltr 
                      / (grdxr - grdox) + .5   );
    /* now use freqindex to recalc tempfreq */


     /*
      * FIND THE LOCAL MAXIMUM
      *  Erika 2/27/97
      */

     if (spectro->localfreqmax)
	do {
	 
	 mid = spectro->dftmag[freqindex];
	 
	 if (freqindex == 0) left = 0.0;
	 else left = spectro->dftmag[freqindex-1];
	 
	 if (freqindex == spectro->lenfltr-1) right = 0.0;
	 else right = spectro->dftmag[freqindex+1];
	 
	 if (mid < right) freqindex++;
	 else if (mid < left) freqindex--;
	 else break;
       } while (1);

     /* LOCAL MAXIMUM FOUND */

     tempfr = halfsrate / spectro->lenfltr * (float)freqindex;

     spectro->specfreq = (int)(tempfr + .5);
     spectro->specamp = spectro->fltr[freqindex]/10.0;


    /* store x location for cursor */
    spectro->spec_cursor = spectro->specfreq / halfsrate *
    (grdxr - grdox) + .5 +  grdox;


  }/* assume the freq is a linear map based on lenfltr and halfsrate*/

  /**********/
  /* savfltr */

  /* first check freq range, in 's' if savfltr is 5k
     and fltr is 8 put -1 in savspecfreq if x > 5k */

  savhalf = spectro->savspers / 2.0;

  /* this won't work for case where lensavfltr is different */
  freqrange = 1;
  if( spectro->option == 's'){
    /* need to work directly from x */
   if( x >= (int)((grdxr - grdox) * savhalf/halfsrate +  grdox) )
           freqrange = 0; /* don't bother */    
  }/* check range */


  if(drawsav && spectro->savspectrum && freqrange){

    if(spectro->option == 'c' || spectro->option == 'T'){
       if(spectro->option == 'c')
             histnfr = spectro->savnfr;
       if(spectro->option == 'T' )
             histnfr = spectro->nfr;
     /* know that this has to be the same sampling rate */
     tempfr = ((float)x -  grdox) * savhalf / (grdxr - grdox);

	/* find nfr index for this freq value */
       
        diff = fabsf(tempfr - (float)histnfr[0]);
        freqindex = 0;
	 for(i = 1; i < spectro->lensavfltr; i++){
           tempdiff = fabsf(tempfr - (float)histnfr[i]);
           if(tempdiff < diff){
              diff = tempdiff;
              freqindex = i;
	   }/* closer value */

	 }/* all nfr values */

      spectro->savspecfreq = histnfr[freqindex];
      spectro->savspecamp = spectro->savfltr[freqindex]/10.0;
      if(spectro->lensavfltr > spectro->lenfltr){
	/* store x location for cursor */
	spectro->spec_cursor = spectro->savspecfreq / savhalf *
	  (grdxr - grdox) + .5 +  grdox;
      }/* finer resolution in x so base cursor position on this */


    }/* find freqindex from nfr */


    else{

      /* it might be 5k on 8k */

     freqindex = (int)(   ((float)x -  grdox) * (float)spectro->lensavfltr 
        / ((grdxr - grdox) * savhalf/halfsrate)  + .5   );


     /***** Erika: FIND THE LOCAL MAXIMUM *****/

     if (spectro->localfreqmax) do {

       mid = spectro->dftmag[freqindex];

       if (freqindex == 0) left = 0.0;
       else left = spectro->dftmag[freqindex-1];

       if (freqindex == spectro->lenfltr-1) right = 0.0;
       else right = spectro->dftmag[freqindex+1];

       if (mid < right) freqindex++;
       else if (mid < left) freqindex--;
       else break;
     } while (1);
     /* LOCAL MAXIMUM FOUND */

     /* this may be a little off from the freq found in
        critical band */

     /* now use freqindex to recalc tempfreq */

     tempfr = savhalf / spectro->lensavfltr * (float)freqindex;

     spectro->savspecfreq = (int)(tempfr + .5);


     spectro->savspecamp = spectro->savfltr[freqindex]/10.0;
    if(spectro->lensavfltr > spectro->lenfltr || 
       halfsrate > savhalf){
   /* store x location for cursor */
    spectro->spec_cursor = spectro->savspecfreq / savhalf *
    (grdxr - grdox) * savhalf/halfsrate  + .5 +  grdox;
    }/* finer resolution in x */


    }/*linear map based on lenfltr and halfsrate*/

  }/* valid savfltr and freq in range */


  else{
  /* sav freq and amp values won't be printed */
   spectro->savspecfreq = -1;
   spectro->savspecamp = -1.0;
   
    
  }/* no valid savfltr values */

  return(1);

}/* x is in range */

  return(0); /* button click wasn't in grid */ 

}/* end findspecindex*/
/*****************************************************************/
/*          validindex(XSPECTRO *spectro)                         */
/*****************************************************************/
void validindex(XSPECTRO *spectro){
/* set 0 <= cursor <= totsamp - 1 */
/* and check for enough samples in new_spectrum */
char str [100];

if(spectro->saveindex < 0){
   spectro->saveindex = 0;
   spectro->savetime = 0.0;
  }
else if(spectro->saveindex > spectro->totsamp - 1){
  spectro->saveindex = spectro->totsamp - 1;
  spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000;
  /* Show error message when edge of waveform is breached */
  /* Used primarily when waveforms are locked together */
  sprintf (str, "Warning: the value chosen is beyond\nthe boundary of one or more waveforms.");
  ShowOops(str);
  }
}
/*****************************************************************/
/*           draw_cursor(spectro,window,info)                     */
/*****************************************************************/
void draw_cursor(XSPECTRO *spectro, int window,INFO *info){
int  x,n;
char str[30], title[200];
Arg args[3];
 LABEL *lp;

 if(spectro->iwave == NULL) {
    return;  /* no waveform displayed */
  }

 /*update startindex to make sure cursor is visible in all windows*/

    if(spectro->saveindex > spectro->startindex[window] +
       spectro->sampsused[window]  || spectro->saveindex <
       spectro->startindex[window]){
 
     if(info->pixmap){     
         if(window == GRAM && spectro->spectrogram){
             gramoffset(spectro); 
             make_pixmap(info,spectro,window);
            XCopyArea(info->display,info->pixmap,
            info->win,info->gc,
            0 ,0,spectro->xr[window],spectro->yb[window],0,0);
                      
         }/*GRAM*/
         else if (window != GRAM){
             /* wave_window */
                  if(spectro->sampsused[window] / 2 > spectro->saveindex){
                      spectro->startindex[window] = 0;
                        }
                  else{
                      spectro->startindex[window] = spectro->saveindex - 
                      spectro->sampsused[window] / 2;
                       if ( spectro->startindex[window] + 
                          spectro->sampsused[window] > spectro->totsamp){
                             spectro->startindex[window] = spectro->totsamp -
                             spectro->sampsused[window];
			     }/* may be past totsamp */ 
		    }
             spectro->startime[window] = spectro->startindex[window] /
                    spectro->spers * 1000;
             make_pixmap(info,spectro,window);
             XCopyArea(info->display,info->pixmap,
             info->win,info->gc,
             0 ,0,spectro->xr[window],spectro->yb[window],0,0);
                      

		}/* wave window */

	  }/* wait until there's a gc */

       }/*change pixmap to include cursor*/


    else {
             if(window == GRAM && !spectro->spectrogram){/* no spectrogram*/}
             else{
               XCopyArea(info->display,info->pixmap,
               info->win,info->gc,
               0 ,0,spectro->xr[window],spectro->yb[window],0,0);
	     }
    }/*same pixmap*/

if(window == GRAM && !spectro->spectrogram){/* no spectrogram*/}
else{
    /* draw cursor */
       XSetForeground(info->display,info->gc,info->color[1].pixel);
       x = mstox(spectro,window,spectro->savetime);
       xline(spectro,window,x,info);
       if(window != GRAM){
           draw_hamm(spectro,window,info,NULL);
           sprintf(str,"%.2f ms (%d)",spectro->savetime, (int)(spectro->iwave[spectro->saveindex] / 32767.0 * 9997.56 ));
	   if (x + (spectro->wchar * strlen(str)) > rborder(spectro, window)) 
	     {
	       XDrawString(info->display,info->win,info->gc,
			   x - (spectro->wchar*(strlen(str)+1)),yoffset(spectro) + 
			   spectro->hchar,str,strlen(str)  );
	     }
	   else
	     {
	   XDrawString(info->display,info->win,info->gc,
		       x + spectro->wchar,yoffset(spectro) + 
		       spectro->hchar,str,strlen(str)  );
	     }
	 }

    /* draw start*/

    XSetLineAttributes(info->display,info->gc,1,LineOnOffDash,
     CapNotLast,JoinMiter);  
    x = samptox(spectro,window,spectro->startmarker);
    xline(spectro,window,x,info);
    x = x - spectro->wchar + 2;

   if( x >= (xoffset(spectro) - spectro->wchar) &&
       x <= rborder(spectro,window))
           XDrawString(info->display,info->win,info->gc,
           x,yoffset(spectro) - 2,"w",strlen("w")  );


    /* draw end*/

    x = samptox(spectro,window,spectro->endmarker);
    xline(spectro,window,x,info);

   if( x >= xoffset(spectro) && x <= rborder(spectro,window))
            XDrawString(info->display,info->win,info->gc,
           x,yoffset(spectro) - 2,"e",strlen("e")  );

    XSetLineAttributes(info->display,info->gc,1,LineSolid,
     CapNotLast,JoinMiter); 

       dowavtitle(spectro);

 /* draw labels (mkt) */
       for (lp = spectro->labels; lp; lp=lp->next) {
	 x = mstox(spectro, window, lp->offset);
	 xline(spectro, window, x, info);
	 XDrawString(info->display, info->win, info->gc,
		     x, yoffset(spectro)-3, lp->name, strlen(lp->name));
	 sprintf(str, "%g", lp->offset);
	 XDrawString(info->display, info->win, info->gc,
		     x+5, yoffset(spectro)+13, str, strlen(str));
       }
       
   }/* some place to draw*/

}/* end  draw_cursor */

/****************************************************************/
/*   xline(XSPECTRO *spectro, int window,  int x, INFO *info)    */
/* check borders and draw line at x */
/****************************************************************/
void  xline(XSPECTRO *spectro, int window,  int x, INFO *info)
{
       if( x >= xoffset(spectro) && x <= rborder(spectro,window))
          XDrawLine(info->display,info->win, info->gc,
          x,yoffset(spectro),x,bborder(spectro,window) - 2);
}

/****************************************************************/
/*     draw_hamm(XSPECTRO *spectro,int window,INFO *info,FILE *fp)       */
/* postscript file - used with fourspectra */
/****************************************************************/

void  draw_hamm(XSPECTRO *spectro, int window,INFO *info,FILE *fp){
int i,n,sampindex;
XPoint points[2048];  /* For sampling rates of 44 kHz */
int sizwin,ymax,numpoints, temp;
float map;
int yps ; /* switch X ycoord to postscript ycoord */

ymax = findymax(spectro,window);
yps = spectro->yb[window] ;

/*based on spectro->option set window size*/
/*should only store hamming window for spectrum
  whose hamming window is being displayed
  in the case where the DFT type is being displayed
  with something else don't store it
*/
 map = (float)ymax * .75 / 4.0 ;
 sampindex = spectro->saveindex - spectro->sizwin / 2;

numpoints = 0;
for( i = 0; i < spectro->sizwin; i++){
    /* clip */
    if( (temp = samptox(spectro, window,sampindex) ) > xoffset(spectro) &&
         temp <  xoffset(spectro) + findxmax(spectro,window)  ){   
      points[numpoints].y = (short)((bborder(spectro,window) - 
      spectro->axisdist - 1) - (float)ymax / 2.0 - 
      (map * (spectro->hamm[i] ) + .005)  ) ; 
     points[numpoints].x = (short)temp;
      numpoints ++;
      }/* points are in range */
      sampindex++;

    }

if(fp == NULL){
   
    XSetForeground(info->display,info->gc,
                  info->color[1].pixel);

    XDrawLines(info->display,info->win,
            info->gc, points,
             numpoints,CoordModeOrigin);
}
else{


  fprintf(fp," .5 setlinewidth\n");
  fprintf(fp,"%f %f m \n",
         (float)points[0].x, (float)yps - points[0].y );
     for(i = 1; i < numpoints - 1; i++)
       fprintf(fp,"%f %f l\n",
            (float)points[i].x,yps - (float)points[i].y);

  fprintf(fp," 1.0 setlinewidth\n");

  }


}/* draw_hamm*/

/*****************************************************************/
/* eraseline(XSPECTRO *spectro, int window, int x, INFO *info)    */
/*****************************************************************/
void  eraseline(XSPECTRO *spectro, int window, int x, INFO *info){

  XCopyArea(info->display,info->pixmap,  info->win,info->gc,
  x,yoffset(spectro),1,findymax(spectro,window) + spectro->axisdist,
  x,yoffset(spectro)); 

}/* end eraseline */

/*****************************************************************/
/* erasechar(XSPECTRO *spectro, int window, int x, INFO *info)    */
/*****************************************************************/
void erasechar(XSPECTRO *spectro, int window, int x, INFO *info){

  XCopyArea(info->display,info->pixmap,  info->win,info->gc,
  x,0,spectro->wchar,spectro->hchar,x,0); 

}/* end erasechar */

/*****************************************************************/
/* samptox(XSPECTRO *spectro, int window, int whichsamp)          */
/*****************************************************************/
int samptox(XSPECTRO *spectro, int window, int whichsamp){ 
int x;
float ms;


ms = (float)whichsamp / spectro->spers * 1000.0;

x = mstox(spectro,window,ms);

return(x);

}/* end samptox */

/*****************************************************************/
/*          findxmax(XSPECTRO *spectro, int index)               */
/*****************************************************************/
int findxmax(XSPECTRO *spectro, int index) {

return(spectro->xr[index] - spectro->wchar * 10  - spectro->axisdist - 1);

}
/*****************************************************************/
/*          findymax(XSPECTRO *spectro, int index)               */
/*****************************************************************/
int findymax(XSPECTRO *spectro, int index) {
  /* changed this from 2 to 3 so TIME (ms) could have a line of its own*/ 
return(spectro->yb[index] - (spectro->hchar * 3) - spectro->axisdist);

}

/*****************************************************************/
/*                  draw_axes(spectro,info,index)                */
/*****************************************************************/

void draw_axes(XSPECTRO *spectro,INFO *info,int index, FILE *postfp)
{
/* draw axes for waveform and spectrogram */
/* called by printwav and postgvs for postscript output*/
int x,y,x2,y2,xt,yt,y0,ygram0;
int tmax, i, tstep, len;
char str[15];
static char sectxt[] = "TIME (sec)";
static char mstxt[] = "TIME (ms)";
int xmax, ymax;
float ms, fx , inc, fy;
int yps;  /* switch X ycoord to postscript ycoord */
float max, min, mid, mv = 0, map; /* waveform y-axis*/

xmax = findxmax(spectro, index);
ymax = findymax(spectro, index);

  XSetForeground(info->display,info->gc,
                  info->color[1].pixel);

yps = spectro->yb[index] - 1; /* used to make y normal again for postscript*/


/*labels  ms*/
/*debug
x =  spectro->xr[index] - 3 * spectro->wchar + 1;
*/
x = xoffset(spectro) + xmax / 2 - strlen("TIME (ms)") / 2 * spectro->wchar;

y = spectro->yb[index] - 2; 


if(spectro->sec){
    if(postfp != NULL){
    fprintf(postfp, "%d %d m (%s) o\n", x, yps -  y, sectxt);
    }
    else{
    XDrawString(info->display,info->pixmap,info->gc,
           x,y,sectxt,strlen(sectxt)  );
    }
  }/*sec*/

else{
    if(postfp != NULL){
    fprintf(postfp, "%d %d m (%s) o\n", x, yps - y, mstxt);
    }
    else{
     XDrawString(info->display,info->pixmap,info->gc,
           x,y,mstxt,strlen(mstxt)  );
   }
}/*ms*/

if(index == GRAM){
 /*spectrogram y axis label*/
   x =  xoffset(spectro) - 5 * spectro->wchar - spectro->axisdist;
   y = 2 * spectro->hchar ;

  if(postfp != NULL){
    
    fprintf(postfp,"gsave\n");
    fprintf(postfp,"%d %d translate 90 rotate\n",spectro->xr[GRAM],0);
    /*
    fprintf(postfp, "%d %d m (%s) o\n", x, yps - y,"kHz");
    */
    
    fprintf(postfp, "%d %d m (%s) o\n",
    yps/2 - y, spectro->xr[GRAM] - xoffset(spectro) + 2 * spectro->wchar + 10,
    "FREQ (kHz)");
    fprintf(postfp,"grestore \n");
       
    }
    else{
   XDrawString(info->display,info->pixmap,info->gc,
           x,y,"kHz",3 );
    }


  }/*kHz label*/



/*axes*/
  y =  bborder(spectro,index) - 1;/* y starts at 0 */
  y2 = yoffset(spectro) - 1;
  x =  xoffset(spectro) - spectro->axisdist; 
  x2 = rborder(spectro,index);


  if(postfp != NULL){
  fprintf(postfp, "%d %d m %d %d k\n", x, yps - y, x, yps - y2);
  fprintf(postfp, "%d %d m %d %d k\n", x, yps - y, x2, yps - y);
  fprintf(postfp, "%d %d m %d %d k\n", x2, yps - y, x2, yps - y2);
  fprintf(postfp, "%d %d m %d %d k\n", x, yps - y2, x2, yps - y2);
  }
  else{
    XDrawLine(info->display,info->pixmap, info->gc,x,y,x,y2);
    
    XDrawLine(info->display,info->pixmap, info->gc,x,y,x2,y);
    
    XDrawLine(info->display,info->pixmap, info->gc,x2,y,x2,y2);
    
    XDrawLine(info->display,info->pixmap, info->gc,x,y2,x2,y2);
  }


/* tick marks on y axis */

if(index == GRAM){
  /* find max in kHz, sample rate/2 /10*/
  tmax = spectro->spers/2000.0;
  
  inc = (float)ymax/(spectro->spers/2000.0);
  
  /*even though there are xr X yb coord in window*/
  /* the range is 0 to yb -1(gridlines)*/
  y  = spectro->yb[index] - 2 * spectro->hchar - spectro->axisdist - 1;
  ygram0 = spectro->yb[index] - 2 * spectro->hchar - spectro->axisdist - 1;
  /*fy = spectro->yb[index] - spectro->hchar - spectro->axisdist - 1;*/
  fy = 0.0;
  xt = xoffset(spectro) - (int)(1.5*spectro->wchar) - spectro->axisdist;
  /*x2 = xoffset(spectro);*/
  /* draw gridlines here */
  x2 = rborder(spectro,index);
  
  for( i = 0; i <= tmax; i++){
    sprintf(str,"%d",i);
    yt =  y + spectro->hchar * (float)strlen(str) / 2.0;
    
   if(postfp != NULL){
   fprintf(postfp, "%d %d m %d %d k\n",x , yps - y,x2, yps - y);
   fprintf(postfp, "%d %d m (%s) o\n",xt ,yps - yt, str);
 }
   else{
     
     XDrawLine(info->display,info->pixmap, info->gc,x,y,x2,y);
     XDrawString(info->display,info->pixmap,info->gc,
		 xt,yt,str,strlen(str));
   }
    
    fy += inc;
    y = ygram0 - (int)(fy + .5);
    
    
  } /*end y tick marks*/
  
}/* spectrogram y axis */

else{
/* y-axis for waveforms */
/* wavemin and wavemax are the min and max for all of iwave */

  mid = spectro->wavemax - (spectro->wavemax - spectro->wavemin) / 2.0;
  max = mid + 
  (spectro->wavemax - spectro->wavemin) / 2.0 * spectro->wvfactor[index];
  min = mid - 
  (spectro->wavemax - spectro->wavemin) / 2.0 * spectro->wvfactor[index];

 /* convert these numbers to millivolts, wavemin+wavemax were divided
     by 16 */
  mid = mid * 16.0 / 32767.0 * 9997.56;
  max = max * 16.0 / 32767.0 * 9997.56;
  min = min * 16.0 / 32767.0 * 9997.56;

  map = (float)ymax / (max - min);

   tstep = (float)spectro->hchar * 1.5 * (max - min) / (float)ymax ;
   if( max - min > 4000)
              {      tstep =  tstep + 1000 -  (tstep % 1000);   }
    else if( max - min > 800)
              {       tstep =  tstep + 100 -  (tstep % 100);   }
    else if( max - min > 150)
              {       tstep =  tstep + 50 -  (tstep % 50);   }           
    else if( max - min > 50)
              {       tstep =  tstep + 5 -  (tstep % 5);   }           
    else
      {tstep =  tstep + 1 - (tstep % 1);}

if((float)((int)(min / tstep ) * tstep) == mv)
   { mv = min;} 
else if(min < 0.0)
   {mv = (int)(min / tstep ) * tstep ;}
else
   {mv = ((int)min/tstep + 1) * tstep ;}

     tmax = (max - mv ) / tstep ;
     x2 = xoffset(spectro);
     x = x2 - spectro->axisdist; 
     y0 = bborder(spectro,index) - spectro->axisdist - 1;
     y = y0 - (int)(map * (mv - min)+ .5);
     yt = y + spectro->hchar/ 2.0;
     for( i = 0; i <= tmax; i++){
     sprintf(str,"%d",(int)mv );

    xt = x - (spectro->wchar * (strlen(str) + 1));

    if(postfp != NULL){
        if((int)mv == 0){
        /* origin */
         fprintf(postfp," .5 setlinewidth\n");
         fprintf(postfp, 
         "%d %d m %d %d k\n",x , yps - y,rborder(spectro,index) , yps - y);
         fprintf(postfp," 1.0 setlinewidth\n");
        }
       else{
        fprintf(postfp, "%d %d m %d %d k\n",x , yps - y, x2, yps - y);
       }
     fprintf(postfp, "%d %d m (%s) o\n",xt, yps - yt, str);
     }
    else{
      if((int)mv == 0){
        /* origin */
       XDrawLine(info->display,info->pixmap, 
         info->gc,x,y,rborder(spectro,index),y);
       }
      else{
       XDrawLine(info->display,info->pixmap, info->gc,x,y,x2,y);
      }

    XDrawString(info->display,info->pixmap,info->gc,
           xt,yt,str,strlen(str));
  }
  
    mv += tstep;
    y = y0 - (int)(map * (mv - min)+ .5);
    yt = y + spectro->hchar/ 2.0; 
    }
}/* y-axis for waveforms */



/* x tick marks */

/* make increments large enough for text*/


if(spectro->sec){
      ms = (float)spectro->startime[index] + 
           (float)spectro->sampsused[index]  / spectro->spers * 1000 ;
      sprintf(str,"%.2f",ms/1000.0);
    }
else{
     ms = (float)spectro->startime[index] +
           (float)spectro->sampsused[index] / spectro->spers * 1000;
      sprintf(str,"%d",(int)ms);

   }

      tstep = (float)spectro->wchar *  (float)strlen(str) *
           ((float)spectro->sampsused[index] / spectro->spers * 1000)
           /(float)xmax ;

   if(  ((float)spectro->sampsused[index] / spectro->spers * 1000) > 150)
              {       tstep =  tstep + 50 -  (tstep % 50);   }

    else if(  ((float)spectro->sampsused[index] / spectro->spers * 1000) > 80)
              {       tstep =  tstep + 10 -  (tstep % 10);   }
           

    else if(  ((float)spectro->sampsused[index] / spectro->spers * 1000) > 50)
              {       tstep =  tstep + 5 -  (tstep % 5);   }
           

      else{
      tstep =  tstep + 1 - (tstep % 1);
      }
      tmax = spectro->sampsused[index] / 
              spectro->spers * 1000 / tstep ;

y =  spectro->yb[index] - 2 * spectro->hchar - 1;
y2 = spectro->yb[index] - 2 * spectro->hchar - 
    spectro->axisdist - 1 + spectro->tickspace;
yt = spectro->yb[index] - 1 * spectro->hchar - 1; 

if((int)(spectro->startime[index]  / tstep) * tstep
           == spectro->startime[index])
    { ms = spectro->startime[index];} 
else
  {ms = ((int)spectro->startime[index]  / tstep + 1) *  tstep ;}

  x = mstox(spectro,index,ms);
  for( i = 0; i <= tmax; i++){
    if(spectro->sec)
      sprintf(str,"%.2f",ms/1000.0);
    else
      sprintf(str,"%d",(int)ms );

    xt = x - spectro->wchar * strlen(str)  / 2 + 3;

    if(xt + spectro->wchar * strlen(str) / 2 + 5 <
         spectro->xr[index] - spectro->wchar * 3){

    if(postfp != NULL){
     fprintf(postfp, "%d %d m %d %d k\n",x , yps - y, x, yps - y2);
     fprintf(postfp, "%d %d m (%s) o\n",xt, yps - yt, str);
     }
     else{
       XDrawLine(info->display,info->pixmap, info->gc,x,y,x,y2);
       XDrawString(info->display,info->pixmap,info->gc,
           xt,yt,str,strlen(str));
      }
     }  /*don't draw into ms label*/ 
    ms += tstep; 
    x = mstox(spectro,index,ms);
   }  /* x tick marks */

}/*end draw_axes*/
/******************************************************************/
/*                 rborder(XSPECTRO *spectro)                      */
/******************************************************************/
int rborder(XSPECTRO *spectro, int index) {
/*right border*/
return(spectro->xr[index] - 4 * spectro->wchar);
}
/******************************************************************/
/*                 bborder(XSPECTRO *spectro)                      */
/******************************************************************/
int bborder(XSPECTRO *spectro, int index) {
/* bottom border */
return(spectro->yb[index] - 2 * spectro->hchar);
}/* end bborder*/

/****************************************************************/
/*               set_time(spectro,startime,index)               */
/****************************************************************/
/*startime is the index of the sample in the data array that*/
/*is the first sample of the spectrogram*/ 

void set_time(spectro,startindex,index)
XSPECTRO *spectro;
int startindex;   /*  absolute data index for start of spectrogram */
int index;  /* which window */
{
/* this function assumes that spectro->spers has
been set to the correct number of samples per sec*/

/* allow for 0th sample*/
spectro->startindex[index] = startindex + 1;
spectro->startime[index] = startindex / spectro->spers * 1000.0;/*ms*/


}/* end set_time*/


/**********************************************************************

DFT.C - SOURCE CODE FOR DISCRETE FOURIER TRANSFORM FUNCTIONS

dft     Discrete Fourier Transform
idft    Inverse Discrete Fourier Transform
fft     In-place radix 2 decimation in time FFT
ifft    In-place radix 2 decimation in time inverse FFT
rfft    Trig recombination real input FFT
ham     Hamming window
han     Hanning window
triang  Triangle window
black   Blackman window
harris  4 term Blackman-Harris window
ilog2    Base 2 logarithm

***********************************************************************/

/***********************************************************************
  code from Embreee and Kimble

spec_dft - Discrete Fourier Transform
  
This function performs a straight DFT of N points on an array of
complex numbers whose first member is pointed to by Datain.  The
output is placed in an array pointed to by Dataout.

void spec_dft(COMPLEX *Datain, COMPLEX *Dataout, int N)

*************************************************************************/

void spec_dft(Datain, Dataout, N)
    COMPLEX *Datain, *Dataout;
    int N;
{
    int i,k,n,p;
    static int nstore = 0;      /* store N for future use */
    static COMPLEX *cf;         /* coefficient storage */
    COMPLEX *cfptr,*Dinptr;
    double arg;

/* Create the coefficients if N has changed */

    if(N != nstore) {
        if(nstore != 0) free((char *) cf);    /* free previous */

        cf = (COMPLEX  *) calloc(N, sizeof(COMPLEX));
        if (!cf) {
            printf("\nUnable to allocate memory for coefficients.\n");
            exit(1);
        }

        arg = 8.0*atan(1.0)/N;
        for (i=0 ; i<N ; i++) {
            cf[i].real = (float)cos(arg*i);
            cf[i].imag = -(float)sin(arg*i);
        }
    }

/* Perform the DFT calculation */

    printf("\n");
    for (k=0 ; k<N ; k++) {

        Dinptr = Datain;
        Dataout->real = Dinptr->real;
        Dataout->imag = Dinptr->imag;
        Dinptr++;
        for (n=1; n<N; n++) {

        p = (int)((long)n*k % N);
            cfptr = cf + p;         /* pointer to cf modulo N */

            Dataout->real += Dinptr->real * cfptr->real
                             - Dinptr->imag * cfptr->imag;

            Dataout->imag += Dinptr->real * cfptr->imag
                             + Dinptr->imag * cfptr->real;
            Dinptr++;
        }
        if (k % 32 == 31) printf("*");
        Dataout++;          /* next output */
    }
    printf("\n");
}

/***********************************************************************

idft - Inverse Discrete Fourier Transform
  
This function performs an inverse DFT of N points on an array of
complex numbers whose first member is pointed to by Datain.  The
output is placed in an array pointed to by Dataout.
It returns nothing.

void idft(COMPLEX *Datain, COMPLEX *Dataout, int N)

*************************************************************************/

void idft(Datain, Dataout, N)
    COMPLEX *Datain, *Dataout;
    int N;
{
    int i,k,n,p;
    static int nstore = 0;      /* store N for future use */
    static COMPLEX *cf;         /* coefficient storage */
    COMPLEX *cfptr,*Dinptr;
    double arg;

/* Create the coefficients if N has changed */

    if(N != nstore) {
        if(nstore != 0) free((char *) cf);    /* free previous */

        cf = (COMPLEX  *) calloc(N, sizeof(COMPLEX));
        if (cf == 0) {
            printf("\nUnable to allocate memory for coefficients.\n");
            exit(1);
        }

/* scale stored values by 1/N */
        arg = 8.0*atan(1.0)/N;
        for (i=0 ; i<N ; i++) {
            cf[i].real = (float)(cos(arg*i)/(double)N);
            cf[i].imag = (float)(sin(arg*i)/(double)N);
        }
    }

/* Perform the DFT calculation */

    printf("\n");
    for (k=0 ; k<N ; k++) {

        Dinptr = Datain;
        Dataout->real = Dinptr->real * cf[0].real;
        Dataout->imag = Dinptr->imag * cf[0].real;
        Dinptr++;
        for (n=1; n<N; n++) {

        p = (int)((long)n*k % N);
            cfptr = cf + p;         /* pointer to cf modulo N */

            Dataout->real += Dinptr->real * cfptr->real
                             - Dinptr->imag * cfptr->imag;

            Dataout->imag += Dinptr->real * cfptr->imag
                             + Dinptr->imag * cfptr->real;
            Dinptr++;
        }
        if (k % 32 == 31) printf("*");
        Dataout++;          /* next output */
    }
    printf("\n");
}

/**************************************************************************

fft - In-place radix 2 decimation in time FFT

Requires pointer to complex array, x and power of 2 size of FFT, m
(size of FFT = 2**m).  Places FFT output on top of input COMPLEX array.

void fft(COMPLEX *x, int m)

*************************************************************************/

void fft(x,m)
    COMPLEX *x;
    int m;
{
    static COMPLEX *w;           /* used to store the w complex array */
    static int mstore = 0;       /* stores m for future reference */
    static int n = 1;            /* length of fft stored for future */

    COMPLEX u,temp,tm;
    COMPLEX *xi,*xip,*xj,*wptr;

    int i,j,k,l,le,windex;

    double arg,w_real,w_imag,wrecur_real,wrecur_imag,wtemp_real;

    if(m != mstore) {

/* free previously allocated storage and set new m */

        if(mstore != 0) free(w);
        mstore = m;
        if(m == 0) return;       /* if m=0 then done */

/* n = 2**m = fft length */

        n = 1 << m;

        le = n/2;

/* allocate the storage for w */

        w = (COMPLEX *) calloc(le-1,sizeof(COMPLEX));
        if(!w) {
            printf("\nUnable to allocate complex W array\n");
            exit(1);
        }

/* calculate the w values recursively */

        arg = 4.0*atan(1.0)/le;         /* PI/le calculation */
        wrecur_real = w_real = cos(arg);
        wrecur_imag = w_imag = -sin(arg);
        xj = w;
        for (j = 1 ; j < le ; j++) {
            xj->real = (float)wrecur_real;
            xj->imag = (float)wrecur_imag;
            xj++;
            wtemp_real = wrecur_real*w_real - wrecur_imag*w_imag;
            wrecur_imag = wrecur_real*w_imag + wrecur_imag*w_real;
            wrecur_real = wtemp_real;
        }
    }

/* start fft */

    le = n;
    windex = 1;
    for (l = 0 ; l < m ; l++) {
        le = le/2;

/* first iteration with no multiplies */

        for(i = 0 ; i < n ; i = i + 2*le) {
            xi = x + i;
            xip = xi + le;
            temp.real = xi->real + xip->real;
            temp.imag = xi->imag + xip->imag;
            xip->real = xi->real - xip->real;
            xip->imag = xi->imag - xip->imag;
            *xi = temp;
        }

/* remaining iterations use stored w */

        wptr = w + windex - 1;
        for (j = 1 ; j < le ; j++) {
            u = *wptr;
            for (i = j ; i < n ; i = i + 2*le) {
                xi = x + i;
                xip = xi + le;
                temp.real = xi->real + xip->real;
                temp.imag = xi->imag + xip->imag;
                tm.real = xi->real - xip->real;
                tm.imag = xi->imag - xip->imag;             
                xip->real = tm.real*u.real - tm.imag*u.imag;
                xip->imag = tm.real*u.imag + tm.imag*u.real;
                *xi = temp;
            }
            wptr = wptr + windex;
        }
        windex = 2*windex;
    }            

/* rearrange data by bit reversing */

    j = 0;
    for (i = 1 ; i < (n-1) ; i++) {
        k = n/2;
        while(k <= j) {
            j = j - k;
            k = k/2;
        }
        j = j + k;
        if (i < j) {
            xi = x + i;
            xj = x + j;
            temp = *xj;
            *xj = *xi;
            *xi = temp;
        }
    }
}

/**************************************************************************

ifft - In-place radix 2 decimation in time inverse FFT

Requires pointer to complex array, x and power of 2 size of FFT, m
(size of FFT = 2**m).  Places inverse FFT output on top of input
frequency domain COMPLEX array.

void ifft(COMPLEX *x, int m)

*************************************************************************/

void ifft(x,m)
    COMPLEX *x;
    int m;
{
    static COMPLEX *w;           /* used to store the w complex array */
    static int mstore = 0;       /* stores m for future reference */
    static int n = 1;            /* length of ifft stored for future */

    COMPLEX u,temp,tm;
    COMPLEX *xi,*xip,*xj,*wptr;

    int i,j,k,l,le,windex;

    double arg,w_real,w_imag,wrecur_real,wrecur_imag,wtemp_real;
    float scale;

    if(m != mstore) {

/* free previously allocated storage and set new m */

        if(mstore != 0) free(w);
        mstore = m;
        if(m == 0) return;       /* if m=0 then done */

/* n = 2**m = inverse fft length */

        n = 1 << m;
        le = n/2;

/* allocate the storage for w */

        w = (COMPLEX *) calloc(le-1,sizeof(COMPLEX));
        if(!w) {
            printf("\nUnable to allocate complex W array\n");
            exit(1);
        }

/* calculate the w values recursively */

        arg = 4.0*atan(1.0)/le;         /* PI/le calculation */
        wrecur_real = w_real = cos(arg);
        wrecur_imag = w_imag = sin(arg);  /* opposite sign from fft */
        xj = w;
        for (j = 1 ; j < le ; j++) {
            xj->real = (float)wrecur_real;
            xj->imag = (float)wrecur_imag;
            xj++;
            wtemp_real = wrecur_real*w_real - wrecur_imag*w_imag;
            wrecur_imag = wrecur_real*w_imag + wrecur_imag*w_real;
            wrecur_real = wtemp_real;
        }
    }

/* start inverse fft */

    le = n;
    windex = 1;
    for (l = 0 ; l < m ; l++) {
        le = le/2;

/* first iteration with no multiplies */

        for(i = 0 ; i < n ; i = i + 2*le) {
            xi = x + i;
            xip = xi + le;
            temp.real = xi->real + xip->real;
            temp.imag = xi->imag + xip->imag;
            xip->real = xi->real - xip->real;
            xip->imag = xi->imag - xip->imag;
            *xi = temp;
        }

/* remaining iterations use stored w */

        wptr = w + windex - 1;
        for (j = 1 ; j < le ; j++) {
            u = *wptr;
            for (i = j ; i < n ; i = i + 2*le) {
                xi = x + i;
                xip = xi + le;
                temp.real = xi->real + xip->real;
                temp.imag = xi->imag + xip->imag;
                tm.real = xi->real - xip->real;
                tm.imag = xi->imag - xip->imag;             
                xip->real = tm.real*u.real - tm.imag*u.imag;
                xip->imag = tm.real*u.imag + tm.imag*u.real;
                *xi = temp;
            }
            wptr = wptr + windex;
        }
        windex = 2*windex;
    }            

/* rearrange data by bit reversing */

    j = 0;
    for (i = 1 ; i < (n-1) ; i++) {
        k = n/2;
        while(k <= j) {
            j = j - k;
            k = k/2;
        }
        j = j + k;
        if (i < j) {
            xi = x + i;
            xj = x + j;
            temp = *xj;
            *xj = *xi;
            *xi = temp;
        }
    }

/* scale all results by 1/n */
    scale = (float)(1.0/n);
    for(i = 0 ; i < n ; i++) {
        x->real = scale*x->real;
        x->imag = scale*x->imag;
        x++;
    }
}

/************************************************************

rfft - trig recombination real input FFT

Requires real array pointed to by x, pointer to complex
output array, y and the size of real FFT in power of
2 notation, m (size of input array and FFT, N = 2**m).
On completion, the COMPLEX array pointed to by y 
contains the lower N/2 + 1 elements of the spectrum.

void rfft(float *x, COMPLEX *y, int m)

***************************************************************/

void rfft(x,y,m)
    float      *x;
    COMPLEX    *y;
    int        m;
{
    static    COMPLEX  *cf;
    static    int      mstore = 0;
    int       p,num,k,index;
    float     Realsum, Realdif, Imagsum, Imagdif;
    double    factor, arg;
    COMPLEX   *ck, *xk, *xnk, *cx;

/* First call the fft routine using the x array but with
   half the size of the real fft */

    p = m - 1;
    cx = (COMPLEX *) x;
    fft(cx,p);

/* Next create the coefficients for recombination, if required */

    num = 1 << p;    /* num is half the real sequence length.  */

    if (m!=mstore){
      if (mstore != 0) free(cf);
      cf = (COMPLEX *) calloc(num - 1,sizeof(COMPLEX));
      if(!cf){
        printf("\nUnable to allocate trig recomb coefficients.");
        exit(1);
      }

      factor = 4.0*atan(1.0)/num;
      for (k = 1; k < num; k++){
        arg = factor*k;
        cf[k-1].real = (float)cos(arg);
        cf[k-1].imag = (float)sin(arg);
      }
    }  

/* DC component, no multiplies */
    y[0].real = cx[0].real + cx[0].imag;
    y[0].imag = 0.0;

/* other frequencies by trig recombination */
    ck = cf;
    xk = cx + 1;
    xnk = cx + num - 1;
    for (k = 1; k < num; k++){
      Realsum = ( xk->real + xnk->real ) / 2;
      Imagsum = ( xk->imag + xnk->imag ) / 2;
      Realdif = ( xk->real - xnk->real ) / 2;
      Imagdif = ( xk->imag - xnk->imag ) / 2;

      y[k].real = Realsum + ck->real * Imagsum
                          - ck->imag * Realdif ;

      y[k].imag = Imagdif - ck->imag * Imagsum
                          - ck->real * Realdif ;
      ck++;
      xk++;
      xnk--;
    }
}

/*************************************************************************

ham - Hamming window

Scales both real and imaginary parts of input array in-place.
Requires COMPLEX pointer and length of input array.

void ham(COMPLEX *x, int n)

*************************************************************************/

void ham(x, n)
  COMPLEX    *x;
  int        n;
{
  int    i;
  double ham,factor;

  factor = 8.0*atan(1.0)/(n-1);
  for (i = 0 ; i < n ; i++){
    ham = 0.54 - 0.46*cos(factor*i);
    x->real *= ham;
    x->imag *= ham;
    x++;
  }
}

/*************************************************************************

han - Hanning window

Scales both real and imaginary parts of input array in-place.
Requires COMPLEX pointer and length of input array.

void han(COMPLEX *x, int n)

*************************************************************************/

void han(x, n)
  COMPLEX    *x;
  int        n;
{
  int    i;
  double factor,han;

  factor = 8.0*atan(1.0)/(n-1);
  for (i = 0 ; i < n ; i++){
    han = 0.5 - 0.5*cos(factor*i);
    x->real *= han;
    x->imag *= han;
    x++;
  }
}

/*************************************************************************

triang - triangle window

Scales both real and imaginary parts of input array in-place.
Requires COMPLEX pointer and length of input array.

void triang(COMPLEX *,int n)

*************************************************************************/

void triang(x, n)
  COMPLEX    *x;
  int        n;
{
  int    i;
  double tri,a;
  a = 2.0/(n-1);

  for (i = 0 ; i <= (n-1)/2 ; i++) {
    tri = i*a;
    x->real *= tri;
    x->imag *= tri;
    x++;
  }
  for ( ; i < n ; i++) {
    tri = 2.0 - i*a;
    x->real *= tri;
    x->imag *= tri;
    x++;
  }
}

/*************************************************************************

black - Blackman window

Scales both real and imaginary parts of input array in-place.
Requires COMPLEX pointer and length of input array.

void black(COMPLEX *x, int n)

*************************************************************************/

void black(x, n)
  COMPLEX    *x;
  int        n;
{
  int    i;
  double black,factor;

  factor = 8.0*atan(1.0)/(n-1);
  for (i=0; i<n; ++i){
    black = 0.42 - 0.5*cos(factor*i) + 0.08*cos(2*factor*i);
    x->real *= black;
    x->imag *= black;
    x++;
  }
}

/*************************************************************************

harris - 4 term Blackman-Harris window

Scales both real and imaginary parts of input array in-place.
Requires COMPLEX pointer and length of input array.

void harris(COMPLEX *x, int n)

*************************************************************************/

void harris(x, n)
  COMPLEX    *x;
  int        n;
{
  int    i;
  double harris,factor,arg;

  factor = 8.0*atan(1.0)/n;
  for (i=0; i<n; ++i){
    arg = factor * i;
    harris = 0.35875 - 0.48829*cos(arg) + 0.14128*cos(2*arg)
               - 0.01168*cos(3*arg);
    x->real *= harris;
    x->imag *= harris;
    x++;
  }
}

/**************************************************************************

ilog2 - base 2 logarithm

Returns base 2 log such that i = 2**ans where ans = ilog2(i).
if ilog2(i) is between two values, the larger is returned.

int ilog2(unsigned int x)

*************************************************************************/

int ilog2(x)
    unsigned int x;
{
    unsigned int mask,i;

    if(x == 0) return(-1);      /* zero is an error, return -1 */

    x--;                        /* get the max index, x-1 */

    for(mask = 1 , i = 0 ; ; mask *= 2 , i++) {
        if(x == 0) return(i);   /* return ilog2 if all zero */
        x = x & (~mask);        /* AND off a bit */
    }
}
/****************************************************************************/
/*                  initime(XSPECTRO *spectro)                               */
/****************************************************************************/
void initime(XSPECTRO *spectro) {
/* set cursor in the middle of the file and find startindex*/

   if(spectro->iwave == NULL)
        return;

  
   /* the same for all three windows */
   spectro->saveindex = spectro->totsamp / 2 ;
   spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000;
 
   /*w0*/
   spectro->startime[0] = 0.0;
   spectro->startindex[0] = 0;
   spectro->sampsused[0] = spectro->totsamp;

   /*w1*/
   spectro->sampsused[1] = spectro->params[1];/* samples in DFT window */
   if(spectro->sampsused[1] > spectro->totsamp)
         spectro->sampsused[1] = spectro->totsamp;

      spectro->startindex[1] = spectro->saveindex - 
               spectro->sampsused[1] / 2;

   spectro->startime[1] = spectro->startindex[1] / spectro->spers * 1000;

    /* set spectrogram startindex and offset for Ximage */
    if(spectro->spectrogram){gramoffset(spectro);}
 
     spectro->oldindex = spectro->saveindex;
     spectro->oldtime  = spectro->savetime;

     spectro->startmarker = 0;
     spectro->endmarker = spectro->totsamp - 1;
     spectro->oldstart = spectro->startmarker;
     spectro->oldend = spectro->endmarker;

  
}/* end initime */

/**********************************************************************/
/*                gramoffset(XSPECTRO *spectro)                        */
/**********************************************************************/
void gramoffset(XSPECTRO *spectro){

   /* sampsused is based on number of ms displayed,win_size*/



   if(spectro->sampsused[GRAM] / 2 > spectro->saveindex){
       {spectro->startindex[GRAM] = 0;}
     }
   else if(spectro->sampsused[GRAM] / 2 + spectro->saveindex > 
                     spectro->totsamp)
     {spectro->startindex[GRAM] = spectro->totsamp - spectro->sampsused[GRAM];}

   else{
      spectro->startindex[GRAM] = spectro->saveindex - 
               spectro->sampsused[GRAM] / 2;
       } 

          
   spectro->startime[GRAM] = spectro->startindex[GRAM] / spectro->spers * 1000;

   /* check this out */
   spectro->t0 =(spectro->startindex[GRAM] ) /
   ((spectro->winsamps - spectro->ovlap) * spectro->numav) * spectro->xpix;
 }/* end gramoffset*/

/******************************************************************/
/*void  newwave(Widget w, XtPointer client_data, XtPointer call_data) */
/******************************************************************/
void  newwave(Widget w, XtPointer client_data, XtPointer call_data){
/* add a new wave form and update allspectros*/
XmString mstr;
Arg args[3];
char str[500],mess[550];
char *temp;  
XSPECTRO *spectro;
XSPECTRO **tmplist;
XSPECTRO *oldspectro;
int i,n;
FILE *fp;
int spectrogram;
XmFileSelectionBoxCallbackStruct *c_data;
extern Widget oops;

   c_data = (XmFileSelectionBoxCallbackStruct *)call_data;
   XtUnmanageChild(filesb_dialog);
   XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);
   mstr = XmStringCopy(c_data->value);


   XmStringGetLtoR(mstr,XmSTRING_DEFAULT_CHARSET,&temp);
   XmStringFree(mstr);
   strcpy(str,temp); XtFree(temp);
 
   /* from file browser could have ; in str on VMS*/
   /* strip this off and check for .wav */

       if(strcmp(&str[strlen(str) - 3 ],"wav") != 0 &&
          strcmp(&str[strlen(str) - 3 ],"WAV") != 0  ){
          strcat(str,".wav");        
        }   


   if(!(fp = fopen(str,"r")) ){
    sprintf(mess,"can't open: %s\n",str);
    ShowOops(mess);
       return ;  /* no file*/  
     }/* can't find file */
    fclose(fp);

   LoadNewWaveform((XSPECTRO *)client_data, str);


   /* Save current working directory: */
   current_dir = XmStringCopy(c_data->dir);

}/* end newwave */


/*******************************************************************/
/*   void LoadNewWaveform(XSPECTRO *oldspectro, char *filename)            */
/*******************************************************************/
void LoadNewWaveform(XSPECTRO *oldspectro, char *filename){
XSPECTRO *spectro;
XSPECTRO **tmplist;
int i,n;
int spectrogram = 0;

   spectro = (XSPECTRO *) malloc( sizeof(XSPECTRO) );

   tmplist = (XSPECTRO **) malloc( sizeof(XSPECTRO *) * 
                      (allspectros.count + 1));

  /* update the allspectros list */

  for( i = 0; i < allspectros.count; i++)
         tmplist[i] = allspectros.list[i];

  tmplist[allspectros.count] = spectro;

  allspectros.count++;
  free(allspectros.list);
  allspectros.list = 
      (XSPECTRO **) malloc( sizeof( XSPECTRO *) * (allspectros.count));
  for( i = 0; i < allspectros.count; i++)
    allspectros.list[i] = tmplist[i] ;
  free(tmplist);

   spectro->swap = allspectros.list[0]->swap;
   spectro->spectrogram = spectrogram;
   spectro->devwidth = allspectros.list[0]->devwidth;
   spectro->devheight = allspectros.list[0]->devheight;
   spectro->wchar = allspectros.list[0]->wchar;
   spectro->hchar = allspectros.list[0]->hchar;

   //strcpy(spectro->wavefile,filename);

   add_spectro(spectro,"xkl_defs.dat",filename);
   /* copy current spectrum parameters */
   n = XtNumber(spectro->params);
   /* this overwrites the defaults values used in add_spectro*/
   /* see how this works before cleaning things up */
   for(i = 0; i < 4; i++){
   spectro->hamm_in_ms[i] = oldspectro->hamm_in_ms[i];
   spectro->params[i] = 
            spectro->hamm_in_ms[i]  * spectro->spers / 1000.00 + .5;
   }
   for(i = 4; i < n; i++) spectro->params[i] = oldspectro->params[i];
   
   initime(spectro); 
   createwindows(spectro);
   mapwindows(spectro);
   writcurpos(spectro);/* time in text window */

}/* end newstuff*/


/*******************************************************************/
/*void  swapwave(Widget w, XtPointer client_data, XtPointer call_data) */
/*******************************************************************/
void swapwave(Widget w, XtPointer client_data,XtPointer call_data){
/* read in new waveform but keep previous widgets */
/* free all old spectro pointers */
Arg args[3];
char *temp,title[100],str[500];
XSPECTRO *spectro;
int i,n;
FILE *fp;
int swap;  /* flag for swapping bytes on sgi */
char mess[550];
XmFileSelectionBoxCallbackStruct *c_data;
extern Widget oops;
XmString mstr;


   c_data = (XmFileSelectionBoxCallbackStruct *)call_data;
   XtUnmanageChild(filesb_dialog);
   XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);

   spectro = (XSPECTRO *) client_data;

   n = 0;
   XtSetArg(args[n],XmNvalue,&temp); n++;      
   XtGetValues(XmFileSelectionBoxGetChild(filesb_dialog,
              XmDIALOG_TEXT),args,n);

   strcpy(str,temp);
   XtFree(temp);

#ifdef VMS
    for(i = strlen(str) - 1; i >= 0; i-- )
         if(str[i] == ';') break;

         str[i] = '\000';
#endif
       if(strcmp(&str[strlen(str) - 3 ],"wav") != 0 &&
          strcmp(&str[strlen(str) - 3 ],"WAV") != 0  ){
          strcat(str,".wav");        
        }

   /* does file exist is it a .wav file */
   if(!(fp = fopen(str,"r")) ){ 
       sprintf(mess,"can't open: %s\n",str);
       ShowOops(mess);
       return ;  /* no file*/  
     }/* can't find file */
    fclose(fp);

   delete_wave(spectro);
   strcpy(spectro->wavefile,str);
   swapstuff(spectro);
   draw_spectrum(spectro, 0, NULL);/* this draws onto the pixmap */ 
   for (i = 0; i < NUM_WINDOWS; i++) {
      update(spectro->info[i], spectro, i);
   }

/* get current directory so that file selection box can remember position */
/* getcwd (current_dir, DIR_LENGTH); */
current_dir = XmStringCopy(c_data->dir);


}/* end swapwave */

/**************************************************************/
/*             dowavtitle(XSPECTRO *spectro)                  */
/**************************************************************/
void dowavtitle(XSPECTRO *spectro) {
   char title[400], totdur[50], wedur[50];
   char *space = "          ";
   int i, n;
   double tmpdur;
   Arg args[3];

   if (spectro->iwave) {

     tmpdur = spectro->totsamp/spectro->spers;
     if (tmpdur < 1) sprintf(totdur, "%3.1f ms", tmpdur * 1000);
     else sprintf(totdur, "%3.1f s", tmpdur);

     sprintf(wedur, "%3.1f ms", 1000*(spectro->endmarker - 
				      spectro->startmarker)/spectro->spers);

     sprintf(title, "%s :0%sSF: %d Hz%sXKL v%s%sTotal Dur: %s%sw-e Dur: %s",
	     spectro->wavename, space, (int) spectro->spers, space, VERSION,
	     space, totdur, space, wedur);
   }
   else {
     sprintf(title, "%s :0%sSF: %d Hz%sXKL v%s%sTotal Dur: %s%sw-e Dur: %s",
	     spectro->wavename, space, (int) spectro->spers, space, VERSION,
	     space, "0", space, "0");
   }

   n = 0;
   XtSetArg(args[n], XmNtitle, title); n++;
   XtSetArg(args[n], XmNiconName, title); n++;
   XtSetValues(spectro->toplevel[0],args,n);
}/* end dowavtitle */


/**************************************************************/
/*             dotitles(XSPECTRO *spectro)                    */
/**************************************************************/
void dotitles(XSPECTRO *spectro) {
   char title[220];
   int i, n;
   Arg args[3];

   dowavtitle(spectro);
   for(i = 1; i < NUM_WINDOWS; i++) {
      sprintf(title,"%s :%d",spectro->wavename,i);
      n = 0;
      XtSetArg(args[n], XmNtitle, title); n++;
      XtSetArg(args[n], XmNiconName, title); n++;
      XtSetValues(spectro->toplevel[i],args,n);
   }
}/* end dotitles */

/**************************************************************/
/*            swapstuff(XSPECTRO *spectro)                    */
/**************************************************************/
void  swapstuff(XSPECTRO *spectro) {
int i,n;
Arg args[3];
char title[110];


   wave_stuff(spectro);
   initime(spectro);
   spectro->history = 0;

  dotitles(spectro);

     
    /* update title of parameter window if it is active */
   if(spectro->param_active){
      XtSetArg(args[0],XmNtitle,spectro->wavename);
      XtSetValues(param_tl, args,1);
    }

  if(spectro->spectrogram){
       if(!resizegram(spectro))
          update(spectro->info[GRAM],spectro,GRAM); 
     }

       for(i = 0; i < 2; i++){
           make_pixmap(spectro->info[i],spectro,i);

           XCopyArea(spectro->info[i]->display,spectro->info[i]->pixmap,
           spectro->info[i]->win,spectro->info[i]->gc,
           0,0,spectro->xr[i],spectro->yb[i],0,0);
           
           draw_cursor(spectro,i,spectro->info[i]);
         }

   update(spectro->info[SPECTRUM],spectro,SPECTRUM);
   writcurpos(spectro);/* time in text window */

   recstart(spectro,0);

}/* end swapstuff*/

/**************************************************************/
/* settime(Widget w,XtPointer client_data,XtPointer call_data)   */
/**************************************************************/
void settime(Widget w,XtPointer client_data, XtPointer call_data){

XSPECTRO *spectro;
Arg args[3];
int i;
char str[500];
char *temp;


   XtUnmanageChild(input_dialog);
   spectro = (XSPECTRO *) client_data;

   XtSetArg(args[0],XmNvalue,&temp);
   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   strcpy(str,temp);
   spectro->oldindex = spectro->saveindex;
   spectro->oldtime = spectro->savetime;
   spectro->savetime = (float)atof(str) + spectro->timeoffset;
   spectro->saveindex =  (spectro->savetime  * spectro->spers / 1000.0) + .5;
   spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000;
   validindex(spectro); 

       for(i = 0; i < NUM_WINDOWS; i++){
           update(spectro->info[i], spectro, i);
         }
     writcurpos(spectro);/* time in text window */

   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);


/*the unmap callback is called after this so callbacks are removed there*/
}/* end settime */


/**************************************************************/
/*    settime_lock(Widget w,XtPointer client_data,XtPointer call_data)   */
/**************************************************************/

void settime_lock (Widget w, XtPointer client_data, XtPointer call_data)
{
  int i;
  /* LOCK: update times for all locked waveforms */
  for( i = 0; i < allspectros.count; i++) 
    if (allspectros.list[i]->locked) 
      {
	settime(w, (XtPointer) allspectros.list[i], call_data);
      }
}


/**************************************************************/
/*    setms(Widget w,XtPointer client_data,XtPointer call_data)   */
/**************************************************************/

void setms(Widget w,XtPointer client_data, XtPointer call_data){

XSPECTRO *spectro;
INFO *info;
Arg args[3];
int i;
char *str;
char temp[500];

   XtUnmanageChild(input_dialog);
   spectro = (XSPECTRO *) client_data;
   XtSetArg(args[0],XmNvalue,&str);
   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   strcpy(temp,str);

   if((float)atof(temp) != spectro->specms && spectro->spectrogram){
      info = spectro->info[GRAM];
      spectro->specms = (float)atof(temp);
      win_size(spectro);
      gramoffset(spectro);
      if(!resizegram(spectro))
	{   /* window didn't change size */
            /* copy new pixmap and cursor here*/
            draw_cursor(spectro,GRAM,info);
	  }/* no resize callback */

    }/* ms changed */


   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);

   sprintf(temp,"%.1f ms in spectrogram\n",spectro->specms);
   writetext(temp);
}/* end setms*/

/**************************************************************/
/*    writedata(Widget w,XtPointer client_data,XtPointer call_data)   */
/**************************************************************/

void writedata(Widget w,XtPointer client_data, XtPointer call_data){

XSPECTRO *spectro;
INFO *info;
Arg args[3];
int i,n;
char fname[300];
char *temp,mess[350];
FILE *fp;
XmString mstr;
extern XmString save_dir;
XmFileSelectionBoxCallbackStruct *c_data = (XmFileSelectionBoxCallbackStruct *) call_data;

//   XtUnmanageChild(input_dialog);
   XtUnmanageChild(filesb_dialog);
   spectro = (XSPECTRO *) client_data;
   save_dir = XmStringCopy(c_data->dir);


   XtSetArg(args[0],XmNvalue,&temp);
//   XtGetValues(XmSelectionBoxGetChild(input_dialog,XmDIALOG_TEXT),args,1);
   XtGetValues(XmFileSelectionBoxGetChild(filesb_dialog,XmDIALOG_TEXT),args,1);
   strcpy(fname,temp);
   XtFree(temp);

//   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
//   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
   XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);



  if(!(fp = fopen(fname,"r"))   ){
      /* can write with impunity */



    if(!(fp = fopen(fname,"w")) ){
       sprintf(mess,"can't open: %s\n",fname);
       ShowOops(mess);
       return ; 
     }/* can't find file */


    else{
      /* if changes are made here change dowritedata as well*/

      fprintf(fp,"%d     number of samples\n",(int) spectro->sampsplay);
      fprintf(fp,"%f     samples per second\n",spectro->spers);  
      for(i = 0; i < spectro->sampsplay; i++)
        fprintf(fp,"%d\n",spectro->startplay[i]);

      sprintf(mess,"wrote: %s\n",fname);
      writetext(mess);   
      fclose(fp);
    }/*wrote file*/

  }/* don't have to worry about overwrite */


  /* the file did exist so popup overwrite query*/
  else{
 /* was opened for read */
  fclose(fp);
    if(!(fp = fopen(fname,"w")) ){
       sprintf(mess,"can't open: %s\n",fname);
       ShowOops(mess);
       return ;   
     }/* can't open file */


   else{
    /* was opened for write */
   fclose(fp);   
   strcpy(spectro->tmp_name,fname);
   sprintf(mess,"OVERWRITE %s ??",fname);
   mstr = XmCvtCTToXmString(mess);
   n = 0;
   XtSetArg(args[n], XmNmessageString,mstr); n++;
   XtSetValues(warning,args,n);
   XmStringFree(mstr);
   mstr = XmCvtCTToXmString("OK");
   XtSetArg(args[0], XmNokLabelString,mstr);
   XtSetValues(warning,args,1);
   XmStringFree(mstr);
   XtRemoveAllCallbacks(input_dialog, XmNokCallback);
   XtRemoveAllCallbacks(input_dialog, XmNcancelCallback);  
   XtAddCallback(warning,XmNokCallback,dowritedata,(XtPointer)spectro);
   XtAddCallback(warning,XmNcancelCallback,cancelwarning,NULL);
   opendialog(warning);
   }/* popup warning window */
  }/*could read file check for write or overwrite */


}/* end writedata*/


/****/
void dowritedata(Widget w,XtPointer client_data, XtPointer call_data){
XSPECTRO *spectro;
int i;
FILE *fp;
char mess[550];

   XtUnmanageChild(warning);
   spectro = (XSPECTRO *) client_data;

   XtRemoveAllCallbacks(warning, XmNokCallback);
   XtRemoveAllCallbacks(warning, XmNcancelCallback);



   /* file has already been checked so go ahead and open it */
   fp = fopen(spectro->tmp_name,"w");


   fprintf(fp,"%d     number of samples\n",(int) spectro->sampsplay);
   fprintf(fp,"%f     samples per second\n",spectro->spers);  
   for(i = 0; i < spectro->sampsplay; i++)
        fprintf(fp,"%d\n",spectro->startplay[i]);

   fclose(fp);
   sprintf(mess,"wrote: %s\n",spectro->tmp_name);
   writetext(mess);

}/* end dowritedata */


/*********************************************************************/
/*             findseg(XSPECTRO *spectro,int window )                 */
/*********************************************************************/
void findseg(XSPECTRO *spectro,int window ){
/* based on window and segmode find section
   of waveforem to be played 
*/
int h, t, n;
  switch (spectro->segmode) {
  	case 1: /* write w to e */
   		if(spectro->endmarker > spectro->startmarker){
    		spectro->sampsplay = spectro->endmarker - spectro->startmarker + 1;
    		spectro->startplay = &spectro->iwave[spectro->startmarker];
    	}/* good segment */
  		break;
  	case 2: // play mouseDn : mouseUp selection
  		h = spectro->clickedWinTime * spectro->spers / 1000.00 + .5;
  		t = spectro->saveindex;
  		if (h > t) {
  			n = h;
  			h = t;
  			t = n;
  		}
  		spectro->sampsplay = t - h + 1;
  		spectro->startplay = &spectro->iwave[h];
  		break;
  	case 0: /* write part of waveform displayed in window */
       if(window == 1){
          spectro->sampsplay = spectro->sampsused[window];
          spectro->startplay = &spectro->iwave[spectro->startindex[window] ];
       }/* window 1 */
       else { /* if any window besides 1 is active write window 0 */
          spectro->sampsplay = spectro->sampsused[0];
          spectro->startplay = &spectro->iwave[spectro->startindex[0] ];
       }/*window 0*/
	   break;
  }
}/* end findseg*/


/***********************************************************************/
/*              addgram(XSPECTRO *spectro)                              */
/***********************************************************************/
void addgram(XSPECTRO *spectro){
  /* assumes spectro already has wave information*/
  Arg args[2];
  int n;
  
  spectro->spectrogram = 1;
  
  if (readgram(spectro) == 0){
    findsdelta(spectro);
    
    findypix(spectro);
    
    calculate_spectra(spectro);   
    
    writegram(spectro); 
    
  }/* calculate and save */
  /* see how many ms to display on screen */
  win_size(spectro);
  n = 0;
  XtSetArg(args[n], XmNwidth, spectro->xr[GRAM] );n++;
  XtSetArg(args[n], XmNheight,spectro->yb[GRAM]+spectro->menu_height);n++;
  XtSetValues(spectro->toplevel[GRAM],args,n);
  n = 0;
  XtSetArg(args[n], XmNwidth, spectro->xr[GRAM] );n++;
  XtSetArg(args[n], XmNheight,spectro->yb[GRAM] );n++;
  XtSetValues(spectro->draww[GRAM],args,n);
  XtManageChild(spectro->toplevel[GRAM]);
  XMoveWindow(XtDisplay(spectro->toplevel[GRAM]),
	      XtWindow(spectro->toplevel[GRAM]),
	      spectro->ox[GRAM],spectro->oy[GRAM]);
  setupcmap(spectro->draww[GRAM],spectro->info[GRAM]);

  gramoffset(spectro);/* find startindex of spectrogram */ 
  resizegram(spectro);
  /* activate spectrogram entries on menus */
  recstart(spectro,0);

} /* add spectrogram display*/

