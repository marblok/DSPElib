#include <DSP_modules2.h>
#include <DSP_clocks.h>

#include <sstream>
#include <iomanip>      // std::setprecision

/**************************************************/
// AGC - automatic gain control
DSP::u::AGC::AGC(DSP::Float alfa_in, DSP::Float init_signal_power,
                 DSP::Float output_signal_power)
  : DSP::Block()
{
  SetName("AGC", false);

  SetNoOfInputs(2, false);
  DefineInput("in", 0, 1);
  DefineInput("in.re", 0);
  DefineInput("in.im", 1);
  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  alfa=alfa_in;
  signal_power=init_signal_power;
  out_power=(DSP::Float)sqrt(output_signal_power);

  Execute_ptr = &InputExecute;
}

DSP::u::AGC::~AGC()
{
}

void DSP::u::AGC::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Float temp;

  if (InputNo==0)
    ((DSP::u::AGC *)block)->input.re = value;
  else
    ((DSP::u::AGC *)block)->input.im = value;
  ((DSP::u::AGC *)block)->NoOfInputsProcessed++;

  if (((DSP::u::AGC *)block)->NoOfInputsProcessed < ((DSP::u::AGC *)block)->NoOfInputs)
    return;
  ((DSP::u::AGC *)block)->NoOfInputsProcessed = 0;

  ((DSP::u::AGC *)block)->signal_power =
      ((DSP::u::AGC *)block)->signal_power * (1-((DSP::u::AGC *)block)->alfa) +
       ((DSP::u::AGC *)block)->alfa *
              (((DSP::u::AGC *)block)->input.re * ((DSP::u::AGC *)block)->input.re +
               ((DSP::u::AGC *)block)->input.im * ((DSP::u::AGC *)block)->input.im);
//  temp=1.0/sqrt(signal_power);
  temp = ((DSP::u::AGC *)block)->out_power / SQRT(((DSP::u::AGC *)block)->signal_power);

//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], input.re*temp, this);
  ((DSP::u::AGC *)block)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::AGC *)block)->OutputBlocks[0],
      ((DSP::u::AGC *)block)->OutputBlocks_InputNo[0],
      ((DSP::u::AGC *)block)->input.re * temp, block);
//  OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], input.im*temp, this);
  ((DSP::u::AGC *)block)->OutputBlocks[1]->EXECUTE_PTR(
      ((DSP::u::AGC *)block)->OutputBlocks[1],
      ((DSP::u::AGC *)block)->OutputBlocks_InputNo[1],
      ((DSP::u::AGC *)block)->input.im * temp, block);
};

DSP::Float DSP::u::AGC::GetPower(void)
{
  return signal_power;
}


// ***************************************************** //
// Signal to noise ratio estimation for BPSK modulation
/*
 *  Inputs and Outputs names:
 *   - Output:
 *    -# "snr" - estimated SNR value
 *   - Input:
 *    -# "in" - complex valued signal
 *    -# "in.re" - real component
 *    -# "in.im" - imag component
 *
 */
DSP::u::BPSK_SNR_estimator::BPSK_SNR_estimator(int segment_size)
  : DSP::Block()
{
  SetName("BPSK_SNR_estimator", false);

  SetNoOfInputs(2, false);
  DefineInput("in", 0, 1);
  DefineInput("in.re", 0);
  DefineInput("in.im", 1);

  SetNoOfOutputs(1);
  DefineOutput("snr", 0);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  SegmentSize = segment_size;
  RealBuffer.clear();
  RealBuffer.resize(SegmentSize, 0.0);
  current_ind = 0;

  Input_Real = 0.0; Input_Imag = 0.0;
  EstimatedSNR = 0.0;

  Execute_ptr = &InputExecute;
}

DSP::u::BPSK_SNR_estimator::~BPSK_SNR_estimator(void)
{
  RealBuffer.clear();
}

void DSP::u::BPSK_SNR_estimator::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo == 0)
    ((DSP::u::BPSK_SNR_estimator *)block)->Input_Real = value;
  else
    ((DSP::u::BPSK_SNR_estimator *)block)->Input_Imag = value;
  ((DSP::u::BPSK_SNR_estimator *)block)->NoOfInputsProcessed++;

  if (((DSP::u::BPSK_SNR_estimator *)block)->NoOfInputsProcessed < ((DSP::u::BPSK_SNR_estimator *)block)->NoOfInputs)
    return;

  /*
   * Input samples processing
   */
  DSP::Float s_abs, s_smoothed, v_BPSK, S, N;
  s_smoothed=0.0;

  s_abs=FABS(((DSP::u::BPSK_SNR_estimator *)block)->Input_Real);

  s_smoothed = s_smoothed - ((DSP::u::BPSK_SNR_estimator *)block)->RealBuffer[((DSP::u::BPSK_SNR_estimator *)block)->current_ind] + s_abs;
  ((DSP::u::BPSK_SNR_estimator *)block)->RealBuffer[((DSP::u::BPSK_SNR_estimator *)block)->current_ind] = s_abs;
  ((DSP::u::BPSK_SNR_estimator *)block)->current_ind =
      ((((DSP::u::BPSK_SNR_estimator *)block)->current_ind + 1) % ((DSP::u::BPSK_SNR_estimator *)block)->SegmentSize);

  v_BPSK = s_smoothed / DSP::Float(((DSP::u::BPSK_SNR_estimator *)block)->SegmentSize);

  ((DSP::u::BPSK_SNR_estimator *)block)->Input_Real = s_abs - v_BPSK;

  S = v_BPSK * v_BPSK;
  N = ((DSP::u::BPSK_SNR_estimator *)block)->Input_Real * ((DSP::u::BPSK_SNR_estimator *)block)->Input_Real +
      ((DSP::u::BPSK_SNR_estimator *)block)->Input_Imag * ((DSP::u::BPSK_SNR_estimator *)block)->Input_Imag;

  ((DSP::u::BPSK_SNR_estimator *)block)->EstimatedSNR = S / N; //linear

//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], EstimatedSNR, this);
  ((DSP::u::BPSK_SNR_estimator *)block)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::BPSK_SNR_estimator *)block)->OutputBlocks[0],
      ((DSP::u::BPSK_SNR_estimator *)block)->OutputBlocks_InputNo[0],
      ((DSP::u::BPSK_SNR_estimator *)block)->EstimatedSNR, block);
  ((DSP::u::BPSK_SNR_estimator *)block)->NoOfInputsProcessed = 0;
};

/* SNR estimation for PSK modulation (BPSK & QPSK) based on complex symbol samples
 * 1) buffer_size is in symbols
 * 2) buffer contains 2*buffer_size floating point values (real & imag)
 *
 * \note Buffer content is preserved.
 */
void DSP::f::PSK_SNR_estimator(const int &buffer_size, const DSP::Float_vector &buffer,
                             DSP::Float &BPSK_SNR, DSP::Float &QPSK_SNR)
{
  DSP::Float s_re_abs, s_im_abs;
  DSP::Float v_BPSK, S_BPSK, N_BPSK;
  DSP::Float v_QPSK, S_QPSK, N_QPSK;
  int ind;

  //constelation points position estimation for BPSK & QPSK
  s_re_abs= FABS(buffer[0]);
  s_im_abs= FABS(buffer[1]);
  for (ind = 1; ind < buffer_size; ind++)
  {
    s_re_abs += FABS(buffer[2*ind]);
    s_im_abs += FABS(buffer[2*ind+1]);
  }
  s_re_abs /= (DSP::Float)buffer_size;
  s_im_abs /= (DSP::Float)buffer_size;

  v_BPSK = s_re_abs;
  v_QPSK = (v_BPSK + s_im_abs)/2;


  // Signal power for BPSK
  N_BPSK = (FABS(buffer[0]) - v_BPSK) * (FABS(buffer[0]) - v_BPSK);
  N_BPSK += (buffer[1]) * (buffer[1]);
  for (ind = 1; ind < buffer_size; ind++)
  {
    N_BPSK += (FABS(buffer[2*ind]) - v_BPSK) * (FABS(buffer[2*ind]) - v_BPSK);
    N_BPSK += (buffer[2*ind+1]) * (buffer[2*ind+1]);
  }
  N_BPSK /= (DSP::Float)buffer_size;
  S_BPSK = v_BPSK*v_BPSK;

  // Signal power for QPSK
  N_QPSK = (FABS(buffer[0]) - v_QPSK) * (FABS(buffer[0]) - v_QPSK);
  for (ind = 1; ind < 2*buffer_size; ind++)
  {
    N_QPSK += (FABS(buffer[ind]) - v_QPSK) * (FABS(buffer[ind]) - v_QPSK);
  }
  N_QPSK /= (DSP::Float)buffer_size;
  S_QPSK = 2*v_QPSK*v_QPSK;


  BPSK_SNR = S_BPSK / N_BPSK; //linear
  QPSK_SNR = S_QPSK / N_QPSK; //linear
};


/**************************************************/
// DynamicCompressor - changes input signal dynamic
/*
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" - real or complex output signal
 *   -# "out.re" - real part of output signal
 *   -# "out.im" - imaginary part of output signal (if it exists)
 *  - Input:
 *   -# "in" - real or complex input signal
 *   -# "in.re" - real part of input signal
 *   -# "in.im" - imaginary part of input signal (if it exists)
 *
 */
DSP::u::DynamicCompressor::DynamicCompressor(
                           int BufferSize_in, DSP::Float a0, DSP::Float Po_dB,
                           bool IsInputComplex, int OutputDelay_in)
  : DSP::Block()
{
  SetName("DynamicCompressor", false);

  if (IsInputComplex == true)
  {
    SetNoOfInputs(2, false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);

    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    SetNoOfInputs(1, false);
    DefineInput("in", 0);
    DefineInput("in.re", 0);

    SetNoOfOutputs(1);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  if (BufferSize_in <= 0)
  {
    #ifdef __DEBUG__
      DSP::log << "DSP::u::DynamicCompressor::DynamicCompressor" << DSP::LogMode::second << "BufferSize_in <= 0" << endl;
    #endif
    BufferSize_in = 1;
  }
  if (OutputDelay_in < -1)
  {
    #ifdef __DEBUG__
      DSP::log << "DSP::u::DynamicCompressor::DynamicCompressor" << DSP::LogMode::second << "OutputDelay_in < -1" << endl;
    #endif
    OutputDelay_in = 0;
  }
  if (OutputDelay_in >= BufferSize_in)
  {
    #ifdef __DEBUG__
      DSP::log << "DSP::u::DynamicCompressor::DynamicCompressor" << DSP::LogMode::second << "OutputDelay_in >= BufferSize_in" << endl;
    #endif
    OutputDelay_in = BufferSize_in - 1;
  }
  if (OutputDelay_in == -1)
  {
    OutputDelay_in = (BufferSize_in - 1) / 2;
  }

  index = 0;
  power_index = 0;
  if (IsInputComplex == true)
  {
    SamplesBufferSize = OutputDelay_in * 2;
  }
  else
  {
    SamplesBufferSize = OutputDelay_in;
  }

  SamplesBuffer.clear();
  SamplesBuffer.resize(SamplesBufferSize, 0.0);

  PowerBufferSize = BufferSize_in;
  PowerBuffer.clear();
  PowerBuffer.resize(PowerBufferSize, 0.0);

  CurrentPower = 0.0;
  alfa = (DSP::Float(1.0)/a0 - 1)/2;
  factor_0 = POW( POW(10, Po_dB/10), -alfa);

  if (OutputDelay_in == 0)
    if (IsInputComplex == true)
      Execute_ptr = &InputExecute_no_delay_complex;
    else
      Execute_ptr = &InputExecute_no_delay_real;
  else
    if (IsInputComplex == true)
      Execute_ptr = &InputExecute_complex;
    else
      Execute_ptr = &InputExecute_real;
}

DSP::u::DynamicCompressor::~DynamicCompressor()
{
  SamplesBuffer.clear();
  PowerBuffer.clear();
}

#define  THIS  ((DSP::u::DynamicCompressor *)block)
// first read buffer then write to it
void DSP::u::DynamicCompressor::InputExecute_no_delay_real(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Float factor;

  THIS->CurrentPower -= THIS->PowerBuffer[THIS->power_index];
  THIS->PowerBuffer[THIS->power_index] = (value * value) / DSP::Float(THIS->PowerBufferSize);
  THIS->CurrentPower += THIS->PowerBuffer[THIS->power_index];
  THIS->power_index++; THIS->power_index %= THIS->PowerBufferSize;

  if (THIS->CurrentPower >= 10E-12)
    factor = THIS->factor_0 * POW(THIS->CurrentPower, THIS->alfa);
  else
    factor = 1.0;

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      value * factor, block);
};

