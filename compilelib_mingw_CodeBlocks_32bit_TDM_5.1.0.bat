rem comflag=-m32 - kompilacja w wersji 32-bitowej (tdm-gcc 4.9.2)
set comflag=-m32
set com_ver=TDM_5.1.0


IF exist compilelib_mingw_CodeBlocks_32bit_TDM_5.1.0.bat GOTO AllOk
echo Working directory is not correct
rem pause
echo Trying "git_folder/DSPElib/DSPE_lib"
cd git_folder/DSPElib/DSPE_lib

IF exist compilelib_mingw_CodeBlocks_32bit_TDM_5.1.0.bat GOTO AllOk
echo Working directory is definitely is not correct !!!
pause
GOTO End

:AllOK
path=d:\CodeBlocks\MinGW\bin;d:\CodeBlocks\MinGW\mingw32\bin\;d:\msys\1.0\bin\;%path%


:MakeLib

cd ..\..\..\workspace
mkdir _DSPE_lib_minGW_
mkdir _DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%
mkdir _DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\rls
mkdir _DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\dbg
mkdir _DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\include
mkdir _DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\examples
mkdir _DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\examples\matlab
mkdir _DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\toolbox
cd ..\git_folder\DSPElib\DSPE_lib


mkdir "Debug CodeBlocks%comflag%_%com_ver%"
mkdir "Debug CodeBlocks%comflag%_%com_ver%\src"
mkdir "Debug CodeBlocks%comflag%_%com_ver%\src\cpp"
mkdir "Debug CodeBlocks%comflag%_%com_ver%\src\nvwa"
cd "Debug CodeBlocks%comflag%_%com_ver%"
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
mkdir "Release CodeBlocks%comflag%_%com_ver%"
mkdir "Release CodeBlocks%comflag%_%com_ver%\src"
mkdir "Release CodeBlocks%comflag%_%com_ver%\src\cpp"
mkdir "Release CodeBlocks%comflag%_%com_ver%\src\nvwa"
cd Release CodeBlocks%comflag%_%com_ver%"
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

copy /B examples\*.*        ..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\examples\*.*
copy /B examples\matlab\*.*        ..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\examples\matlab\*.*
copy /B matlab\toolbox\*.m  ..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\toolbox\*.m
copy /B src\include\*.h     ..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\include\*.*
copy /B src\include\rls\DSP_setup.h   ..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\rls\*.*
copy /B src\include\dbg\DSP_setup.h   ..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\dbg\*.*

cd "Debug CodeBlocks%comflag%_%com_ver%\src\cpp"

ar rc libDSPE.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o DSP_Fourier.o DSP_AudioMixer.o DSPmodules_misc.o DSP_DOT.o
ranlib libDSPE.a
ar rc libDSPEsockets.a DSPsockets.o
ranlib libDSPEsockets.a
pause

copy /B libDSPE.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\dbg\*.*
del libDSPE.a Y
copy /B libDSPEsockets.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\dbg\*.*
del libDSPEsockets.a Y

cd ..\..\..

cd "Release CodeBlocks%comflag%_%com_ver%\src\cpp"

ar rc libDSPE.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o DSP_Fourier.o DSP_AudioMixer.o DSPmodules_misc.o DSP_DOT.o
ranlib libDSPE.a
ar rc libDSPEsockets.a DSPsockets.o
ranlib libDSPEsockets.a
pause

copy /B libDSPE.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\rls\*.*
del libDSPE.a Y
copy /B libDSPEsockets.a ..\..\..\..\..\..\workspace\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\rls\*.*
del libDSPEsockets.a Y

cd ..\..\..

:End