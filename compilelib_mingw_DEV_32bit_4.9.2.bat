rem comflag=-m32 - kompilacja w wersji 32-bitowej (tdm-gcc 4.9.2)
set comflag=-m32


IF exist compilelib_mingw_DEV_32bit_4.9.2.bat GOTO AllOk
echo Working directory is not correct
rem pause
echo Trying "git_folder/DSPElib/DSPE_lib"
cd git_folder/DSPElib/DSPE_lib

IF exist compilelib_mingw_DEV_32bit_4.9.2.bat GOTO AllOk
echo Working directory is definitely is not correct !!!
pause
GOTO End

:AllOK
path=e:\Dev-Cpp\MinGW64\bin;e:\Dev-Cpp\MinGW64\x86_64-w64-mingw32\bin\;%path%


:MakeLib

cd ..\..\..\workspace
mkdir _DSPE_lib_minGW_
mkdir _DSPE_lib_minGW_\Dev-cpp%comflag%
mkdir _DSPE_lib_minGW_\Dev-cpp%comflag%\rls
mkdir _DSPE_lib_minGW_\Dev-cpp%comflag%\dbg
mkdir _DSPE_lib_minGW_\Dev-cpp%comflag%\include
mkdir _DSPE_lib_minGW_\Dev-cpp%comflag%\examples
mkdir _DSPE_lib_minGW_\Dev-cpp%comflag%\examples\matlab
mkdir _DSPE_lib_minGW_\Dev-cpp%comflag%\toolbox
cd ..\git_folder\DSPElib\DSPE_lib


mkdir "Debug Dev-cpp%comflag%"
mkdir "Debug Dev-cpp%comflag%\src"
mkdir "Debug Dev-cpp%comflag%\src\cpp"
mkdir "Debug Dev-cpp%comflag%\src\nvwa"
cd "Debug Dev-cpp%comflag%"
rm -rf src/cpp/DSPclocks.o src/Main.o src/Main.d src/cpp/DSPclocks.d DSPE_lib.exe src/cpp/DSP_DOT.d src/cpp/DSP_misc.o src/cpp/DSP_misc.d src/cpp/DSPmodules2.d src/cpp/DSP_AudioMixer.d src/cpp/DSPmodules2.o src/cpp/DSP_AudioMixer.o src/cpp/DSP_IO.o src/cpp/DSP_IO.d src/cpp/DSP_Fourier.d src/cpp/DSP_Fourier.o src/cpp/DSPmodules_misc.o src/cpp/DSPmodules_misc.d src/cpp/DSPmodules.d src/cpp/DSPmodules.o src/cpp/DSP_DOT.o src/cpp/DSPsockets.o src/cpp/DSPsockets.d

g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -fpermissive -o src\cpp\DSP_AudioMixer.o ..\src\cpp\DSP_AudioMixer.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSP_Fourier.o ..\src\cpp\DSP_Fourier.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPsockets.o ..\src\cpp\DSPsockets.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSP_misc.o ..\src\cpp\DSP_misc.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPclocks.o ..\src\cpp\DSPclocks.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPmodules.o ..\src\cpp\DSPmodules.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPmodules2.o ..\src\cpp\DSPmodules2.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSP_DOT.o ..\src\cpp\DSP_DOT.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\Main.o ..\src\Main.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPmodules_misc.o ..\src\cpp\DSPmodules_misc.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSP_IO.o ..\src\cpp\DSP_IO.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\nvwa\debug_new.o ..\src\nvwa\debug_new.cpp
g++ %comflag% -static-libgcc -static-libstdc++ -o DSPE_lib.exe src\nvwa\debug_new.o src\cpp\DSPsockets.o src\cpp\DSPmodules_misc.o src\cpp\DSPmodules2.o src\cpp\DSPmodules.o src\cpp\DSPclocks.o src\cpp\DSP_misc.o src\cpp\DSP_IO.o src\cpp\DSP_Fourier.o src\cpp\DSP_DOT.o src\cpp\DSP_AudioMixer.o src\Main.o -lwinmm -lws2_32

