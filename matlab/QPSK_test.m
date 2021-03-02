% uwaga, przy stymacji po³o¿enia punktów konstelacji nale¿y
% pamiêtaæ, ¿ê punkty te mog¹ nie wystêpowaæ w badanej realizacji
%
% poza szacowaniem SNR 
% mo¿na oceniaæ dla ka¿dego punktu konstelacji z osobna
% 1) snr dla ka¿dego punktu konstelacji osobno - i na tej podstawie szacowaæ Pb
%     - przy ró¿nicach amplitudy ka¿dy punkt konstelacji bêdzi oferowa³ inny Pb
% 2) b³¹d amplitudy - tylko informacyjnie (tutaj wp³yw na Pb jest zawarty w SNR)
% 3) b³¹d fazy - niezale¿nie od SNR wp³ywa na Pb
%  - dodatkowy parametr do szacowania Pb
%  - równie¿ powinno pos³u¿yæ do szacowania obrotu konstelacji
% 
% Pb mo¿na albo szacowaæ w oparciu o najgorszy punkt konstelacji, albo 
%   wynliczaæ zak³adaj¹c równe prawdopodobieñstwa, albo wa¿¹c oszacowanymi 
%   prawdopodobieñstwami czyli czêstoœci¹
function test2
Czekaj=0;
warning off;

% plik_name='07-pcz.wav'; %czysty
% plik_name='10-pcz.wav';
% % plik_name='S5.wav';
% % plik_name='S7.wav';
% 
% % \bug BPSK powinien dostaæ wy¿sz¹ ocenê dla dwóch punktów konstelacji QPSK
% % tylko ma³a kara za obrót
% % \todo !!!!!!!!!!!!!!!!!!!!!!!!!!!!! dla pojedynczych punktów bardzo
% % odstaj¹cych od konstelacji zaznaczaæ punkty karne (jako bardzo
% % podejrzane) mo¿na próbkowaæ rekonstruowaæ
% % \test ???? sprawdziæ czy nie ma jakiegoœ prostego kodowania korekcyjnego 
% % dla kana³u radiowego tylkio bit parzystoœci to jednak za ma³o
% plik_name='03-pcz.wav'; %³adny sygna³ przed wyjœciem ró¿nicowym (segment_ind=10; - brak punktów konstelacji)
% plik_name='07-pcz.wav'; 
% % plik_name='S5.wav';
dir_name='../outputs';

% plik_name='05-pcz.wav'; % problem
% plik_name='06-pcz.wav'; % problem
% plik_name='08-pcz.wav'; % problem
% plik_name='S7.wav'; % problem
% plik_name='S10.wav'; % problem
% plik_name='S11.wav'; % problem

close all
N0=5000;
N=500


segment_ind=0;
% segment_ind=10;
while (1)
  N0=1+segment_ind*N;
  segment_ind=segment_ind+1;
  
	figure(1)
	for ind=1:12,
	%   tekst=sprintf('./test/dbg/subchannels_diff_%02i.out', ind-1);
      tekst=sprintf('%s/subchannels_gardner_%02i.out', dir_name, ind-1);
      plik=fopen(tekst, 'rb');
      fseek(plik,N0*2*4 ,'bof');
	if (ftell(plik) ~= N0*2*4)
      fclose(plik);
      error('fseek argument exceeds file size');
	end
      u2=fread(plik,N,'float');
      u2=u2(1:2:end)+j*u2(2:2:end);
      fclose(plik);
      tekst=sprintf('%s/subchannels_diff_%02i.out', dir_name, ind-1);
      plik=fopen(tekst, 'rb');
      fseek(plik,N0*2*4,'bof');
	if (ftell(plik) ~= N0*2*4)
      fclose(plik);
      error('fseek argument exceeds file size');
	end
      u3=fread(plik,N,'float');
      u3=u3(1:2:end)+j*u3(2:2:end);
      fclose(plik);
	
      
	% u2=u2(40:end);
	% u3=u3(40:end);
	
      figure(1)
      subplot(3,4,ind)
      plot(u2(1:2:end),'ro')
      hold on
      plot(u2(2:2:end),'bo')
      hold off
      set(gca, 'Ylim', [-2 2]);
      set(gca, 'Xlim', [-2 2]);
      
      figure(2)
      subplot(3,4,ind)
      plot(u2(1:2:end),'ro')
      hold on
      plot(u2(2:2:end).*exp(j*pi/4),'bo')
      hold off
      set(gca, 'Ylim', [-2 2]);
      set(gca, 'Xlim', [-2 2]);
      
      figure(3)
      subplot(3,4,ind)
      plot(u3,'bo')
      set(gca, 'Ylim', [-2 2]);
      set(gca, 'Xlim', [-2 2]);
      [BPSK_Mark(ind), SNR]=testSNR_BPSK(u3);  
	
      figure(4)
      subplot(3,4,ind)
      plot(u3,'bo')
      set(gca, 'Ylim', [-2 2]);
      set(gca, 'Xlim', [-2 2]);
      [QPSK_Mark(ind), SNR]=testSNR_QPSK(u3);  
      
      if sum(BPSK_Mark) > sum(QPSK_Mark),
        figure(3)
