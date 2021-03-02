function [coefficients, file_version] = load_filter_coef(filename) 
% [coefficients, file_version] = load_filter_coef(filename) 
% 
% FIR filter coefficients
%   coefficients.h
%    - can be cell vector to store multiple FIR impulse responces
% For file_version >= 1 (default == 0),
%   coefficients.Fp - sampling frequency
%
% This file is a part of Digital Signal Processing Engine
% \author Marek Blok
% \date 2008.03.28

% /*! File format (*.coef) - this is open format, for general use
%  *  (not only for storing coefficients)
%  *
ind = find(filename == '.');
if length(ind) == 0,
  filename = [filename, '.coef'];
end
plik = fopen(filename, 'rb');

%  *  - (uchar) 1B - file version number
file_version = fread(plik, 1, 'uchar')

switch file_version,
  case 0,
  case 1,
    %  *  - (uint) 4B - Sampling frequency
    coefficients.Fp = fread(plik, 1, 'uint32')
  otherwise,
    fclose(plik);
    error('This version of coefficients file is unsupported');
end

%  *  -  data - coefficients data (depends on fle version)
%  *  .
%  *  Data segment format:
%  *  -# (version: 0x00)
%  *   -  (uchar) 1B - number of sample dimensions
%  *         1 - real, 2 - complex, ...
no_of_channels = fread(plik, 1, 'uchar'); % complex
if no_of_channels == 1,
  isComplex = 0;
elseif no_of_channels == 2,
  isComplex = 1;
else
  fclose(plik);
  error('to many channels');
end
%  *   -  (uchar) 1B - sample component type
%  *    - DSP_FT_float (=1) : C++ float (32bit floating point)
%  *    - DSP_FT_short (=2) : C++ short (16bit signed integer)
%  *    - DSP_FT_uchar (=3) : C++ unsigned char (8bit unsigned integer with bias (0x80))
%  *    - DSP_FT_double (=7) : C++ double (64bit floating point)
%  *    - DSP_FT_long_double (=8) : C++ long double (80bit floating point)
sample_type = fread(plik, 1, 'uchar');
switch sample_type,
  case 1, 
    sample_type = 'float';
  case 2, 
    sample_type = 'short';
  case 3, 
    sample_type = 'uchar';
  case 7, 
    sample_type = 'double';
  case 8, 
    sample_type = 'float80';
  otherwise
  fclose(plik);
  error('unknown sample type');
end

%  *   -  (uchar) 1B - number of vectors
%  *    -   1 - FIR filter coefficients (one vector)
%  *    -   2 - IIR filter coefficients (two vectors)
%  *   -  (x number of vectors)
%  *    -   (ushort) 2B - number of samples in vector
%  *    -   (x number of samples)
%  *      -   (x number of sample dimensions)
%  *       -    (sample componet type) xB - sample component
%  *               e.g. real, imag part
resp_no = fread(plik, 1, 'uchar');

if resp_no == 1,
  N_FIR = fread(plik, 1, 'uint16');
  if isComplex,
    dane = fread(plik, 2*N_FIR, sample_type);
    coefficients.h = dane(1:2:2*N_FIR) + j*dane(2:2:2*N_FIR);
  else
    dane = fread(plik, N_FIR, sample_type);
    coefficients.h = dane;
  end
else
  for ind_resp = 1:resp_no,
    N_FIR = fread(plik, 1, 'uint16');
    if isComplex,
      dane = fread(plik, 2*N_FIR, sample_type);
      coefficients.h{ind_resp} = dane(1:2:2*N_FIR) + j*dane(2:2:2*N_FIR);
    else
      dane = fread(plik, N_FIR, sample_type);
      coefficients.h{ind_resp} = dane;
    end
  end
end

fclose(plik);
