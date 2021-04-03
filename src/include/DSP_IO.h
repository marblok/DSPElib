//---------------------------------------------------------------------------
/*! \file DSP_IO.h
 * This is Input/Output DSP module header file.
 *
 * Module blocks providing input and output from external devices
 * like hard drive or sound card.
 *
 * \todo fully integrate DSP::u::WaveInput with DSP::u::FileInput
 *
 * \author Marek Blok
 */
#ifndef DSP_IO_H
#define DSP_IO_H

#include <string.h>
//#include <cmath>
#include <stdio.h>

//#if !defined(__WIN32__) && (defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__))
//#  define __WIN32__
//#endif

////#ifdef __CYGWIN__
//#ifdef WIN32
#if defined(WIN32) || defined(WIN64)
  #include <windef.h>
  #include <mmsystem.h>
#else
  #ifdef LINUX
    //! \TODO adapt for linux
    #include "ALSA_support.h"

  #else
    #error NO WIN32
  #endif
#endif

//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------
#include <DSP_modules.h>
#include <DSP_types.h>
//---------------------------------------------------------------------------

//#define AUDIO_DEBUG_MESSAGES_ON
#define AUDIO_DEBUG_MESSAGES_OFF

namespace DSP {
  const unsigned long File_buffer_size = 8192;
  const unsigned long DefaultSamplingFrequency = 8000;

  //! Reference sampling frequency for which buffer sizes are selected
  const unsigned long ReferenceFs = 48000;
  // (1024*4) for Fp=48000 ==> 85ms ==> 11,72 buffer per second
  const unsigned long Reference_audio_outbuffer_size = (1024*4); //(1024*64)
  const unsigned long Reference_audio_inbuffer_size = (1024*4);

  namespace e {
    enum struct AudioBufferType {none=0, input=1, output=2};
  }

  //! Number of input buffers in DSP::u::AudioInput
  const unsigned long NoOfAudioInputBuffers = 4;
  //! Number of output buffers in DSP::u::AudioOutput
  /*! with one  spare buffer
  *
  *  in this case its internal buffer
  */
  const unsigned long NoOfAudioOutputBuffers = 3;

  namespace f { //! Special functions namespace
    unsigned long ReadCoefficientsFromFile(DSP::Float_vector &Buffer, unsigned long N, const string &FileName, const string &FileDir, DSP::e::SampleType type, unsigned long offset);
    unsigned long ReadCoefficientsFromFile(DSP::Complex_vector &Buffer, unsigned long N, const string &FileName, const string &FileDir, DSP::e::SampleType type, unsigned long offset);
    bool GetWAVEfileParams(const string &FileName, const string &FileDir, T_WAVEchunk_ptr WAVEparams);
    //! Determines file type by filename extension
    DSP::e::FileType FileExtToFileType(const string &filename);

    //! This function adds the ability to get wav file params before DSP::Block creation
    /*! Returns false if the file cannot be opened.
    *
    * \ingroup load_func
    */
    bool GetWAVEfileParams(const string &FileName, const string &FileDir, T_WAVEchunk_ptr WAVEparams);

    //! returns audio buffer size for given Fs based on reference values given for DSP::ReferenceFs
    uint32_t GetAudioBufferSize(const unsigned long &SamplingFreq, const DSP::e::AudioBufferType &type);

    #ifdef WINMMAPI
      //! Issues error message and returns true if error detected
      bool AudioCheckError(MMRESULT result);
    #endif
  }

  //! Namespace for units' classes (componenst: blocks and sources)
  namespace u {
    class Vacuum;

    class AudioInput;
    class AudioOutput;

    class FileInput;
    class FileOutput;
    class WaveInput;

    class InputBuffer;
    class OutputBuffer;
  }
}

//! Reads coefficients from file.
/*! Reads N coefficients from file Filename (in directory DirName)
 *  to the Buffer.\n
 *  Values in file must be stored in given format (type). Function makes conversion
 *  from given format to DSP::Float format.\n
 *  File reading starts from given offset (in bytes) which allows for reading
 *  from mixed data type files.
 *
 *  Function returns number of bytes read from file.
 *
 *  Supported data types:
 *   - DSP::e::FileType::FT_uchar - 8bit unsigned char: out=(val-128)/128 (0x80)
 *   - DSP::e::FileType::FT_short - 16bit signed integer: out=val/32768 (0x8000)
 *   - DSP::e::FileType::FT_float - single precision (32bit) floating point
 *
 * \ingroup load_func
 */
unsigned long DSP::f::ReadCoefficientsFromFile(DSP::Float_vector &Buffer, unsigned long N,
                     const string &FileName, const string &FileDir,
                     DSP::e::SampleType type=DSP::e::SampleType::ST_float,
                     unsigned long offset=0);

namespace DSP {
  /*! \todo Add support for Sony Wave64 (.w64)
  */
  class T_WAVEchunk
  {
    public:
      char Type[4];  // "RIFF"
      uint32_t size;    // chunk size = file size - 8 (size of chunk header)

      char SubType[4]; // "WAVE"

      char FmtType[4]; // "fmt "
      uint32_t Fmtsize;   // 16 + extra bytes

      uint16_t wFormatTag; // Data encoding format: 1 - PCM
      uint16_t nChannels;  // Number of channels
      uint32_t nSamplesPerSec;   // Samples per second
      uint32_t nAvgBytesPerSec;  // Avg transfer rate = nSamplesPerSec*nBlockAlign;
      uint16_t  nBlockAlign;      // Block alignment
      uint16_t  wBitsPerSample;   // Bits per sample