%         figure(1)
      end
	%   figure(1)
	% subplot(1,2,1)
	% % u2=u2(20:end);
	% plot(u2,'ro')
	% set(gca, 'Ylim', [-3 3]);
	% set(gca, 'Xlim', [-2 2]);
	% % axis equal
	% % pause
	% subplot(1,2,2)
	% plot(u3,'bo')
	% set(gca, 'Ylim', [-3 3]);
	% set(gca, 'Xlim', [-2 2]);
	% % axis equal
	% pause
	% % plot(u3-u2,'mo')
	% % pause
	% 
	end
	sprintf('%s: [%s%.1f]', 'BPSK_Mark', sprintf('%.1f, ', BPSK_Mark(1:end-1)), BPSK_Mark(end))
	sprintf('%s: [%s%.1f]', 'QPSK_Mark', sprintf('%.1f, ', QPSK_Mark(1:end-1)), QPSK_Mark(end))
  
  segment_ind-1
  N0
  if Czekaj == 1,
    pause
  else
    pause(0);
  end
end


function [QPSK_Mark, SNR]=testSNR_QPSK(sygn)

b1=real(sygn)>0;
b2=imag(sygn)>0;

ind00=find((b1==0) & (b2==0));
s=sygn(ind00); 
val00=mean(s);
N00=sqrt(mean(abs(s-val00).^2));

hold on
plot([s; NaN],'ro');
hold off

ind01=find((b1==0) & (b2==1));
s=sygn(ind01);
val01=mean(s);
N01=sqrt(mean(abs(s-val01).^2));

hold on
plot([s; NaN],'go');
hold off

ind10=find((b1==1) & (b2==0));
s=sygn(ind10);
val10=mean(s);
N10=sqrt(mean(abs(s-val10).^2));

hold on
plot([s; NaN],'co');
hold off

ind11=find((b1==1) & (b2==1));
s=sygn(ind11);
val11=mean(s);
N11=sqrt(mean(abs(s-val11).^2));

hold on
plot([s; NaN],'mo');
hold off


% S=mean(abs([val00, val01, val10, val11]).^2);
S=abs([val00, val01, val10, val11]).^2;
S=mean(S(isfinite(S)));
% N=mean([N00, N01, N10, N11].^2);
N=[N00, N01, N10, N11].^2;
N=mean(N(isfinite(N)));
SNR=10*log10(S/N);

hold on
 hp(1)=plot(val00, 'kx');
  plot(val00+N00*exp(j*pi/10*[0:20]),'k-');
  plot(val00+2*N00*exp(j*pi/10*[0:20]),'k-');
 hp(2)=plot(val01, 'kx');
  plot(val01+N01*exp(j*pi/10*[0:20]),'k-');
  plot(val01+2*N01*exp(j*pi/10*[0:20]),'k-');
 hp(3)=plot(val10, 'kx');
  plot(val10+N10*exp(j*pi/10*[0:20]),'k-');
  plot(val10+2*N10*exp(j*pi/10*[0:20]),'k-');
 hp(4)=plot(val11, 'kx');
  plot(val11+N11*exp(j*pi/10*[0:20]),'k-');
  plot(val11+2*N11*exp(j*pi/10*[0:20]),'k-');

%  plot((val00/sqrt(2)).^4, 'ko');
%  plot((val01/sqrt(2)).^4, 'ko');
%  plot((val10/sqrt(2)).^4, 'ko');
%  plot((val11/sqrt(2)).^4, 'ko');
hold off

set(hp, 'LineWidth', 3, 'MarkerSize', 14)

format compact
% temp1=[val00, ...
%       val01, ...
%       val10, ...
%       val11]

% temp=[val00.*exp(+j*3*pi/4), ...
%   val01.*exp(-j*3*pi/4), ...
%   val10.*exp(+j*pi/4), ...
%   val11.*exp(-j*pi/4)]
temp=[-real(val00)-imag(val00)+j*(+real(val00)-imag(val00)), ...
      -real(val01)+imag(val01)+j*(-real(val01)-imag(val01)), ...
      +real(val10)-imag(val10)+j*(+real(val10)+imag(val10)), ...
      +real(val11)+imag(val11)+j*(-real(val11)+imag(val11))];
