rem dir *
rem pause
echo Compiling DSPElib library

rem comflag=-m32 - kompilacja w wersji 32-bitowej (tdm-gcc 4.9.2)
set comflag=-m32

IF exist compilelib_mingw_tdm_32.bat GOTO AllOk
echo Working directory is not correct
rem pause
echo Trying "git_folder/DSPElib/DSPE_lib"
cd git_folder/DSPElib/DSPE_lib

IF exist compilelib_mingw_tdm_32.bat GOTO AllOk
echo Working directory is definitely is not correct !!!
pause
GOTO End

:AllOK
path=e:\TDM-GCC-32\bin\;e:\MinGW_gcc_4\msys\1.0\bin\;%path%

:MakeLib
echo MakeLib

cd ..\..\..\workspace
mkdir _DSPE_lib_minGW_
mkdir _DSPE_lib_minGW_\mingw_tdm_32
mkdir _DSPE_lib_minGW_\mingw_tdm_32\rls
mkdir _DSPE_lib_minGW_\mingw_tdm_32\dbg
mkdir _DSPE_lib_minGW_\mingw_tdm_32\include
mkdir _DSPE_lib_minGW_\mingw_tdm_32\examples
mkdir _DSPE_lib_minGW_\mingw_tdm_32\examples\matlab
mkdir _DSPE_lib_minGW_\mingw_tdm_32\toolbox
cd ..\git_folder\DSPElib\DSPE_lib

mkdir "Debug mingw_tdm_32"
mkdir "Debug mingw_tdm_32\src"
mkdir "Debug mingw_tdm_32\src\cpp"
mkdir "Debug mingw_tdm_32\src\nvwa"
cd "Debug mingw_tdm_32"
rm -rf src/cpp/DSPclocks.o src/Main.o src/Main.d src/cpp/DSPclocks.d DSPE_lib.exe src/cpp/DSP_DOT.d src/cpp/DSP_misc.o src/cpp/DSP_misc.d src/cpp/DSPmodules2.d src/cpp/DSP_AudioMixer.d src/cpp/DSPmodules2.o src/cpp/DSP_AudioMixer.o src/cpp/DSP_IO.o src/cpp/DSP_IO.d src/cpp/DSP_Fourier.d src/cpp/DSP_Fourier.o src/cpp/DSPmodules_misc.o src/cpp/DSPmodules_misc.d src/cpp/DSPmodules.d src/cpp/DSPmodules.o src/cpp/DSP_DOT.o src/cpp/DSPsockets.o src/cpp/DSPsockets.d

g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -fpermissive -o src\cpp\DSP_AudioMixer.o ..\src\cpp\DSP_AudioMixer.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSP_Fourier.o ..\src\cpp\DSP_Fourier.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPsockets.o ..\src\cpp\DSPsockets.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSP_misc.o ..\src\cpp\DSP_misc.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPclocks.o ..\src\cpp\DSPclocks.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPmodules.o ..\src\cpp\DSPmodules.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPmodules2.o ..\src\cpp\DSPmodules2.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSP_DOT.o ..\src\cpp\DSP_DOT.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\Main.o ..\src\Main.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSPmodules_misc.o ..\src\cpp\DSPmodules_misc.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\cpp\DSP_IO.o ..\src\cpp\DSP_IO.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o src\nvwa\debug_new.o ..\src\nvwa\debug_new.cpp
g++ %comflag% -static-libgcc -static-libstdc++ -o DSPE_lib.exe src\nvwa\debug_new.o src\cpp\DSPsockets.o src\cpp\DSPmodules_misc.o src\cpp\DSPmodules2.o src\cpp\DSPmodules.o src\cpp\DSPclocks.o src\cpp\DSP_misc.o src\cpp\DSP_IO.o src\cpp\DSP_Fourier.o src\cpp\DSP_DOT.o src\cpp\DSP_AudioMixer.o src\Main.o -lwinmm -lws2_32

