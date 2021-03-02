/*! Simple macro usage example.
 * \author Marek Blok
 * \date 2010.02.24
 * \date updated 2021.01.18
 */
#include <DSP_lib.h>

class DDS_macro : public DSP::Macro
{
  private:
    DSPu_PCCC *alpha_state_correction;
    DSPu_Multiplication *Alpha_state_MUL;
    DSPu_LoopDelay *Alpha_state;

  public:
  DDS_macro(DSP::Clock_ptr Fp2Clock, DSP_float exp_w_n);

    ~DDS_macro(void);
};

DDS_macro::DDS_macro(DSP::Clock_ptr Fp2Clock, DSP_float w_n) : DSP::Macro("DDS", 1, 2)
{
  DSP_complex factor = DSP_complex(cos(w_n), sin(w_n));

  alpha_state_correction = NULL;
  Alpha_state_MUL = NULL;
  Alpha_state = NULL;

  MacroInitStarted();

  alpha_state_correction = new DSPu_PCCC;
  alpha_state_correction->SetConstInput("in.abs", 1.0);

  Alpha_state_MUL = new DSPu_Multiplication(0, 3); Alpha_state_MUL->SetConstInput("in3", factor);
  this->MacroInput("in") >> alpha_state_correction->Input("in.arg");
  alpha_state_correction->Output("out") >> Alpha_state_MUL->Input("in2");

  Alpha_state = new DSPu_LoopDelay(Fp2Clock, 1, 2); Alpha_state->SetState("in.re", 1.0);
  Alpha_state->Output("out") >> Alpha_state_MUL->Input("in1");
  Alpha_state_MUL->Output("out") >> Alpha_state->Input("in");

  Alpha_state->Output("out") >> this->MacroOutput("out");

  MacroInitFinished();
}

DDS_macro::~DDS_macro(void)
{
  delete alpha_state_correction;
  delete Alpha_state_MUL;
  delete Alpha_state;
}

int main(void)
{
  DSP::Clock_ptr MasterClock;
  int temp;
  long int Fp;

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP_lib_version_string() << endl << endl;

  MasterClock=DSP::Clock::CreateMasterClock();


  DSPu_WaveInput AudioIn(MasterClock, "test.wav", ".");
  Fp = AudioIn.GetSamplingRate();
  DDS_macro DDS(MasterClock, 0.15*DSP_M_PIx1);
  DSPu_Amplifier gain(1.0/2);
  DSPu_AudioOutput AudioOut(Fp, 2);
  DSPu_FILEoutput FileOut("test_out.wav", DSP::e::SampleType::ST_short, 2, DSP::e::FileType::FT_wav, Fp);

  AudioIn.Output("out") >> gain.Input("in");
  gain.Output("out") >> DDS.Input("in");

  //!\bug double splitters at macro output occur
  DDS.Output("out") >> AudioOut.Input("in");
  DDS.Output("out") >> FileOut.Input("in");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "macro_wraped.dot");
  DSP::Clock::SchemeToDOTfile(MasterClock, "macro_DDS.dot", &DDS);

  DDS.SetDOTmode(DSP_DOT_macro_unwrap);
  DSP::Clock::SchemeToDOTfile(MasterClock, "macro_unwraped.dot");

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::LogMode::second << temp << endl;
    temp++;
  }
  while (AudioIn.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  DSP::log << "MAIN" << DSP::LogMode::second << "end" << endl;

  return 0;
}
