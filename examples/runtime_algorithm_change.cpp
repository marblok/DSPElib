/*! Simple Digital Signal Processing Engine usage example.
 * \author Marek Blok
 * \date 2023.10.31
 * \date updated 2023.10.31
 */
#include <DSP_lib.h>

class Branch_A {
    private:
        std::unique_ptr <DSP::u::Const> Const;
        std::unique_ptr <DSP::u::Addition> Add;
    public:
        //! adds blocks 
        void create_branch(DSP::Clock_ptr clock, DSP::output &input_Signal, DSP::input &output_Signal, DSP::Float value = 0.5) {
            Const.reset(new DSP::u::Const(clock, value));
            std::stringstream ss;
            ss << value;
            Const->SetName(ss.str());
            
            Add.reset(new DSP::u::Addition(2U, 0U));

            input_Signal >> Add->Input("in1.re");
            Const->Output("out") >> Add->Input("in2.re");

            Add->Output("out") >> output_Signal;
        }
        void clear_branch(void) {
            Const.reset(nullptr);
            Add.reset(nullptr);
        }
}; 

#ifndef INCLUDE_DSPE_EXAMPLES
int main(void)
#else
#include "DSPE_examples.h"
int test_runtime_algorithm_change(void)
#endif // INCLUDE_DSPE_EXAMPLES
{
  DSP::Clock_ptr MasterClock;
  std::string tekst;
  int temp;
  long int Fp;
  Branch_A branch_A;

  DSP::log.SetLogState(DSP::e::LogState::console | DSP::e::LogState::file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP::lib_version_string() << std::endl << std::endl;
  DSP::log << "Hello" << DSP::e::LogMode::second << "World !!!" << std::endl;
 
  MasterClock=DSP::Clock::CreateMasterClock();

#ifndef INCLUDE_DSPE_EXAMPLES
  DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
#else
  DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", "../examples");
#endif // INCLUDE_DSPE_EXAMPLES
  Fp = AudioIn.GetSamplingRate();

  DSP::u::AudioOutput AudioOut(Fp);

//   AudioIn.Output("out") >> AudioOut.Input("in");
  branch_A.create_branch(MasterClock, AudioIn.Output("out"), AudioOut.Input("in"), -0.5);

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Component::ListOfAllComponents(true);
  DSP::Clock::SchemeToDOTfile(MasterClock, "test_runtime_algorithm_change.dot");

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::e::LogMode::second << temp << std::endl;

    branch_A.clear_branch();
    DSP::Component::ListOfAllComponents(true);
    {
        std::stringstream filename;
        filename << "test_runtime_algorithm_change_" << 2*temp - 1 << ".dot";
        DSP::Clock::SchemeToDOTfile(MasterClock, filename.str());
    }
    if ((temp % 2) == 0) 
        branch_A.create_branch(MasterClock, AudioIn.Output("out"), AudioOut.Input("in"), -0.5);
    else
        branch_A.create_branch(MasterClock, AudioIn.Output("out"), AudioOut.Input("in"), +0.5);
    DSP::Component::ListOfAllComponents(true);

    // AudioIn.Output("out") >> AudioOut.Input("in"); // !!! TODO add way to remove connection without removing blocks???
    {
        std::stringstream filename;
        filename << "test_runtime_algorithm_change_" << 2*temp << ".dot";
        DSP::Clock::SchemeToDOTfile(MasterClock, filename.str());
    }

    temp++;
  }
  while (AudioIn.GetBytesRead() != 0);

  DSP::Clock::SchemeToDOTfile(MasterClock, "test_runtime_algorithm_change_B.dot");

  branch_A.clear_branch();
  DSP::Clock::FreeClocks();
  DSP::log << DSP::e::LogMode::Error << "MAIN" << DSP::e::LogMode::second << "end" << std::endl;

  return 0;
}
