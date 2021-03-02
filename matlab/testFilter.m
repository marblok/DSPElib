function testFilter

DoWrite=0;

%test_signal=[1+j];
test_signal=[1 j*0.2 -2*j 1.2];

if DoWrite==1,
	% generate test signal (delta)
	plik=fopen('delta.flt', 'wb');
	if (plik ~= -1)
      fwrite(plik,real(test_signal),'float');
      fclose(plik);
	end

	plik=fopen('delta_im.flt', 'wb');
	if (plik ~= -1)
      fwrite(plik,imag(test_signal),'float');
      fclose(plik);
	end
else  
  
	% read output response 
	plik=fopen('Response.out', 'rb');
	if (plik ~= -1)
      h=fread(plik,inf,'float');
      fclose(plik);
	end
	
	figure(1)
	subplot(1,1,1);
	stem(h(1:10));
	pause
	
	h1=h(1:2:end);
	h2=h(2:2:end);
	subplot(2,1,1);
	stem(h1(1:10));
	subplot(2,1,2);
	stem(h2(1:10));
	
	% test
	a=[0.5+j*0.1, -0.2+j*-0.3];
	b=[1.0, 0.0];
	% a0=a(1);	% a=a/a0;	% b=b/a0;
  a=[1.0, -0.25];  b=[1.0, 0.0];
	
  b=[1.0, 1.5, -1.2, 0] +j*[-1.0, 2.5, -1.2, 0.1]; a=1;
  b=real(b);
% 	delta=1; delta(10)=0;
test_signal(100)=0;
	y=filter(b,a,test_signal);
	
	figure(2)
	h1=real(y);
	h2=imag(y);
	subplot(2,1,1);
	stem(h1(1:10),'r');
	subplot(2,1,2);
	stem(h2(1:10),'r');
end