void DSP::u::DynamicCompressor::InputExecute_no_delay_complex(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo == 0)
    THIS->temp_value.re = value;
  else
    THIS->temp_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    DSP::Float factor;

    THIS->CurrentPower -= THIS->PowerBuffer[THIS->power_index];
    THIS->PowerBuffer[THIS->power_index] =
        ((THIS->temp_value.re * THIS->temp_value.re) +
         (THIS->temp_value.im * THIS->temp_value.im))
        / DSP::Float(THIS->PowerBufferSize);
    THIS->CurrentPower += THIS->PowerBuffer[THIS->power_index];
    THIS->power_index++; THIS->power_index %= THIS->PowerBufferSize;

    if (THIS->CurrentPower >= 10E-12)
      factor = THIS->factor_0 * POW(THIS->CurrentPower, THIS->alfa);
    else
      factor = 1.0;

    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
        THIS->temp_value.re * factor, block);
    THIS->OutputBlocks[1]->EXECUTE_PTR(
        THIS->OutputBlocks[1], THIS->OutputBlocks_InputNo[1],
        THIS->temp_value.im * factor, block);

    THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;
  }
};

void DSP::u::DynamicCompressor::InputExecute_real(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Float factor;

  THIS->CurrentPower -= THIS->PowerBuffer[THIS->power_index];
  THIS->PowerBuffer[THIS->power_index] = (value * value) / DSP::Float(THIS->PowerBufferSize);
  THIS->CurrentPower += THIS->PowerBuffer[THIS->power_index];
  THIS->power_index++; THIS->power_index %= THIS->PowerBufferSize;

  if (THIS->CurrentPower >= 10E-12)
    factor = THIS->factor_0 * POW(THIS->CurrentPower, THIS->alfa);
  else
    factor = 1.0;

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      THIS->SamplesBuffer[THIS->index] * factor, block);

  THIS->SamplesBuffer[THIS->index] = value;
  THIS->index++; THIS->index %= THIS->SamplesBufferSize;
};

/*! \todo_later <b>2006.08.13</b> reconsider condition (THIS->CurrentPower >= 10E-12)
 *     processing
 */
void DSP::u::DynamicCompressor::InputExecute_complex(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo == 0)
    THIS->temp_value.re = value;
  else
    THIS->temp_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    DSP::Float factor;

    THIS->CurrentPower -= THIS->PowerBuffer[THIS->power_index];
    THIS->PowerBuffer[THIS->power_index] =
        ((THIS->temp_value.re * THIS->temp_value.re) +
        (THIS->temp_value.im * THIS->temp_value.im))
       / DSP::Float(THIS->PowerBufferSize);
    THIS->CurrentPower += THIS->PowerBuffer[THIS->power_index];
    THIS->power_index++; THIS->power_index %= THIS->PowerBufferSize;

    if (THIS->CurrentPower >= 10E-12)
      factor = THIS->factor_0 * POW(THIS->CurrentPower, THIS->alfa);
    else
      factor = 1.0;

    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
        THIS->SamplesBuffer[0] * factor, block);
    THIS->OutputBlocks[1]->EXECUTE_PTR(
        THIS->OutputBlocks[1], THIS->OutputBlocks_InputNo[1],
        THIS->SamplesBuffer[1] * factor, block);

    THIS->SamplesBuffer[THIS->index] = THIS->temp_value.re;
    THIS->index++;
    THIS->SamplesBuffer[THIS->index] = THIS->temp_value.im;
    THIS->index++; THIS->index %= THIS->SamplesBufferSize;

    THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;
  }
};
#undef THIS


///**************************************************/
//DSP::u::Farrow::Farrow(bool IsComplex, unsigned int N_FSD_in, unsigned int order_in, DSP::Float_ptr *Farrow_coefs_in,
//                         DSP::Clock_ptr InputClock, DSP::Clock_ptr OutputClock)
//  : DSP::Block(), DSP::Source()
//{
//  unsigned int ind, ind2;
//
//  SetName("Farrow", false);
//  epsilon = 0.0;
//
//  if (IsComplex == false)
//  {
//    SetNoOfOutputs(1);
//    SetNoOfInputs(2, false);
//    IsMultiClock=false; // single output clock
//
//    DefineInput("in", 0);
//    DefineInput("in.re", 0);
//    DefineInput("eps", 1);
//    DefineOutput("out", 0);
//    DefineOutput("out.re", 0);
//  }
//  else
//  {
//    SetNoOfOutputs(2);
//    SetNoOfInputs(3, false);
//    IsMultiClock=false; // single output clock
//
//    DefineInput("in", 0, 1);
//    DefineInput("in.re", 0);
//    DefineInput("in.im", 1);
//    DefineInput("eps", 2);
//    DefineOutput("out", 0, 1);
//    DefineOutput("out.re", 0);
//    DefineOutput("out.im", 1);
//  }
//
//  if (InputClock == NULL)
//  {
//    DSP::f::ErrorMessage("DSP::u::Farrow", "Undefined InputClock (AutoUpdate not implemented yet)");
//    return;
//  }
//  else
//  {  //Register for notifications
//    //if (InputClock->MasterClockIndex == OutputClock->MasterClockIndex)
//    if (InputClock == OutputClock)
//    {
//      RegisterForNotification(InputClock); // notifications for "in" and "eps"
//    }
//    else
//    {
//      RegisterForNotification(InputClock); // notifications for "in"
//      RegisterForNotification(OutputClock);// notifications for "eps"
//    }
//  }
//
//  RegisterOutputClock(OutputClock);
//  int L_factor, M_factor;
//  IsMultirate = GetMultirateFactorsFromClocks(InputClock, OutputClock, L_factor, M_factor, false);
//  //IsMultirate = true;  L_factor = -1; M_factor = -1;
//  ClockGroups.AddInput2Group("input", Input("in"));
//  ClockGroups.AddOutput2Group("output", Output("out"));
//  ClockGroups.AddInput2Group("output", Input("eps"));
//  ClockGroups.AddClockRelation("input", "output", L_factor, M_factor);
//
//  N_FSD = N_FSD_in;
//  if (NoOfOutputs == 1)
//  {
//    Buffer = (uint8_t *)(new DSP::Float[N_FSD]);
//    for (ind=0; ind<N_FSD; ind++)
//      ((DSP::Float *)Buffer)[ind] = 0.0;
//
//    Execute_ptr = &InputExecute_real;
//  }
//  else
//  {
//    Buffer = (uint8_t *)(new DSP::Complex[N_FSD]);
//    for (ind=0; ind<N_FSD; ind++)
//      ((DSP::Complex *)Buffer)[ind] = DSP::Complex(0.0, 0.0);
//
//    Execute_ptr = &InputExecute_cplx;
//  }
//
//  ///////////////////////////////////
//  Farrow_len = order_in+1;
//  Farrow_coefs = new DSP::Float_ptr[Farrow_len];
//  for (ind = 0; ind < Farrow_len; ind++)
//  {
//    Farrow_coefs[ind] = new DSP::Float[N_FSD];
//  }
//
////  // copy with transposition
////  std::stringstream ss;
////  ss <<"Farrow coefficients" << std::endl;
////  ss <<" Farrow_len = " << Farrow_len << ", N_FSD = " << N_FSD << std::endl;
////  DSP::f::InfoMessage(ss.str().c_str());
//  for (ind = 0; ind < Farrow_len; ind++)
//    for (ind2 = 0; ind2 < N_FSD; ind2++)
//    {
//      Farrow_coefs[ind][ind2] = Farrow_coefs_in[ind2][ind];
////      ss = std::stringstream("");
////      ss <<" Farrow_coefs[ = " << ind << "][" << ind2 << "]=" << Farrow_coefs[ind][ind2] << std::endl;
////      DSP::f::InfoMessage(ss.str().c_str());
//    }
//
//  // work as if sample and delay are ready
//  // if not this variables will be reset in
//  // Notification function
//  InputSampleReady = true;
//  DelayReady = true;
//
//  OutputExecute_ptr = &OutputExecute;
//}

DSP::u::Farrow::Farrow(const bool& IsComplex, const vector<DSP::Float_vector>& Farrow_coefs_in,
  const DSP::Clock_ptr& InputClock, const DSP::Clock_ptr& OutputClock)
  : DSP::Block(), DSP::Source()
{
  unsigned int ind, ind2;

  SetName("Farrow", false);
  epsilon = 0.0;

  if (IsComplex == false)
  {
    SetNoOfOutputs(1);
    SetNoOfInputs(2, false);
    IsMultiClock = false; // single output clock

    DefineInput("in", 0);
    DefineInput("in.re", 0);
    DefineInput("eps", 1);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }
  else
  {
    SetNoOfOutputs(2);
    SetNoOfInputs(3, false);
    IsMultiClock = false; // single output clock

    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineInput("eps", 2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }

  if (InputClock == NULL)
  {
    DSP::log << DSP::LogMode::Error << "DSP::u::Farrow" << DSP::LogMode::second << "Undefined InputClock (AutoUpdate not implemented yet)" << endl;
    return;
  }
  else
  {  //Register for notifications
    //if (InputClock->MasterClockIndex == OutputClock->MasterClockIndex)
    if (InputClock == OutputClock)
    {
      RegisterForNotification(InputClock); // notifications for "in" and "eps"
    }
    else
    {
      RegisterForNotification(InputClock); // notifications for "in"
      RegisterForNotification(OutputClock);// notifications for "eps"
    }
  }

  RegisterOutputClock(OutputClock);
  long L_factor, M_factor;
  IsMultirate = GetMultirateFactorsFromClocks(InputClock, OutputClock, L_factor, M_factor, false);
  //IsMultirate = true;  L_factor = -1; M_factor = -1;
  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));
  ClockGroups.AddInput2Group("output", Input("eps"));
  ClockGroups.AddClockRelation("input", "output", L_factor, M_factor);

  N_FSD = (unsigned long)(Farrow_coefs_in.size());
  if (NoOfOutputs == 1)
  {
    Buffer.resize(sizeof(DSP::Float) * N_FSD);
    for (ind = 0; ind < N_FSD; ind++)
      ((DSP::Float*)Buffer.data())[ind] = 0.0;

    Execute_ptr = &InputExecute_real;
  }
  else
  {
    Buffer.resize(sizeof(DSP::Complex)*N_FSD);
    for (ind = 0; ind < N_FSD; ind++)
      ((DSP::Complex*)Buffer.data())[ind] = DSP::Complex(0.0, 0.0);

    Execute_ptr = &InputExecute_cplx;
  }

  ///////////////////////////////////
  Farrow_len = (unsigned long)(Farrow_coefs_in[0].size()); // p_order+1
  Farrow_coefs.resize(Farrow_len);
  for (ind = 0; ind < Farrow_len; ind++)
  {
    Farrow_coefs[ind].resize(N_FSD);
  }

  //  // copy with transposition
  //  std::stringstream ss;
  //  ss <<"Farrow coefficients" << std::endl;
  //  ss <<" Farrow_len = " << Farrow_len << ", N_FSD = " << N_FSD << std::endl;
  //  DSP::f::InfoMessage(ss.str().c_str());
  for (ind = 0; ind < Farrow_len; ind++)
    for (ind2 = 0; ind2 < N_FSD; ind2++)
    {
      Farrow_coefs[ind][ind2] = Farrow_coefs_in[ind2][ind];
      //      ss = std::stringstream("");
      //      ss <<" Farrow_coefs[ = " << ind << "][" << ind2 << "]=" << Farrow_coefs[ind][ind2] << std::endl;
      //      DSP::f::InfoMessage(ss.str().c_str());
    }

  // work as if sample and delay are ready
  // if not this variables will be reset in
  // Notification function
  InputSampleReady = true;
  DelayReady = true;

  OutputExecute_ptr = &OutputExecute;
}


