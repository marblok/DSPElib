/*! \file DSP_modules2.h
 * This is DSP engine components and sources definition module header file.
 *
 * \author Marek Blok
 */
#ifndef DSPmodules2H
#define DSPmodules2H

//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------
#include <DSP_types.h>
#include <DSP_misc.h>
#include <DSP_Fourier.h>
#include <DSP_modules.h>


/**************************************************/
//! AGC - automatic gain control
/*!
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (complex valued)
 *   -# "out.re" - real component\n
 *      "out.im" - imag component
 *  - Input:
 *   -# "in" (complex valued)
 *   -# "in.re" - real component\n
 *      "in.im" - imag component
 */
class DSPu_AGC : public DSP::Block
{
  private:
    DSP_float alfa;
    DSP_float signal_power;
    DSP_float out_power;

    DSP_complex input;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    DSPu_AGC(DSP_float alfa_in,  //! forgeting factor
            DSP_float init_signal_power=1.0,  //! initial value of the signal power
            DSP_float output_signal_power=1.0  //! output signal power
          );
    ~DSPu_AGC(void);

    //! returns current estimated signal power
    DSP_float GetPower(void);
};


// ***************************************************** //
//! Signal to noise ratio estimation for BPSK modulation
/*!
 *  Inputs and Outputs names:
 *   - Output:
 *    -# "snr" - estimated SNR value
 *   - Input:
 *    -# "in" - complex valued signal
 *    -# "in.re" - real component
 *    -# "in.im" - imag component
 */
class DSPu_BPSK_SNR_estimator : public DSP::Block
{
  private:
    //! number of symbols used for current value estimation
    int SegmentSize;

    //! buffer for real input values of the size SegmentSize
    DSP_float_ptr RealBuffer;
    //! current free slot RealBuffer index
    int current_ind;

    //! Current values of real and imag symbol components
    DSP_float Input_Real, Input_Imag;

    //! Current value of the estimated SNR
    DSP_float EstimatedSNR;

    //! Processing block execution function
    /*! This function provides input parameters
     *  reception and processing
     */
    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    //! DSPu_BPSK_SNR_estimator constructor
    /*! segment_size - number of symbols used for current value estimation,
     */
    DSPu_BPSK_SNR_estimator(int segment_size);

    ~DSPu_BPSK_SNR_estimator(void);
};

namespace DSP {
  namespace f {
    //! SNR estimation for PSK modulation (BPSK & QPSK) based on complex symbol samples
    /*! 1) buffer_size is in symbols
    *  2) buffer contains 2*buffer_size floating point values (real & imag)
    *
    * \warning in this version Buffer is overwrited
    *
    */
    void PSK_SNR_estimator(int buffer_size, DSP_float_ptr buffer,
                            DSP_float &BPSK_SNR, DSP_float &QPSK_SNR);
    //! SNR estimation for PSK modulation (BPSK & QPSK) based on complex symbol samples
    /*! 1) buffer_size is in symbols
    *  2) buffer contains 2*buffer_size floating point values (real & imag)
    *
    * \warning in this version preserves input buffer
    *
    */
    void PSK_SNR_estimator2(int buffer_size, DSP_float_ptr buffer,
                            DSP_float &BPSK_SNR, DSP_float &QPSK_SNR);
  }
}

/**************************************************/
//! DynamicCompressor - changes input signal dynamic
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" - real or complex output signal
 *   -# "out.re" - real part of output signal
 *   -# "out.im" - imaginary part of output signal (if it exists)
 *  - Input:
 *   -# "in" - real or complex input signal
 *   -# "in.re" - real part of input signal
 *   -# "in.im" - imaginary part of input signal (if it exists)
 */
class DSPu_DynamicCompressor : public DSP::Block
{
  private:
    DSP_float_ptr SamplesBuffer;
    DSP_float_ptr PowerBuffer; //! Sample Energy per PowerBufferSize
    unsigned int index; //! index of the current free entry in SamplesBuffer
    unsigned int power_index; //! index of the current free entry in PowerBuffer
    //output_index == index because the SampleBuffer is shorter then PowerBuffer
    //unsigned int output_index; //! index of the output sample

