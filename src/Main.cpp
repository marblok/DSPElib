#include <time.h>

#include <DSP_sockets.h>
#include <DSP_lib.h>
//#include <DSP_AudioMixer.h>
#include "Main.h"

#include <memory>
#include <algorithm>
#include <iomanip>

#ifdef INCLUDE_DSPE_EXAMPLES
  #include "../../examples/DSPE_examples.h"
#endif // INCLUDE_DSPE_EXAMPLES

#ifndef CLK_TCK
  #define CLK_TCK CLOCKS_PER_SEC
#endif

/*
int main1(int argc, char*argv)
{
// _/_*
//  DSP::Float A_in[]= {3.0, 1.0, 1.0,
//                     0.5, 1.0, 1.5,
//                     0.1, 1.0, 0.1};
//  DSP::Float X[3];
//  DSP::Float B_in[]= { 1, 1, 1};
//  SolveMatrixEqu_prec(3, A_in, X, B_in);
// _*_/
  T_WAVEchunk temp_WAVEparams;
  unsigned long Fs, Fs1, Fs2;
  char *InputFileName="03-pcz.wav";
  DSP::Clock_ptr MasterClock;

  MasterClock=NULL;

  //Wczytanie sygnau z pliku wav  (w tym weryfikacja zgodności parametrw)
  Fs=48000;
  if (DSP::f::GetWAVEfileParams(InputFileName, ".", &temp_WAVEparams))
  {
    if (temp_WAVEparams.nSamplesPerSec!=Fs)
    {
      #ifdef TestCompilation
        printf("WAVE with sampling rate different from %li Sa/s\n", Fs);
      #endif
      return 1;
    }
  }
  else
  {
    #ifdef TestCompilation
      printf("Can't GetWAVEfileParams\n");
    #endif
    return 1;
  }
  DSP::u::WaveInput WaveInput(MasterClock, InputFileName, ".");
  WaveInput.DefineOutput("x[n]",0);

// ********************************************************* //
// Demodulacja do pasma podstawowego:
// ********************************************************* //
  //Demodulacja pilota z czstotliwoci 15800 do czstotliwosci 1500
  // //cao? czyli kanay i pilot symetrycznie : (12*200+200+100=2700)/2
  // tylko kana?y symetrycznie (12*200=2400)/2+300=1500
  DSP::u::DDScos  Heter1(MasterClock, true, 1.0, (M_PIx2*(1500-15800))/Fs, 0.0);
  Heter1.DefineOutput("x",0);
  Heter1.DefineOutput("y",1);

  DSP::u::Multiplication Mul1(1, 1);
  Mul1.DefineOutput("x",0);
  Mul1.DefineOutput("y",1);

  // / * decymacja z filtacj dolnopasmow * /
  // / * \todo implement this using: DSP::u::SamplingRateConversion * /
  DSP::Float h_FIR[]={-0.00469737357514, -0.02189844354959,  -0.01863144426339,
                      0.08394044867363,  0.27332881783852,   0.37579179117021,
                      0.27332881783852,  0.08394044867363,  -0.01863144426339,
                     -0.02189844354959, -0.00469737357514};
  DSP::u::FIR filter1(true, 11, h_FIR);
  filter1.DefineOutput("x",0);
  filter1.DefineOutput("y",1);

  DSP::u::RawDecimator Decymator1(MasterClock, 15, 2); Fs1=Fs/15;
  Decymator1.DefineOutput("x",0);
  Decymator1.DefineOutput("y",1);

  // ****************************
  DSP::u::Splitter Splitter1(true, 2);
  Splitter1.DefineOutput("x1",0);
  Splitter1.DefineOutput("y1",1);
  Splitter1.DefineOutput("x2",2);
  Splitter1.DefineOutput("y2",3);


  // / * i zapis do plikw wav * /
  DSP::u::FileOutput DemodOut("Demod_complex.out", DSP::e::SampleType::ST_float, 2);
//  WaveInput.SetOutput(0, &DemodOut, 0); // WaveInput.SetOutput(1, &DemodOut, 1);

  DSP::_connect_class::connect_to_block(WaveInput.Output("x[n]")(),) &Mul1, 0);
  DSP::_connect_class::connect_to_block(Heter1.Output("x"), &Mul1, 1);
  DSP::_connect_class::connect_to_block(Heter1.Output("y"), &Mul1, 2);
  DSP::_connect_class::connect_to_block(Mul1.Output("x"), &filter1, 0);
  DSP::_connect_class::connect_to_block(Mul1.Output("y"), &filter1, 1);
  DSP::_connect_class::connect_to_block(filter1.Output("x"), &Decymator1, 0);
  DSP::_connect_class::connect_to_block(filter1.Output("y"), &Decymator1, 1);
  DSP::_connect_class::connect_to_block(Decymator1.Output("x"), &Splitter1, 0);
  DSP::_connect_class::connect_to_block(Decymator1.Output("y"), &Splitter1, 1);
  DSP::_connect_class::connect_to_block(Splitter1.Output("x1"), &DemodOut, 0);
  DSP::_connect_class::connect_to_block(Splitter1.Output("y1"), &DemodOut, 1);


// ********************************************************* //
// Dostrojenie si do czstotliwoci pilota: (praca z szybko?ci prbkowania Fs1
// ********************************************************* //
  DSP::Clock_ptr Clock1;
  DSP::Clock_ptr Clock2;
  Clock1=Decymator1.GetOutputClock(0);
  Clock2=DSP::Clock::GetClock(Clock1,1,15); //This clock is needed before the block which creates it is created !!

  DSP::u::LoopDelay  PilotFreqDelay(Clock2, 1);
//  DSP::u::Delay  PilotFreqDelay(1);
  PilotFreqDelay.DefineOutput("out", 0);

  DSP::u::Zeroinserter Hold1(Clock2, 15, true);
  Hold1.DefineOutput("out", 0);

  DSP::u::Addition   PilotFreqSum(2U,0U); PilotFreqSum.SetName("PilotFreqSum");
  PilotFreqSum.DefineOutput("sum", 0);

  DSP::u::Splitter   PilotFreqSplit(3U);
  PilotFreqSplit.DefineOutput("out1", 0);
  PilotFreqSplit.DefineOutput("out2", 1);
  PilotFreqSplit.DefineOutput("out3", 2);


// i zapis do pliku
DSP::u::FileOutput PilotErrorLoopOut("Pilot_loop_error.out", DSP::e::SampleType::ST_float, 1);


  // / * \todo heterodynowanie korekcyjne pilota do czstotliwoci 1500Hz * /
  DSP::u::DDScos CorrHeter(Clock1, true);
  CorrHeter.SetConstInput("ampl",1.0); //Amplitude
//CorrHeter.SetConstInput(1,0.0); //Angular frequency
  CorrHeter.SetConstInput("phase",0.0); //Initial phase
  CorrHeter.DefineOutput("re", 0);
  CorrHeter.DefineOutput("im", 1);

  DSP::u::Multiplication CorrMul(0, 2);
  CorrMul.DefineOutput("re", 0);
  CorrMul.DefineOutput("im", 1);


  DSP::_connect_class::connect_to_block(PilotFreqSplit.Output("out1"), &Hold1, 0);
  DSP::_connect_class::connect_to_block(PilotFreqSplit.Output("out2"), &PilotFreqSum, 0);
  DSP::_connect_class::connect_to_block(PilotFreqSum.Output("sum"),&PilotFreqDelay,0);
  DSP::_connect_class::connect_to_block(PilotFreqDelay.Output("out"),&PilotFreqSplit,0);

  DSP::_connect_class::connect_to_block(PilotFreqSplit.Output("out3"), &PilotErrorLoopOut, 0);
  DSP::_connect_class::connect_to_block(Hold1.Output("out"),&CorrHeter,1);

  DSP::_connect_class::connect_to_block(CorrHeter.Output("re"),&CorrMul,0);
  DSP::_connect_class::connect_to_block(CorrHeter.Output("im"),&CorrMul,1);

  DSP::_connect_class::connect_to_block(Splitter1.Output("x2"),&CorrMul,2);
  DSP::_connect_class::connect_to_block(Splitter1.Output("y2"),&CorrMul,3);


//  / *   \todo heterodynowanie pilota do zerowej pulsacji * /
    DSP::u::DDScos  Heter2(Clock1, true, 1.0, (M_PIx2*(-1500))/Fs1, 0.0);
    Heter2.DefineOutput("re", 0);
    Heter2.DefineOutput("im", 1);

    DSP::u::Multiplication Mul2(0, 2);
    Mul2.DefineOutput("re",0);
    Mul2.DefineOutput("im",1);

//  / *   \todo filtracja i decymacja np. 16-krotna * /
    DSP::Float h_FIR2[]={-0.00469737357514, -0.02189844354959,  -0.01863144426339,
                        0.08394044867363,  0.27332881783852,   0.37579179117021,
                        0.27332881783852,  0.08394044867363,  -0.01863144426339,
                       -0.02189844354959, -0.00469737357514};
    DSP::u::FIR filter2(true, 11, h_FIR2);
    filter2.DefineOutput("re", 0);
    filter2.DefineOutput("im", 1);

    DSP::u::RawDecimator Decymator2(Clock1, 15, 2); Fs2=Fs1/15;
    Decymator2.SetName("Pilot decimator");
    Decymator2.DefineOutput("out.re", 0);
    Decymator2.DefineOutput("out.im", 1);

    Clock2=Decymator2.GetOutputClock(0);


    DSP::_connect_class::connect_to_block(Heter2.Output("re"),&Mul2,0);
    DSP::_connect_class::connect_to_block(Heter2.Output("im"),&Mul2,1);
    DSP::_connect_class::connect_to_block(CorrMul.Output("re"),&Mul2,2);
    DSP::_connect_class::connect_to_block(CorrMul.Output("im"),&Mul2,3);

    DSP::_connect_class::connect_to_block(Mul2.Output("re"), &filter2, 0);
    DSP::_connect_class::connect_to_block(Mul2.Output("im"), &filter2, 1);

    DSP::_connect_class::connect_to_block(filter2.Output("re"), &Decymator2, 0);
    DSP::_connect_class::connect_to_block(filter2.Output("im"), &Decymator2, 1);

// / * \todo implement SetOutput version that puts a new block
// * between the block to which output we connect and
// * the block witch is connected to this output.
// *
// * This would allow for add splitter and FileOutput without
// * interfering into the current algorithm structure
// * /

  // / *   \todo wyznaczenie b?du heterodyny korekcyjnej * /
    //y=imag(y1).*real(y2)-real(y1).*imag(y2);

  // * \todo potrzebny modu ARW poniewa? dalszy modu jest
  // * wraliwy na amplitud sygnau i najlepiej pracuje dla amplitudy =1
  // * - istotne jednak, ?eby niezalenie od amplitudy sygnau
  // * zawsze ka samo dziaao
  // * /
//    DSP::Float h1[]={0.0, 23.0};
//    DSP::Float h2[]={23.0, 0.0, -23.0};
    DSP::Float h1[]={0.0, 1.0};
    DSP::Float h2[]={1.0, 0.0, -1.0};
    DSP::u::FIR filter_y1(true, 2, h1);
    filter_y1.DefineOutput("re",0);
    filter_y1.DefineOutput("im",1);
    DSP::u::FIR filter_y2(true, 3, h2);
    filter_y2.DefineOutput("re",0);
    filter_y2.DefineOutput("im",1);
    DSP::u::Splitter Splitter_y(true, 3);
    Splitter_y.DefineOutput("out1.re",0);
    Splitter_y.DefineOutput("out1.im",1);
    Splitter_y.DefineOutput("out2.re",2);
    Splitter_y.DefineOutput("out2.im",3);
    Splitter_y.DefineOutput("out3.re",4);
    Splitter_y.DefineOutput("out3.im",5);
//    Decymator2.SetOutput(0, &Splitter_y, 0); Decymator2.SetOutput(1, &Splitter_y, 1);

    // / * i zapis do pliku* /
    DSP::u::FileOutput PilotOut("Pilot_corr.out", DSP::e::SampleType::ST_float, 2);

    DSP::_connect_class::connect_to_block(Splitter_y.Output("out1.re"), &filter_y1, 0);
    DSP::_connect_class::connect_to_block(Splitter_y.Output("out1.im"), &filter_y1, 1);
    DSP::_connect_class::connect_to_block(Splitter_y.Output("out2.re"), &filter_y2, 0);
    DSP::_connect_class::connect_to_block(Splitter_y.Output("out2.im"), &filter_y2, 1);
    DSP::_connect_class::connect_to_block(Splitter_y.Output("out3.re"), &PilotOut, 0);
    DSP::_connect_class::connect_to_block(Splitter_y.Output("out3.im"), &PilotOut, 1);

DSP::u::Splitter HeterSplit(true, 2);
HeterSplit.DefineOutput("out1.re", 0);
HeterSplit.DefineOutput("out1.im", 1);
HeterSplit.DefineOutput("out2.re", 2);
HeterSplit.DefineOutput("out2.im", 3);

// / * i zapis do pliku* /
DSP::u::FileOutput HeterOut("Heter.out", DSP::e::SampleType::ST_float, 2);

    DSP::u::Multiplication Mul_y1_im(2);
    Mul_y1_im.DefineOutput("out");

    DSP::u::Multiplication Mul_y1_re(3);
//    Mul_y1_re.SetConstInput(2,-1.0);
    Mul_y1_re.SetConstInput("in2",-1.0);
    Mul_y1_re.DefineOutput("out");

    DSP::u::Addition Sum_err(2); Sum_err.SetName("Sum_err");
    Sum_err.DefineOutput("sum");


    DSP::_connect_class::connect_to_block(Decymator2.Output("out.re"),&HeterSplit,0);
    DSP::_connect_class::connect_to_block(Decymator2.Output("out.im"),&HeterSplit,1);
    DSP::_connect_class::connect_to_block(HeterSplit.Output("out2.re"),&HeterOut, 0);
    DSP::_connect_class::connect_to_block(HeterSplit.Output("out2.im"),&HeterOut, 1);
    DSP::_connect_class::connect_to_block(HeterSplit.Output("out1.re"),&Splitter_y,0);
    DSP::_connect_class::connect_to_block(HeterSplit.Output("out1.im"),&Splitter_y,1);

    DSP::_connect_class::connect_to_block(filter_y1.Output("im"), &Mul_y1_im, 0);
    DSP::_connect_class::connect_to_block(filter_y2.Output("re"), &Mul_y1_im, 1);

    DSP::_connect_class::connect_to_block(filter_y1.Output("re"), &Mul_y1_re, 0);
    DSP::_connect_class::connect_to_block(filter_y2.Output("im"), &Mul_y1_re, 1);

    DSP::_connect_class::connect_to_block(Mul_y1_re.Output("out"), &Sum_err, 0);
    DSP::_connect_class::connect_to_block(Mul_y1_im.Output("out"), &Sum_err, 1);


    DSP::u::Amplifier GainError(+0.5);
    GainError.DefineOutput("out");

    DSP::u::Splitter ErrorSplit(2U); ErrorSplit.SetName("ErrorSplit");
    ErrorSplit.DefineOutput("out1",0);
    ErrorSplit.DefineOutput("out2",1);

  // * i zapis do pliku* /
  DSP::u::FileOutput PilotErrorOut("Pilot_error.out", DSP::e::SampleType::ST_float, 1);


    DSP::_connect_class::connect_to_block(Sum_err.Output("sum"),&ErrorSplit, 0);
    DSP::_connect_class::connect_to_block(ErrorSplit.Output("out1"),&GainError,0);
    DSP::_connect_class::connect_to_block(ErrorSplit.Output("out2"), &PilotErrorOut, 0);
    DSP::_connect_class::connect_to_block(GainError.Output("out"),&PilotFreqSum, 1);



//  DSP::Clock::Execute(25);
//  DSP::Clock::Execute(4410000); // ~4s DDScos + two outputs
//  DSP::Clock::Execute(44100*3); // ~4s DDScos + two outputs
  DSP::Clock::Execute(WaveInput.GetOutputClock(), 44100*30); // ~4s DDScos + two outputs

  DSP::Clock::FreeClocks();
  return 0;
}
*/