cd ..
mkdir "Release Dev-cpp%comflag%"
mkdir "Release Dev-cpp%comflag%\src"
mkdir "Release Dev-cpp%comflag%\src\cpp"
mkdir "Release Dev-cpp%comflag%\src\nvwa"
cd Release Dev-cpp%comflag%"
rm -rf src/cpp/DSPclocks.o src/Main.o src/Main.d src/cpp/DSPclocks.d DSPE_lib.exe src/cpp/DSP_DOT.d src/cpp/DSP_misc.o src/cpp/DSPmodules2.d src/cpp/DSP_AudioMixer.d src/cpp/DSPmodules2.o src/cpp/DSP_AudioMixer.o src/cpp/DSP_IO.o src/cpp/DSP_IO.d src/cpp/DSP_Fourier.d src/cpp/DSP_Fourier.o src/cpp/DSPmodules_misc.o src/cpp/DSPmodules_misc.d src/cpp/DSPmodules.d src/cpp/DSPmodules.o src/cpp/DSP_DOT.o src/cpp/DSPsockets.o src/cpp/DSPsockets.d

g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -fpermissive -o src\cpp\DSP_AudioMixer.o ..\src\cpp\DSP_AudioMixer.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSP_Fourier.o ..\src\cpp\DSP_Fourier.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPsockets.o ..\src\cpp\DSPsockets.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSP_Misc.o ..\src\cpp\DSP_Misc.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPclocks.o ..\src\cpp\DSPclocks.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPmodules.o ..\src\cpp\DSPmodules.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPmodules2.o ..\src\cpp\DSPmodules2.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSP_DOT.o ..\src\cpp\DSP_DOT.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\Main.o ..\src\Main.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPmodules_misc.o ..\src\cpp\DSPmodules_misc.cpp
g++ %comflag% -DWIN32 -DDEVCPP -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSP_IO.o ..\src\cpp\DSP_IO.cpp
g++ %comflag% -static-libgcc -static-libstdc++ -o DSPE_lib.exe src\cpp\DSPsockets.o src\cpp\DSPmodules_misc.o src\cpp\DSPmodules2.o src\cpp\DSPmodules.o src\cpp\DSPclocks.o src\cpp\DSP_misc.o src\cpp\DSP_IO.o src\cpp\DSP_Fourier.o src\cpp\DSP_DOT.o src\cpp\DSP_AudioMixer.o src\Main.o -lwinmm -lws2_32

cd ..

copy /B examples\*.*        ..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\examples\*.*
copy /B examples\matlab\*.*        ..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\examples\matlab\*.*
copy /B matlab\toolbox\*.m  ..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\toolbox\*.m
copy /B src\include\*.h     ..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\include\*.*
copy /B src\include\rls\DSP_setup.h   ..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\rls\*.*
copy /B src\include\dbg\DSP_setup.h   ..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\dbg\*.*

cd "Debug Dev-cpp%comflag%\src\cpp"

ar rc libDSPE.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o DSP_Fourier.o DSP_AudioMixer.o DSPmodules_misc.o DSP_DOT.o
ranlib libDSPE.a
ar rc libDSPEsockets.a DSPsockets.o
ranlib libDSPEsockets.a
pause

copy /B libDSPE.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\dbg\*.*
del libDSPE.a Y
copy /B libDSPEsockets.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\dbg\*.*
del libDSPEsockets.a Y

cd ..\..\..

cd "Release Dev-cpp%comflag%\src\cpp"

ar rc libDSPE.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o DSP_Fourier.o DSP_AudioMixer.o DSPmodules_misc.o DSP_DOT.o
ranlib libDSPE.a
ar rc libDSPEsockets.a DSPsockets.o
ranlib libDSPEsockets.a
pause

copy /B libDSPE.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\rls\*.*
del libDSPE.a Y
copy /B libDSPEsockets.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\Dev-cpp%comflag%\rls\*.*
del libDSPEsockets.a Y

cd ..\..\..

:End