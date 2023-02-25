function [y, Fs] = kl2mat(fname)

%[Y, FS] = KL2MAT(FNAME)
%
%KL2MAT     Read a .wav (Speech Group/xkl) file, specified by FNAME, 
%           and loads it into matlab via the variable Y.  FS is the 
%           sampling rate.
% 					

% $Log: kl2mat.m,v $
% Revision 1.5  1998/07/22 01:13:59  krishna
% Added ability to read in .wav files that have different header
% versions (versions 0, 1, 2).
%
% Revision 1.4  1998/07/16  23:29:09  krishna
% Added argument checking and prettified.
%
% Revision 1.3  1998/06/12 03:52:36  krishna
% Hmmm. Don't really know what I changed.
%
% Revision 1.2  1998/06/06 22:52:00  krishna
% Added RCS log.
%

if (nargin ~= 1)
  error('Specify filename.  Usage: kl2mat(fname).');
end

fid = fopen(fname, 'r');
if (fid == -1) 
  error(['File ' fname ' not readable.']);
end

%
% Get the header info
%

[usec, count] = fread(fid, 1, 'int');      % sampling period in microsecs.
[numSamples, count] = fread(fid, 1, 'int'); % number of samples
[version, count] = fread(fid, 1, 'int');    % .wav version

Fs=10^6/usec;  % sampling rate based on sampling period in microseconds

if (version == 1)
  [junk, count] = fread(fid, 125, 'int');  % read remainder of header
elseif (version == 2)
  [Fs, count] = fread(fid, 1, 'int');
  [junk, count] = fread(fid, 124, 'int');  % read remainder of header
else 
  [junk, count] = fread(fid, 121, 'int');  % read remainder of header
end

[y, count] = fread(fid, inf, 'int16');
fclose(fid);