    //! ??? BufferSize_in * (number of sample components)
    /*! !!! it seems that OutputDelay * (number of sample components) would be enough
     */
    unsigned int SamplesBufferSize;
    unsigned int PowerBufferSize; //! BufferSize_in

    DSP_complex temp_value;
    DSP_float factor_0, alfa;
    DSP_float CurrentPower; //! Current input signal power evaluated based on values in PowerBuffer

    static void InputExecute_no_delay_real(INPUT_EXECUTE_ARGS);
    static void InputExecute_no_delay_complex(INPUT_EXECUTE_ARGS);
    static void InputExecute_real(INPUT_EXECUTE_ARGS);
    static void InputExecute_complex(INPUT_EXECUTE_ARGS);
  public:
    /*! DynamicCompressor constructor
     *  BufferSize - size (in samples) of the buffer used to estimate input signal power.
     *  a0 - compression factor, dynamic [dB] at output  will be a0 times smaller than at input
     *  Po_dB - power reference point [dB]. If input power equals Po_dB then
     *    output power will also be Po_dB.
     *  IsInputComplex - indicates whether the input is real or complex.
     *  OutputDelay - delay (in samples) of the output signal.
     *   Default delay (OutputDelay = -1) is floor((BufferSize_in-1)/2).
     *   Any value in range from 0 to BufferSize_in-1 can be set.
     */
    DSPu_DynamicCompressor(int BufferSize_in, DSP_float a0, DSP_float Po_dB = 0.0,
                           bool IsInputComplex = false, int OutputDelay_in = -1);
    ~DSPu_DynamicCompressor(void);
};


//! Farrow structure - gives output sample when output clock is activated
/*! \note Input & Output clocks can have different clocks.
 *
 * \warning because Input & Output clocks are independent to some extend then
 *   time hazards can occur.
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real or complex) evaluated sample
 *   -# "out.re" (real component)\n
 *      "out.im" (imag component if exists)
 *  - Input:
 *   -# "in" (real or complex) input signal to be delayed
 *   -# "eps" (real) net delay to be implemented
 *   .
 *
 * "eps" input can have the same clock as "in" or "out".
 * If "eps" has clock common with "out" notification
 * function must be used for both clocks. Separate for "in" and "eps".
 *
 * \note For current implementation "eps" must have the same clock as "out" which can be the same as the clock for "in"
 */
class DSPu_Farrow  : public DSP::Block, public DSP::Source
{
  private:
    //! length of implemented FSD filter
    unsigned long N_FSD;
    //! input signal buffer
    /*! for real input table od DSP_float /n
     *  for complex input table od DSP_complex
     *
     * \note that most recent sample is the last sample in buffer
     */
    uint8_t *Buffer;

    //! Order of the Farrow structure + 1 (length of Farrow polynomials)
    unsigned long Farrow_len;
    //! Table of Farrow structure coefficients
    /*! Number of vectors == Farrow_order.
     *  Each vector length == N_FSD.
     *
     *  This is transposition of the constructor input table Farrow_coefs_in.
     */
    DSP_float_ptr *Farrow_coefs;

    //! variable storing currently evaluated output sample
    DSP_complex currentOutput;
    //! current net delay value
    DSP_float epsilon;


    //! Function calculates output sample and stores it in currentOutput
    void CalculateOutputSample_real(void);
    //! Function calculates output sample and stores it in currentOutput
    void CalculateOutputSample_cplx(void);

    /*! false if Input Sample is expected in current clock cycle.
     *  Output sample generation must wait until input sample will be ready.
     */
    bool InputSampleReady;
    /*! false if Delay Sample is expected in current clock cycle.
     *  Output sample generation must wait until delay value will be ready.
     */
    bool DelayReady;


//  protected:
//    void SetBlockInputClock(int InputNo, DSP::Clock_ptr InputClock);

