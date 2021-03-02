function test_sinc

t= -5:0.0001:5;
% t= -0.5:0.0001:0.5;
% t= -0.000001:0.0000001:0.000001;

x = sinc(t);

x2 = sinc_1(t, 4);
x3 = sinc_2(t, 160);
x4 = sinc_3(t, 4);

figure(1)
subplot(4,1,1)
plot(t, x, 'k');
hold on
plot(t, x2, 'b');
plot(t, x3, 'r');
plot(t, x4, 'm');
hold off
subplot(4,1,2)
plot(t, x2-x);
hold on
plot(t, x3-x, 'r');
plot(t, x4-x, 'm');
hold off

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function x=sinc_1(t, K)

t = pi*t/2;
x = cos(t);
for k = 2:K,
  t = t/2;
  x = x .* cos(t);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function x=sinc_2(t, K)

t = pi*t;
x = 1;
for k = 1:K,
  x = x .* (1 - (t.^2)./((pi^2)*(k^2)));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function x=sinc_3(t, K)

t = pi*t/3;
x = 1 - 4/3*sin(t).^2;
for k = 2:K,
  t = t/3;
  x = x .* (1 - 4/3*sin(t).^2);
end