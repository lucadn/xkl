/** Header file for web.c **/

/* Procedure declarations: */
void DrawSpectrum(XSPECTRO *, FILE *);
void DrawSpectrogram(XSPECTRO *, FILE *);

/* Initializations: */
void SetupXklEnvironment();
void AllocateNewSpectro (XSPECTRO *, char *);

/* File Converters: */
void ConvertPStoJPEG(String, String);