    static void InputExecute_real(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx(INPUT_EXECUTE_ARGS);
    void Notify(DSP::Clock_ptr clock); //, bool State);
    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);
  public:

    ////! InputClock == NULL <- auto detection at connection time
    ///*! \param N_FSD_in - FSD filter length
    // *  \param order_in - Farrow structure order;
    // *   number of polynomial coefficients equals (order_in + 1) !!!.
    // *  \param Farrow_coefs_in - tablica wektor�w wsp�czynnik�w struktury Farrow'a.
    // *    N_FSD_in vectors of the length order_in with polynomial coefficients.
    // *    One polynomial for each FSD filter impulse response sample. /n
    // *    h[n] = p[n][0]*eps^order_in + p[n][1]*eps^(order_in-1) + ... +p[n][order_in] /n
    // *    n = 0, 1, ..., N_FSD-1
    // *
    // * \param IsComplex true if complex input is used
    // * \param InputClock clock for input signal
    // * \param OutputClock clock for output signal
    // */
    //DSPu_Farrow(bool IsComplex, unsigned int N_FSD_in, unsigned int order_in, DSP_float_ptr *Farrow_coefs_in,
    //            DSP::Clock_ptr InputClock, DSP::Clock_ptr OutputClock);

    //! InputClock == NULL <- auto detection at connection time
    /*! \param Farrow_coefs_in - wektor wektor�w wsp�czynnik�w struktury Farrow'a.
     *    - liczba wektor�w wsp�czynnik�w (Farrow_coefs_in.size() == N_FSD)
     *      odpowiada d�ugo�ci odpowiedzi impulsowej filtru FD implementowanego przez struktur�,
     *    - d�ugo�� poszczeg�lnych wektor�w (Farrow_coefs_in[n].size() == p_Farrow+1) jest o jeden 
     *      wi�ksza od rz�du wsp�czynnik�w wielomian�w aproksymuj�cych poszczeg�lne wsp�czynniki filtru FD
     *
     *    One polynomial for each FSD filter impulse response sample. /n
     *    h[n] = p[n][0]*eps^order_in + p[n][1]*eps^(order_in-1) + ... +p[n][order_in] /n
     *    n = 0, 1, ..., N_FSD-1
     *
     * \param IsComplex true if complex input is used
     * \param InputClock clock for input signal
     * \param OutputClock clock for output signal
     */
    DSPu_Farrow(const bool &IsComplex, const vector<DSP_float_vector> &Farrow_coefs_in,
      const DSP::Clock_ptr &InputClock, const DSP::Clock_ptr &OutputClock);
    
    ~DSPu_Farrow(void);
};
/**************************************************/

enum DSPe_GardnerSamplingOptions {
    //! all additional options off
    DSP_GS_none = 0,
    //! block must activate output clock each time output sample is generated
    DSP_GS_activate_output_clock = 1,
    //! block must output clock activation signal (signal working with input clock)
    DSP_GS_use_activation_signal = 2,
    //! block must output input samples delay offsets (signal working with input clock)
    DSP_GS_use_delay_output = 4,
    //! OQPSK signal mode: signal is sampled according to N_symb (which should be half of OQPSK symbol period) but error signal is calculated for 2*N_symb
    DSP_GS_OQPSK = 8,
    };
inline DSPe_GardnerSamplingOptions operator|(DSPe_GardnerSamplingOptions __a,
                                             DSPe_GardnerSamplingOptions __b)
  { return DSPe_GardnerSamplingOptions(static_cast<int>(__a) | static_cast<int>(__b)); }

