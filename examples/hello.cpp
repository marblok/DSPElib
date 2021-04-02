/*! Simple Digital Signal Processing Engine usage example.
 * \author Marek Blok
 * \date 2006.09.11
 * \date updated 2021.01.18
 */
#include <DSP_lib.h>

#ifndef _DSPE_TEST_
int main(void)
#else
#include "DSPE_examples.h"
int test_hello(void)
#endif // _DSPE_TEST_
{
  DSP::Clock_ptr MasterClock;
  string tekst;
  int temp;
  long int Fp;

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP_lib_version_string() << endl << endl;
  DSP::log << "Hello" << DSP::LogMode::second << "World !!!" << endl;
 
  MasterClock=DSP::Clock::CreateMasterClock();

  DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
  Fp = AudioIn.GetSamplingRate();

  DSP::u::AudioOutput AudioOut(Fp);

  AudioIn.Output("out") >> AudioOut.Input("in");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "hello.dot");

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::LogMode::second << temp << endl;
    temp++;
  }
  while (AudioIn.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  DSP::log << DSP::LogMode::Error << "MAIN" << DSP::LogMode::second << "end" << endl;

  return 0;
}
