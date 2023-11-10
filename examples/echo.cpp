/*! Simple Digital Signal Processing Engine echo example.
 * \author Marek Blok
 * \date 2010.04.26
 * \date updated 2023.11.10
 */
#include <DSP_lib.h>

int main(void)
{
  DSP::Clock_ptr MasterClock;
  int loop_counter, final_counter;
  long int Fp;

  DSP::log.SetLogState(DSP::e::LogState::console | DSP::e::LogState::file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP::lib_version_string() << std::endl;
  DSP::log << std::endl;
  DSP::log << "Hello" << DSP::e::LogMode::second << "This is echo !!!" << std::endl;

  MasterClock=DSP::Clock::CreateMasterClock();

  DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
  Fp = AudioIn.GetSamplingRate();

  DSP::u::Addition Add(2U);
  DSP::u::AdjustableDelay AdjustableDelay(1*Fp, 0); // delay up to 1 second starting from 0 seconds
  AdjustableDelay.SetName("max 1 sec");
  DSP::u::LoopDelay Delay(MasterClock, Fp/80); // basic delay - block needed for digital feedback loop
  Delay.SetName("1/80 sec");
  DSP::u::Amplifier Scale(0.7);
  Scale.SetName("0.7");

  DSP::u::AudioOutput AudioOut(Fp);

  // Examples of connections
  AudioIn.Output("out") >> Add.Input("in1");
  Add.Output("out") >> AdjustableDelay.Input("in");
  AdjustableDelay.Output("out") >> Delay.Input("in");
  Delay.Output("out") >> Scale.Input("in");
  Scale.Output("out") >> Add.Input("in2");
  // Note the reversed connection below !!!
  AudioOut.Input("in") << Add.Output("out");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "echo.dot");

  bool use_const_delay = false;  // set to true for constant feedback lop delay
  unsigned int actual_delay = 0;
  loop_counter=0;
  final_counter=0;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/16);
    if (use_const_delay == true) {
      actual_delay = AdjustableDelay.SetDelay(Fp/2); // set new delay - will not get larger then the max_delay setup on block construction
    } 
    else {
      actual_delay = AdjustableDelay.SetDelay(loop_counter*Fp/100); // set new delay - will not get larger then the max_delay setup on block construction
    }

    DSP::log << "MAIN" << DSP::e::LogMode::second << final_counter << ": actual_delay=" << actual_delay << " samples." << std::endl;

    if (AudioIn.GetBytesRead() == 0)
      final_counter++;
    loop_counter++;
  }
  while (final_counter < 30);

  DSP::Clock::FreeClocks();
  DSP::log << "MAIN" << DSP::e::LogMode::second << "end" << std::endl << DSP::e::LogMode::Error << std::endl;
 
  return 0;
}