int test_1(int argc, char*argv[])
{
  UNUSED_ARGUMENT(argc);
  UNUSED_ARGUMENT(argv);

/*
  DSP::Complex test;

  test.set(-1.0);
  printf("x: %f+i%f -> %f\n", test.re, test.im, test.angle());
  getchar();
  for (int ind=0; ind<20; ind++)
  {
    test.set(cos(M_PIx2/20*ind), sin(M_PIx2/20*ind));
    printf("%i: %f+i%f -> %f\n", ind, test.re, test.im, test.angle());
    getchar();
  }
*/

  DSP::Clock_ptr MasterClock, Fp1Zegar;
  MasterClock=NULL;
  string InputName="DSPElib.wav";
  string OutputName="output.flt";
//  DSP::u::WaveInput WaveInput(InputName, ".", 1);
  DSP::u::FileInput WaveInput(MasterClock, InputName, 1, DSP::e::SampleType::ST_float);
  DSP::u::FileOutput FileOutput(OutputName, DSP::e::SampleType::ST_float, 2);

  DSP::u::CMPO test;
//  DSP::u::Angle test2;

  WaveInput.Output("out") >> test.Input("in.re");
  //  WaveInput.Output("out") >> test.Input("in.im");
  WaveInput.Output("out")>> test.Input("in.im");
//  test.Output("out") >> test2.Input("in");
  DSP::File_ptr temp_file = WaveInput.Convert2File();

  test.Output("out") >> FileOutput.Input("in");

  Fp1Zegar=WaveInput.GetOutputClock();
  DSP::Clock::Execute(Fp1Zegar, 120000);
  temp_file->GetBytesRead();

  DSP::Clock::FreeClocks();
  return 0;

/*
  string InputName="input.flt";
  string CoefficientsName="hI_coef.flt";
  string OutputName="output.flt";
  DSP::Clock_ptr Fp1Zegar, Fp2Zegar;

  int N_hI;
  DSP::Float hI[256];
  N_hI=DSP_ReadCoefficientsFromFile(hI, 256,
                     CoefficientsName, ".", DSP::e::SampleType::ST_float);

  DSP::u::FileInput FileInput(InputName, DSP::e::SampleType::ST_float, 2);
  Fp1Zegar=FileInput.GetOutputClock();

  //real input OK
  DSP::u::SamplingRateConversion Convertor(true, 2,5, N_hI, hI, Fp1Zegar);
  Fp2Zegar=Convertor.GetOutputClock();

  DSP::u::FileOutput FileOutput(OutputName, DSP::e::SampleType::ST_float, 2);
  DSP::u::Vacuum Empty1(true);

  FileInput.Output("out") >> Convertor.Input("in");
  Convertor.Output("out") >> Empty1.Input("in");
  Convertor.Output("out") >> FileOutput.Input("in");
*/

/*
//  DSP::u::FIR test
  // real input + real coefficients // OK
  // real input + complex coefficients // OK
  // complex input + real coefficients // OK
  // complex input + complex coefficients // OK
  string InputName="delta.flt";
  string InputName2="delta_im.flt";
  DSP::Clock_ptr MainZegar;

  DSP::u::FileInput FileInput(InputName);
  FileInput.DefineOutput("file");
  MainZegar=FileInput.GetOutputClock();

  DSP::u::FileInput FileInput2(InputName2);
  FileInput2.DefineOutput("file");


  DSP::Float h_re[]={1.0, 1.5, -1.2};
  DSP::Complex h_C[]={{1.0, -1.0},
                     {1.5, 2.5},
                     {-1.2, -1.2},
                     {0.0, 0.1}};
  DSP::u::FIR Filter1(true, 4, h_re);
  Filter1.DefineOutput("re", 0);
  Filter1.DefineOutput("im", 1);
  Filter1.DefineOutput("out", 0,1);

  DSP::u::FileOutput ResponseOut("Response.out", DSP::e::SampleType::ST_float, 2);
  ResponseOut.DefineInput("complex.in",0,1);

  FileInput.Output("file"), &Filter1, 0);
  FileInput2.Output("file"), &Filter1, 1);

//  Filter1.Output("re"), &ResponseOut,0);
//  Filter1.Output("im"), &ResponseOut,1);
  Filter1.Output("out"), ResponseOut.Input("complex.in");
*/

/*
//  DSP::u::IIR test

  DSP::Float a[]={1.0, -0.25};  DSP::Float b[]={1.0, 0.0};
//  DSP::u::IIR Filter1(2, a); // OK
//  DSP::u::IIR Filter1(2, a, 2, b); // OK
//  DSP::u::IIR Filter1(1, a, 1, b); // OK
//  DSP::Float_ptr temp=NULL; DSP::u::IIR Filter1(0, temp, 0, temp); // OK
//  DSP::u::IIR Filter1(true, 2, a, 2, b); // OK

  DSP::Complex aC[2], bC[2];
  aC[0].set(0.5, 0.1);  aC[1].set(-0.2, -0.3);
  bC[0].set(1.0);  bC[1].set(0.0);
//  DSP::u::IIR Filter1(2, aC); // OK
//  DSP::u::IIR Filter1(2, aC, 2, bC); // OK
//  DSP::u::IIR Filter1(1, aC, 1, bC); // OK
//  DSP::Complex_ptr temp=NULL; DSP::u::IIR Filter1(0, temp, 0, temp); // OK
  DSP::u::IIR Filter1(true, 2, aC, 2, bC); // OK
  FileInput.SetOutput(0,&Filter1, 0);
  FileInput2.SetOutput(0,&Filter1, 1);

//TEST bC different than {1.0, 0.0} // OK
//TEST a[0] or aC[0] != 1.0 // OK

//TEST DSP::u::IIR with complex input // OK

  DSP::u::FileOutput ResponseOut("Response.out", DSP::e::SampleType::ST_float, 2);
  Filter1.SetOutput(0, &ResponseOut, 0);
  Filter1.SetOutput(1, &ResponseOut, 1);
*/

/*
  DSP::Clock::Execute(24, Fp2Zegar);
  DSP::Clock::Execute(26, Fp2Zegar);

  DSP::Clock::FreeClocks();
  return 0;
*/
}








long int CheckFs(const string &WaveName, const string &Dir)
{
  DSP::T_WAVEchunk WaveParams;

  if (DSP::f::GetWAVEfileParams(WaveName, Dir, &WaveParams))
    return WaveParams.nSamplesPerSec;
  return 0;
}

int ReadResamplerCoef(const string &name, const string &dir)
{ //reads impulse response length
  DSP::Float_vector temp;
  int FilterOffset;
  int N_LPF;

  //ignore filter specification details
  DSP::f::ReadCoefficientsFromFile(temp, 1, name, dir, DSP::e::SampleType::ST_float,2*sizeof(float));
  FilterOffset=int(temp[0]);
  FilterOffset*=5;
  FilterOffset+=3;

  temp.resize(1);  temp[0]=0.0;
  //read filter response length
  DSP::f::ReadCoefficientsFromFile(temp, 1, name, dir, DSP::e::SampleType::ST_float,uint32_t(FilterOffset*sizeof(float)));
  N_LPF=int(temp[0]);

  return N_LPF;
}

long int ReadResamplerCoef(const string &name, const string &dir, DSP::Float_vector h_LPF)
{
  long int Fs;
  DSP::Float_vector temp;
  int FilterOffset;
  int N_LPF;

  DSP::f::ReadCoefficientsFromFile(temp, 1, name, dir, DSP::e::SampleType::ST_float,0);
  Fs=(long int)(temp[0]);

//  DSP_ReadCoefficientsFromFile(&temp, 1,
//    name, dir, DSP::e::SampleType::ST_float,sizeof(float));
//  M1=(int)temp;

  //ignore filter specification details
  DSP::f::ReadCoefficientsFromFile(temp, 1, name, dir, DSP::e::SampleType::ST_float,2*sizeof(float));
  FilterOffset=int(temp[0]);
  FilterOffset*=5;
  FilterOffset+=3;

  //read filter response length
  DSP::f::ReadCoefficientsFromFile(temp, 1, name, dir, DSP::e::SampleType::ST_float,uint32_t(FilterOffset*sizeof(float)));
  N_LPF=int(temp[0]);

  // read filter coefficients
  DSP::f::ReadCoefficientsFromFile(h_LPF, N_LPF,
    name, dir, DSP::e::SampleType::ST_float,uint32_t((FilterOffset+1)*sizeof(float)));
  return Fs;
}

int ReadIIRCoef(const string &name, const string &dir)
{ //Read IIR filter order
  DSP::Float_vector temp(1);
  uint32_t ile;

  temp[0]=0.0;
  ile=DSP::f::ReadCoefficientsFromFile(temp, 1, name, dir, DSP::e::SampleType::ST_float, 0);
  assert(ile > 0);

  return int(temp[0]);
}

int ReadIIRCoef(const string &name, const string &dir, int Order,
                DSP::Complex_vector &A, DSP::Complex_vector &B)
{ //returns mean bandpass group delay (int)
  DSP::Float_vector temp;

  DSP::f::ReadCoefficientsFromFile(A, Order+1,
    name, dir, DSP::e::SampleType::ST_float, sizeof(float));
  DSP::f::ReadCoefficientsFromFile(B, Order+1,
    name, dir, DSP::e::SampleType::ST_float, uint32_t((2*(Order+1)+1)*sizeof(float)));

  DSP::f::ReadCoefficientsFromFile(temp, 1, name, dir, DSP::e::SampleType::ST_float, (unsigned long)((4*(Order+1)+1)*sizeof(float)));

  return int(temp[0]);
}

