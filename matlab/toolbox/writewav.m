function writewav(y,wavefile,N_start,Fs, reset)
%WAVWRITE Write Microsoft WAVE (".wav") sound file.
%   WAVWRITE(Y,FS,NBITS,WAVEFILE) writes data Y to a Windows WAVE
%   file specified by the file name WAVEFILE, with a sample rate
%   of FS Hz and with NBITS number of bits.  NBITS must be 8 or 16.
%   Stereo data should be specified as a matrix with two columns.
%   Amplitude values outside the range [-1,+1] are clipped.
%
%   WAVWRITE(Y,FS,WAVEFILE) assumes NBITS=16 bits.
%   WAVWRITE(Y,WAVEFILE) assumes NBITS=16 bits and FS=8000 Hz.
%
%   See also WAVREAD, AUWRITE.

%   Copyright 1984-2000 The MathWorks, Inc. 
%   $Revision: 5.10 $  $Date: 2000/06/01 17:29:55 $

%   D. Orofino, 11/95
% Marek Blok 23.05.2002

%% Parse inputs:
%error(nargchk(2,5,nargin));
%if nargin < 3,
%  wavefile = Fs;
%  Fs       = 8000;
%  nbits    = 16;
%elseif nargin < 4,
%  wavefile = nbits;
%  nbits    = 16;
%end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% we assume that the data are 16bit mono wav file %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if (reset==1) | (exist(wavefile)==0),
  %even if file exists you should start from the begining
  delete(wavefile);

  %prepare and write new header
	% Determine number of bytes in chunks
	% (not including pad bytes, if needed):
	% ----------------------------------
	%  'RIFF'           4 bytes
	%  size             4 bytes (ulong)
	%  'WAVE'           4 bytes
	%  'fmt '           4 bytes
	%  size             4 bytes (ulong)
	% <wave-format>     14 bytes
	% <format_specific> 2 bytes (PCM)
	%  'data'           4 bytes
	%  size             4 bytes (ulong)
	% <wave-data>       N bytes
	% ----------------------------------

  %empty data
	total_samples    = 0; %samples; % * channels; but: channels=1;
	total_bytes      = total_samples * 2 %* bytes_per_sample; but: bytes_per_sample = 2; %16bit wav file
	
	riff_cksize = 36+total_bytes;   % Don't include 'RIFF' or its size field
	fmt_cksize  = 16;               % Don't include 'fmt ' or its size field
	data_cksize = total_bytes;      % Don't include 'data' or its size field
	
	% Determine pad bytes:
	%%16 bit file so there's no need of data padding
	%data_pad    = rem(data_cksize,2);
	%riff_cksize = riff_cksize + data_pad; % + fmt_pad, always 0
	
	% ----------------------------------
	% Open file for output:
	%% Open file, little-endian:
	[fid,err] = fopen(wavefile,'wb','l');
	error(err);
	
	% Prepare basic chunk structure fields:
	% ----------------------------------
	% Write RIFF chunk:
	fwrite(fid, 'RIFF', 'char'); %pos: 0B, len: 4B
	fwrite(fid, riff_cksize, 'ulong'); %pos: 4B, len: 4B
	
	% Write WAVE subchunk:
	fwrite(fid, 'WAVE', 'char'); %pos: 8B, len: 4B
	% Indicate a subchunk (no chunk size)
	
	% Write <fmt-ck>:
	fwrite(fid, 'fmt ', 'char'); %pos: 12B, len: 4B
	fwrite(fid, fmt_cksize, 'ulong'); %pos: 16B, len: 4B
	
	% ----------------------------------
	% Write <wave-format>:
  fmt.filename        = wavefile;
	fmt.wFormatTag      = 1;            % Data encoding format (1=PCM)
	fmt.nChannels       = 1;            % Number of channels
	fmt.nSamplesPerSec  = Fs;           % Samples per second
	fmt.nAvgBytesPerSec = 1*2*Fs; % Avg transfer rate
	fmt.nBlockAlign     = 1*2;    % Block alignment
	fmt.nBitsPerSample  = 16;        % standard <PCM-format-specific> info
	
	% Create <wave-format> data:
	fwrite(fid, fmt.wFormatTag, 'ushort'); %pos: 20B, len: 2B
	fwrite(fid, fmt.nChannels,  'ushort'); %pos: 22B, len: 2B
	fwrite(fid, fmt.nSamplesPerSec, 'ulong' ); %pos: 24B, len: 4B
	fwrite(fid, fmt.nAvgBytesPerSec, 'ulong' ); %pos: 28B, len: 4B
	fwrite(fid, fmt.nBlockAlign, 'ushort'); %pos: 32B, len: 2B
	
	% Write format-specific info:
	% Write standard <PCM-format-specific> info:
	fwrite(fid, fmt.nBitsPerSample, 'ushort'); %pos: 34B, len: 2B
      
	
	% ----------------------------------
	% Write <data-ck>:
	fwrite(fid, 'data', 'char'); %pos: 36B, len: 4B
	fwrite(fid, data_cksize, 'ulong'); %pos: 40B, len: 4B
	% ------------------%
	% header end == 44B %
	% ------------------%

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Close file:
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  fclose(fid);
end

