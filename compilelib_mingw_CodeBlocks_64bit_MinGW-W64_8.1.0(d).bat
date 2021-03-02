rem Wersja wykorzystująca katalog %TEMP% do zapisu tymczasowych plików wyjściowych
rem ------------------------------------------------------------------------------
rem comflag=-m32 - kompilacja w wersji 32-bitowej (tdm-gcc 4.9.2)
set comflag=-m64
set com_ver=MinGW-W64_8.1.0

set flags=-fmax-errors=5 %comflag%

set start_dir=%cd%
rem set in_dir=%cd%
rem set in_dir=d:\Eclipse_Mars\git_folder\DSPElib\DSPE_lib
rem set in_dir=r:\DSPE_lib
set in_dir=d:\CodeBlocks_20_03\DSPElib

rem set workspace_dir=d:\Eclipse_Mars\workspace
rem set workspace_dir=R:\workspace
set workspace_dir=d:\CodeBlocks_20_03\DSPElib\workspace
mkdir "%workspace_dir%"


set filename=%in_dir%\compilelib_mingw_CodeBlocks_64bit_MinGW-W64_8.1.0(d).bat

IF exist "%filename%" GOTO AllOk
echo Source directory is not correct
rem pause
echo Trying "git_folder/DSPElib/DSPE_lib"
cd git_folder/DSPElib/DSPE_lib

IF exist "%filename%" GOTO AllOk
echo Source directory is definitely is not correct !!!
pause
GOTO End

:AllOK
rem path=d:\CodeBlocks\MinGW\bin;d:\CodeBlocks\MinGW\mingw32\bin\;d:\msys\1.0\bin\;%path%
path=d:\CodeBlocks_20_03\MinGW\bin;d:\CodeBlocks_20_03\MinGW\x86_64-w64-mingw32\bin\;d:\msys\1.0\bin\;%path%


:MakeLib

rem cd ..\..\..\workspace
cd /d "%workspace_dir%"
set library_dir=%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%
mkdir _DSPE_lib_minGW_
mkdir "%library_dir%"
mkdir "%library_dir%\rls"
mkdir "%library_dir%\dbg"
mkdir "%library_dir%\include"
mkdir "%library_dir%\examples"
mkdir "%library_dir%\examples\matlab"
mkdir "%library_dir%\toolbox"
rem cd ..\git_folder\DSPElib\DSPE_lib
cd /d "%in_dir%"


set tmp_out_dir_dbg=%TEMP%\DEBUG CodeBlocks%comflag%_%com_ver%
set tmp_out_dir_rls=%TEMP%\Release CodeBlocks%comflag%_%com_ver%

set tmp_out_dir=%tmp_out_dir_dbg%

mkdir "%tmp_out_dir%"
mkdir "%tmp_out_dir%\src"
mkdir "%tmp_out_dir%\src\cpp"
mkdir "%tmp_out_dir%\examples"
mkdir "%tmp_out_dir%\src\nvwa"

cd /d "%tmp_out_dir%"
rm -rf src/cpp/DSP_clocks.o src/Main.o src/Main.d src/cpp/DSP_clocks.d "%library_dir%/DSPE_lib_dbg.exe" src/cpp/DSP_DOT.d src/cpp/DSP_misc.o src/cpp/DSP_misc.d src/cpp/DSP_modules2.d src/cpp/DSP_AudioMixer.d src/cpp/DSP_modules2.o src/cpp/DSP_AudioMixer.o src/cpp/DSP_IO.o src/cpp/DSP_IO.d src/cpp/DSP_Fourier.d src/cpp/DSP_Fourier.o src/cpp/DSP_modules_misc.o src/cpp/DSP_modules_misc.d src/cpp/DSP_modules.d src/cpp/DSP_modules.o src/cpp/DSP_DOT.o src/cpp/DSP_sockets.o src/cpp/DSP_sockets.d
rm -rf src/cpp/DSP_logstream.o src/cpp/DSP_logstream.d

cd /d "%in_dir%"
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -fpermissive -o "%tmp_out_dir%\src\cpp\DSP_AudioMixer.o" .\src\cpp\DSP_AudioMixer.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_Fourier.o" .\src\cpp\DSP_Fourier.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_sockets.o" .\src\cpp\DSP_sockets.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_misc.o" .\src\cpp\DSP_misc.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_clocks.o" .\src\cpp\DSP_clocks.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_modules.o" .\src\cpp\DSP_modules.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_modules2.o" .\src\cpp\DSP_modules2.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_DOT.o" .\src\cpp\DSP_DOT.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\Main.o" .\src\Main.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_modules_misc.o" .\src\cpp\DSP_modules_misc.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_IO.o" .\src\cpp\DSP_IO.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_logstream.o" .\src\cpp\DSP_logstream.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\src\nvwa\debug_new.o" .\src\nvwa\debug_new.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish

g++ %flags% -D_DSPE_TEST_ -DWIN32 -DDEVCPP -I./src/include -I./src/include/dbg -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -o "%tmp_out_dir%\examples\hello.o" .\examples\hello.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish

cd /d "%tmp_out_dir%"
g++ %flags% -static -static-libgcc -static-libstdc++ -o "%library_dir%/DSPE_lib_dbg.exe" src\nvwa\debug_new.o src\cpp\DSP_sockets.o src\cpp\DSP_modules_misc.o src\cpp\DSP_modules2.o src\cpp\DSP_modules.o src\cpp\DSP_clocks.o src\cpp\DSP_misc.o src\cpp\DSP_IO.o src\cpp\DSP_Fourier.o src\cpp\DSP_DOT.o src\cpp\DSP_AudioMixer.o src\cpp\DSP_logstream.o examples\hello.o src\Main.o -lwinmm -lws2_32
IF %ERRORLEVEL% NEQ 0 Goto Finish