DSP::u::Farrow::~Farrow(void)
{
  Buffer.clear();
  Farrow_coefs.clear();
}

#define THIS ((DSP::u::Farrow *)block)
//Execution as an processing block
void DSP::u::Farrow::InputExecute_cplx(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo < THIS->NoOfOutputs)
  { //update buffer
    if (InputNo == 0)
      ((DSP::Complex *)(THIS->Buffer.data()))[THIS->N_FSD-1].re = value;
    else
      ((DSP::Complex *)(THIS->Buffer.data()))[THIS->N_FSD-1].im = value;
    THIS->NoOfInputsProcessed++;

    if (THIS->NoOfInputsProcessed == THIS->NoOfOutputs)
    {
      THIS->InputSampleReady = true;
      THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;
      //DSP::f::InfoMessage(const_cast < char* > (THIS->GetName()), "PROCESS: InputSampleReady == true !!!");
    }
  }
  else
  {
    THIS->epsilon = value;
    THIS->DelayReady = true;
    //DSP::f::InfoMessage(const_cast < char* > (THIS->GetName()), "PROCESS: DelayReady == true !!!");
  }
}

void DSP::u::Farrow::InputExecute_real(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo == 0)
  { //update buffer
    ((DSP::Float *)(THIS->Buffer.data()))[THIS->N_FSD-1] = value;
    THIS->InputSampleReady = true;
    //DSP::f::InfoMessage(const_cast < char* > (THIS->GetName()), "PROCESS: InputSampleReady == true !!!");
  }
  else
  {
    THIS->epsilon = value;
    THIS->DelayReady = true;
    //DSP::f::InfoMessage(const_cast < char* > (THIS->GetName()), "PROCESS: DelayReady == true !!!");
  }
}
#undef THIS

#define THIS ((DSP::u::Farrow *)source)
//Execution as a source block
bool DSP::u::Farrow::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  #ifdef __DEBUG__
    if (clock != THIS->OutputClocks[0])
    { // This is InputClock calling ;-)
      if (clock == THIS->InputClocks[0])
      {
        DSP::log << DSP::LogMode::Error << "DSP::u::Farrow::OutputExecute" << DSP::LogMode::second << "WARNING: InputClock not expected !!!" << endl;
      }
      else
      {
        DSP::log << DSP::LogMode::Error << "DSP::u::Farrow::OutputExecute" << DSP::LogMode::second << "WARNING: Unexpected clock called !!!" << endl;
      }
      return true;
    }
  #endif

  //if (NoOfInputsProcessed == NoOfInputs)
  if ((THIS->InputSampleReady == true) && (THIS->DelayReady == true))
  {
    if (THIS->NoOfOutputs == 1)
    {
      THIS->CalculateOutputSample_real();

      THIS->OutputBlocks[0]->EXECUTE_PTR(
          THIS->OutputBlocks[0],
          THIS->OutputBlocks_InputNo[0],
          THIS->currentOutput.re, source);
    }
    else // (THIS->NoOfOutputs == 2)
    {
      THIS->CalculateOutputSample_cplx();

      THIS->OutputBlocks[0]->EXECUTE_PTR(
          THIS->OutputBlocks[0],
          THIS->OutputBlocks_InputNo[0],
          THIS->currentOutput.re, source);
      THIS->OutputBlocks[1]->EXECUTE_PTR(
          THIS->OutputBlocks[1],
          THIS->OutputBlocks_InputNo[1],
          THIS->currentOutput.im, source);
    }
    return true;
  }

  /*
  if (THIS->InputSampleReady == false)
  {
    DSP::f::ErrorMessage(const_cast < char* > (THIS->GetName()), "InputSampleReady == false !!!");
  }
  if (THIS->DelayReady == false)
  {
    DSP::f::ErrorMessage(const_cast < char* > (THIS->GetName()), "DelayReady == false !!!");
  }
  */
  return false; //wait for input signals
}

#undef THIS

/*! \note This is required only if Input and Output
 *  clocks have the same MasterClock
 *
 * \bug What if input and output clocks are independant.
 *   -# "in" & ("eps", "out")
 *   -# ("in", "eps") & "out"
 *   .
 */
void DSP::u::Farrow::Notify(DSP::Clock_ptr clock) //, bool State)
{
  if (InputClocks[0] == clock)
  { // input sample is expected in this cycle
    // update buffer
    if (NoOfOutputs == 1)
    {
      memmove((uint8_t *)Buffer.data(), (uint8_t *)Buffer.data()+sizeof(DSP::Float), (N_FSD-1) * sizeof(DSP::Float));
//      ((DSP::Float_ptr)Buffer)[N_FSD-1] = 100.0;
    }
    else
    {
      memmove((uint8_t *)Buffer.data(), (uint8_t *)Buffer.data()+sizeof(DSP::Complex), (N_FSD-1) * sizeof(DSP::Complex));
//      ((DSP::Complex_ptr)Buffer)[N_FSD-1].re = 100.0;
//      ((DSP::Complex_ptr)Buffer)[N_FSD-1].im = 200.0;
    }
    InputSampleReady = false;
    //DSP::f::InfoMessage(const_cast < char* > (this->GetName()), "NOTIFY: InputSampleReady == false !!!");
  }
  if (InputClocks[NoOfInputs-1] == clock)
  { // new delay value is expected in this cycle
    DelayReady = false;
    //DSP::f::InfoMessage(const_cast < char* > (this->GetName()), "NOTIFY: DelayReady == false !!!");
  }
}


// Function calculates output sample and stores it in currentOutput
void DSP::u::Farrow::CalculateOutputSample_cplx(void)
{
  unsigned int ind, ind2;
  DSP::Complex temp, tmp2;
  currentOutput = DSP::Complex(0.0, 0.0);

  // implement Farrow structure
  for (ind = 0; ind < Farrow_len; ind++)
  {
    currentOutput.multiply_by(epsilon);

    temp = 0.0;
    for (ind2 = 0; ind2 < N_FSD; ind2++)
    {
      // remember that most recent sample is the last sample
//      temp += ((DSP::Complex *)Buffer)[N_FSD-1-ind2] * Farrow_coefs[ind][ind2];
      tmp2 = ((DSP::Complex *)Buffer.data())[N_FSD-1-ind2];
      tmp2.multiply_by(Farrow_coefs[ind][ind2]);
      temp.add(tmp2);
    }

    currentOutput.add(temp);
  }
}

// Function calculates output sample and stores it in currentOutput
void DSP::u::Farrow::CalculateOutputSample_real(void)
{
  unsigned int ind, ind2;
  DSP::Float temp;
  currentOutput.re = 0.0;

  // implement Farrow structure
  for (ind = 0; ind < Farrow_len; ind++)
  {
    currentOutput.re *= epsilon;

    temp = 0.0;
    for (ind2 = 0; ind2 < N_FSD; ind2++)
      // remember that most recent sample is the last sample
      temp += ((DSP::Float *)Buffer.data())[N_FSD-1-ind2] * Farrow_coefs[ind][ind2];

    currentOutput.re += temp;
  }
}


/**************************************************/
// GardnerSampling - sample selection based on Gardner sampling time recovery algorithm
DSP::u::GardnerSampling::GardnerSampling(
            DSP::Clock_ptr InputClock,
            DSP::Clock_ptr OutputClock,
            //! initial value of the sampling period
            DSP::Float SamplingPeriod_in,
            //! sampling period correction factor
            DSP::Float beta_in,
            //! maximum allowed delay correction
            DSP::Float max_korekta_in,
            //! number of simultaneously processed subchannels
            unsigned int NoOfChannels_in,
            DSPe_GardnerSamplingOptions options_in)
  : DSP::Block()
{
  Init(InputClock, OutputClock, SamplingPeriod_in, beta_in, max_korekta_in, NoOfChannels_in, options_in);
}

DSP::u::GardnerSampling::GardnerSampling(DSP::Float SamplingPeriod_in,
                                         DSP::Float beta_in,
                                         DSP::Float max_korekta_in, // maximum allowed delay correction
                                         unsigned int NoOfChannels_in)
  : DSP::Block()
{
  Init(NULL, NULL, SamplingPeriod_in, beta_in, max_korekta_in, NoOfChannels_in, DSP_GS_none);
}

void DSP::u::GardnerSampling::Init(
            DSP::Clock_ptr InputClock,
            DSP::Clock_ptr OutputClock,
            DSP::Float SamplingPeriod_in,
            DSP::Float beta_in,
            DSP::Float max_korekta_in, // maximum allowed delay correction
            unsigned int NoOfChannels_in,
            DSPe_GardnerSamplingOptions options_in)
{
  unsigned int ind;
  string tekst;

  SetName("GardnerSampling", false);


  NoOfChannels = NoOfChannels_in;
  options = options_in;

  int no_of_outputs = 2*NoOfChannels;
  int activation_signal_index = -1;
  int delay_signal_index = -1;
  if (options & DSP_GS_use_activation_signal)
  {
    activation_signal_index = no_of_outputs;
    no_of_outputs++;
  }
  if (options & DSP_GS_use_delay_output)
  {
    delay_signal_index = no_of_outputs;
    no_of_outputs++;
  }

  SetNoOfInputs(2*NoOfChannels, false);
  DefineInput("in", 0, 1);
  DefineInput("in.re", 0);
  DefineInput("in.im", 1);
  for (ind=0; ind < NoOfChannels; ind++)
  {
    tekst = "in" + to_string(ind+1);
    DefineInput(tekst, ind*2, ind*2+1);
    tekst = "in" + to_string(ind+1) + ".re";
    DefineInput(tekst, ind*2);
    tekst = "in" + to_string(ind+1) + ".im";
    DefineInput(tekst, ind*2+1);
  }

  SetNoOfOutputs(no_of_outputs);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);
  for (ind=0; ind < NoOfChannels; ind++)
  {
    tekst = "out" + to_string(ind+1);
    DefineOutput(tekst, ind*2, ind*2+1);
    tekst = "out" + to_string(ind+1) + ".re";
    DefineOutput(tekst, ind*2);
    tekst = "out" + to_string(ind+1) + ".im";
    DefineOutput(tekst, ind*2+1);
  }

//  ClockGroups.AddInput2Group("input", Input("in"));
//  ClockGroups.AddOutput2Group("output", Output("out"));
  for (ind=0; ind < NoOfChannels; ind++)
  {
    tekst = "in" + to_string(ind+1);
    ClockGroups.AddInput2Group("input", Input(tekst));
    tekst = "out" + to_string(ind+1);
    ClockGroups.AddOutput2Group("output", Output(tekst));
  }

  y0.resize(NoOfChannels);
  y1.resize(NoOfChannels);
  y2.resize(NoOfChannels);

  /* inner state
   * 0 - waiting for y0
   * 1 - waiting for y1
   * 2 - waiting for second sample of y1 for linear interpolation
   * 3 - waiting for y2
   * 4 - waiting for second sample of y2 for linear interpolation
   */
  state_1 = 0;
