/*
  ===========================================================================
	XKL_DLOG  - Xkl spectum/spectrogram parameters dialog support
  ---------------------------------------------------------------------------

	Contains:
	  
	  (Local)
	  	HandleSpecParams 	- spectral parameters callback handler
	    MapSGValues			- map spectrogram parameters from spectro
	  	HandleSGParams 		- spectrogram parameters callback handler
	  
	  (Global)	
	  	HandleMapping	 	- mapping callback handler (center window)
		InitSpecParams	 	- initialize spectral parameters dialog
		InitSGParams		- initialize spectrogram parameters dialog
  ===========================================================================
*/

#include "xspec_util.h"	/* includes X headers (xinfo.h) */
/* ***  local static data  *** */

enum {					// spectral parameter dialog button actions
	spApplyCurrent,			// apply to current waveform
	spApplyAll,				// apply to all waveforms
	spDefaults,				// reset defaults
	spCancel				// cancel (no change)
};

static Widget gSpecParamsD = NULL;			// spectral parameters dialog
static Widget gSpecParamsV[numSpecParams];	// spectral parameter value widgets

static int gDFTvals[] = {16,32,64,128,256,512,1024,2048,4096};	// supported (spectra) DFT sizes

enum {					// spectrogram parameters
	sgpWindow = 0,			// window size (ms)
	sgpFrame,				// FFT frame length
	sgpAveragd,				// # FFTs averaged
	sgpOffset,				// ms offset of each FFT from previous
	sgpDisplay,				// ms length of displayed spectrogram
	sgpLow,					// magnitudes less than this mapped to white
	sgpHigh,				// magnitudes greater than this mapped to black
	sgpScale,				// displayed spectrogram scaling (1 or 2)
	sgpDiff,				// differencing
	sgpUnits,				// X axis units (0: ms, 1: sec)
	numSGParams
};

static Widget gSGParamsD = NULL;			// spectrogram parameters dialog
static Widget gSGParamsV[numSGParams];		// spectrogram parameter value widgets

#define numDFTvalsSG 6	// number of gDFTvals supported for spectrogram

typedef struct {		// spectrogram initialization structure
	float	v;				// current value
	int		type;			// float | int
	char	*blurb;			// description
} SGINIT;

static SGINIT gSGInit[numSGParams] = {
	{6.4,	FLOAT,	"window in ms (zero-padded to FFT frame length)"},
	{128,	INT,	"FFT frame length (samples)"},
	{3,		INT,	"number of FFTs averaged"},
	{1.0,	FLOAT,	"ms offset of each FFT from previous"},
	{1300.,	FLOAT,	"ms length of displayed spectrogram"},
	{5.0,	FLOAT,	"magnitudes less than this mapped to white"},
	{25.0,	FLOAT,	"magnitudes greater than this mapped to black"},
	{1,		INT,	"displayed spectrogram scaling"},
	{100,	INT,	"0 = no pre-emphasis, 100 = exact first difference"},
	{0,		INT,	"X axis units"}
};
static float gSGSaveVals[numSGParams];

// XmString creation macro
#define XpMakeXstring(asciiz) (Xstring[sc++]=XmStringCreateLocalized(asciiz))

/*
  ===========================================================================
	HandleSpecParams  - spectral parameters callback handler
  ---------------------------------------------------------------------------
  
	  Args:
		button		- clicked button widget
		buttonID	- button identifier

	  Called from:
		XmNactivateCallback (InitSpecParams)
		
	  Side effects:
		updates spectro parameters

	  Returns:
		nada
  ===========================================================================
*/

