/*! Simple Digital Signal Processing Engine echo example.
 * \author Marek Blok
 * \date 2010.04.26
 * \date updated 2021.01.18
 */
#include <DSP_lib.h>

int main(void)
{
  DSP::Clock_ptr MasterClock;
  int temp;
  long int Fp;

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP_lib_version_string() << endl;
  DSP::log << endl;
  DSP::log << "Hello" << DSP::LogMode::second << "This is echo !!!" << endl;

  MasterClock=DSP::Clock::CreateMasterClock();

  DSPu_WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
  Fp = AudioIn.GetSamplingRate();

  DSPu_Addition Add(2U);
  DSPu_LoopDelay Delay(MasterClock, Fp/2);
  Delay.SetName("0.5s");
  DSPu_Amplifier Scale(0.7);
  Scale.SetName("0.7");

  DSP::u::AudioOutput AudioOut(Fp);

  // Examples of connections
  AudioIn.Output("out") >> Add.Input("in1");
  Add.Output("out") >> Delay.Input("in");
  Delay.Output("out") >> Scale.Input("in");
  Scale.Output("out") >> Add.Input("in2");
  // Note the reversed connection below !!!
  AudioOut.Input("in") << Add.Output("out");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "echo.dot");

  temp=0;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::LogMode::second << temp << endl;

    if (AudioIn.GetBytesRead() == 0)
      temp++;
  }
  while (temp < 30);

  DSP::Clock::FreeClocks();
  DSP::log << "MAIN" << DSP::LogMode::second << "end" << endl << DSP::LogMode::Error << endl;
 
  return 0;
}
