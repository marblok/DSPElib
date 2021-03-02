rem comflag=-m32 - kompilacja w wersji 32-bitowej
set comflag=-m64
set com_ver=MinGW-W64_8.1.0

set start_dir=%cd%
rem set in_dir=%cd%
set in_dir=d:\Eclipse_Mars\git_folder\DSPElib\DSPE_lib

rem set workspace_dir=d:\Eclipse_Mars\workspace
set workspace_dir=R:\workspace

set filename=%in_dir%\make_examples_mingw_CodeBlocks_64bit_8.1.0(c).bat

IF exist "%filename%" GOTO AllOk
echo Working directory is not correct
rem pause
echo Trying "workspace/Dsp_lib"
cd workspace/Dsp_lib

IF exist "%filename%" GOTO AllOk
echo Working directory is definitely is not correct !!!
pause
GOTO End

:AllOK
rem path=c:\Dev-Cpp\MinGW64\bin;c:\Dev-Cpp\MinGW64\x86_64-w64-mingw32\bin\;%path%
rem path=e:\Dev-Cpp\MinGW64\bin;e:\Dev-Cpp\MinGW64\x86_64-w64-mingw32\bin\;%path%
rem path=e:\CodeBlocks\MinGW\bin;e:\CodeBlocks\MinGW\mingw32\bin\;e:\msys\1.0\bin\;%path%
path=d:\CodeBlocks_20_03\MinGW\bin;d:\CodeBlocks_20_03\MinGW\x86_64-w64-mingw32\bin\;d:\msys\1.0\bin\;%path%


GOTO MakeExamples

:MakeExamples
rem cd ../_DSPE_lib_minGW_/CodeBlocks%comflag%_%com_ver%/examples
cd /d "%workspace_dir%/_DSPE_lib_minGW_/CodeBlocks%comflag%_%com_ver%/examples"

mingw32-make clean
pause
mingw32-make -f makefile%comflag%

cd %start_dir%

:End