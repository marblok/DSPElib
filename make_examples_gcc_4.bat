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

path=E:\mingw\bin;E:\msys\1.0\bin\;%path%
GOTO MakeExamples

:DriveC
path=C:\mingw_gcc_4\bin;C:\msys\1.0\bin\;%path%

:MakeExamples
cd ../_DSP_lib_minGW_gcc4/examples

make clean
make

cd ../../Dsp_lib

:End