void Process(long int Fs, const string &WaveName, const string &Dir)
{
  int ind;

  DSP::Clock_ptr MasterClock, Zegar1, Zegar2, Zegar3;
  long int Decym_Fs, Decym2_Fs;
  int M1, L1, N_LPF1;
  DSP::Float_vector h_LPF1;
  int M2, N_LPF2;
  DSP::Float_vector h_LPF2;

  int IIR_order[5], IIR_delay[5];
  DSP::Complex_vector IIR_A[5], IIR_B[5];
  string IIR_name="IIR_1.flt";

  long czas0, czas1, czas2;

  MasterClock=NULL;
  L1=1; czas0=clock();

  // ************************************************* //

  DSP::u::WaveInput FileIn(MasterClock, WaveName, Dir, 1);
//  DSP::u::FileInput FileIn("waves/DSPElib.wav", DSP::e::SampleType::ST_short, 1);
//  DSP::u::COSpulse FileIn(false, 1.0, 0.0, 0.0, 0.0, 0, 10, 0, NULL);
//  DSP::u::COSpulse FileIn(false, 1.0);
  Zegar1=FileIn.GetOutputClock();


  // ************************************************* //

  N_LPF1=ReadResamplerCoef("Stage1_fp44100.flt", "coef");
  h_LPF1.resize(N_LPF1);
  Decym_Fs=ReadResamplerCoef("Stage1_fp44100.flt", "coef", h_LPF1);
  if (Fs>Decym_Fs) //input sampling frequency is too high
  {
    return;
  }
  if (Fs<Decym_Fs) // check whether there is common dividor
  {
    if ((Decym_Fs % Fs)!=0)
    {
      return;
    }
    L1=Decym_Fs / Fs;
  }
  M1=Decym_Fs/8000;

  DSP::u::SamplingRateConversion Stage1(false, Zegar1, L1, M1, h_LPF1);
  Zegar2=Stage1.GetOutputClock();


  // ************************************************* //

  N_LPF2=ReadResamplerCoef("Stage2_fp4000.flt", "coef");
  h_LPF2.resize(N_LPF2);
  Decym2_Fs=ReadResamplerCoef("Stage2_fp4000.flt", "coef", h_LPF2);
  if (Decym2_Fs != 4000) //input sampling frequency is wrong
  {
    return;
  }
  M2=4;


  DSP::u::SamplingRateConversion Stage2(false, Zegar2, 1, M2, h_LPF2);
  Zegar3=Stage2.GetOutputClock();


  // ************************************************* //
  for (ind=0; ind<5; ind++)
  {
    IIR_order[ind]=0;
    IIR_A[ind].clear();
    IIR_B[ind].clear();
    IIR_delay[ind]=0;
  }
  for (ind=0; ind<5; ind++)
  {
    IIR_name[4]=(char)(ind+49); //'1', '2', ...
    IIR_order[ind]=ReadIIRCoef(IIR_name, "coef");
    if (IIR_order[ind]==0)
      break;
    IIR_A[ind].resize(IIR_order[ind]+1);
    IIR_B[ind].resize(IIR_order[ind]+1);
    IIR_delay[ind]=ReadIIRCoef(IIR_name, "coef", IIR_order[ind],
                IIR_A[ind], IIR_B[ind]);
  }
  if (IIR_order[4] == 0)
  {
    for (ind=0; ind<5; ind++)
    {
      IIR_A[ind].clear();
      IIR_B[ind].clear();
    }
    return;
  }

  DSP::u::Splitter SplitterIIR(6U);
  DSP::u::Delay IIRdelay1(IIR_delay[0]);
  DSP::u::Delay IIRdelay2(IIR_delay[1]);
  DSP::u::Delay IIRdelay3(IIR_delay[2]);
  DSP::u::Delay IIRdelay4(IIR_delay[3]);
  DSP::u::Delay IIRdelay5(IIR_delay[4]);

/*
  DSP::Complex h_temp[]={{1.0, 0.0}};
  DSP::u::FIR IIRfilter1(false, 1, h_temp);
  DSP::u::FIR IIRfilter2(false, 1, h_temp);
  DSP::u::FIR IIRfilter3(false, 1, h_temp);
  DSP::u::FIR IIRfilter4(false, 1, h_temp);
  DSP::u::FIR IIRfilter5(false, 1, h_temp);
*/

  DSP::u::IIR IIRfilter1(IIR_A[0], IIR_B[0]);
  DSP::u::IIR IIRfilter2(IIR_A[1], IIR_B[1]);
  DSP::u::IIR IIRfilter3(IIR_A[2], IIR_B[2]);
  DSP::u::IIR IIRfilter4(IIR_A[3], IIR_B[3]);
  DSP::u::IIR IIRfilter5(IIR_A[4], IIR_B[4]);

  for (ind=0; ind<5; ind++)
  {
    IIR_A[ind].clear();
    IIR_B[ind].clear();
  }

  // ************************************************* //
  DSP::u::Splitter SplitIIR1(true,2);
  DSP::u::Splitter SplitIIR2(true,2);
  DSP::u::Splitter SplitIIR3(true,2);
  DSP::u::Splitter SplitIIR4(true,2);
  DSP::u::Splitter SplitIIR5(true,2);

  // ************************************************* //
  DSP::u::ABS a1;
  DSP::u::ABS a2;
  DSP::u::ABS a3;
  DSP::u::ABS a4;
  DSP::u::ABS a5;

  // ************************************************* //
  DSP::u::CMPO  b1;
  DSP::u::Angle w1;
  DSP::u::CMPO  b2;
  DSP::u::Angle w2;
  DSP::u::CMPO  b3;
  DSP::u::Angle w3;
  DSP::u::CMPO  b4;
  DSP::u::Angle w4;
  DSP::u::CMPO  b5;
  DSP::u::Angle w5;

  // ************************************************* //
//  DSP::u::Splitter a1_split(2);
//  DSP::u::Splitter a2_split(2);
//  DSP::u::Splitter a3_split(2);
//  DSP::u::Splitter a4_split(2);
//  DSP::u::Splitter a5_split(2);
  DSP::u::Maximum Maks(5);
  DSP::u::Splitter MaksIndSplitter(2U);

  // ************************************************* //
//  DSP::u::Splitter w1_split(2);
//  DSP::u::Splitter w2_split(2);
//  DSP::u::Splitter w3_split(2);
//  DSP::u::Splitter w4_split(2);
//  DSP::u::Splitter w5_split(2);
  DSP::u::Selector PulsSelect(5U);

  // ************************************************* //
  DSP::u::FileOutput FileOut("test.out", DSP::e::SampleType::ST_float, 1);
  DSP::u::FileOutput FileIndOut("test_ind.out", DSP::e::SampleType::ST_float, 1);
  DSP::u::FileOutput FileMaxOut("test_max.out", DSP::e::SampleType::ST_float, 1);
  DSP::u::FileOutput FilePulsOut("test_puls.out", DSP::e::SampleType::ST_float, 1);


  // ************************************************* //
  // ************************************************* //
  // ************************************************* //
  FileIn.Output("out") >> Stage1.Input("in");
  Stage1.Output("out") >> Stage2.Input("in");
  Stage2.Output("out") >> SplitterIIR.Input("in");

  // ********************* //
  SplitterIIR.Output("out1") >> IIRdelay1.Input("in");
  IIRdelay1.Output("out") >> IIRfilter1.Input("in");
  IIRfilter1.Output("out") >> SplitIIR1.Input("in");
  SplitIIR1.Output("out1") >> a1.Input("in");
  SplitIIR1.Output("out2") >> b1.Input("in");
  b1.Output("out") >> w1.Input("in");

  a1.Output("out") >> Maks.Input("in1");
  w1.Output("out") >> PulsSelect.Input("in1");

//  DSP::u::FileOutput FileOut_Amp2("test_a2.out", DSP::e::SampleType::ST_float, 1);
//  DSP::u::FileOutput FileOut_Puls2("test_w2.out", DSP::e::SampleType::ST_float, 1);
//  a1.Output("out") >> FileOut_Amp1.Input("in");
//  w1.Output("out") >> FileOut_Puls1.Input("in");

  // ********************* //
  SplitterIIR.Output("out2") >> IIRdelay2.Input("in");
  IIRdelay2.Output("out") >> IIRfilter2.Input("in");
  IIRfilter2.Output("out") >> SplitIIR2.Input("in");
  SplitIIR2.Output("out1") >> a2.Input("in");
  SplitIIR2.Output("out2") >> b2.Input("in");
  b2.Output("out") >> w2.Input("in");

  a2.Output("out") >> Maks.Input("in2");
  w2.Output("out") >> PulsSelect.Input("in2");

//  DSP::u::FileOutput FileOut_Amp1("test_a1.out", DSP::e::SampleType::ST_float, 1);
//  DSP::u::FileOutput FileOut_Puls1("test_w1.out", DSP::e::SampleType::ST_float, 1);
//  a2.Output("out") >> FileOut_Amp2.Input("in");
//  w2.Output("out") >> FileOut_Puls2.Input("in");

  // ********************* //
  SplitterIIR.Output("out3") >> IIRdelay3.Input("in");
  IIRdelay3.Output("out") >> IIRfilter3.Input("in");
  IIRfilter3.Output("out") >> SplitIIR3.Input("in");
  SplitIIR3.Output("out1") >> a3.Input("in");
  SplitIIR3.Output("out2") >> b3.Input("in");
  b3.Output("out") >> w3.Input("in");

  a3.Output("out") >> Maks.Input("in3");
  w3.Output("out") >> PulsSelect.Input("in3");

//  DSP::u::FileOutput FileOut_Amp3("test_a3.out", DSP::e::SampleType::ST_float, 1);
//  DSP::u::FileOutput FileOut_Puls3("test_w3.out", DSP::e::SampleType::ST_float, 1);
//  a3.Output("out") >> FileOut_Amp3.Input("in");
//  w3.Output("out") >> FileOut_Puls3.Input("in");

  // ********************* //
  SplitterIIR.Output("out4") >> IIRdelay4.Input("in");
  IIRdelay4.Output("out") >> IIRfilter4.Input("in");
  IIRfilter4.Output("out") >> SplitIIR4.Input("in");
  SplitIIR4.Output("out1") >> a4.Input("in");
  SplitIIR4.Output("out2") >> b4.Input("in");
  b4.Output("out") >> w4.Input("in");

  a4.Output("out") >> Maks.Input("in4");
  w4.Output("out") >> PulsSelect.Input("in4");

//  DSP::u::FileOutput FileOut_Amp4("test_a4.out", DSP::e::SampleType::ST_float, 1);
//  DSP::u::FileOutput FileOut_Puls4("test_w4.out", DSP::e::SampleType::ST_float, 1);
//  a4.Output("out") >> FileOut_Amp4.Input("in");
//  w4.Output("out") >> FileOut_Puls4.Input("in");

  // ********************* //
  SplitterIIR.Output("out5") >> IIRdelay5.Input("in");
  IIRdelay5.Output("out") >> IIRfilter5.Input("in");
  IIRfilter5.Output("out") >> SplitIIR5.Input("in");
  SplitIIR5.Output("out1") >> a5.Input("in");
  SplitIIR5.Output("out2") >> b5.Input("in");
  b5.Output("out") >> w5.Input("in");

  a5.Output("out") >> Maks.Input("in5");
  w5.Output("out") >> PulsSelect.Input("in5");

//  DSP::u::FileOutput FileOut_Amp5("test_a5.out", DSP::e::SampleType::ST_float, 1);
//  DSP::u::FileOutput FileOut_Puls5("test_w5.out", DSP::e::SampleType::ST_float, 1);
//  a5.Output("out") >> FileOut_Amp5.Input("in");
//  w5.Output("out") >> FileOut_Puls5.Input("in");

  // ********************* //
  Maks.Output("ind") >> MaksIndSplitter.Input("in");
  MaksIndSplitter.Output("out1") >> PulsSelect.Input("ind");

  SplitterIIR.Output("out6") >> FileOut.Input("in1");
  MaksIndSplitter.Output("out2") >> FileIndOut.Input("in1");
  Maks.Output("max") >> FileMaxOut.Input("in1");
  PulsSelect.Output("out") >> FilePulsOut.Input("in1");


  czas1=clock();

  DSP::Clock::Execute(Zegar3, 30000);

  czas2=clock();

  DSP::Clock::FreeClocks();

  DSP::log << "MAIN"  << DSP::LogMode::second << "(" << czas1-czas0 << " ms"
    << " + " << czas2-czas1 << " ms)" << endl;
  /*! \todo DSP::u::CCPC instead of DSP::u::ABS + DSP::u::Angle
   */
}


int test_2()
{
  long int Fs, Fs2;
  DSP::Clock_ptr MasterClock, Zegar1, Zegar2;

  MasterClock=DSP::Clock::CreateMasterClock();

  Fs=CheckFs("DSPElib.wav", "examples");
  Fs2=CheckFs("test2.wav", "examples");
  DSP::u::WaveInput FileIn2(MasterClock, "test2.wav", "examples", 1);
  DSP::u::RawDecimator Decym(MasterClock, Fs2/Fs);
  DSP::u::Amplifier GainIn(0.5);

  Zegar1=FileIn2.GetOutputClock();
  Zegar2=DSP::Clock::GetClock(Zegar1, 1, Fs2/Fs);
  DSP::u::WaveInput FileIn(Zegar2, "DSPElib.wav", "examples", 1);


//  DSP::u::DDScos  FileIn(false, 1.0, (M_PIx2*(440))/Fs, 0.0);
  DSP::u::AudioOutput AudioOut(Fs,2);
//  DSP::u::AudioOutput AudioOut2(Fs);

  DSP::u::Addition Sum(2);
  DSP::u::Amplifier Gain(0.75);
  DSP::u::LoopDelay Delay(Zegar2, 3000);

  AudioOut.DefineInput("in1",0);
  FileIn2.Output("out1") >> Decym.Input("in");
  Decym.Output("out") >> AudioOut.Input("in1");

  FileIn.Output("out") >> GainIn.Input("in");
  GainIn.Output("out") >> Sum.Input("in1");
  Sum.Output("out") >> Delay.Input("in");
  Delay.Output("out") >> Gain.Input("in");
  Gain.Output("out") >> Sum.Input("in2");

  AudioOut.DefineInput("in2",1);
  Sum.Output("out") >> AudioOut.Input("in2");

  DSP::Clock::Execute(MasterClock, 260000);

  DSP::Clock::FreeClocks();
  return 0;
/*
  DSP::Float val;
  short Znak;

  val=2.0;
  while (1)
  {
   if (val < 0)
     Znak=-1;
   else
     Znak=1;
   printf("%f : %d\n", val, (short)(val+Znak*0.5));
   getchar();
   val-=0.1;
  }
*/
   Fs=CheckFs("DSPElib.wav", "examples");
   if (Fs > 0)
   {
      Process(Fs, "DSPElib.wav", "examples");
   }

/*
  DSP::Complex temp1, temp2;

  temp1.set(1.0, 0.0);
//  temp2.set(cos(0.1), sin(0.1));
  temp2.set(0.00000001, 0.00000000001);
  for (long ind=0; ind<2000000; ind++)
  {
    temp1.multiply_by(temp2);
  }
*/

  return 0;
}

int test_3()
{
  long int Fs, Fs2;
  DSP::Clock_ptr MasterClock; //, MasterClock2, Zegar1, Zegar2, Zegar3;


  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file_append);
  DSP::log.SetLogFileName("log_file.log");

  MasterClock=DSP::Clock::CreateMasterClock();

  Fs=CheckFs("DSPElib.wav", "examples");
  Fs2=CheckFs("test2.wav", "examples");

  printf("Fs = %li, Fs2 = %li\n", Fs, Fs2);

  DSP::u::WaveInput FileIn2(MasterClock, "DSPElib.wav", "examples", 1);
  DSP::u::WaveInput FileIn(MasterClock, "test2.wav", "examples", 1);
  DSP::u::RawDecimator Decym(MasterClock, Fs/Fs2);

  FileIn2.Output("out") >> Decym.Input("in");

//  DSP::u::SampleSelector Select(Decym.GetOutputClock());
  DSP::Clock_ptr SecondaryMasterClock;
  SecondaryMasterClock=DSP::Clock::CreateMasterClock();
  DSP::u::SampleSelector Select(Decym.GetOutputClock(), SecondaryMasterClock, true);

  Decym.Output("out") >> Select.Input("in");
  Decym.Output("out") >> Select.Input("act");

  DSP::u::Hold Hold(Select.GetOutputClock(), Decym.GetOutputClock(), true, 1);
  Select.Output("out") >> Hold.Input("in");

  DSP::u::FileOutput FileOut1("test1.out", DSP::e::SampleType::ST_float, 1);
//  DSP::u::FileOutput FileOut2("test2.out", DSP::e::SampleType::ST_float, 1);
//  Hold.Output("out") >> FileOut1.Input("in");
//  Decym.Output("out") >> FileOut2.Input("in");
//
//  DSP::u::FileOutput FileOut3("test3.out", DSP::e::SampleType::ST_float, 1);
//  Select.Output("out") >> FileOut3.Input("in");

  DSP::u::AudioOutput AudioOut(Fs,2);
  AudioOut.DefineInput("in1",0);
  AudioOut.DefineInput("in2",1);
  FileIn.Output("out") >> AudioOut.Input("in1");
  Hold.Output("out") >> AudioOut.Input("in2");

  Hold.Output("out") >> FileOut1.Input("in");

