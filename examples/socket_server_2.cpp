/*! Digital Signal Processing Engine sockets usage example.
 *  Simple server application.
 * \author Marek Blok
 * \date 2010.02.09
 * \date updated 2021.01.18
 */
#include <DSP_sockets.h>
#include <DSP_lib.h>
#include <DSP_modules_misc.h>

int main(void)
{
  DSP::Clock_ptr MasterClock, MorseClock;
  int temp;
  long int Fp, Fp1;

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP::lib_version_string() << endl << endl;

  MasterClock = DSP::Clock::CreateMasterClock();

//  DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
//  Fp = AudioIn.GetSamplingRate();
  Fp = 8000;
  DSP::u::DDScos AudioIn(MasterClock, false, 1.0, DSP::Float(1000*DSP::M_PIx2/Fp));

  Fp1 = 1000;
  MorseClock = DSP::Clock::GetClock(MasterClock, 1, Fp/Fp1);
  DSP::u::MORSEkey MorseKey(MorseClock, 20, Fp1);
  MorseKey.AddString("Digital Signal Processing Engine library");
  DSP::u::Zeroinserter MorseHold(MorseClock, Fp/Fp1, true);
  MorseKey.Output("out") >> MorseHold.Input("in");

  DSP::u::RealMultiplication Mul(2);

  AudioIn.Output("out") >> Mul.Input("in1");
  MorseHold.Output("out") >> Mul.Input("in2");

  // use server socket
//  DSP::u::SocketOutput out_socket("0.0.0.0", false, 0x00000003);
  string bind_address = "0.0.0.0:10000";
  DSP::u::SocketOutput out_socket(bind_address, false, 0x00000003);
  out_socket.SetName(bind_address);
  Mul.Output("out") >> out_socket.Input("in");

  DSP::u::RawDecimator MorseDec(MorseClock, 5);
  DSP::u::FileOutput WAVEfile("morse_key.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp1/5);
  MorseKey.Output("out") >> MorseDec.Input("in");
  MorseDec.Output("out") >> WAVEfile.Input("in");
  //DSP::u::AudioOutput AudioOut(Fp);
  //Mul.Output("out") >> AudioOut.Input("in");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "socket_server_2.dot");

  //! \bug Implement WaitForAllConnections(MasterClock ??)
  out_socket.WaitForConnection();
  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::LogMode::second << temp << endl;
    temp++;
  }
  while (temp < 200); //(AudioIn.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();

  DSP::log << DSP::LogMode::Error << "MAIN" << DSP::LogMode::second << "end" << endl;
  //! \bug socket will be closed at application finish not at processing end

  return 0;
}
