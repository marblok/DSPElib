/*! Digital Signal Processing Engine example.
 *  Multirate algorithm.
 * 
 * Execution:
 *   multirate [input_mode]
 * where input_mode = 0, 1 or 2
 * 
 * \author Marek Blok 
 * \date 2006.09.23
 * \date updated 2021.03.31
 */
#include <memory>

#include <DSP_lib.h>

int main(int argn, char *args[])
{
  int input_mode = 0;
  int Fp1 = 22050;
  int Fp2 = 8000;
  DSP::LoadCoef coef_info;
  int L, M;
  int N_LPF_resample;
//  DSP::Float_ptr h_LPF_resample;
  DSP::Float_vector h_LPF_resample;
  float sound_factor = 0.5; // input sound gain - to avoid saturation in processed sound
  float factor = 0.6; // loop decay factor
  float delay = 0.7; // loop delay in seconds
  
  if (argn == 2)
  {
    input_mode = args[1][0]-'0';
  }
  
  /*************************************************************/
  std::map<std::string,std::shared_ptr<DSP::Component> > blocks;
  // DSP::Component *SoundIn = NULL;
  // DSPu_Amplifier *SoundInFactor = NULL;
  // DSPu_Const *Chirp_frequ = NULL;
  // DSPu_Accumulator *Chirp_Acum = NULL;
  // DSPu_Addition *Adder = NULL;
  // DSPu_SamplingRateConversion *Resampler_InOut = NULL;
  // DSPu_SamplingRateConversion *Resampler_OutIn = NULL;
  // DSPu_Amplifier *Factor = NULL;
  // DSPu_LoopDelay *Delay = NULL;
  // DSPu_AudioOutput *SoundOut = NULL;
  // DSPu_FILEoutput *FileOut = NULL;

  /*************************************************************/
  // Log file setup  
  DSP::log.SetLogFileName("log_file.log");
  DSP::log.SetLogState(DSP::E_LS_Mode::LS_file | DSP::E_LS_Mode::LS_console);

  DSP::log << DSP_lib_version_string() << endl << endl;
  /*************************************************************/
  DSP::Clock_ptr MasterClock, OutputClock; 
  MasterClock=DSP::Clock::CreateMasterClock();

  /*************************************************************/
  //Input source creation
  switch (input_mode)
  {
    case 2: // Chirp signal
      blocks["SoundIn"] = std::make_shared<DSPu_DDScos>(MasterClock);
    
      blocks["SoundIn"]->Convert2Block()->SetConstInput("ampl",1.0);  //Amplitude
      blocks["SoundIn"]->Convert2Block()->SetConstInput("phase",0.0); //Initial phase
        
      blocks["Chirp_frequ"] = std::make_shared<DSPu_Const>(MasterClock, DSP_M_PIx2*0.5/Fp1/2);
      blocks["Chirp_Acum"]  = std::make_shared<DSPu_Accumulator>();
      
      blocks["Chirp_frequ"]->Output("out") >> blocks["Chirp_Acum"]->Input("in");
      blocks["Chirp_Acum"]->Output("out")  >> blocks["SoundIn"]->Convert2Block()->Input("puls");
      break;
      
    case 1: //Input from soundcard
      blocks["SoundIn"] = std::make_shared<DSPu_AudioInput>(MasterClock, Fp1, 1, 16);
      break;
      
    case 0: //Input from file
    default:
      input_mode = 0;
      
      blocks["SoundIn"] = std::make_shared<DSPu_WaveInput>(MasterClock, "DSPElib.wav", ".", 1);
      Fp1 = ((DSPu_WaveInput *)blocks["SoundIn"]->Convert2Source())->GetSamplingRate();
      
      if (Fp1 != 22050)
      {
        DSP::log << DSP::LogMode::Error << "Input wave file's sampling rate must be 22050 Sa/s" << endl;
        
        blocks["SoundIn"].reset();
        return 1;
      }
      break;
  }
  
  blocks["SoundInFactor"] = std::make_shared<DSPu_Amplifier>(sound_factor);
  
  /* ***************************************** */
  // Conversion from 22050 Sa/s to 8000 Sa/s
  L = 160; M = 441;
  coef_info.Open("LPF_22050_8000.coef", "matlab");
  N_LPF_resample = coef_info.GetSize(0);
  
  if (N_LPF_resample < 1)
  {
    // problem
    N_LPF_resample = L*21;
    h_LPF_resample.resize(N_LPF_resample, 0.0);
    
    // design LPF filter with gain equal L
    DSP::f::Hann(N_LPF_resample, h_LPF_resample.data(), true);
    for (int n = 0; n < N_LPF_resample; n++)
    {
      h_LPF_resample[n] *= DSP::f::sinc((DSP_M_PIx1*3000)/(L*Fp1)*(n-N_LPF_resample/2)); 
      h_LPF_resample[n] *= 4.48; 
      h_LPF_resample[n] *= L; 
    }
     
    DSP::log << "Filter coeficients files should be generated beforehand using >>multirate_filters.m<<" << endl;
  }
  else
  {
    coef_info.Load(h_LPF_resample);
  }
   
  blocks["Resampler_InOut"] = std::make_shared<DSPu_SamplingRateConversion>(false, MasterClock, L, M, h_LPF_resample);
  OutputClock = blocks["Resampler_InOut"]->GetOutputClock();
  
  /* ***************************************** */
  // Conversion from 8000 Sa/s to 22050 Sa/s
  // \note filter can be reused
  L = 441; M = 160;
  for (int n = 0; n < N_LPF_resample; n++)
  {
      h_LPF_resample[n] /= M; 
      h_LPF_resample[n] *= L; 
  }
  blocks["Resampler_OutIn"] = std::make_shared<DSPu_SamplingRateConversion>(false, OutputClock, L, M, h_LPF_resample);
  
  /*************************************************************/
  // Output to the soundcard 
  blocks["SoundOut"] = std::make_shared<DSPu_AudioOutput>(Fp2, 1, 16);
  // Output to the mono 16bit *.wav file 
  blocks["FileOut"]  = std::make_shared<DSPu_FILEoutput>("multirate.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp2);

  /*************************************************************/
  blocks["Adder"] = std::make_shared<DSPu_Addition>(2);
  blocks["Factor"] = std::make_shared<DSPu_Amplifier>(factor);
  blocks["Delay"] = std::make_shared<DSPu_LoopDelay>(OutputClock, (int)(delay * Fp2));
  
  /*************************************************************/
  // Connections definitions
  blocks["SoundIn"]->Output("out") >> blocks["SoundInFactor"]->Input("in");
  blocks["SoundInFactor"]->Output("out") >> blocks["Adder"]->Input("real_in1");
  blocks["Adder"]->Output("out") >> blocks["Resampler_InOut"]->Input("in");
  blocks["Resampler_InOut"]->Output("out") >> blocks["SoundOut"]->Input("in");
  blocks["Resampler_InOut"]->Output("out") >> blocks["FileOut"]->Input("in");
  blocks["Resampler_InOut"]->Output("out") >> blocks["Factor"]->Input("in");
  blocks["Factor"]->Output("out") >> blocks["Delay"]->Input("in");
  blocks["Delay"]->Output("out") >> blocks["Resampler_OutIn"]->Input("in");
  blocks["Resampler_OutIn"]->Output("out") >> blocks["Adder"]->Input("real_in2");
  
  
  /////////////////////////////////
  // check if there are signals 
  // connected to all inputs  
  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "multirate.dot");

  // *********************************** // 
  // *********************************** //
  int SamplesInSegment = 512;
  __int64 NoOfSamplesProcessed = 0;
  // 10 seconds
  #define MAX_SAMPLES_TO_PROCESS 10*8000 
  while(NoOfSamplesProcessed < MAX_SAMPLES_TO_PROCESS) 
  {

    // ********************************************************** //
    DSP::Clock::Execute(OutputClock, SamplesInSegment);
    // ********************************************************** //
    
    if (input_mode == 0)
    {
      if (((DSPu_WaveInput *)blocks["SoundIn"]->Convert2Source())->GetBytesRead() > 0)
      {
        NoOfSamplesProcessed = 0; // Play the whole file
      } 
      else // Play five seconds more
        if (NoOfSamplesProcessed < MAX_SAMPLES_TO_PROCESS - 5*8000) 
          NoOfSamplesProcessed = MAX_SAMPLES_TO_PROCESS - 5*8000;
    }

    NoOfSamplesProcessed += SamplesInSegment;
    // ********************************************************** //
  }
  
  
  
  /*************************************************************/
  blocks["SoundIn"].reset();
  blocks["SoundInFactor"].reset();
  blocks["Chirp_Acum"].reset();
  blocks["Chirp_frequ"].reset();
  blocks["Adder"].reset();
  blocks["Resampler_InOut"].reset();
  blocks["Resampler_OutIn"].reset();
  blocks["Factor"].reset();
  blocks["Delay"].reset();
  blocks["SoundOut"].reset();
  blocks["FileOut"].reset();
  
  /*************************************************************/
  DSP::Clock::ListOfAllComponents();
  
  /*************************************************************/
  DSP::Clock::FreeClocks();
  
  return 0;
}
