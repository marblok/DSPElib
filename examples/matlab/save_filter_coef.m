function save_filter_coef(filename, coefficients, file_version) 
% save_filter_coef(filename, coefficients [, file_version]) 
% 
% FIR filter coefficients
%   coefficients.h
%    - can be cell vector to store multiple FIR impulse responces
%    - if any of the impulse responses is complex then all responses will
%      be stored as complex
% IIR filter coefficients
%   coefficients.a
%   coefficients.b
% 
% For file_version >= 1 (default == 0),
%   coefficients.Fp - sampling frequency
%
% This file is a part of Digital Signal Processing Engine
% 
% File format (*.coef) - this is open format, for general use
%  *  (not only for storing coefficients)
%  
% File format (*.h) - this is C++ header file for DSPElib with hardcoded *.coef equivalent 
%
% \author Marek Blok
% \date 2020.02.22

if nargin == 2,
  file_version = 0;
end

ind = find(filename == '.');
if length(ind) == 0,
  filename = [filename, '.coef'];
end

% detect mode based on file extention
[PATHSTR,NAME,EXT] = fileparts(filename);
switch EXT
  case '.h'
    mode = 'h-file';
    plik = fopen(filename, 'wt');

  case '.coef'
    mode = 'coefs-file';
    plik = fopen(filename, 'wb');

  otherwise
    mode = 'unknown-file';
    error(mode);
end

