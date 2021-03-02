%sampling time reconstruction for BPSK - modification
function testSTR_b(x,L, M)
% no phase correction

wariant=0; %Gardner, matched filter - rectangular
wariant=1; %Gardner, matched filter - rcos

wariant=2+0; %modification 1, matched filter - rectangular
wariant=2+1; %modification 1, matched filter - rcos

wariant=4+2+0; %modification 2, matched filter - rectangular
wariant=4+2+1; %modification 2, matched filter - rcos

% wariant=4+2+1;
wariant=1;

N=length(x)/L;
figure
plot(x);
% x=x/max(abs(x));
hold on
plot(x, 'r');
hold off
pause

close all;
figure(1)
subplot(2,1,1)
plot(real(x),'b')
hold on
plot(imag(x),'m')
hold off

y=x;
% y=(y.^2); % optional in QPSK : fails in case of j*(-1)^n 
% % y=y.*conj(y); % inefficient (optional in QPSK)

% x_0=x(1:L:end);
% x_1=x(((L-1)/2+1):L:end);
% x_2=[x((L+1):L:end) 0];
n_0=1:(N*L-L);
y_0=y(n_0);
n_1=(L/2+1):(N*L-L/2);
y_1=y(n_1);
n_2=(L+1):N*L;
y_2=y(n_2);

% subplot(2,1,2)
% stem(n_0,real(y_0),'r');
% hold on
% stem(n_1,real(y_1),'g');
% stem(n_2,real(y_2),'m');
% hold off
% pause

alfa_2=sign(y_1);
if rem(floor(wariant/2), 2) == 0,
  %Gardner 
  epsilon=conj(y_1).*(y_0-y_2);
else
  alfa_1=((sign(y_2)-sign(y_0))/2).^2; % my modification
end
if rem(floor(wariant/4), 2) == 0,
  alfa_3=1; %Gardner ??
else
  alfa_3=abs(y_2-y_1).*abs(y_1-y_0); % my modification
end

% alfa_3=abs(y_2-conj(y_1)).*abs(conj(y_1)-y_0); % g³upie ale ten kierunek

% % alfa_3=abs((y_2-y_1).*conj(y_1-y_0)); % my modification
% A=1; %BPSK ?? mo¿e te¿ byæ 2
% A=1/2; %QPSK
A=1; %< 1 !!!
% alfa_3=abs((y_2-y_1).*(y_0-y_1)); % my modification for BPSK /QPSK
alfa3=1;
epsilon=A*alfa_3.*conj(y_1).*(y_0-y_2);

% epsilon2=A*alfa_3.*real(y_1).*(real(y_0)-real(y_2)) + ...
%   A*alfa_3.*imag(y_1).*(imag(y_0)-imag(y_2));
% close all
% subplot(2,1,1)
% plot(real(epsilon))
% subplot(2,1,2)
% plot(epsilon2)
% sum(abs(epsilon2-real(epsilon)))
% 
% pause



subplot(2,1,2)
plot(real(epsilon))
% hold on
% stem(imag(epsilon),'g')
% hold off
epsilon=real(epsilon);

beta=1/2;
if length(alfa_3) ~= 1,
  beta=2.5; %5; % for alfa_3
end
ind_=1; korekta=0; val=[];
while ind_<=length(epsilon),
%   if abs(epsilon(ind_))>0.5,
%     epsilon(ind_)=0.5*sign(epsilon(ind_));
%   end

%   subplot(2,1,2)
%   hold on
%   hp=plot(ind_,epsilon(ind_), 'ro');
%   set(hp,'LineWidth', 3);
%   hold off
% 
%   subplot(2,1,1)
%   hold on
%   hp(1)=plot(ind_+L/2,real(x(ind_+L/2)), 'bo');
%   hp(2)=plot(ind_+L/2,imag(x(ind_+L/2)), 'ro');
%   set(hp,'LineWidth', 3);
%   hold off

  val(end+1)=x(ind_+L/2);

  korekta=korekta-beta*epsilon(ind_);
  ind_=ind_+round(korekta);
  korekta=korekta-round(korekta);
  ind_=ind_+L;
%   rem(ind_,L)

%   pause(0.1)
%   tic; while toc<0.01, pause(0); end
  pause(0);
end

subplot(2,1,1)
hold off
subplot(2,1,2)
hold off

L
% % M=8; %8;
% x=x.^M;
% % x=x.*exp(j*pi/2);
% % x=x.^4;
% % x=filter(ones(1,L*10),1,x);
% x=filter(ones(1,L),1,x);
% x=x./abs(x);
% figure
% subplot(2,1,1)
% plot(real(x),'b');
% hold on
% plot(imag(x),'r');
% hold off
% subplot(2,1,2)
% plot(angle(x))
% % plot(unwrap(angle(x)))
% 
% N=11;
% hD=0; hD((N-1)/2+1)=1;
% val_old=filter(hD,1,val);;
% val=val./abs(val);
% val=val.^M;
% val=filter(ones(1,N)/N,1,val);
% % val=GetMeanVal(val,N);
% faza=unwrap(angle(val))/M;
% % faza=0;

figure
% % val_new=val_old.*exp(-j*faza);
% val_new=val_old;
val_new=val;
% val_new=val_new(50:end); %dodatek ???
plot(val_new,'o');
axis equal
pause
plot(val_new(2:end).*conj(val_new(1:end-1)),'ro');
axis equal
pause

figure
% subplot(2,1,1)
plot(real(val),'b');
hold on
plot(imag(val),'r');
hold off
% subplot(2,1,2)
% plot(faza*M)
% plot(angle(val))
% % plot(unwrap(angle(val)))
% % plot(angle(val(2:end).*conj(val(1:end-1))))
pause

