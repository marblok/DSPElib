function test_audio_input

plik=fopen('../outputs/AudioIn.out', 'rb');
x=fread(plik, inf,'float');
fclose(plik)
plik=fopen('../outputs/AudioIn2.out', 'rb');
y=fread(plik, inf,'float');
fclose(plik)

plik=fopen('../outputs/AudioIn2.raw', 'wb');
fwrite(plik, y*(2.^15),'int16');
fclose(plik)

Buffer_in=1024*4;
Buffer_out=1024*4; %64;

Buffer=Buffer_out;


x=x(2*Buffer:end);
y=y(1:(end-2*Buffer));

figure(1)
subplot(2,1,1)
plot(x)
subplot(2,1,2)
plot(y)
% y2=angle(exp(j*y));
% hold on
% plot(y2, 'r')
% hold off

figure(2)
subplot(2,1,1)
psd(x)
subplot(2,1,2)
psd(y)

figure(3)
subplot(2,1,1)
specgram(x, 512, 96000, blackman(512), 240)
set(gca, 'Clim', [-100, 0]);
colorbar 

subplot(2,1,2)
% % specgram(round(y*(2.^8)))
% y(abs(y)>0.2) = sign(y(abs(y)>0.2))*0.2;
specgram(y, 512, 96000, blackman(512), 240)
set(gca, 'Clim', [-100, 0]);
colorbar 
