function testSRC(DoWrite)

if nargin==0,
  DoWrite=0;
end

%test_signal=[1+j];
%test_signal=ones(100,1);
%test_signal=rand(100,1)+j*sin(0.1*[0:99].');
test_signal=[0:99].'+j*sin(0.1*[0:99].');
hI=conv(ones(5,1), ones(5,1)/5);
hI=ones(5,1);

if DoWrite==1,
	% generate test signal (delta)
	plik=fopen('input.flt', 'wb');
	if (plik ~= -1)
      x(1:2:200)=real(test_signal);
      x(2:2:200)=imag(test_signal);
      fwrite(plik,x,'float');
      fclose(plik);
	end
  
  
	plik=fopen('hI_coef.flt', 'wb');
	if (plik ~= -1)
      fwrite(plik,real(hI),'float');
%       fwrite(plik,real(hI)*32768,'integer*2');
%       fwrite(plik,real(hI)*128+128,'uchar');
      fclose(plik);
	end
else  

  % read output response 
	plik=fopen('output.flt', 'rb');
	if (plik ~= -1)
      y=fread(plik,inf,'float');
      fclose(plik);
	end
	
  y2r=resample([0; real(test_signal); 0; 0; 0; 0; 0; 0; 0],2,5, hI);
  y2i=resample([0; imag(test_signal); 0; 0; 0; 0; 0; 0; 0],2,5, hI);
  y2=y2r+j*y2i;
	figure(1)
	subplot(2,1,1);
	stem(y(1:2:end));
	subplot(2,1,2);
	stem(y(2:2:end));
	figure(2)
	subplot(2,1,1);
	stem(real(y2),'r');
  set(gca,'Xlim', [0 50]);
	subplot(2,1,2);
	stem(imag(y2),'r');
  set(gca,'Xlim', [0 50]);
end	
