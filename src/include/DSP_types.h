/*! \file DSP_types.h
 * This is DSP engine types definition header file.
 *
 * \author Marek Blok
 */
#ifndef DSP_types_H
#define DSP_types_H

#ifndef _USE_MATH_DEFINES
  #define  _USE_MATH_DEFINES
#endif
#include <cmath>
#include <limits.h>
#include <vector>
#include <string>
#include <functional>

using namespace std;
//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------

#define MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS 100
//#define DSP_FILE_NAME_LEN     1024
#define DSP_FILE_READING_NOT_STARTED UINT_MAX

//---------------------------------------------------------------------------
#define UNUSED_ARGUMENT(x) (void)x
#ifdef __DEBUG__
  #define UNUSED_DEBUG_ARGUMENT(x) (void)x
  #define UNUSED_RELEASE_ARGUMENT(x) {}
#else
  #define UNUSED_DEBUG_ARGUMENT(x) {}
  #define UNUSED_RELEASE_ARGUMENT(x) (void)x
#endif

//---------------------------------------------------------------------------
//! main DSPE library namespace
namespace DSP {
  class output;
  //typedef DSP_output * DSP_output_ptr;
  class input;
  //typedef DSP_input *  DSP_input_ptr;
}

namespace DSP {
  class File;
  typedef File * File_ptr;

  class Clock_trigger;
  typedef Clock_trigger * Clock_trigger_ptr;

  class Component;
  typedef Component * Component_ptr;

  class Block;
  typedef Block * Block_ptr;
  class Source;
  typedef Source * Source_ptr;

  class Macro;
  typedef Macro * Macro_ptr;

  namespace u {
    class Copy;
    typedef Copy * Copy_ptr;

    class Switch;
    typedef Switch * Switch_ptr;
  }
}

namespace DSP {
  class Clock;
  typedef Clock * Clock_ptr;
}

namespace DSP {
  namespace e {
    enum struct ComponentType : unsigned int;
    ComponentType operator|(ComponentType __a, ComponentType __b);
    ComponentType operator&(ComponentType __a, ComponentType __b);

    enum struct SampleType;
    enum struct FileType;
    enum struct OffsetMode;
    enum struct PSK_type;
    enum struct ModulationType;
    enum struct BufferType;
  }

  class T_WAVEchunk;
  typedef T_WAVEchunk * T_WAVEchunk_ptr;
}

//! Definition of floating point type used in the DSP module
#define DSP_USE_float

#ifdef DSP_USE_float
//  typedef float DSP::Float;
  namespace DSP {
    typedef float Float;
  }

  #define COS cosf
  #define SIN sinf
  #define EXP expf
  #define FMOD fmodf
  #define CEIL ceilf
  #define FLOOR floorf
  #define SQRT sqrtf
  #define FABS fabsf
  #define POW powf
  #define ATAN2 atan2f

#else
  namespace DSP {
    typedef double DSP::Float;
  }

  #define COS cos
  #define SIN sin
  #define EXP exp
  #define FMOD fmod
  #define CEIL ceil
  #define FLOOR floor
  #define SQRT sqrt
  #define FABS fabs
  #define POW pow
  #define ATAN2 atan2
#endif

namespace DSP {
  const long double M_PIx1_prec = 3.14159265358979323846264338328;
  const long double M_PIx2_prec = 6.28318530718058647692528676656;

  // and DSP::Float precision
  const DSP::Float M_Ef         = DSP::Float(M_E);
  const DSP::Float M_LOG2Ef     = DSP::Float(M_LOG2E);
  const DSP::Float M_LOG10Ef    = DSP::Float(M_LOG10E);
  const DSP::Float M_LN2f       = DSP::Float(M_LN2);
  const DSP::Float M_LN10f      = DSP::Float(M_LN10);
  const DSP::Float M_PIf        = DSP::Float(M_PI);
  const DSP::Float M_PI_2f      = DSP::Float(M_PI_2);
  const DSP::Float M_PI_4f      = DSP::Float(M_PI_4);
  const DSP::Float M_1_PIf      = DSP::Float(M_1_PI);
  const DSP::Float M_2_PIf      = DSP::Float(M_2_PI);
  const DSP::Float M_2_SQRTPIf  = DSP::Float(M_2_SQRTPI);
  const DSP::Float M_SQRT2f     = DSP::Float(M_SQRT2);
  const DSP::Float M_SQRT1_2f   = DSP::Float(M_SQRT1_2);

