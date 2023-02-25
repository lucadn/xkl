.TH MAT2KL 1 "XKL Utilities (March 1, 1999)" "MIT Speech Group" \" -*- nroff -*-
.SH NAME
mat2kl.m \- write a Matlab waveform into a Klatt (.wav)
.SH SYNOPSIS
.B mat2kl(filename, wav, Fs)
.SH DESCRIPTION
.PP
.B mat2kl.m
is a Matlab script that will write the waveform, wav, at a sampling
frequency, Fs, in Hz, into a a Klatt (.wav) file, filename.

.SS OPTIONS
.TP

.SH EXAMPLES
matlab> mat2kl('ba.wav', wave, 16000);

.SH AUTHOR
Send bugs or comments to xkldev@speech.mit.edu.

.SH SEE ALSO
.BR concat (1),
.BR klinfo (1),
.BR klgain (1),
.BR klplay (1),
.BR klrecord (1),
.BR kl2raw (1),
.BR raw2kl (1),
.BR kl2sd (1),
.BR sd2kl (1),
.BR kl2mac (1),
.BR mac2kl (1),
.BR kl2ms (1),
.BR ms2kl (1),
.BR kl2mat (1)

