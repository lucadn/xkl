/* $Log: xspec_util.c,v $
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

mstox(XSPECTRO *spectro, int window, float ms)
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
  fprintf(fp, "0 0 m (SF = %d Hz, %.1fms Hamming window every %.1fms, %d pt. DFT) o\n",
    (int)spectro->spers,spectro->winms,spectro->msdelta,spectro->slice);
  fprintf(fp,"grestore gsave\n");
/* end date and filename*/
 
 /* axes etc. */
/* debug */
/* fprintf(fp,
  "/Times-Roman findfont %d scalefont setfont\n",spectro->hchar); */
 fprintf(fp,"%f 72 translate 90 rotate\n", 
         (spectro->yb[GRAM] - 1) * scale + 144);
 fprintf(fp,"%f %f scale\n",scale,scale);
/* fprintf(fp,
  "/Times-Roman findfont %d scalefont setfont\n", spectro->hchar);   */

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
char filename[200], mess[300];


  /* return 0 if error reading file, 1 if ok */

  sprintf(filename,"%s.gram",spectro->wavename);
  /* error checking */
  if(!(fp = fopen(filename,"r")))
       return(0);  /* no file*/

 if(!fscanf(fp,"%d %d %f %f %d %d %f %d %d %f %f\n", &spectro->xgram,
      &spectro->wymax,&spectro->spers,&spectro->winms,
      &spectro->slice,&spectro->numav,&spectro->msdelta,
      &spectro->xpix,&spectro->ypix,&spectro->maxmag,&spectro->minmag)){

printf ("ypix = %d\n", spectro->ypix);
printf ("xpix = %d\n", spectro->xpix);
      printf("error reading file\n");
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
          printf("error reading file\n");
          return(0);
	}/* read error */
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
printf(mess);


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

if(fp = fopen(spectro->fname,"r")){

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

for(p = 0; p < NUM_WINDOWS; p++) spectro->info[p]->pixmap = 0;

startup(spectro,defname,wavefile);

wave_stuff(spectro);

}/* end  add_spectro*/
/*****************************************************************/
/*           strip_path(char *in, char *out)                     */
/*****************************************************************/
strip_path(char *in, char *out){
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

    /*    writegram(spectro);  */
    
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




/****************************************************************/
/*              delete_wave(XSPECTRO *spectro)                  */
/****************************************************************/
void delete_wave(XSPECTRO *spectro) {
int i;

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
printf ("winsamps = %f\n", spectro->winsamps);
printf ("ovlap = %f\n", spectro->ovlap);


    spectro->xmaxests = (spectro->specsize - spectro->winsamps) /
                   ((spectro->winsamps - spectro->ovlap) * spectro->numav);

    spectro->wxmax = spectro->xpix  * (spectro->xmaxests ) ;

printf ("devwidth = %f\n", spectro->devwidth);

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

printf ("wxmax = %d\n", spectro->wxmax);
printf ("wymax = %d\n", spectro->wymax);

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
/*
if(spectro->wymax > spectro->devheight ){
      spectro->wymax = spectro->devheight ;
      spectro->ypix = spectro->wymax / (spectro->slice / 2);

   
      spectro->wxmax = (spectro->sampsused[GRAM] / spectro->spers * 1000) /
      spectro->spratio  * spectro->wymax;

   }
else{
  spectro->ypix = spectro->wymax / (spectro->slice / 2);
  }
*/

  spectro->ypix = spectro->wymax / (spectro->slice / 2);
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

  printf("calculating...\n");

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
  spectro->allgvs = (char *) malloc(totgvs);
/***debug**/
  for(i=0; i < totgvs; i++)
       spectro->allgvs[i] = (char)0;


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
   /*    writegram(spectro); */
    XDestroyImage(spectro->xim);
    /* XDestroyImage doesn't set it to NULL*/
    /* assume for now that this deallocates spectro->pix */ 
    spectro->xim = NULL;

    /*    make_pixmap(info,spectro,GRAM); */

    /*    draw_cursor(spectro,GRAM,info); */

 }/* end remapgray */


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

/*****************************************************************/
/*  spectro_pixmap(INFO *info,XSPECTRO *spectro,int index);       */
/*****************************************************************/
void spectro_pixmap(INFO *info,XSPECTRO *spectro,int index){

int i,t,fq;
char *pix;
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
}/* monochrome */
        
 else{
   /* this may have to be changed for workstations
   that have 16 or 32 bits_per_pixel   */

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
  bitspp = 8;
 }/* more than one bit plane */


  spectro->xim =  XCreateImage(info->display,
           DefaultVisual(info->display,info->screen_num),
           DefaultDepth(info->display,info->screen_num),
           ZPixmap,0,spectro->pix,(unsigned int)spectro->xgram,
           (unsigned int)spectro->wymax ,8, bytesperline );

spectro->xim->bits_per_pixel = bitspp;

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
/* src */   spectro->t0,0,
/* dest*/   xoffset(spectro),yoffset(spectro),
/* w,h */  spectro->wxmax,spectro->wymax);

   
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

if(spectro->saveindex < 0){
   spectro->saveindex = 0;
   spectro->savetime = 0.0;
  }
else if(spectro->saveindex > spectro->totsamp - 1){
  spectro->saveindex = spectro->totsamp - 1;
  printf ("totsamp = %f\n", spectro->totsamp);
 spectro->savetime = (float)spectro->saveindex / spectro->spers * 1000; 

  }
}


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
findymax(XSPECTRO *spectro, int index) {
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
char str[10];
static char sectxt[] = "TIME (sec)";
static char mstxt[] = "TIME (ms)";
int xmax, ymax;
float ms, fx , inc, fy;
int yps;  /* switch X ycoord to postscript ycoord */
float max, min, mid, mv = 0, map; /* waveform y-axis*/

xmax = findxmax(spectro, index);
ymax = findymax(spectro, index);

/*  XSetForeground(info->display,info->gc,
                  info->color[1].pixel);
*/

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
      printf ("An error occurred.\n");
    }
  }/*sec*/

else{
    if(postfp != NULL){
    fprintf(postfp, "%d %d m (%s) o\n", x, yps - y, mstxt);
    }
    else{
      printf ("An error occurred.\n");
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
      printf ("An error occurred.\n");
    }


  }/*kHz label*/


printf ("wchar = %d\n", spectro->wchar);
printf ("hchar = %d\n", spectro->hchar);
printf ("yb = %d\n", spectro->yb[GRAM]);
printf ("xr = %d\n", spectro->xr[GRAM]);


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
    printf ("An error occurred.\n");
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
     printf ("An error!.\n");
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
rborder(XSPECTRO *spectro, int index) {
/*right border*/
return(spectro->xr[index] - 4 * spectro->wchar);
}
/******************************************************************/
/*                 bborder(XSPECTRO *spectro)                      */
/******************************************************************/
bborder(XSPECTRO *spectro, int index) {
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

   strcpy(spectro->wavefile,filename);
   add_spectro(spectro,"xkl_defs.dat",spectro->wavefile);


   /* copy current spectrum parameters */
   n = XtNumber(spectro->params);

   /* this overwrites the defaults values used in add_spectro*/
   /* see how this works before cleaning things up */
   for(i = 0; i < 4; i++){
     spectro->hamm_in_ms[i] = oldspectro->hamm_in_ms[i];
     spectro->params[i] = 
       spectro->hamm_in_ms[i]  * spectro->spers / 1000.00 + .5;
   }

   for(i = 4; i < n; i++)
     spectro->params[i] = oldspectro->params[i];
   
   initime(spectro); 

}/* end LoadNewWaveform */