//  MasterClock2=DSP::Clock::CreateMasterClock();
//
//int log_ind=1; char log_buffer[1024];
//
//  DSP::u::AudioOutput AudioOut1(Fs,1);
//  /* \class DSP::u::AudioOutput \todo zrobi moliwo?c wypuszczenia tylko lewego lub prawego kanau
//   * z zerowym drugim
//   */
//
//  Zegar1=FileIn2.GetOutputClock();
//  Zegar2=DSP::Clock::GetClock(Zegar1, 1, Fs2/Fs);
//  DSP::Clock::GetClock(Zegar2, 1, 2);
//
//  DSP::u::Amplifier GainIn(0.5);
//
//  DSP::u::Addition Sum(2);
//  DSP::u::Amplifier Gain(0.75);
//  DSP_LoopDelay Delay(MasterClock, 3000);
//  DSP::u::AudioOutput AudioOut2(Fs,1);
//
//  AudioOut1.DefineInput("in1",0);
//  FileIn2.Output("out1") >> Decym.Input("in");
//  Decym.Output("out") >> AudioOut1.Input("in1");
//
//  FileIn.Output("out") >> GainIn.Input("in");
//  GainIn.Output("out") >> Sum.Input("in1");
//  Sum.Output("out") >> Delay.Input("in");
//  Delay.Output("out") >> Gain.Input("in");
//  Gain.Output("out") >> Sum.Input("in2");
//
//  DSP::u::SampleSelector Select(MasterClock);
//  AudioOut2.DefineInput("in2",0);
//  Sum.Output("out") >> Select.Input("in");
//  Sum.Output("out") >> Select.Input("act");
//  Select.Output("out") >> AudioOut2.Input("in2");
//
//  DSP::u::FileOutput FileOut("test_.out", DSP::e::SampleType::ST_float, 2);
//  Select.Output("out") >> FileOut.Input("in1");
//sprintf(log_buffer,">> %i A <<", log_ind++); DSP::Block::LogInnerState(log_buffer);
//  Sum.Output("out") >> FileOut.Input("in2");
//sprintf(log_buffer,">> %i B <<", log_ind++); DSP::Block::LogInnerState(log_buffer);
//
//  Zegar3=Select.GetOutputClock();
//  DSP::u::Hold Hold1(Zegar3, MasterClock, true, 1);
//
//  DSP::u::FileOutput FileOutS("test_select.out", DSP::e::SampleType::ST_float, 1);
//  Select.Output("out") >> Hold1.Input("in");
//DSP::u::FileOutput FileOutS2("test_select2.out", DSP::e::SampleType::ST_float, 1);
//Select.Output("out") >> FileOutS2.Input("in");
//  Hold1.Output("out") >> FileOutS.Input("in1");
//
//sprintf(log_buffer,">> %i <<", log_ind++);
//DSP::Block::LogInnerState(log_buffer);
//
////  return(0);
//
  for (int temp=0; temp<40; temp++)
  {
    DSP::Clock::Execute(MasterClock, 1000);
//    DSP::Clock::Execute(MasterClock2, 1000);
  }
//

  DSP::Clock::FreeClocks();

//  DSP::log << DSP::LogMode::Error << "Koniec");
  return 0;
}

/*****************************************************************/
int MeduzaSimulation(void)
{
  DSP::Clock_ptr MasterClock, ZerosOutClock; // InterpOutClock;
  int ind;
  char temp[128];

  DSP::Float Fb = 120.0; //baud rate
//  DSP::Float alfa=1.0; //wypelnienie
  DSP::Float F1=800.0; //center frequency of the first channel
  DSP::Float F_channel=200.0; //distans between channels
  DSP::Float Fs = 9600.0; //Output signal sampling frequency
  DSP::Float Fs2 = 2*Fs;//48000.0; //Output signal sampling frequency

  DSP::Float_vector h_forming((long)(Fs/Fb)); //]={1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
  for (ind=0; ind<(long)(Fs/Fb); ind++)
    h_forming[ind]= 1.0;

  MasterClock=DSP::Clock::CreateMasterClock();
  DSP::u::FileInput BinaryInput(MasterClock, "Tekst.txt", 10, DSP::e::SampleType::ST_bit);

  ZerosOutClock=DSP::Clock::GetClock(MasterClock, (long)(Fs/Fb), 1);
//  InterpOutClock=DSP::Clock::GetClock(ZerosOutClock, (long)(Fs2/Fs), 1);

  DSP::u::PSKencoder *DBPSKencoder[10];

  DSP::u::SamplingRateConversion *Interp[10];
//  DSP::u::Zeroinserter *Zeros[10];
//  DSP::u::FIR *Filter[10];

  DSP::u::DDScos *DDScos[10];
  DSP::u::RealMultiplication *Multip[10];
  DSP::u::Addition Sumator(11);
  for (ind=0; ind<10; ind++)
  {
    DBPSKencoder[ind]= new DSP::u::PSKencoder(DSP_DBPSK);
    sprintf(temp, "out%i", ind+1);
    BinaryInput.Output(temp), DBPSKencoder[ind]->Input("in");


//    Zeros[ind]= new DSP::u::Zeroinserter(MasterClock, (long)(Fs/Fb), false);
//    sprintf(temp, "out%i", ind+1);
//    DBPSKencoder[ind]->Output("out") >> Zeros[ind]->Input("in");
//    Filter[ind] = new DSP::u::FIR((long)(Fs/Fb), h_forming);
//    Zeros[ind]->Output("out") >> Filter[ind]->Input("in");
    Interp[ind] = new DSP::u::SamplingRateConversion (false, MasterClock, (long)(Fs/Fb), 1, h_forming);
    DBPSKencoder[ind]->Output("out") >> Interp[ind]->Input("in");

    DDScos[ind] = new DSP::u::DDScos (ZerosOutClock, DSP::Float(1.0),
      DSP::Float(M_PIx2*(F1+DSP::Float(ind)*F_channel)/Fs));
    Multip[ind] = new DSP::u::RealMultiplication(2);

//    Filter[ind]->Output("out") >> Multip[ind]->Input("real_in1");
    Interp[ind]->Output("out") >> Multip[ind]->Input("real_in1");
    DDScos[ind]->Output("out") >> Multip[ind]->Input("real_in2");

    sprintf(temp, "real_in%i", ind+1);
    Multip[ind]->Output("out") >> Sumator.Input(temp);
  }

  DSP::u::DDScos DDSpilot(ZerosOutClock, DSP::Float(1.0),
      DSP::Float(M_PIx2*(F1+11*F_channel)/Fs));
  DDSpilot.Output("out") >> Sumator.Input("real_in11");


  DSP::u::SamplingRateConversion  InterpOut(false, ZerosOutClock,  (long)(Fs2/Fs), 1, h_forming);
  Sumator.Output("out") >> InterpOut.Input("in");

  DSP::u::FileOutput SignalOutput("Signal.out");
  InterpOut.Output("out") >> SignalOutput.Input("in");

  DSP::u::AudioOutput AudioOut((long)Fs2,1);
  InterpOut.Output("out") >> AudioOut.Input("in");

//sprintf(log_buffer,">> %i <<", log_ind++);
//DSP::Block::LogInnerState(log_buffer);
  for (int temp_ind=0; temp_ind<40; temp_ind++)
  {
    DSP::Clock::Execute(Sumator.GetOutputClock(), (long)(Fs/4));
//    DSP::Clock::Execute(MasterClock2, 1000);
  }
//

  for (ind=0; ind<10; ind++)
  {
//    delete Zeros[ind];
//    delete Filter[ind];
    delete Interp[ind];
    delete DDScos[ind];
    delete Multip[ind];
  }

  DSP::Clock::FreeClocks();

  return 0;
}
/*********************************************************************/

int test_4()
{
  int ind_;

//  MeduzaSimulation();
/*! \todo test version without IIR filter
 */
  char tekst[1024];

//  char filename[]="03-pcz.wav";
  char filename[]="S2.wav";
  char dir_name[]="c:/Meduza_2004/Input";
//  char dir_name[]="e:/Meduza2004";
//  char filename[]="DSPElib.wav";
//  char dir_name[]=".";

//  DSP::f::SetLogState(DSP_LS_console | DSP_LS_file | DSP_LS_errors_only);
  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file_append);
  DSP::log.SetLogFileName("log_file.log");

  DSP::Float Fp, Fp1, Fp2; //sampling frequencies
  DSP::Float Fo = 15800.0; //pilot frequency
  DSP::Float Band=14*200.0;
  DSP::Float dF_channel=200.0;
  DSP::Float Fb = 120.0; //baud rate
  DSP::Float wo;

  int M1=10;
  int L1, L2;
  int channel_ind, NoOfChannels=12;

  DSP::Clock_ptr MasterClock;
  DSP::Clock_ptr FirstStageClock, SecondStageClock;

  Fp=(DSP::Float)CheckFs(filename, dir_name);
  if (Fp!=48000)
    return 1; // uncompatibile wave file

  /*************************************************************/
  MasterClock=DSP::Clock::CreateMasterClock();
  DSP::u::WaveInput FileIn(MasterClock, filename, dir_name, 1);

  /*************************************************************/
  /* Heterodyna wejciowa: wyj?cie sygna? podstawowo-pasmowy na 48kHz */
  wo=DSP_M_PIx2*(Fo-Band/2)/Fp; // dw_channel=2*pi*(df_channel)/fp;
  DSP::u::DDScos  MainHeter(MasterClock, true, 1.0, -wo, 0.0);
  DSP::u::Multiplication Mul1(1,1);

  FileIn.Output("out") >> Mul1.Input("real_in1");
  MainHeter.Output("out") >> Mul1.Input("cplx_in1");

  /*************************************************************/
  /* pierwszy stopie decymacji z 48kHz na 4800Hz */
  /* filtracja usuwajca sk?adowe antyhilbertowskie */
  /*! \todo filtracj po?czy z decymacj w implementacji polifazowej */
  DSP::Complex_vector h_LPF_Hilbert= {DSP::Complex(0.74203089692178, 0.47500487676950),
                            DSP::Complex(0.97483857322795, -0.70890299411674),
                            DSP::Complex(-1.50631102255127, -0.08862708027019),
                            DSP::Complex(0.24364258116658, 0.78553528997237),
                            DSP::Complex(0.04579897123496, -0.96755524298324),
                            DSP::Complex(0.04579897123496, 0.96755524298325),
                            DSP::Complex(0.24364258116658, -0.78553528997238),
                            DSP::Complex(-1.50631102255127, 0.08862708027020),
                            DSP::Complex(0.97483857322796, 0.70890299411674),
                            DSP::Complex(0.74203089692178, -0.47500487676950)};
  DSP::u::FIR LPF_Hilbert(true, h_LPF_Hilbert);
  Mul1.Output("out") >> LPF_Hilbert.Input("in");

  DSP::u::RawDecimator MainDecimator(MasterClock, M1, 2);
  LPF_Hilbert.Output("out") >> MainDecimator.Input("in");

  FirstStageClock = MainDecimator.GetOutputClock();
  MainDecimator.SetName("MainDecimator");
  Fp1=Fp/DSP::Float(M1);

  /*************************************************************/
  L1=(int)(Fp1/Fb);
  DSP::log << "MAIN" << DSP::LogMode::second <<  "L1=" << L1 << " (Fp1=" << std::setprecision(1) << Fp1 << "Hz)" << endl;

  int M2=5;

//  DSP::u::DCO pilot_DCO(0.0, 00, 0.0); //Open loop
//  DSP::u::DCO pilot_DCO(0.0, -1.0/(100*M2), 0.0); //frequency loop closed
//  DSP::u::DCO pilot_DCO(0.0, -1.0/(100*M2), -1.0/(1000*M2)); //closed loop
  DSP::u::DCO pilot_DCO(-DSP_M_PIx2*(5*dF_channel)/Fp1,
                     DSP::Float(+DSP_M_PIx2*(0.1*dF_channel)/Fp1),
                     DSP::Float(-1.0/(200*M2)), DSP::Float(-1.0/(4000*M2))); //closed loop
  pilot_DCO.SetName("pilot_DCO");

  DSP::u::Multiplication DCO_Main_Mul(0, 2);
  DCO_Main_Mul.SetName("DCO_Main_Mul");
  pilot_DCO.Output("out") >> DCO_Main_Mul.Input("cplx_in1");
  MainDecimator.Output("out") >> DCO_Main_Mul.Input("cplx_in2");

  DSP::u::DDScos  pilot_Heter(FirstStageClock, true, 1.0f, -M_PIx2f*(2*dF_channel)/Fp1, 0.0);
  pilot_Heter.SetName("pilot_Heter");

  DSP::u::Multiplication pilot_heter_Mul(0, 2);
  pilot_heter_Mul.SetName("pilot_heter_Mul");
  DCO_Main_Mul.Output("out") >> pilot_heter_Mul.Input("cplx_in1");
  pilot_Heter.Output("out") >> pilot_heter_Mul.Input("cplx_in2");

  /*************************************************************/
  /* drugi stopie decymacji z 4800Hz ma 960Hz dla pilota*/
  /*! \todo filtracj po?czy z decymacj w implementacji polifazowej */
  DSP::Float_vector h_PilotLPF(L1);
  for (ind_=0; ind_<L1; ind_++)
    h_PilotLPF[ind_] = 1.0f/(DSP::Float)L1;
  DSP::u::FIR LPF_Pilot(true, h_PilotLPF);
  LPF_Pilot.SetName("LPF_Pilot");
  pilot_heter_Mul.Output("out") >> LPF_Pilot.Input("in");

  DSP::u::RawDecimator PilotDecimator(FirstStageClock, M2, 2);
  PilotDecimator.SetName("PilotDecimator");
  SecondStageClock = PilotDecimator.GetOutputClock();
  LPF_Pilot.Output("out") >> PilotDecimator.Input("in");
  Fp2 = Fp1/(DSP::Float)M2;

  /*************************************************************/
  L2=(int)(Fp2/Fb);
  sprintf(tekst, "L2=%i (Fp2=%.1fHz)", L2, Fp2);
  DSP::log << "MAIN" << DSP::LogMode::second << tekst << endl;

  /*************************************************************/
  // automatic gain control
  DSP::u::AGC PilotAGC(0.01f, 0.0002f);
  PilotAGC.SetName("PilotAGC");
  PilotDecimator.Output("out") >> PilotAGC.Input("in");

  /*************************************************************/
  // frequency and phase error detectors
  DSP::Float error_alfa=0.9f; //0.99
  DSP::Float_vector error_IIR_a={1, -error_alfa};