set tmp_out_dir=%tmp_out_dir_rls%
mkdir "%tmp_out_dir%"
mkdir "%tmp_out_dir%\src"
mkdir "%tmp_out_dir%\src\cpp"
mkdir "%tmp_out_dir%\examples"
mkdir "%tmp_out_dir%\src\nvwa"

cd /d "%tmp_out_dir%"
rm -rf src/cpp/DSP_clocks.o src/Main.o src/Main.d src/cpp/DSP_clocks.d "%library_dir%/DSPE_lib_rls.exe" src/cpp/DSP_DOT.d src/cpp/DSP_misc.o src/cpp/DSP_modules2.d src/cpp/DSP_AudioMixer.d src/cpp/DSP_modules2.o src/cpp/DSP_AudioMixer.o src/cpp/DSP_IO.o src/cpp/DSP_IO.d src/cpp/DSP_Fourier.d src/cpp/DSP_Fourier.o src/cpp/DSP_modules_misc.o src/cpp/DSP_modules_misc.d src/cpp/DSP_modules.d src/cpp/DSP_modules.o src/cpp/DSP_DOT.o src/cpp/DSP_sockets.o src/cpp/DSP_sockets.d
rm -rf src/cpp/DSP_logstream.o src/cpp/DSP_logstream.d

cd /d "%in_dir%"
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -fpermissive -o "%tmp_out_dir%\src\cpp\DSP_AudioMixer.o" .\src\cpp\DSP_AudioMixer.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_Fourier.o" .\src\cpp\DSP_Fourier.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_sockets.o" .\src\cpp\DSP_sockets.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_Misc.o" .\src\cpp\DSP_Misc.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_clocks.o" .\src\cpp\DSP_clocks.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_modules.o" .\src\cpp\DSP_modules.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_modules2.o" .\src\cpp\DSP_modules2.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_DOT.o" .\src\cpp\DSP_DOT.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\Main.o" .\src\Main.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_modules_misc.o" .\src\cpp\DSP_modules_misc.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_IO.o" .\src\cpp\DSP_IO.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish
g++ %flags% -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\src\cpp\DSP_logstream.o" .\src\cpp\DSP_logstream.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish

g++ %flags% -D_DSPE_TEST_ -DWIN32 -DDEVCPP -I./src/include -I./src/include/rls -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -o "%tmp_out_dir%\examples\hello.o" .\examples\hello.cpp
IF %ERRORLEVEL% NEQ 0 Goto Finish

cd /d "%tmp_out_dir%"
g++ %flags% -static -static-libgcc -static-libstdc++ -o "%library_dir%/DSPE_lib_rls.exe" src\cpp\DSP_sockets.o src\cpp\DSP_modules_misc.o src\cpp\DSP_modules2.o src\cpp\DSP_modules.o src\cpp\DSP_clocks.o src\cpp\DSP_misc.o src\cpp\DSP_IO.o src\cpp\DSP_Fourier.o src\cpp\DSP_DOT.o src\cpp\DSP_AudioMixer.o src\cpp\DSP_logstream.o examples\hello.o src\Main.o -lwinmm -lws2_32
IF %ERRORLEVEL% NEQ 0 Goto Finish

cd /d "%in_dir%"

copy /B examples\*.*                  "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\examples\*.*"
copy /B examples\matlab\*.*           "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\examples\matlab\*.*"
copy /B matlab\toolbox\*.m            "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\toolbox\*.m"
copy /B src\include\*.h               "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\include\*.*"
copy /B src\include\rls\DSP_setup.h   "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\rls\*.*"
copy /B src\include\dbg\DSP_setup.h   "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\dbg\*.*"

rem cd "Debug CodeBlocks%comflag%_%com_ver%\src\cpp"
cd /d "%tmp_out_dir_dbg%\src\cpp"

ar rc libDSPE.a DSP_IO.o DSP_misc.o DSP_clocks.o DSP_modules.o DSP_modules2.o DSP_Fourier.o DSP_AudioMixer.o DSP_modules_misc.o DSP_DOT.o DSP_logstream.o
ranlib libDSPE.a
ar rc libDSPEsockets.a DSP_sockets.o
ranlib libDSPEsockets.a
pause

copy /B libDSPE.a "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\dbg\*.*"
del libDSPE.a Y
copy /B libDSPEsockets.a "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\dbg\*.*"
del libDSPEsockets.a Y

rem cd ..\..\..

rem cd "Release CodeBlocks%comflag%_%com_ver%\src\cpp"
cd /d "%tmp_out_dir_rls%\src\cpp"

ar rc libDSPE.a DSP_IO.o DSP_misc.o DSP_clocks.o DSP_modules.o DSP_modules2.o DSP_Fourier.o DSP_AudioMixer.o DSP_modules_misc.o DSP_DOT.o DSP_logstream.o
ranlib libDSPE.a
ar rc libDSPEsockets.a DSP_sockets.o
ranlib libDSPEsockets.a
pause

copy /B libDSPE.a "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\rls\*.*"
del libDSPE.a Y
copy /B libDSPEsockets.a "%workspace_dir%\_DSPE_lib_minGW_\CodeBlocks%comflag%_%com_ver%\rls\*.*"
del libDSPEsockets.a Y

cd /d "%start_dir%"
rm -rf "%tmp_out_dir_dbg%"
rm -rf "%tmp_out_dir_rls%"

:Finish
rem cd ..\..\..
cd /d "%start_dir%"

:End