/**************************************************/
//! GardnerSampling - sample selection based on Gadner sampling time recovery algorithm
/*!
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out1", "out2", ... (complex valued)
 *   -# "out1.re", "out2.re", ... - real component\n
 *      "out1.im", "out2.im", ... - imag component
 *   -# "out" == "out1" (complex valued)
 *   -# "out.re" == "out1.re" - real component\n
 *      "out.im" == "out1.im" - imag component
 *   -# "act" - [OPTIONAL] clock activation signal (works with input clock if OutputClock == NULL)
 *   -# "offset" - [OPTIONAL] delay offset signal (works with input clock if OutputClock == NULL)
 *  - Input:
 *   -# "in1", "in2", ... (complex valued)
 *   -# "in1.re", "in2.re", ... - real component\n
 *      "in1.im", "in2.im", ... - imag component
 *   -# "in" == "in1" (complex valued)
 *   -# "in.re" == "in1.re" - real component\n
 *      "in.im" == "in1.im" - imag component
 *
 *
 * \todo_later use DSP_float_ptr y0, y1, y2 instead of DSP_complex_ptr y0, y1, y2
 * to increase performance
 *
 * \todo_later output can be generated earlier when the interpolated values of y1 are available
 *
 * \todo <b>2006.08.03</b>
 *   -# separate processing function for single input signal
 *   .
 */
class DSPu_GardnerSampling : public DSP::Block, public DSP::Source, public DSP::Clock_trigger
{
  private:
    DSP_float delay;
    DSP_float delay_1; //(all channels simultaneously)
//    DSP_float_ptr delay;
    //! half of the sampling period
    DSP_float half_SamplingPeriod;
    DSP_float estimated_SamplingPeriod;
    DSP_float beta, korekta; //, tmp_korekta;
    //! maximum allowed delay correction
    DSP_float max_korekta;
    //! inner state (all channels simultaneously)
    int state_1;
//    int *state; //! inner state

    unsigned int NoOfChannels;
    DSP_complex_ptr y0, y1, y2;

    //! construction options
    DSPe_GardnerSamplingOptions options;
    //! true if in last execution of InputExecute output samples have been generated
    bool output_generated;
    //! true if block should work as source
    bool use_OutputExecute;

    void Init(
            DSP::Clock_ptr InputClock,
            DSP::Clock_ptr OutputClock,
            //! initial value of the sampling period
            DSP_float SamplingPeriod_in,
            //! sampling period correction factor
            DSP_float beta_in,
            //! maximum allowed delay correction
            DSP_float max_korekta_in,
            //! number of simultaneously processed subchannels
            unsigned int NoOfChannels_in,
            DSPe_GardnerSamplingOptions options_in);

    static void InputExecute_with_options(INPUT_EXECUTE_ARGS);
    // static void InputExecute(DSP::Block *block, int InputNo, DSP_float value, DSP::Component *Caller);
    static void InputExecute_new(INPUT_EXECUTE_ARGS);
    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

  public:
    DSP::Clock_trigger_ptr Convert2ClockTrigger(void)
    { return GetPointer2ClockTrigger(); };

    //! OBSOLETE constructor: backward compatibility.
    /*! - ParentClock = NULL,
     *  - OutputClock = NULL
     *  - options = DSP_GS_none
     */
    DSPu_GardnerSampling(DSP_float SamplingPeriod_in,  //! initial value of the sampling period
            DSP_float beta_in, //! sampling period correction factor
            DSP_float max_korekta_in, //! maximum allowed delay correction
            unsigned int NoOfChannels_in=1.0  //! number of simultaneously processed subchannels
          );
    //! Gardner sampling block constructor
    /*! Options (can be combination):
     *  - DSP_GS_none - all additional options off
     *  - DSP_GS_activate_output_clock - activates output clock each time output sample is generated
     *  - DSP_GS_use_activation_signal - outputs clock activation signal for each cycle of the input clock if OutputClock == NULL otherwise output clock is used
     *  - DSP_GS_use_delay_output - outputs input samples delay offsets for each cycle of the input clock if OutputClock == NULL otherwise output clock is used
     *  .
     */
    DSPu_GardnerSampling(
            DSP::Clock_ptr InputClock,
            DSP::Clock_ptr OutputClock,
            //! initial value of the sampling period
            DSP_float SamplingPeriod_in,
            //! sampling period correction factor
            DSP_float beta_in,
            //! maximum allowed delay correction
            DSP_float max_korekta_in,
            //! number of simultaneously processed subchannels
            unsigned int NoOfChannels_in=1.0,
            DSPe_GardnerSamplingOptions options = DSP_GS_none
          );
    ~DSPu_GardnerSampling(void);

