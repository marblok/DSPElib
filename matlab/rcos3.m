function h_rc=rcos3(N,L, r)

if nargin==2,
  r=0.5;
end

n=(0:N-1)-(N-1)/2;
% L=pi*L; h_rc=sinc(pi*n/L).*cos(pi*r*n/L)./(L-4*(r.^2)*(n.^2)/L);
h_rc=pi*sinc(n/L).*cos(pi*r*n/L)./(pi*L-4*pi*(r.^2)*(n.^2)/L);
ind=~isfinite(h_rc);
h_rc(ind)=0;

if nargout==0,
	figure(1)
	subplot(2,1,1)
	stem(h_rc)
	subplot(2,1,2)
	H=freqz(h_rc,1,2048, 'whole');
	H=abs(H(1:end/2));
	f=linspace(0,1000,length(H));
	plot(f,H)
  
	H=freqz(ones(1,L)/L,1,2048, 'whole');
	H=abs(H(1:end/2));
	f=linspace(0,1000,length(H));
  hold on
	plot(f,H,'k')
  hold off
end