//  DSP::Float error_IIR_b[]={1.0/(1-error_alfa)};
  DSP::Float_vector error_IIR_b={((DSP::Float)M2)*(1-error_alfa)};

  //* frequency error = filtered interpolated imaginary part of PilotAGC output multiplied
  //                    by conjugate of previous PilotAGC output
  DSP::u::CMPO freq_CMPO;
  freq_CMPO.SetName("freq_CMPO");
  PilotAGC.Output("out") >> freq_CMPO.Input("in");
  DSP::u::Vacuum freq_CMPO_real;
  freq_CMPO.Output("out.re") >> freq_CMPO_real.Input("in");

  DSP::u::Zeroinserter Freq_error_zeroins(SecondStageClock, M2);
  Freq_error_zeroins.SetName("Freq_error_zeroins");
/*
  freq_CMPO.Output("out.im") >> Freq_error_zeroins.Input("in");
  */
  freq_CMPO.Output("out.im") >> Freq_error_zeroins.Input("in");

  DSP::u::IIR Freq_error_filter(error_IIR_a, error_IIR_b);
  Freq_error_filter.SetName("Freq_error_filter");
  Freq_error_zeroins.Output("out") >> Freq_error_filter.Input("in");

  //* phase error = filtered imaginary part of interpolated PilotDecimator output
  DSP::u::Zeroinserter phase_error_zeroins(SecondStageClock, M2);
  phase_error_zeroins.SetName("phase_error_zeroins");
//  PilotAGC.Output("out.im") >> phase_error_zeroins.Input("in");
  PilotAGC.Output("out.im") >> phase_error_zeroins.Input("in");

  DSP::u::IIR Phase_error_filter(error_IIR_a, error_IIR_b);
  Phase_error_filter.SetName("Phase_error_filter");
  phase_error_zeroins.Output("out") >> Phase_error_filter.Input("in");

  /*************************************************************/
  // closing the loop
  Phase_error_filter.Output("out") >> pilot_DCO.Input("in.phase_err");
  Freq_error_filter.Output("out") >> pilot_DCO.Input("in.freq_err");


  /*************************************************************/
  /*************************************************************/
  // matched filtering and decimation in all subchannels
  /*! \todo filtracj po?czy z decymacj w implementacji polifazowej */
  int N_matched=(int)(0.6*L1+0.5);
  DSP::Float_vector h_matched(N_matched);
  for (ind_=0; ind_<N_matched; ind_++)
    h_matched[ind_]=1.0f/DSP::Float(N_matched);

  DSP::u::FIR *MatchedFilters[NoOfChannels];
  DSP::u::RawDecimator *MatchedDecimators[NoOfChannels];
  DSP::u::AGC *MatchedAGC[NoOfChannels];
  DSP::u::Multiplication *MatchedDDS_Muls[NoOfChannels-1];
  DSP::u::DDScos  channels_Heter(FirstStageClock, true, 1.0, M_PIx2f*(dF_channel)/Fp1, 0.0);
//  DSP::u::GardnerSampling GardnerSampling(L2, 0.0005, NoOfChannels);
  DSP::u::GardnerSampling GardnerSampling(DSP::Float(L2), 0.01f, 1.0f, NoOfChannels);
  DSP::u::CMPO *OutputDiff[NoOfChannels];
  for (channel_ind=0; channel_ind<NoOfChannels; channel_ind++)
  {
    MatchedFilters[channel_ind]= new DSP::u::FIR(true, h_matched);
    sprintf(tekst, "MatchedFilters[%i]", channel_ind);
    MatchedFilters[channel_ind]->SetName(tekst);

    MatchedDecimators[channel_ind]= new DSP::u::RawDecimator(FirstStageClock, M2, 2);
    sprintf(tekst, "MatchedDecimators[%i]", channel_ind);
    MatchedDecimators[channel_ind]->SetName(tekst);

    MatchedAGC[channel_ind]= new DSP::u::AGC(0.01f, 0.0002f, (DSP::Float)sqrt(2.0));
    sprintf(tekst, "MatchedAGC[%i]", channel_ind);
    MatchedAGC[channel_ind]->SetName(tekst);

    if (channel_ind > 0)
    {
      MatchedDDS_Muls[channel_ind-1]= new DSP::u::Multiplication(0,2);
      sprintf(tekst, "MatchedDDS_Muls[%i]", channel_ind-1);
      MatchedDDS_Muls[channel_ind-1]->SetName(tekst);

      if (channel_ind > 1)
      {
        channels_Heter.Output("out"),
          MatchedDDS_Muls[channel_ind-1]->Input("cplx_in1");
        MatchedDDS_Muls[channel_ind-2]->Output("out"),
          MatchedDDS_Muls[channel_ind-1]->Input("cplx_in2");
      }
      else
      {
        channels_Heter.Output("out"),
          MatchedDDS_Muls[channel_ind-1]->Input("cplx_in1");
        DCO_Main_Mul.Output("out"),
          MatchedDDS_Muls[channel_ind-1]->Input("cplx_in2");
      }

      MatchedDDS_Muls[channel_ind-1]->Output("out"),
        MatchedFilters[channel_ind]->Input("in");
    }
    else
    {
      DCO_Main_Mul.Output("out"),
        MatchedFilters[channel_ind]->Input("in");
    }
    MatchedFilters[channel_ind]->Output("out"),
      MatchedDecimators[channel_ind]->Input("in");
    MatchedDecimators[channel_ind]->Output("out"),
      MatchedAGC[channel_ind]->Input("in");

    sprintf(tekst, "in%i", channel_ind+1);
    MatchedAGC[channel_ind]->Output("out"),
      GardnerSampling.Input(tekst);

    OutputDiff[channel_ind]= new DSP::u::CMPO;
    sprintf(tekst, "out%i", channel_ind+1);
    GardnerSampling.Output(tekst), OutputDiff[channel_ind]->Input("in");
  }
//  DCO_Mul_pilot_signal.Output("out") >> LPF_Pilot.Input("in");

  /*************************************************************/
  // saving control signals to files
  /*************************************************************/
  DSP::f::MakeDir("outputs");
  DSP::u::FileOutput MainHeterOut("outputs/MainHeter.out", DSP::e::SampleType::ST_float, 2);
  DSP::u::FileOutput FileInOut("outputs/FileIn.out", DSP::e::SampleType::ST_float, 1);
  DSP::u::FileOutput LPF_HilbertOut("outputs/LPF_Hilbert.out", DSP::e::SampleType::ST_float, 2);
  DSP::u::FileOutput DCO_Main_MulOut("outputs/DCO_Main_Mul.out", DSP::e::SampleType::ST_float, 2);
//  DSP::u::FileOutput MainDecimatorOut("MainDecimator.out", DSP::e::SampleType::ST_float, 2);

  DSP::u::FileOutput PilotDecimatorOut("outputs/PilotDecimator.out", DSP::e::SampleType::ST_float, 2);
  DSP::u::FileOutput PilotDCO_Out("outputs/PilotDCO.out", DSP::e::SampleType::ST_float, 2);
  DSP::u::FileOutput pilot_heter_MulOut("outputs/pilot_heter_Mul.out", DSP::e::SampleType::ST_float, 2);

  DSP::u::FileOutput phase_errOut("outputs/phase_err.out", DSP::e::SampleType::ST_float, 1);
  DSP::u::FileOutput freq_errOut("outputs/freq_err.out", DSP::e::SampleType::ST_float, 1);
  DSP::u::FileOutput Freq_error_zeroinsOut("outputs/Freq_error_zeroins.out", DSP::e::SampleType::ST_float, 1);

  DSP::u::FileOutput *SubchannelsOut[NoOfChannels];
  DSP::u::FileOutput *SubchannelsGardnerOut[NoOfChannels];
  DSP::u::FileOutput *SubchannelsDifferatorOut[NoOfChannels];
  for (channel_ind=0; channel_ind<NoOfChannels; channel_ind++)
  {
    sprintf(tekst,"outputs/subchannels_%02i.out", channel_ind);
    SubchannelsOut[channel_ind]= new DSP::u::FileOutput(tekst, DSP::e::SampleType::ST_float, 2);

    MatchedAGC[channel_ind]->Output("out") >> SubchannelsOut[channel_ind]->Input("in");

    sprintf(tekst,"outputs/subchannels_gardner_%02i.out", channel_ind);
    SubchannelsGardnerOut[channel_ind]= new DSP::u::FileOutput(tekst, DSP::e::SampleType::ST_float, 2);

    sprintf(tekst, "out%i", channel_ind+1);
    GardnerSampling.Output(tekst), SubchannelsGardnerOut[channel_ind]->Input("in");

    sprintf(tekst,"outputs/subchannels_diff_%02i.out", channel_ind);
    SubchannelsDifferatorOut[channel_ind]= new DSP::u::FileOutput(tekst, DSP::e::SampleType::ST_float, 2);

    OutputDiff[channel_ind]->Output("out"),
                SubchannelsDifferatorOut[channel_ind]->Input("in");
  }

  FileIn.Output("out") >> FileInOut.Input("in");
  MainHeter.Output("out") >> MainHeterOut.Input("in");
  LPF_Hilbert.Output("out") >> LPF_HilbertOut.Input("in");
  DCO_Main_Mul.Output("out") >> DCO_Main_MulOut.Input("in");
//  MainDecimator.Output("out") >> MainDecimatorOut.Input("in");
  PilotAGC.Output("out") >> PilotDecimatorOut.Input("in");
  pilot_DCO.Output("out") >> PilotDCO_Out.Input("in");
  pilot_heter_Mul.Output("out") >> pilot_heter_MulOut.Input("in");


  Phase_error_filter.Output("out") >> phase_errOut.Input("in");
  Freq_error_filter.Output("out") >> freq_errOut.Input("in");
  Freq_error_zeroins.Output("out") >> Freq_error_zeroinsOut.Input("in");

  /*************************************************************/
  DSP::Float dF;
  clock_t  start_clk, elapsed_clk;
  DSP::Float elapsed_time;
  start_clk=clock();

  int ind=0;
  int NoOfEmptySegments2Process=2;
  while(NoOfEmptySegments2Process > 0)
//  for (ind=0; ind<40; ind++)
  {
    DSP::Clock::Execute(MainDecimator.GetOutputClock(), 500);

    elapsed_clk= clock()-start_clk;
    elapsed_time=((DSP::Float)elapsed_clk)/CLK_TCK;
    dF=-(5*dF_channel+pilot_DCO.GetFrequency(Fp1));
    sprintf(tekst, "%2i) pilot power:%f,\n"
            "   pilot frequency:%.1f Hz (%.2f Hz)\n"
            "   elapsed time=%.2f[s], %.2f[kSa/s] / %.1f[kSa/s]\n"
            "   L=%.7f", ind,
            PilotAGC.GetPower(),
            Fo+dF, dF,
            elapsed_time, float(500.0*(ind+1)/elapsed_time/1000), Fp1/1000,
            GardnerSampling.GetSamplingPeriod());
//            PilotAGC.GetPower(), (Fo-Band/2)-pilot_DCO.GetFrequency(Fp1));
    DSP::log << "MAIN" << DSP::LogMode::second << tekst << endl;

    #ifdef WIN32
      MSG temp_msg;
      PeekMessage(&temp_msg, NULL, 0, 0, PM_NOREMOVE);
    #endif
    if (FileIn.GetBytesRead() == 0L)
    {
      NoOfEmptySegments2Process--;
    }
    ind++;
  }

  /*************************************************************/
  for (channel_ind=0; channel_ind<NoOfChannels; channel_ind++)
  {
    delete MatchedFilters[channel_ind];
    delete MatchedDecimators[channel_ind];
    if (channel_ind > 0)
      delete MatchedDDS_Muls[channel_ind-1];

    delete SubchannelsOut[channel_ind];
    delete SubchannelsGardnerOut[channel_ind];
    delete SubchannelsDifferatorOut[channel_ind];

    delete MatchedAGC[channel_ind];
  }
  DSP::Clock::FreeClocks();

  DSP::log << DSP::LogMode::Error << "MAIN" << DSP::LogMode::second << "Finished" << endl;
  return 0;
}


int test_5()
{
  DSP::Clock_ptr MasterClock;
  long int Fp;

  Fp=44100; //96000; //192000; //32000; //11025; //22050; //44100;

  TAudioMixer *tmp;
  tmp = new TAudioMixer;
  delete tmp;


  MasterClock=DSP::Clock::CreateMasterClock();

  DSP::u::AudioInput AudioIn(MasterClock, Fp);
  AudioIn.SetName("Test");

  DSP::u::DDScos AudioIn2(MasterClock);
  DSP::u::DDScos AudioIn3(MasterClock, 0.4f, M_PIx2f*8*400.0/8000);

  AudioIn2.SetConstInput("ampl",0.05f); //Amplitude
  AudioIn2.SetConstInput("phase",0.0f); //Initial phase

DSP::u::COSpulse AudioIn2_frequ(MasterClock, M_PIx2f*0.5f/DSP::Float(Fp));
DSP::Float_vector a_in={1.0, -1.0};
DSP::u::IIR Acum(a_in);
  AudioIn2_frequ.Output("out") >> Acum.Input("in");
  Acum.Output("out") >> AudioIn2.Input("puls");

  DSP::u::AudioOutput AudioOut(Fp);

DSP::u::Addition Sum;
  AudioIn2.Output("out") >> Sum.Input("in1");
  AudioIn3.Output("out") >> Sum.Input("in2");

  Sum.Output("out") >> AudioOut.Input("in");


  DSP::u::FileOutput FileOut("outputs/AudioIn.out", DSP::e::SampleType::ST_float, 1);
  AudioIn.Output("out") >> FileOut.Input("in");

  DSP::u::FileOutput FileOut2("outputs/AudioIn2.out", DSP::e::SampleType::ST_float, 1);
  Sum.Output("out") >> FileOut2.Input("in");


  DSP::Clock::SchemeToDOTfile(MasterClock, "test_scheme_file.dot");
  for (int temp=0; temp<40; temp++)
  {
    DSP::Clock::Execute(MasterClock, Fp/8);
    DSP::log << "MAIN" << DSP::LogMode::second << temp << endl;
  }

  DSP::log << DSP::LogMode::Error << "MAIN" << DSP::LogMode::second << "end" << endl;
  DSP::Clock::FreeClocks();

  return 0;
}