/*
  state = new int[NoOfChannels]; delay=new DSP::Float[NoOfChannels];
  for (ind = 0; ind < NoOfChannels; ind++)
  {
    state[ind] = 0;
    delay[ind] = 0.0;
  }
  */

  if(options_in & DSP_GS_OQPSK)
  {
    half_SamplingPeriod=SamplingPeriod_in;
    estimated_SamplingPeriod=2*SamplingPeriod_in;
  }
  else
  {
    half_SamplingPeriod=SamplingPeriod_in/2;
    estimated_SamplingPeriod=SamplingPeriod_in;
  }

  delay_1 = -half_SamplingPeriod;

  beta = beta_in;
  beta/=DSP::Float(NoOfChannels);
  beta/=2; //for half_Sampling_period
  max_korekta=max_korekta_in;


//  if (InputClock != NULL)
//    RegisterForNotification(InputClock);
  Execute_ptr = &InputExecute_new;

  use_OutputExecute = false;
  if (OutputClock != NULL)
  {
    use_OutputExecute = true;
    RegisterOutputClock(OutputClock);
    OutputExecute_ptr = &OutputExecute;
  }

  if (options & DSP_GS_activate_output_clock)
  { //activate output clock
    if ((InputClock == NULL) || (OutputClock == NULL))
    {
      DSP::log << DSP::LogMode::Error << "DSP::u::GardnerSampling::Init" << DSP::LogMode::second << "Can't activate output clock: InputClock or OutputClock is NULL" << endl;
      return;
    }
    MasterClockIndex = InputClock->GetMasterClockIndex();
    SignalActivatedClock = OutputClock;
//    OutputClocks[0] = SignalActivatedClock; //2016.03.31 OutputClock => OutputClocks
    for (ind = 0; ind < 2*NoOfChannels; ind++)
      OutputClocks[ind] = SignalActivatedClock;

    ////ProtectOutputClock = true;
    //IsMultirate = true; // multirate block
    //L_factor = -1; M_factor = -1; // asynchronous block -> interpolation factor cannot be specified
    Execute_ptr = &InputExecute_with_options;
  }
  if (options & DSP_GS_use_activation_signal)
  { // additional output: activation signal
//    SignalActivatedClock = OutputClock;
//    for (ind = 0; ind < 2*NoOfChannels; ind++)
//      OutputClocks[ind] = SignalActivatedClock;

    DefineOutput("act", activation_signal_index);
    if (use_OutputExecute == false)
    {
      OutputClocks[activation_signal_index] = InputClock;
      ClockGroups.AddOutput2Group("input", Output("act"));
    }
    else
    {
      OutputClocks[activation_signal_index] = OutputClock;
      ClockGroups.AddOutput2Group("output", Output("act"));
    }

    Execute_ptr = &InputExecute_with_options;
  }
  if (options & DSP_GS_use_delay_output)
  { // additional delay signal
//    SignalActivatedClock = OutputClock;
//    for (ind = 0; ind < 2*NoOfChannels; ind++)
//      OutputClocks[ind] = SignalActivatedClock;

    DefineOutput("offset", delay_signal_index);

    if (use_OutputExecute == false)
    {
      OutputClocks[delay_signal_index] = InputClock;
      ClockGroups.AddOutput2Group("input", Output("offset"));
    }
    else
    {
      OutputClocks[delay_signal_index] = OutputClock;
      ClockGroups.AddOutput2Group("output", Output("offset"));
    }
//    IsMultiClock = true;

    Execute_ptr = &InputExecute_with_options;
  }

  long L_factor, M_factor;
  IsMultirate = GetMultirateFactorsFromClocks(InputClock, SignalActivatedClock, L_factor, M_factor, false);

  ClockGroups.AddClockRelation("input", "output", L_factor, M_factor);
}

DSP::u::GardnerSampling::~GardnerSampling()
{
  y0.clear();
  y1.clear();
  y2.clear();
}

DSP::Float DSP::u::GardnerSampling::GetSamplingPeriod(void)
{
  return estimated_SamplingPeriod;
}

#define THIS ((DSP::u::GardnerSampling *)block)

void DSP::u::GardnerSampling::InputExecute_with_options(INPUT_EXECUTE_ARGS)
{
  // optional processing
  if (THIS->NoOfInputsProcessed == 0)
  { // Process once per InputClock cycle before any input processing is done
    if (THIS->options & DSP_GS_use_delay_output)
    {

      THIS->delay = THIS->delay_1;
      /*
      switch (THIS->state_1)
      {
        case 0:
          delay = 0; // initial value
          break;
        case 3:
        case 4:
          if (delay < 0)
            delay += 2*THIS->half_SamplingPeriod;
          delay -= THIS->half_SamplingPeriod;
          break;
        default:
          break;
      }
      */
      //delay++;
      if (THIS->delay > THIS->half_SamplingPeriod)
        THIS->delay -= 2*THIS->half_SamplingPeriod;

      if (THIS->use_OutputExecute == false)
      {
        THIS->OutputBlocks[THIS->NoOfOutputs - 1]->EXECUTE_PTR(
            THIS->OutputBlocks[THIS->NoOfOutputs - 1],
            THIS->OutputBlocks_InputNo[THIS->NoOfOutputs - 1],
            THIS->delay, block);
      }
    }
  }

  // standard execution procedure
  EXECUTE_INPUT_CALLBACK(InputExecute_new, block, InputNo, value, Caller);

  // optional processing (cont.)
  if (THIS->output_generated)
    if (THIS->options & DSP_GS_activate_output_clock)
    {
      DSP::Clock::AddSignalActivatedClock(
          THIS->MasterClockIndex,
          THIS->SignalActivatedClock, 1); // just one clock cycle
    }

  if (THIS->use_OutputExecute == false)
  {
    if (THIS->NoOfInputsProcessed == 0)
    { // Process once per InputClock cycle after all inputs have been processed
      if (THIS->options & DSP_GS_use_activation_signal)
      {
        if (THIS->options & DSP_GS_use_delay_output)
        {
          THIS->OutputBlocks[THIS->NoOfOutputs - 2]->EXECUTE_PTR(
              THIS->OutputBlocks[THIS->NoOfOutputs - 2],
              THIS->OutputBlocks_InputNo[THIS->NoOfOutputs - 2],
              (THIS->output_generated ? 1.0 : 0.0), block);
        }
        else
        {
          THIS->OutputBlocks[THIS->NoOfOutputs - 1]->EXECUTE_PTR(
              THIS->OutputBlocks[THIS->NoOfOutputs - 1],
              THIS->OutputBlocks_InputNo[THIS->NoOfOutputs - 1],
              (THIS->output_generated ? 1.0 : 0.0), block);
        }
      }
    }
  }
}

void DSP::u::GardnerSampling::InputExecute_new(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Float tmp_korekta;

  THIS->output_generated = false;

  /*! \todo_later Version processing channels separately should be implemented !!!
   */
  switch (THIS->state_1)
  {
    case 0:
      // Algorithm initialization:
      //   y0 <- input
      // //   D = -L/2
      //   D = D + 1;
      //   Stan = 1;

      if ((InputNo % 2) ==0)
        THIS->y0[InputNo >> 1].re = value;
      else
        THIS->y0[InputNo >> 1].im = value;
      THIS->NoOfInputsProcessed++;

      if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
        return;
      THIS->NoOfInputsProcessed=0;

//      THIS->delay_1 = -THIS->half_SamplingPeriod;
      THIS->delay_1++;

      THIS->state_1 = 1;
      return; //wait for y1
      break;

    case 1:
      // Central sample acquisition:
      //   if D < -1.0
      //     ignore input
      //     D = D + 1; % next sample
      //   else
      //     y1 <- input
      //     D = D + 1; % next sample
      //     Stan = 2
      //   endif

      if (THIS->delay_1 < -1.0)
      { //just ignore all input
        THIS->NoOfInputsProcessed++;

        if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
          return;
        THIS->NoOfInputsProcessed = 0;
        THIS->delay_1++;
        return; //still waiting for y1
      }

      if ((InputNo % 2) ==0)
        THIS->y1[InputNo >> 1].re = value;
      else
        THIS->y1[InputNo >> 1].im = value;
      THIS->NoOfInputsProcessed++;

      if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
        return;
      THIS->NoOfInputsProcessed = 0;

      THIS->delay_1++;
      THIS->state_1 = 2;
      return; //wait for next sample to do linear interpolation
      break;

    case 2:
      // Central sample linear interpolation:
      //   y1 = D*y1 + (1-D)*input;
      //   D = D + 1; % next sample
      //   Stan = 3
      //
      //   y1 -> output

      if ((InputNo % 2) ==0)
      {
        THIS->y1[InputNo >> 1].re *= THIS->delay_1;
        THIS->y1[InputNo >> 1].re += (1-THIS->delay_1) * value;
      }
      else
      {
        THIS->y1[InputNo >> 1].im *= THIS->delay_1;
        THIS->y1[InputNo >> 1].im += (1-THIS->delay_1) * value;
      }
      THIS->NoOfInputsProcessed++;

      if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
        return;
      THIS->NoOfInputsProcessed=0;

      THIS->delay_1++;
      THIS->state_1=3;

      // output samples
      if (THIS->use_OutputExecute == false)
      {
        for (unsigned int ind=0; ind < THIS->NoOfChannels; ind++)
        {
          THIS->OutputBlocks[2*ind]->EXECUTE_PTR(
              THIS->OutputBlocks[2*ind],
              THIS->OutputBlocks_InputNo[2*ind],
              THIS->y1[ind].re, block);
          THIS->OutputBlocks[2*ind+1]->EXECUTE_PTR(
              THIS->OutputBlocks[2*ind+1],
              THIS->OutputBlocks_InputNo[2*ind+1],
              THIS->y1[ind].im, block);
        }
      }
      THIS->output_generated = true; // output sample just beeing sent
      return; //wait for y2
      break;

    case 3:
      // Edge sample acquisition:
      //   if D < (L/2 - 1.0) % we're looking for D == L/2
      //     ignore input
      //     D = D + 1; % next sample
      //   else
      //     y2 <- input
      //     D = D + 1; % next sample
      //     Stan = 4
      //   endif


      if (THIS->delay_1 < (THIS->half_SamplingPeriod - 1.0))
      { //just ignore all inputs
        THIS->NoOfInputsProcessed++;

        if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
          return;
        THIS->NoOfInputsProcessed=0;
        THIS->delay_1++;
        return; //still waiting for y2
      }

      if ((InputNo % 2) ==0)
        THIS->y2[InputNo >> 1].re = value;
      else
        THIS->y2[InputNo >> 1].im = value;
      THIS->NoOfInputsProcessed++;

      if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
        return;
      THIS->NoOfInputsProcessed = 0;

      THIS->delay_1++;
      THIS->state_1 = 4;
      return; //wait for next sample to do linear interpolation
      break;

    case 4:
      // Edge sample linear interpolation:
      //   D = D - L/2; (D in <-0.5, 0.5> % preparation for y0 <- y2 (first stage)
      //   y2 = D*y2 + (1-D)*input;
      //
      //   D = D - L/2; (D in <-0.5, 0.5> % preparation for y0 <- y2 (second stage)
      //
      //   update L based on y0, y1, y2
      //
      //   y0 = y2
      //   D = D + 1; % next sample
      //   Stan = 1

      if (THIS->NoOfInputsProcessed == 0)
      {
        THIS->delay_1 -= THIS->half_SamplingPeriod;
      }

      if ((InputNo % 2) ==0)
      {
        THIS->y2[InputNo >> 1].re *= THIS->delay_1;
        THIS->y2[InputNo >> 1].re += (1-THIS->delay_1) * value;
      }
      else
      {
        THIS->y2[InputNo >> 1].im *= THIS->delay_1;
        THIS->y2[InputNo >> 1].im += (1-THIS->delay_1) * value;
      }
      THIS->NoOfInputsProcessed++;

      if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
        return;
      THIS->NoOfInputsProcessed = 0;
      //all values are ready

      THIS->delay_1 -= THIS->half_SamplingPeriod;

      THIS->korekta=0;
      for (unsigned int ind=0; ind < THIS->NoOfChannels; ind++)
      {
        ////evaluate Sampling Period correction
        // epsilon = A* real(conj(y1)*(y0-y2))

        // epsilon = A* real((y1.re-j*y1.im)*(y0-y2))
        // epsilon = A* real((y1.re-j*y1.im)*((y0.re-y2.re)+j*(y0.im-y2.im)))
        // epsilon = A* real(y1.re*((y0.re-y2.re)+j*(y0.im-y2.im))
        //            + (-j*y1.im)*((y0.re-y2.re)+j*(y0.im-y2.im)))
        // epsilon = A* (y1.re*(y0.re-y2.re) + y1.im*(y0.im-y2.im)) //??
    //    korekta+=beta*y1[ind].re*(y0[ind].re-y2[ind].re);
    //    korekta+=beta*y1[ind].im*(y0[ind].im-y2[ind].im);

        tmp_korekta = THIS->beta *
                      THIS->y1[ind].re *
                      (THIS->y0[ind].re -
                       THIS->y2[ind].re);
        tmp_korekta += THIS->beta *
                      THIS->y1[ind].im *
                      (THIS->y0[ind].im -
                       THIS->y2[ind].im);
        if (tmp_korekta > THIS->max_korekta)
          tmp_korekta = THIS->max_korekta;
        if (tmp_korekta < -THIS->max_korekta)
          tmp_korekta = -THIS->max_korekta;
        THIS->korekta += tmp_korekta;
      }

      THIS->delay_1 += THIS->korekta;
      THIS->estimated_SamplingPeriod =
          DSP::Float(0.9) * THIS->estimated_SamplingPeriod +
          DSP::Float(0.1) * (2*(THIS->half_SamplingPeriod -
                    THIS->korekta));
      /*! \todo_later <b>2006.08.05</b> half_SamplingPeriod should also be updated not only estimated_SamplingPeriod
       */


      //y0 <= y2 (just swap memory pointers !!!)
      // DSP::Complex_ptr temp_y;
      // temp_y = THIS->y0;
      // THIS->y0 = THIS->y2;
      // THIS->y2 = temp_y;
      std::swap(THIS->y0, THIS->y2); 

      THIS->delay_1++;
      THIS->state_1 = 1;

      return;
      break;  // and wait for y1
  }

};