    //! returns current estimated sampling period
    DSP_float GetSamplingPeriod(void);
};



//! PSK encoder - prepares symbols for PSK modulations
/*!
 *  \note Will become obsolete block, use DSPu_SymbolMapper instead if possible.
 *
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
 *    -# "in0" - first input bit
 *    -# "in1" - second input bit
 */
class DSPu_PSKencoder : public DSP::Block
{
  private:
    DSPe_PSK_type Type;
    int State_re, State_im;
    int tmp_re, tmp_im;
    //! used in InputExecute_XXXX for current symbol index evaluation
    int symbol_index;

    static void InputExecute_BPSK(INPUT_EXECUTE_ARGS);
    static void InputExecute_DBPSK(INPUT_EXECUTE_ARGS);
    static void InputExecute_QPSK_A(INPUT_EXECUTE_ARGS);
    static void InputExecute_QPSK_B(INPUT_EXECUTE_ARGS);
  public:
    DSPu_PSKencoder(DSPe_PSK_type type=DSP_BPSK);
    ~DSPu_PSKencoder(void);
};

//! Calculates constellations used in DSPu_SymbolMapper and DSPu_SymbolDemapper
/*!
 *  constellation_phase_offset - phase offset of constellation symbols [rad] (ignored in case of ASK)
 */
unsigned int getConstellation(DSP_complex_vector &constellation, DSPe_Modulation_type type, const DSP_float &constellation_phase_offset, const unsigned int &bits_per_symbol_in, bool &is_real);

//! Serial to parallel converter
/*! Collects NoOfParallelOutputs samples from each input and outputs them in parallel (at the same clock cycle).
 *
 *  Each input can be multivalued. For example for NoOfLinesPerInput == 1 the inputs are real valued while for NoOfLinesPerInput == 2 the inputs are complex valued.
 *  Output clock is NoOfParallelOutputs times slower than the input clock.
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" parallel output all samples
 *    -# "out1", "out2", ...  first,second, ... output sample (each with NoOfLinesPerInput lines)
 *    -# "out1.re", "out2.re", ... - real part (first line) of output samples
 *    -# "out1.im", "out2.im", ... - imag part (second line) of output samples (available only if NoOfLinesPerInput > 1)
 *    -# "out1[1]", "out1[2]", ... - first, second, ... line of output of first output sample ...
 *   - Input:
 *    -# "in" - input sample (all lines)
 *    -# "in.re" - real part of input sample
 *    -# "in.im" - imag part of input sample (available only if NoOfLinesPerInput > 1)
 *    -# "in1", "in2", ...  first,second, ... input sample (for all NoOfLinesPerInput lines)
 *
 * \note Output clock cycle is equivalent to first input clock cycle.
 *
 * If NoOfParallelOutputs == 1 (input - output: one in one):
 *   - output can be generated in the same cycle as input comes, we just need to wait,
 *   - this case can be treated as improper input parameter
 *   .
 * else:
 *   - at first output clock cycle not all inputs are ready, thus vector of zeros needs to be output,
 *   - at next clock cycles first input clock cycle starts and input values can be overwritten (a copy of previous state is needed)
 */
class DSPu_Serial2Parallel : public DSP::Block, public DSP::Source
{
  private:
    static void InputExecute(INPUT_EXECUTE_ARGS);
    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

    int no_of_parallel_outputs; //! number of inputs cycles before the output is generated
    vector<DSP_float> inputs;  //! vector for input samples
    vector<DSP_float> outputs;  //! vector for input samples
    bool output_ready; //! set true when all inputs are collected in inputs vector
    int current_cycle_no; // 0, 1, ..., no_of_inputs_per_output-1
  public:

