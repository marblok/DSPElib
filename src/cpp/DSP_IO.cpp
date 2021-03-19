/*! \file DSP_IO.cpp
 * This is Input/Output DSP module header file.
 *
 * Module blocks providing input and output from external devices
 * like hard drive or sound card.
 *
 * \author Marek Blok
 */

#include <DSP_clocks.h>
#include <DSP_IO.h>
#include <algorithm>
#include <string>
#include <limits.h>
#include <typeinfo>
#include <cstdint> // int16_t, itp.

#ifdef WINMMAPI
//  #include <stdarg.h>
//  #include <winbase.h>  // provides Sleep()
//  #include <winuser.h>  // provides WaitMessage()

  #include <windows.h>  // provides WaitMessage()
#endif

//#define AUDIO_DEBUG_MESSAGES_ON
//#define VerboseCompilation

//*****************************************************//
//*****************************************************//
DWORD DSP::f::ReadCoefficientsFromFile(DSP_float *Buffer, DWORD N,
                     const string &FileName, const string &FileDir,
                     DSP::e::SampleType type,
                     DWORD offset)
{
  FILE *plik;
  uint8_t *tempBuffer;
  string file_name;
  DWORD ValuesRead;
  unsigned int ind;

  file_name = FileDir;
  if (file_name.length() > 0)
    if ((file_name.back() != '/') || (file_name.back() != '\\'))
    { file_name += '/'; }
  file_name += FileName;

  ValuesRead=0;
  plik=fopen(file_name.c_str(), "rb");
  if (plik != NULL)
  {
    if (fseek(plik, offset, SEEK_SET)==0)
    {
      switch (type)
      {
        case DSP::e::SampleType::ST_uchar:
          //reading
          tempBuffer=(uint8_t *)new uint8_t[N];
          ValuesRead=(DWORD)fread(tempBuffer, sizeof(uint8_t), N, plik);
          //convertion
          for (ind=0; ind<ValuesRead; ind++)
            Buffer[ind]=(((DSP_float)((uint8_t  *)tempBuffer)[ind]-80)
                        )/0x80;
          //clean up
          delete []tempBuffer;
          break;
        case DSP::e::SampleType::ST_short:
          //reading
          tempBuffer=(uint8_t *)new short[N];
          ValuesRead=(DWORD)fread(tempBuffer, sizeof(short), N, plik);
          //convertion
          for (ind=0; ind<ValuesRead; ind++)
            Buffer[ind]=((DSP_float)((short *)tempBuffer)[ind])
                        /0x8000;
          //clean up
          delete []tempBuffer;
          break;
        case DSP::e::SampleType::ST_float:
          //reading
          tempBuffer=(uint8_t *)new float [N];
          ValuesRead=(DWORD)fread(tempBuffer, sizeof(float), N, plik);
          //convertion
          for (ind=0; ind<ValuesRead; ind++)
            Buffer[ind]=((float *)tempBuffer)[ind];
          //clean up
          delete []tempBuffer;
          break;
        default:
          ValuesRead=0;
          break;
      }
    }
    #ifdef __DEBUG__
      else
      {
        DSP::log << DSP::LogMode::Error << "DSP::f::ReadCoefficientsFromFile" << DSP::LogMode::second << "ReadCoefficientsFromFile wrong offset" << endl;
      }
    #endif
    fclose(plik);
  }
  return ValuesRead;
}

//*****************************************************//
//*****************************************************//
//    unsigned char  version;        1B
unsigned char T_FLT_header::version(void)
{
  return data[0];
}
void T_FLT_header::version(unsigned char val)
{
  data[0] = val;
}
//    unsigned short sample_type;    2B
unsigned short T_FLT_header::sample_type(void)
{
  return *((unsigned short *)(data + 1));
}
void T_FLT_header::sample_type(unsigned short val)
{
  *((unsigned short *)(data+1)) = val;
}
//    unsigned char  no_of_channels; 1B
unsigned char T_FLT_header::no_of_channels(void)
{
  return data[3];
}
void T_FLT_header::no_of_channels(unsigned char val)
{
  data[3] = val;
}
//    unsigned int   sampling_rate;  4B
unsigned int T_FLT_header::sampling_rate(void)
{
  return *((unsigned int *)(data + 4));
}
void T_FLT_header::sampling_rate(unsigned int val)
{
  *((unsigned int *)(data+4)) = val;
}

//*****************************************************//
//*****************************************************//
DSPu_Vacuum::DSPu_Vacuum(unsigned int NoOfInputs_in)
  : DSP::Block()
{
  Init(false, NoOfInputs_in);

  Execute_ptr = &InputExecute;
}

DSPu_Vacuum::DSPu_Vacuum(bool AreInputsComplex, unsigned int NoOfInputs_in)
  : DSP::Block()
{
  Init(AreInputsComplex, NoOfInputs_in);

  Execute_ptr = &InputExecute;
}

void DSPu_Vacuum::Init(bool AreInputsComplex, unsigned int NoOfInputs_in)
{
  unsigned int ind;
  string temp;

  SetName("Vacuum", false);

  //if (NoOfInputs <=0)
  //  NoOfInputs=1;

  if (AreInputsComplex == false)
  {
    SetNoOfInputs(NoOfInputs_in,false);
    // meaning of "in" should depend on NoOfInputs
    //DefineInput("in", 0);
    if (NoOfInputs_in == 1) {
      DefineInput("in.re", 0);
    }
  }
  else
  {
    SetNoOfInputs(2*NoOfInputs_in,false);
    // meaning of "in" should depend on NoOfInputs
    //DefineInput("in", 0, 1);
    if (NoOfInputs_in == 1) {
      DefineInput("in.re", 0);
      DefineInput("in.im", 1);
    }
  }

  vector <unsigned int> all_indexes;
  for (ind=0; ind<NoOfInputs_in; ind++)
  {
    all_indexes.push_back(ind);

    if (AreInputsComplex == false)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, ind);
      temp = "in" + to_string(ind+1) + ".re";
      DefineInput(temp, ind);
    }
    else
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, 2*ind, 2*ind+1);
      temp = "in" + to_string(ind+1) + ".re";
      DefineInput(temp, 2*ind);
      temp = "in" + to_string(ind+1) + ".im";
      DefineInput(temp, 2*ind+1);
    }
  }
  DefineInput("in", all_indexes);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
}

DSPu_Vacuum::~DSPu_Vacuum()
{
}

void DSPu_Vacuum::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_ARGUMENT(value);
  UNUSED_ARGUMENT(block);
  UNUSED_DEBUG_ARGUMENT(Caller);
}

//*****************************************************//
//*****************************************************//
// Determines file type by filename extension
DSP::e::FileType DSP::f::FileExtToFileType(const string &filename)
{
  DSP::e::FileType FileType;
  int ind;
  string ext;

  FileType = DSP::e::FileType::FT_raw;

  // 1. Check if there is an extension
  // - start from the end
  ext = "";
  for (ind = int(filename.length())-1; ind >= 0; ind--)
  {
    if (filename[ind] == '.')
    {
      //ext = filename + ind + 1;
      ext = filename.substr(ind+1);
      break;
    }
  }

  if (ext.length() > 0)
  {
    string upper_ext = ext;
    std::transform(upper_ext.begin(), upper_ext.end(),upper_ext.begin(), ::toupper);
    if (upper_ext.compare("FLT") == 0)
      FileType = DSP::e::FileType::FT_flt;
    if (upper_ext.compare("WAV") == 0)
      FileType = DSP::e::FileType::FT_wav;
    if (upper_ext.compare("TAPE") == 0)
      FileType = DSP::e::FileType::FT_tape;
  }

  return FileType;
}

//*****************************************************//
//*****************************************************//
DSPu_FILEinput::DSPu_FILEinput(DSP::Clock_ptr ParentClock,
                  const string &FileName,
                  unsigned int NoOfChannels,
                  DSP::e::SampleType sample_type,
                  DSP::e::FileType FILEtype)
  : DSP::File(), DSP::Source()
{
  string temp;
  bool ready;

  SetName("FILEinput", false);

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  RawBuffer = NULL;
  flt_header = NULL; tape_header = NULL; wav_header = NULL;
  FileHandle = NULL;

  ready = OpenFile(FileName, sample_type, FILEtype, NoOfChannels);

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  if (NoOfChannels == 0)
  {
    if (FILEtype == DSP::e::FileType::FT_raw)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second
          << "Detection of no of channels is not supported for DSP::e::FileType::FT_raw (defaulting to 1 channels)" << endl;
        NoOfChannels = 1;
      #endif
    }
    else
    {
      if (NoOfFileChannels > 0)
        NoOfChannels = NoOfFileChannels;
      else
        NoOfChannels = 1;
    }
  }
  SetNoOfOutputs(NoOfChannels); // NoOfOutputs = NoOfChannels;

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  //Data are stored in buffer in DSP_float format
  Buffer = new DSP_float [NoOfOutputs*DSP_file_buffer_size];
  BufferIndex=0; //this means also that Buffer is empty
  memset(Buffer, 0, NoOfOutputs*sizeof(DSP_float)*DSP_file_buffer_size);
  // +++++++++++++++++++++++++++++++++++++++++++++ //


  if ((FILEtype == DSP::e::FileType::FT_tape) &&  (NoOfOutputs != 2))
  {
    //NoOfChannels = 2;
    //! \bug support for reading files with different number of channels then number of block's outputs
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second << "Wrong number of channels for *.tape file !!!" << endl;
    #endif
    ready = false;
  }

  if (NoOfOutputs != NoOfFileChannels)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second
        << "Incorrect number of input channels: expected "
        << NoOfOutputs << " but found " << NoOfFileChannels << "!!!" << endl;
    #endif
    //! \bug support for reading files with different number of channels then number of block's outputs
    ready = false;
  }

  // NoOfChannels ==> NoOfOutputs
  if (NoOfOutputs == 1)
  {
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
    DefineOutput("out1", 0);
  }
  if (NoOfOutputs == 2)
  {
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    DefineOutput("out1", 0);
    DefineOutput("out2", 1);
  }
  if (NoOfOutputs > 2)
  {
    unsigned int ind;
    vector <unsigned int> tempOut;
    tempOut.resize(NoOfOutputs);
    for (ind=0; ind<NoOfOutputs; ind++)
      tempOut[ind]=ind;
    DefineOutput("out", tempOut);

    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    for (ind=0; ind<NoOfOutputs; ind++)
    {
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, ind);
    }
  }

  if (ParentClock != NULL)
    RegisterOutputClock(ParentClock);

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  if (ready == false) {
    DSP::log << DSP::LogMode::Error << "DSPu_FILEinput" << DSP::LogMode::second
      << "Block <" << GetName() << "> failed to open file \"" << FileName << "\"" << endl;
    OutputExecute_ptr = &OutputExecute_Dummy;
  }
  else
    OutputExecute_ptr = &OutputExecute;
}

bool DSPu_FILEinput::SetSkip(long long Offset)
{
  UNUSED_ARGUMENT(Offset);

  #ifdef __DEBUG__
    DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::SetSkip" << DSP::LogMode::second << "not implemented yet" << endl;
  #endif
  return false;
}

bool DSPu_FILEinput::OpenFile(const string &FileName,
    DSP::e::SampleType sample_type, DSP::e::FileType FILEtype,
    unsigned int Default_NoOfChannels)
{
  bool ready = true;

  CloseFile();

  SampleType=sample_type;

  //Open file
  FileHandle=fopen(FileName.c_str(), "rb");
  SamplingRate = 0;
  NoOfFileChannels = 0;
  if (FileHandle != NULL)
  {
    switch (FILEtype)
    {
      case DSP::e::FileType::FT_flt:
        {
          flt_header = new T_FLT_header;
          BytesRead = (unsigned int)fread(flt_header, 1, FLT_header_LEN, FileHandle);

          if (flt_header->version() != 0)
          {
            ready = false;

            #ifdef __DEBUG__
              DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second << "Unsupported *.flt file version !!!" << endl;
              //! \todo verify if file header data match constructor parameters
            #endif
          }

          NoOfFileChannels = flt_header->no_of_channels();
          SamplingRate = flt_header->sampling_rate();

          //! \TODO determine sample type based on flt_header->sample_type()
        }
        break;

      case DSP::e::FileType::FT_tape:
        {
          tape_header = new T_TAPE_header;
          BytesRead = (unsigned int)fread(tape_header, 1, TAPE_header_LEN, FileHandle);

          NoOfFileChannels = 2;
          if (SampleType != DSP::e::SampleType::ST_short)
          {
            SampleType = DSP::e::SampleType::ST_short;
            ready = false;

            #ifdef __DEBUG__
              DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second << "Unsupported *.tape file sample type!!!" << endl;
            #endif
          }

          #ifdef __DEBUG__
            if (sizeof(T_TAPE_header) != TAPE_header_LEN)
              DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second << "TAPE_header_LEN does not much sizeof T_TAPE_header structure !!!" << endl;
            if (tape_header->header_size() != TAPE_header_LEN)
              DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second << "Unsupported *.tape file version !!!" << endl;
          #endif

          SamplingRate = tape_header->sampling_rate();
        }
        break;

      case DSP::e::FileType::FT_wav:
        {
          wav_header = new T_WAVEchunk;

          //Read WAVE file header
          if (wav_header->WAVEinfo(FileHandle)==false)
          {
            ready = false;

            #ifdef __DEBUG__
            {
              DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second
                << "This (" << FileName << ") is not PCM WAVE file or file is corrupted !!!" << endl;
            }
            #endif
          }

          NoOfFileChannels = wav_header->nChannels;

          switch (wav_header->wBitsPerSample)
          {
            case 8:
              SampleType = DSP::e::SampleType::ST_uchar;
              break;
            case 16:
              SampleType = DSP::e::SampleType::ST_short;
              break;
            case 32:
              SampleType = DSP::e::SampleType::ST_scaled_float;
              break;
            case 24:
            default:
              ready = false;
              #ifdef __DEBUG__
              {
                DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second
                  << "Unsupported PCM sample size ==> " << wav_header->wBitsPerSample << " bits!!!" << endl;
              }
              #endif
              break;
          }

          SamplingRate = wav_header->nSamplesPerSec;

          if (wav_header->FindDATA(FileHandle)==false)
          {
            ready = false;
            #ifdef __DEBUG__
            {
              DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::DSPu_FILEinput" << DSP::LogMode::second
                << "No data have been found in PCM file " << FileName << "!!!" << endl;
            }
            #endif
          }

        }
        break;

      case DSP::e::FileType::FT_raw:
      default:
        // no header thus do nothing
        break;
    }
  }
  else
  {
    ready = false;
  }

  if (NoOfFileChannels == 0)
  {
    if (Default_NoOfChannels != 0)
      NoOfFileChannels = Default_NoOfChannels;
    else
      NoOfFileChannels = 1;
  }

  if (RawBuffer != NULL)
    delete [] RawBuffer;

  SampleSize = GetSampleSize(SampleType);
  RawBuffer = new uint8_t[(SampleSize*DSP_file_buffer_size+7)/8 + 1];
  BytesRead=0;

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  if (ready == false)
    OutputExecute_ptr = &OutputExecute_Dummy;
  else
    OutputExecute_ptr = &OutputExecute;

  return ready;
}

bool DSPu_FILEinput::CloseFile(void)
{
  int res = -1;

  if (flt_header != NULL)
  {
    delete flt_header;
    flt_header = NULL;
  }
  if (tape_header != NULL)
  {
    delete tape_header;
    tape_header = NULL;
  }
  if (wav_header != NULL)
  {
    delete wav_header;
    wav_header = NULL;
  }

  if (RawBuffer != NULL)
  {
    delete [] RawBuffer;
    RawBuffer = NULL;
  }
  if (FileHandle != NULL)
  {
    res = fclose(FileHandle);
    FileHandle = NULL;
  }

  return (res == 0);
}

//! needed to get function DSPu_FILEinput::GetHeader instances into library
void dummy_GetHeader(void)
{
  DSPu_FILEinput *temp = NULL;

  temp->GetHeader<T_TAPE_header>();
  temp->GetHeader<T_FLT_header>();
  temp->GetHeader<T_WAVEchunk>();
}

template <class T>
T *DSPu_FILEinput::GetHeader(void)
{
  T *temp;

  if (typeid(temp) == typeid(flt_header))
    return (T *)flt_header;
  if (typeid(temp) == typeid(wav_header))
    return (T *)wav_header;
  if (typeid(temp) == typeid(tape_header))
    return (T *)tape_header;
  return NULL;
}
template T_FLT_header *DSPu_FILEinput::GetHeader<T_FLT_header>(void);
template T_TAPE_header *DSPu_FILEinput::GetHeader<T_TAPE_header>(void);
template T_WAVEchunk *DSPu_FILEinput::GetHeader<T_WAVEchunk>(void);



unsigned int DSPu_FILEinput::GetSampleSize(DSP::e::SampleType SampleType_in)
{
  unsigned int sample_size;

  switch (SampleType_in)
  {
    case DSP::e::SampleType::ST_float:
      sample_size=8*sizeof(float);
      break;
    case DSP::e::SampleType::ST_uchar:
      sample_size=8*sizeof(unsigned char);
      break;
    case DSP::e::SampleType::ST_tchar:
      sample_size = 8 * sizeof(char);
      break;
    case DSP::e::SampleType::ST_bit_reversed:
    case DSP::e::SampleType::ST_bit:
      // In this case its number of bits (raw samples) per output sample
      sample_size=1;
      if ((DSP_file_buffer_size % 8) != 0)
      {
        DSP::log << DSP::LogMode::Error << "DSPu_FILEinput" << DSP::LogMode::second << "Can't read bit stream corectly:"
          " DSP_file_buffer_size is not a multiply of byte size (8 bits)" << endl;
      }
      break;
    case DSP::e::SampleType::ST_short:
      sample_size=8*sizeof(short);
      break;
    case DSP::e::SampleType::ST_none: // return internal sample size used in DSPu_FILEinput
      sample_size = SampleSize/NoOfFileChannels;
      break;
    default:
      sample_size=8*sizeof(short);
      DSP::log << DSP::LogMode::Error << "DSPu_FILEinput" << DSP::LogMode::second << "Unsupported data type" << endl;
      break;
  }
  sample_size*=NoOfFileChannels;

  return sample_size;
}

DSPu_FILEinput::~DSPu_FILEinput(void)
{
//  SetNoOfOutputs(0);
  CloseFile();

  if (Buffer != NULL)
    delete [] Buffer;
}

// returns number of bytes read during last file access
unsigned int DSPu_FILEinput::GetBytesRead(void)
{
  return BytesRead;
}
// returns sampling rate of audio sample
long int DSPu_FILEinput::GetSamplingRate(void)
{
  return SamplingRate;
}

// Returns raw buffer size in bytes needed for NoOfSamples samples.
/* If NoOfSamples == 0 return allocated internal raw buffer size.
 * \note FlushBuffer requires one additional byte bot bit modes
 */
unsigned int DSPu_FILEinput::GetRawBufferSize(unsigned int NoOfSamples)
{
  if (NoOfSamples == 0)
    return (SampleSize * DSP_file_buffer_size + 7)/8;
  else
    return (SampleSize * NoOfSamples + 7)/8;
}

// Returns DSP_float buffer size needed for SizeInSamples samples.
/* If SizeInSamples == 0 return allocated internal DSP_float buffer size.
 *
 *  \note Returned value is NoOfSamples * NoOfChannels.
 */
unsigned int DSPu_FILEinput::GetFltBufferSize(unsigned int NoOfSamples)
{
  if (NoOfSamples == 0)
    return (NoOfOutputs * DSP_file_buffer_size);
  else
    return (NoOfOutputs * NoOfSamples);
}

//! moves file pointer no_to_skip samples forward
long long DSPu_FILEinput::SkipSamples(long long no_to_skip)
{
  long long no_of_bytes_to_skip;

  if (FileHandle != NULL)
  {
    no_of_bytes_to_skip = (no_to_skip * SampleSize)/8;

//    if (fseeko64(FileHandle, no_of_bytes_to_skip, SEEK_CUR) == 0)
    if (fseek(FileHandle, long(no_of_bytes_to_skip), SEEK_CUR) == 0)
      return no_to_skip;
  }

  return 0;
}