int test_6()
{
  DSP::Clock_ptr MasterClock;
  //DSP::u::Const *const_;
  DSP::u::LFSR *const_;
  DSP::u::LFSR_tester *tester;
  DSP::u::FileOutput *temp_file;
  //DSP::u::Accumulator *akum;

  MasterClock = DSP::Clock::CreateMasterClock();
  //const_ = new DSP::u::Const(MasterClock,2.0);

  unsigned int *taps;
  taps = new unsigned int[2];
  taps[0] = 49; taps[1] = 52;
  const_ = new DSP::u::LFSR(MasterClock, 52, 2, taps);
  taps[0] = 49; taps[1] = 52;
  tester = new DSP::u::LFSR_tester(52, 2, taps);

  //akum = new DSP::u::Accumulator();
  //akum->SetInitialState(1.0);

  temp_file = new DSP::u::FileOutput("temp.flt", DSP::e::SampleType::ST_int, 1, DSP::e::FileType::FT_flt_no_scaling, 100);
  //const_->Output("out") >> akum->Input("in");
  //akum->Output("out") >> temp_file->Input("in1");

  const_->Output("out") >> tester->Input("in");
  tester->Output("out") >> temp_file->Input("in");

  for (int temp=0; temp<4; temp++)
  {
    DSP::Clock::Execute(MasterClock, 512);
  }

  //delete akum;
  delete temp_file;
  delete tester;
  delete const_;

  DSP::log << DSP::LogMode::Error << "MAIN" << DSP::LogMode::second << "end" << endl;
  DSP::Clock::FreeClocks();

  return 0;
}



/*! Simple macro usage example.
 * \author Marek Blok
 * \date 2010.02.24
 */
#include <DSP_lib.h>

class DDS_macro : public DSP::Macro
{
  private:
    DSP::u::PCCC *alpha_state_correction;
    DSP::u::Multiplication *Alpha_state_MUL;
    DSP::u::LoopDelay *Alpha_state;

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

  alpha_state_correction = new DSP::u::PCCC;
  alpha_state_correction->SetConstInput("in.abs", 1.0);

  Alpha_state_MUL = new DSP::u::Multiplication(0, 3); Alpha_state_MUL->SetConstInput("in3", factor);
  this->MacroInput("in") >> alpha_state_correction->Input("in.arg");
  alpha_state_correction->Output("out") >> Alpha_state_MUL->Input("in2");

  Alpha_state = new DSP::u::LoopDelay(Fp2Clock, 1, 2); Alpha_state->SetState("in.re", 1.0);
//Alpha_state->Output("out") >> this->OutputInput("in");
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

int test_7()
{
  DSP::Clock_ptr MasterClock;
  int temp;
  long int Fp;

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP_lib_version_string() << endl << endl;

  MasterClock=DSP::Clock::CreateMasterClock();


  DSP::u::WaveInput *AudioIn;
  AudioIn = new DSP::u::WaveInput(MasterClock, "DSPElib.wav", ".");
  Fp = AudioIn->GetSamplingRate();
  DDS_macro *DDS;
  DDS = new DDS_macro(MasterClock, 0.15f*M_PIx1f);
  DSP::u::Amplifier *gain;
  gain = new DSP::u::Amplifier(1.0/2);
  DSP::u::AudioOutput *AudioOut;
  AudioOut = new DSP::u::AudioOutput(Fp, 2);
  DSP::u::FileOutput *FileOut;
  FileOut = new DSP::u::FileOutput("test_out.wav", DSP::e::SampleType::ST_short, 2, DSP::e::FileType::FT_wav, Fp);

  AudioIn->Output("out") >> gain->Input("in");
  gain->Output("out") >> DDS->Input("in");

  DDS->Output("out") >> AudioOut->Input("in");
  DDS->Output("out") >> FileOut->Input("in");

  DSP::Clock::SchemeToDOTfile(MasterClock, "macro_wraped.dot");
  DSP::Clock::SchemeToDOTfile(MasterClock, "macro_DDS.dot", DDS);

  DDS->SetDOTmode(DSP_DOT_macro_unwrap);
  DSP::Clock::SchemeToDOTfile(MasterClock, "macro_unwraped.dot");

  //! \todo 2010.03.31 DSP::Clock::ListOfAllComponents should show number of AutoSplitters and DSP::u::Copy objects
  DSP::Clock::ListOfAllComponents();
  DSP::Component::CheckInputsOfAllComponents();

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::LogMode::second << temp << endl;
    temp++;
  }
  while (AudioIn->GetBytesRead() != 0);

  delete AudioIn;
  delete gain;
  delete AudioOut;
  delete FileOut;
  delete DDS;

  DSP::Clock::ListOfAllComponents();

  DSP::Clock::FreeClocks();
  DSP::log << "MAIN" << DSP::LogMode::second << "end" << endl;

  return 0;
}