      char DataType[4];
      uint32_t DataSize; // numbers if bytes of samples' data (without padding byte)
                      // note: data segment must be padded with 0 if it is not of even length
      // END OF HEADER //

      uint32_t BytesRead;
      int HeaderSize;

      bool WAVEinfo(FILE *hIn);
      bool FindDATA(FILE *hIn);

      void PrepareHeader(uint32_t nSamplesPerSec_in = 8000, uint16_t nChannels_in = 1, uint16_t  wBitsPerSample_in = 16);
      bool WriteHeader(FILE *hOut);
      bool UpdateHeader(FILE *hOut);

      T_WAVEchunk(void);
      // zeruj stan
      void clear();

      friend bool DSP::f::GetWAVEfileParams(const string &FileName, const string &FileDir, T_WAVEchunk_ptr WAVEparams);
    
    private:
      int strncmpi(const char* str1, const char* str2, int N);
  };


  const unsigned long FLT_header_LEN = 8;
  /*! DSP::e::FileType::FT_flt : floating point content file header
  * - 3 B - file version (ubit24)
  *    - 1B - header version
  *    - 2B - sample type
  *    0x00 0x0000 - sample_type = 'float'; sample_size = 4;
  *    0x00 0x0001 - sample_type = 'uchar'; sample_size = 1;
  *    0x00 0x0002 - sample_type = 'short'; sample_size = 2;
  *    0x00 0x0003 - sample_type = 'int';   sample_size = 4;
  * - 1 B - number of channels       (uint8)
  * - 4 B - sampling rate            (uint32)
    */
  class T_FLT_header
  {
    private:
      unsigned char data[FLT_header_LEN];
    public:
  //    unsigned char  version;        1B
      unsigned char version(void);
      void version(unsigned char val);
  //    unsigned short sample_type;    2B
      unsigned short sample_type(void);
      void sample_type(unsigned short val);
  //    unsigned char  no_of_channels; 1B
      unsigned char no_of_channels(void);
      void no_of_channels(unsigned char val);
  //    unsigned int   sampling_rate;  4B
      unsigned int sampling_rate(void);
      void sampling_rate(unsigned int val);
  };

  const unsigned long TAPE_header_LEN = 1388;
  /*! DSP::e::FileType::FT_tape : tape file header
  */
  class T_TAPE_header
  {
    public:
      unsigned int TotalSize; // length of following header // actual header size + 4
      char Filename[256];
      char CFG_filename[256];

      unsigned int SwRev;
      unsigned int HwRev;
      unsigned int file_pointer;
      unsigned int TAPE_TYPE;

      unsigned int start_time; //time_t
      unsigned int end_time;   //time_t

      unsigned int TotalSamples;
      unsigned int current_sample;

      //long long loop_start;
      unsigned int loop_start_MSW;
      unsigned int loop_start_LSW;
      //long long loop_end;
      unsigned int loop_end_MSW;
      unsigned int loop_end_LSW;
      unsigned int loop;

      unsigned int group_size_32;
      unsigned int block_size;
      unsigned int block_count;
      unsigned int fifo_size;

      char comment[256];
      char misc[20];

      unsigned int status;
      int time_stamps;

      float central_frequency;
      float cplx_datarate;

      unsigned char reserved[512];

    public:
      //! size of the TAPE file header
      unsigned int header_size(void)
      { return TotalSize + 4; };

      //! always 2 <==> short // DSP::e::SampleType::ST_short
      unsigned short sample_type(void)
      { return 0x0002; };

      //! no_of_channels always == 2
      unsigned char no_of_channels(void)
      { return 2; };

      //! \todo implement sampling rate detection
      unsigned int sampling_rate(void)
      { return 0; };
      void sampling_rate(unsigned int val);
  };
}

//! Block for connecting loose outputs
/*! If there is output which we don't need,
 *  we can connect it to this block in order
 *  to avoid error messages about unconnected
 *  outputs.
 *
 * Inputs and Outputs names:
 *   - Input: none
 *   - Output:
 *    -# "in1", "in2", ... - real or complex
 *    -# "in1.re", "in2.re", ... - (real components)\n
 *       "in1.im", "in2.im", ... - (imag components if exist)
 *    -# "in" == "in1"
 *    -# "in.re" == "in1.re"\n
 *       "in.im" == "in1.im"\n
 *
 * \test Test DSP::u::Vacuum block for multiple inputs
 *
 * \bug <b>2006.09.04</b> This block should not require any parameters.
 *   It should accept any number of output lines connected to its inputs.
 * \todo Consider two versions
 *    -# with strict option working the same way like now
 *    -# relaxed, e.g. accepting any number of connections
 *    .
 *
 */
class DSP::u::Vacuum : public DSP::Block
{
  private:
    void Init(bool AreInputsComplex, unsigned int NoOfInputs);

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    Vacuum(unsigned int NoOfInputs=1);
    Vacuum(bool AreInputsComplex, unsigned int NoOfInputs=1);
    ~Vacuum(void);
};

//! Creates object for *.wav files reading
/*! \todo_later 24-bit wav-files
 *
 *  OutputsNo - number of outputs (one channel per output)
 *
 *  If file has more channels then zeros are set to excesive outputs\n
 *  If file has less channels then execive channels are discarded
 *
 *   \note Supports reading 8-,16- and 32-bit PCM wave file
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" - real, complex or multiple-components
 *    -# "out.re" - first channel (real component)\n
 *       "out.im" - second channel (imag component)
 *    -# "out1", "out2", ... - i-th channel input
 *   - Input: none
 *
 */
