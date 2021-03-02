function testDCO
% N=100000;
% n=0:N-1;
% fo=15800+23; fp=48000;
% s=sin(2*pi*fo/fp*n);
% wavwrite(s,fp,16,'test.wav') 

plik=fopen('MainHeter.out', 'rb');
x=fread(plik,inf,'float');
x=x(1:2:end)+j*x(2:2:end);
fclose(plik);

plik=fopen('FileIn.out', 'rb');
y=fread(plik,inf,'float');
fclose(plik);

plik=fopen('LPF_Hilbert.out', 'rb');
z2=fread(plik,inf,'float');
z2=z2(1:2:end)+j*z2(2:2:end);
fclose(plik);

% plik=fopen('MainDecimator.out', 'rb');
plik=fopen('DCO_Main_Mul.out', 'rb');
z3=fread(plik,inf,'float');
z3=z3(1:2:end)+j*z3(2:2:end);
fclose(plik);

plik=fopen('pilot_heter_Mul.out', 'rb');
z4=fread(plik,inf,'float');
z4=z4(1:2:end)+j*z4(2:2:end);
fclose(plik);

fp=48000;
figure(1)
y=y+j*ones(size(y))/10000;
[Pyy,F]=psd(y,2048);
[Pxx,F]=psd(x,2048);
[Pzz,F]=psd(z3,2048);
[Pzz2,F]=psd(z2,2048);
[Pzz4,F]=psd(z4,2048);
f=linspace(-fp/2,fp/2,2049); f(end)=[];
Pxx=fftshift(Pxx);
Pyy=fftshift(Pyy);
Pzz=fftshift(Pzz);
Pzz2=fftshift(Pzz2);
Pzz4=fftshift(Pzz4);
plot(f,10*log10(Pxx));
hold on
plot(f,10*log10(Pyy),'r');
plot(f,10*log10(Pzz),'m');
% set(plot(f,10*log10(Pzz),'m'), 'linewidth', 3);
plot(f,10*log10(Pzz2),'g');
plot(f,10*log10(Pzz4),'k');
hold off


plik=fopen('PilotDecimator.out', 'rb');
x=fread(plik,inf,'float');
x=x(1:2:end)+j*x(2:2:end);
fclose(plik);

plik=fopen('PilotDCO.out', 'rb');
y=fread(plik,inf,'float');
y=y(1:2:end)+j*y(2:2:end);
fclose(plik);

figure(2)
plot(real(x))
hold on
plot(imag(y),'r')
hold off
y=y+j*rand(size(y))/10000;
pause
[Pxx,F]=psd(x,1024); fp=4800;
[Pyy,F]=psd(y,1024); %fp=1;
f=linspace(-fp/2,fp/2,1025); f(end)=[];
Pyy=fftshift(Pyy);
Pxx=fftshift(Pxx);
plot(f,10*log10(Pxx));
hold on 
plot(f,10*log10(Pyy),'r');
hold off



plik=fopen('phase_err.out', 'rb');
x=fread(plik,inf,'float');
fclose(plik);

plik=fopen('Freq_error_zeroins.out', 'rb');
z=fread(plik,inf,'float');
fclose(plik);

plik=fopen('freq_err.out', 'rb');
y=fread(plik,inf,'float');
fclose(plik);

z2=filter(5*(1-0.99),[1, -0.99],z)
figure(3)
plot(x)
hold on
%set(plot(z,'g'), 'linewidth', 2)
set(plot(z2,'m'), 'linewidth', 1)
plot(y,'r')
hold off


figure(4)
fp=fp/5; K=1024;
for ind=1:12,
  tekst=sprintf('subchannels_%02i.out', ind-1);
  plik=fopen(tekst, 'rb');
  u=fread(plik,inf,'float');
  u=u(1:2:end)+j*u(2:2:end);
  fclose(plik);
  
  [Pu,F]=psd(u,K);
  f=linspace(-fp/2,fp/2,K+1); f(end)=[];
  Pu=fftshift(Pu);

  [Pua,F]=psd(abs(u)+j*rand(size(u))/100000,K);
  Pua=fftshift(Pua);
  
  subplot(2,1,1)
  plot(real(u));
  hold on
  plot(imag(u),'r');
  plot(abs(u),'k');
  hold off
  subplot(2,1,2)
  plot(f,10*log10(Pua),'r');
  hold on
  plot(f,10*log10(Pu));
  hold off
  pause
  
% %  testSTR_b(u,L, M);
 testSTR_b(u,8, 8);


  tekst=sprintf('subchannels_gardner_%02i.out', ind-1);
  plik=fopen(tekst, 'rb');
  u2=fread(plik,inf,'float');
  u2=u2(1:2:end)+j*u2(2:2:end);
  fclose(plik);
  tekst=sprintf('subchannels_diff_%02i.out', ind-1);
  plik=fopen(tekst, 'rb');
  u3=fread(plik,inf,'float');
  u3=u3(1:2:end)+j*u3(2:2:end);
  fclose(plik);
  
  n=(0:length(u)-1)/(8.5);
  n2=0.5+ (0:length(u2)-1);
figure(5)
subplot(2,1,1)
plot(n,real(u), 'bx');
hold on
plot(n2,real(u2), 'r.');
hold off
subplot(2,1,2)
plot(n,imag(u), 'bx');
hold on
plot(n2,imag(u2), 'r.');
hold off
  pause
subplot(1,2,1)
% u2=u2(20:end);
plot(u2,'ro')
axis equal
% pause
subplot(1,2,2)
plot(u3,'bo')
axis equal
pause
% hp=plot(u(1:20), 'ro');
% hold on
% axis equal
% set(gca,'Xlim',[-2, 2]);
% set(gca,'ylim',[-1.5, 1.5]);
% for ind=5:5:length(u)-20,
%   
%   hp2=plot(u(ind+[0:19]), 'ro');
%   delete(hp)
%   hp=hp2;
%   pause(0.1);
% end
% hold off
% axis equal
  pause
end
