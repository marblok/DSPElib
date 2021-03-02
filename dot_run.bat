path = c:\Program Files\Graphviz 2.21\bin\;%path%

cd ../_DSP_lib_minGW/examples

rem dot -Tps2 test_scheme_file.dot -otest.ps
rem ps2pdf test.ps test.pdf
dot -Tgif hello.dot -ohello.gif
dot -Tgif socket_client_2.dot -osocket_client_2.gif
dot -Tgif socket_server_2.dot -osocket_server_2.gif
rem dot -Tsvg test_scheme_file.dot -otest.svg

cd ../../Dsp_lib

