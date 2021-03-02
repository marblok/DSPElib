function testSNR
%BPSK

N=100000; ile_razy=100; %1000;
% All is OK till pi/8
divider=6
phase_err=pi/divider;
dosave=0
if dosave==1,
	vector=exp(j*phase_err);
	SNR_all=(0:1:20); % in dB
	for SNR_ind=1:length(SNR_all),
      SNR=SNR_all(SNR_ind);
      % SNR=10*log10(SNR_lin)
      SNR_lin=10.^(SNR/10);
      sygn_val=1;
      err_ind=0;
      for ind=1:ile_razy,
	%     sqrt(SNR_lin)
        noise=1/sqrt(SNR_lin)*(randn(1,N)+j*randn(1,N))/sqrt(2);
      
	%     10*log10(mean(abs(noise).^2)), pause
        sygn=sygn_val*vector+noise;
        err_ind=err_ind+sum(real(sygn)>0);
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
	save(sprintf('BER_BPSK_%i',divider), 'BER', '-ascii', '-double');
	save(sprintf('SNR_all_BPSK_%i',divider), 'SNR_all', '-ascii', '-double');
end

% divider=8
phase_err=pi/divider;
BER=load(sprintf('BER_BPSK_%i',divider))
SNR_all=load(sprintf('SNR_all_BPSK_%i',divider));

SNR_lin_all=10.^(SNR_all/10);
BER_est=1/2*erfc(sqrt(SNR_lin_all));
% BER_est=BER_est*2; % for complex signals

% %this one works till pi/8
% BER_est2=1/2*erfc((1-0.2*sin(phase_err))*sqrt(SNR_lin_all));
% % % BER_est2=1/2*erfc((1-0.2*phase_err)*sqrt(SNR_lin_all));
% % % BER_est2=(1+phase_err)*BER_est2
% 
% % this works only for pi/4
% BER_est2=1/2*erfc((1-0.375*sin(phase_err))*sqrt(SNR_lin_all));
% BER_est2=BER_est2/(1-0.375*sin(phase_err))
% 
% %this one works till pi/4
% BER_est2=1/2*erfc((1-sin(phase_err).^3)*sqrt(SNR_lin_all));

%this one works till pi/2
% BER_est2=1/2*erfc((1-sin(phase_err).^2).^0.5*sqrt(SNR_lin_all));
BER_est2=1/2*erfc(sqrt((1-sin(phase_err).^2)*SNR_lin_all));

figure(1)
semilogy(SNR_all, BER);
hold on
semilogy(SNR_all, BER_est,'r');
semilogy(SNR_all, BER_est2,'g');
hold off
set(gca,'Ylim', [10.^-7, 1]);