unsigned int DSPu_FILEinput::ReadSegmentToBuffer(
    unsigned int buffer_size, // in samples (raw_buffer_size == buffer_size * sample_size / 8)
    uint8_t *raw_buffer,
    DSP_float   *flt_buffer,
    int pad_size)
{
  DSP_float *tempBuffer;
  uint8_t *tempUChar;
  //short *tempShort;
  int16_t *tempShort;
  //int *tempInt;
  int32_t *tempInt;
  float *tempFloat;
  unsigned int ind, ind2;
  unsigned char mask;

  //! \TODO in input/output operations use int16_t and int32_t instead short and int

  /*! \todo <b>27.07.2008</b> Check first if there is something to read
   *  in DSPu_FILEinput internal buffer. If yes then use it before
   *  reading the file.
   */
  if (FileHandle != NULL)
    BytesRead =
        (unsigned int)fread(raw_buffer, 1, (SampleSize * buffer_size)/8, FileHandle);
  else
    BytesRead=0;

  if (BytesRead <
      ((SampleSize * buffer_size)/8))
  { //clear the end of the buffer if neccessary
    memset(raw_buffer + BytesRead, 0,
           (SampleSize * buffer_size)/8 - BytesRead);
  }

  //Now we need to convert data from RawBuffer
  //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
  tempBuffer = flt_buffer;
  switch (SampleType)
  {
    case DSP::e::SampleType::ST_bit:
      tempUChar=(uint8_t *)raw_buffer;
      mask=0x80;
      for (ind=0; ind<buffer_size; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer = (((*tempUChar) & mask) != 0) ? DSP_float(+1.0) : DSP_float(-1.0);
          tempBuffer++;

          //mask >>= 1;
          mask = (unsigned char)(mask >> 1);

          if (mask == 0)
          {
            mask = 0x80;
            tempUChar++;
          }
        }
        tempBuffer += pad_size;
      }
      break;
    case DSP::e::SampleType::ST_bit_reversed:
      tempUChar=(uint8_t *)raw_buffer;
      mask=0x01;
      for (ind=0; ind<buffer_size; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer = (((*tempUChar) & mask) != 0) ? DSP_float(+1.0) : DSP_float(-1.0);
          tempBuffer++;

          //mask <<= 1;
          mask = (unsigned char)(mask << 1);

          if (mask == 0)
          {
            mask = 0x01;
            tempUChar++;
          }
        }
        tempBuffer += pad_size;
      }
      break;
    case DSP::e::SampleType::ST_bit_text:
      tempUChar=(uint8_t *)raw_buffer;
      for (ind=0; ind<buffer_size; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer=(*tempUChar == '0')?-1.0f:+1.0f;
          tempBuffer++;
          tempUChar++;
        }
        tempBuffer += pad_size;
      }
      break;
    case DSP::e::SampleType::ST_uchar:
      tempUChar=(uint8_t *)raw_buffer;
      for (ind=0; ind<buffer_size; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer=DSP_float(*tempUChar-0x80)/0x80;
          tempBuffer++;
          tempUChar++;
        }
        tempBuffer += pad_size;
      }
      break;
    case DSP::e::SampleType::ST_short:
      tempShort=(short *)raw_buffer;
      for (ind=0; ind<buffer_size; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer=DSP_float(*tempShort)/0x8000;
          tempBuffer++;
          tempShort++;
        }
        tempBuffer += pad_size;
      }
      break;
    case DSP::e::SampleType::ST_int:
      tempInt=(int *)raw_buffer;
      for (ind=0; ind<buffer_size; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer=DSP_float(*tempInt);
          tempBuffer++;
          tempInt++;
        }
        tempBuffer += pad_size;
      }
      break;
    case DSP::e::SampleType::ST_float:
      tempFloat=(float *)raw_buffer;
      for (ind=0; ind<buffer_size; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer=*tempFloat;
          tempBuffer++;
          tempFloat++;
        }
        tempBuffer += pad_size;
      }
      break;
    default:
      break;
  }

  return BytesRead;
}

#define DSP_THIS ((DSPu_FILEinput *)source)
// Input file could not be open so output zeros
bool DSPu_FILEinput::OutputExecute_Dummy(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  unsigned int ind2;

  for (ind2=0; ind2 < DSP_THIS->NoOfOutputs; ind2++)
  {
    DSP_THIS->OutputBlocks[ind2]->EXECUTE_PTR(
        DSP_THIS->OutputBlocks[ind2], DSP_THIS->OutputBlocks_InputNo[ind2],
        0.0, source);
  }
  return true;
}
// returns true if source is ready
bool DSPu_FILEinput::OutputExecute(OUTPUT_EXECUTE_ARGS)
{ // we assume only one output
  UNUSED_DEBUG_ARGUMENT(clock);
  unsigned int ind2;

  if (DSP_THIS->BufferIndex == 0)
  { // Data must be read from file to buffer
    DSP_THIS->ReadSegmentToBuffer(DSP_file_buffer_size,
        DSP_THIS->RawBuffer, DSP_THIS->Buffer);
  }
  for (ind2=0; ind2 < DSP_THIS->NoOfOutputs; ind2++)
  {
//    OutputBlocks[ind2]->Execute(OutputBlocks_InputNo[ind2],
//                                Buffer[BufferIndex], this);
    DSP_THIS->OutputBlocks[ind2]->EXECUTE_PTR(
        DSP_THIS->OutputBlocks[ind2], DSP_THIS->OutputBlocks_InputNo[ind2],
        DSP_THIS->Buffer[DSP_THIS->BufferIndex], source);
    DSP_THIS->BufferIndex++;
  }
  DSP_THIS->BufferIndex %= (DSP_file_buffer_size * DSP_THIS->NoOfOutputs);

  return true;
}
#undef DSP_THIS

//*****************************************************//
//*****************************************************//
DSPu_FILEoutput::DSPu_FILEoutput(unsigned char NoOfChannels) : DSP::Block()
{
  string temp;
  ReOpen_FileType = DSP::e::FileType::FT_raw;
  ReOpen_sampling_rate = 0;
  ReOpen_SampleType = DSP::e::SampleType::ST_none;
  SampleType = DSP::e::SampleType::ST_none;

  FileType_no_scaling = true;
  FileType = DSP::e::FileType::FT_raw;
  FlushBuffer_type = E_FB_default;

  BufferIndex = 0;

  SetName("FILEoutput", false);
  SetNoOfOutputs(0);
  if (NoOfChannels == 0)
    NoOfChannels=1;
  SetNoOfInputs(NoOfChannels, 0, true);
  if (NoOfChannels == 1)
  {
    DefineInput("in", 0);
    DefineInput("in.re", 0);
    DefineInput("in1", 0);
  }
  if (NoOfChannels == 2)
  {
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineInput("in1", 0);
    DefineInput("in2", 1);
  }
  if (NoOfChannels > 2)
  {
    unsigned int ind;
    vector <unsigned int> tempOut;
    tempOut.resize(NoOfChannels);
    for (ind=0; ind<NoOfChannels; ind++)
      tempOut[ind]=ind;
    DefineInput("in", tempOut);

    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    for (ind=0; ind<NoOfChannels; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, ind);
    }
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);

  ReOpenFile = false;
  ReOpen_FileName[0] = 0;

  TmpBuffer = NULL;
  RawBuffer = NULL;
  FileHandle = NULL;
  Execute_ptr = &InputExecute_blocked;

  Stored_Execute_ptr = NULL;
  IsBlocked = false;
  BlockFile = false;
  UnblockFile = false;
}

DSPu_FILEoutput::DSPu_FILEoutput(const string &FileName,
                DSP::e::SampleType sample_type,
                unsigned int NoOfChannels,
                DSP::e::FileType file_type, long int sampling_rate)
  : DSP::Block()
{
  string temp;

  SetName("FILEoutput", false);
  SetNoOfOutputs(0);
  if (NoOfChannels == 0)
    NoOfChannels=1;
  SetNoOfInputs(NoOfChannels, 0, true);
  if (NoOfChannels == 1)
  {
    DefineInput("in", 0);
    DefineInput("in.re", 0);
    DefineInput("in1", 0);
  }
  if (NoOfChannels == 2)
  {
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineInput("in1", 0);
    DefineInput("in2", 1);
  }
  if (NoOfChannels > 2)
  {
    unsigned int ind;
    vector<unsigned int> tempOut;
    tempOut.resize(NoOfChannels);
    for (ind=0; ind<NoOfChannels; ind++)
      tempOut[ind]=ind;
    DefineInput("in", tempOut);

    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    for (ind=0; ind<NoOfChannels; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, ind);
    }
  }
//  IsMultiClock=false;

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);

  Open(FileName, sample_type, NoOfChannels, file_type, sampling_rate);
  ReOpenFile = false;
  ReOpen_FileName[0] = 0;

  Stored_Execute_ptr = NULL;
  IsBlocked = false;
  BlockFile = false;
  UnblockFile = false;
}

//! returns number of bytes read during last file access
unsigned int DSPu_FILEoutput::GetBytesRead(void)
{
  return 0;
}
//! returns sampling rate of audio sample
long int DSPu_FILEoutput::GetSamplingRate(void)
{
  return ReOpen_sampling_rate;
}

bool DSPu_FILEoutput::Open(const string &FileName, DSP::e::SampleType sample_type, unsigned int NoOfChannels,
                           DSP::e::FileType file_type, long int sampling_rate)
{
  FileType = file_type;
  SampleType=sample_type;
  // //Data are stored in buffer in DSP_float format
  // Buffer = new DSP_float [DSP_file_buffer_size*NoOfInputs];
  // BufferIndex=0; //this means also that Buffer is empty
  // memset(Buffer, 0, NoOfInputs*sizeof(DSP_float)*DSP_file_buffer_size);

  if (NoOfChannels > UCHAR_MAX)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput::Open" << DSP::LogMode::second << "NoOfChannels to large (> UCHAR_MAX)" << endl;
    #endif
    NoOfChannels = UCHAR_MAX;
  }
  TmpBuffer = NULL;
  RawBuffer = NULL;
  FlushBuffer_type = E_FB_default;

  FileType_no_scaling = true;
  SampleSize = GetSampleSize(SampleType);
  switch (SampleType)
  {
    case DSP::e::SampleType::ST_float:
      Execute_ptr = &InputExecute_float;
      FlushBuffer_type = E_FB_raw;
      break;
    case DSP::e::SampleType::ST_scaled_float:
      FileType_no_scaling = false;
      Execute_ptr = &InputExecute_scaled_float;
      FlushBuffer_type = E_FB_raw;
      break;
    case DSP::e::SampleType::ST_uchar:
      Execute_ptr = &InputExecute_uchar;
      FlushBuffer_type = E_FB_raw;
      break;
    case DSP::e::SampleType::ST_tchar:
      Execute_ptr = &InputExecute_uchar_no_scaling;
      FlushBuffer_type = E_FB_raw;
      break;
    case DSP::e::SampleType::ST_bit_text:
      Execute_ptr = &InputExecute_bit_text;
      FlushBuffer_type = E_FB_raw;
      break;
    case DSP::e::SampleType::ST_short:
      Execute_ptr = &InputExecute_short;
      FlushBuffer_type = E_FB_raw;
      break;
    case DSP::e::SampleType::ST_int:
      Execute_ptr = &InputExecute_int;
      FlushBuffer_type = E_FB_raw;
      break;
    case DSP::e::SampleType::ST_bit:
    case DSP::e::SampleType::ST_bit_reversed:
      if ((DSP_file_buffer_size % 8) != 0)
      {
        DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput" << DSP::LogMode::second << "Can't write bit stream correctly:"
          " DSP_file_buffer_size is not a multiply of byte size (8 bits)" << endl;
      }
      // "0 / 1" byte by byte
      TmpBuffer = new unsigned char[SampleSize*DSP_file_buffer_size];

      Execute_ptr = &InputExecute_bit; // FlushBuffer is used
      FlushBuffer_type = E_FB_default;
      break;
    default:
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput" << DSP::LogMode::second << "Unsupported data type changing to DSP::e::SampleType::ST_short" << endl;
      #endif
      SampleType = DSP::e::SampleType::ST_short;
      Execute_ptr = &InputExecute_short;
      FlushBuffer_type = E_FB_raw;
      break;
  }
  RawBuffer = new uint8_t[(SampleSize*DSP_file_buffer_size+7)/8 + 1];
  BufferIndex=0; //this means also that Buffer is empty

  //Open file
  FileHandle=fopen(FileName.c_str(), "wb");

  if (FileHandle != NULL)
  {
    switch (FileType)
    {
      case DSP::e::FileType::FT_wav:
        switch (SampleType)
        {
          case DSP::e::SampleType::ST_scaled_float: // 32bit
          case DSP::e::SampleType::ST_short: // 16bit
          case DSP::e::SampleType::ST_uchar: // 8bit
            FileType_no_scaling = false;
            break;
          default:
            #ifdef  __DEBUG__
              DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput" << DSP::LogMode::second << "*.wav: unsupported sample type" << endl;
            #endif
            break;
        }

        WAV_header.PrepareHeader((DWORD)sampling_rate, (WORD)NoOfInputs, (WORD)(SampleSize/NoOfInputs));
        WAV_header.WriteHeader(FileHandle);

        FlushBuffer_type = E_FlushBuffer(FlushBuffer_type | E_FB_update_header);
        break;

/*
      case DSP::e::FileType::FT_tape:
        #ifdef  __DEBUG__
          switch (SampleType)
          {
            case DSP::e::SampleType::ST_short: // 16bit
              break;
            default:
              DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput", "*.tape: unsupported sample type");
              break;
          }
        #endif
        break;
 */
      case DSP::e::FileType::FT_flt:
        FileType_no_scaling = false;
        //no break
        // fall through
      case DSP::e::FileType::FT_flt_no_scaling:
        {
          T_FLT_header header_flt;
          header_flt.version(0x00);
          header_flt.sample_type(0xffff);
          header_flt.sampling_rate((unsigned int)sampling_rate);
          header_flt.no_of_channels((unsigned char)NoOfChannels);

          switch (SampleType)
          {
            case DSP::e::SampleType::ST_float:
              header_flt.sample_type(0x0000); // floating point: float (4B)
              break;
            case DSP::e::SampleType::ST_uchar:
              header_flt.sample_type(0x0001); // unsigned char (1B)
              //if (FileType == DSP::e::FileType::FT_flt_no_scaling)
              if (FileType_no_scaling == true)
                Execute_ptr = &InputExecute_uchar_no_scaling;
              break;
            case DSP::e::SampleType::ST_short:
              header_flt.sample_type(0x0002); // short (2B)
              if (FileType_no_scaling == true)
                Execute_ptr = &InputExecute_short_no_scaling;
              break;
            case DSP::e::SampleType::ST_int:
              header_flt.sample_type(0x0003); // int (4B)
              if (FileType_no_scaling == true)
                Execute_ptr = &InputExecute_int_no_scaling;
              break;
            default:
              #ifdef  __DEBUG__
                DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput" << DSP::LogMode::second << "*.flt: unsupported sample type" << endl;
              #endif
              break;
          }

          fwrite(&header_flt, sizeof(T_FLT_header), 1, FileHandle);
        }
        break;
      case DSP::e::FileType::FT_raw:
      default:
        // do nothing: header does not exist
        break;
    }
  }
  else
  {
    Execute_ptr = &InputExecute_Dummy;
    return false;
  }
  return true;
}

bool DSP::File::SetOffset(long long Offset, DSPe_offset_mode mode)
{
  bool result;
  result = false;

  Offset *= SampleSize;
  Offset /= 8;

  switch (mode)
  {
    case DSP_OM_skip:
      if (FileHandle != NULL)
      {
//        fseeko64(FileHandle, Offset, SEEK_SET);
        fseek(FileHandle, long(Offset), SEEK_SET);
        result = true;
      }
      break;

    case DSP_OM_standard:
    default:
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput:: SetOffset" << DSP::LogMode::second << "The mode is unsuported" << endl;
      #endif
      break;
  }
  return result;
}

bool DSPu_FILEoutput::SetSkip(long long Offset)
{
  skip_counter = Offset;
  return true;
}

unsigned int DSPu_FILEoutput::GetSampleSize(DSP::e::SampleType SampleType_in)
{
  unsigned int sample_size;

  switch (SampleType_in)
  {
    case DSP::e::SampleType::ST_float:
      sample_size=8*sizeof(float);
      break;
    case DSP::e::SampleType::ST_scaled_float:
      sample_size=8*sizeof(float);
      break;
    case DSP::e::SampleType::ST_uchar:
      sample_size=8*sizeof(unsigned char);
      break;
    case DSP::e::SampleType::ST_tchar:
      sample_size = 8 * sizeof(char);
      break;
    case DSP::e::SampleType::ST_bit_text:
      sample_size=8*sizeof(unsigned char);
      break;
    case DSP::e::SampleType::ST_short:
      sample_size=8*sizeof(short);
      break;
    case DSP::e::SampleType::ST_int:
      sample_size=8*sizeof(int);
      break;
    case DSP::e::SampleType::ST_bit:
    case DSP::e::SampleType::ST_bit_reversed:
      sample_size=1;
      if ((DSP_file_buffer_size % 8) != 0)
      {
        DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput::GetSampleSize" << DSP::LogMode::second << "Can't write bit stream corectly:"
          " DSP_file_buffer_size is not a multiply of byte size (8 bits)" << endl;
      }
      break;
    case DSP::e::SampleType::ST_none:
      sample_size = SampleSize;
      break;
    default:
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput::GetSampleSize" << DSP::LogMode::second << "Unsupported data type changing to DSP::e::SampleType::ST_short" << endl;
      #endif
      sample_size=8*sizeof(short);
      break;
  }
  sample_size*=NoOfInputs;
  return sample_size;
}

DSPu_FILEoutput::~DSPu_FILEoutput(void)
{
//  SetNoOfOutputs(0);
  Close();
}

void DSPu_FILEoutput::Flush(void)
{
  if (FileHandle != NULL)
  {
    if ((FlushBuffer_type & E_FB_raw) != 0)
      raw_FlushBuffer();
    else
      FlushBuffer();

    if ((FlushBuffer_type & E_FB_update_header) != 0)
    {
      switch (FileType)
      {
        case DSP::e::FileType::FT_wav:
          // header needs updating
          WAV_header.UpdateHeader(FileHandle);
          break;
        case DSP::e::FileType::FT_flt_no_scaling:
        case DSP::e::FileType::FT_flt:
          // header does not need updating
          break;
        case DSP::e::FileType::FT_raw:
        default:
          // do nothing: header does not exist
          break;
      }
    }
  }
}

void DSPu_FILEoutput::Close(void)
{
  Stored_Execute_ptr = NULL;
  IsBlocked = false;

  Flush();

  if (RawBuffer != NULL)
  {
    delete [] RawBuffer;
    RawBuffer=NULL;
  }
  if (TmpBuffer != NULL)
  {
    delete [] TmpBuffer;
    TmpBuffer=NULL;
  }
  if (FileHandle != NULL)
  {
    fclose(FileHandle);
    FileHandle=NULL;
  }
}

void DSPu_FILEoutput::PerformReOpen(void)
{
  Close();
  Open(ReOpen_FileName, ReOpen_SampleType, NoOfInputs,
       ReOpen_FileType, ReOpen_sampling_rate);

  ReOpenFile = false;
}

void DSPu_FILEoutput::ReOpen(const string &FileName, DSP::e::SampleType sample_type,
                             DSP::e::FileType file_type, long int sampling_rate)
{
  ReOpen_FileName = FileName;

  ReOpen_SampleType = sample_type;
  ReOpen_FileType = file_type;
  ReOpen_sampling_rate = sampling_rate;

  ReOpenFile = true;
}

bool DSPu_FILEoutput::BlockOutput(bool block)
{
  if (FileHandle == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << "DSPu_FILEoutput::BlockOutput" << DSP::LogMode::second << "Warning: FileHandle == NULL !!!" << endl;
    #endif
    return false;
  }
  if (block == IsBlocked)
    return false;

  if (block == true)
  {
    BlockFile = true;
    UnblockFile = false;
  }
  else
  {
    BlockFile = false;
    UnblockFile = true;
  }
  return true;
}

void DSPu_FILEoutput::PerformBlock(bool block)
{
  if (block == true)
  {
    Stored_Execute_ptr = Execute_ptr;
    Execute_ptr = InputExecute_blocked;

    IsBlocked = true;
  }
  else
  {
    Execute_ptr = Stored_Execute_ptr;
    Stored_Execute_ptr = NULL;

    IsBlocked = false;
  }
  BlockFile = false;
  UnblockFile = false;
}

// Returns raw buffer size in bytes needed for NoOfSamples samples.
/* If NoOfSamples == 0 return allocated internal raw buffer size.
 */
unsigned int DSPu_FILEoutput::GetRawBufferSize(unsigned int NoOfSamples)
{
  if (NoOfSamples == 0)
    return (SampleSize * DSP_file_buffer_size + 7)/8;
  else
    return (SampleSize * NoOfSamples + 7)/8;
}

unsigned int DSPu_FILEoutput::WriteSegmentFromBuffer( unsigned int buffer_size,
                                    uint8_t *raw_buffer,  DSP_float *flt_buffer,
                                    int skip)
{
  unsigned int ind, ind_raw;
  unsigned int BytesWritten;
  short temp_short;
  int temp_int;

  // Write to file content of the internal buffer
  if ((FlushBuffer_type & E_FB_raw) != 0)
    raw_FlushBuffer();
  else
    FlushBuffer();

  // convert flt_buffer to raw_buffer
  switch (SampleType)
  {
    case DSP::e::SampleType::ST_float:
      if ((sizeof(float) == sizeof(DSP_float)) && (skip == 0))
      { // write without conversion
        BytesWritten = (unsigned int)fwrite(flt_buffer, 1, SampleSize/8*buffer_size, FileHandle);
        return BytesWritten;
      }
      ind = 0;
      for (ind_raw=0; ind_raw < buffer_size*NoOfInputs; ind_raw++)
      {
        ((float *)raw_buffer)[ind_raw] = flt_buffer[ind++];
        ind+=skip;
      }
      break;

    case DSP::e::SampleType::ST_scaled_float:
      ind = 0;
      for (ind_raw=0; ind_raw < buffer_size*NoOfInputs; ind_raw++)
      {
        ((float *)raw_buffer)[ind_raw] = 0x8000 * flt_buffer[ind++];
        ind+=skip;
      }
      break;

    case DSP::e::SampleType::ST_uchar:
      if (FileType_no_scaling == true)
      {
        ind = 0;
        for (ind_raw=0; ind_raw < buffer_size*NoOfInputs; ind_raw++)
        {
          temp_short = (short)(flt_buffer[ind++]);
          if (temp_short < 0)
            temp_short = 0;
          if (temp_short > 0xff)
            temp_short = 0xff;
          ((uint8_t *)raw_buffer)[ind_raw] = (uint8_t)temp_short;
          ind+=skip;
        }
      }
      else
      {
        ind = 0;
        for (ind_raw=0; ind_raw < buffer_size*NoOfInputs; ind_raw++)
        {
          temp_short = (short)((flt_buffer[ind++]*0x80)+0x80);
          if (temp_short < 0)
            temp_short = 0;
          if (temp_short > 0xff)
            temp_short = 0xff;
          ((uint8_t *)raw_buffer)[ind_raw] = (uint8_t)temp_short;
          ind+=skip;
        }
      }
      break;


    case DSP::e::SampleType::ST_tchar:
      // assumes FileType_no_scaling == true
      {
        ind = 0;
        for (ind_raw = 0; ind_raw < buffer_size * NoOfInputs; ind_raw++)
        {
          temp_short = (short)(flt_buffer[ind++]);
          if (temp_short < 0)
            temp_short = 0;
          if (temp_short > 0xff)
            temp_short = 0xff;
          ((uint8_t*)raw_buffer)[ind_raw] = (uint8_t)temp_short;
          ind += skip;
        }
      }
      break;

    case DSP::e::SampleType::ST_short:
      if (FileType_no_scaling == true)
      {
        ind = 0;
        for (ind_raw=0; ind_raw < buffer_size*NoOfInputs; ind_raw++)
        {
          temp_int = (int)(flt_buffer[ind++]);
          if (temp_int < -0x8000)
            temp_int = -0x8000;
          if (temp_int > 0x7fff)
            temp_int = 0x7fff;
          ((short *)raw_buffer)[ind_raw] = (short)temp_int;
          ind+=skip;
        }
      }
      else
      {
        ind = 0;
        for (ind_raw=0; ind_raw < buffer_size*NoOfInputs; ind_raw++)
        {
          temp_int = (int)(flt_buffer[ind++]*0x8000);
          if (temp_int < -0x8000)
            temp_int = -0x8000;
          if (temp_int > 0x7fff)
            temp_int = 0x7fff;
          ((short *)raw_buffer)[ind_raw] = (short)temp_int;
          ind+=skip;
        }
      }
      break;

    default:
      DSP::log << DSP::LogMode::Error << "DSPu_FILEoutput::WriteSegmentToBuffer" << DSP::LogMode::second << "Unsupported file sample type - write aborted" << endl;
      return 0;
  }

  // write rawbuffer
  BytesWritten = (unsigned int)fwrite(raw_buffer, 1, SampleSize/8*buffer_size, FileHandle);
  return BytesWritten;
}

