/*
  ===========================================================================
	XKL_LABEL  - Xkl label support
  ---------------------------------------------------------------------------

	Contains:
	  
	  (Local)
		MakeLabelList		- create displayed label list
		InsertIntoQueue		- find offset-sorted location in labels queue
		HandleEditLabel		- (specific) label editing callback handler
		HandleEditLabels	- label list editing callback handler
	  
	  (Global)	
		LoadLabels		- load labels from file
		SaveLabels		- save labels to file
		KillLabels		- delete existing labels
		EditLabels		- display existing labels in a list for selection
		MakeLabel		- create label
		InitLabel		- open label creation dialog
		
	Labels are stored in offset order as the linked list spectro->labels
	Currently supported:  name, comment and (msec) offset fields
  ===========================================================================
*/

#include "xspec_util.h"		/* includes X headers (xinfo.h) */

/* ***  local static data  *** */

static Widget gEditLabelsD = NULL;		// edit existing labels dialog
static Widget gEditLabelD = NULL;		// edit (specific) label dialog

enum {					// label editing fields
	lfOffset = 0,
	lfName,
	lfComment,
	lfNumFields
};
static Widget gLabFields[lfNumFields];	// label editing widgets

// XmString creation macro
#define XpMakeXstring(asciiz) (Xstring[sc++]=XmStringCreateLocalized(asciiz))

enum {					// edit labels dialog button actions
	elDone,					// close dialog
	elEdit,					// edit selected label
	elDelete,				// delete selected labels
	elUpdate				// update list contents
};

/*
  ===========================================================================
	MakeLabelList  - create displayed label list
  ---------------------------------------------------------------------------
	Args:
	  spectro  - current waveform
	  lc		- label count

	Called from:
	  EditLabels
	  HandleEditLabels

	Side effects:
	  sets lc to label count
	  allocates memory for string table; free after list instantiation

	Returns:
	  string table from spectro->labels for scrolled list display
  ===========================================================================
*/

static XmStringTable
MakeLabelList(
	XSPECTRO *spectro,
	int *lc)
{
	LABEL *lp;
	int n;
	char s[100];
	XmStringTable labels;

	for (n=0,lp=spectro->labels; lp; n++,lp=lp->next) ;	 // get label count
	*lc = n;
	labels = (XmStringTable)calloc(n, sizeof(XmString));
	for (n=0,lp=spectro->labels; lp; n++,lp=lp->next) {
		sprintf(s, "%10.2f   %s", lp->offset, lp->name);
		labels[n] = XmStringCreateLocalized(s);
	}
	return labels;

} /* MakeLabelList */


/*
  ===========================================================================
	InsertIntoQueue  - find offset-sorted location in labels queue
  ---------------------------------------------------------------------------
	Args:
	  spectro	- current waveform
	  offset	- msec offset from start of waveform
	
	Called from:
	  HandleEditLabel
	  MakeLabel
	
	Side effects:
	  creates label node
	  sets lp->offset to input offset
	
	Returns:
	  pointer to label node at insertion location
	
	If multiple labels with same offset exist returns first in queue
  ===========================================================================
*/

static LABEL*
InsertIntoQueue(
	XSPECTRO *spectro,
	float offset)
{
	LABEL *lp, *plp;

	if (spectro->labels) {		// labels exist -- find place in queue
		for (plp=NULL,lp=spectro->labels; lp; lp=lp->next) {
			if (offset < lp->offset) break;
			plp = lp;
		}
		if (plp) {					// not first label
			plp->next = (LABEL *)calloc(1, sizeof(LABEL));
			plp->next->next = lp;
			lp = plp->next;
		} else {					// first in queue
			plp = spectro->labels = (LABEL *)calloc(1, sizeof(LABEL));
			plp->next = lp;
			lp = plp;
		}
	} else {					// no previous labels exist
		lp = spectro->labels = (LABEL *)calloc(1, sizeof(LABEL));
	}
	lp->offset = offset;
	
	return lp;
	
} /* InsertIntoQueue */