  const DSP::Float M_PIx2 = DSP::Float(M_PIx2_prec);
  const DSP::Float M_PIx1 = DSP::Float(M_PIx1_prec);

  typedef DSP::Float * Float_ptr;
  typedef long double Prec_Float; //high precition float type
  class Complex;

  typedef DSP::Prec_Float * Prec_Float_ptr;
  typedef DSP::Complex * Complex_ptr;

  typedef void * void_ptr;


  typedef std::vector<DSP::Float > Float_vector;
  typedef std::vector<DSP::Prec_Float > Prec_Float_vector;
  typedef std::vector<DSP::Complex > Complex_vector;

  //! Pointer to the callback function
  /*! void func(unsigned int NoOfInputs,  DSP::Float_ptr InputSamples,
  *            unsigned int NoOfOutputs, DSP::Float_ptr OutputSamples,
  *            DSP::void_ptr *UserDataPtr, unsigned int UserDefinedIdentifier,
  *            DSP::Component_ptr Caller)
  *
  * UserDataPtr - default value is NULL, in this variable user can store
  *   the pointer to his own data structure
  *
  * This callback function apart from calls when input samples are
  * to be processed is called two additional times:
  *  -# call from block constructor with NoOfInputs = DSP::Callback_Init;
  *  this is when the user can initiate UserData structure
  *  and set its pointer to UserDataPtr.
  *  -# call from block destructor with NoOfInputs = DSP::Callback_Delete;
  *  this is when the user can free UserData structure.
  *
  */
  typedef void (*Callback_ptr)(unsigned int, DSP::Float_ptr,
                               unsigned int, DSP::Float_ptr,
                               DSP::void_ptr *, unsigned int,
                               DSP::Component_ptr);

  //! Pointer to the buffer callback function
  /*! void func(unsigned int NoOfInputs,
  *            unsigned int NoOfOutputs, DSP::Float_ptr OutputSamples,
  *            DSP::void_ptr *UserDataPtr, unsigned int UserDefinedIdentifier,
  *            DSP::Component_ptr Caller)
  *
  * UserDataPtr - default value is NULL, in this variable user can store
  *   the pointer to his own data structure
  *
  * This callback function apart from calls when input samples are
  * to be processed is called two additional times:
  *  -# call from block constructor with NoOfInputs = DSP::Callback_Init;
  *  this is when the user can initiate UserData structure
  *  and set its pointer to UserDataPtr.
  *  -# call from block destructor with NoOfInputs = DSP::Callback_Delete;
  *  this is when the user can free UserData structure.
  *
  */
  typedef void (*Buffer_callback_ptr)(unsigned int,
                                    unsigned int, DSP::Float_vector &,
                                    DSP::void_ptr *, unsigned int,
                                    DSP::Component_ptr);

  //! Pointer to the notification callback function
  /*! void func(DSP::Component_ptr Caller, unsigned int UserDefinedIdentifier)
  */
  typedef void (*Notify_callback_ptr)(DSP::Component_ptr, unsigned int);

  //! Pointer to the external sleep function implementation
  /*! void func(uint32_t time)
  */
  typedef void (*ExternalSleep_ptr)(uint32_t);
}

class DSP::Complex
{
  public:
   DSP::Float re;
   DSP::Float im;

   Complex(void)
   {
     re=0.0; im=0.0;
   }
   Complex(const DSP::Float& re_in)
   {
     re=re_in; im=0.0;
   }
   template <typename T>
   Complex(const T& re_in, const T& im_in)
   {
     re=(DSP::Float)re_in; im=(DSP::Float)im_in;
   }

   void set(DSP::Float re_in)
   { re=re_in; im=0.0; };
   void set(DSP::Float re_in, DSP::Float im_in)
   { re=re_in; im=im_in; };
   void set(double re_in, double im_in)
   { re=(DSP::Float)re_in; im=(DSP::Float)im_in; };
   void set(DSP::Complex number)
   { re=number.re; im=number.im; };

   friend const DSP::Complex operator* (const DSP::Complex& left,
                                       const DSP::Complex& right)
   {
//     (a.re+j*a.im)*(b.re+j*b.im)
//     a.re*b.re-a.im*b.im+j*(a.re*b.im+a.im*b.re)
     return DSP::Complex(left.re*right.re-left.im*right.im,
                        left.re*right.im+left.im*right.re);
   }