class DSP::u::WaveInput : public DSP::File, public DSP::Source//: public CAudioInput
{
  private:
    //FILE *hIn;
    bool FileEnd;
    DSP::T_WAVEchunk WAVEchunk;
    string FileName;
    string FileDir;

    uint32_t ReadBufferLen;  // in bytes
    std::vector<char>  ReadBuffer;
    uint32_t AudioBufferLen;  // in bytes
    unsigned int BufferIndex;
    std::vector<DSP::Float> AudioBuffer;
    bool ConvertionNeeded;

    //! Number of bytes to read remaining in file
    uint32_t BytesRemainingInFile;
    //! number of bytes read during last file access
    uint32_t BytesRead;


    unsigned int SegmentSize;
    unsigned int SamplingRate;

    //! To be used in constructor
    bool Init(void);
    //! Closes opened WAV file
    bool CloseFile(void);
    //! Reads next file segment (returns number of samples read per channel)
    uint32_t ReadAudioSegment(void);

    /*!  if file has more channels then zeros are set to excesive outputs
     *  if file has less channels then execive channels are discarded
     *
     * \todo_later Do this on the pointer stored in the object to avoid recalculation after ReadAudioSegment
     *
     */
    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

  public:
    WaveInput(DSP::Clock_ptr ParentClock,
                  const string &FileName_in, const string &FileDir_in,
                  unsigned int OutputsNo=1); //just one channel
    ~WaveInput(void);

    bool SetSkip(long long Offset);

    DSP::File_ptr Convert2File(void)
    { return GetPointer2File(); };

    //! returns number of bytes read during last file access
    virtual unsigned int GetBytesRead(void);

    //! returns sampling rate of audio sample
    virtual long int GetSamplingRate(void);

//    void SourceDescription(TStringList *);
};


// ***************************************************** //
// ***************************************************** //
//! Multichannel file input block - sample format can be specified
/*! Inputs and Outputs names:
 *   - Output:
 *    -# "out" - real, complex or multiple-components
 *    -# "out.re" - first channel (real component)\n
 *       "out.im" - second channel (imag component)
 *    -# "out1", "out2", ... - i-th channel input
 *   - Input: none
 *
 * Supported file formats:
 *  - DSP::e::FileType::FT_raw
 *  - DSP::e::FileType::FT_flt
 *  - DSP::e::FileType::FT_wav
 *  - DSP::e::FileType::FT_tape
 *  .
 *
 * DSP::e::SampleType: DSP::e::SampleType::ST_bit and DSP::e::SampleType::ST_bit_reversed give -1.0 & +1.0 values
 */
class DSP::u::FileInput : public DSP::File, public DSP::Source
{
  private:
    DSP::e::SampleType SampleType;
    //! detected no of channels written in file
    unsigned int NoOfFileChannels;

    DSP::Float_vector Buffer;
    unsigned int BufferIndex;

    unsigned int BytesRead;
    unsigned int SamplingRate;

    // in bits (all channels together)
    //unsigned int InputSampleSize; ==> moved to DSP::File
    std::vector<uint8_t> RawBuffer;

    static bool OutputExecute_Dummy(OUTPUT_EXECUTE_ARGS);
    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

    // Variables storing headers
    //! *.flt header
    std::vector<DSP::T_FLT_header>  flt_header;
    //! *.tape header
    std::vector<DSP::T_TAPE_header> tape_header;
    //! *.wav header
    std::vector<DSP::T_WAVEchunk> wav_header;

    bool CloseFile(void);
  public:
    /*! Opens new file.
     *
     * Can be used between calls to DSP::Clock::execute
     * or in blocks' callbacks.
     */
    bool OpenFile(const string &FileName,
        DSP::e::SampleType sample_type=DSP::e::SampleType::ST_float,
        DSP::e::FileType FILEtype = DSP::e::FileType::FT_raw,
        unsigned int Default_NoOfFileChannels = 0);

    //! NoOfChannels - expected no of outputs (channels in input file)
    /*! NoOfChannels == 0 - autodetect no of channels
     */
    FileInput(DSP::Clock_ptr ParentClock,
                   const string &FileName,
                   unsigned int NoOfChannels = 1U,
                   DSP::e::SampleType sample_type=DSP::e::SampleType::ST_float,
                   DSP::e::FileType FILEtype = DSP::e::FileType::FT_raw
                  );
    ~FileInput(void);

    bool SetSkip(long long Offset);

    //! Returns pointer to the header class
    /*! If no header data are available or
     *  no header of the required type is available
     *  NULL is returned.
     *
     *  Usage:
     *   -# "*.wav"  : x = GetHeader<T_WAVEchunk>();
     *   -# "*.flt"  : x = GetHeader<T_FLT_header>();
     *   -# "*.tape" : x = GetHeader<T_TAPE_header>();
     *   .
     */
    template <class T>
    T *GetHeader(const unsigned int &index = 0);

    DSP::File_ptr Convert2File(void)
    { return GetPointer2File(); };
    //! returns number of bytes read during last file access
    unsigned int GetBytesRead(void);
    //! returns number of bytes read during last file access
    long int GetSamplingRate(void);
    //! Returns raw sample size in bytes corresponding to given SampleType
    /*! \note For SampleType = DSP::e::SampleType::ST_none returns internal raw sample size
     *   used in DSP::u::FileInput.
     *
     *  \warning Sample size is given in bits and encloses all channels
     */
    unsigned int GetSampleSize(DSP::e::SampleType SampleType = DSP::e::SampleType::ST_none);

