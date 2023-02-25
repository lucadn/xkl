#include "xspec_util.h"
#include "web.h"
#include <string.h>

  ALLSPECTROS allspectros;


/** see makefbank **/

float cbskrt[SIZCBSKIRT] = {
1.00,  .998,  .995,  .984,  .972,  .952,  .931,  .907,  .883,  .849,
.814,  .775,  .735,  .699,  .662,  .618,  .573,  .529,  .484,  .447,
.410,  .370,  .329,  .292,  .255,  .238,  .221,  .206,  .191,  .179,
.167,  .157,  .146,  .137,  .128,  .120,  .112,  .106,  .099,  .093,
.087,  .082,  .077,  .073,  .069,  .065,  .061,  .058,  .055,  .052,
.049,  .047,  .044,  .042,  .040,  .038,  .036,  .035,  .033,  .032,
.030,  .029,  .027,  .026,  .025,  .024,  .023,  .022,  .021,  .021,
.020,  .019,  .018,  .018,  .017,  .017,  .016,  .016,  .015,  .015,
.014,  .014,  .013,  .013,  .012,  .012,  .0115, .0113, .011,  .0105,
.010,  .0098, .0095, .0093, .009,  .0088, .0085, .0083, .008,  .0077,
};




void main (int argc, char **argv)
{
  char* wavename;
  char* gifname;
  String psfile, newfile;
  XSPECTRO *spectro;
  FILE *fp;

  wavename = argv[1];
  printf ("waveform = %s\n", wavename);

  gifname = argv[2];
  printf ("gifname = %s\n", gifname);


  /* Initialize xkl, allocate memory for one new spectro */
  SetupXklEnvironment ();
  AllocateNewSpectro(spectro, wavename); 

  /* Add the first spectro */
  add_spectro(spectro, "xkl_defs.dat", wavename);
  initime(spectro);
  allspectros.count++;

  /* Initializations */

  spectro->wchar = 6;
  spectro->hchar = 13; 

/*
  spectro->yb[GRAM] = 240;
  spectro->xr[GRAM] = 500;
*/

  /* Load Wave form into environment */
  LoadNewWaveform(spectro, wavename);  

  set_defaults(spectro);

  ChangeSpectrumOption (spectro, 'd');
  printf ("option = %c\n", spectro->option);

  SetCurrentTime (spectro, 200);

  psfile = "test.ps";
  /* Open a .ps file, and write selected graph */
  if (fp = fopen(psfile,"w"))
    {
/*      DrawSpectrum (spectro, fp);   */
      DrawSpectrogram (spectro, fp); 
    }
  else
    printf ("Couldn't open .ps file for writing!\n");

  fclose(fp);

  newfile = "test.jpg";
  /** Convert to jpg file: **/
  ConvertPStoJPEG(psfile, newfile);
}


void SetupXklEnvironment()
{
  int s;
  /* Initialization to make filter skirts go down further (to 0.00062)*/
  
  for (s = 100; s<SIZCBSKIRT; s++) 
    cbskrt[s] = 0.975 * cbskrt[s-1];
  /* init cbskrt */

  allspectros.count = 0;
  allspectros.list = (XSPECTRO **) malloc( sizeof( XSPECTRO *) * 1);

}


void AllocateNewSpectro (XSPECTRO *newspectro, char* wavename)
{
  allspectros.list[allspectros.count]=(XSPECTRO *) malloc( sizeof(XSPECTRO) );
  newspectro = allspectros.list[allspectros.count];

}


void ConvertPStoJPEG(String psfile, String newfile)
{
  /* requires: psfile exists
   * effects: creates a JPEG image file of postscript file <psfile> with
   *          the name <newfile>.
   */

  String add = " -dNOPAUSE - < ";
  String command = "gs -q -sDEVICE=jpeg -sOutputFile=";

  strcat(command, newfile);
  strcat(command, add);
  strcat(command, psfile);

/*  printf ("command = %s\n", command); */
  system(command); 

}

void DrawSpectrum (XSPECTRO *spectro, FILE *fp)
{
  /* requires: File fp exists and spectrum option is set
   * effects: draws spectrum with preset option to a .ps file
   */

  new_spectrum (spectro);  /* initialize the spectrum */

  printspectrum (spectro, fp);  /** prints spectrum to a ps file **/

}


void DrawSpectrogram (XSPECTRO *spectro, FILE *fp)
{
  spectro->spectrogram = 1;
  spectro->devwidth = 1024;
  spectro->devheight = 768;
  spectro->hchar = 13;
  spectro->wchar = 13;


/*  readgram(spectro); */


  findsdelta(spectro);

  findypix(spectro);

  printf ("wxmax = %d\n", spectro->wxmax);
  printf ("wymax = %d\n", spectro->wymax);
  printf ("ypix = %d\n", spectro->ypix);
  printf ("xpix = %d\n", spectro->xpix);

  calculate_spectra(spectro);

  writegram(spectro);  

  win_size(spectro); 

  postgvs (spectro, fp);
}


