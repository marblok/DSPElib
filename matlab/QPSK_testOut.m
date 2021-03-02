function testOut

dir_name='../outputs';
N=1000;

% ind=1;
% tekst=sprintf('./test/%s/subchannels_gardner_%02i.out', plik_name, ind-1);

% tekst=sprintf('./test/%s/FileIn.out', plik_name); % OK
tekst=sprintf('%s/subchannels_gardner_00.out', dir_name); % OK
plik=fopen(tekst, 'rb');
x=fread(plik,N,'float');
%u2=u2(1:2:end)+j*u2(2:2:end);
fclose(plik);

figure(1)
plot(x)