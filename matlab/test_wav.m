function test_wav

filename = 'P:\Sygnaly_na_wyjsciu_nadajnika\13_1700_Hz_scrambled_H.wav';
[Y,FS,NBITS]=wavread(filename, 1000)

plik = fopen(filename, 'rb');
header =  fread(plik, 44, 'uint8');
x = fread(plik, 1000000, 'float');
fclose(plik);

figure(1)
plot(x)
