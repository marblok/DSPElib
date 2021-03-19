/*! \file DSP_types.h
 * This is DSP engine types definition header file.
 *
 * \author Marek Blok
 */
#ifndef DSP_types_H
#define DSP_types_H

#include <limits.h>
#include <math.h>
#include <vector>
#include <string>

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

}

class DSPu_Copy;
typedef DSPu_Copy * DSPu_Copy_ptr;

class DSPu_Switch;
typedef DSPu_Switch * DSPu_Switch_ptr;

namespace DSP {
  class Clock;
  typedef Clock * Clock_ptr;
}

namespace DSP {
  namespace e {
    enum struct SampleType;
    enum struct FileType;
  }
}
class DSP_complex;
typedef DSP_complex * DSP_complex_ptr;

class T_WAVEchunk;
typedef T_WAVEchunk * T_WAVEchunk_ptr;

//  16-bit types  -----------------------------------------------------------//
#if USHRT_MAX == 0xffff
   typedef unsigned short  WORD;
#else
   #error Cannot define WORD
#endif

//  32-bit types  -----------------------------------------------------------//
#if ULONG_MAX == 0xffffffff
  typedef unsigned long   DWORD;
#elif UINT_MAX == 0xffffffff
  typedef unsigned int   DWORD;
#else
  #error Cannot define DWORD
#endif
#ifndef BYTE
  typedef unsigned char BYTE;
#endif
#ifndef MAXDWORD
  #define MAXDWORD  ((unsigned int)0xffffffff)
#endif
#ifndef MAXWORD
  #define MAXWORD  ((unsigned short)0xffff)
#endif


//! Definition of floating point type used in the DSP module
#define DSP_USE_float

#ifdef DSP_USE_float
  typedef float DSP_float;

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

  #define DSP_M_PIx2 M_PIx2f
  #define DSP_M_PIx1 M_PIx1f

#else
  typedef double DSP_float;

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

  #define DSP_M_PIx2 M_PIx2
  #define DSP_M_PIx1 M_PIx1

#endif
typedef DSP_float * DSP_float_ptr;

typedef long double DSP_prec_float; //high precition float type
typedef DSP_prec_float * DSP_prec_float_ptr;

namespace DSP {
  typedef void * void_ptr;
}

//! Pointer to the callback function
/*! void func(unsigned int NoOfInputs,  DSP_float_ptr InputSamples,
 *            unsigned int NoOfOutputs, DSP_float_ptr OutputSamples,
 *            DSP::void_ptr *UserDataPtr, unsigned int UserDefinedIdentifier,
 *            DSP::Component_ptr Caller)
 *
 * UserDataPtr - default value is NULL, in this variable user can store
 *   the pointer to his own data structure
 *
 * This callback function apart from calls when input samples are
 * to be processed is called two additional times:
 *  -# call from block constructor with NoOfInputs = DSP_Callback_Init;
 *  this is when the user can initiate UserData structure
 *  and set its pointer to UserDataPtr.
 *  -# call from block destructor with NoOfInputs = DSP_Callback_Delete;
 *  this is when the user can free UserData structure.
 *
 */
typedef void (*DSPu_callback_ptr)(unsigned int, DSP_float_ptr,
                                  unsigned int, DSP_float_ptr,
                                  DSP::void_ptr *, unsigned int,
                                  DSP::Component_ptr);

//! Pointer to the buffer callback function
/*! void func(unsigned int NoOfInputs,
 *            unsigned int NoOfOutputs, DSP_float_ptr OutputSamples,
 *            DSP::void_ptr *UserDataPtr, unsigned int UserDefinedIdentifier,
 *            DSP::Component_ptr Caller)
 *
 * UserDataPtr - default value is NULL, in this variable user can store
 *   the pointer to his own data structure
 *
 * This callback function apart from calls when input samples are
 * to be processed is called two additional times:
 *  -# call from block constructor with NoOfInputs = DSP_Callback_Init;
 *  this is when the user can initiate UserData structure
 *  and set its pointer to UserDataPtr.
 *  -# call from block destructor with NoOfInputs = DSP_Callback_Delete;
 *  this is when the user can free UserData structure.
 *
 */
typedef void (*DSPu_buffer_callback_ptr)(unsigned int,
                                  unsigned int, DSP_float_ptr,
                                  DSP::void_ptr *, unsigned int,
                                  DSP::Component_ptr);

//! Pointer to the notification callback function
/*! void func(DSP::Component_ptr Caller, unsigned int UserDefinedIdentifier)
 */
