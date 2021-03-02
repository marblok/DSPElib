rem comflag=-m32 - kompilacja w wersji 32-bitowej
set comflag=-m32

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
path=c:\Dev-Cpp\MinGW64\bin;c:\Dev-Cpp\MinGW64\x86_64-w64-mingw32\bin\;%path%

GOTO MakeExamples

:MakeExamples
cd ../_DSPE_lib_minGW_/Dev-cpp%comflag%/examples

mingw32-make clean
pause
mingw32-make -f makefile%comflag%

cd ../../Dsp_lib

:End