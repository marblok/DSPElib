IF exist makelib.bat GOTO AllOk
echo Working directory is not correct
rem pause
echo Trying "workspace/Dsp_lib"
cd workspace/Dsp_lib

IF exist makelib.bat GOTO AllOk
echo Working directory is definitely is not correct !!!
pause
GOTO End

:AllOK
IF not exist C:\MinGW\bin\ranlib.exe GOTO DriveE
path=C:\MinGW\bin\;%path%
rem path=E:\mingw\bin;E:\msys\1.0\bin\;%path%
GOTO MakeLib

:DriveE
path=E:\MinGW\bin\;%path%

:MakeLib

cd ..
mkdir _DSP_lib
mkdir _DSP_lib\dbg
mkdir _DSP_lib\include
cd DSP_lib

copy /B src\include\*.h    ..\_DSP_lib\include\*.*
rem copy /B src\include\DSP_lib.h       ..\_DSP_lib\include\*.*
rem copy /B src\include\DSP_IO.h        ..\_DSP_lib\include\*.*
rem copy /B src\include\DSPtypes.h      ..\_DSP_lib\include\*.*
rem copy /B src\include\DSPmodules.h    ..\_DSP_lib\include\*.*
rem copy /B src\include\DSPmodules2.h    ..\_DSP_lib\include\*.*
rem copy /B src\include\DSPclocks.h     ..\_DSP_lib\include\*.*
rem copy /B src\include\DSP_misc.h      ..\_DSP_lib\include\*.*
rem copy /B src\include\DSP_Fourier.h   ..\_DSP_lib\include\*.*
rem copy /B src\include\DSP_AudioMixer.h   ..\_DSP_lib\include\*.*

cd Debug\src\cpp

ar rc libDSP.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o MISCfunc.o DSP_Fourier.o DSP_AudioMixer.o
ranlib libDSP.a

copy /B libDSP.a ..\..\..\..\_DSP_lib\dbg\*.*
del libDSP.a Y

cd ..\..\..

cd Release\src\cpp

ar rc libDSP.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o MISCfunc.o DSP_Fourier.o DSP_AudioMixer.o
ranlib libDSP.a

copy /B libDSP.a ..\..\..\..\_DSP_lib\*.*
del libDSP.a Y

cd ..\..\..

:End