%%when resert==0
%??? read file header
[fid,err] = fopen(wavefile,'rb','l');
error(err);
fseek(fid, 4, 'bof');
riff_cksize=fread(fid, 1, 'ulong'); %pos: 4B, len: 4B
fseek(fid, 40, 'bof');
data_cksize=fread(fid, 1, 'ulong'); %pos: 40B, len: 4B

total_bytes      = data_cksize;
total_samples    = total_bytes/2;
fclose(fid);

%insert data
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% now we can write the file %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%[fid,err] = fopen(wavefile,'ab','l');
[fid,err] = fopen(wavefile,'r+','l');
%riff_cksize=fread(fid, 44, 'uchar') %pos: 4B, len: 4B
fseek(fid, 0, 'eof');

% If input is a vector, force it to be a column:
data = y(:);
samples=length(data);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Write <wave-data>, and its pad byte if needed:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
err = '';

data = PCM_Quantize(data);

dtype='short'; % signed 16-bit

% Write data, one row at a time (one sample from each channel):
% but only one channel
samples = length(data);
%total_samples = samples;

fwrite(fid, data, dtype);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%update header
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%whe files where just appended
total_samples    = total_samples+samples; % * channels; but: channels=1;
total_bytes      = total_samples * 2; %* bytes_per_sample; but: bytes_per_sample = 2; %16bit wav file
	
riff_cksize = 36+total_bytes;   % Don't include 'RIFF' or its size field
data_cksize = total_bytes;      % Don't include 'data' or its size field

fseek(fid, 4, 'bof');
fwrite(fid, riff_cksize, 'ulong'); %pos: 4B, len: 4B
fseek(fid, 40, 'bof');
fwrite(fid, data_cksize, 'ulong'); %pos: 40B, len: 4B


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Close file:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fclose(fid);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% end of writewav()
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% ------------------------------------------------------------------------
% Private functions:
% ------------------------------------------------------------------------

% -----------------------------------------------------------------------
function y = PCM_Quantize(x)
% PCM_Quantize:
%   Scale and quantize input data, from [-1, +1] range to
%   16-bit data range.

% Clip data to normalized range [-1,+1]:
ClipMsg  = ['Data clipped during write to file'];
ClipWarn = 0;

% Determine slope (m) and bias (b) for data scaling:
nbits = 16;
m = 2.^(nbits-1);

y = round(m .* x);

% Determine quantized data limits, based on the
% presumed input data limits of [-1, +1]:
qlim = m * [-1 +1];
qlim(2) = qlim(2)-1;

% Clip data to quantizer limits:
i = find(y < qlim(1));
if ~isempty(i),
   warning(ClipMsg); ClipWarn=1;
   y(i) = qlim(1);
end

i = find(y > qlim(2));
if ~isempty(i),
   if ~ClipWarn, warning(ClipMsg); end
   y(i) = qlim(2);
end

return