    //! Returns raw buffer size in bytes needed for NoOfSamples samples.
    /*! If NoOfSamples == 0 return allocated internal raw buffer size.
     */
    unsigned int GetRawBufferSize(unsigned int NoOfSamples = 0);
    //! Returns DSP::Float buffer size needed for SizeInSamples samples.
    /*! If SizeInSamples == 0 return allocated internal DSP::Float buffer size.
     *
     *  \note Returned value is NoOfSamples * NoOfChannels.
     */
    unsigned int GetFltBufferSize(unsigned int NoOfSamples = 0);

    //! moves file pointer no_to_skip samples forward
    long long SkipSamples(long long no_to_skip);

    //! Reads segment for file and stores it in the buffer
    /*! \note Size of the segment read depends on the flt_buffer size.
     *
     * Returns number of read bytes.
     */
    unsigned int ReadSegmentToBuffer(
       //! Buffer where read data will be stored in DSP::Float format
       /*! \note size == buffer_size * no_of_channels
        */
       DSP::Float_vector    &flt_buffer,
       //!number of sample components leave free after each (multicomponent) sample
       /*! eg. {re0, im0, {pad}, re1, im1, {pad}, ...}
        */
       int pad_size = 0
       );
};

//! Multichannel file output block - sample format can be specified
/*! Suports
 *  - raw files: file without header file_type=DSP::e::FileType::FT_raw
 *    - sample_type=DSP::e::SampleType::ST_float
 *    - sample_type=DSP::e::SampleType::ST_scaled_float
 *    - sample_type=DSP::e::SampleType::ST_uchar
 *    - sample_type=DSP::e::SampleType::ST_short
 *    - sample_type=DSP::e::SampleType::ST_int
 *    - sample_type=DSP::e::SampleType::ST_bit_text
 *    - sample_type=DSP::e::SampleType::ST_bit
 *    - sample_type=DSP::e::SampleType::ST_bit_reversed
 *    .
 *  - PCM *.wav files: file_type=DSP::e::FileType::FT_wav
 *    - 8-bit  : sample_type=DSP::e::SampleType::ST_uchar
 *    - 16-bit : sample_type=DSP::e::SampleType::ST_short
 *    - 32-bit : sample_type=DSP::e::SampleType::ST_scaled_float
 *    .
 *  - *.flt files: file_type=DSP::e::FileType::FT_flt or DSP::e::FileType::FT_flt_no_scaling (can be read with "fileread.m" from "toolbox")
 *    - sample_type=DSP::e::SampleType::ST_float
 *    - sample_type=DSP::e::SampleType::ST_uchar
 *    - sample_type=DSP::e::SampleType::ST_short
 *    - sample_type=DSP::e::SampleType::ST_int
 *    .
 *  .
 * \note Both *.wav and *.flt files make use of sampling_rate parameter.
 *
 * Inputs and Outputs names:
 *   - Output: none
 *   - Input:
 *    -# "in" - real, complex or multiple-components
 *    -# "in.re" - first channel (real component)\n
 *       "in.im" - second channel (imag component)
 *    -# "in1", "in2", ... - i-th channel input
 *
 * \note Bit stream output (DSP::e::FileType::FT_bit and DSP::e::FileType::FT_bit_reversed)
 *  '-1.0 -> bit 0' & '+1.0 -> bit 1'
 *
 * \note If file_type is DSP::e::FileType::FT_short or DSP::e::FileType::FT_uchar input signal is limited
 *   to the range <-1.0, +1.0>
 *
 * \note NoOfChannels cannot be larger then UCHAR_MAX (255)
 *
 */
class DSP::u::FileOutput  : public DSP::File, public DSP::Block
{
  private:
    DSP::e::SampleType SampleType;
    DSP::e::FileType FileType;
    //! true if file type imposes no input scaling
    bool FileType_no_scaling;
    DSP::T_WAVEchunk WAV_header;

    //DSP::Float *Buffer;
    unsigned int BufferIndex;

    // in bits (all channel together)
    //unsigned int OutputSampleSize; ==> moved to DSP::File
    std::vector<uint8_t> RawBuffer;
    std::vector<uint8_t> TmpBuffer;

    enum E_FlushBuffer {E_FB_default = 0, E_FB_raw = 1, E_FB_update_header = 2};
    E_FlushBuffer FlushBuffer_type;

    void FlushBuffer(void);
    void raw_FlushBuffer(void);

    bool Open(const string &FileName, DSP::e::SampleType sample_type=DSP::e::SampleType::ST_float,
              unsigned int NoOfChannels=1, DSP::e::FileType file_type=DSP::e::FileType::FT_raw,
              long int sampling_rate = -1);
    void Close(void);

    //! true if file must be reopen in current clock cycle
    bool           ReOpenFile;
    string         ReOpen_FileName;
    DSP::e::SampleType ReOpen_SampleType;
    DSP::e::FileType  ReOpen_FileType;
    long int       ReOpen_sampling_rate;
    void PerformReOpen();