int test_8()
{
    long int Fp;
    DSP::Clock_ptr MasterClock, Clock4;
    MasterClock = DSP::Clock::CreateMasterClock();
    Clock4 = DSP::Clock::GetClock(MasterClock, 1, 4);


    DSP::u::WaveInput *AudioIn = NULL;
    DSP::u::RawDecimator *DecM4 = NULL;
    DSP::u::RawDecimator *DecM8 = NULL;
    DSP::u::RawDecimator *DecM11 = NULL;
    DSP::u::FileOutput *WAVEfile = NULL;

    AudioIn = new DSP::u::WaveInput(MasterClock, "test_new.wav", ".");

    Fp = AudioIn->GetSamplingRate();



    /*
    //DSP::u::AudioOutput AudioOut(Fp);
    DSP::u::RawDecimator DecM44(MasterClock, 8);
    //DSP::u::RawDecimator DecM11(MasterClock, 11);
    DSP::u::FileOutput WAVEfile("out.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp/8);
    AudioIn.Output("out") >> DecM44.Input("in");
    //DecM4.Output("out") >> DecM11.Input("in");
    DecM44.Output("out") >> WAVEfile.Input("in");
    */


    if(Fp == 44100){
          DecM4 = new DSP::u::RawDecimator(MasterClock, 4);
          DecM11 = new DSP::u::RawDecimator(Clock4, 11);
          WAVEfile = new DSP::u::FileOutput("out.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp/44);
          AudioIn->Output("out") >> DecM4->Input("in");
          DecM4->Output("out") >> DecM11->Input("in");
          DecM11->Output("out") >> WAVEfile->Input("in");
    }

    else if(Fp == 8000){
          DecM8 = new DSP::u::RawDecimator(MasterClock, 8);
          WAVEfile = new DSP::u::FileOutput("out.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp/8);
          AudioIn->Output("out") >> DecM8->Input("in");
          DecM8->Output("out") >> WAVEfile->Input("in");
    }


    DSP::Clock::SchemeToDOTfile(MasterClock, "test.dot");
    do {
       DSP::Clock::Execute(MasterClock, 1000);
    } while (AudioIn->GetBytesRead() != 0);

    delete AudioIn;
    delete DecM4;
    delete DecM8;
    delete DecM11;
    delete WAVEfile;
    DSP::Clock::FreeClocks();
    return 0;
}


/* ************************************************************* */
/*! Laboratorium: Wieloszybko�ciowe przetwarzanie sygna��w w systemach wielokana�owych
 *  �w. 5. zad. 2.
 *
 * \author Marek Blok
 * \date 2012.04.30
 */
#include <DSP_lib.h>

DSP::Float_ptr read_buffer = NULL;
int buffer_size;
int No_of_samples=0;

void FFTout_clbk(unsigned int NoOfInputs, unsigned int NoOfOutputs, DSP::Float_vector &OutputSamples, DSP::void_ptr *UserDataPtr, unsigned int UserDefinedIdentifier, DSP::Component_ptr Caller)
{
  UNUSED_ARGUMENT(UserDataPtr);
  UNUSED_ARGUMENT(UserDefinedIdentifier);

  DSP::u::OutputBuffer *dsp_buffer;
  //int counter;
  dsp_buffer = (DSP::u::OutputBuffer *)Caller->Convert2Block();

  if (NoOfInputs == DSP::c::Callback_Init)
  {
    buffer_size = dsp_buffer->GetBufferSize(2);
    read_buffer = new DSP::Float[buffer_size];
    return;
  }
  if (NoOfInputs == DSP::c::Callback_Delete)
  {
    delete [] read_buffer;
    read_buffer = NULL;
    return;
  }

  //counter =
  dsp_buffer->ReadBuffer(read_buffer,
                         buffer_size, // read all samples
                         -1,  // full reset
                         DSP::e::SampleType::ST_float); // write to read_buffer in float format

  // output real samples
  for (unsigned int ind = 0; ind < NoOfOutputs; ind ++)
    OutputSamples[ind] = read_buffer[ind];
 // mo�na te� czyta� bezpo�rednio do OutputSamples
}


int test_9()
{

  /*************************************************************/
  // Log file setup
  DSP::log.SetLogFileName("log_file.txt");
  DSP::log.SetLogState(DSP::E_LS_Mode::LS_file | DSP::E_LS_Mode::LS_console);

  DSP::log << DSP_lib_version_string() << endl << endl;
  /*************************************************************/

/*
 idea:

 1. Load impulse responses of prototype and image-reject filters

 2. implement a shaping filter + image-reject filter


*/


  DSP::LoadCoef coef_info;
  unsigned int L_IFIR;
  unsigned int N_sh, N_ir;
  DSP::Float_vector tmp;
  DSP::Float_vector h_sh, h_ir;

  // **********************************
  coef_info.Open("cw4_zad1_h_sh.coef", "matlab");
  N_sh = coef_info.GetSize(0);
  h_sh.resize(N_sh);
  coef_info.Load(h_sh, 0);
  coef_info.Load(tmp, 1);
  L_IFIR = (unsigned int)tmp[0];

  // **********************************
  coef_info.Open("cw4_zad1_h_ir.coef", "matlab");
  N_ir = coef_info.GetSize();
  h_ir.resize(N_ir);
  coef_info.Load(h_ir);

  /*************************************************************/

  DSP::Clock_ptr InputClock;
  InputClock=DSP::Clock::CreateMasterClock();


  DSP::u::FileInput InputSignal(InputClock, "matlab/delta_44100.wav", 1U, DSP::e::SampleType::ST_short, DSP::e::FileType::FT_wav);
  int Fp1 = InputSignal.GetSamplingRate();
  /*if (Fp1_tmp != Fp1)
  {
    DSP::log << DSP::LogMode::Error << "Problem z sygna�em wej�ciowym");
  } */

  DSP::u::FIR H_sh(h_sh, 0, 1, L_IFIR);
  DSP::u::FIR H_ir(h_ir);

//  DSP::u::AudioOutput SoundOut(Fp2, 1, 16);
  DSP::u::FileOutput FileOut_a("matlab/cw3_zad2.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp1);
  DSP::u::FileOutput FileOut_b("matlab/cw3_zad2.flt", DSP::e::SampleType::ST_float, 1, DSP::e::FileType::FT_flt, Fp1);

  DSP::log << "Fp1 = " << Fp1 << ", L_IFIR = " << L_IFIR << endl;

  /*************************************************************/
  // Connections definitions
  InputSignal.Output("out") >> H_sh.Input("in");
  H_sh.Output("out") >> H_ir.Input("in");
  H_ir.Output("out") >> FileOut_a.Input("in");
  H_ir.Output("out") >> FileOut_b.Input("in");


  /////////////////////////////////
  // check if there are signals
  // connected to all inputs
  DSP::Component::CheckInputsOfAllComponents();

  // *********************************** //
  DSP::Clock::SchemeToDOTfile(InputClock, "Cw3_zad2.dot");

  // *********************************** //
  int SamplesInSegment = 512;
  __int64 NoOfSamplesProcessed = 0;
  // 10 seconds
  __int64 MAX_SAMPLES_TO_PROCESS = 10*Fp1;
  while(NoOfSamplesProcessed < MAX_SAMPLES_TO_PROCESS)
  {

    // ********************************************************** //
    DSP::Clock::Execute(InputClock, SamplesInSegment);
    // ********************************************************** //

    if (InputSignal.GetBytesRead() > 0)
    {
        NoOfSamplesProcessed = 0; // Play the whole file
    }
    else // Play 200ms more
    {
      if (NoOfSamplesProcessed < MAX_SAMPLES_TO_PROCESS - Fp1/5)
          NoOfSamplesProcessed = MAX_SAMPLES_TO_PROCESS - Fp1/5;
    }

    NoOfSamplesProcessed += SamplesInSegment;
    // ********************************************************** //
  }

  /*************************************************************/
  DSP::Clock::FreeClocks();

  return 0;
}



int test_10()
{
  DSP::Clock_ptr InputClock;
  InputClock=DSP::Clock::CreateMasterClock();

  long int F_p;
  DSP::LoadCoef coef_info;
  DSP::Complex_vector h_C;
  int N_C;

  coef_info.Open("test.coef", "matlab");
  N_C = coef_info.GetSize(0);
  if (N_C < 1)
  {
    DSP::log << DSP::LogMode::Error << "No test.coef: aborting" << endl;
    return -1;
  }
  else
  {
    coef_info.Load(h_C);
    F_p = coef_info.Fp;
  }

  DSP::u::FileInput InputSignal(InputClock, "matlab/test_in.flt", 2U, DSP::e::SampleType::ST_float, DSP::e::FileType::FT_flt);
  DSP::u::FileOutput OutputSignal("matlab/test_out.flt", DSP::e::SampleType::ST_float, 2U, DSP::e::FileType::FT_flt, F_p);

  DSP::u::FIR FIR(true, h_C);

  InputSignal.Output("out") >> FIR.Input("in");
  FIR.Output("out") >> OutputSignal.Input("in");

  // *********************************** //
  int SamplesInSegment = 512;
  __int64 NoOfSamplesProcessed = 0;
  // 10 seconds
//  #define MAX_SAMPLES_TO_PROCESS 1*F_p
  while(NoOfSamplesProcessed < 10000)
  {

    // ********************************************************** //
    DSP::Clock::Execute(InputClock, SamplesInSegment);
    // ********************************************************** //

    NoOfSamplesProcessed += SamplesInSegment;
    // ********************************************************** //

  }

  /*************************************************************/
  DSP::Clock::FreeClocks();

  return 0;
}

int test_11()
{
    /*! Koncepcja
    * - cztery kana�y - r�ne modulacje - ten sam filtr kszta�tuj�cy
    *   DPSK, pi/4-QPSK, (MSK), OQPSK, QAM-16, OOK
    * - FFT - 32 ==> od razu lokowanie na PCz (nadpor�bkowanie 32)
    * - dost�pne kana�y 1..15 (nie mo�na u�y� dla FMT dw�ch s�siednich
    *   u�ycie: 8, 10, 13
    * reszta sta�e zero !!! SetConstInput (chyba powinno dzia�a�)
    */
  /*************************************************************/
  // Log file setup
  DSP::log.SetLogFileName("log_file.log");
  //DSP::f::SetLogState(DSP_LS_file | DSP_LS_console);
  DSP::log.SetLogState(DSP::E_LS_Mode::LS_file);

  DSP::log << DSP_lib_version_string() << endl << endl;
  /*************************************************************/

  long int Fp2, F_symb;
  DSP::LoadCoef coef_info;
  int N_rc; // , N2;
  //unsigned int L;
  DSP::Float_vector h_rc;
  char text[16];

  coef_info.Open("cw5_zad1_h_rc.coef", "matlab");
  N_rc = coef_info.GetSize(0);
  if (N_rc < 1)
  {
    DSP::log << DSP::LogMode::Error << "No cw5_zad1_h_rc.coef: aborting" << endl;
    return -1;
  }
  else
  {
    h_rc.resize(N_rc);
    coef_info.Load(h_rc);
    F_symb = coef_info.Fp;
  }
  /*************************************************************/

  DSP::Clock_ptr SymbolClock, InputClock;
  InputClock=DSP::Clock::CreateMasterClock();


  //DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
  //F_symb = AudioIn.GetSamplingRate();


  F_symb = 1500; // dla L = 32 => Fp2 = 48000
  int K = 32;
  Fp2 = K*F_symb;

  DSP::log << "Fsymb = " << F_symb << ", Fp2 = " << Fp2 << ", L = " << K << endl;

  SymbolClock=DSP::Clock::GetClock(InputClock, 1, K);


  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // Wielokana�owy sygna� wej�ciowy
  DSP::u::FileInput InputSignal(InputClock, "cw5_zad1.flt", 2U, DSP::e::SampleType::ST_float, DSP::e::FileType::FT_flt);
  DSP::u::Vacuum DumpImag;

  DSP::u::Delay SymbolTimigDelay(1U);

  InputSignal.Output("out.re") >> SymbolTimigDelay.Input("in");
  InputSignal.Output("out.im") >> DumpImag.Input("in");

  DSP::u::OutputBuffer OutputBuffer(  K, // unsigned int   BufferSize_in,
                                   1, // unsigned int   NoOfInputs_in,
                                   DSP_stop_when_full, //DSPe_buffer_type   cyclic,
                                   InputClock, //DSP::Clock_ptr  ParentClock,
                                   SymbolClock, //DSP::Clock_ptr   NotificationsClock,
                                   K, //unsigned int  NoOfOutputs_in,
                                   FFTout_clbk //DSP::Buffer_callback_ptr   func_ptr,
                                   //unsigned int   CallbackIdentifier = 0
                                   );
  SymbolTimigDelay.Output("out") >> OutputBuffer.Input("in");

  DSP::u::FFT *fft = new DSP::u::FFT(K, false);
  // filtry polifazowe
  DSP::u::FIR *H_g[K];

  for (int ind = 0; ind < K; ind++)
  {
    H_g[ind] = new DSP::u::FIR(h_rc, (K-1)-ind, K);

    string name, name2;
    name = "out"; name += to_string(ind+1);
    name2 = "in"; name2 += to_string(ind+1);
    OutputBuffer.Output(name), H_g[ind]->Input("in");
    H_g[ind]->Output("out") >> fft->Input(name2);
  }

  DSP::u::Vacuum *Discard[K-3];
  int channel1, channel2, channel3;
  channel1 = 8; channel2 = 10; channel3 = 13;
//  channel1 = 1; channel2 = 31; channel3 = 32;
  int current = 0;
  for (int ind = 1; ind <= K; ind++)
  {
      if ((ind != channel1) && (ind != channel2) && (ind != channel3))
      {
         Discard[current] = new DSP::u::Vacuum(true);
         sprintf(text, "out%i", ind);
         fft->Output(text), Discard[current++]->Input("in");
      }
  }



  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // Pierwszy kana� danych
  DSP::u::PSKdecoder PSKdecoder1(DSP_QPSK_A);
//  DSP::u::FileOutput BinData1("cw5_zad1.dat", DSP::e::SampleType::ST_bit, 2U, DSP::e::FileType::FT_raw, F_symb);
  DSP::u::FileOutput SymbData1("cw5_zad2a.flt", DSP::e::SampleType::ST_float, 2U, DSP::e::FileType::FT_flt, F_symb);
  DSP::u::FileOutput BinData1("cw5_zad2a.dat", DSP::e::SampleType::ST_bit_text, 2U, DSP::e::FileType::FT_raw);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // drugi kana� danych
  DSP::u::PSKdecoder PSKdecoder2(DSP_QPSK_A);
  DSP::u::FileOutput BinData2("cw5_zad2b.dat", DSP::e::SampleType::ST_bit, 2U, DSP::e::FileType::FT_raw, F_symb);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // trzeci kana� danych
  DSP::u::PSKdecoder PSKdecoder3(DSP_QPSK_A);
  DSP::u::FileOutput BinData3("cw5_zad2c.dat", DSP::e::SampleType::ST_bit, 2U, DSP::e::FileType::FT_raw, F_symb);

  // pod��cz kana�y w�skopasmowe ST
  // kana� nr 8
  string name;
  name = "out"; name += to_string(channel1);
  fft->Output(name), PSKdecoder1.Input("in");
  // kana� nr 10
  name = "out"; name += to_string(channel2);
  fft->Output(name), PSKdecoder2.Input("in");
  fft->Output(name), SymbData1.Input("in");
  // kana� nr 13
  name = "out"; name += to_string(channel3);
  fft->Output(name), PSKdecoder3.Input("in");

  PSKdecoder1.Output("out") >> BinData1.Input("in");
  PSKdecoder2.Output("out") >> BinData2.Input("in");
  PSKdecoder3.Output("out") >> BinData3.Input("in");
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // FFT z filtrami polifazowymi (tylko dla u�ywanych kana��w)


  // *********************************** //
  DSP::Clock::SchemeToDOTfile(SymbolClock, "Cw5_zad2.dot");
  /////////////////////////////////
  // check if there are signals
  // connected to all inputs
  DSP::Component::CheckInputsOfAllComponents();


  // *********************************** //
  int SamplesInSegment = 512;
  __int64 NoOfSamplesProcessed = 0;
  // 10 seconds
  __int64 MAX_SAMPLES_TO_PROCESS = 1*Fp2;
  while(NoOfSamplesProcessed < MAX_SAMPLES_TO_PROCESS)
  {

    // ********************************************************** //
    DSP::Clock::Execute(InputClock, SamplesInSegment);
    // ********************************************************** //

    /*
    if (BinData1.GetBytesRead() > 0)
    {
        NoOfSamplesProcessed = 0; // Play the whole file
    }
    else // Play 200ms more
    {
      if (NoOfSamplesProcessed < MAX_SAMPLES_TO_PROCESS - Fp2/5)
          NoOfSamplesProcessed = MAX_SAMPLES_TO_PROCESS - Fp2/5;
    }
    */

    NoOfSamplesProcessed += SamplesInSegment;
    // ********************************************************** //

    //sprintf(tekst, "NoOfSamplesProcessed = %i", int(NoOfSamplesProcessed));
    //DSP::log << tekst);
 }

  /*************************************************************/
  for (int ind = 0; ind < K; ind++)
  {
    if (H_g[ind] != NULL)
      delete H_g[ind];
  }
  for (int ind = 0; ind < K-3; ind++)
    delete Discard[ind];

  delete fft;

  /*************************************************************/
  DSP::Clock::FreeClocks();

  return 0;
}


#define buffer_size 4
//DSP::Float_ptr read_buffer = NULL;

void BufferCallback(unsigned int NoOfInputs, unsigned int NoOfOutputs, DSP::Float_vector &OutputSamples, DSP::void_ptr *UserDataPtr, unsigned int UserDefinedIdentifier, DSP::Component_ptr Caller)
{
  UNUSED_ARGUMENT(NoOfOutputs);
  UNUSED_ARGUMENT(UserDataPtr);

  if (NoOfInputs == DSP::c::Callback_Init)
  {
    read_buffer = new DSP::Float[buffer_size];
    return;
  }
  if (NoOfInputs == DSP::c::Callback_Delete)
  {
    delete [] read_buffer;
    read_buffer = NULL;
    return;
  }

  DSP::u::OutputBuffer *dsp_buffer;
  int ind, counter;

  dsp_buffer = (DSP::u::OutputBuffer *)Caller->Convert2Block();
  counter = dsp_buffer->ReadBuffer(read_buffer,
                                   buffer_size*sizeof(DSP::Float), // read all samples
                                   -2,  // reset only NotificationsStep slots in buffer block
                                   DSP::e::SampleType::ST_float); // write to read_buffer in float format

  for (ind = 0; ind < counter; ind++)
  {
    switch (UserDefinedIdentifier)
    {
      case 0:
        OutputSamples[ind] = read_buffer[ind];
        break;
      default:
        if ((ind % 2) == 0)
          OutputSamples[ind] = read_buffer[ind];
        else
          OutputSamples[ind] = -read_buffer[ind];
        break;
    }
  }
  for (ind = counter; ind < buffer_size; ind++)
    OutputSamples[ind] = 0.0;
}

int test_12(void)
{
  DSP::Clock_ptr MasterClock, BufferClock, MuxClock, DemuxClock;
  int temp;
  long int Fp;
  int callback_type;

  DSP::u::WaveInput     *AudioIn;
  DSP::u::OutputBuffer  *OutputBuffer;
  DSP::u::Multiplexer   *Multiplexer;
  DSP::u::AudioOutput   *AudioOut;
  DSP::u::Demultiplexer *Demultiplexer;
  DSP::u::Amplifier     *Scale;
  DSP::u::Multiplexer   *Multiplexer2;

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP_lib_version_string() << endl;

  MasterClock=DSP::Clock::CreateMasterClock();


  AudioIn = new DSP::u::WaveInput(MasterClock, "DSPElib.wav", ".");
  Fp = AudioIn->GetSamplingRate();

  //callback_type = 0; // just copy samples
  callback_type = 1; // inverse spectrum
  OutputBuffer = new DSP::u::OutputBuffer(buffer_size,
                              1,
                              DSP_standard,
                              MasterClock,
                              -1,
                              buffer_size,
                              BufferCallback,
                              callback_type);
  BufferClock = OutputBuffer->GetOutputClock();

  Multiplexer = new DSP::u::Multiplexer (BufferClock, false, buffer_size);
  MuxClock = Multiplexer->GetOutputClock();

  Demultiplexer = new DSP::u::Demultiplexer(false, 2);
  DemuxClock = DSP::Clock::GetClock(MuxClock, 1,2);

  Scale = new DSP::u::Amplifier(-1.0, 1);
  Multiplexer2 = new DSP::u::Multiplexer(DemuxClock, false, 2);

  AudioOut = new DSP::u::AudioOutput(Fp);


  AudioIn->Output("out") >> OutputBuffer->Input("in");
  OutputBuffer->Output("out") >> Multiplexer->Input("in");
  Multiplexer->Output("out") >> Demultiplexer->Input("in");

  Demultiplexer->Output("out1") >> Multiplexer2->Input("in1");
  Demultiplexer->Output("out2") >> Scale->Input("in");
  Scale->Output("out") >> Multiplexer2->Input("in2");

  Multiplexer2->Output("out") >> AudioOut->Input("in");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "callbacks_scheme.dot");

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN"<< DSP::LogMode::second << temp << endl;
    temp++;
  }
  while (AudioIn->GetBytesRead() != 0);

  delete AudioOut;
  delete Multiplexer2;
  delete Scale;
  delete Demultiplexer;
  delete OutputBuffer;
  delete Multiplexer;
  delete AudioIn;

  DSP::Clock::ListOfAllComponents();
  DSP::Clock::FreeClocks();
  DSP::log << "MAIN" << DSP::LogMode::second << "end" << endl;

  return 0;
}

int test_SolveMatrix(int mode) {
  vector<DSP::Float_vector> A_in =
     {{3.0, 1.0, 1.0},
      {0.5, 1.0, 1.5}, // {0.5, 2.0, 1.5},
      {0.1, 1.0, 0.1}}; // matrix coefficients (table of rows)
  DSP::Float_vector B_in = { 1, 1, 1};
  DSP::Float_vector X;   // vector reserved for solution

  {
    DSP::log << "A=[" << endl;
    for (const auto &row: A_in) {
      unsigned int ind = 0;
      stringstream ss;
      ss << "    [";
      for (const auto &val: row) {
        ss << val;
        ind++;
        if (ind != row.size()) {
          ss << ",";
        }
      }
      ss << "]";
      DSP::log << ss.str() << endl;
    }
    DSP::log << "  ]" << endl;
  }
  {
    unsigned int ind = 0;
    stringstream ss;
    ss << "B_in = [";
    for (const auto &val: B_in) {
      ss << val;
      ind++;
      if (ind != B_in.size()) {
        ss << ",";
      }
    }
    ss << "]";
    DSP::log << ss.str() << endl;
  }
  switch (mode) {
    case 0:
      DSP::log << "DSP::f::SolveMatrixEqu(A_in, X, B_in);" << endl;
      DSP::f::SolveMatrixEqu(A_in, X, B_in);
      break;

    case 1:
      DSP::log << "DSP::f::SolveMatrixEqu_prec(A_in, X, B_in);" << endl;
      DSP::f::SolveMatrixEqu_prec(A_in, X, B_in);
      break;

    default:
      DSP::log << "test_SolveMatrix" << DSP::LogMode::second << "unsupported mode" << endl;
      break;
  }
  {
    unsigned int ind = 0;
    stringstream ss;
    ss << "X = [";
    for (const auto &val: X) {
      ss << val;
      ind++;
      if (ind != X.size()) {
        ss << ",";
      }
    }
    ss << "]";
    DSP::log << ss.str() << endl;
  }

//  \TODO test also DSP::f::LPF_LS();

//  DSP::Float A_in[]= {3.0, 1.0, 1.0,
//                     0.5, 1.0, 1.5,
//                     0.1, 1.0, 0.1};
//  DSP::Float X[3];
//  DSP::Float B_in[]= { 1, 1, 1};
//  SolveMatrixEqu_prec(3, A_in, X, B_in);

  return 0;
}

int test_SolveMatrix_prec(int mode) {
  vector<DSP::Prec_Float_vector> A_in =
     {{3.0, 1.0, 1.0},
      {0.5, 1.0, 1.5}, // {0.5, 2.0, 1.5},
      {0.1, 1.0, 0.1}}; // matrix coefficients (table of rows)
  DSP::Prec_Float_vector B_in = { 1, 1, 1};
  DSP::Prec_Float_vector X;   // vector reserved for solution

  {
    DSP::log << "A=[" << endl;
    for (const auto &row: A_in) {
      unsigned int ind = 0;
      stringstream ss;
      ss << "    [";
      for (const auto &val: row) {
        ss << val;
        ind++;
        if (ind != row.size()) {
          ss << ",";
        }
      }
      ss << "]";
      DSP::log << ss.str() << endl;
    }
    DSP::log << "  ]" << endl;
  }
  {
    unsigned int ind = 0;
    stringstream ss;
    ss << "B_in = [";
    for (const auto &val: B_in) {
      ss << val;
      ind++;
      if (ind != B_in.size()) {
        ss << ",";
      }
    }
    ss << "]";
    DSP::log << ss.str() << endl;
  }
  switch (mode) {
    case 0:
      DSP::log << "DSP::f::SolveMatrixEqu_prec(A_in, X, B_in, 0);" << endl;
      DSP::f::SolveMatrixEqu_prec(A_in, X, B_in, 0);
      break;

    case 1:
      DSP::log << "DSP::f::SolveMatrixEqu_prec(A_in, X, B_in, 1);" << endl;
      DSP::f::SolveMatrixEqu_prec(A_in, X, B_in, 1);
      break;

    case 2:
      DSP::log << "DSP::f::SolveMatrixEqu_prec(A_in, X, B_in, 2);" << endl;
      DSP::f::SolveMatrixEqu_prec(A_in, X, B_in, 2);
      break;

    default:
      DSP::log << "test_SolveMatrix_prec" << DSP::LogMode::second << "unsupported mode" << endl;
      break;
  }
  {
    unsigned int ind = 0;
    stringstream ss;
    ss << "X = [";
    for (const auto &val: X) {
      ss << val;
      ind++;
      if (ind != X.size()) {
        ss << ",";
      }
    }
    ss << "]";
    DSP::log << ss.str() << endl;
  }

//  \TODO test also DSP::f::LPF_LS();

//  DSP::Float A_in[]= {3.0, 1.0, 1.0,
//                     0.5, 1.0, 1.5,
//                     0.1, 1.0, 0.1};
//  DSP::Float X[3];
//  DSP::Float B_in[]= { 1, 1, 1};
//  SolveMatrixEqu_prec(3, A_in, X, B_in);

  return 0;
}

int test_SymbolMapper() {
  DSP::Clock_ptr BitClock, SymbolClock;
  map<string,shared_ptr<DSP::Component> > blocks;

  int bits_per_symbol = 3;

  BitClock = DSP::Clock::CreateMasterClock();
  blocks["binary_stream"] = shared_ptr<DSP::Source>(new DSP::u::BinRand(BitClock));

  blocks["file_bin"] = shared_ptr<DSP::Block>(new DSP::u::FileOutput("bin_input.txt", DSP::e::SampleType::ST_bit_text, 1, DSP::e::FileType::FT_raw));
//  sources["binary_stream"]->Output("out"),blocks["file_bin"]->Input("in");
  blocks["binary_stream"]->Output("out") >> blocks["file_bin"]->Input("in");

  blocks["SPconv"] = make_shared<DSP::u::Serial2Parallel>(BitClock, bits_per_symbol);
  blocks["mapper"] = make_shared<DSP::u::SymbolMapper>(DSP_MT_ASK, bits_per_symbol);
  blocks["binary_stream"]->Output("out") >> blocks["SPconv"]->Input("in");
  blocks["SPconv"]->Output("out") >> blocks["mapper"]->Input("in");
  SymbolClock = blocks["mapper"]->GetOutputClock();
////  SymbolClock = DSP::Clock::GetClock(BitClock, 1, ((DSP::u::SymbolMapper *)blocks["mapper"])->getBitsPerSymbol());
//  SymbolClock = DSP::Clock::GetClock(BitClock, 1, bits_per_symbol);

  unsigned int noChannels = 1;
  if (dynamic_cast<DSP::u::SymbolMapper *>(blocks["mapper"].get())->isOutputReal() == false)
    noChannels = 2;

  blocks["file_symb"] = shared_ptr<DSP::Block>(new DSP::u::FileOutput("symb_output.flt", DSP::e::SampleType::ST_float, noChannels, DSP::e::FileType::FT_flt));
  blocks["mapper"]->Output("out"),blocks["file_symb"]->Input("in");

  blocks["demapper"] = make_shared<DSP::u::SymbolDemapper>(DSP_MT_ASK, bits_per_symbol);
  blocks["mapper"]->Output("out") >> blocks["demapper"]->Input("in");
  blocks["PSconv"] = make_shared<DSP::u::Parallel2Serial>(SymbolClock, bits_per_symbol);
  blocks["demapper"]->Output("out") >> blocks["PSconv"]->Input("in");


  blocks["file_bin_recovered"] = shared_ptr<DSP::Block>(new DSP::u::FileOutput("bin_output.flt", DSP::e::SampleType::ST_float, 1, DSP::e::FileType::FT_flt));
  blocks["file_bin_recovered2"] = shared_ptr<DSP::Block>(new DSP::u::FileOutput("bin_output.txt", DSP::e::SampleType::ST_bit_text, 1, DSP::e::FileType::FT_raw));
  blocks["PSconv"]->Output("out"),blocks["file_bin_recovered"]->Input("in");
  blocks["PSconv"]->Output("out"),blocks["file_bin_recovered2"]->Input("in");

  /////////////////////////////////
  // check if there are signals
  // connected to all inputs
  DSP::Component::CheckInputsOfAllComponents();

  // *********************************** //
  DSP::Clock::SchemeToDOTfile(BitClock, "SymbolMapper.dot");

  // ********************************************************** //
  DSP::Clock::Execute(BitClock, 1000);

  /*************************************************************/
  blocks.clear();

  DSP::Clock::ListOfAllComponents();
  /*************************************************************/
  DSP::Clock::FreeClocks();

  return 0;
}

int test_ZPSTC_cw_3()
{
  long int Fp1, Fp2, F_symb;
  DSP::LoadCoef coef_info;
  int N_rc, N2;
  unsigned int L1, L2;
  DSP::Float_vector h_rc, h2;

  coef_info.Open("cw3_zad3_h_rc.coef", "matlab");
  N_rc = coef_info.GetSize(0);
  if (N_rc < 1)
  {
    DSP::log << DSP::LogMode::Error << "No cw3_zad3_h_rc.coef: aborting" << endl;
    return -1;
  }
  else
  {
    h_rc.resize(N_rc);
    coef_info.Load(h_rc);
    Fp1 = coef_info.Fp;
  }
  /*************************************************************/
  coef_info.Open("cw3_zad3_h2.coef", "matlab");
  N2 = coef_info.GetSize(0);
  if (N2 < 1)
  {
    DSP::log << DSP::LogMode::Error << "No cw3_zad3_h2.coef: aborting" << endl;
    return -1;
  }
  else
  {
    h2.resize(N2);
    coef_info.Load(h2);
    Fp2 = coef_info.Fp;
  }
  /*************************************************************/

  DSP::Clock_ptr SymbolClock, SecondClock;
  SymbolClock=DSP::Clock::CreateMasterClock();


  //DSP::u::WaveInput AudioIn(MasterClock, "DSPElib.wav", ".");
  //F_symb = AudioIn.GetSamplingRate();

  DSP::u::FileInput BinData(SymbolClock, "/Dev-Cpp/ZPSTC/Cw3/Cw3_zad3.cpp", 2U, DSP::e::SampleType::ST_bit, DSP::e::FileType::FT_raw);
  F_symb = 2400;
  DSP::u::PSKencoder PSKencoder(DSP_QPSK_A);

  L1 = Fp1 / F_symb;
  L2 = Fp2 / Fp1;
  stringstream ss;
  ss << "Fsymb = " << F_symb << ", Fp1 = " << Fp1 << ", Fp2 = " << Fp2 << ", L1 = " << L1 << ", L2 = " << L2;
  DSP::log << ss.str() << endl;

  SecondClock=DSP::Clock::GetClock(SymbolClock, L2, 1);

  DSP::u::SamplingRateConversion SRC2(true, SecondClock, L1, 1, h_rc);
  SRC2.SetName("SRC1");

  DSP::u::SamplingRateConversion SRC1(true, SymbolClock, L2, 1, h2);
  SRC1.SetName("SRC2");
  DSP::u::DDScos Heter(SRC2.GetOutputClock(), true, 0.5, DSP::Float(M_PIx2*2500/Fp2));
  DSP::u::Multiplication Mul(0, 2);
  DSP::u::Vacuum V1;


  // Output to the soundcard
  DSP::u::AudioOutput SoundOut(Fp2, 1, 16);
  DSP::u::FileOutput FileOut1("cw3_zad3a.flt", DSP::e::SampleType::ST_float, 2, DSP::e::FileType::FT_flt, Fp1);
  // Output to the mono 16bit *.wav file
  DSP::u::FileOutput FileOut2a("cw3_zad3b.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp2);
  DSP::u::FileOutput FileOut2b("cw3_zad3b.flt", DSP::e::SampleType::ST_float, 1, DSP::e::FileType::FT_flt, Fp2);

// ???
  DSP::u::FileOutput FileOut_test("test.flt", DSP::e::SampleType::ST_float, 2, DSP::e::FileType::FT_flt, Fp2);

  /*************************************************************/
  // Connections definitions
  //AudioIn.Output("out") >> SRC1.Input("in");
  BinData.Output("out") >> PSKencoder.Input("in");
  PSKencoder.Output("out") >> SRC1.Input("in");

  SRC1.Output("out") >> FileOut1.Input("in");
  SRC1.Output("out") >> SRC2.Input("in");

  SRC2.Output("out") >> Mul.Input("in1");
  Heter.Output("out") >> Mul.Input("in2");

//???
  SRC2.Output("out") >> FileOut_test.Input("in");

  Mul.Output("out.im") >> V1.Input("in");

  Mul.Output("out.re") >> SoundOut.Input("in");
  Mul.Output("out.re") >> FileOut2a.Input("in");
  Mul.Output("out.re") >> FileOut2b.Input("in");


  /////////////////////////////////
  // check if there are signals
  // connected to all inputs
  DSP::Component::CheckInputsOfAllComponents();

  // *********************************** //
  DSP::Clock::SchemeToDOTfile(SymbolClock, "Cw3_zad3.dot");

  // *********************************** //
  int SamplesInSegment = 4*512;
  __int64 NoOfSamplesProcessed = 0;
  // 10 seconds
  __int64 MAX_SAMPLES_TO_PROCESS = 10*Fp1;
  while(NoOfSamplesProcessed < MAX_SAMPLES_TO_PROCESS)
  {

    // ********************************************************** //
    DSP::Clock::Execute(SymbolClock, SamplesInSegment);
    // ********************************************************** //

    int bytes_read = BinData.GetBytesRead();
    DSP::log << "BinData.GetBytesRead() = " << bytes_read << endl;
    if (bytes_read > 0)
    {
        NoOfSamplesProcessed = 0; // Play the whole file
    }
    else // Play 200ms more
    {
      if (NoOfSamplesProcessed < MAX_SAMPLES_TO_PROCESS - Fp1/5)
          NoOfSamplesProcessed = MAX_SAMPLES_TO_PROCESS - Fp1/5;
    }

    NoOfSamplesProcessed += SamplesInSegment;
    // ********************************************************** //
  }

  /*************************************************************/
  DSP::Clock::FreeClocks();

  return 0;
}

int main(int argc, char*argv[])
{
  //  DSP::f::SetLogState(DSP_LS_console | DSP_LS_file);
  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("DSPElib_test_log.txt");

  DSP::log << "test DSP::log" << endl;
  DSP::log << "test DSP::log(2)" << DSP::LogMode::second << "2" << endl;
  DSP::log << DSP::LogMode::Error << "test DSP::log error" << endl;

  DSP::log << "Starting SolveMatrix test" << endl;
  for (auto i=0; i<3; i++) {
    test_SolveMatrix(i);
  }
  for (auto i=0; i<4; i++) {
    test_SolveMatrix_prec(i);
  }
  //! \TODO test DSP::f::LPF_LS
  DSP::log << DSP::LogMode::pause << "Finished SolveMatrix test" << endl;

  DSP::log << "Starting SymbolMapper test" << endl;
  test_SymbolMapper();
  DSP::log << DSP::LogMode::pause << "Finished SymbolMapper test" << endl;

  DSP::log << "Starting test_ZPSTC_cw_3" << endl;
  test_ZPSTC_cw_3();
  DSP::log << "Finished test_ZPSTC_cw_3" << DSP::LogMode::pause << endl;

#ifdef INCLUDE_DSPE_EXAMPLES
  DSP::log << "Starting test_hello" << endl;
  test_hello();
  DSP::log << DSP::LogMode::Error << "Finished test_hello" << endl;
#endif // INCLUDE_DSPE_EXAMPLES

  DSP::log << "Starting test_1" << endl;
  test_1(argc, argv);
  DSP::log << DSP::LogMode::pause << "Finished test_1" << endl;

  DSP::log << "Starting test_2" << endl;
  test_2();
  DSP::log << DSP::LogMode::pause << "Finished test_2" << endl;

  DSP::log << "Starting test_3" << endl;
  test_3();
  DSP::log << DSP::LogMode::pause << "Finished test_3" << endl;

  DSP::log << "Starting test_4" << endl;
  test_4();
  DSP::log << DSP::LogMode::pause << "Finished test_4" << endl;

  DSP::log << "Starting test_5" << endl;
  test_5();
  DSP::log << DSP::LogMode::pause << "Finished test_5" << endl;

  DSP::log << "Starting test_6" << endl;
  test_6();
  DSP::log << DSP::LogMode::pause << "Finished test_6" << endl;

  DSP::log << "Starting test_7" << endl;
  test_7();
  DSP::log << DSP::LogMode::pause << "Finished test_7" << endl;

  DSP::log << "Starting test_8" << endl;
  test_8();
  DSP::log << DSP::LogMode::pause << "Finished test_8" << endl;

  DSP::log << "Starting test_9" << endl;
  test_9();
  DSP::log << DSP::LogMode::pause << "Finished test_9" << endl;

  DSP::log << "Starting test_10" << endl;
  test_10();
  DSP::log << DSP::LogMode::pause << "Finished test_10" << endl;

  DSP::log << "Starting test_11" << endl;
  test_11();
  DSP::log << DSP::LogMode::pause << "Finished test_11" << endl;

  DSP::log << "Starting test_12" << endl;
  test_12();
  DSP::log << DSP::LogMode::pause << "Finished test_12" << endl;
}
