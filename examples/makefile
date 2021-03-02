CC = g++

TYPE = dbg
#TYPE = rls

ifeq ($(TYPE),dbg)
  CFLAGS = -O0 -g3 -Wall
  LINKER_FLAGS = -static-libgcc -static-libstdc++
else #rls
	CFLAGS = -O3 -Wall
	LINKER_FLAGS = -s -static-libgcc -static-libstdc++
endif


SRCS = hello.cpp

all: hello.exe echo.exe multirate.exe callbacks.exe socket_server.exe socket_client.exe socket_server_2.exe socket_client_2.exe macro_example.exe asynchronous.exe asynchronous_CT.exe
#all: echo.exe  

hello.o: hello.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c hello.cpp

hello.exe: hello.o
	$(CC) -L../$(TYPE) hello.o -o"hello.exe" $(LINKER_FLAGS) -lDSP -lwinmm

echo.o: echo.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c echo.cpp

echo.exe: echo.o
	$(CC) -L../$(TYPE) echo.o -o"echo.exe" $(LINKER_FLAGS) -lDSP -lwinmm

multirate.o: multirate.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c multirate.cpp

multirate.exe: multirate.o
	$(CC) -L../$(TYPE) multirate.o -o"multirate.exe" $(LINKER_FLAGS) -lDSP -lwinmm

callbacks.o: callbacks.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c callbacks.cpp

callbacks.exe: callbacks.o
	$(CC) -L../$(TYPE) callbacks.o -o"callbacks.exe" $(LINKER_FLAGS) -lDSP -lwinmm 

socket_server.o: socket_server.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c socket_server.cpp

socket_server.exe: socket_server.o
	$(CC) -L../$(TYPE) socket_server.o -o"socket_server.exe" $(LINKER_FLAGS) -lDSP -lwinmm -lDSPsockets -lws2_32 

socket_client.o: socket_client.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c socket_client.cpp

socket_client.exe: socket_client.o
	$(CC) -L../$(TYPE) socket_client.o -o"socket_client.exe" $(LINKER_FLAGS) -lDSP -lwinmm -lDSPsockets -lws2_32 


socket_server_2.o: socket_server_2.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c socket_server_2.cpp

socket_server_2.exe: socket_server_2.o
	$(CC) -L../$(TYPE) socket_server_2.o -o"socket_server_2.exe" $(LINKER_FLAGS) -lDSP -lwinmm -lDSPsockets -lws2_32 

socket_client_2.o: socket_client_2.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c socket_client_2.cpp

socket_client_2.exe: socket_client_2.o
	$(CC) -L../$(TYPE) socket_client_2.o -o"socket_client_2.exe" $(LINKER_FLAGS) -lDSP -lwinmm -lDSPsockets -lws2_32 

macro_example.o: macro_example.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c macro_example.cpp
macro_example.exe: macro_example.o
	$(CC) -L../$(TYPE) macro_example.o -o"macro_example.exe" $(LINKER_FLAGS) -lDSP -lwinmm -lDSPsockets -lws2_32 

asynchronous.o: asynchronous.cpp
	$(CC) -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c asynchronous.cpp
asynchronous.exe: asynchronous.o
	$(CC) -L../$(TYPE) asynchronous.o -o"asynchronous.exe" $(LINKER_FLAGS) -lDSP -lwinmm -lDSPsockets -lws2_32
	 
asynchronous_CT.o: asynchronous.cpp
	$(CC) -Duse_clock_trigger -DWIN32 -I../include -I../$(TYPE) $(CFLAGS) -c asynchronous.cpp -o asynchronous_CT.o
asynchronous_CT.exe: asynchronous_CT.o
	$(CC) -L../$(TYPE) asynchronous_CT.o -o"asynchronous_CT.exe" $(LINKER_FLAGS) -lDSP -lwinmm -lDSPsockets -lws2_32 

clean:
	rm -f hello.exe hello.o 
	rm -f echo.exe echo.o 
	rm -f asynchronous.exe asynchronous.o
	rm -f asynchronous_CT.exe asynchronous_CT.o
	rm -f multirate.exe multirate.o
	rm -f callbacks.exe callbacks.o 
	rm -f socket_server.exe socket_server.o 
	rm -f socket_client.exe socket_client.o 
	rm -f socket_server_2.exe socket_server_2.o 
	rm -f socket_client_2.exe socket_client_2.o
	rm -f macro_example.exe macro_example.o
	rm -f *.log
	rm -f matlab/LPF_22050_8000.coef
	rm -f multirate.wav
	rm -f morse.wav
	rm -f morse_key.wav
	rm -f test_out.wav
	rm -f *.dot
	rm -f *.gif