static void 
HandleSpecParams(
	Widget	button,
	int buttonID,
	XmAnyCallbackStruct *reason)
{
	Widget w;
	WidgetList wl;
	XSPECTRO *spectro;
	int	i, n, k, v;
	char *text, s[255];
	float newVals[numSpecParams],tempVarHack;		// new spectral parameter values
	extern paramStruct specParams[];

/* ***  no change on cancel  *** */

	if (buttonID == spCancel) {
		XtDestroyWidget(gSpecParamsD);
		gSpecParamsD = NULL;
		return;
	}
	
/* ***  reset default values  *** */

	if (buttonID == spDefaults) {
		for (i=0; i<numSpecParams; i++) {
			switch (i) {
				case NC:		// # LPC coeff -- option menu
				case SD:		// DFT size -- option menu
					XtVaGetValues(gSpecParamsV[i], XmNuserData, &w, NULL);
					XtVaGetValues(w, XmNnumChildren, &k, XmNchildren, &wl, NULL);
					for (n=0; n<k; n++) {
						XtVaGetValues(wl[n], XmNuserData, &v, NULL);
						if (v == (int)specParams[i].value) break;
					}
					XtVaSetValues(gSpecParamsV[i], XmNmenuHistory, wl[n], NULL);		// set selected value
					break;
				case NW:		// use windowing  -- radio buttons
					XtVaGetValues(gSpecParamsV[i], XmNchildren, &wl, NULL);
					k = specParams[i].value;
					XtVaSetValues(wl[0], XmNset, (k==1), NULL);
					XtVaSetValues(wl[1], XmNset, (k==0), NULL);
					break;
				default:		// all others are text fields
					sprintf(s, "%g", specParams[i].value);
					XmTextFieldSetString(gSpecParamsV[i], s);
					break;
			}
		}
		return;
	}
	
/* ***  read values  *** */

	XtVaGetValues(gSpecParamsD, XmNuserData, &spectro, NULL);
	
	for (i=0; i<numSpecParams; i++) {
		switch (i) {
			case NC:		// # LPC coeff -- option menu
			case SD:		// DFT size -- option menu
			case NW:		// use windowing  -- radio buttons
				XtVaGetValues(gSpecParamsV[i], XmNmenuHistory, &w, NULL);
				XtVaGetValues(w, XmNuserData, &k, NULL);	// value from userData
				newVals[i] = k;
				break;
			default:		// all others are text fields
				text = XmTextGetString(gSpecParamsV[i]);
				sscanf(text, "%g", &newVals[i]);
				XtFree(text);
				break;
		}
		if (i==0)
				{
				/* ***Ugly hack: for unknown reasons the first element of newVals[i] gets overwritten to 0
				when the following elements are written. So we save its value and later (line 200)
				we write it again after all other elements are
				written *** */
					tempVarHack=newVals[i];
				
				}

/* ***  range check  *** */

		if (newVals[i] < specParams[i].min || newVals[i] > specParams[i].max) {
			sprintf(s, "%g is out-of-range (%s)\n", newVals[i], specParams[i].desc);
			writetext(s);
			break;		
		} 
			
	} /* for i<numSpecParams */

	if (newVals[WD]*spectro->spers/1000. > newVals[SD]) {	// DFT window size must be less than current DFT size
		sprintf(s, "DFT window size in samples (%g) must be less than current DFT size", newVals[WD]*spectro->spers/1000.);
		writetext(s);
		i = WD;
	}
	
	if (i < numSpecParams) {	// something out of range
		XmTextFieldSetSelection(gSpecParamsV[i], 0, XmTextFieldGetLastPosition(gSpecParamsV[i]), CurrentTime);
		XmProcessTraversal(gSpecParamsV[i], XmTRAVERSE_CURRENT);
		XBell(XtDisplay(gSpecParamsD), 0);
		return;
	}
		newVals[0]=	tempVarHack;
/* ***  report changes  *** */

	for (i=0; i<numSpecParams; i++) {
		if (newVals[i] != (i<NC ? spectro->hamm_in_ms[i] : spectro->params[i])) {
			if (buttonID == spApplyCurrent) {		// apply to current waveform
				if (i < NC) {
					spectro->hamm_in_ms[i] = newVals[i];
					spectro->params[i] = newVals[i]*spectro->spers/1000. + .5;
					sprintf(s, "Updated %s (%.1f)\n", specParams[i].desc, newVals[i]);
				} else {
					sprintf(s, "Updated %s (%d)\n", specParams[i].desc, (int)newVals[i]);
					spectro->params[i] = newVals[i];
				}		
				writetext(s);
			} else {				// apply to all waveforms
				for (n=0; n<allspectros.count; n++) {
					if (i < NC) {
						allspectros.list[n]->hamm_in_ms[i] = newVals[i];
						allspectros.list[n]->params[i] = newVals[i]*spectro->spers/1000. + .5;
					} else {
						allspectros.list[n]->params[i] = newVals[i];
					}
				}
			}
		}
	}
			
	XtDestroyWidget(gSpecParamsD);
	gSpecParamsD = NULL;

	for (i=0; i<NUM_WINDOWS; i++)
		update(spectro->info[i], spectro, i);	// force redraw with new parameters
	
} /* HandleSpecParams */


/*
  ===========================================================================
	MapSGValues  - map spectrogram parameters between spectro and local gSGInit
  ---------------------------------------------------------------------------
	Args:
	  spectro	- current waveform
	  dir		- 0: spectro->gSGInit, 1: gSGInit->spectro

	Called from:
	  HandleSGParams
	  InitSGParams

	Side effects:
	  resets gSGInit/spectro values

	Returns:
	  nada
  ===========================================================================
*/