switch mode
  case 'h-file'
    fprintf(plik, '// save_filter_coef output (hard coded coefficients)\n');
    fprintf(plik, '#pragma once\n');
    fprintf(plik, '#include <DSP_lib.h>\n');
    fprintf(plik, '\n');
    
    if isfield(coefficients, 'h'),
      h = coefficients.h;
      
      for ind_h = 0:length(h)-1
        fprintf(plik, 'DSP_float_vector Farrow_coefs_row_%i = {', ind_h);
        h_tmp = h{ind_h+1};
        for ind_p = 0:length(h_tmp)-1,
          if ind_p > 0
            fprintf(plik, ', ');
          end
          fprintf(plik, '%.15ff', h_tmp(ind_p+1));
        end
        %0.1f, 0.2f, 0.1f
        fprintf(plik, '};\n');
      end
      fprintf(plik, '\n');
      
      %vector <DSP_float_vector> Farrow_coefs = { coefs_0, coefs_1, coefs_2 };
      fprintf(plik, 'vector <DSP_float_vector> Farrow_coefs = {');
      for ind_h = 0:length(h)-1
        if ind_h > 0
          fprintf(plik, ', ');
        end
        if (ind_h < length(h_tmp)) & (rem(ind_h, 5) == 0)
          fprintf(plik, '\n    ');
        end
        fprintf(plik, 'Farrow_coefs_row_%i', ind_h);
      end
      fprintf(plik, '\n  };\n');
      
      fprintf(plik, '\n');
      fprintf(plik, 'const unsigned int p_Farrow = %i; // should be equal to Farrow_coefs[0].size()-1\n', length(h{1})-1);
      fprintf(plik, 'const unsigned int N_FSD = %i; // should be equal to Farrow_coefs.size()\n', length(h));
    end
   
    if isfield(coefficients, 'b'),
      fclose all;
      error('unsupported: coefficients.b');
    end
    if isfield(coefficients, 'a'),
      fclose all;
      error('unsupported: coefficients.a');
    end
    
  case 'coefs-file'
    %  *  - (uchar) 1B - file version number
    fwrite(plik, file_version, 'uchar');

    switch file_version,
      case 0,
      case 1,
        %  *  - (uint) 4B - Sampling frequency
        if isfield(coefficients, 'Fp'),
          fwrite(plik, coefficients.Fp, 'uint32');
        else
          fclose(plik);
          error('Input data does not contain Fp');
        end
      otherwise,
        fclose(plik);
        error('This version of coefficients file is unsupported');
    end

    if isfield(coefficients, 'h'),
      isFIR = 1;
      if iscell(coefficients.h)
        resp_no = length(coefficients.h);
    %     if resp_no = 1;
    %       coefficients.h = coefficients.h{1};
    %     end
      else
        resp_no = 1;
      end
      if resp_no == 1,
        isComplex = any(imag(coefficients.h(:)));
      else
        isComplex = false;
        for ind_resp = 1:resp_no,
          isComplex = isComplex | any(imag(coefficients.h{ind_resp}(:)));
        end
      end
    else
      isFIR = 0;
      isComplex = any(imag(coefficients.a(:))) | any(imag(coefficients.b(:)));
    end

    %  *  -  data - coefficients data (depends on fle version)
    %  *  .
    %  *  Data segment format:
    %  *  -# (version: 0x00)
    %  *   -  (uchar) 1B - number of sample dimensions
    %  *         1 - real, 2 - complex, ...
    if isComplex,
      fwrite(plik, 2, 'uchar'); % complex
    else
      fwrite(plik, 1, 'uchar'); % real
    end
    %  *   -  (uchar) 1B - sample component type
    %  *    - DSP::e::FileType::FT_float (=1) : C++ float (32bit floating point)
    %  *    - DSP::e::FileType::FT_short (=2) : C++ short (16bit signed integer)
    %  *    - DSP::e::FileType::FT_uchar (=3) : C++ unsigned char (8bit unsigned integer with bias (0x80))
    %  *    - DSP::e::FileType::FT_double (=7) : C++ double (64bit floating point)
    %  *    - DSP::e::FileType::FT_long_double (=8) : C++ long double (80bit floating point)
    fwrite(plik, 1, 'uchar');

    %  *   -  (uchar) 1B - number of vectors
    %  *    -   1 - FIR filter coefficients (one vector)
    %  *    -   2 - IIR filter coefficients (two vectors)
    %  *   -  (x number of vectors)
    %  *    -   (ushort) 2B - number of samples in vector
    %  *    -   (x number of samples)
    %  *      -   (x number of sample dimensions)
    %  *       -    (sample componet type) xB - sample component
    %  *               e.g. real, imag part
    if isFIR,
      fwrite(plik, resp_no, 'uchar');

      if iscell(coefficients.h)
        for ind_resp = 1:resp_no,
          N_FIR = length(coefficients.h{ind_resp});
          fwrite(plik, N_FIR, 'uint16');
          if isComplex,
            dane(1:2:2*N_FIR) = real(coefficients.h{ind_resp});
            dane(2:2:2*N_FIR) = imag(coefficients.h{ind_resp});
            fwrite(plik, dane, 'float');
          else
            fwrite(plik, real(coefficients.h{ind_resp}), 'float');
          end
        end
      else
        N_FIR = length(coefficients.h);
        fwrite(plik, N_FIR, 'uint16');
        if isComplex,
          dane(1:2:2*N_FIR) = real(coefficients.h);
          dane(2:2:2*N_FIR) = imag(coefficients.h);
          fwrite(plik, dane, 'float');
        else
          fwrite(plik, real(coefficients.h), 'float');
        end
      end

    else
      fwrite(plik, 2, 'uchar');

      N_a = length(coefficients.a);
      fwrite(plik, N_a, 'uint16');
      if isComplex,
        dane(1:2:2*N_a) = real(coefficients.a);
        dane(2:2:2*N_a) = imag(coefficients.a);
        fwrite(plik, dane, 'float');
      else
        fwrite(plik, real(coefficients.a), 'float');
      end


      N_b = length(coefficients.b);
      fwrite(plik, N_b, 'uint16');
      if isComplex,
        dane(1:2:2*N_b) = real(coefficients.b);
        dane(2:2:2*N_b) = imag(coefficients.b);
        fwrite(plik, dane, 'float');
      else
        fwrite(plik, real(coefficients.b), 'float');
      end
    end

end
fclose(plik);