#undef THIS

#define THIS ((DSP::u::GardnerSampling *)source)
bool DSP::u::GardnerSampling::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS->output_generated== true)
  {
    if (THIS->options & DSP_GS_use_delay_output)
    {
      THIS->OutputBlocks[THIS->NoOfOutputs - 1]->EXECUTE_PTR(
          THIS->OutputBlocks[THIS->NoOfOutputs - 1],
          THIS->OutputBlocks_InputNo[THIS->NoOfOutputs - 1],
          THIS->delay, source);
    }

    // Process once per InputClock cycle after all inputs have been processed
    if (THIS->options & DSP_GS_use_activation_signal)
    {
      if (THIS->options & DSP_GS_use_delay_output)
      {
        THIS->OutputBlocks[THIS->NoOfOutputs - 2]->EXECUTE_PTR(
            THIS->OutputBlocks[THIS->NoOfOutputs - 2],
            THIS->OutputBlocks_InputNo[THIS->NoOfOutputs - 2],
            (THIS->output_generated ? 1.0 : 0.0), source);
      }
      else
      {
        THIS->OutputBlocks[THIS->NoOfOutputs - 1]->EXECUTE_PTR(
            THIS->OutputBlocks[THIS->NoOfOutputs - 1],
            THIS->OutputBlocks_InputNo[THIS->NoOfOutputs - 1],
            (THIS->output_generated ? 1.0 : 0.0), source);
      }
    }
    for (unsigned int ind=0; ind < THIS->NoOfChannels; ind++)
    {
      THIS->OutputBlocks[2*ind]->EXECUTE_PTR(
          THIS->OutputBlocks[2*ind],
          THIS->OutputBlocks_InputNo[2*ind],
          THIS->y1[ind].re, source);
      THIS->OutputBlocks[2*ind+1]->EXECUTE_PTR(
          THIS->OutputBlocks[2*ind+1],
          THIS->OutputBlocks_InputNo[2*ind+1],
          THIS->y1[ind].im, source);
    }

    THIS->output_generated = false;
    return true;
  }

  return false; //wait for input signals
}
#undef THIS


// PSK encoder - prepares symbols for PSK modulations
/*
 * Supports DSP_PSK_type types:
 *  - BPSK,
 *  - DBPSK,
 *  - QPSK_A, <= QPSK symbols: { 1, -1, j, -j}
 *  - QPSK_B  <= QPSK symbols: { 1+j, -1+j, 1-j, -1-j}
 *  .
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" output symbol (complex valued)
 *    -# "out.re" - real part
 *    -# "out.im" - imag part
 *   - Input:
 *    -# "in" (complex (variants of QPSK) or real (variants of BPSK) valued)
 *    -# "in1" - first input bit
 *    -# "in2" - second input bit
 */
DSP::u::PSKencoder::PSKencoder(DSPe_PSK_type type) : DSP::Block()
{
  SetName("PSK encoder", false);
  tmp_re = 0.0;
  tmp_im = 0.0;

  Type = type;
  switch (Type)
  {
    case DSP_QPSK_A:
      Execute_ptr = &InputExecute_QPSK_A;
      break;
    case DSP_QPSK_B:
      Execute_ptr = &InputExecute_QPSK_B;
      break;

    case DSP_DEBPSK:
    case DSP_DBPSK:
      Execute_ptr = &InputExecute_DBPSK;
      break;

    case DSP_BPSK:
      Execute_ptr = &InputExecute_BPSK;
      break;

    default:
      DSP::log << DSP::LogMode::Error << "DSP::u::PSKencoder::PSKencoder" << DSP::LogMode::second << "Unsupported modulation type, falling back to BPSK" << endl;
      Type = DSP_BPSK;
      Execute_ptr = &InputExecute_BPSK;
      break;
  }

  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  switch (Type)
  {
    case DSP_pi4_QPSK:
    case DSP_DQPSK:
    case DSP_QPSK_A:
    case DSP_QPSK_B:
      SetNoOfInputs(2,false);
      DefineInput("in", 0, 1);
      DefineInput("in1", 0);
      DefineInput("in2", 1);
      break;

    case DSP_BPSK:
    case DSP_DBPSK:
    case DSP_DEBPSK:
    default:
      SetNoOfInputs(1,false);
      DefineInput("in", 0);
      DefineInput("in1", 0);
      break;
  }

  switch (Type)
  {
    case DSP_pi4_QPSK:
      State_re=1;
      State_im=1;
      break;
    case DSP_DQPSK:
    case DSP_QPSK_A:
    case DSP_QPSK_B:
    case DSP_BPSK:
    case DSP_DBPSK:
    case DSP_DEBPSK:
    default:
      State_re=1;
      State_im=0;
      break;
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  symbol_index = 0;
}

DSP::u::PSKencoder::~PSKencoder(void)
{
//  SetNoOfOutputs(0);
}

#define  THIS_ ((DSP::u::PSKencoder *)block)
void DSP::u::PSKencoder::InputExecute_BPSK(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS_->OutputBlocks[0]->EXECUTE_PTR(
    THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
    (value > 0.5)? DSP::Float(+1.0) : DSP::Float(-1.0), block);

  THIS_->OutputBlocks[1]->EXECUTE_PTR(
    THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1], DSP::Float(0.0), block);
};

void DSP::u::PSKencoder::InputExecute_DBPSK(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  // current bit
  THIS_->tmp_re = ((value > 0.5)? +1 : -1);
  THIS_->State_re *= THIS_->tmp_re; // current bit defines phase change
  THIS_->OutputBlocks[0]->EXECUTE_PTR(
    THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
    (THIS_->State_re > 0.0)? DSP::Float(+1.0) : DSP::Float(-1.0), block);
  // store current bit

/*
  THIS_->State_re -= ((value > 0.5)?(1):(0));
  if (THIS_->State_re == -1)
    THIS_->State_re = 1;

  THIS_->OutputBlocks[0]->EXECUTE_PTR(
    THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
    (THIS_->State_re == 0)? DSP::Float(+1.0) : DSP::Float(-1.0), block);
*/
  THIS_->OutputBlocks[1]->EXECUTE_PTR(
    THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1], DSP::Float(0.0), block);
};

void DSP::u::PSKencoder::InputExecute_QPSK_B(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS_->OutputBlocks[0]->EXECUTE_PTR(
      THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
      (value > 0.5)? DSP::Float(+1.0) : DSP::Float(-1.0), block);
  else
    THIS_->OutputBlocks[1]->EXECUTE_PTR(
      THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
      (value > 0.5)? DSP::Float(+1.0) : DSP::Float(-1.0), block);
};

void DSP::u::PSKencoder::InputExecute_QPSK_A(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
  {
    if (value > 0.5)
      THIS_->symbol_index++;
  }
  else
  {
    if (value > 0.5)
      THIS_->symbol_index += 2;
  }

  THIS_->NoOfInputsProcessed++;

  if (THIS_->NoOfInputsProcessed < THIS_->NoOfInputs)
    return;
  THIS_->NoOfInputsProcessed = 0;

  switch (THIS_->symbol_index)
  {
    case 0:
      THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
        DSP::Float(+1.0), block);
      THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
        DSP::Float(0.0), block);
      break;
    case 1:
      THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
        DSP::Float(0.0), block);
      THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
        DSP::Float(+1.0), block);
      break;
    case 2:
      THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
        DSP::Float(0.0), block);
      THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
        DSP::Float(-1.0), block);
      break;
    case 3:
    default:
      THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
        DSP::Float(-1.0), block);
      THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
        DSP::Float(0.0), block);
      break;
  }

  THIS_->symbol_index = 0;
};
#undef  THIS_

// ********************************************************************** //
DSP::u::SymbolMapper::SymbolMapper(
    DSPe_Modulation_type type,
    const unsigned int &bits_per_symbol_in,
    const DSP::Float &constellation_phase_offset) : DSP::Block()
{
  SetName("Symbol Mapper", false);

  bits_per_symbol = bits_per_symbol_in;
  input_bits.resize(bits_per_symbol, 0);

  current_constellation.resize(0);
  is_output_real = false;
  Type = type;
  getConstellation(current_constellation,Type,constellation_phase_offset,bits_per_symbol,is_output_real);

  if (is_output_real == true) {
    SetNoOfOutputs(1);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }
  else {
    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }

  SetNoOfInputs(bits_per_symbol_in,false);
  vector<unsigned int> inds;
  for (auto ind = 0U; ind < NoOfInputs; ind++) {
    inds.push_back(ind);
    DefineInput("in"+to_string(ind+1), ind);
  }
  DefineInput("in", inds);

  Execute_ptr = &InputExecute_bits;
  IsMultiClock=false; // single output clock

  ClockGroups.AddInput2Group("all", Input("in"));
  ClockGroups.AddOutput2Group("all", Output("out"));
}

//void DSP::u::SymbolMapper::Notify(DSP::Clock_ptr clock) {
//
//}

