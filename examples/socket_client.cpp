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

  DSP::log.SetLogState(DSP::e::LogState::console | DSP::e::LogState::file);
  DSP::log.SetLogFileName("log_file_client.log");

  DSP::log << DSP::lib_version_string() << std::endl << std::endl;

  MasterClock=DSP::Clock::CreateMasterClock();

  DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
  //Fp = AudioIn.GetSamplingRate();

  // use client socket
  //DSP::u::SocketInput in_socket(MasterClock, "153.19.48.213", true);
  DSP::u::SocketInput in_socket(MasterClock, "127.0.0.1", true, 0x00000002);
  Fp = 22050;

  DSP::u::SocketOutput out_socket("127.0.0.1", true, 0x00000001);
  DSP::u::AudioOutput AudioOut(Fp);

  in_socket.Output("out") >> AudioOut.Input("in");
  AudioIn.Output("out")   >> out_socket.Input("in");

  DSP::Component::CheckInputsOfAllComponents();

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::e::LogMode::second << temp << std::endl;
    temp++;
  }
  while (in_socket.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  DSP::log << DSP::e::LogMode::Error << "MAIN" << DSP::e::LogMode::second << "end" << std::endl;

  return 0;
}