   friend const DSP::Complex operator+ (const DSP::Complex& left,
                                       const DSP::Complex& right)
   {
     return DSP::Complex(left.re+right.re, left.im+right.im);
   }

   friend const DSP::Complex operator- (const DSP::Complex& left,
                                       const DSP::Complex& right)
   {
     return DSP::Complex(left.re-right.re, left.im-right.im);
   }

   friend DSP::Complex& operator+= (DSP::Complex& left,
                                   const DSP::Complex& right)
   {
     left.re+=right.re;
     left.im+=right.im;
     return left;
   }

   void add(DSP::Complex number)
   {
     re+=number.re;
     im+=number.im;
   }

   void sub(DSP::Complex number)
   {
     re-=number.re;
     im-=number.im;
   }

   void divide_by(DSP::Complex factor)
   {
     DSP::Float mod;

     mod=factor.re*factor.re+factor.im*factor.im;
     mod=SQRT(mod);
     factor.re/=mod; factor.im/=(-mod);
     re/=mod; im/=mod;

     multiply_by(factor);
   }

   void multiply_by(DSP::Complex factor)
   {
     DSP::Float temp;

     temp=re;
     re=temp*factor.re-im*factor.im;
     im=temp*factor.im+im*factor.re;

     /* _bug this is temporary solution but problem probably lays
      * somewhere else
     #ifdef DSP_USE_float
       unsigned char *s;

       s=(unsigned char *)(&re)+3;
       if (((*s)<<1)==0)
         re=0.0;
       s=(unsigned char *)(&im)+3;
       if (((*s)<<1)==0)
         im=0.0;
//       #error Float problem
     #endif
     */
   }

   void multiply_by_conj(DSP::Complex factor)
   {
     DSP::Float temp;

     temp=re;
     re= temp*factor.re+im*factor.im;
     im=-temp*factor.im+im*factor.re;
   }

   void multiply_by(DSP::Float factor)
   {
     re*=factor;
     im*=factor;
   }

   DSP::Float abs(void)
   {
     return SQRT(re*re+im*im);
   }

   friend DSP::Float abs(const DSP::Complex& val)
   {
     return SQRT(val.re*val.re+val.im*val.im);
   }

   // square of absolute value of  complex number
   friend DSP::Float abs2(const DSP::Complex& val)
   {
     return val.re*val.re+val.im*val.im;
   }

   //! result for -1+j0 should rather be -PI but is +PI
   /*! However the atan2 function is extremely sensitive to
    * quantization noise of the imaginary part of the sample.
    */
   DSP::Float angle(void)
   {
//     return -atan2(-im, re);
     return ATAN2(im, re);
   }
};

//! component type
enum struct DSP::e::ComponentType : unsigned int {
            none=0, 
            block=1,
            source=2, 
            mixed=3,
            copy=4
};
inline DSP::e::ComponentType DSP::e::operator|(DSP::e::ComponentType __a, DSP::e::ComponentType __b)
{ 
  return static_cast<DSP::e::ComponentType>(static_cast<std::underlying_type<DSP::e::ComponentType>::type>(__a) 
                                          | static_cast<std::underlying_type<DSP::e::ComponentType>::type>(__b));
}
inline DSP::e::ComponentType DSP::e::operator&(DSP::e::ComponentType __a, DSP::e::ComponentType __b)
{ 
  return static_cast<DSP::e::ComponentType>(static_cast<std::underlying_type<DSP::e::ComponentType>::type>(__a) 
                                          & static_cast<std::underlying_type<DSP::e::ComponentType>::type>(__b));
}

//! Enums for different Sample Types for file IO operations
/*! - DSP::e::SampleType::ST_float    : C++ float  (32bit floating point)
 *  - DSP::e::SampleType::ST_scaled_float : C++ float * 0x8000 (32bit floating point scalled by 0x8000 for 32bit PCM wav file)
 *  - DSP::e::SampleType::ST_double   : C++ double (64bit floating point)
 *  - DSP::e::SampleType::ST_long_double : C++ long double (80bit floating point)
 *  - DSP::e::SampleType::ST_short : C++ short  (16bit signed integer)
 *  - DSP::e::SampleType::ST_uchar : C++ unsigned char  (8bit unsigned integer with bias (0x80))
 *  - DSP::e::SampleType::ST_bit   : 1 bit stream (MSB first)
 *  - DSP::e::SampleType::ST_bit_reversed : reversed 1 bit stream (LSB first)
 *  - DSP::e::SampleType::ST_bit_text : 1 bit stream writen as a text (0 & 1 characters)
 *  - DSP::e::SampleType::ST_tchar : 8-bit char (text) - no scalling of input data (only DSP::u::FileOutput with DSP::e::FileType::FT_raw)
 *  .
 *
 * Fixed <b>2006.06.30</b> Changed name from DSP_FileType to DSP_SampleType
 *   and prefix from DSP::e::FileType::FT_ to DSP_ST_:
 */