    //! Just ignore inputs and process block and reopen signals
    static void InputExecute_Dummy(INPUT_EXECUTE_ARGS);
    static void InputExecute_float(INPUT_EXECUTE_ARGS);
    static void InputExecute_scaled_float(INPUT_EXECUTE_ARGS);
    static void InputExecute_uchar(INPUT_EXECUTE_ARGS);
    static void InputExecute_uchar_no_scaling(INPUT_EXECUTE_ARGS);
    static void InputExecute_short(INPUT_EXECUTE_ARGS);
    static void InputExecute_short_no_scaling(INPUT_EXECUTE_ARGS);
    static void InputExecute_int(INPUT_EXECUTE_ARGS);
    static void InputExecute_int_no_scaling(INPUT_EXECUTE_ARGS);
    static void InputExecute_bit_text(INPUT_EXECUTE_ARGS);
    static void InputExecute_bit(INPUT_EXECUTE_ARGS);
    static void InputExecute_blocked(INPUT_EXECUTE_ARGS);
//    static void InputExecute(DSP::Block *block, int InputNo, DSP::Float value, DSP::Component_ptr Caller);

    //! true if file output is blocked
    bool IsBlocked;
    //! true if file output is to be blocked
    bool BlockFile;
    //! true if file output is to be unblocked
    bool UnblockFile;
    //! if source is blocked proper execute pointer is stored here
    DSP::Block_Execute_ptr Stored_Execute_ptr;
    //! Blocks or unblocks file output
    void PerformBlock(bool block);

  public:
    /*! Create object but not the file.
     * \note Use ReOpen to create file.
     */
    FileOutput(unsigned char NoOfChannels=1);
    /*! \test constant inputs must be tested
     */
    FileOutput(const string &FileName,
                   DSP::e::SampleType sample_type=DSP::e::SampleType::ST_float,
                   unsigned int NoOfChannels=1,
                   DSP::e::FileType file_type=DSP::e::FileType::FT_raw,
                   long int sampling_rate = -1);
    ~FileOutput(void);

    bool SetSkip(long long Offset);

    //! returns number of bytes read during last file access
    unsigned int GetBytesRead(void);
    //! returns sampling rate of audio sample
    long int GetSamplingRate(void);

    //! Returns raw buffer size in bytes needed for NoOfSamples samples.
    /*! If NoOfSamples == 0 return allocated internal raw buffer size.
     */
    unsigned int GetRawBufferSize(unsigned int NoOfSamples = 0);
    //! Returns raw sample size in bytes corresponding to given SampleType
    /*! \note For SampleType = DSP::e::SampleType::ST_none returns internal raw sample size
     *   used in DSP::u::FileOutput.
     *
     *  \warning Sample size is given in bits and encloses all channels
     */
    unsigned int GetSampleSize(DSP::e::SampleType SampleType = DSP::e::SampleType::ST_none);

    //! Writes segment stores in buffer to file
    /*! Returns number of written bytes.
     *
     * \note No all input sample types are supported
     *
     * \warning This function ignores file blocking state.
     */
    unsigned int WriteSegmentFromBuffer(
       //! Buffer where data which must be written is stored in DSP::Float format
       /*! \note size == buffer_size * no_of_channels
        */
       const DSP::Float_vector &flt_buffer,
       //! number of samples to skip after each written sample
       int skip = 0
       );

    //! Flushes current buffer content to file
    void Flush(void);

    //! Closes output file and opens new file with new parameters
    /*! File must be closed when all input sample
     *  in the current cycle are ready.
     *
     * \note This function only marks output file to be reopened.
     *  All samples from current cycle will be stored in the old file.
     */
    void ReOpen(const string &FileName,
                DSP::e::SampleType sample_type=DSP::e::SampleType::ST_float,
                DSP::e::FileType file_type=DSP::e::FileType::FT_raw,
                long int sampling_rate = -1);
    //! Call to block or unblock file output
    /*! \param block if true samples are ignored instead of outputing to file
     *
     * \note similar result can be achieved with the help of signal activated clock
     */
    bool BlockOutput(bool block = true);
};


//! Creates object for recording audio
/*!
 *  OutputsNo - number of outputs (one channel per output)
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" - real or complex
 *    -# "out.re" - first channel (real component)\n
 *       "out.im" - second channel (imag component if exists)
 *    -# "out1", "out2" - i-th channel output
 *   - Input: none
 *
 *  \todo_later Implement this for Win32 case / Linux ???
 *
 *  \Fixed <b>2005.07.23</b> No longer tries to process audio when audio object creation failed
 */
class DSP::u::AudioInput : public DSP::Source
{
  private:
    DSP::T_WAVEchunk WAVEchunk;
    #ifdef WINMMAPI
      HWAVEIN hWaveIn;
    #endif

    //! Index of the buffer which is expected to filled next
    short NextBufferInd;
    //! Type of samples in WaveInBuffers
    DSP::e::SampleType InSampleType;
    #ifdef WINMMAPI
      std::vector<WAVEHDR> waveHeaderIn;
    #endif
    uint32_t WaveInBufferLen;  // in bytes
    //! Buffers for audio samples prepared for playing
    std::vector<std::vector<char>> WaveInBuffers;

    //! size of the buffers used internally with WMM driver
    uint32_t audio_inbuffer_size;

    //! in samples times number of channels
    uint32_t InBufferLen;
    //! Buffer for storing samples in DSP::Float format
    DSP::Float_vector InBuffers[DSP::NoOfAudioInputBuffers];
    //! current read index in current InBuffer
    unsigned int BufferIndex;
    //! Index of the next buffer to fill
    unsigned int EmptyBufferIndex;
    //! Index of the buffer currently being read
    unsigned int CurrentBufferIndex;


//    int SegmentSize;
    int SamplingFrequency;

    //! stores parent clock in case we must stall it for some time
    DSP::Clock_ptr my_clock;