    DSPu_Serial2Parallel(const DSP::Clock_ptr &InputClock,
        const unsigned int &NoOfParallelOutputs,
        const unsigned int &NoOfLinesPerInput=1,
        const vector<DSP_float> &first_output_vector={});
    ~DSPu_Serial2Parallel(void);
};

//! Parallel to serial converter
/*! Reads NoOfParallelInputs input samples in parallel and outputs them sample by sample in serial (for consecutive clock cycles).
 *
 *  Each input can be multivalued. For example for NoOfLinesPerInput == 1 the inputs are real valued while for NoOfParallelInputs == 2 the inputs are complex valued.
 *  Output clock is NoOfParallelInputs times faster than the input clock.
 *
 *  If reversed_order == true then last input is output first.
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" - output sample (all lines)
 *    -# "out.re" - real part of output sample
 *    -# "out.im" - imag part of output sample (available only if NoOfLinesPerInput > 1)
 *    -# "out1", "oout2", ...  first,second, ... input sample (for all NoOfLinesPerInput lines)
 *   - Input:
 *    -# "in" parallel input all samples
 *    -# "in1", "in2", ...  first,second, ... input sample (each with NoOfLinesPerInput lines)
 *    -# "in1.re", "in2.re", ... - real part (first line) of input samples
 *    -# "in1.im", "in2.im", ... - imag part (second line) of input samples (available only if NoOfLinesPerInput > 1)
 *    -# "in1[1]", "in1[2]", ... - first, second, ... line of input of first input sample ...
 *
 * \note First output clock cycle is equivalent to input clock cycle.
 */
class DSPu_Parallel2Serial : public DSP::Block, public DSP::Source
{
  private:
    static void InputExecute(INPUT_EXECUTE_ARGS);
    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

    int no_of_parallel_inputs; //! number of output cycles per input cycle
    vector<DSP_float> inputs;  //! vector for input samples for current clock cycle
    bool ready; //! set true when all lines of the first input are collected in inputs vector and output can be generated for the first output clock cycle
    unsigned int no_of_first_output_sample_lines_ready; // allows for detection when the first output sample can be generated
    int current_out; //! index of currently generated output
  public:

    DSPu_Parallel2Serial(const DSP::Clock_ptr &InputClock,
        const unsigned int &NoOfParallelInputs,
        const unsigned int &NoOfLinesPerInput=1,
        const bool &reversed_order = false);
    ~DSPu_Parallel2Serial(void);
};

//! DSPu_SymbolMapper - prepares symbols for digital modulations based on binary input vector
/*!
 * Supports DSPe_Modulation_type types:
 *  - DSP_MT_PSK,
 *  - DSP_MT_APSK,
 *  - \TODO DSP_MT_DPSK,
 *  - \TODO DSP_MT_QAM,
 *  - \TODO DSP_MT_FSK,
 *  - \TODO DSP_MT_OQPSK,
 *  .
 *
 * \TODO Implement differential encoding (DSP_MT_DPSK) & decoding
 * \note First output symbol is the constellation symbol with index 0.
 * \TODO Implement constellation phase offset (e.g. pi/4-QPSK)
 *
 * \note DSP_MT_PSK | DSP_MT_diff: y[0] = s[0]; y[n] = y[n-1] * s[n];
 *
 * bits_per_symbol - number of bits per symbol.
 *  - bits_per_symbol = -1 (default) - default number of bits per symbol (based on modulation type type).
 * constellation_phase_offset - phase rotation of constellation symbols [rad]
 *  - constellation_phase_offset = DSP_M_PIx1/4 for pi/4-QPSK
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" output symbol (real or complex valued)
 *    -# "out.re" - real part
 *    -# "out.im" - imag part (only for complex valued modulations)
 *   - Input:
 *    -# "in" - vector of binary inputs: ("0" - in < 0.5, "1" - in >= 0.5)
 *    -# "in1","in2", ... - separate binary inputs; "in1" is the LSB of the symbol index in the constellation.
 */