enum struct DSP::e::SampleType {
       ST_none = 0, ST_float = 1, ST_scaled_float = 2,
       ST_uchar = 3, ST_short = 4, ST_int = 5,
       ST_bit = 6, ST_bit_reversed = 7, ST_bit_text = 8,
       ST_double = 9, ST_long_double = 10,
       ST_tchar = 11};

//! Enums for different File Types == file header type for file IO operations
/*! -# DSP::e::FileType::FT_raw   : no header just raw data
 *  -# DSP::e::FileType::FT_flt   : floating point content file with additional info
 *     - See ::T_FLT_header
 *     .
 *  -# DSP::e::FileType::FT_wav   : Windows WAV file (uncompressed/PCM)
 *  -# DSP::e::FileType::FT_tape  : *.tape file
 *     - See ::T_TAPE_header
 *     .
 *  .
 */
enum struct DSP::e::FileType {
        FT_raw = 0,
        FT_flt = 1,
        // FT_no_scaling = 2,
        FT_flt_no_scaling = 3, // (FT_flt | FT_no_scaling)
        FT_wav = 4,
        FT_tape = 5};

//! Enums for DSP::u::FileOutput
enum struct DSP::e::OffsetMode {
  //! from beginning (after header)
  standard     = 0,
  //! from current position
  skip         = 1};

//! Enums for PSK type
enum struct DSP::e::PSK_type {
        BPSK=0,
        DEBPSK=1,
        DBPSK=2,
        QPSK_A=3,
        QPSK_B=4,
        DQPSK=5,
        pi4_QPSK=6
      };

//! Enums for Modulation types
enum struct DSP::e::ModulationType {
        PSK=0,
        ASK=1,
        DPSK=2, // differential variant of the PSKodulation
        QAM=3
      };

//! Enums for DSP::u::OutputBuffer
enum struct DSP::e::BufferType {
        standard       = 0,
        cyclic         = 1,
        stop_when_full = 2
        };

namespace DSP {
  const uint32_t CallbackID_mask = 0x00ffffff;
  const uint32_t CallbackID_signal_mask = 0xff000000;
  const uint32_t CallbackID_signal_start = 0x01ffffff;
  const uint32_t CallbackID_signal_stop  = 0x02ffffff;

//#define UINT_MAX 0xffffffff
  const uint32_t MaxOutputIndex = (UINT_MAX-2);
  const uint32_t MaxInputIndex  = (UINT_MAX-2);
//! DSP::Block::FindOutputIndex_by_InputIndex constants
  const uint32_t Callback_Init = (UINT_MAX);
  const uint32_t Callback_Delete = (UINT_MAX-1);
  const uint32_t FO_TheOnlyOutput = (UINT_MAX-1);
  const uint32_t FO_NoOutput      = (UINT_MAX);
  const uint32_t FO_NoInput       = (UINT_MAX);
}

namespace DSP {
//! Pointer to the DSP::Block input callback function
/*! void func(DSP::Block_ptr block, unsigned int InputNo, DSP::Float value, DSP::Component *Caller)
 *
 *  \note In release mode last parameter (DSP::Component *) is skipped
 */
#ifdef __DEBUG__
  typedef void (*Block_Execute_ptr)(const DSP::Block_ptr, unsigned int, DSP::Float, const DSP::Component_ptr);
#else
  typedef void (*Block_Execute_ptr)(const DSP::Block_ptr, unsigned int, DSP::Float);
#endif
}

//! Pointer to the DSP::Source output callback function
/*! bool func(DSP::Source_ptr source, DSP::Clock_ptr clock)
 *
 * This function should return false when source is not ready yet
 * (in such case this function will be called again later still for the same clock cycle),
 * and true when source was ready and have already sent the output value(s).
 *
 * \note In release mode last parameter (DSP::Clock_ptr) is skipped
 */