    //! To be used in constructor
    /*! \bug <b>2006.08.13</b> when 8bit audio stream is created initial values should be 0x80 or 0x79 not 0x00
     *
     *  \Fixed <b>2007.10.31</b> WaveIn device can now be selected,
     *     if WaveInDevNo is out of range WAVE_MAPPER is used.
     */
    void Init(DSP::Clock_ptr ParentClock,
              long int SamplingFreq,
              unsigned int OutputsNo=1, //just one channel
              char BitPrec=16,
              unsigned int WaveInDevNo=UINT_MAX); // use WAVE_MAPPER

    // indicates whether the recording started yet
    bool IsRecordingNow;

    bool StartAudio(void);
    bool StopAudio(void);
    bool StopRecording;
    //! (returns number of samples read per channel)
    uint32_t GetAudioSegment(void);

    static unsigned long Next_CallbackInstance;
    unsigned long Current_CallbackInstance;
    //! Addresses of audio object connected with CallbackInstances;
    /*! Current callback instanse is also the index to this array
     */
    static std::vector<DSP::u::AudioInput *> AudioObjects;

    #ifdef WINMMAPI
      static void CALLBACK waveInProc_uchar(HWAVEIN hwi, UINT uMsg,
        uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2);
      static void CALLBACK waveInProc_short(HWAVEIN hwi, UINT uMsg,
        uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2);
    #endif

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

  public:
//    DSP::u::AudioInput(DSP::Clock_ptr ParentClock); //SamplingFrequency=8000;
    /*!  \Fixed <b>2007.10.31</b> WaveIn device can now be selected,
     *     if WaveInDevNo is out of range WAVE_MAPPER is used.
     */
    AudioInput(DSP::Clock_ptr ParentClock,
                   long int SamplingFreq=8000,
                   unsigned int OutputsNo=1, //just one channel
                   char BitPrec=16,
                   unsigned int WaveInDevNo=UINT_MAX); // use WAVE_MAPPER
    ~AudioInput(void);
//    void SourceDescription(TStringList *);

    //! return No of free audio buffer
    int GetNoOfFreeBuffers(void);
    //! return No of all audio buffer
    int GetNoOfBuffers(void);
};

//! Creates object for playing audio
/*!
 *  InputsNo - number of inputs (one channel per input)
 *
 * Inputs and Outputs names:
 *   - Output: none
 *   - Input:
 *    -# "in" - real or complex
 *    -# "in.re" - first channel (real component)\n
 *       "in.im" - second channel (imag component if exists)
 *    -# "in1", "in2" - i-th channel input
 *
 *  \todo_later Implement this for Win32 case / Linux ???
 *
 * \Fixed <b>2005.07.01</b> problem when recording and playing audio
 * simultaneously: output stream is interrupted until it synchronizes
 * with input. The flaw of the current solution is increased output delay.
 *
 * \Fixed <b>2005.07.23</b> No longer tries to process audio when audio object creation failed
 */
class DSP::u::AudioOutput : public DSP::Block
{
  private:
    DSP::T_WAVEchunk WAVEchunk;
    #ifdef WINMMAPI
      HWAVEOUT hWaveOut;
    #endif

    //! Index of the buffer which must be used next time
    unsigned long NextBufferInd;
    //! Type of samples in WaveOutBuffers
    DSP::e::SampleType OutSampleType;
    #ifdef WINMMAPI
      std::vector<WAVEHDR> waveHeaderOut;
    #endif
    uint32_t WaveOutBufferLen;  // in bytes
    //! Buffers for audio samples prepared for playing
    std::vector<std::vector<uint8_t>> WaveOutBuffers;

    //! size of the buffers used internally with WMM driver
    uint32_t audio_outbuffer_size;

    //! in samples times number of channels
    uint32_t OutBufferLen;
    //! Buffer for storing samples in DSP::Float format
    DSP::Float_vector OutBuffer;
    unsigned int BufferIndex;

    //! Prepares buffers for playing and sends it to the audio device
    /*! saturation logic should be implemented */
    void FlushBuffer(void);

    bool IsPlayingNow;

    bool StartAudio(void);
    bool StopAudio(void);
    bool StopPlaying;

    static unsigned long Next_CallbackInstance;
    unsigned long Current_CallbackInstance;
    //! Addresses of audio object connected with CallbackInstances;
    /*! Current callback instanse is also the index to this array
     */
    static std::vector<DSP::u::AudioOutput *> AudioObjects;

    //! (returns number of samples read per channel)
    uint32_t GetAudioSegment(void);

    #ifdef WINMMAPI
      static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg,
        uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2);
    #endif

    void Init(unsigned long SamplingFreq,
              unsigned int InputsNo=1, //just one channel
              unsigned char BitPrec=16,
              unsigned int WaveOutDevNo=UINT_MAX);

    /*! \test Test with constant inputs
     * \Fixed <b>2007.10.31</b> WaveOut device can now be selected,
     *     if WaveOutDevNo is out of range WAVE_MAPPER is used.
     */
    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    AudioOutput(void); //SamplingFrequency=8000;
    //! DSP::u::AudioOutput constructor
    /*! \Fixed <b>2005.04.14</b> Cannot be initialized after previous object destruction
     *  \Fixed <b>2007.10.31</b> WaveOut device can now be selected,
     *     if WaveOutDevNo is out of range WAVE_MAPPER is used.
     */
    AudioOutput(unsigned long SamplingFreq, //=8000,
                    unsigned int InputsNo=1, //just one channel
                    unsigned char BitPrec=16,
                    unsigned int WaveOutDevNo=UINT_MAX);
    ~AudioOutput(void);