unsigned int getConstellation(
                DSP::Complex_vector &constellation,
                DSPe_Modulation_type type,
                const DSP::Float &constellation_phase_offset,
                const unsigned int &bits_per_symbol,
                bool &is_real) {
  is_real = true;
  unsigned int M = static_cast<unsigned int>(round(pow(2,bits_per_symbol)));

  switch (type) {
    case DSP_MT_PSK: {
        constellation.resize(M);
//        for (unsigned int n=0; n < M; n++) {
//          constellation[n].re = static_cast<DSP::Float>(cos(constellation_phase_offset+(DSP_M_PIx2*n)/M));
//          constellation[n].im = static_cast<DSP::Float>(sin(constellation_phase_offset+(DSP_M_PIx2*n)/M));
//        }
        // Gray codding (start from LSB)
        uint16_t mask = 0x0001;
        uint16_t n = 0;
        for (unsigned int ind=0; ind < M; ind++) {
          constellation[ind].re = static_cast<DSP::Float>(cos(constellation_phase_offset+(DSP_M_PIx2*n)/DSP::Float(M)));
          constellation[ind].im = static_cast<DSP::Float>(sin(constellation_phase_offset+(DSP_M_PIx2*n)/DSP::Float(M)));

//          stringstream ss;
//          ss << "constellation[" << ind << "]={" << setprecision(2) << constellation[ind].re << "," << constellation[ind].im << "}; n=" << n;
//          DSP::f::InfoMessage(ss.str());

          n ^= mask;
          //mask <<= 1u;
          mask = uint16_t(mask << 1);
          if (mask == M) {
            mask = 0x0001;
          }
        }
      }
      //! \TODO is_real ustala� zbiorczo na podstawie gwiazdek konstelacji
      if ((M == 2) && (constellation_phase_offset == 0.0)) {
        is_real = true;
      }
      else{
        is_real = false;
      }
      break;

    case DSP_MT_ASK: {
      constellation.resize(M);
        for (unsigned int n=0; n < M; n++) {
          constellation[n].re = static_cast<DSP::Float>(n)/static_cast<DSP::Float>(M-1);
          constellation[n].im = 0;
        }
        is_real = true;
      }
      break;

    default:
      DSP::log << DSP::LogMode::Error << "getConstellation" << DSP::LogMode::second << "Unsupported modulation type" << endl;
      break;
  }

  return M;
}

bool DSP::u::SymbolMapper::isOutputReal(void) {
  return is_output_real;
}
unsigned int DSP::u::SymbolMapper::getBitsPerSymbol(void) {
  return bits_per_symbol;
}

DSP::u::SymbolMapper::~SymbolMapper(void)
{
//  SetNoOfOutputs(0);
}

#define  THIS_ ((DSP::u::SymbolMapper *)block)

void DSP::u::SymbolMapper::InputExecute_bits(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  //! \TODO configurable threshold
  if (value >= 0.5) {
    THIS_->input_bits[InputNo] = 1;
  }
  else {
    THIS_->input_bits[InputNo] = 0;
  }

  THIS_->NoOfInputsProcessed++;

  if (THIS_->NoOfInputsProcessed == THIS_->NoOfInputs)
  {
    //! used for current symbol index evaluation
    unsigned int symbol_index = 0;
    //! \TODO should this be first bit as LSB or differently?
    //! in1 is the LSB
    for (int n=THIS_->bits_per_symbol-1; n >= 0; n--) {
      symbol_index *= 2;
      symbol_index += THIS_->input_bits[n]; // MSB last
    }

    // output sample
    THIS_->OutputBlocks[0]->EXECUTE_PTR(
      THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
      THIS_->current_constellation[symbol_index].re, block);
    if (THIS_->NoOfOutputs == 2) {
      THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
        THIS_->current_constellation[symbol_index].im, block);
    }

    THIS_->NoOfInputsProcessed = THIS_->InitialNoOfInputsProcessed;
  }
}
#undef  THIS_

// ********************************************************************** //
DSP::u::Serial2Parallel::Serial2Parallel(const DSP::Clock_ptr &InputClock,
                                           const unsigned int &NoOfParallelOutputs,
                                           const unsigned int &NoOfLinesPerInput,
                                           const vector<DSP::Float> &first_output_vector)
  : DSP::Block(), DSP::Source()
{
  string tekst;

  SetName("Serial2Parallel", false);

  Execute_ptr = &InputExecute;

  // ++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // Inputs definitions
  vector <unsigned int> inds;
  SetNoOfInputs(NoOfLinesPerInput,false);
  DefineInput("in.re", 0);
  if (NoOfInputs >= 2) {
    DefineInput("in.im", 1);
  }
  inds.clear();
  for (auto ind = 0U; ind < NoOfInputs; ind++) {
    inds.push_back(ind);
    tekst = "in" + to_string(ind+1);
    DefineInput(tekst, ind);
  }
  DefineInput("in", inds);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // Outputs definitions
  SetNoOfOutputs(NoOfParallelOutputs * NoOfInputs);
  inds.clear();
  for (auto ind = 0U; ind < NoOfOutputs; ind++) {
    inds.push_back(ind);
  }
  DefineOutput("out", inds);
  for (auto ind = 0U; ind < NoOfParallelOutputs; ind++) {
    inds.clear();
    for (auto ind2 = 0U; ind2 < NoOfInputs; ind2++) {
      inds.push_back(ind*NoOfInputs+ind2);

      // # "out1[1]", "out1[2]", ... - first, second, ... line of output first output sample ...
      tekst = "out" + to_string(ind+1) + "[" + to_string(ind2+1) + "]";
      DefineOutput(tekst, ind*NoOfInputs+ind2);
    }
    // # "out1.re", "out2.re", ... - real part (first line) of output samples
    tekst = "out" + to_string(ind+1) + ".re";
    DefineOutput(tekst, ind*NoOfInputs+0);
    // # "out1.im", "out2.im", ... - imag part (second line) of output samples (available only if NoOfInputs > 1)
    if (NoOfInputs >= 2) {
      tekst = "out" + to_string(ind+1) + ".im";
      DefineOutput(tekst, ind*NoOfInputs+1);
    }
    // # "out1", "out2", ...  first,second, ... output sample (each with NoOfInputs lines)
    tekst = "out" + to_string(ind+1);
    DefineOutput(tekst, inds);
  }

  OutputExecute_ptr = &OutputExecute;
  IsMultiClock=false; // single output clock
  DSP::Clock_ptr OutputClock = DSP::Clock::GetClock(InputClock, 1, NoOfParallelOutputs);
  RegisterOutputClock(OutputClock);
  long L_factor, M_factor;
  IsMultirate = GetMultirateFactorsFromClocks(InputClock, OutputClock, L_factor, M_factor, false);
  //IsMultirate = true;  L_factor = -1; M_factor = -1;
  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));
  ClockGroups.AddClockRelation("input", "output", L_factor, M_factor);

  no_of_parallel_outputs = NoOfParallelOutputs;
  inputs.reserve(NoOfInputs*(NoOfParallelOutputs+1)); // with one row of reserve
  inputs.resize(NoOfInputs*NoOfParallelOutputs);

  if (NoOfParallelOutputs == 1) {
    output_ready = false;
  }
  else {
    output_ready = true;
  }
  if (first_output_vector.size() != NoOfOutputs) {
    outputs.resize(NoOfOutputs, 0.0);
    if (first_output_vector.size() > 0) {
      DSP::log << DSP::LogMode::Error << "DSP::u::Serial2Parallel" << DSP::LogMode::second << "Wrong size of first_output_vector" << endl;
    }
  }
  else {
    outputs = first_output_vector;
  }

  current_cycle_no = 0;
}

DSP::u::Serial2Parallel::~Serial2Parallel() {
  inputs.clear();
}

#define THIS_ ((DSP::u::Serial2Parallel *)block)
void DSP::u::Serial2Parallel::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  UNUSED_DEBUG_ARGUMENT(InputNo);

  THIS_->inputs[THIS_->current_cycle_no*THIS_->NoOfInputs + InputNo] = value;
  THIS_->NoOfInputsProcessed++;

  if (THIS_->NoOfInputsProcessed < THIS_->NoOfInputs)
    return;
  THIS_->NoOfInputsProcessed = THIS_->InitialNoOfInputsProcessed;

  THIS_->current_cycle_no++;
  if (THIS_->current_cycle_no == THIS_->no_of_parallel_outputs) {
    // make copy of collected inputs
    THIS_->outputs = THIS_->inputs;
    THIS_->current_cycle_no = 0;
    THIS_->output_ready = true;
  }
}

#undef  THIS_

#define THIS_ ((DSP::u::Serial2Parallel *)source)
bool DSP::u::Serial2Parallel::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS_->output_ready == true) {
    // output samples
    for (auto n=0U; n<THIS_->NoOfOutputs; n++) {
      THIS_->OutputBlocks[n]->EXECUTE_PTR(
        THIS_->OutputBlocks[n], THIS_->OutputBlocks_InputNo[n], THIS_->outputs[n], source);
    }
    THIS_->output_ready = false;
    return true;
  }
  // wait for all input lines
  return false;
}
#undef  THIS_


// ********************************************************************** //
DSP::u::Parallel2Serial::Parallel2Serial(const DSP::Clock_ptr &InputClock,
                                           const unsigned int &NoOfParallelInputs,
                                           const unsigned int &NoOfLinesPerInput,
                                           const bool &reversed_order)
  : DSP::Block(), DSP::Source()
{
  string tekst;

  SetName("Parallel2Serial", false);

  Execute_ptr = &InputExecute;

  // ++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // Outputs definitions
  vector <unsigned int> inds;
  SetNoOfOutputs(NoOfLinesPerInput);
  DefineOutput("out.re", 0);
  if (NoOfOutputs >= 2) {
    DefineOutput("out.im", 1);
  }
  inds.clear();
  for (auto ind = 0U; ind < NoOfOutputs; ind++) {
    inds.push_back(ind);
    tekst = "out" + to_string(ind+1);
    DefineOutput(tekst, ind);
  }
  DefineOutput("out", inds);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // Inputs definitions
  SetNoOfInputs(NoOfParallelInputs * NoOfOutputs,false);
  if (reversed_order == false) {
    // normal order of output samples
    inds.clear();
    for (auto ind = 0U; ind < NoOfInputs; ind++) {
      inds.push_back(ind);
    }
    DefineInput("in", inds);
    for (auto ind = 0U; ind < NoOfParallelInputs; ind++) {
      inds.clear();
      for (auto ind2 = 0U; ind2 < NoOfOutputs; ind2++) {
        inds.push_back(ind*NoOfOutputs+ind2);

        // # "in1[1]", "in1[2]", ... - first, second, ... line of input of first input sample ...
        tekst = "in" + to_string(ind+1) + "[" + to_string(ind2+1) + "]";
        DefineInput(tekst, ind*NoOfOutputs+ind2);
      }
      // # "in1.re", "in2.re", ... - real part (first line) of input samples
      tekst = "in" + to_string(ind+1) + ".re";
      DefineInput(tekst, ind*NoOfOutputs+0);
      // "in1.im", "in2.im", ... - imag part (second line) of input samples (available only if NoOfLinesPerInput > 1)
      if (NoOfOutputs >= 2) {
        tekst = "in" + to_string(ind+1) + ".im";
        DefineInput(tekst, ind*NoOfOutputs+1);
      }
      // "in1", "in2", ...  first,second, ... input sample (each with NoOfLinesPerInput lines)
      tekst = "in" + to_string(ind+1);
      DefineInput(tekst, inds);
    }
  }
  else {
    // reversed order of output samples
    inds.clear();
    //for (int ind = NoOfParallelInputs-1; ind >= 0; ind--) {
    for (auto ind = 0U; ind < NoOfParallelInputs; ind++) {
      //NoOfLinesPerInput == NoOfOutputs
      for (auto ind2 = 0U; ind2 < NoOfOutputs; ind2++) {
        inds.push_back((NoOfParallelInputs-1-ind)*NoOfOutputs+ind2);
      }
    }
    DefineInput("in", inds);
    for (auto ind = 0U; ind < NoOfParallelInputs; ind++) {
      inds.clear();
      for (auto ind2 = 0U; ind2 < NoOfOutputs; ind2++) {
        inds.push_back((NoOfParallelInputs-1-ind)*NoOfOutputs+ind2);

        // # "in1[1]", "in1[2]", ... - first, second, ... line of input of first input sample ...
        tekst = "in" + to_string(ind+1) + "[" + to_string(ind2+1) + "]";
        DefineInput(tekst, (NoOfParallelInputs-1-ind)*NoOfOutputs+ind2);
      }
      // # "in1.re", "in2.re", ... - real part (first line) of input samples
      tekst = "in" + to_string(ind+1) + ".re";
      DefineInput(tekst, (NoOfParallelInputs-1-ind)*NoOfOutputs+0);
      // "in1.im", "in2.im", ... - imag part (second line) of input samples (available only if NoOfLinesPerInput > 1)
      if (NoOfOutputs >= 2) {
        tekst = "in" + to_string(ind+1) + ".im";
        DefineInput(tekst, (NoOfParallelInputs-1-ind)*NoOfOutputs+1);
      }
      // "in1", "in2", ...  first,second, ... input sample (each with NoOfLinesPerInput lines)
      tekst = "in" + to_string(ind+1);
      DefineInput(tekst, inds);
    }
  }

  OutputExecute_ptr = &OutputExecute;
  IsMultiClock=false; // single output clock
  DSP::Clock_ptr OutputClock = DSP::Clock::GetClock(InputClock, NoOfParallelInputs, 1);
  RegisterOutputClock(OutputClock);
  long L_factor, M_factor;
  IsMultirate = GetMultirateFactorsFromClocks(InputClock, OutputClock, L_factor, M_factor, false);
  //IsMultirate = true;  L_factor = -1; M_factor = -1;
  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));
  ClockGroups.AddClockRelation("input", "output", L_factor, M_factor);

  no_of_parallel_inputs = NoOfParallelInputs;
  inputs.reserve(NoOfInputs);
  inputs.resize(NoOfInputs);
  ready = false;
  no_of_first_output_sample_lines_ready = 0;
  current_out = 0;
}

