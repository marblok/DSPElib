function testSNR_QPSK
% measured vaule is not the BER but symbol error rate

N=100000; ile_razy=100; %1000;
divider=5.5
phase_err=pi/divider;
dosave=0
if dosave==1,
	vector=exp(j*phase_err);
	SNR_all=(0:1:20); % in dB
	for SNR_ind=1:length(SNR_all),
      SNR=SNR_all(SNR_ind);
      % SNR=10*log10(SNR_lin)
      SNR_lin=10.^(SNR/10);
      QPSK=1;
      if QPSK==1,
        sygn_val=(1+j)/sqrt(2);
      else
        sygn_val=1;
      end
      err_ind=0;
      for ind=1:ile_razy,
	%     sqrt(SNR_lin)
        noise=1/sqrt(SNR_lin)*(randn(1,N)+j*randn(1,N))/sqrt(2);
      
	%     10*log10(mean(abs(noise).^2)), pause
        sygn=sygn_val*vector+noise;
        err_ind=err_ind+sum((real(sygn)>0) & (imag(sygn)>0));
	%     ind
        pause(0)
      end
      SNR_est_tmp=abs(sygn_val.^2)/mean(abs(noise).^2);
      SNR_est(SNR_ind)=10.*log10(SNR_est_tmp);
      BER(SNR_ind)=(N-err_ind/ile_razy)/N
      sprintf('BER=%0.4g; SNR=%.1f[dB]', BER(SNR_ind), SNR)
	end
	
	SNR_all
	SNR_est
	save(sprintf('BER_%i',divider), 'BER', '-ascii', '-double');
	save(sprintf('SNR_all_%i',divider), 'SNR_all', '-ascii', '-double');
end

% divider=8
phase_err=pi/divider;
SER=load(sprintf('BER_%i',divider))
SNR_all=load(sprintf('SNR_all_%i',divider));

SNR_lin_all=10.^(SNR_all/10);
SER_est=1/2*erfc(sqrt(SNR_lin_all/2));
SER_est=SER_est*2; % for two bits per symbol

% % % % % BER_est2=1/2*erfc((1-sin(phase_err))*sqrt(SNR_lin_all/2));
% % % % % BER_est2=1/2*erfc(sqrt((1-sin(phase_err))*SNR_lin_all/2));
% % % % BER_est2=1/2*erfc(sqrt((1-5*phase_err/pi)*SNR_lin_all/2));
% % % BER_est2=1/2*erfc((1-sin(phase_err))*sqrt(SNR_lin_all/2));
% % % BER_est2=(1-sin(phase_err))*BER_est2*2; % for complex signals
% % SER_est2=1/2*erfc((1-sin(phase_err))*sqrt(SNR_lin_all/2));
% % SER_est2=sqrt(1-sin(phase_err))*SER_est2*2; % for two bits per symbol

% %works good till pi/16
% SER_est2=1/2*erfc(sqrt((1-2*sin(phase_err))*SNR_lin_all/2));
% SER_est2=(1-2*sin(phase_err))*SER_est2*2; % for two bits per symbol

% if (1-4*sin(phase_err)) < 0,
% % %   SER_est2=1/2*erfc(sqrt((1-2*sin(phase_err)).^sqrt(0.5)*SNR_lin_all/2));
% %   SER_est2=1/2*erfc(sqrt((1-2*sin(phase_err)).^sqrt(0.707)*SNR_lin_all/2));
%   SER_est2=1/2*erfc(sqrt((1-2*sin(phase_err)).^sqrt(sqrt(0.5))*SNR_lin_all/2));
%   SER_est2=(1-sin(phase_err))*SER_est2*2; % for two bits per symbol
% else
% %works good till pi/16
%   SER_est2=1/2*erfc(sqrt((1-4*sin(phase_err)).^0.25*SNR_lin_all/2));
%   SER_est2=(1-2*sin(phase_err))*SER_est2*2; % for two bits per symbol
% end

% % works good till pi/8 (pi/7) sucks for pi/6
% SER_est2=1/2*erfc(sqrt((1-2*sin(phase_err)).^sqrt(sqrt(0.5))*SNR_lin_all/2));
% SER_est2=(1-sin(phase_err))*SER_est2*2; % for two bits per symbol

% % % works good for pi/6 pi/7 : sin(pi/6)=0.5
% SER_est2=1/2*erfc(sqrt((1-sin(phase_err)).^sqrt(sqrt(0.5*128))*SNR_lin_all/2));
% SER_est2=(1-sin(phase_err))*SER_est2*2; % for two bits per symbol

% % % works good for pi/5 : sin(pi/5)=0.5878
% SER_est2=1/2*erfc(sqrt((1-sin(phase_err)).^sqrt(sqrt(128))*SNR_lin_all/2));
% SER_est2=sqrt(2)*(1-sin(phase_err))*SER_est2*2; % for two bits per symbol

% % % works good for pi/4 : sin(pi/4)=0.7071
% SER_est2=1/2*erfc(sqrt((1-sin(phase_err)).^sqrt(sqrt(16*128))*SNR_lin_all/2));
% SER_est2=sqrt(4)*(1-sin(phase_err))*SER_est2*2; % for two bits per symbol

% % works good for ???
if sin(phase_err) >= 0.45 % phase_err > pi/6 // sin(pi/8) sin(pi/7)
  % 0.5+(100*(sin(pi./[4 5 6])-0.5).^2).^2
  alfa=0.5+(100*(sin(phase_err)-0.5).^2).^2
  % (1+(sin(pi./[4 5 6])-0.5)*5).^2
  beta=(1+(sin(phase_err)-0.5)*5).^2
  SER_est2=1/2*erfc(sqrt((1-sin(phase_err)).^sqrt(sqrt(alfa*128))*SNR_lin_all/2));
  SER_est2=sqrt(beta)*(1-sin(phase_err))*SER_est2*2; % for two bits per symbol
else
  SER_est2=1/2*erfc(sqrt((1-2*sin(phase_err)).^sqrt(sqrt(0.5))*SNR_lin_all/2));
  SER_est2=(1-sin(phase_err))*SER_est2*2; % for two bits per symbol
end
% figure(1)
% plot([0.5 0.5878 0.707], [0.5, 1, 16]);
% return
figure(1)
semilogy(SNR_all, SER);
hold on
semilogy(SNR_all, SER_est,'r');
semilogy(SNR_all, SER_est2,'g');
hold off
set(gca,'Ylim', [10.^-7, 1]);
