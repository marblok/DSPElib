/*! Digital Signal Processing Engine example.
 *  Multirate algorithm.
 * 
 * Execution:
 *   multirate [input_mode]
 * where input_mode = 0, 1 or 2
 * 
 * \author Marek Blok 
 * \date 2006.09.23
 * \date updated 2021.01.18
 */
#include <DSP_lib.h>

int main(int argn, char *args[])
{
  int input_mode = 0;
  int Fp1 = 22050;
  int Fp2 = 8000;
  DSP::LoadCoef coef_info;
  int L, M;
  int N_LPF_resample;
//  DSP_float_ptr h_LPF_resample;
  DSP_float_vector h_LPF_resample;
  float sound_factor = 0.5; // input sound gain - to avoid saturation in processed sound
  float factor = 0.6; // loop decay factor
  float delay = 0.7; // loop delay in seconds
  
  if (argn == 2)
  {
    input_mode = args[1][0]-'0';
  }
  
  /*************************************************************/
  DSP::Component *SoundIn = NULL;
  DSPu_Amplifier *SoundInFactor = NULL;
  DSPu_Const *Chirp_frequ = NULL;
  DSPu_Accumulator *Chirp_Acum = NULL;
  DSPu_Addition *Adder = NULL;
  DSPu_SamplingRateConversion *Resampler_InOut = NULL;
  DSPu_SamplingRateConversion *Resampler_OutIn = NULL;
  DSPu_Amplifier *Factor = NULL;
  DSPu_LoopDelay *Delay = NULL;
  DSPu_AudioOutput *SoundOut = NULL;
  DSPu_FILEoutput *FileOut = NULL;

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
      SoundIn = new DSPu_DDScos(MasterClock);
    
      SoundIn->Convert2Block()->SetConstInput("ampl",1.0);  //Amplitude
      SoundIn->Convert2Block()->SetConstInput("phase",0.0); //Initial phase
        
      Chirp_frequ = new DSPu_Const(MasterClock, DSP_M_PIx2*0.5/Fp1/2);
      Chirp_Acum  = new DSPu_Accumulator();
      
      Chirp_frequ->Output("out") >> Chirp_Acum->Input("in");
      Chirp_Acum->Output("out") >>  SoundIn->Convert2Block()->Input("puls");
      break;
      
    case 1: //Input from soundcard
      SoundIn = new DSPu_AudioInput(MasterClock, Fp1, 1, 16);
      break;
      
    case 0: //Input from file
    default:
      input_mode = 0;
      
      SoundIn = new DSPu_WaveInput(MasterClock, "test.wav", ".", 1);
      Fp1 = ((DSPu_WaveInput *)SoundIn->Convert2Source())->GetSamplingRate();
      
      if (Fp1 != 22050)
      {
        DSP::log << DSP::LogMode::Error << "Input wave file's sampling rate must be 22050 Sa/s" << endl;
        
        delete SoundIn;
        return 1;
      }
      break;
  }
  
  SoundInFactor = new DSPu_Amplifier(sound_factor);
  
  /* ***************************************** */
  // Conversion from 22050 Sa/s to 8000 Sa/s
  L = 160; M = 441;
  coef_info.Open("LPF_22050_8000.coef", "matlab");
  N_LPF_resample = coef_info.GetSize(0);
  
  if (N_LPF_resample < 1)
  {
    // problem
    N_LPF_resample = L*21;
//    h_LPF_resample = new DSP_float[N_LPF_resample];
//    memset(h_LPF_resample, 0, N_LPF_resample*sizeof(DSP_float));
    h_LPF_resample.resize(N_LPF_resample, 0.0);
    
    // design LPF filter with gain equal L
    DSPf_Hann(N_LPF_resample, h_LPF_resample.data(), true);
    for (int n = 0; n < N_LPF_resample; n++)
    {
      h_LPF_resample[n] *= DSPf_sinc((DSP_M_PIx1*3000)/(L*Fp1)*(n-N_LPF_resample/2)); 
      h_LPF_resample[n] *= 4.48; 
      h_LPF_resample[n] *= L; 
    }
     
    DSP::log << "Filter coeficients files should be generated beforehand using >>multirate_filters.m<<" << endl;
  }
  else
  {
    coef_info.Load(h_LPF_resample);
  }
   
//  Resampler_InOut = new DSPu_SamplingRateConversion(MasterClock, L, M, N_LPF_resample, h_LPF_resample);
  Resampler_InOut = new DSPu_SamplingRateConversion(false, MasterClock, L, M, h_LPF_resample);
  OutputClock = Resampler_InOut->GetOutputClock();
  
  /* ***************************************** */
  // Conversion from 8000 Sa/s to 22050 Sa/s
  // \note filter can be reused
  L = 441; M = 160;
  for (int n = 0; n < N_LPF_resample; n++)
  {
      h_LPF_resample[n] /= M; 
      h_LPF_resample[n] *= L; 
  }
//  Resampler_OutIn = new DSPu_SamplingRateConversion(OutputClock, L, M, N_LPF_resample, h_LPF_resample);
  Resampler_OutIn = new DSPu_SamplingRateConversion(false, OutputClock, L, M, h_LPF_resample);
  
//  delete [] h_LPF_resample; h_LPF_resample = NULL;
  
  /*************************************************************/
  // Output to the soundcard 
  SoundOut = new DSPu_AudioOutput(Fp2, 1, 16);
  // Output to the mono 16bit *.wav file 
  FileOut  = new DSPu_FILEoutput("multirate.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp2);

  /*************************************************************/
  Adder = new DSPu_Addition(2);
  Factor = new DSPu_Amplifier(factor);
  Delay = new DSPu_LoopDelay(OutputClock, (int)(delay * Fp2));
  
  /*************************************************************/
  // Connections definitions
  SoundIn->Output("out") >> SoundInFactor->Input("in");
  SoundInFactor->Output("out") >> Adder->Input("real_in1");
  Adder->Output("out") >> Resampler_InOut->Input("in");
  Resampler_InOut->Output("out") >> SoundOut->Input("in");
  Resampler_InOut->Output("out") >> FileOut->Input("in");
  Resampler_InOut->Output("out") >> Factor->Input("in");
  Factor->Output("out") >> Delay->Input("in");
  Delay->Output("out") >> Resampler_OutIn->Input("in");
  Resampler_OutIn->Output("out") >> Adder->Input("real_in2");
  
  
  /////////////////////////////////
  // check if there are signals 
  // connected to all inputs  
  DSP::Component::CheckInputsOfAllComponents();
  
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
      if (((DSPu_WaveInput *)SoundIn->Convert2Source())->GetBytesRead() > 0)
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
  delete SoundIn;
  delete SoundInFactor;
  if (Chirp_Acum != NULL)
    delete Chirp_Acum;
  if (Chirp_frequ != NULL)
    delete Chirp_frequ;
  delete Adder;
  delete Resampler_InOut;
  delete Resampler_OutIn;
  delete Factor;
  delete Delay;
  delete SoundOut;
  delete FileOut;
  
  /*************************************************************/
  DSP::Clock::ListOfAllComponents();
  
  /*************************************************************/
  DSP::Clock::FreeClocks();
  
  return 0;
}