//    void SourceDescription(TStringList *);
};


//************************************************************//
//! Source block providing input from the memory buffer
/*! Feeds samples from the buffer to the connected blocks.
 *
 * Inputs and Outputs names:
 *   - Input: none
 *   - Output:
 *    -# "out1", "out2", ... - real
 *    -# "out.re" == "out1" - (real component)\n
 *       "out.im" == "out2" - (imag component if exist)
 *    -# "out" - all outputs together
 */
class DSP::u::InputBuffer : public DSP::Source
{
  private:
    //! callback function ID
    unsigned int UserCallbackID;
    //! callback notification function pointer
    DSP::Notify_callback_ptr NotificationFunction_ptr;
    //! Number of cycles between notifications
    int NotificationsStep;
    //! executes when notification or callback function must be processed
    void Notify(DSP::Clock_ptr clock);

    //! actual size = BufferSize*sizeof(DSP::Float)*NoOfOutputs
    long int BufferSize, BufferIndex;
    //! data are internally stored in DSP::Float format
    DSP::Float_vector Buffer;

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);
    static bool OutputExecute_single_channel(OUTPUT_EXECUTE_ARGS);
    static bool OutputExecute_cyclic(OUTPUT_EXECUTE_ARGS);
    static bool OutputExecute_cyclic_single_channel(OUTPUT_EXECUTE_ARGS);

  public:
    /*!  NotificationsStep_in - Number of cycles between notifications
     *    - if NotificationsStep_in = -1 then NotificationsStep_in = BufferSize_in
     *    .
     *
     *  If func_ptr != NULL then callback notification function is called.
     *  The calling block pointer and CallbackIdentifier are passed to this function.
     *
     *  \note apart of standard notifications, notification function
     *    will be called twice:
     *    -# at the end of block contructor processing with CallbackIdentifier = (CallbackID_signal_start | UserCallbackID)
     *    -# at the beginning of block destuctor processing with CallbackIdentifier = (CallbackID_signal_stop | UserCallbackID)
     *    .
     *
     *
     *  BufferSize_in - size of the buffer in samples (each sample with NoOfChannels components)
     *
     *
     *  Cyclic:
     *   - DSP::e::BufferType::standard - fill with zeros when full
     *   - DSP::e::BufferType::cyclic - do not reset buffer content (will output the same content again)
     *   .
     *  \note buffer content might be overwritten in notification callback
     *
     *  \warning CallbackIdentifier cannot be larger than CallbackID_mask.
     */
    InputBuffer(DSP::Clock_ptr ParentClock, int BufferSize_in,
                     unsigned int NoOfChannels=1, DSP::e::BufferType cyclic=DSP::e::BufferType::standard,
                     int NotificationsStep_in = -1, DSP::Notify_callback_ptr func_ptr = NULL,
                     unsigned int CallbackIdentifier=0);
    ~InputBuffer(void);

    //! copies source_size bytes from the source buffer to block's internal buffer
    /*! \Fixed <b>2005.03.17</b> Error in buffer size checking for multiple channels
     */
    void WriteBuffer(void *source, long int source_size, DSP::e::SampleType source_DataType=DSP::e::SampleType::ST_float);
};

//! Block providing output to the memory buffer
/*! Writes input samples from the connected blocks to the memory buffer.
 *
 * Inputs and Outputs names:
 *   - Input:
 *    -# "in1", "in2", ... - real
 *    -# "in.re" - (real component)\n
 *       "in.im" - (imag component if exist)
 *    -# "in" - all inputs together
 *   - Output:
 *    - if Callback function is not defined
 *     -# none
 *     .
 *    - if Callback function is defined
 *     -# "out" - all output lines
 *     -# "out1", "out2", ... - 1st, 2nd, ... output line
 *     .
 *    .
 *   .
 */
class DSP::u::OutputBuffer : public DSP::Block, public DSP::Source
{
  private:
    //unsigned int ind;
    //! callback function ID
    unsigned int UserCallbackID;
    //! callback notification function pointer
    DSP::Notify_callback_ptr NotificationFunction_ptr;
    //! callback function pointer
    DSP::Buffer_callback_ptr CallbackFunction_ptr;
    //! Table for output values from callback function
    DSP::Float_vector OutputsValues;
    //! if true block must output samples values from OutputsValues table
    bool OutputSamples_ready;
    //! Variable storing the user data pointer from callback function
    DSP::void_ptr UserData_ptr;


    //! Number of cycles between notifications
    /*! default: NotificationsStep == BufferSize
     *  if NotificationsStep > BufferSize
     *  {
     *    CyclesToSkip = BufferSize - NotificationsStep
     *    IsCyclic - does not have special sense
     *  }
     *  if NotificationsStep < BufferSize
     *  {
     *    CyclesToSkip = 0;
     *    if (IsCyclic == false)
     *      Buffer should be pushed instead of reset
     *  }
     */
    long NotificationsStep;
    //! executes when notification or callback function must be processed
    void Notify(DSP::Clock_ptr clock);

    //! actual size = BufferSize*sizeof(DSP::Float)*NoOfInputs
    long int BufferSize, BufferIndex;
    //! data are internally stored in DSP::Float format
    DSP::Float_vector Buffer;
    //! Buffer wraps up if TRUE
    bool IsCyclic;
    //! Buffer stops when full if TRUE
    /*! \note IsCyclic and StopWhenFull cannot be used together
     */
    bool StopWhenFull;