static void
MapSGValues(
	XSPECTRO *spectro,
	int dir)
{
	if (dir) {		// gSGInit -> spectro
		spectro->winms		= gSGInit[sgpWindow].v;
		spectro->slice		= gSGInit[sgpFrame].v;
		spectro->numav		= gSGInit[sgpAveragd].v;
		spectro->msdelta	= gSGInit[sgpOffset].v;
		spectro->specms 	= gSGInit[sgpDisplay].v;
		spectro->minmag		= gSGInit[sgpLow].v;
		spectro->maxmag		= gSGInit[sgpHigh].v;
		spectro->xpix		= gSGInit[sgpScale].v;
		spectro->fdifcoef	= gSGInit[sgpDiff].v;
		spectro->sec		= gSGInit[sgpUnits].v;
	} else {		// spectro -> gSGInit
		gSGInit[sgpWindow].v	= spectro->winms;
		gSGInit[sgpFrame].v		= spectro->slice;
		gSGInit[sgpAveragd].v	= spectro->numav;
		gSGInit[sgpOffset].v	= spectro->msdelta;
		gSGInit[sgpDisplay].v	= spectro->specms;
		gSGInit[sgpLow].v		= spectro->minmag;
		gSGInit[sgpHigh].v		= spectro->maxmag;
		gSGInit[sgpScale].v		= spectro->xpix;
		gSGInit[sgpDiff].v		= spectro->fdifcoef;
		gSGInit[sgpUnits].v		= spectro->sec;	
	}

} /* MapSGValues */


/*
  ===========================================================================
	HandleSGParams  - spectrogram parameters callback handler
  ---------------------------------------------------------------------------
  
	  Args:
		button		- clicked button widget
		buttonID	- button identifier

	  Called from:
		XmNactivateCallback (InitSpecParams)
		
	  Side effects:
		updates spectro parameters

	  Returns:
		nada
  ===========================================================================
*/

static void 
HandleSGParams(
	Widget	button,
	int buttonID,
	XmAnyCallbackStruct *reason)
{
	Widget w;
	WidgetList wl;
	XSPECTRO *spectro;
	int	i, n, k, v;
	char *text, s[255];
	float newVals[numSGParams],tempVarHack;
	extern void writetext(char *);

	XtVaGetValues(gSGParamsD, XmNuserData, &spectro, NULL);
	
/* ***  reset entry values on cancel  *** */

	if (buttonID == spCancel) {
		XtDestroyWidget(gSGParamsD);
		gSGParamsD = NULL;
		for (i=0; i<numSGParams; i++)
			gSGInit[i].v = gSGSaveVals[i];
		MapSGValues(spectro, 1);		// gSGInit -> spectro (reset values on entry)
		return;
	}
	
/* ***  reset default values  *** */

	if (buttonID == spDefaults) {
		set_defaults(spectro);			// xspec_util.c
		MapSGValues(spectro, 0);		// spectro -> gSGInit
		for (i=0; i<numSGParams; i++) {
			switch (i) {
				case sgpFrame:		// DFT size -- option menu
					XtVaGetValues(gSGParamsV[i], XmNuserData, &w, NULL);
					XtVaGetValues(w, XmNnumChildren, &k, XmNchildren, &wl, NULL);
					for (n=0; n<k; n++) {
						XtVaGetValues(wl[n], XmNuserData, &v, NULL);
						if (v == (int)(gSGInit[i].v)) break;
					}
					XtVaSetValues(gSGParamsV[i], XmNmenuHistory, wl[n], NULL);		// set selected value
					break;
				case sgpScale:		// scaling  -- radio buttons
					XtVaGetValues(gSGParamsV[i], XmNchildren, &wl, NULL);
					k = gSGInit[i].v;
					XtVaSetValues(wl[0], XmNset, (k==1), NULL);
					XtVaSetValues(wl[1], XmNset, (k==2), NULL);
					break;
				case sgpUnits:		// X axis units  -- radio buttons
					XtVaGetValues(gSGParamsV[i], XmNchildren, &wl, NULL);
					k = gSGInit[i].v;
					XtVaSetValues(wl[0], XmNset, (k==0), NULL);
					XtVaSetValues(wl[1], XmNset, (k==1), NULL);
					break;
				default:			// all others are text fields
					sprintf(s, "%g", gSGInit[i].v);
					XmTextFieldSetString(gSGParamsV[i], s);
					break;
			}
		}
		return;
	}
	
/* ***  read values (no range checking!)  *** */

	for (i=0; i<numSGParams; i++) {
		switch (i) {
			case sgpFrame:		// DFT size -- option menu
			case sgpScale:		// scaling  -- radio buttons
			case sgpUnits:		// X axis units  -- radio buttons
				XtVaGetValues(gSGParamsV[i], XmNmenuHistory, &w, NULL);
				XtVaGetValues(w, XmNuserData, &k, NULL);	// value from userData
				newVals[i] = k;
				break;
			default:			// all others are text fields
				text = XmTextGetString(gSGParamsV[i]);
				sscanf(text, "%g", &newVals[i]);
				if (i==sgpWindow)
				{
				/* ***Ugly hack: for unknown reasons the first element of newVals[i] gets overwritten to 0
				when the following elements are written. So we save its value and later (line 391)
				we write it again after all other elements are
				written *** */
					tempVarHack=newVals[i];
				
				}
				XtFree(text);
				break;
		}
	}
	newVals[sgpWindow]=tempVarHack;
	
/* ***  update  *** */

	for (i=0; i<numSGParams; i++)
		gSGInit[i].v = newVals[i];
	MapSGValues(spectro, 1);		// spectro -> gSGInit
	
	XtDestroyWidget(gSGParamsD);
	gSGParamsD = NULL;

	if (spectro->spectrogram) {
		swapstuff(spectro);			// force redraw with new parameters (xspec_util.c) 
		remapgray(spectro);
	}
	
} /* HandleSGParams */

	
/*
  ===========================================================================
	HandleMapping  - mapping callback handler (center window)
  ---------------------------------------------------------------------------
  
	  Args:
		dlog		- dialog widget
		buttonID	- button identifier

	  Called from:
		XmNmapCallback handler
		
	  Side effects:
		centers dlog in current display

	  Returns:
		nada
  ===========================================================================
*/