/*
  ===========================================================================
	HandleEditLabels  - label list editing callback handler
  ---------------------------------------------------------------------------
	Args:
	  button	- parent widget
	  buttonID	- button identifier
	  spectro  - current waveform

	Called from:
	  InitLabel
	  EditLabels
	  HandleEditLabel

	Side effects:
	  sets lc to label count
	  allocates memory for string table; free after list instantiation

	Returns:
	  nada
  ===========================================================================
*/

static void HandleEditLabels(
	Widget	button,
	int buttonID,
	XmAnyCallbackStruct *reason)
{
	Widget list;
	XSPECTRO *spectro;
	XmString *items;
	XmString *selItems;
	LABEL *lp, *plp;
	int n, j, k, numItems, numSelItems;
	void InitLabel();

// retrieve data
	XtVaGetValues(gEditLabelsD, XmNuserData, &list, NULL);
	XtVaGetValues(list, XmNuserData, &spectro, 
						  XmNitemCount, &numItems, 
						  XmNitems, &items, 
						  XmNselectedItemCount, &numSelItems, 
						  XmNselectedItems, &selItems, NULL);

// update list contents
	if (buttonID == elUpdate) { 
		XmListDeleteAllItems(list);
		items = MakeLabelList(spectro, &numItems); 
		XmListAddItemsUnselected(list, items, numItems, 0);
		for (n=0; n<numItems; n++)
			XmStringFree(items[n]);
		free(items);
		return;
	}

// finished editing
	if (buttonID == elDone) {
		XtDestroyWidget(gEditLabelsD);
		gEditLabelsD = NULL;
		return;
	}

// editing requires single selection
	if (buttonID == elEdit) {
		if (numSelItems == 1) {
			InitLabel(gEditLabelsD, spectro, XmListItemPos(list, selItems[0])-1);
		} else {
			XBell(XtDisplay(button), 0);
		}
	}

// delete selected labels
	if (buttonID == elDelete) {
		for (n=numSelItems; n; n--) {
			k = XmListItemPos(list, selItems[n-1]) - 1;
			for (j=0,lp=spectro->labels,plp=NULL; j<k; j++) {
				plp = lp;
				lp = lp->next;
			}
			if (plp)
				plp->next = (lp ? lp->next : NULL);
			else
				spectro->labels = (lp ? lp->next : NULL);
			free(lp);
		}
		XmListDeleteAllItems(list);
		items = MakeLabelList(spectro, &numItems); 
		XmListAddItemsUnselected(list, items, numItems, 0);
		for (n=0; n<numItems; n++)
			XmStringFree(items[n]);
		free(items);
		for (n=0; n<3; n++)
			draw_cursor(spectro, n, spectro->info[n]);
	}

} /* HandleEditLabels */


/*
  ===========================================================================
	HandleEditLabel  - (specific) label editing callback handler
  ---------------------------------------------------------------------------
	Args:
	  button	- parent widget
	  spectro	- current waveform

	Called from:
	   InitLabel

	Side effects:
	  sets lc to label count
	  allocates memory for string table; free after list instantiation

	Returns:
	  nada
  ===========================================================================
*/

