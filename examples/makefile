# For 32-bit compilation use Mbit = 32 
CC = g++
com_ver = $(shell $(CC) -dumpmachine)-gcc_$(shell $(CC) -dumpversion)

ifndef TYPE
	TYPE = dbg
	#TYPE = rls
endif

ifndef SRC_DIR
	SRC_DIR = .
endif
ifndef DSPELIB_DIR_ROOT
	DSPELIB_DIR_ROOT = ../_DSPE_lib_minGW_/$(com_ver)
endif

ifndef Mbit
	Mbit = 64
endif
ifeq ($(TYPE),dbg)
	CFLAGS = -m$(Mbit) -O0 -g3 -Wall -std=c++0x 
	LINKER_FLAGS = -m$(Mbit) -static-libgcc -static-libstdc++ -static
else #rls
	CFLAGS = -m$(Mbit) -O3 -Wall -std=c++0x 
	LINKER_FLAGS = -m$(Mbit) -s -static-libgcc -static-libstdc++ -static
endif


SRCS = hello.cpp

all: hello.exe echo.exe sound_input.exe multirate.exe callbacks.exe socket_server.exe socket_client.exe socket_server_2.exe socket_client_2.exe macro_example.exe asynchronous.exe asynchronous_CT.exe
#all: echo.exe  

hello.o: $(SRC_DIR)/hello.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/hello.cpp" -o "$(SRC_DIR)/hello.o"
hello.exe: hello.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/hello.o" -o "$(SRC_DIR)/hello.exe" $(LINKER_FLAGS) -lDSPE -lwinmm

echo.o: $(SRC_DIR)/echo.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c $(SRC_DIR)/echo.cpp -o "$(SRC_DIR)/echo.o"
echo.exe: echo.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/echo.o" -o"$(SRC_DIR)/echo.exe" $(LINKER_FLAGS) -lDSPE -lwinmm

sound_input.o: $(SRC_DIR)/sound_input.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c $(SRC_DIR)/sound_input.cpp -o "$(SRC_DIR)/sound_input.o"
sound_input.exe: sound_input.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/sound_input.o" -o"$(SRC_DIR)/sound_input.exe" $(LINKER_FLAGS) -lDSPE -lwinmm

multirate.o: $(SRC_DIR)/multirate.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/multirate.cpp" -o "$(SRC_DIR)/multirate.o"
multirate.exe: multirate.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/multirate.o" -o"$(SRC_DIR)/multirate.exe" $(LINKER_FLAGS) -lDSPE -lwinmm

callbacks.o: $(SRC_DIR)/callbacks.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/callbacks.cpp" -o "$(SRC_DIR)/callbacks.o"
callbacks.exe: callbacks.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/callbacks.o" -o"$(SRC_DIR)/callbacks.exe" $(LINKER_FLAGS) -lDSPE -lwinmm 

socket_server.o: $(SRC_DIR)/socket_server.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/socket_server.cpp" -o "$(SRC_DIR)/socket_server.o"
socket_server.exe: socket_server.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/socket_server.o" -o"$(SRC_DIR)/socket_server.exe" $(LINKER_FLAGS) -lDSPE -lwinmm -lDSPEsockets -lws2_32 

socket_client.o: $(SRC_DIR)/socket_client.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/socket_client.cpp" -o "$(SRC_DIR)/socket_client.o"
socket_client.exe: socket_client.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/socket_client.o" -o"$(SRC_DIR)/socket_client.exe" $(LINKER_FLAGS) -lDSPE -lwinmm -lDSPEsockets -lws2_32 


socket_server_2.o: $(SRC_DIR)/socket_server_2.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/socket_server_2.cpp" -o "$(SRC_DIR)/socket_server_2.o"
socket_server_2.exe: socket_server_2.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/socket_server_2.o" -o"$(SRC_DIR)/socket_server_2.exe" $(LINKER_FLAGS) -lDSPE -lwinmm -lDSPEsockets -lws2_32 

socket_client_2.o: $(SRC_DIR)/socket_client_2.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/socket_client_2.cpp" -o "$(SRC_DIR)/socket_client_2.o"
socket_client_2.exe: socket_client_2.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/socket_client_2.o" -o"$(SRC_DIR)/socket_client_2.exe" $(LINKER_FLAGS) -lDSPE -lwinmm -lDSPEsockets -lws2_32 

macro_example.o: $(SRC_DIR)/macro_example.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/macro_example.cpp" -o "$(SRC_DIR)/macro_example.o"
macro_example.exe: macro_example.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/macro_example.o" -o"$(SRC_DIR)/macro_example.exe" $(LINKER_FLAGS) -lDSPE -lwinmm 

asynchronous.o: $(SRC_DIR)/asynchronous.cpp
	$(CC) -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/asynchronous.cpp" -o "$(SRC_DIR)/asynchronous.o"
asynchronous.exe: asynchronous.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/asynchronous.o" -o"$(SRC_DIR)/asynchronous.exe" $(LINKER_FLAGS) -lDSPE -lwinmm
	 
asynchronous_CT.o: $(SRC_DIR)/asynchronous.cpp
	$(CC) -Duse_clock_trigger -DWIN32 -I"$(DSPELIB_DIR_ROOT)/include" -I"$(DSPELIB_DIR_ROOT)/include/$(TYPE)" $(CFLAGS) -c "$(SRC_DIR)/asynchronous.cpp" -o "$(SRC_DIR)/asynchronous_CT.o"
asynchronous_CT.exe: asynchronous_CT.o
	$(CC) -L"$(DSPELIB_DIR_ROOT)/$(TYPE)" "$(SRC_DIR)/asynchronous_CT.o" -o"$(SRC_DIR)/asynchronous_CT.exe" $(LINKER_FLAGS) -lDSPE -lwinmm 

clean:
	rm -f $(SRC_DIR)/hello.exe $(SRC_DIR)/hello.o 
	rm -f $(SRC_DIR)/echo.exe $(SRC_DIR)/echo.o 
	rm -f $(SRC_DIR)/sound_input.exe $(SRC_DIR)/sound_input.o 
	rm -f $(SRC_DIR)/asynchronous.exe $(SRC_DIR)/asynchronous.o
	rm -f $(SRC_DIR)/asynchronous_CT.exe $(SRC_DIR)/asynchronous_CT.o
	rm -f $(SRC_DIR)/multirate.exe $(SRC_DIR)/multirate.o
	rm -f $(SRC_DIR)/callbacks.exe $(SRC_DIR)/callbacks.o 
	rm -f $(SRC_DIR)/socket_server.exe $(SRC_DIR)/socket_server.o 
	rm -f $(SRC_DIR)/socket_client.exe $(SRC_DIR)/socket_client.o 
	rm -f $(SRC_DIR)/socket_server_2.exe $(SRC_DIR)/socket_server_2.o 
	rm -f $(SRC_DIR)/socket_client_2.exe $(SRC_DIR)/socket_client_2.o
	rm -f $(SRC_DIR)/macro_example.exe $(SRC_DIR)/macro_example.o
	rm -f $(SRC_DIR)/*.log
	rm -f $(SRC_DIR)/matlab/*.coef
	rm -f $(SRC_DIR)/multirate.wav
	rm -f $(SRC_DIR)/morse.wav
	rm -f $(SRC_DIR)/morse_key.wav
	rm -f $(SRC_DIR)/test_out.wav
	rm -f $(SRC_DIR)/captured_sample.wav
	rm -f $(SRC_DIR)/*.dot
	rm -f $(SRC_DIR)/*.gif
