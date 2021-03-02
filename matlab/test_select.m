function test_select

% plik=fopen('test_.out', 'rb');
plik=fopen('test1.out', 'rb');
X=fread(plik,inf,'float');
fclose(plik);

% plik=fopen('test_select.out', 'rb');
plik=fopen('test2.out', 'rb');
Y=fread(plik,inf,'float');
fclose(plik);

% plik=fopen('test_select2.out', 'rb');
plik=fopen('test3.out', 'rb');
Z=fread(plik,inf,'float');
fclose(plik);

subplot(3,1,1)
plot(X,'.');
% plot(Z(Z>0),'r.');
subplot(3,1,2)
% plot(X(1:2:end),'.');
plot(Y,'.');
subplot(3,1,3)
plot(Z,'.');
% plot(X(2:2:end),'.');