void 
HandleMapping(
	Widget	dlog,
	int data,
	XmAnyCallbackStruct *reason)
{
	short	h, w, dh, dw, x, y, s;
	Display	*d;

/* ***  center dialog on screen  *** */

	d = XtDisplay(dlog);
	s = XDefaultScreen(d);
	dh = DisplayHeight(d, s);
	dw = DisplayWidth(d, s);
	XtVaGetValues(dlog, XmNheight, &h, XmNwidth, &w, NULL);
	x = (dw - w)/2;
	y = (dh - h)/2;
	XtVaSetValues(dlog, XmNx, x, XmNy, y, NULL);
	XmProcessTraversal(dlog, XmTRAVERSE_CURRENT);

} /* HandleMapping */


/*
  ===========================================================================
	InitSpecParams   - initialize spectral parameters dialog
  ---------------------------------------------------------------------------

	  Args:
		parentShell		- application shell
		spectro			- current waveform

	  Called from:
		dobutton in xkl.c
	
	  Side effects:
	  	opens dialog 

	  Returns:
		nada
  ===========================================================================
*/

void
InitSpecParams(
  Widget parentShell,
  XSPECTRO *spectro)
{
	XmString Xstring[25];
	Arg a[10];
	int i, n, k, wc, sc=0;
	char s[250];
	Widget argForm, f, w[5], b;
	extern paramStruct specParams[];

/* ***  create dialog if necessary  *** */

	if (gSpecParamsD != NULL) {				// dialog already active
		XRaiseWindow(XtDisplay(gSpecParamsD), XtWindow(gSpecParamsD));
		XSetInputFocus(XtDisplay(gSpecParamsD), XtWindow(gSpecParamsD), RevertToParent, CurrentTime);
		return;
	}

	n = 0; 
	sprintf(s, "%s:  Set Spectral Parameters ...", spectro->wavename);
	XtSetArg(a[n], XmNdialogTitle,			XpMakeXstring(s)); n++;
//	XtSetArg(a[n], XmNdialogStyle,			XmDIALOG_FULL_APPLICATION_MODAL); n++;	// some WMs ignore this
	XtSetArg(a[n], XmNautoUnmanage,			FALSE); n++;
	XtSetArg(a[n], XmNnoResize,				TRUE); n++;
	XtSetArg(a[n], XmNdefaultPosition,		FALSE); n++;
	XtSetArg(a[n], XmNhorizontalSpacing,	10); n++;
	XtSetArg(a[n], XmNverticalSpacing,		10); n++;
	XtSetArg(a[n], XmNuserData,				spectro); n++;		// retain pointer to current waveform
	gSpecParamsD = XmCreateFormDialog(parentShell, "", a, n);
	XtAddCallback(gSpecParamsD, XmNmapCallback, (XtCallbackProc)HandleMapping, 0);

/* ***  arguments form  *** */

	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNhorizontalSpacing,	0); n++;
	XtSetArg(a[n], XmNverticalSpacing,		2); n++;
	XtSetArg(a[n], XmNtraversalOn,			TRUE); n++;
	argForm = XmCreateForm(gSpecParamsD, 0, a, n);
		
