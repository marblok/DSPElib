#!/bin/bash

if [ ! -f makelib_linux.sh ]; then
  echo Working directory is not correct
  #rem pause
  echo Trying "workspace/Dsp_lib"
  cd workspace/Dsp_lib

  if [ ! -f makelib_linux.sh ]; then 
    echo Working directory is definitely is not correct !!!
    return
  fi
fi

#:MakeLib

cd ..
mkdir _DSP_lib_Linux
mkdir _DSP_lib_Linux/rls
mkdir _DSP_lib_Linux/dbg
mkdir _DSP_lib_Linux/include
cd DSP_lib

cp src/include/*.h    ../_DSP_lib_Linux/include/
cp src/include/rls/DSP_setup.h   ../_DSP_lib_Linux/rls/
cp src/include/dbg/DSP_setup.h   ../_DSP_lib_Linux/dbg/


cd 'Debug Linux/src/cpp'

ar rc libDSP.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o MISCfunc.o DSP_Fourier.o DSP_AudioMixer.o
ranlib libDSP.a

cp libDSP.a ../../../../_DSP_lib_Linux/dbg/
rm libDSP.a

cd ../../..

cd 'Release Linux/src/cpp'

ar rc libDSP.a DSP_IO.o DSP_misc.o DSPclocks.o DSPmodules.o MISCfunc.o DSP_Fourier.o DSP_AudioMixer.o
ranlib libDSP.a

cp libDSP.a ../../../../_DSP_lib_Linux/rls/
rm libDSP.a

cd ../../..

#:End