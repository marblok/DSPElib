/*! Simple asynchronous clocks usage example.
 * \author Marek Blok
 * \date 2010.02.26
 * \date updated 2021.04.01
 */
#include <DSP_lib.h>

//#define use_clock_trigger 1

int main(void)
{
  DSP::Clock_ptr MasterClock, SignalActivatedClock;
  int temp;
  long int Fp;

  DSP::log.SetLogState(DSP::e::LogState::console | DSP::e::LogState::file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP::lib_version_string() << endl << endl;

  MasterClock=DSP::Clock::CreateMasterClock();
  SignalActivatedClock=DSP::Clock::CreateMasterClock();


  DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".", 1);
  Fp = AudioIn.GetSamplingRate();
  DSP::u::ABS ABS(false);
  DSP::u::Amplifier gain(-8);
  DSP::u::Addition sum; sum.SetConstInput("in2",0.9);

  DSP::u::Hold hold(SignalActivatedClock, MasterClock);
  DSP::u::Amplifier gain2(8);

  DSP::u::AudioOutput AudioOut(Fp, 1);
  DSP::u::FileOutput FileOut("test_out.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp);

  AudioIn.Output("out") >> ABS.Input("in");
  ABS.Output("out") >> gain.Input("in");
  gain.Output("out") >> sum.Input("in1");

  #ifdef use_clock_trigger
    DSP::u::ClockTrigger CT(MasterClock, SignalActivatedClock);
    DSP::u::SampleSelector sampler(MasterClock, SignalActivatedClock, false);

    sum.Output("out") >> CT.Input("act");
    sampler.Output("out") >> hold.Input("in");
  #else
    DSP::u::SampleSelector sampler(MasterClock, SignalActivatedClock, true);

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

    DSP::log << "MAIN" << DSP::e::LogMode::second <<  temp << endl;
    temp++;
  }
  while (AudioIn.GetBytesRead() != 0);
  // process a bit more so the buffered samples are also sent to output
  DSP::Clock::Execute(MasterClock, Fp/8);

  DSP::Clock::FreeClocks();
  DSP::log << "MAIN" << DSP::e::LogMode::second << "end" << endl;

  return 0;
}
