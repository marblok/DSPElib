/*! Simple macro usage example.
 * \author Marek Blok
 * \date 2010.02.24
 * \date updated 2021.03.31
 */
#include <DSP_lib.h>

class DDS_macro : public DSP::Macro
{
  private:
    std::shared_ptr<DSP::u::PCCC> alpha_state_correction;
    std::shared_ptr<DSP::u::Multiplication> Alpha_state_MUL;
    std::shared_ptr<DSP::u::LoopDelay> Alpha_state;

  public:
    DDS_macro(DSP::Clock_ptr Fp2Clock, DSP::Float exp_w_n);

    ~DDS_macro(void);
};

DDS_macro::DDS_macro(DSP::Clock_ptr Fp2Clock, DSP::Float w_n) : DSP::Macro("DDS", 1, 2)
{
  DSP::Complex factor = DSP::Complex(cos(w_n), sin(w_n));

  alpha_state_correction = NULL;
  Alpha_state_MUL = NULL;
  Alpha_state = NULL;

  MacroInitStarted();

  alpha_state_correction = std::make_shared<DSP::u::PCCC>();
  alpha_state_correction->SetConstInput("in.abs", 1.0);

  Alpha_state_MUL = std::make_shared<DSP::u::Multiplication>(0, 3); Alpha_state_MUL->SetConstInput("in3", factor);
  this->MacroInput("in") >> alpha_state_correction->Input("in.arg");
  alpha_state_correction->Output("out") >> Alpha_state_MUL->Input("in2");

  Alpha_state = std::make_shared<DSP::u::LoopDelay>(Fp2Clock, 1, 2); Alpha_state->SetState("in.re", 1.0);
  Alpha_state->Output("out") >> Alpha_state_MUL->Input("in1");
  Alpha_state_MUL->Output("out") >> Alpha_state->Input("in");

  Alpha_state->Output("out") >> this->MacroOutput("out");

  MacroInitFinished();
}

DDS_macro::~DDS_macro(void)
{
  // this is not necessary, just remains after the previous use of delete 
  alpha_state_correction.reset();
  Alpha_state_MUL.reset();
  Alpha_state.reset();
}

int main(void)
{
  DSP::Clock_ptr MasterClock;
  int temp;
  long int Fp;

  DSP::log.SetLogState(DSP::e::LogState::console | DSP::e::LogState::file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP::lib_version_string() << endl << endl;

  MasterClock=DSP::Clock::CreateMasterClock();


  DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
  Fp = AudioIn.GetSamplingRate();
  DDS_macro DDS(MasterClock, 0.15*DSP::M_PIx1);
  DSP::u::Amplifier gain(1.0/2);
  DSP::u::AudioOutput AudioOut(Fp, 2);
  DSP::u::FileOutput FileOut("test_out.wav", DSP::e::SampleType::ST_short, 2, DSP::e::FileType::FT_wav, Fp);

  AudioIn.Output("out") >> gain.Input("in");
  gain.Output("out") >> DDS.Input("in");

  DDS.Output("out") >> AudioOut.Input("in");
  DDS.Output("out") >> FileOut.Input("in");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "macro_wraped.dot");
  DSP::Clock::SchemeToDOTfile(MasterClock, "macro_DDS.dot", &DDS);

  DDS.SetDOTmode(DSP::e::DOTmode::DOT_macro_unwrap);
  DSP::Clock::SchemeToDOTfile(MasterClock, "macro_unwraped.dot");

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::e::LogMode::second << temp << endl;
    temp++;
  }
  while (AudioIn.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  DSP::log << "MAIN" << DSP::e::LogMode::second << "end" << endl;

  return 0;
}