/* ***   populate it using specParms (xkl.c), spectro->params (current values)  *** */

	for (i=0; i<numSpecParams; i++) {
		n = 0;
		if (i == 0) {
			XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
		} else {
			XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
			XtSetArg(a[n], XmNtopWidget, 			f); n++;
		}
		XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
		XtSetArg(a[n], XmNrightAttachment,		XmATTACH_FORM); n++;
		XtSetArg(a[n], XmNleftOffset,			15); n++;
		f = XmCreateForm(argForm, 0, a, n);
		
		wc = 0;
		switch (i) {
		
			case NC:			// # LPC coeff -- implemented as an option menu
				w[4] = XmCreatePulldownMenu(f, "", NULL, 0);	// park pulldown widget in w[4]
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNsubMenuId, w[4]); n++;		// it is managed by the option menu
				gSpecParamsV[i] = w[wc++] = XmCreateOptionMenu(f, 0, a, n);
				for (k=2; k<=specParams[i].max; k++) {			// add entries; userData holds value
					sprintf(s, "%5d ", k);
					b = XtVaCreateManagedWidget(s, xmPushButtonGadgetClass, w[4], XmNuserData, k, NULL);
					if (k == spectro->params[i]) w[3] = b;		// park selected value in w[3]
				}
				XtVaSetValues(w[0], XmNmenuHistory, w[3], XmNuserData, w[4], NULL);	// set selected value

				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
				XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftOffset,			26); n++;
				XtSetArg(a[n], XmNlabelString, 			XpMakeXstring(specParams[i].desc)); n++;
				w[wc++] = (Widget)XmCreateLabelGadget(f, 0, a, n);
				
				break;

			case NW:		// use windowing  -- Yes|No radio buttons
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNorientation,			XmHORIZONTAL); n++;
				gSpecParamsV[i] = w[wc++] = XmCreateRadioBox(f, 0, a, n);
				k = spectro->params[i];					// select yes | no
				w[4] = XtVaCreateManagedWidget("Yes", xmToggleButtonGadgetClass, w[0], XmNset, (k==1), XmNuserData, 1, NULL);
				w[3] = XtVaCreateManagedWidget("No", xmToggleButtonGadgetClass, w[0], XmNset, (k==0), XmNuserData, 0, NULL);
				
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
				XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftOffset,			2); n++;
				XtSetArg(a[n], XmNlabelString, 			XpMakeXstring("Use Windowing")); n++;
				w[wc++] = (Widget)XmCreateLabelGadget(f, 0, a, n);
				
				break;
				
			case SD:		// DFT size -- implemented as an option menu
				w[4] = XmCreatePulldownMenu(f, "", NULL, 0);	// park pulldown widget in w[4]
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNsubMenuId, w[4]); n++;		// it is managed by the option menu
				gSpecParamsV[i] = w[wc++] = XmCreateOptionMenu(f, 0, a, n);
				for (k=0; k<sizeof(gDFTvals)/sizeof(int); k++) {		// add entries; userData holds value
					sprintf(s, "%5d ", gDFTvals[k]);
					b = XtVaCreateManagedWidget(s, xmPushButtonGadgetClass, w[4], XmNuserData, gDFTvals[k], NULL);
					if (gDFTvals[k] == spectro->params[i]) w[3] = b;	// park selected value in w[3]
				}
				XtVaSetValues(w[0], XmNmenuHistory, w[3], XmNuserData, w[4], NULL);		// set selected value

				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
				XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftOffset,			21); n++;
				XtSetArg(a[n], XmNlabelString, 			XpMakeXstring(specParams[i].desc)); n++;
				w[wc++] = (Widget)XmCreateLabelGadget(f, 0, a, n);
				
				break;
				
			default:		// all others are field | label
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNcolumns,				10); n++;
				XtSetArg(a[n], XmNmaxLength,			10); n++;
				if (specParams[i].type == FLOAT)
					sprintf(s, "%.1f", spectro->hamm_in_ms[i]);
				else
					sprintf(s, "%d", spectro->params[i]);
				XtSetArg(a[n], XmNvalue, 				s); n++;
				gSpecParamsV[i] = w[wc++] = (Widget)XmCreateTextField(f, 0, a, n);
				
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
				XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftOffset,			10); n++;
				if (specParams[i].type == FLOAT)
					sprintf(s, "%s  (%.1f : %.1f)", specParams[i].desc, specParams[i].min, specParams[i].max);
				else
					sprintf(s, "%s  (%d : %d)", specParams[i].desc, (int)specParams[i].min, (int)specParams[i].max);
				XtSetArg(a[n], XmNlabelString, 			XpMakeXstring(s)); n++;
				w[wc++] = (Widget)XmCreateLabelGadget(f, 0, a, n);
				
				break;
		}
		XtManageChildren(w, wc);			
		XtManageChild(f);

	} /* for i<numSpecParams */
	
	XtManageChild(argForm);
		