/**************************************************************/
/*             dowavtitle(XSPECTRO *spectro)                  */
/**************************************************************/
dowavtitle(XSPECTRO *spectro) {
   char title[300], totdur[50], wedur[50];
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
dotitles(XSPECTRO *spectro) {
   char title[200];
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


/*********************************************************************/
/*             findseg(XSPECTRO *spectro,int window )                 */
/*********************************************************************/
void findseg(XSPECTRO *spectro,int window ){
/* based on window and segmode find section
   of waveforem to be played 
*/

  if(spectro->segmode){
  /* write w to e */
   if(spectro->endmarker > spectro->startmarker){
    spectro->sampsplay = spectro->endmarker - spectro->startmarker + 1;
    spectro->startplay = &spectro->iwave[spectro->startmarker];
    }/* good segment */
  }/*w to e*/
  else{
  /* write part of waveform displayed in window */
       if(window == 1){
          spectro->sampsplay = spectro->sampsused[window];
          spectro->startplay = &spectro->iwave[spectro->startindex[window] ];
       }/* window 1 */
      else{
         /* if any window besides 1 is active write window 0 */
         spectro->sampsplay = spectro->sampsused[0];
         spectro->startplay = &spectro->iwave[spectro->startindex[0] ];
       }/*window 0*/

     }/*window mode*/
  }/* end findseg*/




/**************************************************************
XklSpec.c stuff

/**************************************************************




/****************************************************************/
/*        printspectrum(spectro)                                */
/****************************************************************/
void printspectrum(XSPECTRO *spectro, FILE *fp)
{
  float scale, tmpscale, tmpscale1;
     
  scale = 1.0;

  /* do xr + yb fit on page (.5 inch border, 792 x 612)*/

  /* just set some default values */
  spectro->xr[SPECTRUM] = 720;
  spectro->yb[SPECTRUM] = 540;

  if ( spectro->xr[SPECTRUM] > 720  && spectro->yb[SPECTRUM] > 540){
    scale = (float)720 /(float)spectro->xr[SPECTRUM] ;
    tmpscale = (float)540 / (float)spectro->yb[SPECTRUM] ;
    if(tmpscale < scale) scale = tmpscale;
    
  } /* check to see which scale prevails */
  else if(spectro->xr[SPECTRUM] > 720 )
    scale = (float)720 / (float)spectro->xr[SPECTRUM];
  else  if(spectro->yb[SPECTRUM] > 540)
    scale = (float)540 / (float)spectro->yb[SPECTRUM] ;
  else {
/*    tmpscale = (float)720 / (float)spectro->xr[SPECTRUM];
    tmpscale1 = (float)540 / (float)spectro->yb[SPECTRUM];
    scale = ((tmpscale > tmpscale1)? tmpscale1 : tmpscale); */
  }
  spectro->gscale = 1;

  fprintf(fp,"%c%cBoundingBox: 72 72 %d %d\n",'%','%',
	  (int)( spectro->yb[SPECTRUM] * scale) + 106, 
	  (int)(spectro->xr[SPECTRUM] * scale) + 36);

  fprintf(fp,"gsave\n");
  
  fprintf(fp,"/l {2 copy lineto stroke m} def\n");
  fprintf(fp,"/m {moveto} def /o {show} def\n");
  fprintf(fp,"/k {lineto stroke } def\n");
  
  /*fprintf(fp," .2 setlinewidth\n");*/
  
  /* may not always scale char width properly but should work in most cases */
  
/*  fprintf(fp,
	  "/Times-Roman findfont %d scalefont setfont\n", spectro->hchar);*/
  fprintf(fp,"%f %d translate 90 rotate\n", scale * spectro->yb[SPECTRUM] + 106,
	  36);
  fprintf(fp,"%f %f scale\n",scale, scale);
  draw_spectrum(spectro, 0, fp);
  fprintf(fp,"\nshowpage grestore grestore\n");

} /* end printspectrum */


/*************************************************************/
/*            new_spectrum(spectro)                          */
/*************************************************************/
void new_spectrum(XSPECTRO *spectro){

INFO *info;
int firstsamp;
int i;

    info = spectro->info[SPECTRUM];

    /* everytime there is a new spectrum set all the spectrum
       cursor values to -1 */

    spectro->specfreq = -1;
    spectro->specamp = -1.0;
    spectro->savspecfreq = -1;
    spectro->savspecamp = -1.0;


    /* if an average is currently being displayed
       switch back to previous mode this allows resize of averaging*/
 
    if(spectro->option == 'a' || spectro->option == 'A' ||
       spectro->option == 'k' ){
                  spectro->option = spectro->savoption;
                  spectro->history = 0; 
		}

   /* based on the spectrum combination */
   /* spectro->fltr is calculated if cursor is not to close to end of file*/

   /* look at the top of xkl.c for info on spectro->params[] */

   /* if the params value is the same for all spectra( ex.fd coeff)
      then it is accessed directly in quickspec otherwise it is
      set in this module (ex. sizwin) 
   */


    if(spectro->option == 'd'){
   /*DFT magnitude*/
    /* assume that sizwin and sizfft are compatible(setparam)*/
    spectro->sizwin = spectro->params[WD];
    spectro->sizfft = spectro->params[SD];
    spectro->type_spec = DFT_MAG;
    spectro->nsdftmag = (spectro->sizfft >> 1);
    firstsamp = spectro->saveindex - (spectro->sizwin >> 1);
    if(spectro->sizwin <= spectro->sizfft){
     if(firstsamp >= 0 && firstsamp <= spectro->totsamp - 1){
        spectro->spectrum = 1;
        quickspec(spectro,&spectro->iwave[firstsamp]);
     }/* new spectrum */
     else { spectro->spectrum = 0;
     }/* index too close to end of file for dft */
    }/* window size ok for dft */
    else{
      spectro->spectrum = 0;
      printf("Window greater than DFT, reset parameters.\n");
      }/*spectro->sizwin > spectro->sizfft*/

     }/*d*/

    else if(spectro->option == 's'){
    /*spectrogram-like*/
      if(spectro->history) copysav(spectro);
      dofltr(spectro,SPECTROGRAM,spectro->params[WS]);
    }/*s*/

   else if(spectro->option == 'S'){
   /*spectrogram-like + DFT*/
    /*DFT savfltr*/
    savdft(spectro,spectro->params[WS]);/*make sizwin the same*/
    /*spectrogram-like fltr*/
    dofltr(spectro,SPECTROGRAM,spectro->params[WS]);
   }/*S*/

    else if(spectro->option == 'c'){
    /*critical-band*/
      if(spectro->history){
         copysav(spectro);
        }/*history*/
 
    dofltr(spectro,CRIT_BAND,spectro->params[WC]);
    }/*c*/

   else if(spectro->option == 'j'){
   /*critical-band + DFT*/
   /*DFT savfltr*/
    savdft(spectro,spectro->params[WC]);
   /*critical-band fltr*/
    dofltr(spectro,CRIT_BAND,spectro->params[WC]);
  }/*j*/


   else if(spectro->option == 'T'){
   /* put cb in fltr */ 
   dofltr(spectro,CRIT_BAND,spectro->params[WC]);
   /* if good spectrum,put slope in savfltr */
    if(spectro->spectrum){
     for(i = 0; i < spectro->lenfltr; i++)
           spectro->savfltr[i] = 0;
     tilt(&spectro->fltr[13],24,&spectro->savfltr[13]);
     /* make sure savfltr gets drawn */
      spectro->savspectrum = 1;
      spectro->lensavfltr = spectro->lenfltr;
      spectro->savspectrum = spectro->spectrum;
      spectro->savsizwin = spectro->sizwin;
      spectro->savspers = spectro->spers;
      strcpy(spectro->savname , spectro->wavename);
    }
   else {spectro->savspectrum = 0;}
   }/*T*/

   else if(spectro->option == 'l'){
   /*spectrogram-like + DFT*/
    /*DFT savfltr*/
    savdft(spectro,spectro->params[WL]);
    /*linear prediction*/
    dofltr(spectro,LPC_SPECT,spectro->params[WL]);
   }/*l*/

    /* store the value used for this spectrum for postscript*/

   spectro->fd = spectro->params[FD]; 
  
}/* end new_spectrum */



/***************************************************************/
/*      draw_spectrum(spectro,int resize,FILE *postfp)         */
/***************************************************************/
void draw_spectrum(XSPECTRO *spectro, int resize, FILE *postfp)
{
  float grdox,grdoy,grdxr,grdyb;
  int arox1,arox2,aroy1,aroy2,aroy3;
  int i,n;
  XPoint points[256];
  INFO *info;
  float xinc,yinc,x,y,halfsrate;
  int tmpfreq,asterisk;  /* freq of formants could be neg */
  int drawsav;  /* flag for drawing second fltr (or history)*/
  char str[200], str1[50],str2[50],str3[50],str4[50],str5[50];
  int ytext,y0,yps;
  float winms;
  char botlabel[30];
  float ps_scale;
  int drawarrows; /* flag for drawing formant markers*/
  time_t curtime;
  char datestr[50];
  int titleLoc;

  /* draw spectral data on screen or to postscript file, the resize */
  /* flag is so the formant info is written to the text window only once*/
  
  
  if (spectro->iwave == NULL) return;
  
  /* this string is for first difference */
  strcpy(str5,"");
  
  drawarrows = 1;
  if(postfp != NULL) { /* need to set ps_scale*/
    drawarrows = 0;   /* 'g' don't draw arrows */
    ps_scale = spectro->gscale;
    titleLoc = 330;
  } 
  
  /*NOTE:  X considers y = 0 to be at the top of the screen,
    postscript considers y = 0 to be at the bottom of the page*/
  
  /* actually print label after getform */
  /* label for formant info */
  if(spectro->option == 's' || spectro->option == 'S' ){
    strcpy(botlabel,"Smoothed");  }
  else if(spectro->option == 'c' || spectro->option == 'j' ||
          spectro->option == 't' || spectro->option == 'T'){
    strcpy(botlabel,"CB"); }
  else if(spectro->option == 'l'){
    strcpy(botlabel,"LPC"); }

  info = spectro->info[SPECTRUM];  /*clear*/
  spectrum_pixmap(spectro->info[SPECTRUM],spectro,postfp);

  /* in most cases draw fltr and savfltr */
  drawsav = 1;
  if(spectro->option == 'd' ||
     ((spectro->option == 'c' || spectro->option == 's') &&
              !spectro->history )     ){
       /* just draw spectro->fltr */
       drawsav = 0;
     }     

  grdoy = 0;
  grdyb = (float)spectro->yb[SPECTRUM] - 2 * spectro->hchar - 1;
  
  if(postfp!=NULL){
     grdyb = (float)spectro->yb[SPECTRUM] - 
                   (float)spectro->hchar * 2.0 * ps_scale - 1;
  }
  

  grdox = (float)spectro->xr[SPECTRUM] * SPEC_OX;
  grdxr = (float)spectro->xr[SPECTRUM] * SPEC_XR;

     halfsrate = spectro->spers / 2.0; 

     yinc = (float) grdyb/SPEC_DB;   /*display 60 dB */

     y0 = grdyb - 5.0 * yinc + .5; /* offset zero */
     /* y offset for postscript(yfltr isn't flipped) */
     
     yps = (float)spectro->hchar + 1 + 5.0 * yinc + .5;
     if(postfp != NULL){
       yps = (float)spectro->hchar * ps_scale + 1 + 5.0 * yinc + .5;
     }

/* for postscript output put sampling rate in bottom right corner */
   if(postfp != NULL && (spectro->spectrum || spectro->savspectrum) ){
      ytext = grdyb - 4 * (float)spectro->hchar * ps_scale;

      sprintf(str1,"%.1fms",spectro->savetime);
      sprintf(str2,"SF = %d Hz",(int)spectro->spers);
      sprintf(str3,"FD = %d",spectro->fd);
      sprintf(str4,"SG = %d", spectro->params[SG]);
      sprintf(str5,"");
      ps_toplabel(spectro,postfp,grdxr,ytext,str1,str2,str3,str4,str5);
  
    /* put date and filename outside of bounding box */  
    time(&curtime);
    strcpy(datestr, ctime(&curtime)); datestr[24] = ' ';
      fprintf(postfp, "%d %d m (%s) o\n", 45, (int) 
	      ((titleLoc + 2*spectro->hchar) * ps_scale), 
	      datestr);
      fprintf(postfp, "%d %d m (File: %s) o\n", 45, (int) 
	      ((titleLoc + spectro->hchar) * ps_scale), 
	      spectro->wavefile);
      fprintf(postfp, "%d %d m (Current Dir: %s) o\n", 45, (int) 
	      (titleLoc * ps_scale), "Fix this pleeease");

   }/* file name for postscript */

if (spectro->spectrum) {    /* draw based on spectro->option */

/* draw fltr */
     x = 0.0;
     xinc = halfsrate / spectro->lenfltr;
     for(i = 0; i < spectro->lenfltr; i++){
       if(spectro->option == 'c' || spectro->option == 'j'||
          spectro->option == 'T' || spectro->option == 'l')
          x = spectro->nfr[i];
      /* save for postscript output*/
       spectro->xfltr[i] = x / halfsrate * (grdxr - grdox) ;
      points[i].x = spectro->xfltr[i] + .5 + grdox;
      spectro->yfltr[i] = (float)spectro->fltr[i]/10.0 * yinc;
      points[i].y = y0 - spectro->yfltr[i] + .5;
      x += xinc;    
     } 


   if (postfp != NULL) {      /* SMOOTHED SPECTRUM */

    /* KKG 4/16/98.  Added +12 to shift up spectrum */

      fprintf(postfp, " 1 setlinewidth\n");
      fprintf(postfp,"%f %f m \n",
	      spectro->xfltr[0] + grdox, spectro->yfltr[0] + yps + 12);
      for(i = 1; i < spectro->lenfltr - 1; i++)
	 fprintf(postfp,"%f %f l\n",
		 spectro->xfltr[i] + grdox,spectro->yfltr[i] + yps + 12);
      fprintf(postfp, " 1 setlinewidth\n");
   }
   else {
      XDrawLines(info->display,info->pixmap,
		 info->gc, points,spectro->lenfltr,CoordModeOrigin);
   }

  winms = (float)spectro->sizwin/spectro->spers * 1000.0;
  sprintf(str2,"win:%.1fms",winms);

/* do toplabel */
     if(spectro->option == 'd' || spectro->option == 'a' ||
	spectro->option == 'A' || spectro->option == 'k') {

	ytext =  grdoy + 2 * spectro->hchar;
	if (postfp != NULL) {
	   ytext =  grdoy + 2 * (float)spectro->hchar * ps_scale;
	}

	if (spectro->option == 'd') {
	   strcpy(str1,"DFT");
	   sprintf(str3,"F0 = %d Hz",
		   getf0(spectro->fltr,spectro->sizfft,(int)spectro->spers));
	   sprintf(str4,"Rms = %d dB",spectro->fltr[spectro->lenfltr]/10); 
	} /*DFT*/
	if (spectro->option == 'k' || spectro->option == 'a' ||
	    spectro->option == 'A') {
	   sprintf(str3,"start %d",spectro->avgtimes[0]);
	   sprintf(str4,"end %d",spectro->avgtimes[1]);
	   if (spectro->option == 'A') {
	      /* need to list all times below */
	      strcpy(str1,"Avg DFT-spect (A)");
	      strcpy(str3,"");
	      strcpy(str4,"");
	   }
	   else if (spectro->option == 'k') strcpy(str1,"Avg DFT-spect (kn)");
	   else if (spectro->option == 'a') strcpy(str1,"Avg DFT-spect (a)");
       
   }/*avg*/

    if(postfp != NULL){
      ps_toplabel(spectro,postfp,grdxr,ytext,str1,str2,str3,str4,str5);
     /* if 'A' list all times */
       if(spectro->option == 'A'){
       ytext = grdoy + 3 * (float)spectro->hchar * ps_scale; 
          for(i = 0; i <= spectro->avgcount; i++){
          ytext += (float)spectro->hchar * ps_scale;
          sprintf(str3,"time%d: %d",i+1,spectro->avgtimes[i]);
          fprintf(postfp, "%d %d m (%s) o\n",
         (int)grdxr + spectro->wchar,spectro->yb[SPECTRUM] - ytext,str3); 
        }/*times*/ 
       }/*'A'*/   
    }/* postscript */
   else{
      sc_toplabel(spectro,info,grdxr,ytext,str1,str2,str3,str4,str5);
   /* if 'A' list all times */
      ytext = grdoy + 3 * spectro->hchar; 
       if(spectro->option == 'A'){
         for(i = 0; i <= spectro->avgcount; i++){
          ytext += spectro->hchar;
          sprintf(str3,"time%d: %d",i+1,spectro->avgtimes[i]);
          XDrawString(info->display,info->pixmap,info->gc,
           (int)grdxr + spectro->wchar ,ytext,str3,strlen(str3)  ); 
	}/*times*/
       }/*'A'*/
    }/* screen */

 }/*toplabel*/

 
  /* find formants */
  if(spectro->option == 's' || spectro->option == 'S' ||
     spectro->option == 'l' || spectro->option == 'j'||
     spectro->option == 'c' || spectro->option == 'T'){

        /* formant arrows */
	arox1 = (float)spectro->xr[SPECTRUM] * .008;
	arox2 = (float)spectro->xr[SPECTRUM] * .004;
	aroy1 = (float)spectro->yb[SPECTRUM] * .02;
	aroy2 = (float)spectro->yb[SPECTRUM] * .015;
	aroy3 = (float)spectro->yb[SPECTRUM] * .045;

      ytext =  grdoy + 7 * spectro->hchar;
       if(postfp != NULL){
          ytext =  grdoy + 7 * (float)spectro->hchar * ps_scale;
       }

/*bottom label*/
if(postfp != NULL){
  
    fprintf(postfp, "%d %d m (%s) o\n",
    (int)grdxr + spectro->wchar,spectro->yb[SPECTRUM] - ytext,botlabel);  
    ytext += (float)spectro->hchar * ps_scale;
    fprintf(postfp, "%d %d m (%s) o\n",
      (int)grdxr + spectro->wchar,spectro->yb[SPECTRUM] - ytext,str2); 
    ytext += (float)spectro->hchar * ps_scale;

    fprintf(postfp, "%d %d m (%s) o\n", (int)grdxr + spectro->wchar,
	    spectro->yb[SPECTRUM] - ytext, "FREQ AMP");
 }/*postscript*/
else{
   XDrawString(info->display,info->pixmap,info->gc,
      (int)grdxr + spectro->wchar ,ytext,botlabel,strlen(botlabel)  );
    ytext += spectro->hchar;
   XDrawString(info->display,info->pixmap,info->gc,
      (int)grdxr + spectro->wchar ,ytext,str2,strlen(str2)  );
    ytext += spectro->hchar;
   XDrawString(info->display,info->pixmap,info->gc,
      (int)grdxr + spectro->wchar ,ytext,"FREQ AMP",
      strlen("FREQ AMP")  );
 }/*screen*/

    getform(spectro);
    /* formant info used to go to test window
       now it is printed out with spectrum values 'V'
    if(!resize && !postfp) 
         writetext("\n");
     */


    if(postfp != NULL){
      ytext += (float)spectro->hchar * ps_scale;
    }
    else{
    ytext += spectro->hchar;
    }
       for(i = 0; i < spectro->nforfreq; i++){
         tmpfreq = spectro->forfreq[i];
         asterisk = 0;
         if(tmpfreq < 0){
            asterisk = 1;
            tmpfreq = -tmpfreq;
	 }
         x = (float)tmpfreq / halfsrate * (grdxr - grdox) + grdox + .5;
         y = y0 - (float)spectro->foramp[i]/10. * yinc -
             spectro->hchar/2 ;

         points[0].x = x ;            points[0].y = y ;             
         points[1].x = x - arox1 ;    points[1].y = y - aroy1 ;
         points[2].x = x - arox2 ;    points[2].y = y - aroy2 ;
         points[3].x = x - arox2 ;    points[3].y = y - aroy3 ;
         points[4].x = x + arox2 ;    points[4].y = y - aroy3 ;
         points[5].x = x + arox2 ;    points[5].y = y - aroy2 ;
         points[6].x = x + arox1 ;    points[6].y = y - aroy1 ;
         points[7].x = x ;            points[7].y = y ;


         /* print formants */
           sprintf(str,"%d %d",tmpfreq,(int)(spectro->foramp[i]/10) );
         /* text window
           if(!resize && !postfp)
              {writetext(str); if(asterisk){writetext(" *");}writetext("\n");}
         */

       if(postfp != NULL){
         if (drawarrows) {
         fprintf(postfp," .5 setlinewidth\n");
         fprintf(postfp,"%d %d m \n",
              points[0].x,spectro->yb[SPECTRUM] - points[0].y);
         for(n = 1; n < 8; n++)
                     {fprintf(postfp,"%d %d l\n", points[n].x,
                                 spectro->yb[SPECTRUM] - points[n].y);}
         fprintf(postfp," 1.0 setlinewidth\n");
	 }
         if (asterisk) {
	   fprintf(postfp, "%d %d m (%s) o\n",
           (int)grdxr  ,spectro->yb[SPECTRUM] - ytext,"*");}
         fprintf(postfp, "%d %d m (%s) o\n",(int) grdxr + spectro->wchar, 
		 spectro->yb[SPECTRUM] - ytext,str); 
       } /*write postscript*/
      else{
       XDrawLines(info->display,info->pixmap,
          info->gc, points,8,CoordModeOrigin);
       if(asterisk){XDrawString(info->display,info->pixmap,info->gc,
               (int)grdxr  ,ytext,"*",1  );}
       XDrawString(info->display,info->pixmap,info->gc,
             (int)grdxr + spectro->wchar ,ytext,str,strlen(str)  );

        }/* draw to screen*/


         if(postfp != NULL){
                ytext += (float)spectro->hchar * ps_scale;
	 }
         else {
                ytext += spectro->hchar;
	 }
	 }/* peaks */
       } /* arrows */

}/*good spectrum*/

 if(drawsav && spectro->savspectrum){ 
     x = 0.0;
     xinc =  spectro->savspers / 2.0 / spectro->lensavfltr;
     spectro->savclip = 0;
     for(i = 0; i < spectro->lensavfltr; i++){
       if(spectro->option == 'c' || spectro->option == 'T')
	 if(spectro->option == 'T'){
	   x = spectro->nfr[i];
         }
         else{
          x = spectro->savnfr[i];
	 }
       spectro->xsav[i] = x / halfsrate * (grdxr - grdox); 
       points[i].x = spectro->xsav[i] + .5 + grdox;    
       if(points[i].x <=  (int)grdxr  ){
           spectro->ysav[i] = (float)spectro->savfltr[i]/10. * yinc;      
           points[i].y =  y0 - spectro->ysav[i] + .5 ; 
           spectro->savclip++;
	 }/* clip if drawing 8k on 5k */ 
      x += xinc;    
     }

/* do toplabel*/
if(spectro->option == 'S' || spectro->option == 'j' ||
   spectro->option == 'l' || spectro->option == 's' ||
   spectro->option == 'c'){

      winms = (float)spectro->savsizwin/spectro->savspers * 1000.0;
      sprintf(str2,"win:%.1fms",winms);
      ytext =  grdoy +  1 * spectro->hchar;


    if(spectro->option == 's' || spectro->option == 'c'){
    /* do toplabel, History*/

      strcpy(str1,"History");
      if(postfp != NULL){
       strcpy(str1,"Dashed");                   
      }
      strcpy(str3,spectro->savname);
      sprintf(str4,"%.1f ms",spectro->oldtime);
      if(spectro->savfd != -1){
        sprintf(str5,"FD = %d",spectro->savfd);
      }

     }/*toplabel, History*/

    else if(spectro->option == 'S' || spectro->option == 'j' ||
      spectro->option == 'l' ){
      /* do toplabel, DFT F0 rms */
      strcpy(str1,"DFT");
      /* assume that the DFT was done with same sizfft as fltr */
      sprintf(str3,"F0 = %d Hz",
              getf0(spectro->savfltr,spectro->sizfft,(int)spectro->spers));
      sprintf(str4,"Rms = %d dB",spectro->savfltr[spectro->lensavfltr]/10); 
    }
 
   if(postfp != NULL){
      ps_toplabel(spectro,postfp,grdxr,ytext,str1,str2,str3,str4,str5);
    }/* postscript */
   else{
      sc_toplabel(spectro,info,grdxr,ytext,str1,str2,str3,str4,str5);
    }/* screen */


}/* toplabel , history or DFT with c,s,l */


/* draw sav spectrum*/

     if(postfp != NULL){ /* DFT spectra */
	fprintf(postfp, " 1 setlinewidth\n");
	if(spectro->option == 's' || spectro->option == 'c'){
	   fprintf(postfp,"[2] 0 setdash\n");
	}/* dashed line */
	fprintf(postfp,"%f %f m \n",
           spectro->xsav[0] + grdox,spectro->ysav[0] + yps);

    /* KKG 4/16/98.  Added +12 to shift up spectrum */

         for(i = 1; i < spectro->savclip - 1; i++)
                     fprintf(postfp,"%f %f l\n",spectro->xsav[i] + 
                      grdox, spectro->ysav[i] + yps + 12);
     if(spectro->option == 's' || spectro->option == 'c'){
        fprintf(postfp,"[] 0 setdash\n");
     }/* dashed line */
     fprintf(postfp, " 1 setlinewidth\n");
 }
  else{
   XDrawLines(info->display,info->pixmap,
          info->gc, points,spectro->savclip,CoordModeOrigin);
 }

 }/* savfltr and good savspectrum*/


if(  (spectro->option == 'c' || spectro->option == 's') &&
              !spectro->history   ){ 
               spectro->history = 1;
               spectro->savspectrum = 0; }

}/* end draw_spectrum */



/**************************************************************/
/* spectrum_pixmap(INFO *info,SPECTRO *spectro, FILE *postfp) */
/* draw grid and labels for spectrum window*/
/**************************************************************/
void spectrum_pixmap(INFO *info,XSPECTRO *spectro, FILE *postfp)
{
  int grdox,grdoy,grdxr,grdyb;
  int i,fmax,x,y,y0;
  int x2;
  float xinc,yinc,fx,fy;
  char str[10];
  int yps;
  float ps_scale;
  
  if(postfp != NULL) { /* need to set ps_scale*/
    ps_scale = spectro->gscale;
  } 
  
  grdoy = 0;
  /* add another line for text so the label can be centered
     below the tick marks   */
  /*grdyb = spectro->yb[SPECTRUM] - spectro->hchar;*/
  grdyb = spectro->yb[SPECTRUM] - 2 * spectro->hchar;
  if(postfp!=NULL)
    grdyb = spectro->yb[SPECTRUM] - 2 * 
      (float)spectro->hchar * ps_scale;
  
  grdox = spectro->xr[SPECTRUM] * SPEC_OX;
  grdxr = spectro->xr[SPECTRUM] * SPEC_XR;
  
  if(postfp == NULL){
    /* spectrum background */

   XSetForeground(info->display,info->gc,
                  info->color[2].pixel);

   XFillRectangle(info->display,info->pixmap,
     info->gc, 0,0,
     spectro->xr[SPECTRUM],spectro->yb[SPECTRUM]); 

   XSetForeground(info->display,info->gc,
                  info->color[1].pixel);
 }

   yps = spectro->yb[3];/* filp y for postscript*/
  
     /* outline */

  if(postfp != NULL) {
  fprintf(postfp,"%d setlinejoin\n",1);
  fprintf(postfp, "%d %d m %d %d k\n",
             grdox ,yps - grdoy ,grdox , yps - grdyb );
  fprintf(postfp, "%d %d m %d %d k\n",
             grdox ,yps - grdyb ,grdxr , yps - grdyb );
  fprintf(postfp, "%d %d m %d %d k\n",
             grdxr ,yps - grdoy ,grdxr , yps - grdyb );
  fprintf(postfp, "%d %d m %d %d k\n",
             grdox ,yps - grdoy ,grdxr , yps - grdoy );

  /* rotate the y axis label */
  fprintf(postfp,"gsave\n");

  /* at this point things have already been rotated 90 */
  /* there must be an easier way of doing this but I */
  /* don't know what it is */

  fprintf(postfp,"%d %d translate 90 rotate\n",spectro->xr[3],0);

    fprintf(postfp, "%d %d m (%s) o\n",
    (int) (spectro->yb[SPECTRUM] * .5 ) - 5 * 
    (int)((float)spectro->wchar * ps_scale),
    spectro->xr[3] - (grdox - (int)((float)spectro->wchar * ps_scale * 4)),
            "REL AMP (dB)");

   fprintf(postfp,"grestore \n");
  /* back to normal */
 
   
   fprintf(postfp, "%d %d m (%s) o\n",
    (int)(grdox + (grdxr - grdox) / 2   - spectro->wchar * 4 * ps_scale),
   yps - spectro->yb[SPECTRUM] + 1, "FREQ (kHz)");
   
   }/*postscript*/


   else {
   XDrawLine(info->display, info->pixmap,
          info->gc,grdox, grdoy, grdox, grdyb);
   XDrawLine(info->display, info->pixmap,
          info->gc,grdox, grdyb, grdxr, grdyb);
   XDrawLine(info->display, info->pixmap,
          info->gc,grdxr, grdoy, grdxr, grdyb);
   XDrawLine(info->display, info->pixmap,
          info->gc,grdox, grdoy, grdxr, grdoy);
   XDrawString(info->display,info->pixmap,info->gc,
   (int)grdox - 4 * spectro->wchar,grdoy + spectro->hchar,"dB",
      strlen("dB")  );
   XDrawString(info->display,info->pixmap,info->gc,
      grdox + (grdxr- grdox)/2 - (spectro->wchar * 4 ),
      spectro->yb[SPECTRUM] - 1,"FREQ (kHz)",
      strlen("FREQ (kHz)"));
    }

    /* gridlines */

    fmax = spectro->spers/2000.0;

    xinc = (float)(grdxr - grdox)/(spectro->spers/2000.0);

    fx =  (float)grdox + xinc ;

    x = fx + .5;
    x2 = fx - xinc/2.0 + .5;

    for( i = 1; i <=fmax; i++){
     sprintf(str,"%d", i);

      if(postfp != NULL){
      fprintf(postfp, " .5 setlinewidth\n");
      fprintf(postfp, "%d %d m %d %d k\n",
             x ,yps - grdoy ,x , yps - grdyb );
      fprintf(postfp, "%d %d m %d %d k\n",
             x2 ,yps - grdoy ,x2 , yps - grdyb );

      fprintf(postfp, " 1.0 setlinewidth\n");      
      fprintf(postfp, "%d %d m (%s) o\n",
             x - spectro->wchar / 2, 
             (int)(yps - (spectro->yb[SPECTRUM] - 
        (float)spectro->hchar * ps_scale)) + 1 ,str);
       
      }
      else{
       XDrawLine(info->display, info->pixmap,
          info->gc,x, grdoy, x, grdyb);
 

      XDrawLine(info->display, info->pixmap,
          info->gc,x2, grdoy, x2, grdyb);


         XDrawString(info->display,info->pixmap,info->gc,
          x - spectro->wchar / 2,
          spectro->yb[SPECTRUM] - spectro->hchar - 1,str,strlen(str));
       } 

       fx += xinc ;

       x = fx + .5;
       x2 = fx - xinc/2.0 + .5; 

     }/* vertical grid */

     yinc = (float)grdyb/SPEC_DB;  /* display 60 dB */

     y0 = grdyb - 5.0 * yinc + .5; /* offset zero */

     y = y0;

     x = grdox - 3 * spectro->wchar;
     for(i = 1; i <= 8; i++){

	sprintf(str,"%d", (i - 1) * 10);

      if(postfp != NULL){
       fprintf(postfp," .5 setlinewidth\n");
       fprintf(postfp, "%d %d m %d %d k\n",
            grdox ,yps - y ,grdxr , yps - y );
       fprintf(postfp," 1.0 setlinewidth\n");
       fprintf(postfp, "%d %d m (%s) o\n", 
               x, yps -  y - spectro->hchar / 2, str);
       
      }

       else{
         XDrawLine(info->display, info->pixmap,
          info->gc,grdox, y, grdxr, y);

         sprintf(str,"%d", (i - 1) * 10);
         XDrawString(info->display,info->pixmap,info->gc,
          x , y + spectro->hchar / 2, str, strlen(str));
       }
    
     fy = i * 10 * yinc;
     y =  y0 - fy - .5;
       
    }/* horizontal grid */
 
}/* end spectrum_pixmap */



/**********************************************************************/
/*           void   makefbank(XSPECTRO *spectro)                       */
/**********************************************************************/

/* used to take type_spec as a parameter */

    void makefbank(XSPECTRO *spectro) {
    int np,i1,i3,nsmin,n1000,nfmax;
    float x2,xfmax,fcenter,bwcenter,bwscal,bwscale,xbw1000,xsdftmag,linlogfreq;
    double a,b,c,d,temp1,temp2;


/* Initialization to make filter skirts go down further (to 0.00062) 
   put this in main, just do it once at startup 
    if (spectro->initskrt == 0) {
	spectro->initskrt++;

	for (np = 100; np<SIZCBSKIRT; np++) {
	    cbskrt[np] = 0.975 * cbskrt[np-1];
	}
	printf("\nMax down of cbskrt[] is %f  ",
                         cbskrt[SIZCBSKIRT-1]); 
    }
*/



/*    Highest frequency in input dft magnitude array */
	nfmax = (int)spectro->spers / 2;
	xfmax = nfmax;
	xsdftmag = spectro->nsdftmag;



if ((spectro->type_spec == LPC_SPECT) || ( spectro->type_spec== AVG_SPECT)) {

/*	    printf("\tCompute freq of each dft mag sample (nfr[%d])",
	     spectro->lenfltr); */
/*	  Reset nfr[] */
	    i3 = spectro->lenfltr / 2;	/* For rounding up */
	    for (i1=0; i1<spectro->lenfltr; i1++) {
		spectro->nfr[i1] = ((i1 * nfmax) + i3) / spectro->lenfltr;
	    }
	    return;
	  }

	if ( spectro->type_spec == CRIT_BAND) {
/*	  Do not include samples below 80 Hz */
	    nsmin = ((80 * spectro->nsdftmag) / nfmax) + 1;
/*	  Determine center frequencies of filters, use frequency spacing of
 *	  equal increments in log(1+(f/flinlog)) */
            linlogfreq = (float) spectro->params[FL];
	    temp1 = 1. + ((float)spectro->params[F1]/linlogfreq);
	    temp1 = log(temp1);
	    temp2 = 1. + ((float)spectro->params[F2]/linlogfreq);
	    temp2 = log(temp2);
	    a = temp2 - temp1;
	    b = (temp1/a) - 1.;
	    c = (xsdftmag*linlogfreq)/xfmax;
	}
	else {
	    nsmin = 1;
	}
	n1000 = (1000.0 * spectro->nsdftmag)/nfmax;
	xbw1000 = spectro->params[B1];
	xbw1000 = xbw1000/n1000;
	bwscal = (360.*xfmax)/xsdftmag;		/* 360 is a magic number */

	np = 0;
	spectro->nchan = 0;
	while (spectro->nchan < NFMAX) {

	if (spectro->type_spec == SPECTROGRAM || 
            spectro->type_spec == SPECTO_NO_DFT) {
/*	      Center frequency in Hz of filter i */
		fcenter = spectro->nchan;
/*	      Bandwidth in Hz of filter i */
		bwcenter = spectro->params[BS];
	    }

	    else {
/*	      Center frequency in Hz of filter i */

		d = a*(b + spectro->nchan + 1.0);
		fcenter = c * (exp(d) - 1.);

/*	      Bandwidth in Hz of filter i */
		bwcenter = xbw1000*fcenter;
		if (bwcenter < (float)spectro->params[B0]) {
		    bwcenter = spectro->params[B0];
	        }
	      }
/*	  See if done */
	    if (fcenter >= xsdftmag) {
		break;
	    }/*	  Center frequency and bandwidth in Hz of filter i */
	    spectro->nfr[spectro->nchan] = ((fcenter * xfmax) / xsdftmag) + .5;
	    spectro->nbw[spectro->nchan] = bwcenter;

/* printf("\nFILTER %d:  f=%4d bw=%3d:\n", nchan, nfr[nchan], nbw[nchan]); */

/*	  Find weights for each filter */
	    spectro->nstart[spectro->nchan] = 0;
	    spectro->ntot[spectro->nchan] = 0;

	    bwscale = bwscal/(10. * spectro->nbw[spectro->nchan]);
	    for (i1 = nsmin; i1 < spectro->nsdftmag; i1++) {
/*	      Let x2 be difference between i1 and filter center frequency */
		x2 = fcenter - i1;
		if (x2 < 0) {
		    x2 = -x2;
		    spectro->sizcbskirt = SIZCBSKIRT;	
                  /* Go down 22 dB on low side */
		}
		else spectro->sizcbskirt = SIZCBSKIRT>>1;	
                    /* Go down 32 dB on high side */

		i3 = x2 * bwscale;
		if (i3 < spectro->sizcbskirt) {
/*		  Remember which was first dft sample to be included */
		    if (spectro->nstart[spectro->nchan] == 0) {
			spectro->nstart[spectro->nchan] = i1;
		      }
/*		  Remember pointer to weight for this dft sample */
		     spectro->pweight[np] = i3; np++;

	    if (np >= NPMAX) {
		   printf("\nFATAL ERROR in MAKECBFB.C: 'NPMAX' exceeded\n");
		   printf("\t(while working on filter %d).  ", spectro->nchan);
		   printf("\tToo many filters or bw's too wide.\n");
		   printf("\tRedimension NPMAX and recompile if necessary.\n");
			exit(1);
		    }
/*		  Increment counter of number of dft samples in sum */
		    spectro->ntot[spectro->nchan]++;
		  }
	    }
	    spectro->nchan++;
	}
	spectro->sizcbskirt = SIZCBSKIRT;
/*    NCHAN is now set to be the total number of filter channels */


  }/* end makefbank */



/*************************************************************************/
/*ps_toplabel(spectro, postfp, grdxr,ytext, str1, str2, str3,str4)       */
/*************************************************************************/

ps_toplabel(spectro, postfp, grdxr,ytext, str1, str2, str3, str4, str5)
     XSPECTRO *spectro;
     FILE *postfp;
     float grdxr;
     int ytext;
     char *str1, *str2, *str3, *str4, *str5;
{

  float ps_scale;
  if (postfp != NULL) {/* need to set ps_scale*/
    ps_scale = spectro->gscale;
  }
    

  fprintf(postfp, "%d %d m (%s) o\n",
	  (int)grdxr + spectro->wchar,spectro->yb[SPECTRUM] - ytext,str1);
  ytext += (float)spectro->hchar * ps_scale;
  fprintf(postfp, "%d %d m (%s) o\n",
	  (int)grdxr + spectro->wchar,spectro->yb[SPECTRUM] - ytext,str2);
  ytext += (float)spectro->hchar * ps_scale;
  fprintf(postfp, "%d %d m (%s) o\n",
	  (int)grdxr + spectro->wchar,spectro->yb[SPECTRUM] - ytext,str3);
  ytext += (float)spectro->hchar * ps_scale;
  fprintf(postfp, "%d %d m (%s) o\n",
	  (int)grdxr + spectro->wchar,spectro->yb[SPECTRUM] - ytext,str4);
  if(strlen(str5) >= 1){
    ytext += (float)spectro->hchar * ps_scale;
    fprintf(postfp, "%d %d m (%s) o\n",
	    (int)grdxr + spectro->wchar,spectro->yb[SPECTRUM] - ytext,str5);
  }/* only print out FD with s or c */
  
 
}/* end ps_toplabel */
/**************************************************************************/
/*sc_toplabel(spectro, info, grdxr,ytext,str1, str2, str3,str4)*/
/**************************************************************************/
sc_toplabel(spectro, info, grdxr,ytext,str1, str2, str3,str4, str5)
     XSPECTRO *spectro;
     INFO *info;
     float grdxr;
     int ytext;
     char *str1, *str2, *str3, *str4, *str5;
{
  
  XDrawString(info->display,info->pixmap,info->gc,
	      (int)grdxr + 2 * spectro->wchar ,ytext,str1,strlen(str1)  ); 
  ytext += spectro->hchar;
  XDrawString(info->display,info->pixmap,info->gc,
	      (int)grdxr + 2 * spectro->wchar ,ytext,str2,strlen(str2) );
  ytext += spectro->hchar;
  XDrawString(info->display,info->pixmap,info->gc,
	      (int)grdxr + 2 * spectro->wchar ,ytext,str3,strlen(str3) );
  ytext += spectro->hchar;
  XDrawString(info->display,info->pixmap,info->gc,
	      (int)grdxr + 2 * spectro->wchar ,ytext,str4,strlen(str4) );
  if(strlen(str5) >= 1){
    ytext += spectro->hchar;
    XDrawString(info->display,info->pixmap,info->gc,
		(int)grdxr + 2 * spectro->wchar ,ytext,str5,strlen(str5) );
  }
}  /* end sc_toplabel */




/*************************************************************************/
/*		             M G T O D B				 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Convert long integer to db */
int mgtodb(long nsum,int usergainoffset)  {
static  int dbtable[] = {
         0,  1,  2,  3,  5,  6,  7,  9, 10, 11,
        12, 13, 14, 16, 17, 18, 19, 20, 21, 22,
        23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 37, 38, 39, 40, 41,
        42, 43, 43, 44, 45, 46, 47, 47, 48, 49,
        50, 50, 51, 52, 53, 53, 54, 55, 56, 56,
        57, 58, 58, 59, 60};

	int db,temp,dbinc;

/*    Convert to db-times-10 */
/*    If sum is 03777777777L, db=660 */
/*    If sum is halved, db is reduced by 60 */
        db = 1440;
        if (nsum <= 0L) {
	  if (nsum < 0L) {
	    printf("Error in mgtodb of QUICKSPEC,");
	    printf("nsum=%ld is negative, set to 0\n", nsum);
	  }
	  return(0);
	}
	if (nsum > 03777777777L) {
	    printf ("Error in mgtodb of QUICKSPEC, nsum=%ld too big, truncate\n",
	     nsum);
	    nsum = 03777777777L;
        }
        while (nsum < 03777777777L) {
            nsum = nsum + nsum;
            db = db - 60;
        }
        temp = (nsum >> 23) - 077;
	if (temp > 64) {
	    temp = 64;
	    printf ("Error in mgtodb of QUICKSPEC, temp>64, truncate\n");
	}
        dbinc = dbtable[temp];
        db = db + dbinc;
	db = db >> 1;		    /* Routine expects mag, we have mag sq */
	db = db + usergainoffset - 400;/*User offset to spect gain*/
	if (db < 0) db = 0;
	return (db);
}/* end mgtodb */



/***********************************************************************/
/*void dft(short iwave[],float win[],float *dftmag,int npts1,int npts2,*/
/*                        int fdifcoef){                               */
/***********************************************************************/
 
/*      DFT.C         D.H. Klatt */

/*      Compute npts2-point dft magnitude spectrum (after M.R. Portnoff) */

/*        Input: npts1 fixed-point waveform samples in "iwave[1,...,npts1]"
                  and a floating-point window in "win[1,...,npts1]"

		  If npts1 > npts2, fold waveform re npts2
CHANGE 2-Jan-87:  For first difference preemphasis, set fdifsw = 1  END OUT
		  For first difference preemphasis, set fdifcoef = {0,...,100}
		   where 0 is no preemphasis, and 100 is exact first difference.
		  For no windowing and no preemphasis, fdifcoef = -1 */

/*        Output: npts2/2 floating-point spectral magnitude-squared
                  samples returned in floating array dftmag[0,...,npts2/2-1]
		  (npts2 must be a power of 2, and npts1 must be even) */


void dft(short iwave[],float win[],float *dftmag,int npts1,int npts2,
         int fdifcoef){

        float xrsum;  /* this used to be a global*/
        int npts,mn,nskip,nsize,nbut,nhalf;
        float theta,c,s,wr,wi,tempr,dsumr,dsumi,scale,xre,xro;
        float xie,xio;
        float xri,xii,xfdifcoef;
        int i1,i,j,k,n,nrem,itest,isum,nstage,tstage;
        float *xr, *xi;
        float xtemp;

/*    Multiply waveform by window and place alternate samples
      in xr and xi */

        npts = npts1 >> 1;
	npts2 = npts2 >> 1;
        xr = dftmag;
        xi = dftmag + npts2;


	i1 = 0;
	xfdifcoef = 0.01 * (float)fdifcoef;
        for (i=0; i<npts2; i++) {
	    xr[i] = 0.;
	    xi[i] = 0.;
	}
        for (i=0; i<npts; i++) {
            j = i + i;
            k = j + 1;

            if (fdifcoef >= 0) {
/*	      Use first difference preemphasis and window function */
          xri = (float) (iwave[j] - (xfdifcoef * (float) iwave[j-1]));
          xii = (float) (iwave[k] - (xfdifcoef * (float) iwave[j]));
	        xr[i1] += xri * win[j];
	        xi[i1] += xii * win[k];
	}
	    else if (fdifcoef == -1) {
/*	      No first-difference preemphasis, window contains lpc coefs */
		xr[i1] += win[j];
		xi[i1] += win[k];
	      }
            else {
/*	      No first difference preemphasis, use window function */
                xri = iwave[j];
                xii = iwave[k];
	        xr[i1] += xri * win[j];
	        xi[i1] += xii * win[k];
	      }

	    i1++;
	    if (i1 >= npts2)    i1 = 0;
	  }
 
/*    Bit-reverse input sequence */
        i = 1;
        n = npts2 >> 1;
        while (i < (npts2 - 1)) {
                j = n;
                if (i < j) {
                    xtemp = xr[j];
                    xr[j] = xr[i];
                    xr[i] = xtemp;
                    xtemp = xi[j];
                    xi[j] = xi[i];
                    xi[i] = xtemp;
                }
                nrem = n;
                itest = npts2;
                isum = -npts2;
                while (nrem >= 0) {
                    isum = isum + itest;
                    itest = itest >> 1;
                    nrem = nrem - itest;
                }
                n = n + itest - isum;
                i = i + 1;
	      }
/*    End of bit-reversal algorithm */



/*    Begin FFT */
        nstage = 0;
        nskip = 1;
        while (nskip < npts2) {
  loop:     nstage++;
            tstage = nstage;/* scope problem with -O, d.h.*/
            nsize = nskip;
            nskip = nskip << 1;
/*        The following loop does butterflies which do not
          require complex arith */
            for (i = 0; i < npts2; i += nskip) {
                j = i + nsize;
                xtemp = xr[j];
                xr[j] = xr[i] - xr[j];
                xr[i] = xr[i] + xtemp;
                xtemp = xi[j];
                xi[j] = xi[i] - xi[j];
                xi[i] = xi[i] + xtemp;
	      }
            if(tstage < 2){
              goto loop;
	    }
            theta = TWOPI / nskip;
            c = cos(theta);
            s = sin(theta);
            wr = 1.0;
            wi = 0.0;

/*    Do remaining butterflies */
            for (nbut = 2; nbut <= nsize; nbut++){
                tempr = wr;
                wr = (wr * c) + (wi * s);
                wi = (wi * c) - (tempr * s);
                for(i = nbut - 1; i < npts2; i += nskip) {
                    j = i + nsize;
                    dsumr = (xr[j] * wr) - (xi[j] * wi);
                    dsumi = (xi[j] * wr) + (xr[j] * wi);
                    xr[j] = xr[i] - dsumr;
                    xi[j] = xi[i] - dsumi;
                    xr[i] += dsumr;
                    xi[i] += dsumi;
		  }
	      }
         }

/*    Set coefficients for scrambled real input */
        scale = 0.5;
        wr = 1.0;
        wi = 0.0;
        nhalf = npts2 >> 1;
        tempr = xr[0];
        xr[0] = tempr + xi[0];
        xi[0] = tempr - xi[0];
        theta = TWOPI / (npts2+npts2);
        c = cos(theta);
        s = sin(theta);
        for (n=1; n<nhalf; n++) {
            tempr = wr;
            wr = (wr*c) + (wi*s);
            wi = (wi*c) - (tempr*s);
            mn = npts2 - n;
            xre = (xr[n] + xr[mn]) * scale;
            xro = (xr[n] - xr[mn]) * scale;
            xie = (xi[n] + xi[mn]) * scale;
            xio = (xi[n] - xi[mn]) * scale;
            dsumr = (wr*xie) + (wi*xro);
            dsumi = (wi*xie) - (wr*xro);
            xr[n] = xre + dsumr;
            xi[n] = xio + dsumi;
            xr[mn] = xre - dsumr;
            xi[mn] = dsumi - xio;
	  }

/*    Take magnitude squared */
        xi[0] = 0.;
        xrsum = 0.;
        for (i=0; i<npts2; i++) {
            xr[i] = 0.000000001 * ((xr[i] * xr[i]) + (xi[i] * xi[i]));
            xrsum = xrsum + xr[i];
	  }

      }/* end */


/*******************end Dennis' dft.c*************************/



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		              M K W N D 				 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*    Make window */


mkwnd(XSPECTRO *spectro)

{

	float xi, incr;
	int i,limit;

/* this shouldn't be needed */

	if ((spectro->sizwin > spectro->sizfft) || 
            (spectro->sizwin <= 0) || (spectro->sizfft > 512)) {
	    printf("In mkwnd() of quickspec.c, illegal wsize=%d,dftsize=%d",
	    spectro->sizwin , spectro->sizfft);
	    exit(1);
	}

/*  printf(
       "\tMake Hamming window; width at base = %d points, pad with %d zeros\n",
	  wsize, power2-wsize); */
	xi = 0.;
	incr = TWOPI/(spectro->sizwin);
	limit = spectro->sizwin;
	/*if (spectro->halfwin == 1)    limit = limit >> 1;*/


/*    Compute non-raised cosine */
	for(i=0;i<limit;i++) {
	    if (spectro->params[NW] == 1) {
		spectro->hamm[i] = (1. -  cos(xi));
		xi += incr;
	      }
	    else {
		spectro->hamm[i] = 1.;	/* rectangular window */
	    }
	}

/*    Pad with zeros if window duration less than power2 points */
	while (i < spectro->sizfft) {
	    spectro->hamm[i++] = 0.;
	}
      }/* end mkwnd */





/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		         Q U I C K S P E C				 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* edited to use spectro structure July '93 d. hall*/

int quickspec(XSPECTRO *spectro, short iwave[])
{

  float scratch[1024];
  double pcerror, energy, floatw;
  static float floatwlast, xfdcoef;
  int ier,iwlast,count;
  int pdft,pdftend,nch,pw;
  long nsum;
  float sum,sumlow;


  if (spectro->type_spec == DFT_MAG ||
      spectro->type_spec == DFT_MAG_NO_DB) {
    
    spectro->lenfltr = spectro->sizfft / 2;
    /*makefbank(spectro);*/
  }/*DFT_MAG or DFT_MAG_NO_DB*/
  

  /*    Make filters */
  else if (spectro->type_spec == CRIT_BAND) {
    spectro->lenfltr = spectro->sizfft / 2;
    makefbank(spectro);
    spectro->lenfltr = spectro->nchan;
  }/*CRIT_BAND*/

  else if ((spectro->type_spec == SPECTROGRAM) ||
	   (spectro->type_spec == SPECTO_NO_DFT)) {
    spectro->lenfltr = spectro->sizfft / 2;
    makefbank(spectro);
  }/*SPECTROGRAM or SPECTO_NO_DFT*/

  /*    Make Hamming window  */
  /*does a new window need to be calculated*/
  if( spectro->oldsizwin != spectro->sizwin ||
      spectro->oldwintype!= spectro->params[NW]){
    
    spectro->oldsizwin = spectro->sizwin;
    spectro->oldwintype = spectro->params[NW];
    
    /* need to think about different sized ffts */
    
    mkwnd(spectro);
  }


  /*    Linear Prediction */
  if (spectro->type_spec == LPC_SPECT) {
    spectro->lenfltr = (spectro->sizfft >> 1);
    
    for(nch = 0; nch < 1024; nch++)
      scratch[nch] = 0.0;
    
    spectro->nchan = spectro->lenfltr;

    makefbank(spectro);	/* Actually just set nfr[] */
           

    /*	  Multiply waveform times window, do first difference, use dftmag[] */
    iwlast = 0;
    xfdcoef = 0.01 * (float)(spectro->params[FD]);
    for (nch = 1; nch < spectro->sizfft; nch++) {
      if (spectro->params[FD] > 0) {
	floatw = (float)iwave[nch];
	floatwlast = iwave[nch - 1];
	/*  Use 0.95 to make unbiased estimate of F1  for synthetic stim */
	spectro->dftmag[nch] = spectro->hamm[nch] *
          (floatw - (xfdcoef * floatwlast));
      }
      else {
	spectro->dftmag[nch] = spectro->hamm[nch] * 
	  (float)(iwave[nch]);
      }
    }
    
    
    /* Call Fortran lpc analysis subroutine, coefs in dftmag[256]... */
    pcode(spectro->dftmag,spectro->sizwin,spectro->params[NC],
	  &spectro->dftmag[spectro->sizfft],
	  scratch,&pcerror,&energy,&ier);
    
    
    /*	  Move coefs down 1, make negative */
    for (nch= spectro->params[NC] - 1; nch >= 0; nch--) {
      spectro->dftmag[nch+ spectro->sizfft + 1] = - 
	(spectro->dftmag[nch+spectro->sizfft]);
    }
        
    
    spectro->dftmag[spectro->sizfft] = 1.0;
    if (ier != 0) {
      printf("Loss of significance, ier=%d\n", ier);
    }
    
    /*	  Pad acoef with zeros for dft */
    count = (spectro->sizfft << 1);
    for(nch=spectro->params[NC]+spectro->sizfft+1; nch <= count; nch++) {
      spectro->dftmag[nch] = 0.;
    }
    /*	  Call dft routine with firstdifsw=-1 to indicate lpc */
    dft(iwave,&spectro->dftmag[spectro->sizfft],spectro->dftmag,
	spectro->sizwin,spectro->sizfft,-1);
    
    
  }/* Linear Prediction*/
  

  /*    Compute magnitude spectrum */
  else if (spectro->type_spec != SPECTO_NO_DFT) {
 
    dft(iwave,spectro->hamm,spectro->dftmag,
	spectro->sizwin,spectro->sizfft,spectro->params[FD]);
  }

/*  DFT now done in all cases, next create filters */

  /*    If DFT magnitude spectrum, just convert to dB */
  if (spectro->type_spec == DFT_MAG) {
    /*	  Consider each dft sample as an output filter */
    for (nch=0; nch<spectro->lenfltr; nch++) {
      /* Scaled so smaller than cb */
      nsum = spectro->dftmag[nch] * 20000.;
      spectro->fltr[nch] = mgtodb(nsum,spectro->params[SG]);
    }
  }/*DFT_MAG*/

  /*    If LPC: invert, multiply by energy, and convert to dB */
  else if (spectro->type_spec == LPC_SPECT) {
    /*	  Multiply coefs by square root of energy */
    spectro->denergy = (double)energy * (double)pcerror;
	    spectro->logoffset =(int)
	      (-1450.0 + 100. * log10(spectro->denergy) + .5);
	    /*	  Consider each dft sample as an output filter */
	    for (nch = 0; nch<spectro->lenfltr; nch++) {
	      spectro->denergy = spectro->dftmag[nch];
	      spectro->fltr[nch] = (int)(spectro->logoffset - 100. * 
					 log10(spectro->denergy));
	      if (spectro->fltr[nch] < 0)    spectro->fltr[nch] = 0;
	    }
	    return(0);
  }/*LPC_SPEC*/

  /* Sum dft samples to create Critical-band or Spectrogram filter outputs */

	else if ((spectro->type_spec == CRIT_BAND)
	 || (spectro->type_spec == SPECTO_NO_DFT)
	 || (spectro->type_spec == SPECTROGRAM)) {
            pw = 0;
	    for (nch=0; nch<spectro->lenfltr; nch++) {
                pdft = spectro->nstart[nch];
                pdftend = pdft + spectro->ntot[nch];
                sum = SUMMIN;
/*            Compute weighted sum of relevent dft samples */
                while (pdft < pdftend) {
                    sum += cbskrt[spectro->pweight[pw++]] * 
                    spectro->dftmag[pdft++];
		  }
/*	      Add in weak contribution of high-freq channels */
/*	      Approximate tail of filter skirt by exponential */

	   spectro->attenpersamp = (0.4 * spectro->spers) /
                            (spectro->nbw[nch] * spectro->nsdftmag);
	    spectro->attenpersamp = 1.0 - spectro->attenpersamp;

                if(spectro->attenpersamp > 1.0){spectro->attenpersamp = 1.0;}
                else if(spectro->attenpersamp < 0.0)
                    {spectro->attenpersamp = 0.0;}
		if (pdftend < spectro->lenfltr) {
		    sumlow = 0.;
		    pdft = spectro->lenfltr - 1;
		    while (pdft >= pdftend) {
                    sumlow = (sumlow + spectro->dftmag[pdft--]) 
                    * spectro->attenpersamp;
                    
                    }
		    sum += (sumlow * cbskrt[spectro->sizcbskirt-1]);
		}
/*	      Add in weak contribution of low-freq channels   */
/*	      Approximate tail of filter skirt by exponential */
                pdftend = spectro->nstart[nch];
		if (pdftend > 0) {
		    sumlow = 0.;
		    pdft = 0;
		    while (pdft < pdftend) {
                    sumlow = 
                    (sumlow + spectro->dftmag[pdft++]) * spectro->attenpersamp;
                    }
		    sum += (sumlow * cbskrt[(spectro->sizcbskirt-1)>>1]);
		}
		if (spectro->type_spec == SPECTO_NO_DFT) {
   	            sum = sum * 18000.;	/* Scaled so bigger than dft */
		    nsum = sum;
		    spectro->fltr[nch] = mgtodb(nsum,spectro->params[SG]);
		}
		else {
   	            sum = sum * 18000.;	/* Scaled so bigger than dft */
		    nsum = sum;
		    spectro->fltr[nch] = mgtodb(nsum,spectro->params[SG]);
/*		  Look for maximum filter output in this frame */
	        if (spectro->fltr[nch] > spectro->fltr[spectro->lenfltr + 1]) {
		 spectro->fltr[spectro->lenfltr + 1] = spectro->fltr[nch];
		      }
		  }
	      }
	  }

/*    Compute overall intensity and place in fltr[lenfltr] */

        sum = SUMMIN;
        for (pdft=0; pdft < spectro->sizfft>>1; pdft++) {
            sum = sum + spectro->dftmag[pdft];
	  }
        nsum = sum * 10000.;	/* Scale factor so synth av=60 about right */
	
        spectro->fltr[spectro->lenfltr] = mgtodb(nsum,spectro->params[SG]);

      }/* end quickspec */



/****************************************************************/
/*                 copysav(XSPECTRO *spectro)                   */
/****************************************************************/
void copysav(XSPECTRO *spectro){
int i;
      spectro->lensavfltr = spectro->lenfltr;
      spectro->savspectrum = spectro->spectrum;
      spectro->savsizwin = spectro->sizwin;
      spectro->savspers = spectro->spers;
      spectro->savfd = spectro->fd;
      strcpy(spectro->savname , spectro->wavename);
      for( i = 0; i <= spectro->lensavfltr; i++){
         spectro->savfltr[i] = spectro->fltr[i];
         if(spectro->option == 'c')
	   spectro->savnfr[i] = spectro->nfr[i];
      }


}/* end copysav*/
/*****************************************************************/
/*         dofltr(XSPECTRO *spectro,int type, int winsize)         */
/*****************************************************************/
void dofltr(XSPECTRO *spectro,int type, int winsize) {
/* set up window size and check against dft size */
/* call quickspec and update fltr if cursor is not too close to end*/
int firstsamp;

    spectro->sizwin = winsize;
    spectro->sizfft = 256;
    if(spectro->sizwin > spectro->sizfft)
        spectro->sizfft = 512;
    spectro->type_spec = type;
    spectro->nsdftmag = spectro->sizfft / 2;
    firstsamp = spectro->saveindex - spectro->sizwin / 2;
    if(spectro->sizwin <= spectro->sizfft){
     if(firstsamp >= 0 && firstsamp <= spectro->totsamp - 1){
        spectro->spectrum = 1;
        quickspec(spectro,&spectro->iwave[firstsamp]);
      }/* new spectrum */
     else {
        spectro->spectrum = 0; 
      }/* index too close to end of file for dft */
     }/* window size ok for dft */
    else{
      spectro->spectrum = 0;
      printf("Window greater that DFT reset parameters.\n");
      }/*spectro->sizwin > spectro->sizfft*/

   }/* end dofltr */




/**************************************************************/
/*                savdft(XSPECTRO *spectro, int winsize)      */
/**************************************************************/
void savdft(XSPECTRO *spectro,int winsize ){
int i,firstsamp;

/* do dft type spectrum and put it in savfltr (S and j and l)*/
    spectro->sizwin = winsize;
    spectro->sizfft = 256;
    if(spectro->sizwin > spectro->sizfft)
        spectro->sizfft = 512;    
    spectro->type_spec = DFT_MAG;
    spectro->nsdftmag = spectro->sizfft / 2;
    firstsamp = spectro->saveindex - spectro->sizwin / 2;
    if(spectro->sizwin <= spectro->sizfft){
     if(firstsamp >= 0 && firstsamp <= spectro->totsamp - 1){
        quickspec(spectro,&spectro->iwave[firstsamp]);
        spectro->lensavfltr = spectro->lenfltr;
        spectro->savspectrum = 1;
        spectro->savsizwin = spectro->sizwin;
        spectro->savspers = spectro->spers;
        strcpy(spectro->savname , spectro->wavename);
        /* Rms is in [spectro->lensavfltr] */
        for( i = 0; i <= spectro->lensavfltr; i++)
              spectro->savfltr[i] = spectro->fltr[i]; 
     }/* new spectrum */
     else {
        spectro->savspectrum = 0;
      }/* index too close to end of file for dft */
     }/* window size ok for dft */
    else{
      spectro->savspectrum = 0;
      printf("Window greater that DFT reset parameters.\n");
      }/*spectro->sizwin > spectro->sizfft*/

}/* end savdft*/



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                       T I L T		                         */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Replace set of points infltr[] by straight-line approx in outfltr[] */

tilt(infltr, npts, outfltr)
	int *infltr, *outfltr;
	int npts; {
		
	int i,imin,imax,infltrmax;
	float xsum, ysum, xysum, x2sum;
	float slope;
	float yintc;

        short sloptruncsw;   /* should move this to spectro*/
       sloptruncsw = 0;

/* Adjust limits to include only peaks in range 1 kH to 5 kHz */
	imin = 0;
	imax = npts;
/*    Find first local max after 1000 Hz */
	infltrmax = infltr[0];
	for (i=0; i<imax; i++) {
		if (infltr[i] < infltr[i+1])    infltrmax = infltr[i+1]-60;
		else    break;
	}
/*    Ignore samples less than max-6 db */
/* OUT	for (i=0; i<imax; i++) {
		if (infltr[i] < infltrmax)    imin++;
		else    break;
	}
   END OUT */

/*    Find last local max before 5000 Hz, set infltrmax = peak value */
	infltrmax = infltr[imax-1];
	for (i=imax-1; i>imin; i--) {
		if (infltr[i] < infltr[i-1])    infltrmax = infltr[i-1];
		else    break;
	}
	infltrmax -= 60;
/*    Ignore samples less than max-6 db */
	if (sloptruncsw == 1) {
	    for (i=imax-1; i>imin; i--) {
		if (infltrmax > infltr[i])    imax--;
		else    break;
	    }
	}

	xsum = 0.0;
	ysum = 0.0;
	xysum = 0.0;
	x2sum = 0.0;

	for (i=imin; i<imax; i++){
		xsum = xsum + (i+1);
		ysum = ysum + infltr[i];
		xysum = xysum + (i+1) * infltr[i];
		x2sum = x2sum + (i+1) * (i+1);
	}

	slope = npts*xysum - xsum*ysum;
	slope = slope /( npts*x2sum -xsum*xsum);
	yintc = (ysum - slope*xsum) / npts;

	for (i=imin; i<imax; i++){
		outfltr[i] = slope*(i+1) + yintc;
	}

	i = 100 * slope;
	return(i);	/* Slope as integer value */
}/* end tilt */



/********************************************************************/
/*              void getf0 (int, int, int)                          */
/********************************************************************/

  
getf0(dftmag,npts,samrat) 
int dftmag[],npts,samrat; {

	int direction,npeaks,lasfltr,n;
	int dfpeaks[MAXNPEAKS],apeaks[MAXNPEAKS];
	int alastpeak,flastpeak,np,np1,mp,kp,dfp;
	int dfsum,dfmin,dfmax,dfn,nmax,nmaxa,ampmax;
	int da,da1,da2,df,npaccept;
	int dfsummax,dfnmax,dfnloc;

	direction = DOWN;
	npeaks = 0;
	lasfltr = dftmag[0];
/*    Look for local maxima only up to 1.5 kHz (was 3 kHz) */
	if (samrat > 0) {
	    nmax = (npts * 1500) / samrat;
/*	  Also find biggest amplitude in range from 0 to 900 Hz */
	    nmaxa = (npts * 900) / samrat;
	}
	else printf("In getf0(): somebody walked over samrat, set to zero");
	ampmax = 0;

/*    Begin search for set of local max freqs and ampls */
	for (n=1; n<nmax; n++) {
	    if (n < nmaxa) {
		if (dftmag[n] > ampmax)    ampmax = dftmag[n];
	    }
	    if (dftmag[n] > lasfltr) {
		direction = UP;
	    }
	    else if (dftmag[n] < lasfltr) {
		if ((direction == UP) && (npeaks < 49)) {
/*		  Interpolate the frequency of a peak by looking at
		  rel amp of previous and next filter outputs */
			da1 = dftmag[n-1] - dftmag[n-2];
			da2 = dftmag[n-1] - dftmag[n];
			if (da1 > da2) {
			    da = da1;
			}
			else {
			    da = da2;
			}
			if (da == 0) {
			    da = 1;
			}
			df = da1 - da2;
			df = (df * (samrat>>1)) / da;
			fpeaks[npeaks] = (((n-1) * samrat) + df) / npts;
			apeaks[npeaks] = lasfltr;
			if (npeaks < MAXNPEAKS-1)    npeaks++;
		}
		direction = DOWN;
	    }
	    lasfltr = dftmag[n];
	}

/*    Remove weaker of two peaks within minf0 Hz of each other */
#if 0
printf("\nInitial candidate set of %d peaks:\n", npeaks);
for (np=0; np<npeaks; np++) {
    printf("\t %d.   f = %4d   a = %3d\n", np+1, fpeaks[np], apeaks[np]);
}
#endif

	for (np=0; np<npeaks-1; np++) {

	    if (((np == 0) && (fpeaks[0] < 80))
	      || ((np > 0) && ((fpeaks[np] - fpeaks[np-1]) <= 50))) {
		if ((np > 0) && (apeaks[np] > apeaks[np-1])) np--;
		npeaks--;
		for (kp=np; kp<npeaks; kp++) {
		    fpeaks[kp] = fpeaks[kp+1];
		    apeaks[kp] = apeaks[kp+1];
		}
	    }
	}

/*    Remove initial weak peak before a strong one: */
	if ((apeaks[1] - apeaks[0]) > 200) {
	    npeaks--;
	    for (kp=0; kp<npeaks; kp++) {
		fpeaks[kp] = fpeaks[kp+1];
		apeaks[kp] = apeaks[kp+1];
	    }
	}

/*    Remove weak peak between two strong ones I: */
	for (np=1; np<npeaks-1; np++) {
/*	  See if a weak peak that should be ignorred */
	    if (((apeaks[np-1] - apeaks[np]) > 40)
	     && ((apeaks[np+1] - apeaks[np]) > 40)) {
		npeaks--;
		for (kp=np; kp<npeaks; kp++) {
		    fpeaks[kp] = fpeaks[kp+1];
		    apeaks[kp] = apeaks[kp+1];
		}
	    }
	}

/*    Remove weak peak between two strong ones II: */
	for (np=1; np<npeaks-1; np++) {
/*	  See if a weak peak that should be ignorred */
	    if ((apeaks[np-1] - apeaks[np]) > 80) {
		np1 = 1;
		while (((np+np1) < npeaks)
		  && (fpeaks[np+np1] < (fpeaks[np] + 400))) {
		    if ((apeaks[np+np1] - apeaks[np]) > 80) {
			npeaks--;
			for (kp=np; kp<npeaks; kp++) {
			    fpeaks[kp] = fpeaks[kp+1];
			    apeaks[kp] = apeaks[kp+1];
			}
			np--;
			goto brake1;
		    }
		    np1++;
		}
	    }
brake1:
	    np = np;
	}
#if 0
printf("\nFinal candidate set of %d peaks sent to comb filter:\n", npeaks);
for (np=0; np<npeaks; np++) {
    printf("\t %d.   f = %4d   a = %3d\n", np+1, fpeaks[np], apeaks[np]);
}
#endif

	if (npeaks > 1)        dfp = comb_filter(npeaks);
	else if (npeaks > 0)   dfp = fpeaks[0];
	else                   dfp = 0;
#if 0
printf("\nComb filter says f0 = %d\n", dfp);
printf("\nHit <CR> to continue:");
scanf("%c", &char1);
#endif

/*    Zero this estimate of f0 if little low-freq energy in spect */
	if (ampmax < 200) {
	    dfp = 0;
	}
/*    Or if f0 high and only a few peaks */
/* couldn't find any other reference to npaccept so*/
/* based on the above comment set it to npeaks d.h.*/
  npaccept = npeaks;
	if ( (npaccept <= 2) && (dfp > 300) )    dfp = 0;
	return(dfp);
}



/*****************************************************************/
/*                 int comb_filter (int)                         */
/*****************************************************************/

int comb_filter(npks) int npks; {

/* Look for an f0 with most harmonics present, restrict */
/* search to range from f0=60 to f0=400 */

#define NBINS	65
    float f0_hypothesis,fmin_freq,fmax_freq;
    static float f0_hyp[NBINS];
    int nharm,bin,min_freq,max_freq,cntr_freq,max_cntr_freq;
    int max_bin,max_count_harm,harm_found;
    int sum_f,sum_nharm,f0_estimate;
    int np,freq_harm,count_harm;

/* Initialization */
    if (f0_hyp[0] == 0) {
	f0_hyp[0] = 400.;
	for (bin=1; bin<NBINS; bin++) {
/*	  Compute center freq of each bin */
	    f0_hyp[bin] = .97 * f0_hyp[bin-1];
	}
    }

    if (npks > 6)    npks = 6;
    max_count_harm = 0;
    for (bin=0; bin<NBINS; bin++) {
	fmin_freq = 0.97 * f0_hyp[bin];
	cntr_freq = f0_hyp[bin];		/* for debugging only */
	fmax_freq = 1.03 * f0_hyp[bin];
	count_harm = 0;
	sum_nharm = 0;
	sum_f = 0;
	for (nharm=1; nharm<=6; nharm++) {
	    min_freq = (int) (fmin_freq * (float) nharm);
	    max_freq = (int) (fmax_freq * (float) nharm);
	    if (min_freq < 2500) {
		harm_found = 0;
		for (np=0; np<npks; np++) {
		    freq_harm = fpeaks[np];
		    if ((freq_harm > min_freq) && (freq_harm < max_freq)) {
			if (harm_found == 0) {
			    harm_found++;
			    count_harm++;
			    sum_nharm += nharm;
			    sum_f += freq_harm;
			}
		    }
		}
	    }
	}
/*	printf("%d ", count_harm); */
	if (count_harm > max_count_harm) {
	    max_count_harm = count_harm;
	    max_bin = bin;
	    max_cntr_freq = cntr_freq;
	    f0_estimate = sum_f / sum_nharm;
	}
    }
/*  printf("\n	Best comb freq = %d, f0 = %d\n",
     max_cntr_freq, f0_estimate); */

/*    Zero this estimate of f0 if the distribution of f0 estimates */
/*    is rather scattered */
	if ((3*max_count_harm) < (2*npks)) {
/*	    printf("\nZero f0=%3d", f0_estimate); */
	    f0_estimate = 0;
	}
/*	else {
	    printf("\n     f0=%3d", f0_estimate);
	}
	printf("  nh=%d np=", max_count_harm);
	for (np=0; np<npks; np++) {   spectro->option = spectro->savoption;
   spectro->history = 0;
	    printf("%4d, ", fpeaks[np]);
	} */
	return(f0_estimate);
  }




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		            G E T F O R M . C		D. Klatt	 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */

/*		On VAX, located in directory [klatt.klspec]		*/


/* EDIT HISTORY:
 * 000001 31-Jan-84 DK	Initial creation
 * 000002 24-Mar-84 DK	Fix so hidden formant rejected if too close to peak
 * 000003 23-May-84 DK	Improved peak locating interpolation scheme
 * 000004 17-Apr-86 DK	Improve accuracy of freq estimate for hidden peaks
 */

/*               most of these are no an SPECTRO structure            */
/*	int dfltr[257];			Spectral slope */
/*
/*        #define MAX_INDEX	  7
/*	int nforfreq;			number of formant freqs found   */
/*	int forfreq[16],foramp[16];	nchan and ampl of max in fltr[] */
/*	int valleyfreq[8],valleyamp[8];	nchan and ampl of min in fltr[] */

/*	int nmax_in_d;			number of maxima in deriv       */
/*	int ncdmax[8],ampdmax[8];	nchan and ampl of max in deriv  */
/*	int ncdmin[8],ampdmin[8];	nchan and ampl of min in deriv  */

/*	int hftot;			number of hidden formants found */
/*	int hiddenf[8],hiddena[8];	freq and ampl of hidden formant */


getform(XSPECTRO *spectro) {

	int nch,lasfltr,lasderiv,direction,da,da1,da2,nhf;
	long nsum;
	int nchb,nchp,ff,df;
	int down = 1;
	int up = 2; 
        int index;


/*	  Pass 1: Compute spectral slope */
	    for (nch=2; nch<spectro->nchan-2; nch++) {
/*	      Approximate slope by function involving 5 consecutive channels */
		spectro->dfltr[nch] = 
                spectro->fltr[nch+2] + spectro->fltr[nch+1] -
                spectro->fltr[nch-1] - spectro->fltr[nch-2];
	    }
/*	  Pass 2: Find slope max, slope min; half-way between is a formant */
/*	   If max and min of same sign, this is a potential hidden formant */
	    direction = down;
	    spectro->nmax_in_d = 0;
	    lasderiv = spectro->dfltr[2];
	    spectro->hftot = 0;
	    for (nch=3; nch<spectro->nchan-2; nch++) {
	        if (spectro->dfltr[nch] > lasderiv) {
		    if ( (direction == down) && (spectro->nmax_in_d > 0) ) {
/*		      Found max and next min in deriv, see if hidden formant */
			if ((spectro->hiddenf[spectro->hftot] = testhidden(spectro)) > 0) {
                          index = (spectro->hiddenf[spectro->hftot]*spectro->sizfft)/spectro->spers;
		          spectro->hiddena[spectro->hftot] = spectro->fltr[index];
			    if (spectro->hftot < MAX_INDEX)   spectro->hftot++;
			}
		    }
		    direction = up;
	        }
	        else if (spectro->dfltr[nch] < lasderiv) {
		    if ((spectro->nmax_in_d > 0) && (direction == down)) {
/*		      On way down to valley */
			spectro->ampdmin[spectro->nmax_in_d-1] = spectro->dfltr[nch];
			spectro->ncdmin[spectro->nmax_in_d-1] = nch;
		    }
		    if ((direction == up) && (spectro->nmax_in_d < 7)) {
			spectro->ampdmin[spectro->nmax_in_d] = spectro->dfltr[nch];
			spectro->ncdmin[spectro->nmax_in_d] = nch;
/*		      Peak in deriv found, compute chan # and value of deriv */
			spectro->ampdmax[spectro->nmax_in_d] = lasderiv;
			spectro->ncdmax[spectro->nmax_in_d] = nch-1;
			if (spectro->nmax_in_d < MAX_INDEX)   spectro->nmax_in_d++;
		    }
		    direction = down;
	        }
	        lasderiv = spectro->dfltr[nch];
	    }
		
/*	  Pass 3: Find actual spectral peaks, fold hidden peaks into array */
	    direction = down;
	    spectro->nforfreq = 0;
	    nhf = 0;
	    lasfltr = spectro->fltr[0];
	    for (nch=1; nch<spectro->nchan; nch++) {
	        if (spectro->fltr[nch] > lasfltr) {
		    direction = up;
	        }
	        else if (spectro->fltr[nch] < lasfltr) {
		    if ((spectro->nforfreq > 0) && (direction == down)) {
/*		      On way down to valley */
			spectro->valleyamp[spectro->nforfreq-1] = spectro->fltr[nch];
		    }
		    if ((direction == up) && (spectro->nforfreq < 6)) {
			spectro->valleyamp[spectro->nforfreq] = spectro->fltr[nch];

/*		    Peak found, compute frequency */
/*		      Step 1: Work back to filter defining beginning of peak */
			for (nchb=nch-2; nchb>0; nchb--) {
			    if (spectro->fltr[nchb] < spectro->fltr[nch-1])    break;
			}
/*		      Step 2: Compute nearest filter of peak */
			nchp = (nchb + nch) >> 1;
			ff = spectro->nfr[nchp];
/*		      Step 3: Add half a filter if plateau has even # filters */
			if (nchp < ((nchb + nch + 1) >> 1)) {
			    ff = (spectro->nfr[nchp] + spectro->nfr[nchp+1]) >> 1;
			}
/*		      Step 4: Compute interpolation factor */
		        da1 = spectro->fltr[nchp] - spectro->fltr[nchb];
		        da2 = spectro->fltr[nchp] - spectro->fltr[nch];
		        da = da1 + da1;
			if (da2 > da1)    da = da2 + da2;
			nsum = ((da1-da2) * (spectro->nfr[nch-1] - spectro->nfr[nch-2]));
			df = nsum / da;
			ff += df;


			while ((nhf < spectro->hftot) && (spectro->hiddenf[nhf] < ff)
			&& (spectro->nforfreq < MAX_INDEX)) {
/*			  Hidden formant should be inserted here */
			    spectro->foramp[spectro->nforfreq] = spectro->hiddena[nhf];
			    spectro->forfreq[spectro->nforfreq] = - spectro->hiddenf[nhf];
                            spectro->nforfreq++; nhf++;
			}

			spectro->foramp[spectro->nforfreq] = lasfltr;
			spectro->forfreq[spectro->nforfreq] = ff;
			if (spectro->nforfreq < MAX_INDEX)    spectro->nforfreq++;
		    }
		    direction = down;
	        }
	        lasfltr = spectro->fltr[nch];
	    }
	    while ((nhf < spectro->hftot) && (spectro->nforfreq < MAX_INDEX)) {
/*	      Hidden formant should be inserted here */
		spectro->foramp[spectro->nforfreq] = spectro->hiddena[nhf];
		spectro->forfreq[spectro->nforfreq] = - spectro->hiddenf[nhf]; 
                spectro->nforfreq++;
                nhf++;                                          /* minus indic. hidd */
	    }
	    zap_insig_peak(spectro);
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		        T E S T H I D D E N				 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Found loc max in deriv followed by loc min in deriv, see if hidden formant */

testhidden(XSPECTRO *spectro) {

	int nchpeak;	/* channel number of potential hidden peak */
	int amphidpeak;	/* amplitude of hidden peak = (max+min)/2 in deriv */
	int delamp,delslope,nchvar,f,a,denom,foffset;

        int NCH_OF_PEAK_IN_DERIV, AMP_OF_PEAK_IN_DERIV;
        int NCH_OF_VALLEY_IN_DERIV , AMP_OF_VALLEY_IN_DERIV;

        NCH_OF_PEAK_IN_DERIV = spectro->ncdmax[spectro->nmax_in_d-1];
        AMP_OF_PEAK_IN_DERIV = spectro->ampdmax[spectro->nmax_in_d-1];
        NCH_OF_VALLEY_IN_DERIV = spectro->ncdmin[spectro->nmax_in_d-1];
        AMP_OF_VALLEY_IN_DERIV = spectro->ampdmin[spectro->nmax_in_d-1];




/*    A real peak is where derivative passes down through zero. */
/*    A hidden peak is where local max and succeeding local min of derivative */
/*    both are positive, or both are negative */
	if ((AMP_OF_PEAK_IN_DERIV <= 0) 
	 || (AMP_OF_VALLEY_IN_DERIV >= 0)) {

/*	   OUT printf("ap=%d av=%d fp=%d fv=%d\n",
	     AMP_OF_PEAK_IN_DERIV, AMP_OF_VALLEY_IN_DERIV, 
	     NCH_OF_PEAK_IN_DERIV, NCH_OF_VALLEY_IN_DERIV); END OUT */

/*	  See if diff in slope not simply noise */
	    delslope = AMP_OF_PEAK_IN_DERIV - AMP_OF_VALLEY_IN_DERIV;
	    if (delslope < 25) {
		return(-1);
	    }

/*	  Tentative channel hidden peak = mean of peak & valley locs in deriv */
	    nchpeak = (NCH_OF_PEAK_IN_DERIV + NCH_OF_VALLEY_IN_DERIV) >> 1;

/*	  Find amplitude of nearest formant peak (local spectral max) */
	    if (AMP_OF_PEAK_IN_DERIV < 0) {
/*	      Nearest peak is previous peak, find it */
		nchvar = nchpeak;
		while ( (spectro->fltr[nchvar-1] >= spectro->fltr[nchvar]) && (nchvar > 0) ) {
		    nchvar--;
		}
		delamp = spectro->fltr[nchvar] - spectro->fltr[nchpeak];
		foffset = -80;
	    }
	    else {
/*	      Nearest peak is next peak, find it */
		nchvar = nchpeak;
		while ( (spectro->fltr[nchvar+1] >= spectro->fltr[nchvar]) && (nchvar < 127) ) {
		    nchvar++;
		}
		delamp = spectro->fltr[nchvar] - spectro->fltr[nchpeak];
		foffset = 120;	/* hidden formant is actually between min
				   and next max */
	    }

/*	  See if amp of hidden formant not too weak relative to near peak */
	    if (delamp < 80) {
/*	      Exact location of hidden peak is halfway from max to min in deriv */
		amphidpeak = (AMP_OF_PEAK_IN_DERIV+AMP_OF_VALLEY_IN_DERIV)>>1;
		nchpeak = NCH_OF_PEAK_IN_DERIV;
		if (spectro->dfltr[nchpeak] > amphidpeak) {
		    while ( ((spectro->dfltr[nchpeak] - amphidpeak) >= 0)
		     && (nchpeak <= NCH_OF_VALLEY_IN_DERIV) ) {
			nchpeak++;
		    }
		    nchpeak--;
		}
		a = spectro->dfltr[nchpeak] - amphidpeak;
		denom = spectro->dfltr[nchpeak] - spectro->dfltr[nchpeak+1];
		if (denom > 0) {
		    f = spectro->nfr[nchpeak]
			 + ((a*(spectro->nfr[nchpeak+1]-spectro->nfr[nchpeak]))/denom);
		}
		else {
		    f = spectro->nfr[nchpeak];
		}

		return(f+foffset);
	    }
	}
	return(-1);
      }/* end testhidden */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*		        Z A P - I N S I G - P E A K			 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*	  Eliminate insignificant peaks (last one always significant) */
/*	  Must be down 6 dB from peaks on either side		      */
/*	  And valley on either side must be less than 2 dB	      */

/*	  Also eliminate weaker of two formant peaks within 210 Hz of */
/*	  each other, unless not enough formants in F1-F3 range	      */

zap_insig_peak(XSPECTRO *spectro) {

	int nf,n,xxx,fcur,fnext;

	for (nf=0; nf<spectro->nforfreq-1; nf++) {

	    if ((nf == 0) || (spectro->foramp[nf-1] > spectro->foramp[nf]+60)) {
		if (spectro->foramp[nf+1] > spectro->foramp[nf]+60) {
		    if ((nf == 0) || (spectro->valleyamp[nf-1] > spectro->foramp[nf]-20)) {
			if (spectro->valleyamp[nf] > spectro->foramp[nf]-20) {
			    for (n=nf; n<spectro->nforfreq; n++) {
				spectro->forfreq[n] = spectro->forfreq[n+1];
				spectro->foramp[n] = spectro->foramp[n+1];
			    }
			    spectro->nforfreq--;
			}
		    }
		}
	    }

/*	  Zap weaker of 2 close peaks (hidden peak is alway weaker) */
	    if (nf < spectro->nforfreq-1) {
		fcur = spectro->forfreq[nf];
		if (fcur < 0)    fcur = -fcur;

		fnext = spectro->forfreq[nf+1];
		if (fnext < 0)    fnext = -fnext;

		if ((fnext - fcur) < 210) {

		    if (spectro->foramp[nf] > spectro->foramp[nf+1])    xxx = 1;
		    else    xxx = 0;

		    for (n=nf+xxx; n<spectro->nforfreq-1; n++) {
			spectro->forfreq[n] = spectro->forfreq[n+1];
			spectro->foramp[n] = spectro->foramp[n+1];
		    }
		    spectro->nforfreq--;
		    nf--;
		}
	    }

	}
      }/* end zap_insig_peak */



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
 
  /*store .gram  file */
  sprintf(str,"%s.gram",spectro->wavename);


  if(!(fp = fopen(str,"w")) ){
    printf(mess);
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
}  /* end writegram */


void ChangeSpectrumOption (XSPECTRO *spectro, char new)
{
  spectro->option = new;
}


void SetCurrentTime (XSPECTRO *spectro, float newtime)
{
  spectro->oldindex = spectro->saveindex;
  spectro->oldtime = spectro->savetime;
  spectro->savetime = newtime;
  spectro->saveindex =  (spectro->savetime  * spectro->spers / 1000.0) + .5;
  validindex(spectro); 

}