class DSPu_SymbolMapper : public DSP::Block
{
  private:
    DSPe_Modulation_type Type;

    bool is_output_real;
    unsigned int bits_per_symbol;
    DSP_complex_vector current_constellation;

    friend unsigned int getConstellation(DSP_complex_vector &constellation, DSPe_Modulation_type type, const DSP_float &constellation_phase_offset, const unsigned int &bits_per_symbol_in, bool &is_real);
    vector <unsigned char> input_bits;

    static void InputExecute_bits(INPUT_EXECUTE_ARGS);
  public:

    //! Returns true if block's output is real valued (single output line)
    bool isOutputReal(void);
    //! Returns number of input bits per output sample
    unsigned int getBitsPerSymbol(void);

    //! Mapper selection based on modulation type and number of bits_per_symbol
    DSPu_SymbolMapper(DSPe_Modulation_type type=DSP_MT_PSK,
                      const unsigned int &bits_per_symbol=-1,  //! bits_per_symbol based on given constellation
                      const DSP_float &constellation_phase_offset=0.0);
    /*! Assuming
     * bits_per_symbol = ceil(log2(constellation.size());
     * M = 2^bits_per_symbol;
     *
     * Each bits_per_symbol input bits (first as LSB) index symbol from constellation vector
     *
     * \TODO implement this variant
     */
    DSPu_SymbolMapper(const DSP_complex_vector&constellation);
    ~DSPu_SymbolMapper(void);
};

//! DSPu_SymbolDemapper - based on closest constellation point to current input symbol outputs corresponding bit stream
/*! \note This is counterpart of DSPu_SymbolMapper.
 *
 * Supports DSPe_Modulation_type types:
 *  - PSK,
 *  - APSK,
 *  .
 *
 * \TODO Implement differential decoding
 *
 * bits_per_symbol - number of bits per symbol.
 *  - bits_per_symbol = -1 (default) - default number of bits per symbol (based on modulation type type).
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" - all output bits
 *    -# "out1", "out2", ... - outputs for separate bits: "out1" is the LSB
 *   - Input:
 *    -# "in" - input symbols (real or complex valued (depending on modulation type)
 *    -# "in.re" - real part
 *    -# "in.im" - imag part (only for complex valued modulations)
*/
class DSPu_SymbolDemapper : public DSP::Block
{
  private:
    DSPe_Modulation_type Type;
    //! used for current symbol index evaluation
    DSP_complex input; // input sample

    bool is_input_real;
    unsigned int bits_per_symbol;
    DSP_complex_vector current_constellation;

    friend unsigned int getConstellation(DSP_complex_vector &constellation, DSPe_Modulation_type type, const DSP_float &constellation_phase_offset, const unsigned int &bits_per_symbol_in, bool &is_real);

    static void InputExecute_constellation(INPUT_EXECUTE_ARGS);

  public:
    //! Returns true if block expects input samples to be real valued (single input line)
    bool isInputReal(void);
    //! Returns number of input bits per output sample
    unsigned int getBitsPerSymbol(void);

    //! Demapper selection based on modulation type and number of bits_per_symbol
    DSPu_SymbolDemapper(DSPe_Modulation_type type=DSP_MT_PSK,
                        const unsigned int &bits_per_symbol=-1,  //! bits_per_symbol based on given constellation
                        const DSP_float &constellation_phase_offset=0.0);

    //! Demapper based on given constellation
    /*! Assuming
     * bits_per_symbol = ceil(log2(constellation.size());
     * M = 2^bits_per_symbol;
     *
     * Each bits_per_symbol input bits (first as LSB) index symbol from constellation vector
     */
    DSPu_SymbolDemapper(const DSP_complex_vector &constellation);
    ~DSPu_SymbolDemapper(void);
};



