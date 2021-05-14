/*! Digital Signal Processing Engine Sound capture usage example.
 * \author Marek Blok
 * \date 2021.05.13
 * \date updated 2021.01.18
 */
#include <DSP_lib.h>

#ifndef INCLUDE_DSPE_EXAMPLES
int main(void)
#else
#include "DSPE_examples.h"
int test_sound_input(void)
#endif // INCLUDE_DSPE_EXAMPLES
{
  DSP::Clock_ptr MasterClock, AudioInClock;
  string tekst;
  int temp;
  long int Fp, Fp2;

  DSP::log.SetLogState(DSP::e::LogState::console | DSP::e::LogState::file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP::lib_version_string() << endl << endl;
  DSP::log << "Sound input example" << endl;
 
  MasterClock=DSP::Clock::CreateMasterClock();

  DSP::u::WaveInput WaveIn(MasterClock, "DSPElib.wav", ".");
  Fp = WaveIn.GetSamplingRate();

  DSP::u::AudioOutput AudioOut(Fp);

  WaveIn.Output("out") >> AudioOut.Input("in");

  Fp2 = 8000;
  long Fp_gcd = DSP::f::gcd(Fp, Fp2);
  AudioInClock=DSP::Clock::GetClock(MasterClock, Fp2 / Fp_gcd, Fp / Fp_gcd);
  DSP::u::AudioInput AudioIn(AudioInClock, 8000, 1);
  DSP::u::FileOutput WaveOut("captured_sample.wav",DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp2);

  AudioIn.Output("out") >> WaveOut.Input("in");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "sound_input.dot");

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::e::LogMode::second << temp << endl;
    temp++;
  }
  while (WaveIn.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  DSP::log << DSP::e::LogMode::Error << "MAIN" << DSP::e::LogMode::second << "end" << endl;

  return 0;
}
