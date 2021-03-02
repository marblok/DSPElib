rem comflag=-m32 - kompilacja w wersji 32-bitowej
set comflag=-m32
set com_ver=TDM_5.1.0

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
rem path=c:\Dev-Cpp\MinGW64\bin;c:\Dev-Cpp\MinGW64\x86_64-w64-mingw32\bin\;%path%
rem path=e:\Dev-Cpp\MinGW64\bin;e:\Dev-Cpp\MinGW64\x86_64-w64-mingw32\bin\;%path%
path=e:\CodeBlocks\MinGW\bin;e:\CodeBlocks\MinGW\mingw32\bin\;d:\msys\1.0\bin\;%path%


GOTO MakeExamples

:MakeExamples
rem cd ../_DSPE_lib_minGW_/CodeBlocks%comflag%_%com_ver%/examples
cd ../../../workspace/_DSPE_lib_minGW_/CodeBlocks%comflag%_%com_ver%/examples

mingw32-make clean
pause
mingw32-make -f makefile%comflag%

cd ../../../_DSPE_lib_minGW_

:End