typedef void (*DSPu_notify_callback_ptr)(DSP::Component_ptr, unsigned int);

namespace DSP {
  //! Pointer to the external sleep function implementation
  /*! void func(DWORD time)
  */
  typedef void (*ExternalSleep_ptr)(DWORD);
}

#define M_PIx1  3.14159265358979323846264338328
#define M_PIx2  6.28318530718058647692528676656
#define M_PIx1f      DSP_float(M_PIx1)
#define M_PIx2f      DSP_float(M_PIx2)

// and DSP_float precision
#define M_Ef         DSP_float(M_E)
#define M_LOG2Ef     DSP_float(M_LOG2E)
#define M_LOG10Ef    DSP_float(M_LOG10E)
#define M_LN2f       DSP_float(M_LN2)
#define M_LN10f      DSP_float(M_LN10)
#define M_PIf        DSP_float(M_PI)
#define M_PI_2f      DSP_float(M_PI_2)
#define M_PI_4f      DSP_float(M_PI_4)
#define M_1_PIf      DSP_float(M_1_PI)
#define M_2_PIf      DSP_float(M_2_PI)
#define M_2_SQRTPIf  DSP_float(M_2_SQRTPI)
#define M_SQRT2f     DSP_float(M_SQRT2)
#define M_SQRT1_2f   DSP_float(M_SQRT1_2)
class DSP_complex
{
  public:
   DSP_float re;
   DSP_float im;

   DSP_complex(void)
   {
     re=0.0; im=0.0;
   }
   DSP_complex(const DSP_float& re_in)
   {
     re=re_in; im=0.0;
   }
   template <class T>
   DSP_complex(const T& re_in, const T& im_in)
   {
     re=(DSP_float)re_in; im=(DSP_float)im_in;
   }

   void set(DSP_float re_in)
   { re=re_in; im=0.0; };
   void set(DSP_float re_in, DSP_float im_in)
   { re=re_in; im=im_in; };
   void set(double re_in, double im_in)
   { re=(DSP_float)re_in; im=(DSP_float)im_in; };
   void set(DSP_complex number)
   { re=number.re; im=number.im; };

   friend const DSP_complex operator* (const DSP_complex& left,
                                       const DSP_complex& right)
   {
//     (a.re+j*a.im)*(b.re+j*b.im)
//     a.re*b.re-a.im*b.im+j*(a.re*b.im+a.im*b.re)
     return DSP_complex(left.re*right.re-left.im*right.im,
                        left.re*right.im+left.im*right.re);
   }

   friend const DSP_complex operator+ (const DSP_complex& left,
                                       const DSP_complex& right)
   {
     return DSP_complex(left.re+right.re, left.im+right.im);
   }

   friend const DSP_complex operator- (const DSP_complex& left,
                                       const DSP_complex& right)
   {
     return DSP_complex(left.re-right.re, left.im-right.im);
   }

   friend DSP_complex& operator+= (DSP_complex& left,
                                   const DSP_complex& right)
   {
     left.re+=right.re;
     left.im+=right.im;
     return left;
   }

   void add(DSP_complex number)
   {
     re+=number.re;
     im+=number.im;
   }

   void sub(DSP_complex number)
   {
     re-=number.re;
     im-=number.im;
   }

   void divide_by(DSP_complex factor)
   {
     DSP_float mod;

     mod=factor.re*factor.re+factor.im*factor.im;
     mod=SQRT(mod);
     factor.re/=mod; factor.im/=(-mod);
     re/=mod; im/=mod;

     multiply_by(factor);
   }

