function test_dynamic_compressor

N = 1024;
n = 0:N-1;

x = sin(pi/15*n);

% Pwyj_dB = a0 * (Pwej_dB - Po_dB) + Po_dB
%   if Pwej_dB = Po_dB then also Pwyj_dB = Po_dB
a0 = 1/4; Po_dB = -3; 

Po_lin = 10.^(Po_dB/10);
alfa = Po_lin.^(1-a0);
ind = 1;
for skala = [10, 2, 1, 1/2, 1/4, 1/10, 1/100, 1/1000]
  y_1=x*skala;
  Pwej_lin(ind) = mean(abs(y_1).^2);
  
  beta = alfa*(Pwej_lin(ind)).^(a0-1);
  
  y_2 = y_1 * sqrt(beta);
  Pwyj_lin(ind) = mean(abs(y_2).^2);
  
  ind = ind + 1;
end

figure(1)
subplot(2,1,1);
plot(Pwej_lin, Pwyj_lin);
subplot(2,1,2);
plot(10*log10(Pwej_lin), 10*log10(Pwyj_lin));