cd ..
mkdir "Release mingw_tdm_32"
mkdir "Release mingw_tdm_32\src"
mkdir "Release mingw_tdm_32\src\cpp"
mkdir "Release mingw_tdm_32\src\nvwa"
cd "Release mingw_tdm_32"
rm -rf src/cpp/DSPclocks.o src/Main.o src/Main.d src/cpp/DSPclocks.d DSPE_lib.exe src/cpp/DSP_DOT.d src/cpp/DSP_misc.o src/cpp/DSP_misc.d src/cpp/DSPmodules2.d src/cpp/DSP_AudioMixer.d src/cpp/DSPmodules2.o src/cpp/DSP_AudioMixer.o src/cpp/DSP_IO.o src/cpp/DSP_IO.d src/cpp/DSP_Fourier.d src/cpp/DSP_Fourier.o src/cpp/DSPmodules_misc.o src/cpp/DSPmodules_misc.d src/cpp/DSPmodules.d src/cpp/DSPmodules.o src/cpp/DSP_DOT.o src/cpp/DSPsockets.o src/cpp/DSPsockets.d

g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -fpermissive -o src\cpp\DSP_AudioMixer.o ..\src\cpp\DSP_AudioMixer.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSP_Fourier.o ..\src\cpp\DSP_Fourier.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPsockets.o ..\src\cpp\DSPsockets.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSP_Misc.o ..\src\cpp\DSP_Misc.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPclocks.o ..\src\cpp\DSPclocks.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPmodules.o ..\src\cpp\DSPmodules.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPmodules2.o ..\src\cpp\DSPmodules2.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSP_DOT.o ..\src\cpp\DSP_DOT.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\Main.o ..\src\Main.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSPmodules_misc.o ..\src\cpp\DSPmodules_misc.cpp
g++ %comflag% -DWIN32 -D_FILE_OFFSET_BITS=64 -I../src/include -I../src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o src\cpp\DSP_IO.o ..\src\cpp\DSP_IO.cpp
g++ %comflag% -static-libgcc -static-libstdc++ -o DSPE_lib.exe src\cpp\DSPsockets.o src\cpp\DSPmodules_misc.o src\cpp\DSPmodules2.o src\cpp\DSPmodules.o src\cpp\DSPclocks.o src\cpp\DSP_misc.o src\cpp\DSP_IO.o src\cpp\DSP_Fourier.o src\cpp\DSP_DOT.o src\cpp\DSP_AudioMixer.o src\Main.o -lwinmm -lws2_32

cd ..

copy /B examples\*.*        ..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\examples\*.*
copy /B examples\matlab\*.*        ..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\examples\matlab\*.*
copy /B matlab\toolbox\*.m  ..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\toolbox\*.m
copy /B src\include\*.h     ..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\include\*.*
copy /B src\include\rls\DSP_setup.h   ..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\rls\*.*
copy /B src\include\dbg\DSP_setup.h   ..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\dbg\*.*

cd "Debug mingw_tdm_32\src\cpp"

ar rc libDSPE.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o DSP_Fourier.o DSP_AudioMixer.o DSPmodules_misc.o DSP_DOT.o
ranlib libDSPE.a
ar rc libDSPEsockets.a DSPsockets.o
ranlib libDSPEsockets.a
pause

copy /B libDSPE.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\dbg\*.*
del libDSPE.a Y
copy /B libDSPEsockets.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\dbg\*.*
del libDSPEsockets.a Y

cd ..\..\..

cd "Release mingw_tdm_32\src\cpp"

ar rc libDSPE.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o DSP_Fourier.o DSP_AudioMixer.o DSPmodules_misc.o DSP_DOT.o
ranlib libDSPE.a
ar rc libDSPEsockets.a DSPsockets.o
ranlib libDSPEsockets.a
pause

copy /B libDSPE.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\rls\*.*
del libDSPE.a Y
copy /B libDSPEsockets.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\mingw_tdm_32\rls\*.*
del libDSPEsockets.a Y

cd ..\..\..

:End