static void 
HandleEditLabel(
	Widget	button,
	XSPECTRO *spectro,
	XmAnyCallbackStruct *reason)
{
	LABEL *lp, *plp, *qlp;
	char name[MAXLABLEN], comment[MAXLABCOM], s[100], *text;
	int index, n;
	float offset;
	//static void HandleEditLabels(); //Commented out by LDN on 07/14/2008 to compile under Mac Os X

// no change on cancel
	if (!spectro) {
		XtDestroyWidget(gEditLabelD);
		gEditLabelD = NULL;
		return;
	}

// make/change label
	XtVaGetValues(button, XmNuserData, &index, NULL);	// retrieve label index (-1 new, else index into spectro->labels)
	
	text = XmTextGetString(gLabFields[lfOffset]);				// get new label offset
	offset = atof(text);
	XtFree(text);
	
	text = XmTextGetString(gLabFields[lfName]);					// get new label name
	strncpy(name, text, MAXLABLEN-1);
	name[MAXLABLEN-1] = 0;
	XtFree(text);
	
	text = XmTextGetString(gLabFields[lfComment]);				// get new label comment
	strncpy(comment, text, MAXLABCOM-1);
	comment[MAXLABCOM-1] = 0;
	XtFree(text);
	
	XtDestroyWidget(gEditLabelD);
	gEditLabelD = NULL;

	if (index < 0) {	// insert new label into existing list (if any)
		lp = InsertIntoQueue(spectro, offset);
		strcpy(lp->name, name);
		strcpy(lp->comment, comment);
		sprintf(s, "Created label %s (%.2f ms)\n", lp->name, lp->offset);
		writetext(s);
	} else {			// edit existing label
		for (n=0,lp=spectro->labels,plp=NULL; n<index; n++,lp=lp->next) 
			plp = lp;
		if (lp->offset != offset) {					// resort label list
			if (plp)
				plp->next = lp->next;
			else
				spectro->labels = lp->next;
			for (plp=NULL,qlp=spectro->labels; qlp; qlp=qlp->next) {
				if (offset < qlp->offset) break;
				plp = qlp;
			}
			if (plp) 
				plp->next = lp;
			else
				spectro->labels = lp;
			lp->next = qlp;		
		}
		lp->offset = offset;
		strcpy(lp->name, name);
		strcpy(lp->comment, comment);
		HandleEditLabels(button, elUpdate, NULL);	// update displayed list
	}

// replot the labels
	for (n=0; n<3; n++)
		draw_cursor(spectro, n, spectro->info[n]);

} /* HandleEditLabel */

/*
  ===========================================================================
	LoadLabels   - load labels from file
  ---------------------------------------------------------------------------
	Currently four types of ASCII label file formats are supported, distinguished
	here by file extension:
	
		.lm		- Liu style landmark files
		.lmv	- vowel landmark files
		.label	- LEX style label files
		.lbl	- Xkl user saved files

	Args:
	  spectro   - current waveform

	Called from:
	  filesb_dialog (XmNokCallback; see HandleFileMenu in xkl.c)
		
	Side effects:
	  valid labels appended to labels list of current waveform
	  labels replotted

	Returns:
	  nada
  ===========================================================================
*/

void
LoadLabels(
	Widget w, 
	XSPECTRO *spectro,
	XmAnyCallbackStruct *reason)  
{
	FILE *fp;
	LABEL *lp;
	int i, n;
	float v;
	char *p, *q, k, lbl[20], line[350], fname[300]; 

	XtVaGetValues(XmFileSelectionBoxGetChild(filesb_dialog, XmDIALOG_TEXT), XmNvalue, &p, NULL);
	strcpy(fname, p);
	XtFree(p);

	p = strrchr(fname, '.');
	if (!p) return;		// no selection

	XtUnmanageChild(filesb_dialog);
	XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
	XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);

	for (q=p; *q; q++)
		*q = tolower(*q);					// force lowercase