   void multiply_by(DSP_complex factor)
   {
     DSP_float temp;

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

   void multiply_by_conj(DSP_complex factor)
   {
     DSP_float temp;

     temp=re;
     re= temp*factor.re+im*factor.im;
     im=-temp*factor.im+im*factor.re;
   }

   void multiply_by(DSP_float factor)
   {
     re*=factor;
     im*=factor;
   }

   DSP_float abs(void)
   {
     return SQRT(re*re+im*im);
   }

   friend DSP_float abs(const DSP_complex& val)
   {
     return SQRT(val.re*val.re+val.im*val.im);
   }

   // square of absolute value of  complex number
   friend DSP_float abs2(const DSP_complex& val)
   {
     return val.re*val.re+val.im*val.im;
   }

   //! result for -1+j0 should rather be -PI but is +PI
   /*! However the atan2 function is extremely sensitive to
    * quantization noise of the imaginary part of the sample.
    */
   DSP_float angle(void)
   {
//     return -atan2(-im, re);
     return ATAN2(im, re);
   }
};

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
 *  - DSP::e::SampleType::ST_tchar : 8-bit char (text) - no scalling of input data (only DSPu_FILEoutput with DSP::e::FileType::FT_raw)
 *  .
 *
 * \Fixed <b>2006.06.30</b> Changed name from DSP_FileType to DSP_SampleType
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

//! Enums for DSPu_FILEoutput
enum DSPe_offset_mode {
  //! from beginning (after header)
  DSP_OM_standard     = 0,
  //! from current position
  DSP_OM_skip         = 1};

//! Enums for PSK type
enum DSPe_PSK_type {DSP_BPSK=0,
                    DSP_DEBPSK=1,
                    DSP_DBPSK=2,
                    DSP_QPSK_A=3,
                    DSP_QPSK_B=4,
                    DSP_DQPSK=5,
                    DSP_pi4_QPSK=6};

//! Enums for Modulation types
enum DSPe_Modulation_type {
                    DSP_MT_PSK=0,
                    DSP_MT_ASK=1,
                    DSP_MT_DPSK=2, // differential variant of the PSKodulation
                    DSP_MT_QAM=3
                   };

//! Enums for DSPu_OutputBuffer
enum DSPe_buffer_type {DSP_standard       = 0,
                       DSP_cyclic         = 1,
                       DSP_stop_when_full = 2};
#define CallbackID_mask  0x00ffffff
#define CallbackID_signal_mask  0xff000000
#define CallbackID_signal_start 0x01ffffff
#define CallbackID_signal_stop  0x02ffffff

//#define UINT_MAX 0xffffffff
#define MaxOutputIndex (UINT_MAX-2)
#define MaxInputIndex  (UINT_MAX-2)
//! DSP::Block::FindOutputIndex_by_InputIndex constants
#define DSP_Callback_Init (UINT_MAX)
#define DSP_Callback_Delete (UINT_MAX-1)
#define FO_TheOnlyOutput (UINT_MAX-1)
#define FO_NoOutput      (UINT_MAX)
#define FO_NoInput       (UINT_MAX)


namespace DSP {
//! Pointer to the DSP::Block input callback function
/*! void func(DSP::Block_ptr block, unsigned int InputNo, DSP_float value, DSP::Component *Caller)
 *
 *  \note In release mode last parameter (DSP::Component *) is skipped
 */
#ifdef __DEBUG__
  typedef void (*Block_Execute_ptr)(const DSP::Block_ptr, unsigned int, DSP_float, const DSP::Component_ptr);
#else
  typedef void (*Block_Execute_ptr)(const DSP::Block_ptr, unsigned int, DSP_float);
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
  #define INPUT_EXECUTE_ARGS  DSP::Block_ptr block, unsigned int InputNo, DSP_float value, DSP::Component_ptr Caller
  #define EXECUTE_PTR(block, InputNo, value, Caller)  Execute_ptr((DSP::Block_ptr)block, InputNo, value, (DSP::Component_ptr)Caller)
  #define EXECUTE_INPUT_CALLBACK(InputExecute, block, InputNo, value, Caller) InputExecute(block, InputNo, value, Caller)

  #define OUTPUT_EXECUTE_ARGS DSP::Source_ptr source, DSP::Clock_ptr clock
  #define OUTPUT_EXECUTE_PTR(source, clock) OutputExecute_ptr((DSP::Source_ptr)source, (DSP::Clock_ptr)clock)
#else
  #define INPUT_EXECUTE_ARGS DSP::Block_ptr block, unsigned int InputNo, DSP_float value
  #define EXECUTE_PTR(block, InputNo, value, Caller)  Execute_ptr((DSP::Block_ptr)block, InputNo, value)
  #define EXECUTE_INPUT_CALLBACK(InputExecute, block, InputNo, value, Caller) InputExecute(block, InputNo, value)

  #define OUTPUT_EXECUTE_ARGS const DSP::Source_ptr source
  #define OUTPUT_EXECUTE_PTR(source, clock) OutputExecute_ptr((DSP::Source_ptr)source)
#endif


typedef std::vector<DSP_float > DSP_float_vector;
typedef std::vector<DSP_prec_float > DSP_prec_float_vector;
typedef std::vector<DSP_complex > DSP_complex_vector;



#include <DSP_DOT.h>

#endif
