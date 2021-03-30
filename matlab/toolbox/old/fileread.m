function [x, Fs] = fileread(filename, param)
% [x, Fs] = fileread(filename, param)
%
% returns vector x of the size SIZE=[samples channels].
%  eg. x = x(:,1) + j*x(:,2);
%
% special uses:
%  param == 'size': returns SIZE in x
%  param == 'cplx': for stereo files returns x as a complex vector instead of matrix 
%
%  supported file types:
%    *.flt
%    *.wav
%    *.tape
% last modification: 2021.03.29
% Author: Marek Blok

return_cplx = 0;
if nargin == 1,
  param = inf;
else
  if strcmp(param, 'cplx') == 1,
    return_cplx = 1;
    param = inf;
  end
end

ind = find(filename == '.');
if length(ind) == 0,
  file_type = 'w'; % *.wav
else
  ind = ind(end);
  
  temp = filename(ind+1:end);
  
  file_type = 'u'; % unknown format
  if strcmp(temp, 'flt') == 1,
    file_type = 'f'; % two channel floting point
  elseif strcmp(temp, 'wav') == 1,
    file_type = 'w'; % *.wav
  elseif strcmp(temp, 'tape') == 1,
    file_type = 't'; % *.tape
  end
end

switch file_type,
  case 'w',
    if strcmp(param, 'size') == 1,
      if exist('audioread','file') == 0
        x = wavread(filename, 'size');
        %  siz = [samples channels].
      else
        info = audioinfo(filename);
        x = [info.TotalSamples, info.NumChannels];
        Fs = info.SampleRate;
      end      
    else
      if isfinite(param)
        if exist('audioread','file') == 0
          [x, Fs] = wavread(filename, param);
        else
          [x, Fs] = audioread(filename, param);
        end
      else
        if exist('audioread','file') == 0
          [x, Fs] = wavread(filename);
        else
          [x, Fs] = audioread(filename);
        end
      end
    end
    
  case 't'
    plik = fopen(filename, 'rb');
    if plik == -1,
      error('File does not exist !!!');
    end
    
    header.size = fread(plik, 1, 'uint32', 0) + 4;

    header.fname      = char(fread(plik, 256, 'char', 0).');
    header.cfg_fname  = char(fread(plik, 256, 'char', 0).');
    header.sw_rev     = fread(plik, 1, 'uint32', 0);
    header.hw_rev     = fread(plik, 1, 'uint32', 0);
    header.file_      = fread(plik, 1, 'uint32', 0);
    header.tape_type  = fread(plik, 1, 'uint32', 0);
    header.start_time = fread(plik, 1, 'int32', 0); % time_t
    header.end_time   = fread(plik, 1, 'int32', 0); % time_t
    
    header.total_samples  = fread(plik, 1, 'uint32', 0); 
    file_length = header.total_samples * 4  + header.size
    header.current_sample = fread(plik, 1, 'uint32', 0); 
    header.loop_start     = fread(plik, 1, 'int64', 0); 
    header.loop_end       = fread(plik, 1, 'int64', 0); 
    header.loop           = fread(plik, 1, 'int32', 0); 
    header.group_size_32  = fread(plik, 1, 'uint32', 0); 
    header.block_size     = fread(plik, 1, 'uint32', 0); 
    header.block_count    = fread(plik, 1, 'uint32', 0); 
    header.fifo_size      = fread(plik, 1, 'uint32', 0); 


    header.comment      = char(fread(plik, 256, 'char', 0).');
    header.tmp  = char(fread(plik, 20, 'char', 0).'); % time_t
    header.status       = fread(plik, 1, 'uint32', 0); 
    header.timestamps   = fread(plik, 1, 'int32', 0); 
    header.freq         = fread(plik, 1, 'float', 0); 
    header.cplx_datarate = fread(plik, 1, 'float', 0); 
    
%     ftell(plik)
    header.reserved = fread(plik, 128, 'uint32', 0);
%     header.reserved.'
    
    header
    ftell(plik)

    header.sample_type = 2;
    header.ch_no = 2;
    header.Fs = NaN;
    
    sample_type = 'int16';
    sample_size = 2;

    header_size = header.size;
    if strcmp(param, 'size') == 1,
      fseek(plik, 0, 'eof');
      size = (ftell(plik) - header_size) / sample_size / header.ch_no; % sizeof(float) *2
      x = size;
    else
      fseek(plik, header_size, 'bof');
      
      len = param(1);
      if length(param) > 1,
        fseek(plik, sample_size*header.ch_no*(param(1)-1), 'cof');
        len = param(2) - param(1) + 1;
      end
        
%       x = fread(plik, [header.ch_no, len], sample_type);
      x = fread(plik, [header.ch_no, len], sample_type, 0);
      x = x.';
    end
    fclose(plik);
    
  case 'f'
    plik = fopen(filename, 'rb');
    if plik == -1,
      error('File does not exist !!!');
    end
    
    % 3 B - wersja pliku (w tym typ próbek)
    % 1 B - liczba kana³ów
    % 4 B - szybkoœæ próbkowania
    header_size = 8;
    header.ver = fread(plik, 1, 'uint8');
    header.sample_type = fread(plik, 1, 'uint16');
    header.ch_no = fread(plik, 1, 'uint8');
    header.Fs = fread(plik, 1, 'uint32');
    
    Fs = header.Fs;
    
    switch (header.ver),
      case 0,
        switch (header.sample_type),
          case 0,
            sample_type = 'float';
            sample_size = 4;
          case 1,
            sample_type = 'uchar';
            sample_size = 1;
          case 2,
            sample_type = 'short';
            sample_size = 2;
          case 3,
            sample_type = 'int';
            sample_size = 4;
          otherwise
            error('Unsupported *.flt sample type !!!');
        end
      otherwise
        error('Unsupported *.flt file version !!!');
    end
        
    
    if strcmp(param, 'size') == 1,
      fseek(plik, 0, 'eof');
      size = (ftell(plik) - header_size) / sample_size / header.ch_no; % sizeof(float) *2
      x = size;
    else
      len = param(1);
      status = 0;
      if length(param) > 1,
        status = fseek(plik, sample_size*header.ch_no*(param(1)-1), 'cof');
        len = param(2) - param(1) + 1;
      end
        
      if (status == -1)
        x = [];
      else
        x = fread(plik, [header.ch_no, len], sample_type);
        x = x.';
      end
    end
    
    fclose(plik);
  otherwise
    error('Unsupported file format !!!');
end

if return_cplx == 1,
  if length(x(1,:)) == 2,
    x = x(:,1) + j*x(:,2);
  end
end