/* case 1:	Liu style landmark files (*.lm) */

	if (!strcmp(p,".lm")) {

		if (!(fp = fopen(fname, "rt"))) {			
			sprintf(line, "ERROR attempting to open %s\n", fname);
			writetext(line);
			return;
		}

		while (1) {
			fgets(line, 100, fp);
			if (line[0] == '#') break;
			if (feof(fp)) break;
		}
		while (1) {
			fgets(line, 100, fp);
			if (feof(fp)) break;
			sscanf(line, "%f %*d %s", &v, lbl);
			lp = InsertIntoQueue(spectro, v*1000.);		// secs -> msecs
			strcpy(lp->name, lbl);
		}
 
		fclose (fp);

/* case 2:	LEX style label files (*.label) */

		// Time values currently supported:
		//	 <val>
		//	 [<val>]
		//	 <val>*
		// anything else does not generate a label

	} else if (!strcmp(p, ".label")) {

		if (!(fp = fopen(fname, "rt"))) {
			sprintf(line, "ERROR attempting to open %s\n", fname);
			writetext(line);
			return;
		}

		while (1) {
			if (!fgets(line, 100, fp)) break;
			if (line[0] != '<') continue;
			if (!fgets(line, 100, fp)) break;
			if (strncmp(line,"Time",4)) continue;
			p = &line[5];
			while (p && isspace(*p))
				++p;
			n = sscanf((*p == '[' ? p+1 : p), "%d", &i);
			if (n < 1) continue;				 // no offset available
			n = strlen(line)-1;
			while (isspace(line[n]))		 // find last non whitespace character
				--n;
			k = line[n];
			if (!fgets(line, 100, fp)) break;
			if (strncmp(line,"Symbol",6)) continue;
			sscanf(&line[7], "%s", lbl);
			switch (k) {
				case ']':
					n = strlen(lbl);
					lbl[n+2] = 0;
					lbl[n+1] = ']';
					for (; n; n--)
						lbl[n] = lbl[n-1];
					lbl[0] = '[';
					break;
				case '?':
					strcat(lbl, "?");
					break;
				default:
					break;
			}
			lp = InsertIntoQueue(spectro, (float)i);
			strcpy(lp->name, lbl);
		}
 
		fclose (fp);

/* case 3:	Vowel landmark label files (*.lmv) */

	} else if (!strcmp(p, ".lmv")) {

		if (!(fp = fopen(fname, "rt"))) {
			sprintf(line, "ERROR attempting to open %s\n", fname);
			writetext(line);
			return;
		}

		while (1) {
			fgets(line, 100, fp);
			if (line[0] == '#') break;
			if (feof(fp)) break;
		}
		while (1) {
			fgets(line, 100, fp);
			if (feof(fp)) break;
			sscanf(line, "%f %*d %s", &v, lbl);
			p=strchr(lbl,';');
			if (p)
			 	*p = 0;	 // kill post ';' stuff
			lp = InsertIntoQueue(spectro, v*1000.);		// secs -> msecs
			strcpy(lp->name, lbl);
		}
 
		fclose (fp);

/* case 4:	Xkl user-generated label files (*.lbl) */

	} else if (!strcmp(p, ".lbl")) {

		if (!(fp = fopen(fname, "rt"))) {
			sprintf(line, "ERROR attempting to open %s\n", fname);
			writetext(line);
			return;
		}
		
		while (1) {
			fgets(line, 100, fp);
			if (feof(fp)) break;
			if (!(p = strrchr(line, '\n'))) continue;
			*p = 0;									// kill \n
			if (!(p = strchr(line, '\t'))) continue;
			sscanf(p, "%f", &v);					// scan offset
			lp = InsertIntoQueue(spectro, v);		// make label
			*p = 0;
			strcpy(lp->name, line);					// copy label name
			if (!(p = strchr(p+1, '\t'))) continue;
			strcpy(lp->comment, p+1);				// copy label comment
		}
		
		fclose (fp);

/* else ERROR */

	} else {
		sprintf(line, "ERROR; unknown label file type (%s)\n", fname);
		writetext(line);
		return;
	}

/* plot the labels */

	for (i=0; i<3; i++)
		draw_cursor(spectro, i, spectro->info[i]);

} /* LoadLabels */



/*
  ===========================================================================
	SaveLabels  - save labels to file
  ---------------------------------------------------------------------------
	Args:
	  spectro   - current waveform
	
	Called from:
	  filesb_dialog (XmNokCallback; see HandleFileMenu in xkl.c)
	
	Returns:
	  nada
	  
	Output file format:  each label written one per line as
	  <NAME>\t<OFFSET (msecs)>\t<COMMENT>\n
  ===========================================================================
*/