    static void InputExecute(INPUT_EXECUTE_ARGS);
    static void InputExecute_with_output(INPUT_EXECUTE_ARGS);
    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

    void Init(unsigned int BufferSize_in, unsigned int NoOfChannels, DSP::e::BufferType cyclic, int NotificationsStep_in);
  public:
    /*! If cyclic is true buffer is written continually
     *  after the last sample slot is filled, the first buffer slot will be
     *  filled.
     *
     *  If cyclic is false, Buffer component will ignore input samples
     *  when all buffer slots are filled. Buffer must be reset by
     *  ReadBuffer function (see reset parameter)
     *
     *  NotificationsStep_in - Number of cycles between notifications
     *   - if NotificationsStep_in = -1 then NotificationsStep_in = BufferSize_in
     *   .
     *
     *  If func_ptr != NULL then callback notification function is called.
     *  The calling block pointer and CallbackIdentifier are passed to this function.
     *
     *  \note BufferSize_in is the number of samples (each with NoOfInputs_in components)
     *    that can fit into buffer. Thus the actual buffer size is
     *    BufferSize_in * NoOfInputs_in * sizeof(DSP::Float).
     *
     *  \note apart of standard notifications, notification function
     *    will be called twice:
     *    -# at the end of block constructor processing with CallbackIdentifier = (CallbackID_signal_start | UserCallbackID)
     *    -# at the beginning of block destructor processing with CallbackIdentifier = (CallbackID_signal_stop | UserCallbackID)
     *    .
     *
     *  \warning CallbackIdentifier cannot be larger than CallbackID_mask.
     *
     *
     */
    OutputBuffer(unsigned int BufferSize_in, unsigned int NoOfInputs_in=1, DSP::e::BufferType cyclic=DSP::e::BufferType::stop_when_full,
                 DSP::Clock_ptr ParentClock = NULL, int NotificationsStep_in = -1,
                 DSP::Notify_callback_ptr func_ptr = NULL, unsigned int CallbackIdentifier=0);
    /*! Version with output lines.
     *
     *  Callback function:
     *    void func(int NoOfInputs,  DSP::Float_ptr InputSamples,
     *              int NoOfOutputs, DSP::Float_ptr OutputSamples,
     *              DSP::void_ptr *UserDataPtr, int UserDefinedIdentifier)
     *  \warning NoOfInputs will be set to the buffer inputs number, though
     *    InputSamples will always be NULL.
     *  \note Buffer MUST be read in callback function with reset set to <b>true</b>.
     */
    OutputBuffer(unsigned int BufferSize_in, unsigned int NoOfInputs_in, DSP::e::BufferType cyclic,
                 DSP::Clock_ptr ParentClock, int NotificationsStep_in, unsigned int NoOfOutputs_in,
                 DSP::Buffer_callback_ptr func_ptr, unsigned int CallbackIdentifier=0);
    //! Mixed block version with notifications controlled by given notifications clock
    /*! \note NotificationsStep is determined from
     *    ParentClock and NotificationsClock relations.
     *    If NotificationsClock is not simply M times slower then
     *    ParentClock then NotificationsStep will be set -1.
     *    In that case DSP::u::OutputBuffer::ReadBuffer with reset set to -2
     *    means the same as DSP::u::OutputBuffer::ReadBuffer with reset set to -1.
     *
     * \note If NoOfOutputs_in > 0  NotificationsClock has also the meaning of OutputClock.
     */
    OutputBuffer(unsigned int BufferSize_in, unsigned int NoOfInputs_in, DSP::e::BufferType cyclic,
                 DSP::Clock_ptr ParentClock, DSP::Clock_ptr NotificationsClock,
                 unsigned int NoOfOutputs_in, DSP::Buffer_callback_ptr func_ptr, unsigned int CallbackIdentifier=0);
    ~OutputBuffer(void);

    //! Copies dest_size bytes to the dest buffer from block's internal buffer
    /*!
     * If reset is non zero buffer state is reset after its content is copied.
     * -# buffer type: DSP::e::BufferType::standard, DSP::e::BufferType::stop_when_full
     *  - reset == 0; - no buffer reseting
     *  - reset == -1; - full buffer reset
     *  - reset > 0;  - free only reset slots in buffer
     *  - reset == -2; - free just NotificationsStep slots in buffer
     * -# buffer type: DSP::e::BufferType::cyclic
     *  - reset == 0; - no buffer reseting
     *  - reset != 0; - buffer index reset. Buffer is filled from the beginning
     *   but buffer slots are not set to zero.
     * .
     * reset
     *
     * Returns number of samples read (not bytes !!!)
     * If buffer is cyclic this depends on dest_size or BufferSize.
     * If buffer is NOT cyclic this depends on number of samples in Buffer.
     */
    long int ReadBuffer(void *dest, long int dest_size,
                        long int reset=-1, DSP::e::SampleType dest_DataType=DSP::e::SampleType::ST_float);
    //! low level access to the buffer
    const DSP::Float_vector &AccessBuffer(void);

    //! returns number of samples already in the buffer
    /*! \note Actual number of entries equals returned value
     *   times number of inputs
     */
    long int NoOfSamples(void);

    //! returns size of the buffer
    /*! Returned value depends on mode
     * - mode == 0: number of samples
     *   \note Actual number of entries equals returned value
     *   times number of inputs
     * - mode == 1: number of DSP::Float entries
     * - mode == 2: number of bytes
     * .
     */
    long int GetBufferSize(int mode = 0);
};




#endif