% angle(temp)/pi
temp=imag(temp)./real(temp);
temp=temp(isfinite(temp));
phase_rot=mean(temp);
phase_dev=max(abs(temp-phase_rot));
% phase_err=asin(temp)
% temp1=abs(real(temp1))+j*abs(imag(temp1));
% 0.5*(imag(temp1)-real(temp1))./real(temp1)
QPSK_Mark=GetModulationMark(SNR,phase_dev,phase_rot);
% pause


function [BPSK_Mark, SNR]=testSNR_BPSK(sygn)

b1=real(sygn)>0;

ind0=find(b1==0);
s0=sygn(ind0);
val0=mean(s0);
N0=sqrt(mean(abs(s0-val0).^2));

ind1=find(b1==1);
s1=sygn(ind1);
val1=mean(s1);
N1=sqrt(mean(abs(s1-val1).^2));

s0=[s0; NaN]; s1=[s1; NaN];
hold on
plot(s0,'ro');
hold off

hold on
plot(s1,'co');
hold off


% S=mean(abs([val0, val1]).^2);
S=abs([val0, val1]).^2;
S=mean(S(isfinite(S)));
% N=mean([N0, N1].^2);
N=[N0, N1].^2;
N=mean(N(isfinite(N)));
SNR=10*log10(S/N);

hold on
 hp(1)=plot(val0, 'kx');
  plot(val0+N0*exp(j*pi/10*[0:20]),'k-');
  plot(val0+2*N0*exp(j*pi/10*[0:20]),'k-');
 hp(2)=plot(val1, 'kx');
  plot(val1+N1*exp(j*pi/10*[0:20]),'k-');
  plot(val1+2*N1*exp(j*pi/10*[0:20]),'k-');

%  plot((val00/sqrt(2)).^4, 'ko');
%  plot((val01/sqrt(2)).^4, 'ko');
%  plot((val10/sqrt(2)).^4, 'ko');
%  plot((val11/sqrt(2)).^4, 'ko');
hold off

set(hp, 'LineWidth', 3, 'MarkerSize', 14)

format compact
% temp1=[val0, ...
%        val1]
temp=[-val0, ...
       val1];
% angle(temp)/pi
temp=imag(temp)./real(temp);
temp=temp(isfinite(temp));
% temp1=abs(real(temp1))+j*abs(imag(temp1));
% % (imag(temp1)-real(temp1))./real(temp1) %QPSK only
% imag(temp1)./real(temp1) %BPSK only
phase_rot=mean(temp);
phase_dev=max(abs(temp-phase_rot));
% phase_err=asin(temp);
BPSK_Mark=GetModulationMark(SNR,phase_dev,phase_rot);
% pause

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function Mark=GetModulationMark(SNR, phase_dev, phase_rot)
% phase_dev  is actualy sin(phase_dev)
% the same is with phase_rot
% ewentualnie dla BPSK SNR=SNR+3 ???
Mark=0;
if SNR < -3,
  Mark_kor=-5;
else
  Mark_kor=-3;
  if SNR > 3;
    Mark_kor=Mark_kor+3+0.5; %0.5
  end  
  if SNR > 6;
    Mark_kor=Mark_kor+0.5; %1.0
  end  
  if SNR > 9;
    Mark_kor=Mark_kor+0.5; %1.5
  end  
  if SNR > 12,
    Mark_kor=Mark_kor+0.5; %2
  end  
end
Mark=Mark+Mark_kor;

Mark_kor=0;
phase_dev=abs(phase_dev);
if phase_dev > sin(pi/4);
  Mark_kor=-3;
else
  Mark_kor=-1.5;
  if phase_dev < sin(pi/8);
    Mark_kor=Mark_kor+1.5+0.5; %0.5
  end  
  if phase_dev < sin(pi/16);
    Mark_kor=Mark_kor+0.5; %1.0
  end  
  if phase_dev < sin(pi/32);
    Mark_kor=Mark_kor+0.5; %1.5
  end  
  if phase_dev < sin(pi/64),
    Mark_kor=Mark_kor+0.5; %2
  end  
end
Mark=Mark+Mark_kor;

Mark_kor=0;
phase_rot=abs(phase_rot);
if phase_rot < sin(pi/16);
  Mark_kor=Mark_kor+0.5; %0.5
end  
if phase_rot < sin(pi/32);
  Mark_kor=Mark_kor+0.5; %1.0
end  
Mark=Mark+Mark_kor;

if Mark < 0,
  Mark = 0;
end