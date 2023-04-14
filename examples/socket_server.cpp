/*! Digital Signal Processing Engine sockets usage example.
 *  Simple server application.
 * \author Marek Blok
 * \date 2008.10.06
 * \date updated 2021.01.18
 */
#include <DSP_sockets.h>
#include <DSP_lib.h>
#include <DSP_modules_misc.h>

int main(void)
{
  DSP::Clock_ptr MasterClock;
  int temp;
  long int Fp;

  DSP::log.SetLogState(DSP::e::LogState::console | DSP::e::LogState::file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP::lib_version_string() << std::endl << std::endl;

  MasterClock=DSP::Clock::CreateMasterClock();

//  DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
//  Fp = AudioIn.GetSamplingRate();
  Fp = 22050;
  DSP::u::DDScos AudioIn(MasterClock, false, 1.0, DSP::Float(1000*DSP::M_PIx2/Fp));
  DSP::u::MORSEkey MorseKey(MasterClock, 20, Fp);
  MorseKey.AddString("Digital Signal Processing Engine library");
  DSP::u::RealMultiplication Mul(2);

  AudioIn.Output("out") >> Mul.Input("in1");
  MorseKey.Output("out") >> Mul.Input("in2");

  DSP::u::SocketInput in_socket(MasterClock, "0.0.0.0", false, 0x00000001);
  DSP::u::AudioOutput AudioOut(Fp);
  in_socket.Output("out") >> AudioOut.Input("in");

  // use server socket
  DSP::u::SocketOutput out_socket("0.0.0.0", false, 0x00000002);
  Mul.Output("out") >> out_socket.Input("in");
//  DSP::u::FileOutput FileOut("server.flt", DSP::e::SampleType::ST_float, 1, DSP::e::FileType::FT_flt, Fp);
//  AudioIn.Output("out") >> FileOut.Input("in");

  DSP::Component::CheckInputsOfAllComponents();


  //! \bug Implement WaitForAllConnections(MasterClock ??)
  in_socket.WaitForConnection();
  out_socket.WaitForConnection();
  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::e::LogMode::second << temp << std::endl;
    temp++;
  }
  while (temp < 100); //(AudioIn.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  DSP::log << DSP::e::LogMode::Error << "MAIN" << DSP::e::LogMode::second << "end" << std::endl;
  //! \bug socket will be closed at application finish not at processing end

  return 0;
}
