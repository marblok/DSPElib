function h=multirate_filters

%decimation filter 22050 -> 8000
Fp1 = 22050; Fp2 = 8000;
M = 441; L = 160;
N=160*21;
n = -N/2:(N/2-1);

wind = hann(N).';
wind = wind / sum(wind);
h = sinc(pi*3000/(L*Fp1)*n);
h = 4.48*h.*wind;

figure(1)
plot(h)
pause

figure(2)
freqz(h,1, 8*2048, L*Fp1)


dane.h  = L*h;
dane.Fp = Fp1;
save_filter_coef('LPF_22050_8000', dane);