/* ***  buttons  *** */

	wc = n = 0;			// separator
	XtSetArg(a[n], XmNtopAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 		argForm); n++;
	XtSetArg(a[n], XmNleftAttachment,	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment, 	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNtopOffset,		10); n++;
	w[wc++] = XmCreateSeparatorGadget(gSpecParamsD, 0, a, n);
	
	n = 0;				// Apply to Current
	XtSetArg(a[n], XmNtopAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 		w[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,		20); n++;
	XtSetArg(a[n], XmNwidth, 			130); n++;
	XtSetArg(a[n], XmNheight, 			50); n++;
	XtSetArg(a[n], XmNlabelString,	XpMakeXstring(" Apply to Current \nWaveform")); n++;
	w[wc] = XmCreatePushButtonGadget(gSpecParamsD, 0, a, n);
	XtAddCallback(w[wc++], XmNactivateCallback, (XtCallbackProc)HandleSpecParams, (XtPointer)spApplyCurrent);
	
	n = 0;				// Apply to All
	XtSetArg(a[n], XmNtopAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 		w[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 		w[1]); n++;
	XtSetArg(a[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNwidth, 			130); n++;
	XtSetArg(a[n], XmNheight, 			50); n++;
	XtSetArg(a[n], XmNlabelString,	XpMakeXstring(" Apply to All \nWaveforms")); n++;
	w[wc] = XmCreatePushButtonGadget(gSpecParamsD, 0, a, n);
	XtAddCallback(w[wc++], XmNactivateCallback, (XtCallbackProc)HandleSpecParams, (XtPointer)spApplyAll);
	
	n = 0;				// Defaults
	XtSetArg(a[n], XmNtopAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 		w[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 		w[2]); n++;
	XtSetArg(a[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNwidth, 			80); n++;
	XtSetArg(a[n], XmNheight, 			50); n++;
	XtSetArg(a[n], XmNlabelString,	XpMakeXstring("Defaults")); n++;
	w[wc] = XmCreatePushButtonGadget(gSpecParamsD, 0, a, n);
	XtAddCallback(w[wc++], XmNactivateCallback, (XtCallbackProc)HandleSpecParams, (XtPointer)spDefaults);
	
	n = 0;				// Cancel
	XtSetArg(a[n], XmNtopAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 		w[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 		w[3]); n++;
	XtSetArg(a[n], XmNbottomAttachment,	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment, 	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightOffset,		20); n++;
	XtSetArg(a[n], XmNwidth, 			80); n++;
	XtSetArg(a[n], XmNheight, 			50); n++;
	XtSetArg(a[n], XmNlabelString,	XpMakeXstring("Cancel")); n++;
	w[wc] = XmCreatePushButtonGadget(gSpecParamsD, 0, a, n);
	XtAddCallback(w[wc++], XmNactivateCallback, (XtCallbackProc)HandleSpecParams, (XtPointer)spCancel);
	
	XtManageChildren(w, wc);

	XtVaSetValues(gSpecParamsD, XmNdefaultButton, w[1], NULL);

	XtManageChild(gSpecParamsD);
	
	for (n=0; n<sc; n++)				// deallocate XmStrings
		XmStringFree(Xstring[n]);

} /* InitSpecParams */


/*
  ===========================================================================
	InitSGParams   - initialize spectrogram parameters dialog
  ---------------------------------------------------------------------------

	  Args:
		parentShell		- application shell
		spectro			- current waveform

	  Called from:
		dobutton in xkl.c
	
	  Side effects:
	  	opens dialog 

	  Returns:
		nada
  ===========================================================================
*/

void
InitSGParams(
  Widget parentShell,
  XSPECTRO *spectro)
{
	XmString Xstring[25];
	Arg a[10];
	int i, n, k, wc, sc=0;
	char s[250];
	Widget argForm, f, w[5], b;
	extern paramStruct specParams[];

/* ***  save initial values for restore on cancel  *** */

	MapSGValues(spectro, 0);		// spectro -> gSGInit
	
	for (i=0; i<numSGParams; i++)
		gSGSaveVals[i] = gSGInit[i].v;

/* ***  create dialog if necessary  *** */

	if (gSGParamsD != NULL) {				// dialog already active
		XRaiseWindow(XtDisplay(gSGParamsD), XtWindow(gSGParamsD));
		XSetInputFocus(XtDisplay(gSGParamsD), XtWindow(gSGParamsD), RevertToParent, CurrentTime);
		return;
	}
	
	n = 0; 
	sprintf(s, "%s:  Set Spectrogram Parameters ...", spectro->wavename);
	XtSetArg(a[n], XmNdialogTitle,			XpMakeXstring(s)); n++;
//	XtSetArg(a[n], XmNdialogStyle,			XmDIALOG_FULL_APPLICATION_MODAL); n++;	// some WMs ignore this
	XtSetArg(a[n], XmNautoUnmanage,			FALSE); n++;
	XtSetArg(a[n], XmNnoResize,				TRUE); n++;
	XtSetArg(a[n], XmNdefaultPosition,		FALSE); n++;
	XtSetArg(a[n], XmNhorizontalSpacing,	10); n++;
	XtSetArg(a[n], XmNverticalSpacing,		10); n++;
	XtSetArg(a[n], XmNuserData,				spectro); n++;		// retain pointer to current waveform
	gSGParamsD = XmCreateFormDialog(parentShell, "", a, n);
	XtAddCallback(gSGParamsD, XmNmapCallback, (XtCallbackProc)HandleMapping, 0);

/* ***  arguments form  *** */

	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNhorizontalSpacing,	0); n++;
	XtSetArg(a[n], XmNverticalSpacing,		2); n++;
	XtSetArg(a[n], XmNtraversalOn,			TRUE); n++;
	argForm = XmCreateForm(gSGParamsD, 0, a, n);

/* ***   populate it using gSGInit  *** */

	for (i=0; i<numSGParams; i++) {
		n = 0;
		if (i == 0) {
			XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
		} else {
			XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
			XtSetArg(a[n], XmNtopWidget, 			f); n++;
		}
		XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
		XtSetArg(a[n], XmNrightAttachment,		XmATTACH_FORM); n++;
		XtSetArg(a[n], XmNleftOffset,			15); n++;
		f = XmCreateForm(argForm, 0, a, n);
		
		wc = 0;
		switch (i) {
		
			case sgpFrame:		// DFT size -- implemented as an option menu
				w[4] = XmCreatePulldownMenu(f, "", NULL, 0);	// park pulldown widget in w[4]
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNsubMenuId, w[4]); n++;		// it is managed by the option menu
				gSGParamsV[i] = w[wc++] = XmCreateOptionMenu(f, 0, a, n);
				for (k=0; k<numDFTvalsSG; k++) {		// add entries; userData holds value
					sprintf(s, "%5d ", gDFTvals[k]);
					b = XtVaCreateManagedWidget(s, xmPushButtonGadgetClass, w[4], XmNuserData, gDFTvals[k], NULL);
					if (gDFTvals[k] == (int)(gSGInit[i].v)) w[3] = b;	// park selected value in w[3]
				}
				XtVaSetValues(w[0], XmNmenuHistory, w[3], XmNuserData, w[4], NULL);		// set selected value

				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
				XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftOffset,			21); n++;
				XtSetArg(a[n], XmNlabelString, 			XpMakeXstring(gSGInit[i].blurb)); n++;
				w[wc++] = (Widget)XmCreateLabelGadget(f, 0, a, n);
				
				break;
				
			case sgpScale:		// scaling  -- x1 | x2 radio buttons
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNorientation,			XmHORIZONTAL); n++;
				gSGParamsV[i] = w[wc++] = XmCreateRadioBox(f, 0, a, n);
				k = gSGInit[i].v;					// select msecs | secs
				w[4] = XtVaCreateManagedWidget("x 1", xmToggleButtonGadgetClass, w[0], XmNset, (k==1), XmNuserData, 1, NULL);
				w[3] = XtVaCreateManagedWidget("x 2", xmToggleButtonGadgetClass, w[0], XmNset, (k==2), XmNuserData, 2, NULL);
				
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
				XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftOffset,			10); n++;
				XtSetArg(a[n], XmNlabelString, 			XpMakeXstring(gSGInit[i].blurb)); n++;
				w[wc++] = (Widget)XmCreateLabelGadget(f, 0, a, n);
				
				break;
				
			case sgpUnits:		// X axis units  -- msec | sec radio buttons
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNorientation,			XmHORIZONTAL); n++;
				gSGParamsV[i] = w[wc++] = XmCreateRadioBox(f, 0, a, n);
				k = gSGInit[i].v;					// select msecs | secs
				w[4] = XtVaCreateManagedWidget("ms", xmToggleButtonGadgetClass, w[0], XmNset, (k==0), XmNuserData, 0, NULL);
				w[3] = XtVaCreateManagedWidget("sec", xmToggleButtonGadgetClass, w[0], XmNset, (k==1), XmNuserData, 1, NULL);
				
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
				XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftOffset,			4); n++;
				XtSetArg(a[n], XmNlabelString, 			XpMakeXstring(gSGInit[i].blurb)); n++;
				w[wc++] = (Widget)XmCreateLabelGadget(f, 0, a, n);
				
				break;
				
			default:		// all others are field | label
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNcolumns,				10); n++;
				XtSetArg(a[n], XmNmaxLength,			10); n++;
				if (gSGInit[i].type == FLOAT)
					sprintf(s, "%.1f", gSGInit[i].v);
				else
					sprintf(s, "%d", (int)(gSGInit[i].v));
				XtSetArg(a[n], XmNvalue, 				s); n++;
				gSGParamsV[i] = w[wc++] = (Widget)XmCreateTextField(f, 0, a, n);
				
				n = 0;
				XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
				XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
				XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
				XtSetArg(a[n], XmNleftOffset,			10); n++;
				XtSetArg(a[n], XmNlabelString, 			XpMakeXstring(gSGInit[i].blurb)); n++;
				w[wc++] = (Widget)XmCreateLabelGadget(f, 0, a, n);
				
				break;
		}
		XtManageChildren(w, wc);			
		XtManageChild(f);

	} /* for i<numSpecParams */
	
	XtManageChild(argForm);
		
