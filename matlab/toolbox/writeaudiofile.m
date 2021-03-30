function writeaudiofile(filename, x, sample_type, Fs)

if nargin == 2,
  sample_type = 'float';
  Fs= NaN;
elseif nargin == 3,
  Fs= NaN;
end

ind = find(filename == '.');
if length(ind) == 0,
  filename = [filename, '.flt'];
end

plik = fopen(filename, 'wb');
if plik == -1,
  error
end

% 3 B - wersja pliku (w tym typ próbek)
% 1 B - liczba kana³ów
% 4 B - szybkoœæ próbkowania

header.ver = 0;
switch (sample_type),
  case 'float';
    header.sample_size = 4;
    header.sample_type = 0;
  case 'uchar';
    header.sample_size = 1;
    header.sample_type = 1;
  case 'short';
    header.sample_size = 2;
    header.sample_type = 2;
  case 'int';
    header.sample_size = 4;
    header.sample_type = 3;
  otherwise
    error('Unsupported *.flt sample type !!!');
end
pom = size(x);
if min(pom) == 0,
  error('empty data !!!');
elseif min(pom) == 1,
  header.ch_no = 1;
  x = x(:).';
  if any(imag(x) ~= 0),
    header.ch_no = 2;
    x_ = zeros(1, header.ch_no*length(x));
    x_(1:header.ch_no:end) =  real(x);
    x_(2:header.ch_no:end) =  imag(x);
    x = x_; clear x_;
  end  
elseif pom(1) < pom(2)
  header.ch_no = pom(1);
  x_ = zeros(1, pom(1)*pom(2));
  for ind=1:header.ch_no,
    x_(ind:header.ch_no:end) =  x(ind, :);
  end
  x = x_; clear x_;
else  
  header.ch_no = pom(2);
  
  x_ = zeros(1, pom(1)*pom(2));
  for ind=1:header.ch_no,
    x_(ind:header.ch_no:end) =  x(:, ind);
  end
  x = x_; clear x_;
end
size(x)

if ~isfinite(Fs), 
  header.Fs = 0; 
else
  header.Fs = Fs;
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% header
fwrite(plik, header.ver, 'ubit8');
fwrite(plik, header.sample_type, 'ubit16');
fwrite(plik, header.ch_no, 'uint8');
fwrite(plik, header.Fs, 'uint32');

fwrite(plik, x, sample_type);
fclose(plik);