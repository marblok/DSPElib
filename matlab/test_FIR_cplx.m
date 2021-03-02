function test_FIR_cplx

N = 128;
n = 0:N-1;

x =  exp(j*0.1*n);

h = conj(x(end:-1:1));

dane.h  = h;
dane.Fp = 8000;
save_filter_coef('test', dane);

filewrite('test_in', x, 'float', dane.Fp);