namespace DSP {
  #ifdef __DEBUG__
    typedef bool (*Source_Execute_ptr)(const DSP::Source_ptr, const DSP::Clock_ptr);
  #else
    typedef bool (*Source_Execute_ptr)(const DSP::Source_ptr);
  #endif
}

#ifdef __DEBUG__
  #define INPUT_EXECUTE_ARGS  DSP::Block_ptr block, unsigned int InputNo, DSP::Float value, DSP::Component_ptr Caller
  #define EXECUTE_PTR(block, InputNo, value, Caller)  Execute_ptr((DSP::Block_ptr)block, InputNo, value, (DSP::Component_ptr)Caller)
  #define EXECUTE_INPUT_CALLBACK(InputExecute, block, InputNo, value, Caller) InputExecute(block, InputNo, value, Caller)

  #define OUTPUT_EXECUTE_ARGS DSP::Source_ptr source, DSP::Clock_ptr clock
  #define OUTPUT_EXECUTE_PTR(source, clock) OutputExecute_ptr((DSP::Source_ptr)source, (DSP::Clock_ptr)clock)
#else
  #define INPUT_EXECUTE_ARGS DSP::Block_ptr block, unsigned int InputNo, DSP::Float value
  #define EXECUTE_PTR(block, InputNo, value, Caller)  Execute_ptr((DSP::Block_ptr)block, InputNo, value)
  #define EXECUTE_INPUT_CALLBACK(InputExecute, block, InputNo, value, Caller) InputExecute(block, InputNo, value)

  #define OUTPUT_EXECUTE_ARGS const DSP::Source_ptr source
  #define OUTPUT_EXECUTE_PTR(source, clock) OutputExecute_ptr((DSP::Source_ptr)source)
#endif


namespace DSP {
  namespace u {
    class AudioInput;
    class AudioOutput;
  }

//  typedef std::function<bool(const DSP::e::SampleType &, const std::vector<char> &)> input_callback_function;
  // https://isocpp.org/wiki/faq/pointers-to-members#typedef-for-ptr-to-memfn
  //! defining a type for pointer to member funtion of DSP::u::AudioInput
  typedef bool(DSP::u::AudioInput::*input_callback_function)(const DSP::e::SampleType &, const std::vector<char> &);
  //! defining a type for pointer to member funtion of DSP::u::AudioOutput
  typedef bool(DSP::u::AudioOutput::*output_callback_function)(const DSP::e::SampleType &, const std::vector<char> &);

    //! Base class for classes implementing sound card support for DSP::u::AudioInput and DSP::u::AudioOutput 
    /*! \TODO  convert WMM support in DSP::u::AudioInput and DSP::u::AudioOutput into support through WMM_object_t
     */
  class SOUND_object_t {
    private:
      DSP::u::AudioInput * AudioInput_object;
      //bool (DSP::u::AudioInput::*AudioInput_callback)(const DSP::e::SampleType &InSampleType, const std::vector<char> &wave_in_raw_buffer);
      input_callback_function AudioInput_callback;
      DSP::u::AudioOutput * AudioOutput_object;
      output_callback_function AudioOutput_callback;

      //static unsigned long Next_CallbackInstance;
      unsigned long Current_CallbackInstance;
      static std::vector<DSP::SOUND_object_t *> CallbackSoundObjects;
      static unsigned long get_free_CallbackInstance(void);

    protected:
      long int get_current_CallbackInstance();
      static const DSP::SOUND_object_t * get_CallbackSoundObject(const long int &instance_number);

    public:
      virtual void log_driver_data() = 0;

      virtual unsigned int select_input_device_by_number(const unsigned int &device_number=UINT_MAX) = 0;
      virtual unsigned int select_output_device_by_number(const unsigned int &device_number=UINT_MAX) = 0;

