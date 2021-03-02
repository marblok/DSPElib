/*! Simple asynchronous clocks usage example.
 * \author Marek Blok
 * \date 2010.02.26
 * \date updated 2021.01.18
 */
#include <DSP_lib.h>

//#define use_clock_trigger 1

int main(void)
{
  DSP::Clock_ptr MasterClock, SignalActivatedClock;
  int temp;
  long int Fp;

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP_lib_version_string() << endl << endl;

  MasterClock=DSP::Clock::CreateMasterClock();
  SignalActivatedClock=DSP::Clock::CreateMasterClock();


  DSPu_WaveInput AudioIn(MasterClock, "test.wav", ".", 1);
  Fp = AudioIn.GetSamplingRate();
  DSPu_ABS ABS(false);
  DSPu_Amplifier gain(-8);
  DSPu_Addition sum; sum.SetConstInput("in2",0.9);

  DSPu_Hold hold(SignalActivatedClock, MasterClock);
  DSPu_Amplifier gain2(8);

  DSPu_AudioOutput AudioOut(Fp, 1);
  DSPu_FILEoutput FileOut("test_out.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp);

  AudioIn.Output("out") >> ABS.Input("in");
  ABS.Output("out") >> gain.Input("in");
  gain.Output("out") >> sum.Input("in1");

  #ifdef use_clock_trigger
    DSPu_ClockTrigger CT(MasterClock, SignalActivatedClock);
    DSPu_SampleSelector sampler(MasterClock, SignalActivatedClock, false);

    sum.Output("out") >> CT.Input("act");
    sampler.Output("out") >> hold.Input("in");
  #else
    DSPu_SampleSelector sampler(MasterClock, SignalActivatedClock, true);

    sum.Output("out") >> sampler.Input("act");
    sampler.Output("out") >> hold.Input("in");
  #endif
  AudioIn.Output("out") >> sampler.Input("in");

  hold.Output("out") >> gain2.Input("in");

  gain2.Output("out") >> AudioOut.Input("in");
  FileOut.Input("in") << gain2.Output("out");

  DSP::Component::CheckInputsOfAllComponents();
  #ifdef use_clock_trigger
    DSP::Clock::SchemeToDOTfile(MasterClock, "asynchronous_CT.dot");
  #else
    DSP::Clock::SchemeToDOTfile(MasterClock, "asynchronous.dot");
  #endif

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::LogMode::second <<  temp << endl;
    temp++;
  }
  while (AudioIn.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  DSP::log << "MAIN" << DSP::LogMode::second << "end" << endl;

  return 0;
}