DSP::u::Parallel2Serial::~Parallel2Serial() {
  inputs.clear();
}

#define THIS_ ((DSP::u::Parallel2Serial *)block)
void DSP::u::Parallel2Serial::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  UNUSED_DEBUG_ARGUMENT(InputNo);

  THIS_->inputs[InputNo] = value;
  THIS_->NoOfInputsProcessed++;

  if (InputNo < THIS_->NoOfOutputs) {
    THIS_->no_of_first_output_sample_lines_ready++;
    if (THIS_->no_of_first_output_sample_lines_ready == THIS_->NoOfOutputs){
      THIS_->no_of_first_output_sample_lines_ready = 0;
      THIS_->ready = true;
    }
  }
  if (THIS_->NoOfInputsProcessed < THIS_->NoOfInputs)
    return;
  THIS_->NoOfInputsProcessed = THIS_->InitialNoOfInputsProcessed;
}
#undef  THIS_

#define THIS_ ((DSP::u::Parallel2Serial *)source)
bool DSP::u::Parallel2Serial::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS_->ready == true) {
    // output samples
    for (auto n=0U; n<THIS_->NoOfOutputs; n++) {
      THIS_->OutputBlocks[n]->EXECUTE_PTR(
        THIS_->OutputBlocks[n], THIS_->OutputBlocks_InputNo[n],
                                THIS_->inputs[(THIS_->current_out)*(THIS_->NoOfOutputs)+n], source);
    }
    THIS_->current_out++;
    THIS_->current_out %= THIS_->no_of_parallel_inputs;
    if(THIS_->current_out == 0) {
      THIS_->ready = false;
    }
    return true;
  }
  // wait for all input lines for the first input (first output sample)
  return false;
}
#undef  THIS_


// ********************************************************************** //
DSP::u::SymbolDemapper::SymbolDemapper( DSPe_Modulation_type type,
                                          const unsigned int &bits_per_symbol_in,
                                          const DSP::Float &constellation_phase_offset) : DSP::Block()
{
  SetName("Symbol Demapper", false);

  bits_per_symbol = bits_per_symbol_in;
  current_constellation.resize(0);
  is_input_real = false;
  Type = type;

  getConstellation(current_constellation,Type,constellation_phase_offset,bits_per_symbol,is_input_real);
  Execute_ptr = &InputExecute_constellation;

  SetNoOfOutputs(bits_per_symbol_in);
  vector<unsigned int> inds;
  for (auto ind=0U; ind<NoOfOutputs; ind++) {
    inds.push_back(ind);
    DefineOutput("out"+to_string(ind+1), ind);
  }
  DefineOutput("out", inds);

  if (is_input_real == true) {
    SetNoOfInputs(1,false);
    DefineInput("in", 0);
    DefineInput("in.re", 0);
  }
  else {
    SetNoOfInputs(2,false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
  }

  IsMultiClock=false; // single output clock
  //IsMultirate = true;  L_factor = -1; M_factor = -1;
  ClockGroups.AddInput2Group("all", Input("in"));
  ClockGroups.AddOutput2Group("all", Output("out"));
}

bool DSP::u::SymbolDemapper::isInputReal(void) {
  return is_input_real;
}
unsigned int DSP::u::SymbolDemapper::getBitsPerSymbol(void) {
  return bits_per_symbol;
}

DSP::u::SymbolDemapper::~SymbolDemapper(void)
{
//  SetNoOfOutputs(0);
}

#define  THIS_ ((DSP::u::SymbolDemapper *)block)

void DSP::u::SymbolDemapper::InputExecute_constellation(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  UNUSED_DEBUG_ARGUMENT(InputNo);

  if (InputNo==0)
    THIS_->input.re = value;
  else
    THIS_->input.im = value;
  THIS_->NoOfInputsProcessed++;

  if (THIS_->NoOfInputsProcessed < THIS_->NoOfInputs)
    return;
  THIS_->NoOfInputsProcessed = 0;

  //! determine symbol index based on nearest neighbor rule
  //! \TODO add optimized versions for particular modulations
  DSP::Float d_re, d_im;
  d_re = THIS_->current_constellation[0].re-THIS_->input.re;
  d_im = THIS_->current_constellation[0].im-THIS_->input.im;
  unsigned int symbol_index=0;
  DSP::Float min_squared_dist = d_re*d_re + d_im*d_im;

  DSP::Float squared_dist;
  for (unsigned int n=1; n<THIS_->current_constellation.size(); n++) {
    d_re = THIS_->current_constellation[n].re-THIS_->input.re;
    d_im = THIS_->current_constellation[n].im-THIS_->input.im;
    squared_dist = d_re*d_re + d_im*d_im;

    if (squared_dist < min_squared_dist) {
      min_squared_dist = squared_dist;
      symbol_index = n;
    }
  }

//  stringstream ss;
//  ss << setprecision(2) << "input={" << THIS_->input.re << "," << THIS_->input.im << "}\n";
//  ss << "min_squared_dist=" << min_squared_dist << ",symbol_index=" << symbol_index <<"\n";
//  ss << "bits:";
  unsigned int mask = 0x01;
  // extract LSB from symbol_index
  unsigned int bit = (symbol_index & mask);
  for (auto ind = 0U; ind < THIS_->NoOfOutputs; ind ++){
    // output sample
    THIS_->OutputBlocks[ind]->EXECUTE_PTR(
      THIS_->OutputBlocks[ind], THIS_->OutputBlocks_InputNo[ind], static_cast<DSP::Float>(bit), block);

//    ss << "(" << symbol_index << ")=" << bit << ",";
    // get the next bit
    symbol_index -= bit;
    symbol_index >>= 1;
    bit = (symbol_index & mask);
  }

//  DSP::f::InfoMessage(ss.str());
}
#undef  THIS_


// ********************************************************************** //


// PSK decoder - decodes PSK modulations symbols
/*
 * Supports DSP_PSK_type types:
 *  - BPSK,
 *  - QPSK_A, <= QPSK symbols: { 1, -1, j, -j}
 *  - QPSK_B  <= QPSK symbols: { 1+j, -1+j, 1-j, -1-j}
 *  .
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" (complex (variants of QPSK) or real (variants of BPSK) valued)
 *    -# "out0" - first output bit
 *    -# "out1" - second output bit
 *   - Input:
 *    -# "in" output symbol (complex valued)
 *    -# "in.re" - real part
 *    -# "in.im" - imag part
 */
DSP::u::PSKdecoder::PSKdecoder(DSPe_PSK_type type) : DSP::Block()
{
  SetName("PSK decoder", false);
  Type = type;

  switch (Type)
  {
    case DSP_pi4_QPSK:
    case DSP_DQPSK:
    case DSP_QPSK_A:
    case DSP_QPSK_B:
      SetNoOfOutputs(2);
      DefineOutput("out", 0, 1);
      DefineOutput("out0", 0);
      DefineOutput("out1", 1);
      break;

    case DSP_BPSK:
    case DSP_DBPSK:
    case DSP_DEBPSK:
    default:
      SetNoOfOutputs(1);
      DefineOutput("out", 0);
      DefineOutput("out0", 0);
      break;
  }

  SetNoOfInputs(2, false);
  DefineInput("in", 0, 1);
  DefineInput("in.re", 0);
  DefineInput("in.im", 1);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  switch (Type)
  {
    case DSP_pi4_QPSK:
      State_re=1; State_im=1;
      f_State_re=1.0; f_State_im=1.0;
      break;
    case DSP_DQPSK:
    case DSP_QPSK_A:
    case DSP_QPSK_B:
    case DSP_BPSK:
    case DSP_DBPSK:
    case DSP_DEBPSK:
    default:
      State_re=1; State_im=0;
      f_State_re=1.0; f_State_im=0.0;
      break;
  }

  switch (Type)
  {
    case DSP_QPSK_A:
      Execute_ptr = &InputExecute_QPSK_A;
      break;
    case DSP_QPSK_B:
      Execute_ptr = &InputExecute_QPSK_B;
      break;

    case DSP_DEBPSK:
      Execute_ptr = &InputExecute_DEBPSK;
      break;
    case DSP_DBPSK:
      Execute_ptr = &InputExecute_DBPSK;
      break;

    case DSP_BPSK:
      Execute_ptr = &InputExecute_BPSK;
      break;

    default:
      DSP::log << DSP::LogMode::Error << "DSP::u::PSKdecoder::PSKdecoder" << DSP::LogMode::second << "Unsupported modulation type, falling back to BPSK" << endl;
      Execute_ptr = &InputExecute_BPSK;
      break;
  }

  f_tmp_re = 0.0; f_tmp_im = 0.0;
  i_tmp_re = 0; i_tmp_im = 0;
}

DSP::u::PSKdecoder::~PSKdecoder(void)
{
//  SetNoOfOutputs(0);
}

#define  THIS_ ((DSP::u::PSKdecoder *)block)
void DSP::u::PSKdecoder::InputExecute_BPSK(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo == 0)
    THIS_->OutputBlocks[0]->EXECUTE_PTR(
      THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
      (value > 0.0)? DSP::Float(1.0) : DSP::Float(0.0), block);
   // Ignore InputNo == 1 <= imaginary part
};

void DSP::u::PSKdecoder::InputExecute_DEBPSK(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo == 0)
  {
    // current bit
    THIS_->i_tmp_re = ((value > 0.0)? +1 : -1);
    THIS_->OutputBlocks[0]->EXECUTE_PTR(
      THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
      ((THIS_->State_re*THIS_->i_tmp_re) > 0.0) ? DSP::Float(1.0) : DSP::Float(0.0), block);
    // store current bit
    THIS_->State_re = THIS_->i_tmp_re;
  }
  // Ignore InputNo == 1 <= imaginary part
};

