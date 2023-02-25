/* $Log: resources.h,v $
 * Revision 1.2  1998/06/04 23:11:28  krishna
 * Added RCS header info.
 * */

/* 
 * Resources for small screen 
 */

static char * small_resources[] = { 
      "*foreground: black",
      "*background: gray",

      "xkl*fontList: -adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",

      /* large waveform */      

      "*window_0.x: 10",
      "*window_0.y: 30",
      "*window_0.width: 1000",
      "*window_0.height: 160",

      /* spectrum */

      "*window_3.x: 10",
      "*window_3.y: 230",
      "*window_3.width: 475",
      "*window_3.height: 300",

      /* small waveform */ 

      "*window_1.x: 10",
      "*window_1.y: 570",
      "*window_1.width: 475",
      "*window_1.height: 185",

      /* spectrogram */

      "*window_2.x: 500",
      "*window_2.y: 470",

      "*xkl_textwindow.x: 500",
      "*xkl_textwindow.y: 230",
      "*xkl_textwindow.width: 510",
      "*xkl_textwindow.height: 200",

      "*syn.x: 25",
      "*syn.y: 475",

      "*syngraph.x: 500",
      "*syngraph.y: 275",
    NULL,
    };

/*
 * Resources for big screen
 */

static String big_resources[] = {
      "xkl*fontList: -adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",

      "*foreground: black",
      "*background: gray",

      /* large waveform */

      "*window_0.x: 10",
      "*window_0.y: 20",
      "*window_0.width: 1000",
      "*window_0.height: 210",

      /* small waveform */ 

      "*window_1.x: 10",
      "*window_1.y: 645",
      "*window_1.width: 490",
      "*window_1.height: 205",

      /* spectrogram */

      "*window_2.x: 510", /* size determined by ms in xkl_defs.dat */
      "*window_2.y: 580",

      /* spectrum */

      "*window_3.x: 10",
      "*window_3.y: 260",
      "*window_3.width: 490",
      "*window_3.height: 355",


      "*xkl_textwindow.x: 510",
      "*xkl_textwindow.y: 260",
      "*xkl_textwindow.width: 500",
      "*xkl_textwindow.height: 290",

      "*syn.x: 25",
      "*syn.y: 475",

      "*syngraph.x: 500",
      "*syngraph.y: 275",

     NULL,
    };
