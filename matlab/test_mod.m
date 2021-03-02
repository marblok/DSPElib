function test_mod

% test1
test2

function test2

Nb=300;
Fb=120;
L11=10; Fs_11=Fb*L11;
L11_form=6;

figure(1)
for ind=1:12,
	b=2*((rand(1,Nb)>0.5)-0.5);
	xb{ind}=zeros(Nb*L11,1);
	xb{ind}(1:L11:end)=b;

%   h_form=ones(L11_form,1);
%   h_form=rcos3(4*L11+1,L11,0.9); %1.75);
  h_form=rcos3(4*L11+1,L11,0.25); h_form=h_form.*hamming(length(h_form)).';
  if (ind==1),
%     [H,F]=freqz(h_form,1,2048, 'whole', Fs_11); plot(F,abs(H));pause
  	stem(h_form); pause
  end
  
	xb{ind}=filter(h_form, 1, xb{ind});
	
  if (ind==1),
  	psd(xb{ind}+j.*0.00001,1024,Fs_11)
	  pause
  end
	
	L12=4; Fs_12=Fs_11*L12;
	h_I=remez(4*18-1,2*[0, 100, 900, Fs_12/2]/Fs_12, [1 1 0 0]);
  if (ind==1),
  	freqz(h_I, 1,2048, 'whole', Fs_12)
  	pause
  end

  x_temp=zeros(L12*length(xb{ind}),1);
  x_temp(1:L12:end)=xb{ind};
	xb{ind}=filter(h_I, 1, x_temp);
  if (ind==1),
  	plot(xb{ind}); pause
	
  	psd(xb{ind}+j.*0.00001,1024,Fs_11)
  	pause
  end
end


%wariant z wyjœciow¹ heterodyn¹
fp = 15800; % pilot frequency
Fs=48000; % output sampling frequency

h_I=cremez(12*18-1,2*[0, 1300, 2600, Fs/2]/Fs, [1 1 0 0]);
h_I=12*real(h_I);
freqz(h_I, 1,2048, 'whole', Fs)
pause

%
fp_12=1300;
Fs_12=Fs/12;

N=length(xb{1});
n=0:(N-1); n=n.';
%pilot
x=exp(j*2*pi*fp_12/Fs_12.*n);
fo=fp_12-400;
for ind=12:-1:1,
  x=x+xb{ind}.*exp(j*2*pi*fo/Fs_12.*n);
  fo=fo-200;
  if fo < -2000
    fo=fo+4000;
  end
psd(x+j.*0.00001,1024,Fs_12)
pause
end


y=zeros(1,12*N);
y(1:12:end)=x;
psd(y+j.*0.00001,12*1024,Fs)
pause

y=filter(h_I, 1, y);
psd(y+j.*0.00001,12*1024,Fs)
pause

n=0:(12*N-1);
y=y.*exp(j*2*pi*(fp-fp_12)/Fs.*n);
y=real(y);
psd(y+j.*0.00001,12*1024,Fs)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
return
%wariant bez wyjœciowej heterodyny
fp = 15800; % pilot frequency
Fs=48000; % output sampling frequency

h_I=cremez(12*15-1,2*[-Fs/2, 11800, 13100, 15800, 17100, Fs/2]/Fs, [0 0 1 1 0 0])
freqz(h_I, 1,2048, 'whole', Fs)
pause

%
fp_12=-200;
Fs_12=Fs/12;

N=20000;
n=0:(N-1);
%pilot
x=2*exp(j*2*pi*fp_12/Fs_12.*n);
fo=fp_12-400;
for ind=12:-1:1,
  x=x+exp(j*2*pi*fo/Fs_12.*n);
  fo=fo-200;
  if fo < -2000
    fo=fo+4000;
  end
psd(x+j.*0.00001,1024,Fs_12)
pause
end


y=zeros(1,12*N);
y(1:12:end)=x;
psd(y+j.*0.00001,8*1024,Fs)
pause

y=filter(h_I, 1, y);
psd(y+j.*0.00001,8*1024,Fs)
pause

y=real(y);
psd(y+j.*0.00001,8*1024,Fs)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
return

function test1;
plik=fopen('Signal.out', 'rb')
y=fread(plik,inf, 'float');
fclose (plik);

Fs=5*9600;

figure(1)
plot(y(1:800))
pause
psd(y,1024,Fs)
pause
specgram(y,1024,Fs)

return 
