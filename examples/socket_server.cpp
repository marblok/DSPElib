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

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP_lib_version_string() << endl << endl;

  MasterClock=DSP::Clock::CreateMasterClock();

//  DSPu_WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
//  Fp = AudioIn.GetSamplingRate();
  Fp = 22050;
  DSPu_DDScos AudioIn(MasterClock, false, 1.0, DSP::Float(1000*M_PIx2/Fp));
  DSPu_MORSEkey MorseKey(MasterClock, 20, Fp);
  MorseKey.AddString("Digital Signal Processing Engine library");
  DSPu_RealMultiplication Mul(2);

  AudioIn.Output("out") >> Mul.Input("in1");
  MorseKey.Output("out") >> Mul.Input("in2");

  DSPu_SOCKETinput in_socket(MasterClock, "0.0.0.0", false, 0x00000001);
  DSPu_AudioOutput AudioOut(Fp);
  in_socket.Output("out") >> AudioOut.Input("in");

  // use server socket
  DSPu_SOCKEToutput out_socket("0.0.0.0", false, 0x00000002);
  Mul.Output("out") >> out_socket.Input("in");
//  DSPu_FILEoutput FileOut("server.flt", DSP::e::SampleType::ST_float, 1, DSP::e::FileType::FT_flt, Fp);
//  AudioIn.Output("out") >> FileOut.Input("in");

  DSP::Component::CheckInputsOfAllComponents();


  //! \bug Implement WaitForAllConnections(MasterClock ??)
  in_socket.WaitForConnection();
  out_socket.WaitForConnection();
  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::LogMode::second << temp << endl;
    temp++;
  }
  while (temp < 100); //(AudioIn.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  DSP::log << DSP::LogMode::Error << "MAIN" << DSP::LogMode::second << "end" << endl;
  //! \bug socket will be closed at application finish not at processing end

  return 0;
}