void
SaveLabels(
	Widget w, 
	XSPECTRO *spectro,
	XmAnyCallbackStruct *reason)  
{
	FILE *fp;
	LABEL *lp;
	char *p, fname[300], line[350]; 

	XtVaGetValues(XmFileSelectionBoxGetChild(filesb_dialog, XmDIALOG_TEXT), XmNvalue, &p, NULL);
	strcpy(fname, p);
	XtFree(p);

	p = strrchr(fname, '.');
	if (p) *p = 0;
	if (fname[strlen(fname)-1] == '/') {
		XBell(XtDisplay(w), 0);		// need name
		return;
	}
	strcat(fname, ".lbl");	// force .lbl extension

	XtUnmanageChild(filesb_dialog);
	XtRemoveAllCallbacks(filesb_dialog, XmNokCallback);
	XtRemoveAllCallbacks(filesb_dialog, XmNcancelCallback);

	if (!(fp = fopen(fname, "wt"))) {
		sprintf(line, "ERROR attempting to open %s\n", fname);
		writetext(line);
		return;
	}
	
	for (lp=spectro->labels; lp; lp=lp->next) 
		fprintf(fp, "%s\t%g\t%s\n", lp->name, lp->offset, lp->comment);
		
	fclose(fp);
	
	sprintf(line, "labels saved to %s\n", fname);
	writetext(line);

} /* SaveLabels */


/*
  ===========================================================================
	KillLabels  - delete existing labels
  ---------------------------------------------------------------------------
	Args:
	  spectro		- current waveform
	
	Called from:
	  warning dialog callback (see HandleFileMenu in xkl.c
	
	Side effects:
	  deletes all labels from spectro
	
	Returns:
	  nada
  ===========================================================================
*/

void
KillLabels(
	Widget w, 
	XSPECTRO *spectro,
	XtPointer call_data)
{
	LABEL *lp;
	int n;

	XtUnmanageChild(warning);
	XtRemoveAllCallbacks(warning, XmNokCallback);
	XtRemoveAllCallbacks(warning, XmNcancelCallback);

	while (spectro->labels) {
		lp = spectro->labels;
		spectro->labels = lp->next;
		free(lp);
	}
	for (n=0; n<3; n++)			// replot sans labels
		draw_cursor(spectro, n, spectro->info[n]);

} /* KillLabels */


/*
  ===========================================================================
	EditLabels  - display existing labels in a list for selection
  ---------------------------------------------------------------------------
	Args:
	  parent	- mama widge
	  spectro	- current waveform
	
	Called from:
	  HandleFileMenu in xkl.c
	
	Side effects:
	  opens dialog 
	
	Returns:
	  nada
  ===========================================================================
*/

void
EditLabels(
  Widget parent,
  XSPECTRO *spectro)
{
	Arg a[15];
	XmString Xstring[10];
	Widget list, w[2], b[4];
	XFontStruct *font;
	XmFontList fontList;
	XmStringTable labels;
	int n, lc, sc=0;
	void HandleMapping();	// xkl_dlog.c

// create dialog if necessary

	if (gEditLabelD != NULL) return;	// specific label edit in progress

	if (gEditLabelsD != NULL) {			// dialog already active
		XRaiseWindow(XtDisplay(gEditLabelsD), XtWindow(gEditLabelsD));
		XSetInputFocus(XtDisplay(gEditLabelsD), XtWindow(gEditLabelsD), RevertToParent, CurrentTime);
		return;
	}
	
	n = 0;
	XtSetArg(a[n], XmNdialogTitle, XpMakeXstring("Edit Labels...")); n++;
	XtSetArg(a[n], XmNautoUnmanage,			FALSE); n++;
	XtSetArg(a[n], XmNnoResize,				TRUE); n++;
	XtSetArg(a[n], XmNdefaultPosition,		FALSE); n++;
	XtSetArg(a[n], XmNhorizontalSpacing,	10); n++;
	XtSetArg(a[n], XmNverticalSpacing,		10); n++;
	XtSetArg(a[n], XmNwidth,				300); n++;
	gEditLabelsD = XmCreateFormDialog(parent, "", a, n);
	XtAddCallback(gEditLabelsD, XmNmapCallback, (XtCallbackProc)HandleMapping, 0);

// offset label
	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			25); n++;
	XtSetArg(a[n], XmNlabelString, 			XpMakeXstring("Offset")); n++;
	w[0] = (Widget)XmCreateLabelGadget(gEditLabelsD, 0, a, n);
	