void DSP::u::PSKdecoder::InputExecute_DBPSK(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  // current sample
  if (InputNo == 0)
    THIS_->f_tmp_re = value;
  else
    THIS_->f_tmp_im = value;
  THIS_->NoOfInputsProcessed++;

  if (THIS_->NoOfInputsProcessed < THIS_->NoOfInputs)
    return;
  THIS_->NoOfInputsProcessed = 0;

  // compute current decoded symbol
  // x[n].*conj(x[n-1])=(a+j*b)(c-jd)=ac+bd + j(bc-ad)
  THIS_->f_State_re = (THIS_->f_tmp_re*THIS_->f_State_re) +
                      (THIS_->f_tmp_im*THIS_->f_State_im);

  THIS_->OutputBlocks[0]->EXECUTE_PTR(
      THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
      (THIS_->f_State_re > 0.0) ? DSP::Float(1.0) : DSP::Float(0.0), block);

  // store current symbol bit
  THIS_->f_State_re = THIS_->f_tmp_re;
  THIS_->f_State_im = THIS_->f_tmp_im;
};

void DSP::u::PSKdecoder::InputExecute_QPSK_B(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS_->OutputBlocks[0]->EXECUTE_PTR(
      THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
      (value > 0.0)? DSP::Float(1.0) : DSP::Float(0.0), block);
  else
    THIS_->OutputBlocks[1]->EXECUTE_PTR(
      THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
      (value > 0.0)? DSP::Float(1.0) : DSP::Float(0.0), block);
};

void DSP::u::PSKdecoder::InputExecute_QPSK_A(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS_->f_tmp_re = value;
  else
    THIS_->f_tmp_im = value;
  THIS_->NoOfInputsProcessed++;

  if (THIS_->NoOfInputsProcessed < THIS_->NoOfInputs)
    return;
  THIS_->NoOfInputsProcessed = 0;


  if (fabs(THIS_->f_tmp_re) >= fabs(THIS_->f_tmp_im))
    if (THIS_->f_tmp_re >= 0)
    {
      THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
        DSP::Float(0.0), block);
      THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
        DSP::Float(0.0), block);
    }
    else
    {
      THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
        DSP::Float(1.0), block);
      THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
        DSP::Float(1.0), block);
    }
  else
    if (THIS_->f_tmp_im >= 0)
    {
      THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
        DSP::Float(1.0), block);
      THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
        DSP::Float(0.0), block);
    }
    else
    {
      THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
        DSP::Float(0.0), block);
      THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1], THIS_->OutputBlocks_InputNo[1],
        DSP::Float(1.0), block);
    }
};
#undef  THIS_


DSP::u::FFT::FFT(unsigned int K_in, bool AreInputsComplex)
  : DSP::Block()
{
  string temp;
  unsigned int ind;

  K = K_in;

  SetName("FFT", false);

  if (AreInputsComplex == true)
  {
    SetNoOfInputs(2*K, true); // allow for constant inputs

    for (ind=0; ind<K; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, ind*2, ind*2+1);

      temp = "in" + to_string(ind+1) + ".re";
      DefineInput(temp, ind*2);
      temp = "in" + to_string(ind+1) + ".im";
      DefineInput(temp, ind*2+1);
    }
  }
  else
  {
    SetNoOfInputs(K, true); // allow for constant inputs
    for (ind=0; ind<K; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, ind);
      temp = "in" + to_string(ind+1) + ".re";
      DefineInput(temp, ind);
    }
  }

  SetNoOfOutputs(2*K);
  for (ind=0; ind<K; ind++)
  {
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind*2, ind*2+1);

    temp = "out" + to_string(ind+1) + ".re";
    DefineOutput(temp, ind*2);
    temp = "out" + to_string(ind+1) + ".im";
    DefineOutput(temp, ind*2+1);
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  // init DFT block
  dft.resize(K);
  // create buffers for use with dft
  input_buffer.resize(K, DSP::Complex(0.0,0.0));
  output_buffer.resize(K);
 
  if (AreInputsComplex == true)
    Execute_ptr = &InputExecute_cplx;
  else
    Execute_ptr = &InputExecute_real;
}

DSP::u::FFT::~FFT(void)
{
}

void DSP::u::FFT::RecalculateInitials(void)
{
  for (int ind=0; ind<(int)NoOfInputs; ind++)
  {
    if (IsConstantInput[ind] == true)
    {
      if (ind % 2 == 0)
        input_buffer[ind/2].re = ConstantInputValues[ind];
      else
        input_buffer[(ind-1)/2].im = ConstantInputValues[ind];
    }
  }
}

#define THIS ((DSP::u::FFT *)block)
void DSP::u::FFT::InputExecute_cplx(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  //In general we should check whether each input
  //is executed only once per cycle
  if (InputNo < THIS->NoOfInputs)
  {
    if ((InputNo % 2) == 0)
      THIS->input_buffer[InputNo/2].re = value;
    else
      THIS->input_buffer[(InputNo-1)/2].im = value;
  }
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    THIS->dft.DFT((uint32_t)THIS->K, THIS->input_buffer, THIS->output_buffer);

    for (int ind = 0; ind < (int)(THIS->K); ind++)
    {
      THIS->OutputBlocks[ind*2]->EXECUTE_PTR(
        THIS->OutputBlocks[ind*2], THIS->OutputBlocks_InputNo[ind*2],
        THIS->output_buffer[ind].re, block);
      THIS->OutputBlocks[ind*2+1]->EXECUTE_PTR(
        THIS->OutputBlocks[ind*2+1], THIS->OutputBlocks_InputNo[ind*2+1],
        THIS->output_buffer[ind].im, block);
    }

    THIS->NoOfInputsProcessed=THIS->InitialNoOfInputsProcessed;
  }
}

void DSP::u::FFT::InputExecute_real(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  //In general we should check whether each input
  //is executed only once per cycle
  if (InputNo < THIS->NoOfInputs)
  {
    THIS->input_buffer[InputNo].re = value;
  }
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    THIS->dft.DFT(THIS->K, THIS->input_buffer, THIS->output_buffer);

    for (int ind = 0; ind < (int)(THIS->K); ind++)
    {
      THIS->OutputBlocks[ind*2]->EXECUTE_PTR(
        THIS->OutputBlocks[ind*2], THIS->OutputBlocks_InputNo[ind*2],
        THIS->output_buffer[ind].re, block);
      THIS->OutputBlocks[ind*2+1]->EXECUTE_PTR(
        THIS->OutputBlocks[ind*2+1], THIS->OutputBlocks_InputNo[ind*2+1],
        THIS->output_buffer[ind].im, block);
    }

    THIS->NoOfInputsProcessed=THIS->InitialNoOfInputsProcessed;
  }
}
#undef THIS

/**************************************************/
// Timing Error Detector
#define  THIS  ((DSP::u::TimingErrorDetector *)block)

DSP::u::TimingErrorDetector::TimingErrorDetector(int N, bool InputIsComplex)
  : DSP::Block()
{
  SetName("TimingErrorDetector", false);
  output_real_n1 = 0.0;
  output_real = 0.0;

  N_symb = N;

  if (InputIsComplex == false)
  {
    SetNoOfInputs(1, false);
    DefineInput("in", 0);
    DefineInput("in.re", 0);
    SetNoOfOutputs(1);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);

    ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
    ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

    Real_Buffer.resize(N_symb+1, 0.0);
    Cplx_Buffer.resize(0);

    n0 = N_symb;
    n1 = (N_symb - (N_symb % 2))/2; // N_symb = 4 or 5 ==> n1 = 2

    if ((N_symb % 2) == 0)
      Execute_ptr = &InputExecute_real_even;
    else
      Execute_ptr = &InputExecute_real_odd;
  }
  else
  {
    SetNoOfInputs(2, false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);

    ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
    ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

    Real_Buffer.resize(0);
    Cplx_Buffer.resize(N_symb+1, DSP::Complex(0.0,0.0));

    n0 = N_symb;
    n1 = (N_symb - (N_symb % 2))/2; // N_symb = 4 or 5 ==> n1 = 2

    if ((N_symb % 2) == 0)
      Execute_ptr = &InputExecute_cplx_even;
    else
      Execute_ptr = &InputExecute_cplx_odd;
  }
}

DSP::u::TimingErrorDetector::~TimingErrorDetector()
{
}

void DSP::u::TimingErrorDetector::InputExecute_real_even(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->Real_Buffer[THIS->n0] = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  // e[n] = (y[n] - y[n-2])*conj(y[n-1])
  THIS->output_real = THIS->Real_Buffer[THIS->n0];
  THIS->n0 = (THIS->n0+1) % (THIS->N_symb+1);
  THIS->output_real -= THIS->Real_Buffer[THIS->n0];
  THIS->output_real *= THIS->Real_Buffer[THIS->n1];
  THIS->n1 = (THIS->n1+1) % (THIS->N_symb+1);

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->output_real, block);
};

void DSP::u::TimingErrorDetector::InputExecute_real_odd(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->Real_Buffer[THIS->n0] = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  // e[n] = (y[n] - y[n-2])*conj(y[n-1])
  THIS->output_real = THIS->Real_Buffer[THIS->n0];
  THIS->n0 = (THIS->n0+1) % (THIS->N_symb+1);
  THIS->output_real -= THIS->Real_Buffer[THIS->n0];

  // average of two central samples
  THIS->output_real_n1 = THIS->Real_Buffer[THIS->n1];
  THIS->n1 = (THIS->n1+1) % (THIS->N_symb+1);
  THIS->output_real_n1 += THIS->Real_Buffer[THIS->n1];

  THIS->output_real *= (THIS->output_real_n1) / 2;

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->output_real, block);
};

void DSP::u::TimingErrorDetector::InputExecute_cplx_even(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->Cplx_Buffer[THIS->n0].re = value;
  else
    THIS->Cplx_Buffer[THIS->n0].im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  // e[n] = (y[n] - y[n-2])*conj(y[n-1])
  THIS->output_cplx = THIS->Cplx_Buffer[THIS->n0];
  THIS->n0 = (THIS->n0+1) % (THIS->N_symb+1);
  THIS->output_cplx.sub(THIS->Cplx_Buffer[THIS->n0]);
  THIS->output_cplx.multiply_by_conj(THIS->Cplx_Buffer[THIS->n1]);
  THIS->n1 = (THIS->n1+1) % (THIS->N_symb+1);

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->output_cplx.re, block);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      THIS->output_cplx.im, block);
};


void DSP::u::TimingErrorDetector::InputExecute_cplx_odd(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->Cplx_Buffer[THIS->n0].re = value;
  else
    THIS->Cplx_Buffer[THIS->n0].im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  // e[n] = (y[n] - y[n-2])*conj(y[n-1])
  THIS->output_cplx = THIS->Cplx_Buffer[THIS->n0];
  THIS->n0 = (THIS->n0+1) % (THIS->N_symb+1);
  THIS->output_cplx.sub(THIS->Cplx_Buffer[THIS->n0]);

  // average of two central samples
  THIS->output_cplx_n1 = THIS->Cplx_Buffer[THIS->n1];
  THIS->n1 = (THIS->n1+1) % (THIS->N_symb+1);
  THIS->output_cplx_n1 += THIS->Cplx_Buffer[THIS->n1];

//  THIS->output_cplx.multiply_by_conj(0.5 * THIS->Cplx_Buffer[THIS->n1]);
  THIS->output_cplx.multiply_by_conj(0.5 * THIS->output_cplx_n1); // 2010.03.04 fixed
//  THIS->n1 = (THIS->n1+1) % (THIS->N_symb+1);  // 2010.03.04 removed this line

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->output_cplx.re, block);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      THIS->output_cplx.im, block);
};

#undef THIS