      //! audio_outbuffer_size is in samples (note that, for example, sample for 16bit stereo is represented by 4bytes)
      virtual long open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size = -1) = 0;
      //! audio_inbuffer_size is in samples (note that, for example, sample for 16bit stereo is represented by 4bytes)
      virtual long open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_inbuffer_size = -1) = 0;
      virtual bool close_PCM_device_input(void) = 0;
      //! close sound card output
      /*! if do_drain == true wait until sound card stops playing 
       */ 
      virtual bool close_PCM_device_output(const bool &do_drain) = 0;

      //! returns true is the playback is on
      virtual bool is_device_playing(void) = 0;
      //! initializes playback stopping
      /*! If there are still buffers that haven't been yet sent to sound card then do it now
       */
      virtual bool stop_playback(void) = 0;
      //! returns true is the sound capture is on
      virtual bool is_device_recording(void) = 0;
      //! returns true is the sound capture is on
      virtual bool stop_recording(void) = 0;

      /*! Appends data to audio buffers and sends data to sound card is buffer is full.
      * The class implementation should provide at least DSP::NoOfAudioOutputBuffers buffers with space for audio_outbuffer_size sample each.  
      * When sound is playing then if the function call results in filling the buffer (or buffers) 
      * all full buffers should be sent to the sound card.
      * When sound has not stated playing yest then full buffers sould be sent to the sound card only when 
      * DSP::NoOfAudioOutputBuffers-1 buffers are filled. This prevents sound staterring and leaves one spare buffer for new samples.
      * 
      * \note values stored in float_buffer might be altered
      */
      virtual long append_playback_buffer(DSP::Float_vector &float_buffer)=0;
      //! Starts sound capture
      virtual bool start_recording(void) = 0;


      // https://stackoverflow.com/questions/8865766/get-a-pointer-to-objects-member-function
      /*! Returns false if callbacks are not supported for recording
       */
      virtual bool is_input_callback_supported(void) = 0;
      /*! Registers callback for input buffer data ready. Returns false if callbacks are not supported
       * If this method fails or is not used the SOUND_ocject falls back into non-callback mode.
       * 
       * Registers as a callback function that is a member function of the AudioInput class along with the object of the AudioInput class for which it should be called.
       * - bool input_callback_function(const DSP::e::SampleType &InSampleType, const std::vector<char> &wave_in_raw_buffer);
       * 
       * The callback function will be called by the SOUND_object when sound card's input buffer is ready.
       * The callback function has to return true when it processed the input buffer or false when it cannot to process it.
       * On false the SOUND_object most probably will discard the buffer. Nevertheless it can try to call the callback again for the same data.
       */
      bool register_input_callback_object(DSP::u::AudioInput *callback_object, input_callback_function &cp);
      //! Returns pointer to DSP::u::AudioInput for which callback is registered.
      /*! \note If this function  returns NULL, callbacks are not used even if they are supported.
       */
      DSP::u::AudioInput *get_input_callback_object();
  protected:
      //! \note this method should be used only by descending classes
      bool input_callback_call(const DSP::e::SampleType &InSampleType, const std::vector<char> &wave_in_raw_buffer);
  public:
      //! If enough audio data are already available then fills InSampleType and wave_in_raw_buffer
      /*! return true on success or false if the data are not available 
       * \note this method should be used only if callbacks are not active
       */
      virtual bool get_wave_in_raw_buffer(DSP::e::SampleType &InSampleType, std::vector<char> &wave_in_raw_buffer) = 0;

      /*! Returns false if callbacks are not supported for playback
       */
      virtual bool is_output_callback_supported(void) = 0;
      /*! Returns false if callbacks are not supported
       * 
       * Registers as a callback function that is a member function of the AudioOutput class along with the object of the AudioOutput class for which it should be called.
       * - bool output_callback_function(const DSP::e::SampleType &OutSampleType, const std::vector<char> &wave_out_raw_buffer);
       * 
       * The callback function will be called by the SOUND_object when new sound card's output buffer can processed.
       * The callback function has to return true when it filled the buffer with samples or false when there is not enought data.
       * On false the SOUND_object most probably will discard the buffer. Nevertheless it can try to call the callback again.
       * \TODO revise the concept when the there will be a SOUND_object_t derivative that uses this approach.
       * \note Needs addaptation of DSP::u::AudioOutput class.
       */
      bool register_output_callback_object(DSP::u::AudioOutput *callback_object, output_callback_function &cp);
      /*! \note If this function  returns NULL, callbacks are not used are not used even if they are supported.
       */
      DSP::u::AudioOutput *get_output_callback_object();
  protected:
      bool output_callback_call(const DSP::e::SampleType &OutSampleType, const std::vector<char> &wave_out_raw_buffer);
  
  public:
      SOUND_object_t();
      virtual ~SOUND_object_t();
  };
}


#include <DSP_DOT.h>

#endif