// name label
	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget,			w[0]); n++;
	XtSetArg(a[n], XmNleftOffset,			30); n++;
	XtSetArg(a[n], XmNlabelString, 			XpMakeXstring("Name")); n++;
	w[1] = (Widget)XmCreateLabelGadget(gEditLabelsD, 0, a, n);
	
// label list
	font = XLoadQueryFont(XtDisplay(parent), "-*-courier-medium-r-*--12-*");
	fontList = XmFontListAppendEntry(NULL, XmFontListEntryCreate("font", XmFONT_IS_FONT, font));
	labels = MakeLabelList(spectro, &lc);

	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget,			w[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNvisibleItemCount,		10); n++;
	XtSetArg(a[n], XmNtopOffset,			2); n++;
	XtSetArg(a[n], XmNfontList, 			fontList); n++;
	XtSetArg(a[n], XmNselectionPolicy,		XmEXTENDED_SELECT); n++;
	XtSetArg(a[n], XmNitemCount,			lc); n++;
	XtSetArg(a[n], XmNitems,				labels); n++;
	XtSetArg(a[n], XmNuserData,				spectro); n++;		// userData holds spectro
	list = (Widget)XmCreateScrolledList(gEditLabelsD, 0, a, n);
	XtAddCallback(list, XmNdefaultActionCallback, (XtCallbackProc)HandleEditLabels, (XtPointer)elEdit);
	XmFontListFree(fontList);
	for (n=0; n<lc; n++)
		XmStringFree(labels[n]);
	free(labels);

// buttons
	n = 0;			// separator
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 			list); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment, 		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNtopOffset,			15); n++;
	b[0] = XmCreateSeparatorGadget(gEditLabelsD, 0, a, n);
	
	n = 0;			// done	
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 			b[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNbottomAttachment, 	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			25); n++;
	XtSetArg(a[n], XmNwidth, 				70); n++;
	XtSetArg(a[n], XmNheight,				30); n++;
	XtSetArg(a[n], XmNtraversalOn,			TRUE); n++;
	XtSetArg(a[n], XmNnavigationType,		XmTAB_GROUP); n++;
	XtSetArg(a[n], XmNlabelString, XpMakeXstring("Done")); n++;
	b[1] = XmCreatePushButtonGadget(gEditLabelsD, 0, a, n);
	XtAddCallback(b[1], XmNactivateCallback, (XtCallbackProc)HandleEditLabels, (XtPointer)elDone);
	
	n = 0;			// edit	
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 			b[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 			b[1]); n++;
	XtSetArg(a[n], XmNbottomAttachment, 	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNwidth, 				70); n++;
	XtSetArg(a[n], XmNheight,				30); n++;
	XtSetArg(a[n], XmNleftOffset,			5); n++;
	XtSetArg(a[n], XmNtraversalOn,			TRUE); n++;
	XtSetArg(a[n], XmNnavigationType,		XmTAB_GROUP); n++;
	XtSetArg(a[n], XmNlabelString, XpMakeXstring("Edit")); n++;
	b[2] = XmCreatePushButtonGadget(gEditLabelsD, 0, a, n);
	XtAddCallback(b[2], XmNactivateCallback, (XtCallbackProc)HandleEditLabels, (XtPointer)elEdit);

	n = 0;			// delete	
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 			b[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 			b[2]); n++;
	XtSetArg(a[n], XmNbottomAttachment, 	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNwidth, 				70); n++;
	XtSetArg(a[n], XmNheight,				30); n++;
	XtSetArg(a[n], XmNleftOffset,			5); n++;
	XtSetArg(a[n], XmNtraversalOn,			TRUE); n++;
	XtSetArg(a[n], XmNnavigationType,		XmTAB_GROUP); n++;
	XtSetArg(a[n], XmNlabelString, XpMakeXstring("Delete")); n++;
	b[3] = XmCreatePushButtonGadget(gEditLabelsD, 0, a, n);
	XtAddCallback(b[3], XmNactivateCallback, (XtCallbackProc)HandleEditLabels, (XtPointer)elDelete);

