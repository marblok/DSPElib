function h=socket_filters

%interpolation filter 8000 -> 48000
Fp1 = 8000; Fp2 = 48000;
M = 1; L = 6;
N=L*21;
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
save_filter_coef('LPF_8000_48000', dane);



%interpolation filter 8000 -> 11025
Fp1 = 8000; Fp2 = 11025;
M = 320; L = 441;
N=L*21;
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
save_filter_coef('LPF_8000_11025', dane);

