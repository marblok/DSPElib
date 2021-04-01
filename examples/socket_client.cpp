/*! Digital Signal Processing Engine sockets usage example.
 *  Simple client application.
 * \author Marek Blok
 * \date 2008.10.06
 * \date updated 2021.01.18
 */
#include <DSP_sockets.h>
#include <DSP_lib.h>

int main(void)
{
  DSP::Clock_ptr MasterClock;
  int temp;
  long int Fp;

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file_client.log");

  DSP::log << DSP_lib_version_string() << endl << endl;

  MasterClock=DSP::Clock::CreateMasterClock();

  DSPu_WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
  //Fp = AudioIn.GetSamplingRate();

  // use client socket
  //DSP::u::SOCKETinput in_socket(MasterClock, "153.19.48.213", true);
  DSP::u::SOCKETinput in_socket(MasterClock, "127.0.0.1", true, 0x00000002);
  Fp = 22050;

  DSP::u::SOCKEToutput out_socket("127.0.0.1", true, 0x00000001);
  DSP::u::AudioOutput AudioOut(Fp);

  in_socket.Output("out") >> AudioOut.Input("in");
  AudioIn.Output("out")   >> out_socket.Input("in");

  DSP::Component::CheckInputsOfAllComponents();

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::LogMode::second << temp << endl;
    temp++;
  }
  while (in_socket.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  DSP::log << DSP::LogMode::Error << "MAIN" << DSP::LogMode::second << "end" << endl;

  return 0;
}