// manage everything
	XtManageChildren(w, 2);		
	XtManageChildren(b, 4);
	XtManageChild(list);

	XtVaSetValues(gEditLabelsD, XmNdefaultButton, b[1], XmNuserData, list, NULL);	// userdata holds list

	XtManageChild(gEditLabelsD);
	
	for (n=0; n<sc; n++)				// deallocate XmStrings
		XmStringFree(Xstring[n]);

} /* EditLabels */



/*
  ===========================================================================
	MakeLabel  - create unnamed label immediately
  ---------------------------------------------------------------------------
	Args:
	  spectro	- current waveform
	
	Called from:
	  HandleFileMenu in xkl.c
	
	Side effects:
	  insert new label into spectro->labels
	
	Returns:
	  nada
  ===========================================================================
*/

void
MakeLabel(
	XSPECTRO *spectro)
{
	char s[100];

	InsertIntoQueue(spectro, spectro->savetime);
	sprintf(s, "Created unnamed label (%.2f ms)\n", spectro->savetime);
	writetext(s);

} /* MakeLabel */


/*
  ===========================================================================
	InitLabel  - open label creation/editing dialog
  ---------------------------------------------------------------------------
	Args:
	  parent	- mama widge
	  spectro	- current waveform
	  index		- label to create (-1) or edit (index into labels)
	
	Called from:
	  dobutton in xkl.c
	  HandleFileMenu in xkl.c
	  HandleEditLabels in xkl_dlog.c
	
	Side effects:
	  opens dialog 
	
	Returns:
	  nada
  ===========================================================================
*/

void
InitLabel(
	Widget parent,
	XSPECTRO *spectro,
	int index)
{
	LABEL *lp;
	Arg a[10];
	XmString Xstring[10];
	Widget labForm, comForm, w[3], b[3];
	char s[100];
	int n, sc=0;
	void HandleMapping();	// xkl_dlog.c

// create dialog if necessary

	if (gEditLabelD != NULL) {				// dialog already active
		XRaiseWindow(XtDisplay(gEditLabelD), XtWindow(gEditLabelD));
		XSetInputFocus(XtDisplay(gEditLabelD), XtWindow(gEditLabelD), RevertToParent, CurrentTime);
		return;
	}
	
	n = 0;
	XtSetArg(a[n], XmNdialogTitle, XpMakeXstring("Set Label...")); n++;
//	XtSetArg(a[n], XmNdialogStyle,		XmDIALOG_FULL_APPLICATION_MODAL); n++;	// some WMs ignore this
	XtSetArg(a[n], XmNautoUnmanage,			FALSE); n++;
	XtSetArg(a[n], XmNnoResize,				TRUE); n++;
	XtSetArg(a[n], XmNdefaultPosition,		FALSE); n++;
	XtSetArg(a[n], XmNhorizontalSpacing,	10); n++;
	XtSetArg(a[n], XmNverticalSpacing,		10); n++;
	gEditLabelD = XmCreateFormDialog(parent, "", a, n);
	XtAddCallback(gEditLabelD, XmNmapCallback, (XtCallbackProc)HandleMapping, 0);

// find existing label, if any
	if (index < 0)
		lp = NULL;
	else
		for (n=0,lp=spectro->labels; n<index && lp; n++,lp=lp->next) ;

// label form
	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			15); n++;
	XtSetArg(a[n], XmNrightOffset,			15); n++;
	XtSetArg(a[n], XmNtraversalOn,			TRUE); n++;
	labForm = XmCreateForm(gEditLabelD, 0, a, n);

// label offset field
	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			10); n++;
	XtSetArg(a[n], XmNcolumns,				10); n++;
	XtSetArg(a[n], XmNmaxLength,			10); n++;
	sprintf(s, "%.2f", (lp ? lp->offset : spectro->savetime));
	XtSetArg(a[n], XmNvalue, s); n++;
	gLabFields[lfOffset] = w[0] = (Widget)XmCreateTextField(labForm, 0, a, n);

// label msecs label
	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
	XtSetArg(a[n], XmNleftOffset,			10); n++;
	XtSetArg(a[n], XmNlabelString, 		XpMakeXstring("msecs:")); n++;
	w[1] = (Widget)XmCreateLabelGadget(labForm, 0, a, n);
	
