function mat2kl(wave, fname, Fs)

%MAT2KL(WAVE, FNAME, FS)
%
%MAT2KL	Saves wave into a .wav (MIT speech group format) as fname.wav 
%       (which can be read by xkl). 
%
%	The sample values are saved as 16 bit integers, and the vector x is
%	scaled to fit within the range -2^15+1 to 2^15.

%	WAVE: waveform (vector of samples)
%	FNAME: filename of .wav file
%	FS: sampling frequency

%
% $Log: mat2kl.m,v $
% Revision 1.7  2002/08/07 17:13:11  xkldev
% fsstr is set using int2str, instead of sprintf
%
% Revision 1.6  2002/06/19 17:47:13  hanson
% hmh added line to report if a signal has been scaled.
%
% Revision 1.5  1998/07/16 23:29:54  krishna
% Added argument checking, and added the help info.
%
% Revision 1.4  1998/06/16 23:13:26  krishna
% Changed tmp filename, and rawtowav to raw2kl.
%
% Revision 1.3  1998/06/06 22:20:25  krishna
% Added RCs header, and changed name from mat2wav to mat2kl.
%

if (nargin ~= 3)
  error(' Usage: mat2kl(wave, fname, Fs)');
end

maxx = max(wave);
minx = min(wave);
scale = max(abs(maxx),abs(minx))/(2^15-1);
if (maxx>2^15)|(minx<-2^15+1)
	wave = wave/scale;
	fprintf(1, 'Waveform %s has been scaled to prevent clipping\n',fname)
end

fid = fopen('tmp.mat2kl', 'w');
fwrite(fid, wave, 'int16');
fclose(fid);

fsstr=int2str(Fs);

unix(['raw2kl -f ' fsstr ' tmp.mat2kl ' fname]);
unix(['/bin/rm tmp.mat2kl']);