/* ***  buttons  *** */

	wc = n = 0;			// separator
	XtSetArg(a[n], XmNtopAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 		argForm); n++;
	XtSetArg(a[n], XmNleftAttachment,	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment, 	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNtopOffset,		10); n++;
	w[wc++] = XmCreateSeparatorGadget(gSGParamsD, 0, a, n);
	
	n = 0;				// Apply to Current
	XtSetArg(a[n], XmNtopAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 		w[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,		90); n++;
	XtSetArg(a[n], XmNwidth, 			70); n++;
	XtSetArg(a[n], XmNheight, 			30); n++;
	XtSetArg(a[n], XmNlabelString,	XpMakeXstring("Apply")); n++;
	w[wc] = XmCreatePushButtonGadget(gSGParamsD, 0, a, n);
	XtAddCallback(w[wc++], XmNactivateCallback, (XtCallbackProc)HandleSGParams, (XtPointer)spApplyCurrent);
	
	n = 0;				// Defaults
	XtSetArg(a[n], XmNtopAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 		w[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 		w[1]); n++;
	XtSetArg(a[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNwidth, 			70); n++;
	XtSetArg(a[n], XmNheight, 			30); n++;
	XtSetArg(a[n], XmNlabelString,	XpMakeXstring("Defaults")); n++;
	w[wc] = XmCreatePushButtonGadget(gSGParamsD, 0, a, n);
	XtAddCallback(w[wc++], XmNactivateCallback, (XtCallbackProc)HandleSGParams, (XtPointer)spDefaults);
	
	n = 0;				// Cancel
	XtSetArg(a[n], XmNtopAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 		w[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,	XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 		w[2]); n++;
	XtSetArg(a[n], XmNbottomAttachment,	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNwidth, 			70); n++;
	XtSetArg(a[n], XmNheight, 			30); n++;
	XtSetArg(a[n], XmNlabelString,	XpMakeXstring("Cancel")); n++;
	w[wc] = XmCreatePushButtonGadget(gSGParamsD, 0, a, n);
	XtAddCallback(w[wc++], XmNactivateCallback, (XtCallbackProc)HandleSGParams, (XtPointer)spCancel);
	
	XtManageChildren(w, wc);

	XtVaSetValues(gSGParamsD, XmNdefaultButton, w[1], NULL);

	XtManageChild(gSGParamsD);
	
	for (n=0; n<sc; n++)				// deallocate XmStrings
		XmStringFree(Xstring[n]);

} /* InitSGParams */		