// label name field
	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 			w[1]); n++;
	XtSetArg(a[n], XmNrightAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			10); n++;
	XtSetArg(a[n], XmNrightOffset,			10); n++;
	XtSetArg(a[n], XmNcolumns,				20); n++;
	XtSetArg(a[n], XmNmaxLength,			MAXLABLEN-1); n++;
	XtSetArg(a[n], XmNvalue, (lp ? lp->name : "")); n++;
	gLabFields[lfName] = w[2] = (Widget)XmCreateTextField(labForm, 0, a, n);
	XtManageChildren(w, 3);		
	XtManageChild(labForm);

// comment form
	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 			labForm); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			15); n++;
	XtSetArg(a[n], XmNrightOffset,			10); n++;
	XtSetArg(a[n], XmNtraversalOn,			TRUE); n++;
	comForm = XmCreateForm(gEditLabelD, 0, a, n);

// comment label
	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			10); n++;
	XtSetArg(a[n], XmNlabelString, XpMakeXstring("Comment:")); n++;
	w[0] = (Widget)XmCreateLabelGadget(comForm, 0, a, n);
	
// comment field
	n = 0;
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNbottomAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 			w[0]); n++;
	XtSetArg(a[n], XmNrightAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			10); n++;
	XtSetArg(a[n], XmNrightOffset,			15); n++;
	XtSetArg(a[n], XmNmaxLength,			MAXLABCOM-1); n++;
	if (lp) {
		XtSetArg(a[n], XmNvalue, lp->comment); n++;
	}
	gLabFields[lfComment] = w[1] = (Widget)XmCreateTextField(comForm, 0, a, n);
	XtManageChildren(w, 2);		
	XtManageChild(comForm);

// buttons
	n = 0;			// separator
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 			comForm); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNrightAttachment, 		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNtopOffset,			15); n++;
	b[0] = XmCreateSeparatorGadget(gEditLabelD, 0, a, n);
	
	n = 0;			// OK	
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 			b[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNbottomAttachment, 	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			90); n++;
	XtSetArg(a[n], XmNwidth, 				70); n++;
	XtSetArg(a[n], XmNheight,				30); n++;
	XtSetArg(a[n], XmNuserData,				index); n++;		// pass label index
	XtSetArg(a[n], XmNlabelString, XpMakeXstring("OK")); n++;
	b[1] = XmCreatePushButtonGadget(gEditLabelD, 0, a, n);
	XtAddCallback(b[1], XmNactivateCallback, (XtCallbackProc)HandleEditLabel, (XtPointer)spectro);
	
	n = 0;			// Cancel	
	XtSetArg(a[n], XmNtopAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNtopWidget, 			b[0]); n++;
	XtSetArg(a[n], XmNleftAttachment,		XmATTACH_WIDGET); n++;
	XtSetArg(a[n], XmNleftWidget, 			b[1]); n++;
	XtSetArg(a[n], XmNbottomAttachment, 	XmATTACH_FORM); n++;
	XtSetArg(a[n], XmNleftOffset,			30); n++;
	XtSetArg(a[n], XmNwidth, 				70); n++;
	XtSetArg(a[n], XmNheight,				30); n++;
	XtSetArg(a[n], XmNlabelString, XpMakeXstring("Cancel")); n++;
	b[2] = XmCreatePushButtonGadget(gEditLabelD, 0, a, n);
	XtAddCallback(b[2], XmNactivateCallback, (XtCallbackProc)HandleEditLabel, NULL);

// manage everything
	XtManageChildren(b, 3);

	XtVaSetValues(gEditLabelD, XmNdefaultButton, b[1], NULL);
	
	XtManageChild(gEditLabelD);
	
	if (lp) XmTextFieldSetSelection(gLabFields[lfName], 0, strlen(lp->name), CurrentTime);		// highlight name
	XmProcessTraversal(gLabFields[lfName], XmTRAVERSE_CURRENT);									// select name field
		
	for (n=0; n<sc; n++)				// deallocate XmStrings
		XmStringFree(Xstring[n]);

} /* InitLabel */