//! PSK decoder - decodes PSK modulations symbols
/*!
 *  \note Will become obsolete block, use DSPu_SymbolDemapper instead if possible.
 *
 * Supports DSP_PSK_type types:
 *  - BPSK,
 *  - DBPSK,
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
class DSPu_PSKdecoder : public DSP::Block
{
  private:
    DSPe_PSK_type Type;
    int State_re, State_im;
    DSP_float f_State_re, f_State_im;

    DSP_float f_tmp_re, f_tmp_im;
    int i_tmp_re, i_tmp_im;

    static void InputExecute_BPSK(INPUT_EXECUTE_ARGS);
    static void InputExecute_DEBPSK(INPUT_EXECUTE_ARGS);
    static void InputExecute_DBPSK(INPUT_EXECUTE_ARGS);
    static void InputExecute_QPSK_A(INPUT_EXECUTE_ARGS);
    static void InputExecute_QPSK_B(INPUT_EXECUTE_ARGS);
  public:
    DSPu_PSKdecoder(DSPe_PSK_type type=DSP_BPSK);
    ~DSPu_PSKdecoder(void);
};


/**************************************************/
//! Computes FFT of sequence made form inputs for the current instant
/*! Inputs and Outputs names:
 *  - Output (real or complex):
 *   -# "out_all" - all output lines
 *   -# "out" - first output (real or complex)
 *   -# "out.re", /n
 *      "out.im"
 *   -# "out1", "out2", ...  - consecutive outputs (real or complex)
 *   -# "out1.re", "out2.re", .../n
 *      "out1.im", "out2.im", ...
 *  - Input (always complex):
 *   -# "in_all" - all input lines
 *   -# "in" - first input (complex)
 *   -# "in.re", /n
 *      "in.im"
 *   -# "in1", "in2", ...  - consecutive inputs (complex)
 *   -# "in1.re", "in2.re", .../n
 *      "in1.im", "in2.im", ...
 */
class DSPu_FFT : public DSP::Block
{
  private:
    unsigned int K;
    DSP_Fourier dft;
    //DSP_complex *input_buffer, *output_buffer;
    DSP_complex_vector input_buffer, output_buffer;

    static void InputExecute_cplx(INPUT_EXECUTE_ARGS);
    static void InputExecute_real(INPUT_EXECUTE_ARGS);

    void RecalculateInitials(void);

  public:
    //! FFT block
        /*! K - FFT length,
         */
    DSPu_FFT(unsigned int K_in, bool AreInputsComplex = true);
    ~DSPu_FFT(void);
};

/**************************************************/
//! Timing Error Detector - symbol timing error detector
/*! e[n] = (y[n] - y[n-2])*conj(y[n-1])
 *
 * Inputs and Outputs names:
 *  - Output:
 *  - Input:
 *   -# "in" real or complex valued
 *   -# "in.re" - real component\n
 *      "in.im" - imag component
 *   .
 *  .
 */
class DSPu_TimingErrorDetector : public DSP::Block
{
  private:
    // index to symbols: previous, current and between
    int n0, n1; // n2;
    //DSP_float_ptr   Real_Buffer;
    DSP_float_vector   Real_Buffer;
    //DSP_complex_ptr Cplx_Buffer;
    DSP_complex_vector Cplx_Buffer;
    int N_symb;

    DSP_float output_real;
    DSP_float output_real_n1;

    DSP_complex output_cplx;
    DSP_complex output_cplx_n1;

    static void InputExecute_real_even(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx_even(INPUT_EXECUTE_ARGS);
    static void InputExecute_real_odd(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx_odd(INPUT_EXECUTE_ARGS);

  public:
    //! Timing Error Detector contructor.
    /*! \param N - symbol length in [Sa],
     *  \param InputIsComplex - true if input signal is complex
     */
    DSPu_TimingErrorDetector(int N, bool InputIsComplex = false);
    ~DSPu_TimingErrorDetector(void);
};


#endif