#define  DSP_THIS  ((DSPu_FILEoutput *)block)
// Just ignore inputs and process block and reopen signals
void DSPu_FILEoutput::InputExecute_Dummy(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_ARGUMENT(InputNo);
  UNUSED_ARGUMENT(value);
  UNUSED_DEBUG_ARGUMENT(Caller);

  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }

    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

// returns true if successfully processed given value
// value is put on input number InputNo
void DSPu_FILEoutput::InputExecute_float(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((float *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]=value;
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be written to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((float *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo] =
             DSP_THIS->ConstantInputValues[ind];
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

void DSPu_FILEoutput::InputExecute_scaled_float(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((float *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
                                 = 0x8000 * value;
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((float *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
               = 0x8000 * DSP_THIS->ConstantInputValues[ind];
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

void DSPu_FILEoutput::InputExecute_uchar(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  short temp;

  temp = (short)((value*0x80)+0x80);
  if (temp < 0)
    temp = 0;
  if (temp > 0xff)
    temp = 0xff;

  ((uint8_t *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
      = (unsigned char)(temp);
//      = (unsigned char)((value*0x80)+0x80);
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((uint8_t *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
              = (uint8_t)((DSP_THIS->ConstantInputValues[ind]*0x80)+0x80);
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

void DSPu_FILEoutput::InputExecute_uchar_no_scaling(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (value < 0)
    value = 0;
  if (value > 0xff)
    value = 0xff;

  ((uint8_t *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
                                   = (uint8_t)(value);
//      = (unsigned char)((value*0x80)+0x80);
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be written to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((uint8_t *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
              = (uint8_t)(DSP_THIS->ConstantInputValues[ind]);
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

void DSPu_FILEoutput::InputExecute_short(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);
  int temp;

  temp = (int)(value*0x8000);
  if (temp < -0x8000)
    temp = -0x8000;
  if (temp > 0x7fff)
    temp = 0x7fff;

  ((short *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
      = (short)(temp);
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }


    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((short *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
              = (short)(DSP_THIS->ConstantInputValues[ind]*0x8000);
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }

  #ifdef VerboseCompilation
    printf("%i: %5.3f\r\n", InputNo, value);
  #endif
};

void DSPu_FILEoutput::InputExecute_short_no_scaling(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (value < -0x8000)
    value = -0x8000;
  if (value > 0x7fff)
    value = 0x7fff;

  ((short *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
      = (short)(value);
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((short *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
              = (short)(DSP_THIS->ConstantInputValues[ind]);
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

void DSPu_FILEoutput::InputExecute_int(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);
  long long temp;

  temp = (long long)(value*0x80000000L);
  if (temp < INT_MIN)
    temp = INT_MIN;
  if (temp > INT_MAX)
    temp = INT_MAX;

  ((int *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
      = (int)(temp);
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((int *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
              = (int)(DSP_THIS->ConstantInputValues[ind]*0x80000000L);
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

void DSPu_FILEoutput::InputExecute_int_no_scaling(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (value < (DSP_float)INT_MIN)
    value = (DSP_float)INT_MIN;
  if (value > (DSP_float)INT_MAX)
    value = (DSP_float)INT_MAX;

  ((int *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
      = (int)(value);
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((int *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
              = (int)(DSP_THIS->ConstantInputValues[ind]);
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

void DSPu_FILEoutput::InputExecute_bit_text(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((uint8_t *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
      = (value>0.0)?'1':'0';
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((uint8_t *)DSP_THIS->RawBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
              = (DSP_THIS->ConstantInputValues[ind]>0.0)?'1':'0';
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

// is used for both >>bit<< & >>bit_reversed<< real work is done in FlushBuffer
void DSPu_FILEoutput::InputExecute_bit(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((uint8_t *)DSP_THIS->TmpBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
      = (value>0.0)? 1 : 0;
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->skip_counter > 0)
    {
      DSP_THIS->skip_counter--;
    }
    else
    {
      DSP_THIS->BufferIndex++;
      if (DSP_THIS->BufferIndex == DSP_file_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP_file_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((uint8_t *)DSP_THIS->TmpBuffer)[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
              = (DSP_THIS->ConstantInputValues[ind]>0.0) ? 1 : 0;
          DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      return;
    }
    if (DSP_THIS->BlockFile == true)
    {
      DSP_THIS->PerformBlock(true);
      return;
    }
  }
};

void DSPu_FILEoutput::InputExecute_blocked(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_ARGUMENT(InputNo);
  UNUSED_ARGUMENT(value);
  UNUSED_DEBUG_ARGUMENT(Caller);

  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    if (DSP_THIS->ReOpenFile == true)
    {
      DSP_THIS->PerformReOpen();
      if (DSP_THIS->BlockFile == true)
        DSP_THIS->PerformBlock(true);
      DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;
      return;
    }
    if (DSP_THIS->UnblockFile == true)
    {
      DSP_THIS->PerformBlock(false);
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;
  }

  #ifdef VerboseCompilation
    printf("%i: %5.3f\r\n", InputNo, value);
  #endif
};

#undef DSP_THIS

/*
// returns true if successfully processed given value
// value is put on input number InputNo
void DSPu_FILEoutput::InputExecute(DSP::Block *block, int InputNo, DSP_float value, DSP::Component_ptr Caller)
{ // we assume only one input
  ((DSPu_FILEoutput *)block)->Buffer[((DSPu_FILEoutput *)block)->BufferIndex * ((DSPu_FILEoutput *)block)->NoOfInputs + InputNo]=value;
  ((DSPu_FILEoutput *)block)->NoOfInputsProcessed++;

  if (((DSPu_FILEoutput *)block)->NoOfInputsProcessed == ((DSPu_FILEoutput *)block)->NoOfInputs)
  {
    if (((DSPu_FILEoutput *)block)->SamplesToSkipCounter > 0)
    {
      ((DSPu_FILEoutput *)block)->SamplesToSkipCounter--;
      ((DSPu_FILEoutput *)block)->NoOfInputsProcessed = ((DSPu_FILEoutput *)block)->InitialNoOfInputsProcessed;
      return;
    }

    ((DSPu_FILEoutput *)block)->BufferIndex++;
    ((DSPu_FILEoutput *)block)->BufferIndex %= DSP_file_buffer_size;

    if (((DSPu_FILEoutput *)block)->BufferIndex == 0)
    { // Data must be written to file from buffer
      //First we need to convert data from RawBuffer
      //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
      ((DSPu_FILEoutput *)block)->FlushBuffer();
    }

    //NoOfInputsProcessed=0;
    if (((DSPu_FILEoutput *)block)->IsUsingConstants)
    {
      for (int ind=0; ind < ((DSPu_FILEoutput *)block)->NoOfInputs; ind++)
        if (((DSPu_FILEoutput *)block)->IsConstantInput[ind])
        {
          ((DSPu_FILEoutput *)block)->Buffer[((DSPu_FILEoutput *)block)->BufferIndex * ((DSPu_FILEoutput *)block)->NoOfInputs + InputNo] =
             ((DSPu_FILEoutput *)block)->ConstantInputValues[ind];
          ((DSPu_FILEoutput *)block)->NoOfInputsProcessed++;
        }
    }
    ((DSPu_FILEoutput *)block)->NoOfInputsProcessed = ((DSPu_FILEoutput *)block)->InitialNoOfInputsProcessed;
  }

  #ifdef VerboseCompilation
    printf("%i: %5.3f\r\n", InputNo, value);
  #endif
};
*/

void DSPu_FILEoutput::FlushBuffer(void)
{
  uint8_t *tempUChar, mask;
  uint8_t *tempBuffer;
//#ifdef __DEBUG__
//  DSP_float *tempBuffer_float;
//  short *tempShort;
//  float *tempFloat;
//#endif
  unsigned int ind;

  if (FileHandle != NULL)
  {

    if (BufferIndex == 0)
      return; //BufferIndex=DSP_file_buffer_size;

    switch (SampleType)
    {
    #ifdef __DEBUG__
      case DSP::e::SampleType::ST_short:
        DSP::log << "DSPu_FILEoutput::FlushBuffer" << DSP::LogMode::second << "DSP::e::SampleType::ST_short no longer supported" << endl;
        /*
        tempBuffer_float=Buffer;

        tempShort=(short *)RawBuffer;
        for (ind=0; ind<NoOfInputs*BufferIndex; ind++)
        {
          *tempShort=(short)(*tempBuffer_float*32768);
          tempBuffer_float++;
          tempShort++;
        }
        */
        break;
      case DSP::e::SampleType::ST_float:
        DSP::log << "DSPu_FILEoutput::FlushBuffer" << DSP::LogMode::second << "DSP::e::SampleType::ST_float no longer supported" << endl;
        /*
        tempBuffer_float=Buffer;

        tempFloat=(float *)RawBuffer;
        for (ind=0; ind<NoOfInputs*BufferIndex; ind++)
        {
          *tempFloat=(float)(*tempBuffer_float);
          tempBuffer_float++;
          tempFloat++;
        }
        */
        break;
      case DSP::e::SampleType::ST_tchar:
        DSP::log << "DSPu_FILEoutput::FlushBuffer" << DSP::LogMode::second << "DSP::e::SampleType::ST_tchar no longer supported" << endl;
        break;
      case DSP::e::SampleType::ST_uchar:
        DSP::log << "DSPu_FILEoutput::FlushBuffer" << DSP::LogMode::second << "DSP::e::SampleType::ST_uchar no longer supported" << endl;
        /*
        tempBuffer_float=Buffer;

        tempUChar=(uint8_t *)RawBuffer;
        for (ind=0; ind<NoOfInputs*BufferIndex; ind++)
        {
          *tempUChar=(uint8_t)((*tempBuffer_float*0x80)+0x80);
          tempBuffer_float++;
          tempUChar++;
        }
        */
        break;
      case DSP::e::SampleType::ST_bit_text:
        DSP::log << "DSPu_FILEoutput::FlushBuffer" << DSP::LogMode::second << "DSP::e::SampleType::ST_bit_text no longer supported" << endl;
        /*
        tempBuffer_float=Buffer;

        tempUChar=(uint8_t *)RawBuffer;
        for (ind=0; ind<NoOfInputs*BufferIndex; ind++)
        {
          *tempUChar=(*tempBuffer_float>0.0)?'1':'0';
          tempBuffer_float++;
          tempUChar++;
        }
        */
        break;
    #endif
      case DSP::e::SampleType::ST_bit:
        tempBuffer = TmpBuffer;
        tempUChar=(uint8_t *)RawBuffer;
        mask=0x80; *tempUChar=0;
        for (ind=0; ind<NoOfInputs*BufferIndex; ind++)
        {
          if (*tempBuffer != 0)
          {
            //*tempUChar+=mask;
            *tempUChar = (uint8_t)(*tempUChar + mask);
          }
          tempBuffer++;

          //mask >>= 1;
          mask = (uint8_t)(mask >> 1);

          if (mask == 0)
          {
            mask = 0x80;
            tempUChar++;
            *tempUChar=0; // requires one spare byte in RawBuffer
          }
        }
        break;
      case DSP::e::SampleType::ST_bit_reversed:
        tempBuffer = TmpBuffer;
        tempUChar=(uint8_t *)RawBuffer;
        mask=0x01; *tempUChar=0;
        for (ind=0; ind<NoOfInputs*BufferIndex; ind++)
        {
          if (*tempBuffer != 0)
          {
            //*tempUChar+=mask;
            *tempUChar = (uint8_t)(*tempUChar + mask);
          }
          tempBuffer++;

          //mask <<= 1;
          mask = (uint8_t)(mask << 1);
          if (mask == 0)
          {
            mask = 0x01;
            tempUChar++;
            *tempUChar=0; // requires one spare byte in RawBuffer
          }
        }
        break;
      default:
        #ifdef __DEBUG__
          DSP::log << "DSPu_FILEoutput::FlushBuffer" << DSP::LogMode::second << "Unsupported format detected" << endl;
        #endif
        break;
    }

    fwrite(RawBuffer, 1, (SampleSize*BufferIndex+7)/8, FileHandle);
    fflush(FileHandle);
    BufferIndex=0;
  }
}


void DSPu_FILEoutput::raw_FlushBuffer(void)
{
  if (FileHandle != NULL)
  {
    if (BufferIndex == 0)
      return; // BufferIndex=DSP_file_buffer_size;

    fwrite(RawBuffer, 1, SampleSize/8*BufferIndex, FileHandle);
    fflush(FileHandle);
    BufferIndex=0;
  }
}

//*****************************************************//
//*****************************************************//
bool DSP::f::GetWAVEfileParams(const string &FileName, const string &FileDir,
                           T_WAVEchunk_ptr WAVEparams)
{
  string tekst;
  FILE *hIn;

  tekst = FileDir;
  if (tekst.length() > 0)
    if ((tekst.back() != '/') || (tekst.back() != '\\')) {
      tekst += '/';
    }
  tekst += FileName;

  hIn = fopen(tekst.c_str(), "rb");
  if (hIn == NULL)
  {
    //memset(WAVEparams, 0, sizeof(T_WAVEchunk));
    WAVEparams->clear();
    return false; //Error opening file
  }

  //Read WAVE file header
  if (WAVEparams->WAVEinfo(hIn)==false)
  {
    #ifdef __DEBUG__
    {
      DSP::log << DSP::LogMode::Error << "DSP::f::GetWAVEfileParams" << DSP::LogMode::second
        << "This (" << FileName << ") is not PCM WAVE file or file is corrupted !!!" << endl;
    }
    #endif
    fclose(hIn);
    return false;
  }
  fclose(hIn);

  return true;
}

// Creates object for *.wav files reading
DSPu_WaveInput::DSPu_WaveInput(DSP::Clock_ptr ParentClock,
                             const string &FileName_in, const string &FileDir_in,
                             unsigned int OutputsNo)
  : DSP::File(), DSP::Source()
{
  SetName("WaveInput", false);
  SetNoOfOutputs(OutputsNo);
  if (OutputsNo == 1)
  {
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
    DefineOutput("out1", 0);
  }
  if (OutputsNo == 2)
  {
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    DefineOutput("out1", 0);
    DefineOutput("out2", 1);
  }
  if (OutputsNo > 2)
  {
    unsigned int ind;
    vector<unsigned int> tempOut;
    string temp;

    tempOut.resize(OutputsNo);
    for (ind=0; ind<OutputsNo; ind++)
      tempOut[ind]=ind;
    DefineOutput("out", tempOut);

    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    for (ind=0; ind<OutputsNo; ind++)
    {
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, ind);
    }
  }

  RegisterOutputClock(ParentClock);

  FileName = FileName_in;
  FileDir = FileDir_in;

  SegmentSize=DSP_file_buffer_size;
  SamplingRate=DSP_DefaultSamplingFrequency;

  Init();

  OutputExecute_ptr = &OutputExecute;
}


DSPu_WaveInput::~DSPu_WaveInput(void)
{
  CloseFile();

  if (ReadBuffer !=NULL)
  {
    delete [] ReadBuffer;
    ReadBuffer=NULL;
  }
  if (ConvertionNeeded)
  {
    if (AudioBuffer !=NULL)
      delete [] AudioBuffer;
  }
  AudioBuffer=NULL;
}

bool DSPu_WaveInput::SetSkip(long long Offset)
{
  UNUSED_ARGUMENT(Offset);

  #ifdef __DEBUG__
    DSP::log << DSP::LogMode::Error << "DSPu_FILEinput::SetSkip" << DSP::LogMode::second << "not implemented yet" << endl;
  #endif
  return false;
}


//bool CWaveInput::StartCaptureAudio(void)
//To be used in constructor
bool DSPu_WaveInput::Init(void)
{
  string tekst;
  int len;

  BufferIndex=0; //this means also that Buffer is empty

  ConvertionNeeded=true; //inner buffer in DSP_float format
  AudioBufferLen = (DWORD)(NoOfOutputs*SegmentSize*sizeof(DSP_float));
  AudioBuffer=(DSP_float *)(new char[AudioBufferLen]);
  memset(AudioBuffer, 0, AudioBufferLen);

  tekst = FileDir; len = int(tekst.length());
  if (len>0)
    if ((tekst[len-1]!='/') || (tekst[len-1]!='\\')) {
      tekst += '/';
    }
  tekst += FileName;

//  hIn = CreateFile(tekst,  GENERIC_READ,
//                   FILE_SHARE_READ, NULL, OPEN_EXISTING,
//                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  FileHandle = fopen(tekst.c_str(), "rb");
  if (FileHandle == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << "DSPu_WaveInput::Init" << DSP::LogMode::second
        << "(Input file \"" << tekst
        << "\" in block <" << GetName() << "> could not be opened" << endl;
    #endif

    FileEnd=true;
		WAVEchunk.nChannels=0;
    ReadBufferLen=0; ReadBuffer=NULL;

    return false; //Error opening file
  }


  //Read WAVE file header
  if (WAVEchunk.WAVEinfo(FileHandle)==false)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSPu_WaveInput::Init" << DSP::LogMode::second
        << "This (" << FileName << ") is not PCM WAVE file or file is corrupted !!!" << endl;
    #endif
    fclose(FileHandle);
    FileHandle=NULL;
    FileEnd = true;
    return false;
  }

  //Check channels number
  #ifdef __DEBUG__
  {
    stringstream temp;
    switch (WAVEchunk.nChannels)
    {
      case 1:
        temp << ">> MONO, Fs=" << WAVEchunk.nSamplesPerSec
             << ", " << WAVEchunk.wBitsPerSample << "bits";
         break;
       case 2:
         temp << ">> STEREO, Fs=" << WAVEchunk.nSamplesPerSec
              << ", " << WAVEchunk.wBitsPerSample << "bits";
         break;
       default:
         temp << ">> MULTI(" << WAVEchunk.nChannels
              << "), Fs=" << WAVEchunk.nSamplesPerSec << ", "
              << WAVEchunk.wBitsPerSample << "bits";
         break;
     }
    DSP::log << "DSPu_WaveInput::Init" << DSP::LogMode::second << temp.str() << endl;
  }
  #endif
  //SampleSize = WAVEchunk.wBitsPerSample * WAVEchunk.nChannels;
  SampleSize = WAVEchunk.nBlockAlign * 8;

  if (WAVEchunk.FindDATA(FileHandle)==false)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSPu_WaveInput::Init" << DSP::LogMode::second
        << "No data have been found in PCM file " << FileName << "!!!" << endl;
    #endif
    fclose(FileHandle);
    FileHandle=NULL;
    FileEnd = true;
    return false;
  }

  BytesRemainingInFile=WAVEchunk.DataSize;
  FileEnd=false;

  ReadBufferLen=WAVEchunk.nChannels*WAVEchunk.wBitsPerSample/8*SegmentSize;
  ReadBuffer=new char[ReadBufferLen];
  BytesRead=0;

  SamplingRate=WAVEchunk.nSamplesPerSec;
//  SegmentSize;

  return (FileHandle!=NULL);
}

bool DSPu_WaveInput::CloseFile(void)
{
  if (FileHandle!=NULL)
  {
    fclose(FileHandle);
    FileHandle=NULL;
  }
  return true;
}

// Reads next file segment (returns number of samples read per channel)
/*  if file has more channels then zeros are set to excesive outputs
 *  if file has less channels then execive channels are discarded
 */
DWORD DSPu_WaveInput::ReadAudioSegment(void)
{
  if (FileEnd == true)
  { // AudioBuffer must be cleaned (reset)
    memset((uint8_t *)AudioBuffer, 0, AudioBufferLen);
    BytesRead=0;
    return 0x00000000;
  }
  else
  {
    //ReadFile(hIn, ReadBuffer, ReadBufferLen, &BytesRead, NULL);
    if (BytesRemainingInFile < ReadBufferLen)
      BytesRead=(DWORD)fread(ReadBuffer, 1, BytesRemainingInFile, FileHandle);
    else
      BytesRead=(DWORD)fread(ReadBuffer, 1, ReadBufferLen, FileHandle);
    BytesRemainingInFile-=BytesRead;
    if (BytesRead!=ReadBufferLen)
    {
      if (WAVEchunk.wBitsPerSample==8)
        memset(((uint8_t *)ReadBuffer)+BytesRead,0x80, ReadBufferLen-BytesRead);
      else
        memset(((uint8_t *)ReadBuffer)+BytesRead,0, ReadBufferLen-BytesRead);

//      if (BytesRead==0)
      FileEnd=true;
    }

    //Convert ReadBuffer to AudioBuffe;
    // with multichannel support
    if (ConvertionNeeded)
    {
      unsigned int ind, ind2;
      DSP_float *temp_Audio;

      temp_Audio=AudioBuffer;

      if (WAVEchunk.wBitsPerSample==8)
        for (ind=0; ind<SegmentSize; ind++)
          for (ind2=0; ind2<NoOfOutputs; ind2++)
          {
            if (ind2<WAVEchunk.nChannels)
              *temp_Audio=(DSP_float)(
                ((uint8_t *)ReadBuffer)[ind2+ind*WAVEchunk.nChannels]-0x80)
                / 0x80;
            else
              *temp_Audio=0.0;
            temp_Audio++;
          }
      else
        if (WAVEchunk.wBitsPerSample==16)
          for (ind=0; ind<SegmentSize; ind++)
            for (ind2=0; ind2<NoOfOutputs; ind2++)
            {
              if (ind2<WAVEchunk.nChannels)
                *temp_Audio=DSP_float(
                    ((short *)ReadBuffer)[ind2+ind*WAVEchunk.nChannels])
                    / 0x8000;
              else
                *temp_Audio=0.0;
              temp_Audio++;
            }
        else
          if (WAVEchunk.wBitsPerSample==32)
            for (ind=0; ind<SegmentSize; ind++)
              for (ind2=0; ind2<NoOfOutputs; ind2++)
              {
                if (ind2<WAVEchunk.nChannels)
                  *temp_Audio=DSP_float(
                      ((float *)ReadBuffer)[ind2+ind*WAVEchunk.nChannels])
                      / 0x8000;
                else
                  *temp_Audio=0.0;
                temp_Audio++;
              }
        #ifdef __DEBUG__
          else
          {
            DSP::log << DSP::LogMode::Error << "DSPu_WaveInput::ReadAudioSegment" << DSP::LogMode::second << "unsupported PCM sample size" << endl;
          }
        #endif

    }
    return (DWORD)(BytesRead/sizeof(DSP_float)/WAVEchunk.nChannels);
  }
}

// returns number of bytes read during last file access
unsigned int DSPu_WaveInput::GetBytesRead(void)
{
  return BytesRead;
}

// returns sampling rate of audio sample
long int DSPu_WaveInput::GetSamplingRate(void)
{
  return SamplingRate;
}

#define DSP_THIS ((DSPu_WaveInput *)source)
bool DSPu_WaveInput::OutputExecute(OUTPUT_EXECUTE_ARGS)
{ // we assume only one output
  UNUSED_DEBUG_ARGUMENT(clock);
  unsigned int ind;
  DSP_float *temp;

  if (DSP_THIS->BufferIndex == 0)
  { // Data must be read from file to buffer and convert to DSP_float
    DSP_THIS->ReadAudioSegment();
  }

  /* \todo_later Teh below is done on the pointer stored in the object 
   * so it wouldn't need continous recalculation
   * and just after the ReadAudioSegment above
   * sets the pointer to AudioBuffer and after each read increases just like temp.
   *
   * Check the above also in case of DSPu_FILEinput
   */
  temp = DSP_THIS->AudioBuffer +
         DSP_THIS->BufferIndex * DSP_THIS->NoOfOutputs;
  for (ind=0; ind < DSP_THIS->NoOfOutputs; ind++)
  {
//    OutputBlocks[ind]->Execute(OutputBlocks_InputNo[ind], *temp, this);
    DSP_THIS->OutputBlocks[ind]->EXECUTE_PTR(
        DSP_THIS->OutputBlocks[ind], DSP_THIS->OutputBlocks_InputNo[ind],
        *temp, source);
    temp++;
  }
  DSP_THIS->BufferIndex++;
  DSP_THIS->BufferIndex %= DSP_THIS->SegmentSize;

  return true;
};
#undef DSP_THIS

/*
//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>

#pragma hdrstop

#include "AudioDataIO.h"

//TMultiReadExclusiveWriteSynchronizer
//TCriticalSection
//---------------------------------------------------------------------------
#pragma package(smart_init)

//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TSoundCapture::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

// Konstruktor klasy CAudioInput
// No to tutaj to jest inicjator klasy a parametr wejsciowy
//    ustawiony na true oznacza, ze watek bedzie utworzony jako zawieszony
__fastcall CAudioInput::CAudioInput(bool CreateSuspended)
  : TThread(CreateSuspended)
{
  SegmentSize=DefaultSegmentSize;
  SamplingFrequency=DefaultSamplingFrequency;
}

__fastcall CAudioInput::~CAudioInput(void)
{
}

inline void __fastcall CAudioInput::Execute() {};

//================================================================//
//================================================================//
//================================================================//
inline void CWaveInput::SourceDescription(TStringList *Text)
{
  string temp;
  int ind, ind_;
  SYSTEMTIME stime;

  Text->Clear();
  Text->Add("");
  Text->Add("PCM WAVE FILE INPUT");

  GetLocalTime(&stime);
  Text->Add("Local time: ");
  sprintf(temp, "%2.2i-%2.2i-%4.4i, %2.2i:%2.2i:%2.2i",
    stime.wDay, stime.wMonth, stime.wYear,
    stime.wHour, stime.wMinute, stime.wSecond);
  Text->Add(temp);

  Text->Add("\\~\\~\\~File directory: ");
  ind_=0;
  for (ind=1; ind<=FileDir.Length(); ind++)
  {
    if ((FileDir[ind]=='\\') || (FileDir[ind]=='\/'))
    {
      temp[ind_++]='\\';
      temp[ind_++]=FileDir[ind];
    }
    else
      temp[ind_++]=FileDir[ind];
  }
  temp[ind_]=0;
  Text->Add(temp);

  Text->Add("\\~\\~\\~File name: ");
  Text->Add(FileName.c_str());

  Text->Add("\\~\\~\\~Data encoding format: ");
  sprintf(temp, "%i (1->PCM)", WAVEchunk.wFormatTag);
  Text->Add(temp);

  Text->Add("\\~\\~\\~Channels number: ");
  if (WAVEchunk.nChannels==1)
    sprintf(temp, "%i", WAVEchunk.nChannels);
  else
    sprintf(temp, "%i (only first channel is processed)", WAVEchunk.nChannels);
  Text->Add(temp);

  Text->Add("\\~\\~\\~Sampling frequency: ");
  sprintf(temp, "%i Hz", WAVEchunk.nSamplesPerSec);
  Text->Add(temp);

  Text->Add("\\~\\~\\~Bits per sample: ");
  sprintf(temp, "%i", WAVEchunk.wBitsPerSample);
  Text->Add(temp);

  Text->Add("\\~\\~\\~Total number o samples: ");
  sprintf(temp, "%i",
    WAVEchunk.DataSize/(WAVEchunk.wBitsPerSample/8)/WAVEchunk.nChannels);
  Text->Add(temp);

  Text->Add("\\~\\~\\~Recording time: ");
  sprintf(temp, "%.1f s",
    (double)(WAVEchunk.DataSize/(WAVEchunk.wBitsPerSample/8)/WAVEchunk.nChannels)
    /WAVEchunk.nSamplesPerSec);
  Text->Add(temp);
}

//---------------------------------------------------------------------------
*/
T_WAVEchunk::T_WAVEchunk(void)
{
  clear();
}
void T_WAVEchunk::clear() {
  memset(Type, 0x00, 4);
  size = 0;

  memset(SubType, 0x00, 4);

  memset(FmtType, 0x00, 4);
  Fmtsize = 0;

  wFormatTag = 1;
  nChannels = 0;
  nSamplesPerSec = 0;
  nAvgBytesPerSec = 0;
  nBlockAlign = 0;
  wBitsPerSample = 0;

  memset(DataType, 0x00, 4);
  DataSize = 0;

  // END OF HEADER //

  BytesRead = 0;
  HeaderSize = -1;
}

void T_WAVEchunk::PrepareHeader(
    DWORD nSamplesPerSec_in,
    WORD  nChannels_in,
    WORD  wBitsPerSample_in)
{
  memcpy(Type, "RIFF", 4);
  size = 0; // file size - 8

  memcpy(SubType, "WAVE", 4);

  memcpy(FmtType, "fmt ", 4);
  Fmtsize = 16 + 0;

  wFormatTag = 1; // PCM
  nChannels = nChannels_in;
  nSamplesPerSec = nSamplesPerSec_in;
  wBitsPerSample = wBitsPerSample_in;
  #ifdef __DEBUG__
    if ((wBitsPerSample % 8) != 0)
      DSP::log << DSP::LogMode::Error << "T_WAVEchunk::PrepareHeader" << DSP::LogMode::second << "wBitsPerSample_in in not a multiple of 8" << endl;
  #endif
  nBlockAlign = (WORD)(wBitsPerSample / 8 * nChannels);
  nAvgBytesPerSec = nSamplesPerSec * nBlockAlign;

  memcpy(DataType, "data ", 4);
  DataSize = 0;

  HeaderSize = 44; // without any extra bytes
}

bool T_WAVEchunk::WriteHeader(FILE *hOut)
{
  if ((HeaderSize > 0) && (hOut != NULL))
  {
    BytesRead=(DWORD)fwrite(this,1,HeaderSize,hOut);

    if ((int)BytesRead == HeaderSize)
      return true;
  }

  return false;
}

bool T_WAVEchunk::UpdateHeader(FILE *hOut)
{
  if ((HeaderSize > 0) && (hOut != NULL))
  {
    fpos_t pos;
    long long len;

    // 1. Store position in the file
    fgetpos(hOut, &pos);
    // 2. Get file length
#ifdef DEVCPP
    fseek(hOut, 0, SEEK_END);
    len = ftell(hOut);
#else
//    fseeko64(hOut, 0, SEEK_END);
    fseek(hOut, 0, SEEK_END);
//    len = ftello64(hOut);
    len = ftell(hOut);
#endif
    // 3. Compute size and DataSize
    size = (DWORD)(len - 8);
    DataSize = (DWORD)(len - HeaderSize);
    // 4. Add padding if necessary
    if ((DataSize % 2) == 1)
    {
      char tmp = 0;
      fwrite(&tmp, 1, 1, hOut);
      size++;
    }
    // 5. update header
#ifdef DEVCPP
    fseek(hOut, 0, SEEK_SET);
#else
//    fseeko64(hOut, 0, SEEK_SET);
    fseek(hOut, 0, SEEK_SET);
#endif
    BytesRead=(DWORD)fwrite(this,1,HeaderSize,hOut);

    // 6. Restore position in the file
    fsetpos(hOut, &pos);

    if ((int)BytesRead == HeaderSize)
      return true;
  }

  return false;
}

bool T_WAVEchunk::WAVEinfo(FILE *hIn)
{
//4  read(hIn, Type, 4); "RIFF"
//8  read(hIn, size, 4);
//12  read(hIn, SubType, 4); "WAVE"
//16  read(hIn, FmtType, 4); "fmt "
//20  read(hIn, Fmt_size, 4);
//22(2)  WORD wFormatTag; // Data encoding format
//24(4)  WORD nChannels;  // Number of channels
//28(8)  DWORD nSamplesPerSec;   // Samples per second
//32(12)  DWORD nAvgBytesPerSec;  // Avg transfer rate
//34(14)  WORD  nBlockAlign;      // Block alignment
//36(16)  WORD  nBitsPerSample;   // Bits per sample

//  ReadFile(hIn, this, 36, &BytesRead, NULL);
  BytesRead=(DWORD)fread(this,1,36,hIn);
  if (BytesRead<36)
    return false; //Something is wrong
  if (Fmtsize>16)
  {
    //skip (we don't know what to do with it)
//    SetFilePointer(hIn, Fmtsize-16, NULL, FILE_CURRENT);
//    fseeko64(hIn, Fmtsize-16, SEEK_CUR);
    fseek(hIn, Fmtsize-16, SEEK_CUR);
  }
  if (wFormatTag!=1)
  {
    //This is not PCM file
    return false;
  }


  //  if ((strncasecmp(Type, "RIFF", 4)!=0) &&
  //      (strncasecmp(Type, "FMT ", 4)!=0))
  if ((strncmpi(Type, "RIFF", 4)!=0) &&
      (strncmpi(Type, "FMT ", 4)!=0))
    return false;
  return true;
}


int T_WAVEchunk::strncmpi(const char* str1, const char* str2, int N)
{
  int result;

  result=0;
  for (int ind=0; ind<N; ind++)
  {
    if ((str1[ind]==0) && (str2[ind]==0))
      break;

    if (toupper(str1[ind])!=toupper(str2[ind]))
    {
      result=1;
      break;
    }
  }
  return result;
}

bool T_WAVEchunk::FindDATA(FILE *hIn)
{
  DWORD BytesRead_temp;

//  ReadFile(hIn, DataType, 4, &BytesRead_temp, NULL);
  BytesRead_temp=(DWORD)fread(DataType,1,4,hIn);
  BytesRead+=BytesRead_temp;
//  ReadFile(hIn, &DataSize, 4, &BytesRead_temp, NULL);
  BytesRead_temp=(DWORD)fread(&DataSize,1,4,hIn);
  BytesRead+=BytesRead_temp;

  //  while (strncasecmp(DataType, "DATA", 4)!=0)
  while (strncmpi(DataType, "DATA", 4)!=0)
  {
//    SetFilePointer(hIn, DataSize+(DataSize%2), NULL, FILE_CURRENT);
//    fseeko64(hIn, DataSize+(DataSize%2), SEEK_CUR);
    fseek(hIn, DataSize+(DataSize%2), SEEK_CUR);
//    ReadFile(hIn, DataType, 4, &BytesRead_temp, NULL);
    BytesRead_temp=(DWORD)fread(DataType,1,4,hIn);
    BytesRead+=BytesRead_temp;
//    ReadFile(hIn, &DataSize, 4, &BytesRead_temp, NULL);
    BytesRead_temp=(DWORD)fread(&DataSize,1,4,hIn);
    BytesRead+=BytesRead_temp;
    if (BytesRead_temp==0) //eof(hIn))
    {
      return false;
    }
  }
  return true;
}
//---------------------------------------------------------------------------

/*
//================================================================//
//================================================================//
//================================================================//

__fastcall CDirectXInput::CDirectXInput(bool CreateSuspended)
  : CAudioInput(CreateSuspended)
{
  int ind;

  Priority=tpTimeCritical;

  SegmentSize=DefaultSegmentSize;
  SamplingFrequency=DefaultSamplingFrequency;

  lpDsNotify=NULL;
  Idscb=NULL;
  Idsc=NULL;
  BufferNotEmpty=NULL;

  Error=InitAudio();

  //Prepare the InBuffers
  for (ind=0; ind<NoOfInBlocks; ind++)
  {
    InBuffer[ind]=new char[WaveFormat.nChannels*WaveFormat.wBitsPerSample/8*SegmentSize];
    InBufferLen[ind]=0;
  }
  InBufferFree=0; InBufferFull=0; FullBuffersNo=0;
}

__fastcall CDirectXInput::~CDirectXInput(void)
{
  int ind;

  ReleaseAudio();

  //delete the InBuffers
  for (ind=0; ind<NoOfInBlocks; ind++)
  {
    if (InBuffer[ind]==NULL)
      delete [] InBuffer[ind];
    InBuffer[ind]=NULL;
    InBufferLen[ind]=0;
  }
  InBufferFree=0; InBufferFull=0; FullBuffersNo=0;
}
//---------------------------------------------------------------------------
inline void __fastcall CDirectXInput::Execute()
{
  DWORD Wait_result, Ktory;
  LPVOID CaptureBuforLock;
  DWORD CaptureBuforLockLen;

  if (Error != DS_OK)
    return;

  //---- Place thread code here ----
  while (!Terminated)
  {
//    Wait_result=MsgWaitForMultipleObjects(NoOfSegments+1, notifyEvent, false, 100, 0);
    Wait_result=MsgWaitForMultipleObjects(NoOfSegments, notifyEvent, false, 100, 0);

//    if ((Wait_result >= WAIT_OBJECT_0) & (Wait_result <= WAIT_OBJECT_0 + NoOfSegments))
    if ((Wait_result >= WAIT_OBJECT_0) & (Wait_result < WAIT_OBJECT_0 + NoOfSegments))
    {
      Ktory=Wait_result-WAIT_OBJECT_0;
      //In the future we can check GetCurrentPossition
      Idscb->Lock(notifyPosition[Ktory].dwOffset-notifyPosition[0].dwOffset, inbuffer.dwBufferBytes/NoOfSegments,
           &CaptureBuforLock, &CaptureBuforLockLen, NULL, NULL, 0);

      //Let's copy this segment to the free InBuffer slot
      if (FullBuffersNo!=NoOfInBlocks)
      {
        InBufferLen[InBufferFree]=CaptureBuforLockLen;
        CopyMemory(InBuffer[InBufferFree++],CaptureBuforLock,CaptureBuforLockLen);
        InBufferFree=(short)(InBufferFree % NoOfInBlocks);
        FullBuffersNo++;
        BufferNotEmpty->SetEvent();

//WriteFile(Plik, CaptureBuforLock, CaptureBuforLockLen, &IleWritten, NULL);
      }

      Idscb->Unlock(CaptureBuforLock, CaptureBuforLockLen, NULL, 0);

    }
    else
    {
      //WAIT_TIMEOUT
      //Wait_result=WAIT_TIMEOUT;
    }
    //  Idscb->GetCurrentPosition();
    //  Idscb->Lock();
    //  Idscb->Unlock();
  }
}
//---------------------------------------------------------------------------

HRESULT CDirectXInput::InitAudio(void)
{
  HRESULT Error;

  BufferNotEmpty = new TEvent(NULL, true, false, "BufferNotEmpty");

  Error=DirectSoundCaptureCreate(
    NULL, //DSDEVID_DefaultCapture,
    &Idsc,
    NULL);
//  switch (Error)
//  {
//    case DSERR_ALLOCATED:
//     //Cant get to the device
//      //maybe Full douplex mode not available
//      break;
//    case DSERR_INVALIDPARAM:
//      //Invalid parameter
//      //maybe wrong device
//      break;
//    case DSERR_NOAGGREGATION:
//      //The object does not support aggregation. ???
//      break;
//    case DSERR_OUTOFMEMORY:
//      //Memory problem
//      break;
//  }

  if (Error==DS_OK)
  {
    Error=CreateAudioInBuffer();
    if (Error==DS_OK)
    {
      SetNotifications();
    }
  }

  return Error;
}

bool CDirectXInput::StartCaptureAudio(void)
{
  Resume();
  Idscb->Start(DSCBSTART_LOOPING);

  return true;
}

bool CDirectXInput::StopCaptureAudio(void)
{
  Idscb->Stop();
  Suspend();

  return true;
}

HRESULT CDirectXInput::CreateAudioInBuffer(void)
{
  HRESULT Error;

  inbuffer.dwSize=sizeof(inbuffer);
//  inbuffer.dwFlags=DSCBCAPS_WAVEMAPPED; //DSCBCAPS_CTRLFX
  WaveFormat.wFormatTag=WAVE_FORMAT_PCM;
  WaveFormat.nChannels=1;
  WaveFormat.wBitsPerSample=16;
  WaveFormat.nSamplesPerSec=SamplingFrequency;
  WaveFormat.nBlockAlign=(WORD)(WaveFormat.nChannels*
    WaveFormat.wBitsPerSample/8);
  WaveFormat.nAvgBytesPerSec=WaveFormat.nSamplesPerSec*
    WaveFormat.nBlockAlign;
  WaveFormat.cbSize=0; //No additional data
  inbuffer.dwSize=sizeof(DSCBUFFERDESC);
  inbuffer.dwBufferBytes=WaveFormat.nBlockAlign*
    NoOfSegments*SegmentSize;
  inbuffer.lpwfxFormat=&WaveFormat;
  inbuffer.dwFXCount=0;
  inbuffer.lpDSCFXDesc=NULL;

  Error=Idsc->CreateCaptureBuffer(
    &inbuffer,  &Idscb,  NULL);
  return Error;
}

bool CDirectXInput::ReleaseAudio(void)
{
  if (Suspended)
    Resume();
  Terminate();
  WaitFor();

  if (lpDsNotify!=NULL)
    lpDsNotify->Release();
  if (Idscb!=NULL)
    Idscb->Release();
  if (Idsc!=NULL)
    Idsc->Release();

  if (BufferNotEmpty!=NULL)
  {
    delete BufferNotEmpty;
    BufferNotEmpty=NULL;
  }
  return true;
}

HRESULT CDirectXInput::SetNotifications(void)
{
  // It is assumed that the following variables have
  // been properly initialized, and that wfx was included in the
  // buffer description when the buffer was created.
  //
  // LPDIRECTSOUNDNOTIFY8 lpDsNotify;
  // WAVEFORMATEX    wfx;

  HRESULT    Error;
  int      i;

  for (i = 0; i < NoOfSegments; i++)
  {
    // Create the event.
//    notifyEvent[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
    notifyEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
    // Describe notifications.
    notifyPosition[i].dwOffset = ((i+1)*inbuffer.dwBufferBytes/NoOfSegments) -1;
    notifyPosition[i].hEventNotify = notifyEvent[i];
  }

//  // Create last event.
//  notifyEvent[NoOfSegments] = CreateEvent(NULL, TRUE, FALSE, NULL);
//  // Describe last notifications.
//  notifyPosition[NoOfSegments].dwOffset = DSBPN_OFFSETSTOP;
//  notifyPosition[NoOfSegments].hEventNotify = notifyEvent[NoOfSegments];


  // Create notifications.
  Error = Idscb->QueryInterface(IID_IDirectSoundNotify,
                        (void **)(&lpDsNotify));
  if (Error==DS_OK)
  {
    Error = lpDsNotify->SetNotificationPositions(
              NoOfSegments, notifyPosition);
  }
  return Error;
}


inline int CDirectXInput::BufferNotEmpty_WaitFor(int timeout)
{ //WAIT_OBJECT_0
  return BufferNotEmpty->WaitFor(0);
}

inline bool CDirectXInput::DrawingAllowed(void)
{ //Allow drawing if no new data to process
  return (BufferNotEmpty->WaitFor(0) != WAIT_OBJECT_0);
}

inline short CDirectXInput::GetFullBuffersNo(void)
{
  return FullBuffersNo;
}


inline void CDirectXInput::SourceDescription(TStringList *Text)
{
  string temp;
  int ind, ind_;
  SYSTEMTIME stime;

  Text->Clear();
  Text->Add("");
  Text->Add("DirectX Sound INPUT");

  GetLocalTime(&stime);
  Text->Add("Local time: ");
  sprintf(temp, "%2.2i-%2.2i-%4.4i, %2.2i:%2.2i:%2.2i",
    stime.wDay, stime.wMonth, stime.wYear,
    stime.wHour, stime.wMinute, stime.wSecond);
  Text->Add(temp);

  Text->Add("\\~\\~\\~Data encoding format: ");
  sprintf(temp, "%i (1->PCM)", WaveFormat.wFormatTag);
  Text->Add(temp);

  Text->Add("\\~\\~\\~Channels number: ");
  sprintf(temp, "%i", WaveFormat.nChannels);
  Text->Add(temp);

  Text->Add("\\~\\~\\~Sampling frequency: ");
  sprintf(temp, "%i Hz", WaveFormat.nSamplesPerSec);
  Text->Add(temp);

  Text->Add("\\~\\~\\~Bits per sample: ");
  sprintf(temp, "%i", WaveFormat.wBitsPerSample);
  Text->Add(temp);
}


*/

#ifdef WINMMAPI
  // ****************************************************** //
  // ****************************************************** //
  // ****************************************************** //
  // Issues error message and returns true if error detected
  bool DSP::f::AudioCheckError(MMRESULT result)
  {
    #ifdef __DEBUG__
      switch (result)
      {
        case MMSYSERR_ALLOCATED:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "Specified resource is already allocated." << endl;
          break;
        case MMSYSERR_BADDEVICEID:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "Specified device identifier is out of range." << endl;
          break;
        case MMSYSERR_NODRIVER:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "No device driver is present." << endl;
          break;
        case MMSYSERR_NOMEM:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "Unable to allocate or lock memory." << endl;
          break;
        case WAVERR_BADFORMAT:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "Attempted to open with an unsupported waveform-audio format." << endl;
          break;
        case WAVERR_SYNC:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "The device is synchronous but waveOutOpen was called without using the WAVE_ALLOWSYNC flag." << endl;
          break;
        case MMSYSERR_INVALFLAG:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "Invalid flag" << endl;
          break;
        case MMSYSERR_NOERROR:
          return false;
          //printf("DSP_AudioCheckError: ");
          //printf("No error.");
          //printf("\n"); getchar();
          break;
        case MMSYSERR_INVALHANDLE:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "Specified device handle is invalid." << endl;
          break;
        case WAVERR_UNPREPARED:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "The data block pointed to by the pwh parameter hasn't been prepared." << endl;
          break;
        case MMSYSERR_HANDLEBUSY:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "Handle busy." << endl;
          break;
        case WAVERR_STILLPLAYING:
          DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second <<
            "There are still buffers in the queue." << endl;
          break;
        default:
          {
            DSP::log << DSP::LogMode::Error << "DSP_AudioCheckError" << DSP::LogMode::second
              << "Unknown error " << WAVERR_BASE << " " << result << endl;
          }
          break;
      }
    #else
      if (result == MMSYSERR_NOERROR)
        return false;
    #endif
    return true;
  };
#endif

DWORD DSP::f::GetAudioBufferSize(unsigned long SamplingFreq, DSPe_AudioBufferType type)
{
  DWORD size;

  switch (type)
  {
    case DSP_AB_IN:
      size = DSP_reference_audio_inbuffer_size;
      break;
    case DSP_AB_OUT:
      size = DSP_reference_audio_outbuffer_size;
      break;
    case DSP_AB_none:
    default:
      size = 0;
      break;
  }

//  size / DSP_ReferenceFs == new_size / SamplingFreq
#ifdef __DEBUG__
  if ((SamplingFreq * size) / DSP_ReferenceFs > UINT32_MAX){
    DSP::log << DSP::LogMode::Error << "DSP::f::GetAudioBufferSize" << DSP::LogMode::second
       << "Buffer size (" << (SamplingFreq * size) / DSP_ReferenceFs << ") > UINT32_MAX" << endl;
  }
#endif
  size = (DWORD)((SamplingFreq * size) / DSP_ReferenceFs);
  return size;
}

#ifdef WINMMAPI
  void CALLBACK DSPu_AudioOutput::waveOutProc(HWAVEOUT hwo, UINT uMsg,
    DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
  {
    UNUSED_ARGUMENT(hwo);
    UNUSED_ARGUMENT(uMsg);
    UNUSED_ARGUMENT(dwInstance);
    UNUSED_ARGUMENT(dwParam1);
    UNUSED_ARGUMENT(dwParam2);
  #ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
  //  MMRESULT result;
    DSPu_AudioOutput *Current;
    bool AllDone;
    int ind;
    string tekst;

    Current = AudioObjects[dwInstance];

    switch (uMsg)
    {
      case WOM_OPEN:
        DSP::log << "DSPu_AudioOutput::waveOutProc" << DSP::LogMode::second
          << "WOM_OPEN(" << (int)dwInstance << ")" << endl;
        break;
      case WOM_CLOSE:
        DSP::log << "DSPu_AudioOutput::waveOutProc" << DSP::LogMode::second
          << "WOM_CLOSE(" << (int)dwInstance << ")" << endl;
        break;
      case WOM_DONE:
        DSP::log << "DSPu_AudioOutput::waveOutProc" << DSP::LogMode::second
          << "WOM_DONE(" << (int)dwInstance << ")" << endl;

        if (Current->StopPlaying)
        {
          DSP::log << "DSPu_AudioOutput::waveOutProc" << DSP::LogMode::second << "StopPlaying is set" << endl;
          return;
        }
        else
        {
          AllDone=true;
          for (ind=0; ind < DSP_NoOfAudioOutputBuffers; ind++)
            AllDone &= (Current->waveHeaderOut[ind].dwFlags & WHDR_DONE);
          if (AllDone)
            DSP::log << "DSPu_AudioOutput::waveOutProc" << DSP::LogMode::second << "All buffers had been used - nothing to play" << endl;
        }
        break;
    }
  #endif
  #endif
  }

  //! \bug allow user to select number of internal buffers
  void CALLBACK DSPu_AudioInput::waveInProc_short(HWAVEIN hwi, UINT uMsg,
    DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
  {
    UNUSED_ARGUMENT(hwi);
    UNUSED_ARGUMENT(dwParam1);
    UNUSED_ARGUMENT(dwParam2);

    MMRESULT result;
    DSPu_AudioInput *Current;
    short *temp16;
    DSP_float_ptr Sample;
    unsigned int ind;
  #ifdef __DEBUG__
    #ifdef AUDIO_DEBUG_MESSAGES_ON
      stringstream tekst;
    #endif
  #endif

    Current = AudioObjects[dwInstance];

    switch (uMsg)
    {
  #ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
      case WIM_OPEN:
        DSP::log << "DSPu_AudioInput::waveInProc" << DSP::LogMode::second
          << "WIM_OPEN(" << (int)dwInstance << ")" << endl;
        break;
      case WIM_CLOSE:
        DSP::log << "DSPu_AudioInput::waveInProc" << DSP::LogMode::second
          << "WIM_CLOSE(" << (int)dwInstance << ")" << endl;
        break;
  #endif
  #endif

      case WIM_DATA:
  #ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
        DSP::log << "DSPu_AudioInput::waveInProc" << DSP::LogMode::second
          << "WIM_DATA(" << (int)dwInstance << ")" << endl;
  #endif
  #endif

        if (Current->StopRecording)
          return;
        else
        {
          if (Current->EmptyBufferIndex == Current->CurrentBufferIndex)
          {
  #ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
            DSP::log << "DSPu_AudioInput::waveInProc" << DSP::LogMode::second << "All buffers had been used - skipping input audio frame" << endl;
  #endif
  #endif
            result=waveInUnprepareHeader(Current->hWaveIn,
              &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
            DSP::f::AudioCheckError(result);
            // ignore data

            //add put back into recording queue
            result=waveInPrepareHeader(Current->hWaveIn,
              &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
            DSP::f::AudioCheckError(result);
            result=waveInAddBuffer(Current->hWaveIn,
              &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
            DSP::f::AudioCheckError(result);

            Current->NextBufferInd++; Current->NextBufferInd %= 2; //just two buffers
          }
          else
          { //copy audio frame to buffer form audio frame NextBufferInd
            if (Current->waveHeaderIn[Current->NextBufferInd].dwFlags & WHDR_DONE)
            {
              //copy data
              result=waveInUnprepareHeader(Current->hWaveIn,
                &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
              DSP::f::AudioCheckError(result);

              Sample=Current->InBuffers[Current->EmptyBufferIndex];
              // ************************************************** //
              // Converts samples format to the one suitable for the audio device
              #ifdef __DEBUG__
                if (Current->InSampleType != DSP::e::SampleType::ST_short)
                {
                  DSP::log << "DSPu_AudioInput::waveInProc_short" << DSP::LogMode::second << "Current->InSampleType != DSP::e::SampleType::ST_short" << endl;
                }
              #endif

              temp16=(short *)(Current->WaveInBuffers[Current->NextBufferInd]);
              for (ind=0; ind<Current->InBufferLen; ind++)
              {
                *Sample = (DSP_float)(*temp16) / SHRT_MAX;
                Sample++;
                temp16++;
              }
              Current->EmptyBufferIndex++; Current->EmptyBufferIndex %= DSP_NoOfAudioInputBuffers;

              //add put back into recording queue
              result=waveInPrepareHeader(Current->hWaveIn,
                &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
              DSP::f::AudioCheckError(result);
              result=waveInAddBuffer(Current->hWaveIn,
                &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
              DSP::f::AudioCheckError(result);

              Current->NextBufferInd++; Current->NextBufferInd %= 2; //just two buffers
            }
            else
            {
  #ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
              DSP::log << "DSPu_AudioInput::waveInProc" << DSP::LogMode::second << "Wrong audio frame ready or other unexpected error" << endl;
  #endif
  #endif
            }
          }
        }
        break; // WIM_DATA
    }
  }

  //! \bug allow user to select number of internal buffers
  void CALLBACK DSPu_AudioInput::waveInProc_uchar(HWAVEIN hwi, UINT uMsg,
    DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
  {
    UNUSED_ARGUMENT(hwi);
    UNUSED_ARGUMENT(dwParam1);
    UNUSED_ARGUMENT(dwParam2);

    MMRESULT result;
    DSPu_AudioInput *Current;
    uint8_t *temp8;
    DSP_float_ptr Sample;
    unsigned int ind;
  #ifdef __DEBUG__
    #ifdef AUDIO_DEBUG_MESSAGES_ON
      stringstream tekst;
    #else
      UNUSED_DEBUG_ARGUMENT(uMsg);
    #endif
  #endif

    Current = AudioObjects[dwInstance];

    switch (uMsg)
    {
  #ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
      case WIM_OPEN:
        DSP::log << "DSPu_AudioInput::waveInProc" << DSP::LogMode::second
          << "WIM_OPEN(" << (int)dwInstance << ")" << end;
        break;
      case WIM_CLOSE:
        DSP::log << "DSPu_AudioInput::waveInProc" << DSP::LogMode::second
          << "WIM_CLOSE(" << (int)dwInstance << ")" << endl;
        break;
  #endif
  #endif

      case WIM_DATA:
  #ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
        DSP::log << "DSPu_AudioInput::waveInProc" << DSP::LogMode::second
          << "WIM_DATA(" << (int)dwInstance << ")" << endl;
  #endif
  #endif

        if (Current->StopRecording)
          return;
        else
        {
          if (Current->EmptyBufferIndex == Current->CurrentBufferIndex)
          {
  #ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
            DSP::log << "DSPu_AudioInput::waveInProc"  << DSP::LogMode::second << "All buffers had been used - skipping input audio frame" << endl;
  #endif
  #endif
            result=waveInUnprepareHeader(Current->hWaveIn,
              &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
            DSP::f::AudioCheckError(result);
            // ignore data

            //add put back into recording queue
            result=waveInPrepareHeader(Current->hWaveIn,
              &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
            DSP::f::AudioCheckError(result);
            result=waveInAddBuffer(Current->hWaveIn,
              &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
            DSP::f::AudioCheckError(result);

            Current->NextBufferInd++; Current->NextBufferInd %= 2; //just two buffers
          }
          else
          { //copy audio frame to buffer form audio frame NextBufferInd
            if (Current->waveHeaderIn[Current->NextBufferInd].dwFlags & WHDR_DONE)
            {
              //copy data
              result=waveInUnprepareHeader(Current->hWaveIn,
                &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
              DSP::f::AudioCheckError(result);

              Sample=Current->InBuffers[Current->EmptyBufferIndex];
              // ************************************************** //
              // Converts samples format to the one suitable for the audio device
              #ifdef __DEBUG__
                if (Current->InSampleType != DSP::e::SampleType::ST_uchar)
                {
                  DSP::log << "DSPu_AudioInput::waveInProc_uchar" << DSP::LogMode::second << "Current->InSampleType != DSP::e::SampleType::ST_uchar" << endl;
                }
              #endif

              temp8=(uint8_t *)(Current->WaveInBuffers[Current->NextBufferInd]);
              for (ind=0; ind<Current->InBufferLen; ind++)
              {
                *Sample = (DSP_float)(*temp8 - 128) / 128;
                Sample++;
                temp8++;
              }
              Current->EmptyBufferIndex++; Current->EmptyBufferIndex %= DSP_NoOfAudioInputBuffers;

              //add put back into recording queue
              result=waveInPrepareHeader(Current->hWaveIn,
                &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
              DSP::f::AudioCheckError(result);
              result=waveInAddBuffer(Current->hWaveIn,
                &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
              DSP::f::AudioCheckError(result);

              Current->NextBufferInd++; Current->NextBufferInd %= 2; //just two buffers
            }
            else
            {
  #ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
              DSP::log << "DSPu_AudioInput::waveInProc" << DSP::LogMode::second << "Wrong audio frame ready or other unexpected error" << endl;
  #endif
  #endif
            }
          }
          break;
        }

    }
  }
#endif // WINMMAPI

unsigned long DSPu_AudioOutput::Next_CallbackInstance=0;
unsigned long DSPu_AudioInput::Next_CallbackInstance =0;
DSPu_AudioOutput **DSPu_AudioOutput::AudioObjects = NULL;
DSPu_AudioInput  **DSPu_AudioInput::AudioObjects  = NULL;

DSPu_AudioOutput::DSPu_AudioOutput(void)
  : DSP::Block()
{
  Init(8000);

  Execute_ptr = &InputExecute;
}

/*
DSPu_AudioInput::DSPu_AudioInput(DSP::Clock_ptr ParentClock)
  : DSP::Source(ParentClock)
{
  Init(ParentClock, 8000);
}
*/

DSPu_AudioOutput::DSPu_AudioOutput(
                 unsigned long SamplingFreq,
                 unsigned int InputsNo, //just one channel
                 unsigned char BitPrec,
                 unsigned int WaveOutDevNo)
  : DSP::Block()
{
  Init(SamplingFreq, InputsNo, BitPrec, WaveOutDevNo);

  Execute_ptr = &InputExecute;
}

/* Inputs and Outputs names:
 *   - Output: none
 *   - Input:
 *    -# "in" - real or complex
 *    -# "in.re" - first channel (real component)\n
 *       "in.im" - second channel (imag component if exists)
 *    -# "in1", "in2" - i-th channel input
 */
void DSPu_AudioOutput::Init(unsigned long SamplingFreq,
                           unsigned int InputsNo, //just one channel
                           unsigned char BitPrec,
                           unsigned int WaveOutDevNo)
{
  #ifdef WINMMAPI
    MMRESULT result;
    DWORD_PTR Callback;

    //Rezerwacja pamięci dla formatu WAVE
    //  WAVEFORMATEX wfx; //to wymaga korekty
    PCMWAVEFORMAT wfx;
  #else
    UNUSED_ARGUMENT(WaveOutDevNo);
  #endif

  unsigned long ind;
  DSPu_AudioOutput **tempAudioObject;
  string temp;


  SetNoOfInputs(InputsNo, true);
  SetName("AudioOutput", false);

  if (InputsNo == 1)
  {
    DefineInput("in", 0);
    DefineInput("in.re", 0);

    ClockGroups.AddInput2Group("input", Input("in"));
  }
  if (InputsNo == 2)
  {
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);

    ClockGroups.AddInput2Group("input", Input("in"));
  }
  for (unsigned int ind_i=0; ind_i<NoOfInputs; ind_i++)
  {
    temp = "in" + to_string(ind_i+1);
    DefineInput(temp, ind_i);
    ClockGroups.AddInput2Group("input", Input(temp));
  }

  #ifdef WINMMAPI
    //! \bug in Debug mode this callback does nothing so it would be better just not use it
    Callback = (DWORD_PTR)(&DSPu_AudioOutput::waveOutProc);
  #endif

  Current_CallbackInstance=Next_CallbackInstance;
  Next_CallbackInstance++;
  tempAudioObject=AudioObjects;
  AudioObjects = new DSPu_AudioOutput *[Next_CallbackInstance];
  AudioObjects[Current_CallbackInstance]=this;
  if (tempAudioObject != NULL)
  {
    for (ind=0; ind<Current_CallbackInstance; ind++)
    {
      AudioObjects[ind]=tempAudioObject[ind];
    }
    delete [] tempAudioObject;
  }
  StopPlaying = false;

  switch (BitPrec)
  {
    case 8:
      OutSampleType=DSP::e::SampleType::ST_uchar;
      break;
    case 16:
      OutSampleType=DSP::e::SampleType::ST_short;
      break;
    default:
      OutSampleType=DSP::e::SampleType::ST_short;
      BitPrec=16;
      break;
  }

  audio_outbuffer_size = DSP::f::GetAudioBufferSize(SamplingFreq, DSP_AB_OUT);

  #ifdef WINMMAPI
    //Wypeniamy struktur wfx
    wfx.wf.wFormatTag=WAVE_FORMAT_PCM;
    wfx.wf.nChannels=(WORD)NoOfInputs;
    wfx.wf.nSamplesPerSec=(UINT)SamplingFreq;
    wfx.wBitsPerSample=BitPrec;
    wfx.wf.nAvgBytesPerSec=wfx.wf.nSamplesPerSec*(wfx.wBitsPerSample/8);
    wfx.wf.nBlockAlign=(WORD)(wfx.wf.nChannels*(wfx.wBitsPerSample/8));

    if (WaveOutDevNo >= (UINT)waveOutGetNumDevs())
      result=waveOutOpen(&hWaveOut,
        WAVE_MAPPER, //&DeviceID,
        (WAVEFORMATEX *)(&wfx),
        Callback,
        Current_CallbackInstance, //CallbackInstance,
        CALLBACK_FUNCTION | WAVE_ALLOWSYNC | WAVE_FORMAT_DIRECT //| WAVE_MAPPED //CALLBACK_NULL
        );
    else
      result=waveOutOpen(&hWaveOut,
        WaveOutDevNo, //&DeviceID,
        (WAVEFORMATEX *)(&wfx),
        Callback,
        Current_CallbackInstance, //CallbackInstance,
        CALLBACK_FUNCTION | WAVE_ALLOWSYNC | WAVE_FORMAT_DIRECT //| WAVE_MAPPED //CALLBACK_NULL
        );
    if (DSP::f::AudioCheckError(result) == false)
    { // everything  is ok
      waveHeaderOut = new WAVEHDR[DSP_NoOfAudioOutputBuffers];
      WaveOutBufferLen=wfx.wf.nBlockAlign*audio_outbuffer_size;
      WaveOutBuffers = new uint8_t *[DSP_NoOfAudioOutputBuffers];
      for (ind=0; ind< DSP_NoOfAudioOutputBuffers; ind++)
      {
        WaveOutBuffers[ind] = new uint8_t[WaveOutBufferLen];
        memset(WaveOutBuffers[ind], 0, WaveOutBufferLen);
      }

      OutBufferLen=NoOfInputs*audio_outbuffer_size;
      OutBuffer = new DSP_float[OutBufferLen];
      memset(OutBuffer, 0, OutBufferLen*sizeof(DSP_float));
      BufferIndex=0;

      for (ind=0; ind< DSP_NoOfAudioOutputBuffers; ind++)
      {
        waveHeaderOut[ind].lpData=(char *)(WaveOutBuffers[ind]);
        waveHeaderOut[ind].dwBufferLength=OutBufferLen*(BitPrec/8); //sizeof(short);
        waveHeaderOut[ind].dwFlags= 0; // WHDR_BEGINLOOP | WHDR_ENDLOOP;
        waveHeaderOut[ind].dwLoops=0;

        result=waveOutPrepareHeader(hWaveOut,
          &(waveHeaderOut[ind]), sizeof(WAVEHDR));
        DSP::f::AudioCheckError(result);
        waveHeaderOut[ind].dwFlags= WHDR_DONE; // WHDR_BEGINLOOP | WHDR_ENDLOOP;
      }
    }
    else
    { //error while creating audio output
      waveHeaderOut = NULL;
      WaveOutBufferLen = 0;
      WaveOutBuffers = NULL;
      OutBuffer = NULL; OutBufferLen = 0;
      BufferIndex=0;
    }

  #else

    WaveOutBufferLen = 0;
    WaveOutBuffers = NULL;
    OutBuffer = NULL; OutBufferLen = 0;
    BufferIndex=0;

  #endif  // WINMMAPI

  IsPlayingNow = false;

  NextBufferInd=0;
}

/* ***************************************** */
/* ***************************************** */
DSPu_AudioInput::DSPu_AudioInput(
                 DSP::Clock_ptr ParentClock,
                 long int SamplingFreq,
                 unsigned int OutputsNo, //just one channel
                 char BitPrec,
                 unsigned int WaveInDevNo)
  : DSP::Source()
{
  Init(ParentClock, SamplingFreq, OutputsNo, BitPrec, WaveInDevNo);

  OutputExecute_ptr = &OutputExecute;
}


/* Inputs and Outputs names:
 *   - Output:
 *    -# "out" - real or complex
 *    -# "out.re" - first channel (real component)\n
 *       "out.im" - second channel (imag component if exists)
 *    -# "out1", "out2" - i-th channel output
 *   - Input: none
 */
void DSPu_AudioInput::Init(DSP::Clock_ptr ParentClock,
                           long int SamplingFreq,
                           unsigned int OutputsNo, //just one channel
                           char BitPrec,
                           unsigned int WaveInDevNo)
{
  unsigned long ind;
  DSPu_AudioInput **tempAudioObject;
  string temp;

  #ifdef WINMMAPI
    MMRESULT result;
    DWORD_PTR Callback;
  //Rezerwacja pamici dla formatu WAVE
  //  WAVEFORMATEX wfx; //to wymaga korekty
    PCMWAVEFORMAT wfx;
  #elif defined(ALSA_support_H)
    ALSA_object_t ALSA_object;

  #else
    UNUSED_ARGUMENT(WaveInDevNo);
  #endif

  SetNoOfOutputs(OutputsNo);
  SetName("AudioInput", false);

  if (OutputsNo == 1)
  {
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }
  if (OutputsNo == 2)
  {
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  for (unsigned int ind_i=0; ind_i<NoOfOutputs; ind_i++)
  {
    temp = "out" + to_string(ind_i+1);
    DefineOutput(temp, ind_i);
  }

  RegisterOutputClock(ParentClock);
  my_clock = ParentClock;

  Current_CallbackInstance=Next_CallbackInstance;
  Next_CallbackInstance++;
  tempAudioObject=AudioObjects;
  AudioObjects = new DSPu_AudioInput *[Next_CallbackInstance];
  AudioObjects[Current_CallbackInstance]=this;
  if (tempAudioObject != NULL)
  {
    for (ind=0; ind<Current_CallbackInstance; ind++)
    {
      AudioObjects[ind]=tempAudioObject[ind];
    }
    delete [] tempAudioObject;
  }
  StopRecording = false;

  switch (BitPrec)
  {
    case 8:
      InSampleType=DSP::e::SampleType::ST_uchar;
      #ifdef WINMMAPI
        Callback = (DWORD_PTR)(&DSPu_AudioInput::waveInProc_uchar);
      #endif
      break;
    case 16:
      InSampleType=DSP::e::SampleType::ST_short;
      #ifdef WINMMAPI
        Callback = (DWORD_PTR)(&DSPu_AudioInput::waveInProc_short);
      #endif
      break;
    default:
      InSampleType=DSP::e::SampleType::ST_short;
      #ifdef WINMMAPI
        Callback = (DWORD_PTR)(&DSPu_AudioInput::waveInProc_short);
      #endif
      BitPrec=16;
      break;
  }

  audio_inbuffer_size = DSP::f::GetAudioBufferSize(SamplingFreq, DSP_AB_IN);

  #ifdef WINMMAPI
    //Wypeniamy struktur wfx
    wfx.wf.wFormatTag=WAVE_FORMAT_PCM;
    wfx.wf.nChannels=(WORD)NoOfOutputs;
    wfx.wf.nSamplesPerSec=(UINT)SamplingFreq;
    wfx.wBitsPerSample=BitPrec;
    wfx.wf.nAvgBytesPerSec=wfx.wf.nSamplesPerSec*(wfx.wBitsPerSample/8);
    wfx.wf.nBlockAlign=(WORD)(wfx.wf.nChannels*(wfx.wBitsPerSample/8));

    if (waveInGetNumDevs() <= WaveInDevNo)
      result=waveInOpen(&hWaveIn,
        WAVE_MAPPER, //&DeviceID,
        (WAVEFORMATEX *)(&wfx),
        Callback,
        Current_CallbackInstance, //CallbackInstance,
        CALLBACK_FUNCTION | WAVE_FORMAT_DIRECT //| WAVE_MAPPED //CALLBACK_NULL
        );
    else
      result=waveInOpen(&hWaveIn,
        WaveInDevNo, //&DeviceID,
        (WAVEFORMATEX *)(&wfx),
        Callback,
        Current_CallbackInstance, //CallbackInstance,
        CALLBACK_FUNCTION | WAVE_FORMAT_DIRECT //| WAVE_MAPPED //CALLBACK_NULL
        );
    if (DSP::f::AudioCheckError(result) == false)
    { // no errors
      waveHeaderIn = new WAVEHDR[2];
      WaveInBufferLen=wfx.wf.nBlockAlign*audio_inbuffer_size;
      WaveInBuffers = new char *[2];
      /*! \bug <b>2006.08.13</b> when 8bit audio stream is created initial values should be 0x80 or 0x79 not 0x00
       */
      WaveInBuffers[0] = new char[WaveInBufferLen];
      memset(WaveInBuffers[0], 0, WaveInBufferLen);
      WaveInBuffers[1] = new char[WaveInBufferLen];
      memset(WaveInBuffers[1], 0, WaveInBufferLen);

      InBufferLen=NoOfOutputs*audio_inbuffer_size;
      for (ind = 0; ind < DSP_NoOfAudioInputBuffers; ind++)
      {
        InBuffers[ind] = new DSP_float[InBufferLen];
        memset(InBuffers[ind], 0, InBufferLen*sizeof(DSP_float));
      }
      EmptyBufferIndex=0;
      CurrentBufferIndex=DSP_NoOfAudioInputBuffers-1;
      BufferIndex=InBufferLen;

      waveHeaderIn[0].lpData=(char *)(WaveInBuffers[0]);
      waveHeaderIn[0].dwBufferLength=InBufferLen*(BitPrec/8); //sizeof(short);
      waveHeaderIn[0].dwFlags= 0; // WHDR_BEGINLOOP | WHDR_ENDLOOP;
      waveHeaderIn[0].dwLoops=0;

      result=waveInPrepareHeader(hWaveIn,
        &(waveHeaderIn[0]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);
    //  waveHeaderIn[0].dwFlags= WHDR_DONE; // WHDR_BEGINLOOP | WHDR_ENDLOOP;

      waveHeaderIn[1].lpData=(char *)(WaveInBuffers[1]);
      waveHeaderIn[1].dwBufferLength=InBufferLen*(BitPrec/8); //sizeof(short);
      waveHeaderIn[1].dwFlags= 0; // WHDR_BEGINLOOP | WHDR_ENDLOOP;
      waveHeaderIn[1].dwLoops=0;

      result=waveInPrepareHeader(hWaveIn,
        &(waveHeaderIn[1]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);
    //  waveHeaderIn[1].dwFlags= WHDR_DONE; // WHDR_BEGINLOOP | WHDR_ENDLOOP;
    }
    else
    { // error creating audio object
      waveHeaderIn = NULL; WaveInBufferLen=0;
      WaveInBuffers = NULL;

      InBufferLen=0;
      for (ind = 0; ind < DSP_NoOfAudioInputBuffers; ind++)
      {
        InBuffers[ind] = NULL;
      }
      EmptyBufferIndex=0; CurrentBufferIndex=0;
      BufferIndex=0;
    }

  #else

    InBufferLen=0;
    for (ind = 0; ind < DSP_NoOfAudioInputBuffers; ind++)
    {
      InBuffers[ind] = NULL;
    }
    EmptyBufferIndex=0; CurrentBufferIndex=0;
    BufferIndex=0;

  #endif

  IsRecordingNow = false; //wait until data requested

  NextBufferInd=0; //its overridden in Execute()
}


DSPu_AudioOutput::~DSPu_AudioOutput()
{
  #ifdef WINMMAPI
    MMRESULT result;
  #endif
  DSPu_AudioOutput **tempAudioObject;
  unsigned long ind;

  if (OutBufferLen != 0)
  { // if device was opened successfully
    StopPlaying=true;

    #ifdef WINMMAPI
      result = waveOutReset(hWaveOut);
      DSP::f::AudioCheckError(result);
      for (ind=0; ind< DSP_NoOfAudioOutputBuffers; ind++)
      {
        result=waveOutUnprepareHeader(hWaveOut,
          &(waveHeaderOut[ind]), sizeof(WAVEHDR));
        DSP::f::AudioCheckError(result);
      }

      #ifdef AUDIO_DEBUG_MESSAGES_ON
        DSP::log << "DSPu_AudioOutput" << DSP::LogMode::second << "Closing DSPu_AudioOutput" << endl;
      #endif
      result=waveOutClose(hWaveOut);
      while (result==WAVERR_STILLPLAYING)
      {
      //    #ifdef WINBASEAPI
        DSP::f::Sleep(100);
      //    #else
      //      sleep(100);
      //    #endif
        #ifdef AUDIO_DEBUG_MESSAGES_ON
          DSP::log << "DSPu_AudioOutput" << DSP::LogMode::second << "Closing DSPu_AudioOutput" << endl;
        #endif
        result=waveOutClose(hWaveOut);
      }
      DSP::f::AudioCheckError(result);
    #endif


    // 2) Free buffers
    for (ind=0; ind< DSP_NoOfAudioOutputBuffers; ind++)
    {
      if (WaveOutBuffers[ind] != NULL)
        delete [] WaveOutBuffers[ind];
    }
    if (WaveOutBuffers != NULL)
      delete [] WaveOutBuffers;
    #ifdef WINMMAPI
      delete [] waveHeaderOut;
    #endif

    if (OutBuffer != NULL)
      delete [] OutBuffer;
  }

  // *************************** //
  // Free local resourses
  // 3) remove this audio object from the list
  if (AudioObjects != NULL)
  {
    tempAudioObject=AudioObjects;
    if (Next_CallbackInstance>1)
    {
      AudioObjects = new DSPu_AudioOutput *[Next_CallbackInstance-1];
      for (ind=0; ind<Current_CallbackInstance; ind++)
        AudioObjects[ind]=tempAudioObject[ind];
      for (ind=Current_CallbackInstance; ind<Next_CallbackInstance; ind++)
      {
        AudioObjects[ind-1]=tempAudioObject[ind];
        AudioObjects[ind-1]->Current_CallbackInstance--;
      }
    }
    else
      AudioObjects = NULL;

    delete [] tempAudioObject;
    tempAudioObject = NULL;
  }
  Next_CallbackInstance--;
}

DSPu_AudioInput::~DSPu_AudioInput()
{
  #ifdef WINMMAPI
    MMRESULT result;
  #endif
  DSPu_AudioInput **tempAudioObject;
  unsigned long ind;

  if (InBufferLen != 0)
  { // if device was opened successfully
    StopRecording=true;

    #ifdef WINMMAPI
      result = waveInReset(hWaveIn);
      DSP::f::AudioCheckError(result);
      result=waveInUnprepareHeader(hWaveIn,
        &(waveHeaderIn[0]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);
      result=waveInUnprepareHeader(hWaveIn,
        &(waveHeaderIn[1]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);

      #ifdef AUDIO_DEBUG_MESSAGES_ON
        DSP::log << "DSPu_AudioInput" << DSP::LogMode::second << "Closing DSPu_AudioInput" << endl;
      #endif
      result=waveInClose(hWaveIn);
      while (result==WAVERR_STILLPLAYING)
      {
      //    #ifdef WINBASEAPI
        DSP::f::Sleep(100);
      //    #else
      //      sleep(100);
      //    #endif
        #ifdef AUDIO_DEBUG_MESSAGES_ON
          DSP::log << "DSPu_AudioInput" << DSP::LogMode::second << "Closing DSPu_AudioInput" << endl;
        #endif
        result=waveInClose(hWaveIn);
      }
      DSP::f::AudioCheckError(result);
    #endif

    // 2) Free buffers
    if (WaveInBuffers[0] != NULL)
      delete [] WaveInBuffers[0];
    if (WaveInBuffers[1] != NULL)
      delete [] WaveInBuffers[1];
    if (WaveInBuffers != NULL)
      delete [] WaveInBuffers;
    #ifdef WINMMAPI
      delete [] waveHeaderIn;
    #endif

    for (ind=0; ind<DSP_NoOfAudioInputBuffers; ind++)
      if (InBuffers[ind] != NULL)
        delete [] InBuffers[ind];
  }

  // *************************** //
  // Free local resourses
  // 3) remove this audio object from the list
  if (AudioObjects != NULL)
  {
    tempAudioObject=AudioObjects;
    if (Next_CallbackInstance>1)
    {
      AudioObjects = new DSPu_AudioInput *[Next_CallbackInstance-1];
      for (ind=0; ind<Current_CallbackInstance; ind++)
        AudioObjects[ind]=tempAudioObject[ind];
      for (ind=Current_CallbackInstance; ind<Next_CallbackInstance; ind++)
      {
        AudioObjects[ind-1]=tempAudioObject[ind];
        AudioObjects[ind-1]->Current_CallbackInstance--;
      }
    }
    else
      AudioObjects = NULL;

    delete [] tempAudioObject;
    tempAudioObject = NULL;
  }
  Next_CallbackInstance--;
}

void DSPu_AudioOutput::FlushBuffer(void)
{
  #ifdef WINMMAPI
    MMRESULT result;
    uint8_t *temp8;
    short *temp16;
    DSP_float_ptr Sample;
    short Znak;
    DWORD ind;

    // ************************************************** //
    // Send buffer to the audio device

  #ifdef AUDIO_DEBUG_MESSAGES_ON
    DSP::log << "DSPu_AudioOutput" << DSP::LogMode::second << "Flushing output buffer" << endl;
  #endif

  while (1)
  {
    if (waveHeaderOut[NextBufferInd].dwFlags & WHDR_DONE)
    {
      result=waveOutUnprepareHeader(hWaveOut,
        &(waveHeaderOut[NextBufferInd]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);

      Sample=OutBuffer;
      // ************************************************** //
      // Converts samples format to the one suitable for the audio device
      switch (OutSampleType)
      {
        case DSP::e::SampleType::ST_uchar:
          temp8=(uint8_t *)(WaveOutBuffers[NextBufferInd]);
          for (ind=0; ind<OutBufferLen; ind++)
          {
            if (*Sample < 0)
              Znak=-1;
            else
              Znak=1;

            *Sample*=127;
            if ((*Sample)*Znak > 127)
              *temp8=(unsigned char)(128+Znak*127);
            else
              *temp8=(unsigned char)(128+*Sample+Znak*0.5);

            Sample++;
            temp8++;
          }
          break;
        case DSP::e::SampleType::ST_short:
          temp16=(short *)(WaveOutBuffers[NextBufferInd]);
          for (ind=0; ind<OutBufferLen; ind++)
          {
            if (*Sample < 0)
              Znak=-1;
            else
              Znak=1;

            *Sample*=SHRT_MAX;
            if ((*Sample)*Znak > SHRT_MAX)
              *temp16=(short)(Znak*SHRT_MAX);
            else
              *temp16=(short)(*Sample+Znak*0.5);
            Sample++;
            temp16++;
          }
          break;
        default:
          break;
      }

      result=waveOutPrepareHeader(hWaveOut,
        &(waveHeaderOut[NextBufferInd]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);

      if (IsPlayingNow == false)
      {
        if (NextBufferInd == 1)
        {
          for (ind=0; ind < DSP_NoOfAudioOutputBuffers-1; ind++) //one spare buffer
          {
            result=waveOutWrite(hWaveOut,
              &(waveHeaderOut[ind]), sizeof(WAVEHDR));
            DSP::f::AudioCheckError(result);
          }
          IsPlayingNow = true;
        }

      }
      else
      {
        result=waveOutWrite(hWaveOut,
          &(waveHeaderOut[NextBufferInd]), sizeof(WAVEHDR));
        DSP::f::AudioCheckError(result);
      }
      NextBufferInd++;
      NextBufferInd %= DSP_NoOfAudioOutputBuffers;

      break;
    }
    else
    {
  //    Sleep(10);
  #ifdef AUDIO_DEBUG_MESSAGES_ON
      DSP::log << "DSPu_AudioOutput" << DSP::LogMode::second << "Waiting for free output buffer" << endl;
  #endif
      DSP::f::Sleep(0);
    }
  }
  #endif // WINMMAPI
}

void DSPu_AudioOutput::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (((DSPu_AudioOutput *)block)->OutBufferLen == 0)
    return; // no audio device opened

  ((DSPu_AudioOutput *)block)->OutBuffer[((DSPu_AudioOutput *)block)->BufferIndex * ((DSPu_AudioOutput *)block)->NoOfInputs+InputNo]=value;
  ((DSPu_AudioOutput *)block)->NoOfInputsProcessed++;

  if (((DSPu_AudioOutput *)block)->NoOfInputsProcessed == ((DSPu_AudioOutput *)block)->NoOfInputs)
  {
    ((DSPu_AudioOutput *)block)->BufferIndex++;
    ((DSPu_AudioOutput *)block)->BufferIndex %= ((DSPu_AudioOutput *)block)->audio_outbuffer_size;

    if (((DSPu_AudioOutput *)block)->BufferIndex == 0)
    { // Data must be written to file from buffer
      //First we need to convert data from RawBuffer
      //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
      ((DSPu_AudioOutput *)block)->FlushBuffer();
    }

    //NoOfInputsProcessed=0;
    if (((DSPu_AudioOutput *)block)->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < ((DSPu_AudioOutput *)block)->NoOfInputs; ind++)
        if (((DSPu_AudioOutput *)block)->IsConstantInput[ind])
        {
          ((DSPu_AudioOutput *)block)->OutBuffer[((DSPu_AudioOutput *)block)->BufferIndex * ((DSPu_AudioOutput *)block)->NoOfInputs + InputNo]=
            ((DSPu_AudioOutput *)block)->ConstantInputValues[ind];
          ((DSPu_AudioOutput *)block)->NoOfInputsProcessed++;
        }
    }
    ((DSPu_AudioOutput *)block)->NoOfInputsProcessed = ((DSPu_AudioOutput *)block)->InitialNoOfInputsProcessed;
  }

  #ifdef __DEBUG__
  #ifdef VerboseCompilation
    DSP::log << "DSPu_AudioOutput" << DSP::LogMode::second
      << InputNo << ": " << setw(5) << setprecision(3) << fixed << value << endl;
  #endif
  #endif
}

int DSPu_AudioInput::GetNoOfFreeBuffers(void)
{
  return (CurrentBufferIndex - EmptyBufferIndex + DSP_NoOfAudioInputBuffers) % DSP_NoOfAudioInputBuffers;
}

int DSPu_AudioInput::GetNoOfBuffers(void)
{
  return DSP_NoOfAudioInputBuffers;
}

#define DSP_THIS ((DSPu_AudioInput *)source)
/*! \Fixed <b>2005.10.21</b> cleared up start-up and congestion code
 */
bool DSPu_AudioInput::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  #ifdef WINMMAPI
    MMRESULT result;
  #endif
    unsigned int ind;

  if (DSP_THIS->InBufferLen == 0)
  { // no audio device opened
  	#ifdef AUDIO_DEBUG_MESSAGES_ON
      DSP::log << "DSPu_AudioInput::Execute" << DSP::LogMode::second << "No audio device opened" << endl;
  	#endif

    for (ind=0; ind < DSP_THIS->NoOfOutputs; ind++)
    {
//      OutputBlocks[ind]->Execute(OutputBlocks_InputNo[ind],
//                                 0.0, this);
      DSP_THIS->OutputBlocks[ind]->EXECUTE_PTR(
            DSP_THIS->OutputBlocks[ind], DSP_THIS->OutputBlocks_InputNo[ind],
            0.0, source);
      DSP_THIS->BufferIndex++;
    }
    return true;
  }


  // if no input buffer is ready return false (system will later return here)
  if (DSP_THIS->IsRecordingNow == true)
  { // but must check if data ready
    if ((((DSP_THIS->CurrentBufferIndex + 1) % DSP_NoOfAudioInputBuffers) == DSP_THIS->EmptyBufferIndex)
        && (DSP_THIS->BufferIndex == DSP_THIS->InBufferLen))
    { //no input samples are available
      // so we must wait for some

      #ifdef AUDIO_DEBUG_MESSAGES_ON
        DSP::log << "DSPu_AudioInput::Execute" << DSP::LogMode::second << "no data ready we must wait" << endl;
      #endif
		  DSP::Clock::InputNeedsMoreTime[DSP_THIS->my_clock->MasterClockIndex] = true;
		  return false;
    }
    else
    {
      if (DSP_THIS->BufferIndex == DSP_THIS->InBufferLen)
      { //should try to release buffer
        if (((DSP_THIS->CurrentBufferIndex + 1) % DSP_NoOfAudioInputBuffers) != DSP_THIS->EmptyBufferIndex)
        {
          DSP_THIS->BufferIndex = 0;

          DSP_THIS->CurrentBufferIndex++;
          DSP_THIS->CurrentBufferIndex %= DSP_NoOfAudioInputBuffers;

          #ifdef AUDIO_DEBUG_MESSAGES_ON
            DSP::log << "DSPu_AudioInput::Execute" << DSP::LogMode::second << "fresh data has finally arrived" << endl;
          #endif
        }
        else
        {
          /*! \bug problem if just started recording: we will need to wait
           *  till first buffer fills up
           */
          DSP::Clock::InputNeedsMoreTime[DSP_THIS->my_clock->MasterClockIndex] = true;
          return false;
        }
      }

      // output sample
      for (ind=0; ind < DSP_THIS->NoOfOutputs; ind++)
      {
//        OutputBlocks[ind]->Execute(OutputBlocks_InputNo[ind],
//                                   InBuffers[CurrentBufferIndex][BufferIndex], this);
        DSP_THIS->OutputBlocks[ind]->EXECUTE_PTR(
            DSP_THIS->OutputBlocks[ind], DSP_THIS->OutputBlocks_InputNo[ind],
            DSP_THIS->InBuffers[DSP_THIS->CurrentBufferIndex][DSP_THIS->BufferIndex], source);
        DSP_THIS->BufferIndex++;
      }

      return true; // samples have been generated
    }
  }
  else
  { // starts recording
    // using both buffers
    #ifdef WINMMAPI
      #ifdef AUDIO_DEBUG_MESSAGES_ON
        DSP::log << "DSPu_AudioInput::Execute" << DSP::LogMode::second << "Starting recording using two wave buffers" << endl;
      #endif

      result=waveInAddBuffer(DSP_THIS->hWaveIn,
            &(DSP_THIS->waveHeaderIn[0]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);
      result=waveInAddBuffer(DSP_THIS->hWaveIn,
            &(DSP_THIS->waveHeaderIn[1]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);

      DSP_THIS->NextBufferInd = 0; //first buffer first
      DSP_THIS->EmptyBufferIndex = 2; //simulate two full buffers
      for (ind = 0; ind < DSP_THIS->EmptyBufferIndex; ind++)
        memset(DSP_THIS->InBuffers[ind], 0, DSP_THIS->InBufferLen*sizeof(DSP_float));
      DSP_THIS->BufferIndex = 0;

      result=waveInStart(DSP_THIS->hWaveIn);
      DSP::f::AudioCheckError(result);
      DSP_THIS->IsRecordingNow = true;
      //now just wait for buffer to fill up
      // which should call waveInProc


      //We cannot wait until input buffers fill-up, some other processing
      // blocks like DSPu_AudioOutput might wait for their time
      /*! _bug if there are no DSPu_AudioOutput blocks nothing
       * will stop boost in processing speed
       *
       * we might want to simulated two full input buffers
       */
  //    #ifdef AUDIO_DEBUG_MESSAGES_ON
  //      DSP::log << "DSPu_AudioInput::Execute", "Audio recording started !!!");
  //    #endif
  //
  //    for (ind=0; ind<NoOfOutputs; ind++)
  //    {
  //      OutputBlocks[ind]->Execute(OutputBlocks_InputNo[ind],
  //                                 0.0, this);
  //      BufferIndex++;
  //    }
  //    return true;

    #endif

    // program will return to this function and then output should be dealt with
    DSP::Clock::InputNeedsMoreTime[DSP_THIS->my_clock->MasterClockIndex] = true;
    return false;
  }

//  DSP::log << "DSPu_AudioInput::Execute" << "No input buffer ready yet" << endl;
  DSP::Clock::InputNeedsMoreTime[DSP_THIS->my_clock->MasterClockIndex] = true;
  return false;
}
#undef DSP_THIS



//************************************************************//
// Source block providing input from the memory buffer
/* Feeds samples from the buffer to the connected blocks.
 *
 * Inputs and Outputs names:
 *   - Input: none
 *   - Output:
 *    -# "out1", "out2", ... - real
 *    -# "out.re" == "out1" - (real component)\n
 *       "out.im" == "out2" - (imag component if exist)
 *    -# "out" - all outputs together
 */
DSPu_InputBuffer::DSPu_InputBuffer(DSP::Clock_ptr ParentClock, int BufferSize_in,
                                   unsigned int NoOfChannels, DSPe_buffer_type cyclic,
                                   int NotificationsStep_in, DSPu_notify_callback_ptr func_ptr,
                                   unsigned int CallbackIdentifier)
  : DSP::Source()
{
  DSP::Clock_ptr NotificationClock;
  string temp;

  SetName("InputBuffer", false);

  if (NoOfChannels <=0)
    NoOfChannels=1;

  SetNoOfOutputs(NoOfChannels);
  if (NoOfChannels == 1)
  {
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
    DefineOutput("out1", 0);
  }
  if (NoOfChannels == 2)
  {
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    DefineOutput("out1", 0);
    DefineOutput("out2", 1);
  }
  if (NoOfChannels > 2)
  {
    unsigned int ind;
    vector <unsigned int> tempOut;
    tempOut.resize(NoOfChannels);
    for (ind=0; ind<NoOfChannels; ind++)
      tempOut[ind]=ind;
    DefineOutput("out", tempOut);

    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    for (ind=0; ind<NoOfChannels; ind++)
    {
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, ind);
    }
  }

  NotificationFunction_ptr = NULL;
  if (NotificationsStep_in == -1)
    NotificationsStep_in = BufferSize_in;

  NotificationsStep = NotificationsStep_in;
  NotificationFunction_ptr = func_ptr;
  UserCallbackID = (CallbackIdentifier & CallbackID_mask);

  if ((ParentClock != NULL) && (func_ptr != NULL))
  {
    if (NotificationsStep > 1)
      NotificationClock = DSP::Clock::GetClock(ParentClock, 1, NotificationsStep);
    else
      NotificationClock = ParentClock;

    RegisterForNotification(NotificationClock);
  }


  RegisterOutputClock(ParentClock);

  BufferSize=BufferSize_in;
  Buffer = new DSP_float[BufferSize*NoOfOutputs];
  BufferIndex=0;
  memset(Buffer, 0, sizeof(DSP_float)*BufferSize*NoOfOutputs);

  if (cyclic == DSP_cyclic)
  {
    if (NoOfOutputs == 1)
      OutputExecute_ptr = &OutputExecute_cyclic_single_channel;
    else
      OutputExecute_ptr = &OutputExecute_cyclic;
  }
  else
  {
    if (NoOfOutputs == 1)
      OutputExecute_ptr = &OutputExecute_single_channel;
    else
      OutputExecute_ptr = &OutputExecute;
  }

  if (NotificationFunction_ptr != NULL)
  	(*NotificationFunction_ptr)(this, CallbackID_signal_start | UserCallbackID);
}

DSPu_InputBuffer::~DSPu_InputBuffer(void)
{
  if (NotificationFunction_ptr != NULL)
  	(*NotificationFunction_ptr)(this, CallbackID_signal_stop | UserCallbackID);

  if (Buffer != NULL)
    delete [] Buffer;
}

void DSPu_InputBuffer::Notify(DSP::Clock_ptr clock)
{
  UNUSED_ARGUMENT(clock);

  if (NotificationFunction_ptr != NULL)
    (*NotificationFunction_ptr)(this, UserCallbackID);
}

// copies source_size bytes from the source buffer
// to block's internal buffer (the rest is set to zero
void DSPu_InputBuffer::WriteBuffer(void *source,
  long int source_size, DSP::e::SampleType source_DataType)
{
  int InputSampleSize;
  long int NoOfSourceSamples;
  int ind;

  switch (source_DataType)
  {
    case DSP::e::SampleType::ST_float:
      InputSampleSize=sizeof(float);
      break;
    case DSP::e::SampleType::ST_uchar:
      InputSampleSize=sizeof(unsigned char);
      break;
    case DSP::e::SampleType::ST_short:
    default:
      InputSampleSize=sizeof(short);
      break;
  }
  InputSampleSize*=NoOfOutputs;

  NoOfSourceSamples=source_size/InputSampleSize;
  #ifdef __DEBUG__
    if (NoOfSourceSamples*InputSampleSize != source_size)
    {
      DSP::log << DSP::LogMode::Error << "DSPu_InputBuffer::WriteBuffer" << DSP::LogMode::second
        << "(" << this->GetName() << ") source_size ("
        << source_size << ") doesn't match source_DataType" << endl;
    }
    if (BufferSize < NoOfSourceSamples)
    {
      stringstream tekst;
      DSP::log << DSP::LogMode::Error << "DSPu_InputBuffer::WriteBuffer" << DSP::LogMode::second
        << "(" << this->GetName() << ") source_size ("
        << source_size << ") larger then Buffer size\n" << endl;
    }
  #endif

  // Each sample component counts separately
  NoOfSourceSamples*=NoOfOutputs;
  if (BufferSize > NoOfSourceSamples)
  {
    memset(Buffer, 0, sizeof(DSP_float)*NoOfSourceSamples);
  }

  switch (source_DataType)
  {
    case DSP::e::SampleType::ST_uchar:
      //convertion
      for (ind=0; ind<NoOfSourceSamples; ind++)
        Buffer[ind]=(((DSP_float)((unsigned char  *)source)[ind]-0x80)
                    )/0x80;
      break;
    case DSP::e::SampleType::ST_short:
      //convertion
      for (ind=0; ind<NoOfSourceSamples; ind++)
        Buffer[ind]=((DSP_float)((short *)source)[ind])/0x8000;
      break;
    case DSP::e::SampleType::ST_float:
      //convertion
      for (ind=0; ind<NoOfSourceSamples; ind++)
        Buffer[ind]=((float *)source)[ind];
      break;
    default:
      break;
  }

}

#define DSP_THIS ((DSPu_InputBuffer *)source)
bool DSPu_InputBuffer::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);
  unsigned int ind;

  for (ind=0; ind < DSP_THIS->NoOfOutputs; ind++)
  {
    DSP_THIS->OutputBlocks[ind]->EXECUTE_PTR(
        DSP_THIS->OutputBlocks[ind],
        DSP_THIS->OutputBlocks_InputNo[ind],
        DSP_THIS->Buffer[DSP_THIS->BufferIndex], source);
    DSP_THIS->BufferIndex++;
  }

  DSP_THIS->BufferIndex %= (DSP_THIS->BufferSize * DSP_THIS->NoOfOutputs);
  if (DSP_THIS->BufferIndex == 0)
    memset(DSP_THIS->Buffer, 0,
           sizeof(DSP_float) * DSP_THIS->BufferSize * DSP_THIS->NoOfOutputs);

  return true;
}

bool DSPu_InputBuffer::OutputExecute_single_channel(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  DSP_THIS->OutputBlocks[0]->EXECUTE_PTR(
      DSP_THIS->OutputBlocks[0],
      DSP_THIS->OutputBlocks_InputNo[0],
      DSP_THIS->Buffer[DSP_THIS->BufferIndex], source);

  DSP_THIS->BufferIndex++;
  DSP_THIS->BufferIndex %= DSP_THIS->BufferSize;

  if (DSP_THIS->BufferIndex == 0)
    memset(DSP_THIS->Buffer, 0, sizeof(DSP_float) * DSP_THIS->BufferSize);

  return true;
}

bool DSPu_InputBuffer::OutputExecute_cyclic(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);
  unsigned int ind;

  for (ind=0; ind < DSP_THIS->NoOfOutputs; ind++)
  {
    DSP_THIS->OutputBlocks[ind]->EXECUTE_PTR(
        DSP_THIS->OutputBlocks[ind],
        DSP_THIS->OutputBlocks_InputNo[ind],
        DSP_THIS->Buffer[DSP_THIS->BufferIndex], source);
    DSP_THIS->BufferIndex++;
  }

  DSP_THIS->BufferIndex %= (DSP_THIS->BufferSize * DSP_THIS->NoOfOutputs);

  return true;
}

bool DSPu_InputBuffer::OutputExecute_cyclic_single_channel(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  DSP_THIS->OutputBlocks[0]->EXECUTE_PTR(
      DSP_THIS->OutputBlocks[0],
      DSP_THIS->OutputBlocks_InputNo[0],
      DSP_THIS->Buffer[DSP_THIS->BufferIndex], source);

  DSP_THIS->BufferIndex++;
  DSP_THIS->BufferIndex %= DSP_THIS->BufferSize;

  return true;
}
#undef DSP_THIS

// Block providing output to the memory buffer
/* Writes input samples from the connected blocks to the memory buffer.
 *
 * Inputs and Outputs names:
 *   - Input:
 *   - Input:
 *    -# "in1", "in2", ... - real
 *    -# "in.re" - (real component)\n
 *       "in.im" - (imag component if exist)
 *    -# "in" - all inputs together
 *   - Output: none
 */
DSPu_OutputBuffer::DSPu_OutputBuffer(unsigned int BufferSize_in, unsigned int NoOfInputs_in, DSPe_buffer_type cyclic,
                                     DSP::Clock_ptr ParentClock, int NotificationsStep_in,
                                     DSPu_notify_callback_ptr func_ptr, unsigned int CallbackIdentifier)
  : DSP::Block(), DSP::Source()
{
  DSP::Clock_ptr NotificationClock;

  Init(BufferSize_in, NoOfInputs_in, cyclic, NotificationsStep_in);

  NotificationFunction_ptr = func_ptr;
  CallbackFunction_ptr = NULL;
  UserData_ptr = NULL;
  UserCallbackID = (CallbackIdentifier & CallbackID_mask);

  if ((ParentClock != NULL) && (func_ptr != NULL))
  {
    if (NotificationsStep > 1)
      NotificationClock = DSP::Clock::GetClock(ParentClock, 1, NotificationsStep);
    else
      NotificationClock = ParentClock;

    RegisterForNotification(NotificationClock);
  }

  Execute_ptr = &InputExecute;

  if (NotificationFunction_ptr != NULL)
  	(*NotificationFunction_ptr)(this, CallbackID_signal_start | UserCallbackID);
}

DSPu_OutputBuffer::DSPu_OutputBuffer(unsigned int BufferSize_in, unsigned int NoOfInputs_in,
                                     DSPe_buffer_type cyclic, DSP::Clock_ptr ParentClock,
                                     int NotificationsStep_in, unsigned int NoOfOutputs_in,
                                     DSPu_buffer_callback_ptr func_ptr, unsigned int CallbackIdentifier)
  : DSP::Block(), DSP::Source()
{
  vector<unsigned int> tempOut;
  string temp;
  DSP::Clock_ptr OutputClock;

  Init(BufferSize_in, NoOfInputs_in, cyclic, NotificationsStep_in);

  ///////////////////////////////////////////////
  SetNoOfOutputs(NoOfOutputs_in);
  tempOut.resize(NoOfOutputs);
  for (unsigned int ind=0; ind<NoOfOutputs; ind++)
  {
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind);

    tempOut[ind]=ind;
  }
  DefineOutput("out", tempOut);

  ClockGroups.AddOutputs2Group("output", 0, NoOfOutputs-1);

  ///////////////////////////////////////////////
  if ((ParentClock != NULL) && (func_ptr != NULL))
  {
    if (NotificationsStep > 1)
    {
      IsMultirate = true;
//      L_factor = 1;
//      M_factor = NotificationsStep;
//
//      OutputClock = DSP::Clock::GetClock(ParentClock, L_factor, M_factor);
      OutputClock = DSP::Clock::GetClock(ParentClock, 1, NotificationsStep);

      ClockGroups.AddClockRelation("input","output", 1, NotificationsStep);
    }
    else {
      OutputClock = ParentClock;

      ClockGroups.AddClockRelation("input","output", 1, 1);
    }

    IsMultiClock = false;
    OutputClocks[0] = OutputClock;

    RegisterForNotification(OutputClock);
  }

  ///////////////////////////////////////////////
  OutputsValues = new DSP_float[NoOfOutputs];
  memset(OutputsValues, 0, NoOfOutputs*sizeof(DSP_float));

  NotificationFunction_ptr = NULL;
  CallbackFunction_ptr = func_ptr;
  UserData_ptr = NULL;
  UserCallbackID = CallbackIdentifier;

  Execute_ptr = &InputExecute_with_output;

  if (CallbackFunction_ptr != NULL)
  {
    (*CallbackFunction_ptr)(DSP_Callback_Init, NoOfOutputs, NULL, &UserData_ptr, UserCallbackID, this);
  }
}

DSPu_OutputBuffer::DSPu_OutputBuffer(unsigned int BufferSize_in, unsigned int NoOfInputs_in, DSPe_buffer_type cyclic,
                                     DSP::Clock_ptr ParentClock, DSP::Clock_ptr NotificationsClock,
                                     unsigned int NoOfOutputs_in, DSPu_buffer_callback_ptr func_ptr, unsigned int CallbackIdentifier)
  : DSP::Block(), DSP::Source()
{
  vector <unsigned int> tempOut;
  string temp;

  Init(BufferSize_in, NoOfInputs_in, cyclic, -1);

  ///////////////////////////////////////////////
  SetNoOfOutputs(NoOfOutputs_in);
  tempOut.resize(NoOfOutputs);
  for (unsigned int ind=0; ind<NoOfOutputs; ind++)
  {
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind);

    tempOut[ind]=ind;
  }
  DefineOutput("out", tempOut);

  ClockGroups.AddOutputs2Group("output", 0, NoOfOutputs-1);

  ///////////////////////////////////////////////
  long L_factor, M_factor;
  IsMultirate = GetMultirateFactorsFromClocks(ParentClock, NotificationsClock,
                                              L_factor, M_factor, false);
  ClockGroups.AddClockRelation("input","output", L_factor, M_factor);
  if (L_factor == 1)
    NotificationsStep = M_factor;
  else
    NotificationsStep = -1;

  if (func_ptr != NULL)
  {
    IsMultiClock = false;
    OutputClocks[0] = NotificationsClock;

    RegisterForNotification(NotificationsClock);
    RegisterOutputClock(NotificationsClock);
  }

  ///////////////////////////////////////////////
  OutputsValues = new DSP_float[NoOfOutputs];
  memset(OutputsValues, 0, NoOfOutputs*sizeof(DSP_float));

  NotificationFunction_ptr = NULL;
  CallbackFunction_ptr = func_ptr;
  UserData_ptr = NULL;
  UserCallbackID = (CallbackID_mask & CallbackIdentifier);

  Execute_ptr = &InputExecute;
  OutputExecute_ptr = &OutputExecute;

  if (CallbackFunction_ptr != NULL)
  {
    (*CallbackFunction_ptr)(DSP_Callback_Init, NoOfOutputs, NULL, &UserData_ptr, UserCallbackID, this);
  }
}

DSPu_OutputBuffer::~DSPu_OutputBuffer(void)
{
  if (NotificationFunction_ptr != NULL)
  	(*NotificationFunction_ptr)(this, CallbackID_signal_stop | UserCallbackID);
  if (CallbackFunction_ptr != NULL)
    (*CallbackFunction_ptr)(DSP_Callback_Delete, NoOfOutputs, NULL, &UserData_ptr, UserCallbackID, this);

  if (Buffer != NULL)
  {
    delete [] Buffer;
    Buffer = NULL;
  }
  if (OutputsValues != NULL)
  {
    delete [] OutputsValues;
    OutputsValues = NULL;
  }
}

void DSPu_OutputBuffer::Init(unsigned int BufferSize_in, unsigned int NoOfChannels,
                             DSPe_buffer_type cyclic, int NotificationsStep_in)
{
  string temp;

  SetName("OutputBuffer", false);

  if (NotificationsStep_in == -1)
    NotificationsStep_in = BufferSize_in;
  NotificationsStep = NotificationsStep_in;

  if (NoOfChannels <=0)
    NoOfChannels=1;

  SetNoOfInputs(NoOfChannels, true);
  if (NoOfChannels == 1)
  {
    DefineInput("in", 0);
    DefineInput("in.re", 0);
    DefineInput("in1", 0);
  }
  if (NoOfChannels == 2)
  {
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineInput("in1", 0);
    DefineInput("in2", 1);
  }
  if (NoOfChannels > 2)
  {
    unsigned int ind;
    vector <unsigned int> tempIn;
    tempIn.resize(NoOfChannels);
    for (ind=0; ind<NoOfChannels; ind++)
      tempIn[ind]=ind;
    DefineInput("in", tempIn);

    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    for (ind=0; ind<NoOfChannels; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, ind);
    }
  }

//  IsMultiClock=false;
  SetNoOfOutputs(0);

  ClockGroups.AddInputs2Group("input", 0, NoOfInputs-1);

  OutputsValues = NULL;
  OutputSamples_ready = false;

  switch (cyclic)
  {
    case DSP_cyclic:
      IsCyclic=true;
      StopWhenFull=false;
      break;
    case DSP_stop_when_full:
      IsCyclic=false;
      StopWhenFull=true;
      break;
    case DSP_standard:
    default:
      IsCyclic=false;
      StopWhenFull=false;
      break;
  }

  BufferSize=BufferSize_in;
  Buffer = new DSP_float[BufferSize*NoOfInputs];
  BufferIndex=0;
  memset(Buffer, 0, sizeof(DSP_float)*BufferSize*NoOfInputs);
}

// copies dest_size bytes to the dest buffer
// from block's internal buffer (the rest is set to zero)
long int DSPu_OutputBuffer::ReadBuffer(void *dest, long int dest_size,
                                       long int reset, DSP::e::SampleType dest_DataType)
{
  int OutputSampleSize;
  long int NoOfDestSamples;
  int ind;

  #ifdef __DEBUG__
    if (dest == NULL)
    {
      DSP::log << DSP::LogMode::Error << "DSPu_OutputBuffer::ReadBuffer" << DSP::LogMode::second << "dest == NULL !!!" << endl;
      return 0;
    }
  #endif

  switch (dest_DataType)
  {
    case DSP::e::SampleType::ST_float:
      OutputSampleSize=sizeof(float);
      break;
    case DSP::e::SampleType::ST_uchar:
      OutputSampleSize=sizeof(unsigned char);
      break;
    case DSP::e::SampleType::ST_short:
    default:
      OutputSampleSize=sizeof(short);
      break;
  }
  OutputSampleSize*=NoOfInputs;

  NoOfDestSamples=dest_size/OutputSampleSize;
  #ifdef __DEBUG__
    if (NoOfDestSamples*OutputSampleSize != dest_size)
    {
      DSP::log << DSP::LogMode::Error << "DSPu_OutputBuffer::ReadBuffer" << DSP::LogMode::second
         << "(" << this->GetName() << ") dest_size (" << dest_size << ") doesn't match dest_DataType" << endl;
    }
  #endif

  if (IsCyclic == false)
  { //! \todo <b>06.05.2012</b> Read cyclicaly last NoOfDestSamples of written samples
    if (NoOfDestSamples > BufferIndex)
      NoOfDestSamples = BufferIndex;
  }

  // Each sample component counts separately
  NoOfDestSamples*=NoOfInputs;

  switch (dest_DataType)
  {
    case DSP::e::SampleType::ST_uchar:
      //convertion
      for (ind=0; ind<NoOfDestSamples; ind++)
        ((uint8_t *)dest)[ind]
                     =(uint8_t)((Buffer[ind]*0x80)+0x80);
      if (dest_size-NoOfDestSamples > 0)
        memset((uint8_t *)dest+NoOfDestSamples, 0, dest_size-NoOfDestSamples);
      break;
    case DSP::e::SampleType::ST_short:
      //convertion
      for (ind=0; ind<NoOfDestSamples; ind++)
        ((short *)dest)[ind]=(short)(Buffer[ind]*0x8000);
      if (dest_size-sizeof(short)*NoOfDestSamples > 0)
        memset((short *)dest+NoOfDestSamples, 0, dest_size-sizeof(short)*NoOfDestSamples);
      break;
    case DSP::e::SampleType::ST_float:
      //convertion
      for (ind=0; ind<NoOfDestSamples; ind++)
        ((float *)dest)[ind]=(float)(Buffer[ind]);
      if (dest_size-sizeof(float)*NoOfDestSamples > 0)
        memset((float *)dest+NoOfDestSamples, 0, dest_size-sizeof(float)*NoOfDestSamples);
      break;
    default:
      NoOfDestSamples=0;
      break;
  }

  if (IsCyclic == true)
  {
    if (reset != 0)
      BufferIndex = 0;
  }
  else
  {
    // reset == 0; - no buffer reseting
    if (reset == -2) // - free just NotificationsStep slots in buffer
      reset = NotificationsStep;
    if (reset == -1) // - full buffer reset
      reset = BufferSize;
    if (reset > 0) // free only reset slots in buffer
      if (reset > BufferSize)
        reset = BufferSize;

    if (reset >= BufferSize)
      BufferIndex = 0;
    else
    { // push buffer to make place just for "reset" cycles
      //  BufferIndex - first empty
      //   256            256            128
      //   256 slots       256 filled       needed place for 128
      if (BufferSize - BufferIndex < reset)
      {
        DSP_float_ptr in, out;
        int size, step;

        // BufferIndex \ BufferSize
        //128       128                   256           256
        #ifdef __DEBUG__
          if (reset - (BufferSize - BufferIndex) > INT_MAX) {
            DSP::log << DSP::LogMode::Error << "DSPu_OutputBuffer::ReadBuffer" << DSP::LogMode::second << "step > INT_MAX" << endl;
          }
        #endif // __DEBUG__
        step = (int)(reset - (BufferSize - BufferIndex));

        out = Buffer; in = out + step*NoOfInputs;
        size = (int)(NoOfInputs*sizeof(DSP_float));
        for (ind = 0; ind < BufferIndex-step; ind++)
        {
          memcpy(out, in, size);
          in  += NoOfInputs;
          out += NoOfInputs;
        }
        BufferIndex -= step;
      }
    }
  }

  return  NoOfDestSamples/NoOfInputs;
}

DSP_float_ptr DSPu_OutputBuffer::AccessBuffer(void)
{
  return Buffer;
}


long int DSPu_OutputBuffer::NoOfSamples(void)
{
  return  BufferIndex; // BufferIndex/NoOfInputs;
}

long int DSPu_OutputBuffer::GetBufferSize(int mode)
{
  switch (mode)
  {
    case 2:
      return  (long int)(BufferSize*NoOfInputs*sizeof(DSP_float));
    case 1:
      return  BufferSize*NoOfInputs;
    case 0:
    default:
      return  BufferSize;
  }
}

void DSPu_OutputBuffer::Notify(DSP::Clock_ptr clock)
{
  UNUSED_ARGUMENT(clock);

  if (NotificationFunction_ptr != NULL)
    (*NotificationFunction_ptr)(this, UserCallbackID);

  if (CallbackFunction_ptr != NULL)
  {
    (*CallbackFunction_ptr)(NoOfInputs, NoOfOutputs, OutputsValues,
                            &(UserData_ptr), UserCallbackID, this);
    if (NoOfOutputs > 0)
      OutputSamples_ready = true;
  }
}

#define DSP_THIS ((DSPu_OutputBuffer *)block)
void DSPu_OutputBuffer::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->InitialNoOfInputsProcessed)
  {
    if (DSP_THIS->BufferIndex == DSP_THIS->BufferSize)
    {
      if (DSP_THIS->StopWhenFull == true)
        return;
      else
      { 
        if (DSP_THIS->IsCyclic == false)
        { // place for one input sample needed
          memcpy(DSP_THIS->Buffer, DSP_THIS->Buffer+DSP_THIS->NoOfInputs, (DSP_THIS->BufferSize-1)*DSP_THIS->NoOfInputs*sizeof(DSP_float));
          memset(DSP_THIS->Buffer+(DSP_THIS->BufferSize - 1) * DSP_THIS->NoOfInputs, 0, DSP_THIS->NoOfInputs*sizeof(DSP_float));
          DSP_THIS->BufferIndex--;
        }
      }
    }
  }

  DSP_THIS->Buffer[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs+InputNo] = value;
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    DSP_THIS->BufferIndex++;
    if (DSP_THIS->IsCyclic == true)
      DSP_THIS->BufferIndex %= DSP_THIS->BufferSize;
    //DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;


    // Prefilling with constant inputs values
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          DSP_THIS->Buffer[ DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs+InputNo] =
                  DSP_THIS->ConstantInputValues[ind];
          //DSP_THIS->NoOfInputsProcessed++;
        }
    }

    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

  }
}

void DSPu_OutputBuffer::InputExecute_with_output(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->InitialNoOfInputsProcessed)
  {
    if (DSP_THIS->BufferIndex == DSP_THIS->BufferSize)
    {
      if (DSP_THIS->StopWhenFull == true)
        return;
      else
      {
        if (DSP_THIS->IsCyclic == false)
        { // place for one input sample needed
          memcpy(DSP_THIS->Buffer, DSP_THIS->Buffer+DSP_THIS->NoOfInputs, (DSP_THIS->BufferSize-1)*DSP_THIS->NoOfInputs*sizeof(DSP_float));
          memset(DSP_THIS->Buffer+(DSP_THIS->BufferSize - 1) * DSP_THIS->NoOfInputs, 0, DSP_THIS->NoOfInputs*sizeof(DSP_float));
          DSP_THIS->BufferIndex--;
        }
      }
    }
  }

  DSP_THIS->Buffer[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs+InputNo] = value;
  DSP_THIS->NoOfInputsProcessed++;

  if (DSP_THIS->NoOfInputsProcessed == DSP_THIS->NoOfInputs)
  {
    DSP_THIS->BufferIndex++;
    if (DSP_THIS->IsCyclic == true)
      DSP_THIS->BufferIndex %= DSP_THIS->BufferSize;
    //DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;

    // Prefilling with constant inputs values
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          DSP_THIS->Buffer[ DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs+InputNo] =
                  DSP_THIS->ConstantInputValues[ind];
          //DSP_THIS->NoOfInputsProcessed++;
        }
    }
    DSP_THIS->NoOfInputsProcessed = DSP_THIS->InitialNoOfInputsProcessed;


    // output samples
    if (DSP_THIS->OutputSamples_ready == true)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfOutputs; ind++)
      {
        DSP_THIS->OutputBlocks[ind]->EXECUTE_PTR(
            DSP_THIS->OutputBlocks[ind],
            DSP_THIS->OutputBlocks_InputNo[ind],
            DSP_THIS->OutputsValues[ind], block);
      }
      DSP_THIS->OutputSamples_ready = false;
    }
  }
}
#undef DSP_THIS

#define  DSP_THIS  ((DSPu_OutputBuffer *)source)
//Execution as a source block
bool DSPu_OutputBuffer::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  // output samples
  if (DSP_THIS->OutputSamples_ready == true)
  {
    for (unsigned int ind=0; ind < DSP_THIS->NoOfOutputs; ind++)
    {
      DSP_THIS->OutputBlocks[ind]->EXECUTE_PTR(
           DSP_THIS->OutputBlocks[ind],
           DSP_THIS->OutputBlocks_InputNo[ind],
           DSP_THIS->OutputsValues[ind], source);
    }
    DSP_THIS->OutputSamples_ready = false;
  }
  #ifdef __DEBUG__
  else
  {
    DSP::log << DSP::LogMode::Error << "DSPu_OutputBuffer::OutputExecute" << DSP::LogMode::second << "DSP_THIS->OutputSamples_ready == false" << endl;
  }
  #endif

  return true;
}
#undef DSP_THIS

// ************************************* //
class DSPi_ExternalSleep
{
  private:
    friend void DSP::f::SetSleepFunction(DSP::ExternalSleep_ptr new_function);

    DSP::ExternalSleep_ptr tmp_external_function;
  public:
    void Sleep(DWORD time)
    {
      if (tmp_external_function == NULL)
      {
        #ifdef WINUSERAPI
          if (time == 0)
          {
            MSG msg;
            PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );
          }
          else
          {
          	/*! \Fixed <b>2005.10.05</b> function called itself instead ::Sleep()
          	 */
            ::Sleep(time);
          }
        #else
          #ifndef __MINGW32__
            //#error DSP::f::Sleep not implemented on this platform
            timespec timeOUT, remains;

            timeOUT.tv_sec = time / 1000;
            timeOUT.tv_nsec = (time - 1000*timeOUT.tv_sec) * 1000000;
            nanosleep(&timeOUT, &remains);
          #else
            #warning There is no sleep implementation for __MINGW32__ without WIN32
          #endif // __MINGW32__
        #endif
      }
      else
        (*tmp_external_function)(time);
    }

    DSPi_ExternalSleep(void)
    {
      tmp_external_function = NULL;
    }
} DSPo_SleepObject;

void DSP::f::SetSleepFunction(DSP::ExternalSleep_ptr new_function)
{
  DSPo_SleepObject.tmp_external_function = new_function;
}

void DSP::f::Sleep(DWORD time)
{
  DSPo_SleepObject.Sleep(time);

/*
  #ifdef WINUSERAPI
    if (time == 0)
    {
//      WaitMessage();
      MSG msg;
      PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );
    }
    else
      Sleep(time);
/ *
    DWORD result;

    result = MsgWaitForMultipleObjects(
      1, //DWORD nCount,
      const HANDLE* pHandles,
      false, //BOOL bWaitAll,
      time, //DWORD dwMilliseconds,
      QS_ALLEVENTS | QS_ALLINPUT | QS_ALLPOSTMESSAGE | QS_PAINT | QS_RAWINPUT | QS_SENDMESSAGE |QS_TIMER // DWORD dwWakeMask
      );
* /
  #else
    #error DSP::f::Sleep not implemented on this platform
  #endif
*/
}

/* ************************************************* */
/*                                                   */
/* ************************************************* */
/*! Checks Coefficients file type and fills
 *  DSP_LoadCoef_Info structure.
 */
bool DSP::LoadCoef::Open(const string &Filename, const string &Dir)
{
  FILE *plik;
  DWORD ValuesRead;
  unsigned char temp_uchar;

  this->filename = Dir;
  if ((this->filename.back() != '/') || (this->filename.back() != '\\')) {
    this->filename += '/';
  }
  this->filename +=Filename;

  this->file_version = 0xff;
  this->NoOfVectors = -1;
  this->Fp = 0;
  this->type = DSP::e::LoadCoef_Type::error;

  plik=fopen(this->filename.c_str(), "rb");
  if (plik != NULL)
  {
    ValuesRead=(DWORD)fread(&(this->file_version), sizeof(unsigned char), 1, plik);
    this->header_size = int(ValuesRead * sizeof(unsigned char));

    switch (this->file_version)
    {
      case 0x00:
        ValuesRead=(DWORD)fread(&(this->sample_dim), sizeof(unsigned char), 1, plik);
        this->header_size += ValuesRead * (int)sizeof(unsigned char);

        ValuesRead=(DWORD)fread(&(temp_uchar), sizeof(unsigned char), 1, plik);
        this->sample_type = (DSP::e::SampleType)temp_uchar;
        this->header_size += ValuesRead * (int)sizeof(unsigned char);

        ValuesRead=(DWORD)fread(&(temp_uchar), sizeof(unsigned char), 1, plik);
        this->NoOfVectors = temp_uchar;
        this->header_size += ValuesRead * (int)sizeof(unsigned char);
        break;
      case 0x01:
        ValuesRead=(DWORD)fread(&(this->Fp),sizeof(unsigned int), 1, plik);
        this->header_size += ValuesRead * (int)sizeof(unsigned int);

        ValuesRead=(DWORD)fread(&(this->sample_dim), sizeof(unsigned char), 1, plik);
        this->header_size += ValuesRead * (int)sizeof(unsigned char);

        ValuesRead=(DWORD)fread(&(temp_uchar), sizeof(unsigned char), 1, plik);
        this->sample_type = (DSP::e::SampleType)temp_uchar;
        this->header_size += ValuesRead * (int)sizeof(unsigned char);

        ValuesRead=(DWORD)fread(&(temp_uchar), sizeof(unsigned char), 1, plik);
        this->NoOfVectors = temp_uchar;
        this->header_size += ValuesRead * (int)sizeof(unsigned char);
        break;

      default:
        this->file_version = 0xff;
        break;
    }

    // info.type, ... update
    this->sample_size = DSP::f::SampleType2SampleSize(this->sample_type);
    if (this->sample_dim == 1)
      this->type = DSP::e::LoadCoef_Type::real;
    if (this->sample_dim == 2)
      this->type = DSP::e::LoadCoef_Type::complex;
    if (this->NoOfVectors == 1)
      this->type = this->type | DSP::e::LoadCoef_Type::FIR;
    if (this->NoOfVectors == 2)
      this->type = this->type | DSP::e::LoadCoef_Type::IIR;

    fclose(plik);
  }

  if (this->NoOfVectors == -1)
  {
    DSP::log << DSP::LogMode::Error << "DSP_LoadCoef::Open" << DSP::LogMode::second << "File doesn't exist or unsupported file format." << endl;
    return false;
  }

  return true;
}

// Returns number of vectors in file
int DSP::LoadCoef::GetNoOfVectors(void)
{
  return NoOfVectors;
}

// Checks Coefficients size for given vector
/* vector_no = 0 or 1
 *   - 0 - numerator coefficients (FIR or IIR filters)
 *   - 1 - denominator coefficients (only IIR filters)
 */
int DSP::LoadCoef::GetSize(int vector_no)
{
  FILE *plik;
  DWORD ValuesRead;
  unsigned short vector_size;
  int current_vector_no;

  current_vector_no = -1;
  vector_size = 0;

  plik=fopen(this->filename.c_str(), "rb");
  if (plik != NULL)
  {
//    if (fseeko64(plik, this->header_size, SEEK_SET)==0)
    if (fseek(plik, this->header_size, SEEK_SET)==0)
    {
      do
      {
        ValuesRead=(DWORD)fread(&(vector_size), sizeof(unsigned short), 1, plik);

        if (ValuesRead == 0)
        {
          current_vector_no = -1;
          vector_size = 0;
          break;
        }
        else
        {
          /* skip vector data */
//          if (fseeko64(plik, (vector_size * this->sample_size * this->sample_dim), SEEK_CUR) != 0)
          if (fseek(plik, (vector_size * this->sample_size * this->sample_dim), SEEK_CUR) != 0)
          {
            current_vector_no = -1;
            vector_size = 0;
            break;
          }
          current_vector_no++;
        }
      }
      while (current_vector_no < vector_no);

    }
    fclose(plik);
  }

  if (current_vector_no == vector_no)
    return vector_size;
  return -1;
}

// Loads FIR filter complex coefficients
/*
 *  \param FIR_coef - pointer to vector of given size
 *  \param size - numer for coefficients to load
 *
 * Returns FALSE if file type or size mismatch is detected.
 */
bool DSP::LoadCoef::Load(DSP_complex_vector &FIR_coef, int vector_index)
{
  FILE *plik;
  uint8_t *data;
  DWORD ValuesRead, Values2Read;
  int current_vector_no;

  current_vector_no = 0;
  Values2Read = 0;

  plik=fopen(this->filename.c_str(), "rb");
  if (plik != NULL)
  {
//    if (fseeko64(plik, this->header_size, SEEK_SET)==0)
    if (fseek(plik, this->header_size, SEEK_SET)==0)
    {
      ValuesRead=(DWORD)fread(&(Values2Read), sizeof(unsigned short), 1, plik);
      if (ValuesRead == 0)
        return false;

      while (current_vector_no < vector_index)
      { // skip preceeding vectors
//        fseeko64(plik, vector_size * this->sample_size * this->sample_dim, SEEK_CUR);
        fseek(plik, Values2Read * this->sample_size * this->sample_dim, SEEK_CUR);
        current_vector_no++;

        ValuesRead=(DWORD)fread(&(Values2Read), sizeof(unsigned short), 1, plik);
        if (ValuesRead == 0)
          return false;
      }

      FIR_coef.resize(Values2Read);
      data = new uint8_t[this->sample_size * this->sample_dim];
      for (unsigned int ind = 0; ind < Values2Read; ind++)
      {
        ValuesRead=(DWORD)fread(data, this->sample_size*sizeof(unsigned char), this->sample_dim, plik);

        switch (this->sample_type)
        {
          case DSP::e::SampleType::ST_float:
            FIR_coef[ind] = DSP_complex(((float *)data)[0], ((float *)data)[1]);
            break;
          case DSP::e::SampleType::ST_double:
            FIR_coef[ind] = DSP_complex(((double *)data)[0], ((double *)data)[1]);
            break;
          case DSP::e::SampleType::ST_long_double:
            FIR_coef[ind] = DSP_complex(((long double *)data)[0], ((long double *)data)[1]);
            break;
          default:
            FIR_coef[ind] = DSP_complex(0.0, 0.0);
            break;
        }
      }
      delete [] data;
    }
    fclose(plik);
    return true;
  }

  return false;
}
// Loads FIR filter real coefficients
/*
 *  \param FIR_coef - pointer to vector of given size
 *  \param size - numer for coefficients to load
 *
 * Returns FALSE if file type or size mismatch is detected.
 */
bool DSP::LoadCoef::Load(DSP_float_vector &FIR_coef, int vector_index)
{
  FILE *plik;
  uint8_t *data;
  DWORD ValuesRead, Values2Read;
  int current_vector_no;

  current_vector_no = 0;
  Values2Read = 0;

  plik=fopen(this->filename.c_str(), "rb");
  if (plik != NULL)
  {
//    if (fseeko64(plik, this->header_size, SEEK_SET)==0)
    if (fseek(plik, this->header_size, SEEK_SET)==0)
    {
      ValuesRead=(DWORD)fread(&(Values2Read), sizeof(unsigned short), 1, plik);
      if (ValuesRead == 0)
        return false;

      while (current_vector_no < vector_index)
      { // skip preceding vectors
//        fseeko64(plik, vector_size * this->sample_size * this->sample_dim, SEEK_CUR);
        fseek(plik, Values2Read * this->sample_size * this->sample_dim, SEEK_CUR);
        current_vector_no++;

        ValuesRead=(DWORD)fread(&(Values2Read), sizeof(unsigned short), 1, plik);
        if (ValuesRead == 0)
          return false;
      }

      FIR_coef.resize(Values2Read);
      data = new uint8_t[this->sample_size * this->sample_dim];
      for (unsigned int ind = 0; ind < Values2Read; ind++)
      {
        ValuesRead=(DWORD)fread(data, this->sample_size*sizeof(unsigned char), this->sample_dim, plik);

        switch (this->sample_type)
        {
          case DSP::e::SampleType::ST_float:
            FIR_coef[ind] = DSP_float(((float *)data)[0]);
            break;
          case DSP::e::SampleType::ST_double:
            FIR_coef[ind] = DSP_float(((double *)data)[0]);
            break;
          case DSP::e::SampleType::ST_long_double:
            FIR_coef[ind] = DSP_float(((long double *)data)[0]);
            break;
          default:
            FIR_coef[ind] = DSP_float(0.0);
            break;
        }
      }
      delete [] data;
    }
    fclose(plik);
    return true;
  }

  return false;
}

//!
FILE * CreateFLTfile(const string &filename, const unsigned int &NoOfChannels = 1U, const unsigned int &Fp=0)
{
  FILE *FileHandle;

  T_FLT_header header_flt;
  header_flt.version(0x00);
  header_flt.sample_type(0x0000); // floating point: float (4B)
  header_flt.sampling_rate(Fp);
  header_flt.no_of_channels((unsigned char)NoOfChannels);

  FileHandle=fopen(filename.c_str(), "wb");
  if (FileHandle != NULL)
    fwrite(&header_flt, sizeof(T_FLT_header), 1, FileHandle);

  return FileHandle;
}

// Saves to *.flt file len samples from real valued vector
bool DSP::f::SaveVector(const string &filename, const DSP_float_vector &vector, const unsigned int &Fp)
{
  FILE *FileHandle = CreateFLTfile(filename, 1U, Fp);

  if (FileHandle != NULL)
  {
    fwrite(vector.data(), sizeof(DSP_float), vector.size(), FileHandle);
//    fflush(FileHandle);
    fclose(FileHandle);

    return true;
  }
  return false;
}
// Saves to *.flt file len samples from complex valued vector
bool DSP::f::SaveVector(const string &filename, const DSP_complex_vector &vector, const unsigned int &Fp)
{
  FILE *FileHandle = CreateFLTfile(filename, 2U, Fp);

  if (FileHandle != NULL)
  {
    fwrite(vector.data(), 2*sizeof(DSP_float), vector.size(), FileHandle);
//    fflush(FileHandle);
    fclose(FileHandle);

    return true;
  }
  return false;
}


