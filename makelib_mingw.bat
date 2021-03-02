IF exist makelib_mingw.bat GOTO AllOk
echo Working directory is not correct
rem pause
echo Trying "workspace/Dsp_lib"
cd workspace/Dsp_lib

IF exist makelib_mingw.bat GOTO AllOk
echo Working directory is definitely is not correct !!!
pause
GOTO End

:AllOK
IF not exist E:\mingw\bin\ranlib.exe GOTO DriveC
rem path=E:\cygwin\bin;%path%
path=E:\mingw\bin;E:\msys\1.0\bin\;%path%
GOTO MakeLib

:DriveC
path=C:\mingw\bin;C:\msys\1.0\bin\;%path%

:MakeLib

cd ..
mkdir _DSP_lib_minGW
mkdir _DSP_lib_minGW\rls
mkdir _DSP_lib_minGW\dbg
mkdir _DSP_lib_minGW\include
mkdir _DSP_lib_minGW\examples
mkdir _DSP_lib_minGW\examples\matlab
mkdir _DSP_lib_minGW\toolbox
cd DSP_lib

copy /B examples\*.*        ..\_DSP_lib_minGW\examples\*.*
copy /B examples\matlab\*.*        ..\_DSP_lib_minGW\examples\matlab\*.*
copy /B matlab\toolbox\*.m  ..\_DSP_lib_minGW\toolbox\*.m
copy /B src\include\*.h     ..\_DSP_lib_minGW\include\*.*
copy /B src\include\rls\DSP_setup.h   ..\_DSP_lib_minGW\rls\*.*
copy /B src\include\dbg\DSP_setup.h   ..\_DSP_lib_minGW\dbg\*.*

rem copy /B src\include\DSP_lib.h       ..\_DSP_lib_minGW\include\*.*
rem copy /B src\include\DSP_IO.h        ..\_DSP_lib_minGW\include\*.*
rem copy /B src\include\DSPtypes.h      ..\_DSP_lib_minGW\include\*.*
rem copy /B src\include\DSPmodules.h    ..\_DSP_lib_minGW\include\*.*
rem copy /B src\include\DSPclocks.h     ..\_DSP_lib_minGW\include\*.*
rem copy /B src\include\DSP_misc.h      ..\_DSP_lib_minGW\include\*.*
rem copy /B src\include\DSP_Fourier.h   ..\_DSP_lib_minGW\include\*.*
rem copy /B src\include\DSP_AudioMixer.h   ..\_DSP_lib_minGW\include\*.*

cd Debug\src\cpp

ar rc libDSP.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o MISCfunc.o DSP_Fourier.o DSP_AudioMixer.o DSPmodules_misc.o DSP_DOT.o
ranlib libDSP.a
ar rc libDSPsockets.a DSPsockets.o
ranlib libDSPsockets.a

copy /B libDSP.a ..\..\..\..\_DSP_lib_minGW\dbg\*.*
del libDSP.a Y
copy /B libDSPsockets.a ..\..\..\..\_DSP_lib_minGW\dbg\*.*
del libDSPsockets.a Y

cd ..\..\..

cd Release\src\cpp

ar rc libDSP.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o DSPmodules2.o MISCfunc.o DSP_Fourier.o DSP_AudioMixer.o DSPmodules_misc.o DSP_DOT.o
ranlib libDSP.a
ar rc libDSPsockets.a DSPsockets.o
ranlib libDSPsockets.a

copy /B libDSP.a ..\..\..\..\_DSP_lib_minGW\rls\*.*
del libDSP.a Y
copy /B libDSPsockets.a ..\..\..\..\_DSP_lib_minGW\rls\*.*
del libDSPsockets.a Y

cd ..\..\..

:End