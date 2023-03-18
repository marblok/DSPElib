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
#include <functional> 

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
unsigned long DSP::f::ReadCoefficientsFromFile(DSP::Float_vector &Buffer, unsigned long N,
                     const string &FileName, const string &FileDir,
                     DSP::e::SampleType type,
                     unsigned long offset)
{
  FILE *plik;
  string file_name;
  uint32_t ValuesRead;
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
    Buffer.resize(N);
    if (fseek(plik, offset, SEEK_SET)==0)
    {
      switch (type)
      {
        case DSP::e::SampleType::ST_uchar:
          //reading
          {
            vector<uint8_t> tempBuffer;
            tempBuffer.resize(N);
            ValuesRead=(uint32_t)fread(tempBuffer.data(), sizeof(uint8_t), N, plik);
            //convertion
            for (ind=0; ind<ValuesRead; ind++)
              Buffer[ind]=((DSP::Float)(tempBuffer[ind]-80))/0x80;
          }
          break;
        case DSP::e::SampleType::ST_short:
          //reading
          {
            vector<short> tempBuffer;
            tempBuffer.resize(N);
            ValuesRead=(uint32_t)fread(tempBuffer.data(), sizeof(short), N, plik);
            //convertion
            for (ind=0; ind<ValuesRead; ind++)
              Buffer[ind]=((DSP::Float)(tempBuffer[ind]))/0x8000;
          }
          break;
        case DSP::e::SampleType::ST_float:
          //reading
          {
            ValuesRead=(uint32_t)fread(Buffer.data(), sizeof(float), N, plik);
            //no need for convertion
          }
          break;
        default:
          ValuesRead=0;
          break;
      }
    }
    #ifdef __DEBUG__
      else
      {
        DSP::log << DSP::e::LogMode::Error << "DSP::f::ReadCoefficientsFromFile" << DSP::e::LogMode::second << "ReadCoefficientsFromFile wrong offset" << endl;
      }
    #endif
    fclose(plik);
  }
  return ValuesRead;
}

//*****************************************************//
unsigned long DSP::f::ReadCoefficientsFromFile(DSP::Complex_vector &Buffer, unsigned long N,
                     const string &FileName, const string &FileDir,
                     DSP::e::SampleType type,
                     unsigned long offset)
{
  FILE *plik;
  string file_name;
  uint32_t ValuesRead;
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
    Buffer.resize(N);
    if (fseek(plik, offset, SEEK_SET)==0)
    {
      switch (type)
      {
        case DSP::e::SampleType::ST_uchar:
          //reading
          {
            vector<uint8_t> tempBuffer;
            tempBuffer.resize(2*N);
            ValuesRead=(uint32_t)fread(tempBuffer.data(), sizeof(uint8_t), 2*N, plik);
            //convertion
            for (ind=0; ind<ValuesRead; ind=ind+2) {
              Buffer[ind].set(
                ((DSP::Float)(tempBuffer[ind]-80))/0x80,
                ((DSP::Float)(tempBuffer[ind+1]-80))/0x80
              );
            }
          }
          break;
        case DSP::e::SampleType::ST_short:
          //reading
          {
            vector<short> tempBuffer;
            tempBuffer.resize(2*N);
            ValuesRead=(uint32_t)fread(tempBuffer.data(), sizeof(short), 2*N, plik);
            //convertion
            for (ind=0; ind<ValuesRead; ind=ind+2) {
              Buffer[ind].set(
                ((DSP::Float)(tempBuffer[ind]))/0x8000,
                ((DSP::Float)(tempBuffer[ind+1]))/0x8000
              );
            }
          }
          break;
        case DSP::e::SampleType::ST_float:
          //reading
          {
            vector<DSP::Float> tempBuffer;
            tempBuffer.resize(2*N);
            ValuesRead=(uint32_t)fread(Buffer.data(), sizeof(float), 2*N, plik);
            // convertion
            for (ind=0; ind<ValuesRead; ind=ind+2) {
              Buffer[ind]=DSP::Complex(tempBuffer[ind], tempBuffer[ind+1]);
            }
          }
          break;
        default:
          ValuesRead=0;
          break;
      }
    }
    #ifdef __DEBUG__
      else
      {
        DSP::log << DSP::e::LogMode::Error << "DSP::f::ReadCoefficientsFromFile" << DSP::e::LogMode::second << "ReadCoefficientsFromFile wrong offset" << endl;
      }
    #endif
    fclose(plik);
  }
  return ValuesRead;
}

//*****************************************************//
//*****************************************************//
//    unsigned char  version;        1B
unsigned char DSP::T_FLT_header::version(void)
{
  return data[0];
}
void DSP::T_FLT_header::version(unsigned char val)
{
  data[0] = val;
}
//    unsigned short sample_type;    2B
unsigned short DSP::T_FLT_header::sample_type(void)
{
  return *((unsigned short *)(data + 1));
}
void DSP::T_FLT_header::sample_type(unsigned short val)
{
  *((unsigned short *)(data+1)) = val;
}
//    unsigned char  no_of_channels; 1B
unsigned char DSP::T_FLT_header::no_of_channels(void)
{
  return data[3];
}
void DSP::T_FLT_header::no_of_channels(unsigned char val)
{
  data[3] = val;
}
//    unsigned int   sampling_rate;  4B
unsigned int DSP::T_FLT_header::sampling_rate(void)
{
  return *((unsigned int *)(data + 4));
}
void DSP::T_FLT_header::sampling_rate(unsigned int val)
{
  *((unsigned int *)(data+4)) = val;
}

//*****************************************************//
//*****************************************************//
DSP::u::Vacuum::Vacuum(unsigned int NoOfInputs_in)
  : DSP::Block()
{
  Init(false, NoOfInputs_in);

  Execute_ptr = &InputExecute;
}

DSP::u::Vacuum::Vacuum(bool AreInputsComplex, unsigned int NoOfInputs_in)
  : DSP::Block()
{
  Init(AreInputsComplex, NoOfInputs_in);

  Execute_ptr = &InputExecute;
}

void DSP::u::Vacuum::Init(bool AreInputsComplex, unsigned int NoOfInputs_in)
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

DSP::u::Vacuum::~Vacuum()
{
}

void DSP::u::Vacuum::InputExecute(INPUT_EXECUTE_ARGS)
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
DSP::u::FileInput::FileInput(DSP::Clock_ptr ParentClock,
                  const string &FileName,
                  unsigned int NoOfChannels,
                  DSP::e::SampleType sample_type,
                  DSP::e::FileType FILEtype)
  : DSP::File(), DSP::Source()
{
  string temp;
  bool ready;

  SetName("FileInput", false);

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  RawBuffer.clear();
  flt_header.clear(); tape_header.clear(); wav_header.clear();
  FileHandle = NULL;

  ready = OpenFile(FileName, sample_type, FILEtype, NoOfChannels);

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  if (NoOfChannels == 0)
  {
    if (FILEtype == DSP::e::FileType::FT_raw)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second
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
  //Data are stored in buffer in DSP::Float format
  Buffer.clear();
  Buffer.resize(NoOfOutputs*DSP::File_buffer_size, 0.0);
  BufferIndex=0; //this means also that Buffer is empty
  // +++++++++++++++++++++++++++++++++++++++++++++ //


  if ((FILEtype == DSP::e::FileType::FT_tape) &&  (NoOfOutputs != 2))
  {
    //NoOfChannels = 2;
    //! \bug support for reading files with different number of channels then number of block's outputs
    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second << "Wrong number of channels for *.tape file !!!" << endl;
    #endif
    ready = false;
  }

  if (NoOfOutputs != NoOfFileChannels)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second
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
    DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput" << DSP::e::LogMode::second
      << "Block <" << GetName() << "> failed to open file \"" << FileName << "\"" << endl;
    OutputExecute_ptr = &OutputExecute_Dummy;
  }
  else
    OutputExecute_ptr = &OutputExecute;
}

bool DSP::u::FileInput::SetSkip(long long Offset)
{
  UNUSED_ARGUMENT(Offset);

  #ifdef __DEBUG__
    DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::SetSkip" << DSP::e::LogMode::second << "not implemented yet" << endl;
  #endif
  return false;
}

bool DSP::u::FileInput::OpenFile(const string &FileName,
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
          flt_header.resize(1);
          BytesRead = (unsigned int)fread(&(flt_header[0]), 1, DSP::FLT_header_LEN, FileHandle);

          if (flt_header[0].version() != 0)
          {
            ready = false;

            #ifdef __DEBUG__
              DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second << "Unsupported *.flt file version !!!" << endl;
              //! \todo verify if file header data match constructor parameters
            #endif
          }

          NoOfFileChannels = flt_header[0].no_of_channels();
          SamplingRate = flt_header[0].sampling_rate();

          //! \TODO determine sample type based on flt_header[0].sample_type()
        }
        break;

      case DSP::e::FileType::FT_tape:
        {
          tape_header.resize(1);
          BytesRead = (unsigned int)fread(&(tape_header[0]), 1, DSP::TAPE_header_LEN, FileHandle);

          NoOfFileChannels = 2;
          if (SampleType != DSP::e::SampleType::ST_short)
          {
            SampleType = DSP::e::SampleType::ST_short;
            ready = false;

            #ifdef __DEBUG__
              DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second << "Unsupported *.tape file sample type!!!" << endl;
            #endif
          }

          #ifdef __DEBUG__
            if (sizeof(DSP::T_TAPE_header) != DSP::TAPE_header_LEN)
              DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second << "TAPE_header_LEN does not much sizeof T_TAPE_header structure !!!" << endl;
            if (tape_header[0].header_size() != DSP::TAPE_header_LEN)
              DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second << "Unsupported *.tape file version !!!" << endl;
          #endif

          SamplingRate = tape_header[0].sampling_rate();
        }
        break;

      case DSP::e::FileType::FT_wav:
        {
          wav_header.resize(1);

          //Read WAVE file header
          if (wav_header[0].WAVEinfo(FileHandle)==false)
          {
            ready = false;

            #ifdef __DEBUG__
            {
              DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second
                << "This (" << FileName << ") is not PCM WAVE file or file is corrupted !!!" << endl;
            }
            #endif
          }

          NoOfFileChannels = wav_header[0].nChannels;

          switch (wav_header[0].wBitsPerSample)
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
                DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second
                  << "Unsupported PCM sample size ==> " << wav_header[0].wBitsPerSample << " bits!!!" << endl;
              }
              #endif
              break;
          }

          SamplingRate = wav_header[0].nSamplesPerSec;

          if (wav_header[0].FindDATA(FileHandle)==false)
          {
            ready = false;
            #ifdef __DEBUG__
            {
              DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::FileInput" << DSP::e::LogMode::second
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

  RawBuffer.clear();

  SampleSize = GetSampleSize(SampleType);
  RawBuffer.resize(GetRawBufferSize() + 1);
  BytesRead=0;

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  if (ready == false)
    OutputExecute_ptr = &OutputExecute_Dummy;
  else
    OutputExecute_ptr = &OutputExecute;

  return ready;
}

bool DSP::u::FileInput::CloseFile(void)
{
  int res = -1;

  flt_header.clear();
  tape_header.clear();
  wav_header.clear();

  RawBuffer.clear();

  if (FileHandle != NULL)
  {
    res = fclose(FileHandle);
    FileHandle = NULL;
  }

  return (res == 0);
}

//! needed to get function DSP::u::FileInput::GetHeader instances into library
void dummy_GetHeader(void)
{
  DSP::u::FileInput temp(NULL, "");

  temp.GetHeader<DSP::T_TAPE_header>();
  temp.GetHeader<DSP::T_FLT_header>();
  temp.GetHeader<DSP::T_WAVEchunk>();
}

template <class T>
T *DSP::u::FileInput::GetHeader(const unsigned int &index)
{
  std::vector<T> temp;

  if (typeid(temp) == typeid(flt_header))
  {
    if (flt_header.size() > index)
      return (T *)(&(flt_header[index]));
  }
  if (typeid(temp) == typeid(wav_header))
  {
    if (wav_header.size() > index)
      return (T *)(&(wav_header[index]));
  }
  if (typeid(temp) == typeid(tape_header))
  {
    if (tape_header.size() > index)
      return (T *)(&(tape_header[index]));
  }
  return NULL; 
}
template DSP::T_FLT_header *DSP::u::FileInput::GetHeader<DSP::T_FLT_header>(const unsigned int &index = 0);
template DSP::T_TAPE_header *DSP::u::FileInput::GetHeader<DSP::T_TAPE_header>(const unsigned int &index = 0);
template DSP::T_WAVEchunk *DSP::u::FileInput::GetHeader<DSP::T_WAVEchunk>(const unsigned int &index = 0);



unsigned long DSP::u::FileInput::GetSampleSize(DSP::e::SampleType SampleType_in)
{
  unsigned long sample_size;

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
      if ((DSP::File_buffer_size % 8) != 0)
      {
        DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput" << DSP::e::LogMode::second << "Can't read bit stream corectly:"
          " DSP::File_buffer_size is not a multiply of byte size (8 bits)" << endl;
      }
      break;
    case DSP::e::SampleType::ST_short:
      sample_size=8*sizeof(short);
      break;
    case DSP::e::SampleType::ST_none: // return internal sample size used in DSP::u::FileInput
      sample_size = SampleSize/NoOfFileChannels;
      break;
    default:
      sample_size=8*sizeof(short);
      DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput" << DSP::e::LogMode::second << "Unsupported data type" << endl;
      break;
  }
  sample_size*=NoOfFileChannels;

  return sample_size;
}

DSP::u::FileInput::~FileInput(void)
{
//  SetNoOfOutputs(0);
  CloseFile();

  Buffer.clear();
}

// returns number of bytes read during last file access
unsigned long DSP::u::FileInput::GetBytesRead(void)
{
  return BytesRead;
}
// returns sampling rate of audio sample
unsigned long DSP::u::FileInput::GetSamplingRate(void)
{
  return SamplingRate;
}

// Returns raw buffer size in bytes needed for NoOfSamples samples.
/* If NoOfSamples == 0 return allocated internal raw buffer size.
 * \note FlushBuffer requires one additional byte bot bit modes
 */
unsigned long DSP::u::FileInput::GetRawBufferSize(const unsigned long &NoOfSamples)
{
  if (NoOfSamples == 0)
    return (SampleSize * DSP::File_buffer_size + 7)/8;
  else
    return (SampleSize * NoOfSamples + 7)/8;
}

// Returns DSP::Float buffer size needed for SizeInSamples samples.
/* If SizeInSamples == 0 return allocated internal DSP::Float buffer size.
 *
 *  \note Returned value is NoOfSamples * NoOfChannels.
 */
unsigned long DSP::u::FileInput::GetFltBufferSize(const unsigned long &NoOfSamples)
{
  if (NoOfSamples == 0)
    return (NoOfOutputs * DSP::File_buffer_size);
  else
    return (NoOfOutputs * NoOfSamples);
}

//! moves file pointer no_to_skip samples forward
long long DSP::u::FileInput::SkipSamples(const long long &no_to_skip)
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

unsigned long DSP::u::FileInput::ReadSegmentToBuffer(
    DSP::Float_vector    &flt_buffer,
    int pad_size)
{
  DSP::Float *tempBuffer;
  uint8_t *tempUChar;
  //short *tempShort;
  int16_t *tempShort;
  //int *tempInt;
  int32_t *tempInt;
  float *tempFloat;
  unsigned int ind, ind2;
  unsigned char mask;

  //flt_buffer.resize(buffer_size * NoOfOutputs);
  unsigned long no_of_samples = (unsigned long)(flt_buffer.size()/NoOfOutputs);
  RawBuffer.resize(GetRawBufferSize(no_of_samples), 0);

  //! \TODO in input/output operations use int16_t and int32_t instead short and int

  /*! \todo <b>27.07.2008</b> Check first if there is something to read
   *  in DSP::u::FileInput internal buffer. If yes then use it before
   *  reading the file.
   */
  if (FileHandle != NULL)
    BytesRead =
        (unsigned int)fread(RawBuffer.data(), 1, GetRawBufferSize(no_of_samples), FileHandle);
  else
    BytesRead=0;

  if (BytesRead < RawBuffer.size())
  { //clear the end of the buffer if neccessary
    memset(&(RawBuffer[BytesRead]), 0, RawBuffer.size() - BytesRead);
  }

  //Now we need to convert data from RawBuffer
  //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
  tempBuffer = flt_buffer.data();
  switch (SampleType)
  {
    case DSP::e::SampleType::ST_bit:
      tempUChar=RawBuffer.data();
      mask=0x80;
      for (ind=0; ind<no_of_samples; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer = (((*tempUChar) & mask) != 0) ? DSP::Float(+1.0) : DSP::Float(-1.0);
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
      tempUChar=RawBuffer.data();
      mask=0x01;
      for (ind=0; ind<no_of_samples; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer = (((*tempUChar) & mask) != 0) ? DSP::Float(+1.0) : DSP::Float(-1.0);
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
      tempUChar=RawBuffer.data();
      for (ind=0; ind<no_of_samples; ind++)
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
      tempUChar=RawBuffer.data();
      for (ind=0; ind<no_of_samples; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer=DSP::Float(*tempUChar-0x80)/0x80;
          tempBuffer++;
          tempUChar++;
        }
        tempBuffer += pad_size;
      }
      break;
    case DSP::e::SampleType::ST_short:
      tempShort=(short *)(RawBuffer.data());
      for (ind=0; ind<no_of_samples; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer=DSP::Float(*tempShort)/0x8000;
          tempBuffer++;
          tempShort++;
        }
        tempBuffer += pad_size;
      }
      break;
    case DSP::e::SampleType::ST_int:
      tempInt=(int *)(RawBuffer.data());
      for (ind=0; ind<no_of_samples; ind++)
      {
        for (ind2=0; ind2 < NoOfOutputs; ind2++)
        {
          *tempBuffer=DSP::Float(*tempInt);
          tempBuffer++;
          tempInt++;
        }
        tempBuffer += pad_size;
      }
      break;
    case DSP::e::SampleType::ST_float:
      tempFloat=(float *)(RawBuffer.data());
      for (ind=0; ind<no_of_samples; ind++)
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

#define DSP_THIS ((DSP::u::FileInput *)source)
// Input file could not be open so output zeros
bool DSP::u::FileInput::OutputExecute_Dummy(OUTPUT_EXECUTE_ARGS)
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
bool DSP::u::FileInput::OutputExecute(OUTPUT_EXECUTE_ARGS)
{ // we assume only one output
  UNUSED_DEBUG_ARGUMENT(clock);
  unsigned int ind2;

  if (DSP_THIS->BufferIndex == 0)
  { // Data must be read from file to buffer
    DSP_THIS->ReadSegmentToBuffer(DSP_THIS->Buffer);
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
  DSP_THIS->BufferIndex %= (DSP::File_buffer_size * DSP_THIS->NoOfOutputs);

  return true;
}
#undef DSP_THIS

//*****************************************************//
//*****************************************************//
DSP::u::FileOutput::FileOutput(unsigned char NoOfChannels) : DSP::Block()
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

  SetName("FileOutput", false);
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

  TmpBuffer.clear();
  RawBuffer.clear();
  FileHandle = NULL;
  Execute_ptr = &InputExecute_blocked;

  Stored_Execute_ptr = NULL;
  IsBlocked = false;
  BlockFile = false;
  UnblockFile = false;
}

DSP::u::FileOutput::FileOutput(const string &FileName,
                DSP::e::SampleType sample_type,
                unsigned int NoOfChannels,
                DSP::e::FileType file_type, long int sampling_rate)
  : DSP::Block()
{
  string temp;

  SetName("FileOutput", false);
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
unsigned long DSP::u::FileOutput::GetBytesRead(void)
{
  return 0;
}
//! returns sampling rate of audio sample
unsigned long DSP::u::FileOutput::GetSamplingRate(void)
{
  return ReOpen_sampling_rate;
}

bool DSP::u::FileOutput::Open(const string &FileName, DSP::e::SampleType sample_type, unsigned int NoOfChannels,
                           DSP::e::FileType file_type, long int sampling_rate)
{
  FileType = file_type;
  SampleType=sample_type;
  // //Data are stored in buffer in DSP::Float format
  // Buffer = new DSP::Float [DSP::File_buffer_size*NoOfInputs];
  // BufferIndex=0; //this means also that Buffer is empty
  // memset(Buffer, 0, NoOfInputs*sizeof(DSP::Float)*DSP::File_buffer_size);

  if (NoOfChannels > UCHAR_MAX)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput::Open" << DSP::e::LogMode::second << "NoOfChannels to large (> UCHAR_MAX)" << endl;
    #endif
    NoOfChannels = UCHAR_MAX;
  }
  TmpBuffer.clear();
  RawBuffer.clear();
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
      if ((DSP::File_buffer_size % 8) != 0)
      {
        DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput" << DSP::e::LogMode::second << "Can't write bit stream correctly:"
          " DSP::File_buffer_size is not a multiply of byte size (8 bits)" << endl;
      }
      // "0 / 1" byte by byte
      TmpBuffer.resize(SampleSize*DSP::File_buffer_size);

      Execute_ptr = &InputExecute_bit; // FlushBuffer is used
      FlushBuffer_type = E_FB_default;
      break;
    default:
      #ifdef __DEBUG__
        DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput" << DSP::e::LogMode::second << "Unsupported data type changing to DSP::e::SampleType::ST_short" << endl;
      #endif
      SampleType = DSP::e::SampleType::ST_short;
      Execute_ptr = &InputExecute_short;
      FlushBuffer_type = E_FB_raw;
      break;
  }
  RawBuffer.resize((SampleSize*DSP::File_buffer_size+7)/8 + 1);
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
              DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput" << DSP::e::LogMode::second << "*.wav: unsupported sample type" << endl;
            #endif
            break;
        }

        WAV_header.PrepareHeader((uint32_t)sampling_rate, (uint16_t)NoOfInputs, (uint16_t)(SampleSize/NoOfInputs));
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
              DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput", "*.tape: unsupported sample type");
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
          DSP::T_FLT_header header_flt;
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
                DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput" << DSP::e::LogMode::second << "*.flt: unsupported sample type" << endl;
              #endif
              break;
          }

          fwrite(&header_flt, sizeof(DSP::T_FLT_header), 1, FileHandle);
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

bool DSP::File::SetOffset(long long Offset, DSP::e::OffsetMode mode)
{
  bool result;
  result = false;

  Offset *= SampleSize;
  Offset /= 8;

  switch (mode)
  {
    case DSP::e::OffsetMode::skip:
      if (FileHandle != NULL)
      {
//        fseeko64(FileHandle, Offset, SEEK_SET);
        fseek(FileHandle, long(Offset), SEEK_SET);
        result = true;
      }
      break;

    case DSP::e::OffsetMode::standard:
    default:
      #ifdef __DEBUG__
        DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput:: SetOffset" << DSP::e::LogMode::second << "The mode is unsuported" << endl;
      #endif
      break;
  }
  return result;
}

bool DSP::u::FileOutput::SetSkip(long long Offset)
{
  skip_counter = Offset;
  return true;
}

unsigned long DSP::u::FileOutput::GetSampleSize(DSP::e::SampleType SampleType_in)
{
  unsigned long sample_size;

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
      if ((DSP::File_buffer_size % 8) != 0)
      {
        DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput::GetSampleSize" << DSP::e::LogMode::second << "Can't write bit stream corectly:"
          " DSP::File_buffer_size is not a multiply of byte size (8 bits)" << endl;
      }
      break;
    case DSP::e::SampleType::ST_none:
      sample_size = SampleSize;
      break;
    default:
      #ifdef __DEBUG__
        DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput::GetSampleSize" << DSP::e::LogMode::second << "Unsupported data type changing to DSP::e::SampleType::ST_short" << endl;
      #endif
      sample_size=8*sizeof(short);
      break;
  }
  sample_size*=NoOfInputs;
  return sample_size;
}

DSP::u::FileOutput::~FileOutput(void)
{
//  SetNoOfOutputs(0);
  Close();
}

void DSP::u::FileOutput::Flush(void)
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

void DSP::u::FileOutput::Close(void)
{
  Stored_Execute_ptr = NULL;
  IsBlocked = false;

  Flush();

  RawBuffer.clear();
  TmpBuffer.clear();
  if (FileHandle != NULL)
  {
    fclose(FileHandle);
    FileHandle=NULL;
  }
}

void DSP::u::FileOutput::PerformReOpen(void)
{
  Close();
  Open(ReOpen_FileName, ReOpen_SampleType, NoOfInputs,
       ReOpen_FileType, ReOpen_sampling_rate);

  ReOpenFile = false;
}

void DSP::u::FileOutput::ReOpen(const string &FileName, DSP::e::SampleType sample_type,
                             DSP::e::FileType file_type, long int sampling_rate)
{
  ReOpen_FileName = FileName;

  ReOpen_SampleType = sample_type;
  ReOpen_FileType = file_type;
  ReOpen_sampling_rate = sampling_rate;

  ReOpenFile = true;
}

bool DSP::u::FileOutput::BlockOutput(bool block)
{
  if (FileHandle == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << "DSP::u::FileOutput::BlockOutput" << DSP::e::LogMode::second << "Warning: FileHandle == NULL !!!" << endl;
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

void DSP::u::FileOutput::PerformBlock(bool block)
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
unsigned long DSP::u::FileOutput::GetRawBufferSize(const unsigned long &NoOfSamples)
{
  if (NoOfSamples == 0)
    return (SampleSize * DSP::File_buffer_size + 7)/8;
  else
    return (SampleSize * NoOfSamples + 7)/8;
}

unsigned long DSP::u::FileOutput::WriteSegmentFromBuffer(
                                    const DSP::Float_vector &flt_buffer,
                                    int skip)
{
  unsigned int ind, ind_raw;
  unsigned long BytesWritten;
  short temp_short;
  int temp_int;

  // \note size == buffer_size * no_of_channels
  unsigned long no_of_samples = (unsigned long)(flt_buffer.size()/NoOfInputs);

  // Raw buffer which is used internally by the function
  // \note raw_buffer_size == buffer_size * sample_size / 8.
  std::vector<uint8_t> raw_buffer(GetRawBufferSize(no_of_samples), 0);


  // Write to file content of the internal buffer
  if ((FlushBuffer_type & E_FB_raw) != 0)
    raw_FlushBuffer();
  else
    FlushBuffer();

  // convert flt_buffer to raw_buffer
  switch (SampleType)
  {
    case DSP::e::SampleType::ST_float:
      if ((sizeof(float) == sizeof(DSP::Float)) && (skip == 0))
      { // write without conversion
        BytesWritten = (unsigned int)fwrite(flt_buffer.data(), 1, GetRawBufferSize(no_of_samples), FileHandle);
        return BytesWritten;
      }
      ind = 0;
      for (ind_raw=0; ind_raw < no_of_samples*NoOfInputs; ind_raw++)
      {
        ((float *)raw_buffer.data())[ind_raw] = flt_buffer[ind++];
        ind+=skip;
      }
      break;

    case DSP::e::SampleType::ST_scaled_float:
      ind = 0;
      for (ind_raw=0; ind_raw < no_of_samples*NoOfInputs; ind_raw++)
      {
        ((float *)raw_buffer.data())[ind_raw] = 0x8000 * flt_buffer[ind++];
        ind+=skip;
      }
      break;

    case DSP::e::SampleType::ST_uchar:
      if (FileType_no_scaling == true)
      {
        ind = 0;
        for (ind_raw=0; ind_raw < no_of_samples*NoOfInputs; ind_raw++)
        {
          temp_short = (short)(flt_buffer[ind++]);
          if (temp_short < 0)
            temp_short = 0;
          if (temp_short > 0xff)
            temp_short = 0xff;
          ((uint8_t *)raw_buffer.data())[ind_raw] = (uint8_t)temp_short;
          ind+=skip;
        }
      }
      else
      {
        ind = 0;
        for (ind_raw=0; ind_raw < no_of_samples*NoOfInputs; ind_raw++)
        {
          temp_short = (short)((flt_buffer[ind++]*0x80)+0x80);
          if (temp_short < 0)
            temp_short = 0;
          if (temp_short > 0xff)
            temp_short = 0xff;
          ((uint8_t *)raw_buffer.data())[ind_raw] = (uint8_t)temp_short;
          ind+=skip;
        }
      }
      break;


    case DSP::e::SampleType::ST_tchar:
      // assumes FileType_no_scaling == true
      {
        ind = 0;
        for (ind_raw = 0; ind_raw < no_of_samples * NoOfInputs; ind_raw++)
        {
          temp_short = (short)(flt_buffer[ind++]);
          if (temp_short < 0)
            temp_short = 0;
          if (temp_short > 0xff)
            temp_short = 0xff;
          ((uint8_t*)raw_buffer.data())[ind_raw] = (uint8_t)temp_short;
          ind += skip;
        }
      }
      break;

    case DSP::e::SampleType::ST_short:
      if (FileType_no_scaling == true)
      {
        ind = 0;
        for (ind_raw=0; ind_raw < no_of_samples*NoOfInputs; ind_raw++)
        {
          temp_int = (int)(flt_buffer[ind++]);
          if (temp_int < -0x8000)
            temp_int = -0x8000;
          if (temp_int > 0x7fff)
            temp_int = 0x7fff;
          ((short *)raw_buffer.data())[ind_raw] = (short)temp_int;
          ind+=skip;
        }
      }
      else
      {
        ind = 0;
        for (ind_raw=0; ind_raw < no_of_samples*NoOfInputs; ind_raw++)
        {
          temp_int = (int)(flt_buffer[ind++]*0x8000);
          if (temp_int < -0x8000)
            temp_int = -0x8000;
          if (temp_int > 0x7fff)
            temp_int = 0x7fff;
          ((short *)raw_buffer.data())[ind_raw] = (short)temp_int;
          ind+=skip;
        }
      }
      break;

    default:
      DSP::log << DSP::e::LogMode::Error << "DSP::u::FileOutput::WriteSegmentToBuffer" << DSP::e::LogMode::second << "Unsupported file sample type - write aborted" << endl;
      return 0;
  }

  // write rawbuffer
  BytesWritten = (unsigned int)fwrite(raw_buffer.data(), 1, GetRawBufferSize(no_of_samples), FileHandle);
  return BytesWritten;
}

#define  DSP_THIS  ((DSP::u::FileOutput *)block)
// Just ignore inputs and process block and reopen signals
void DSP::u::FileOutput::InputExecute_Dummy(INPUT_EXECUTE_ARGS)
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
void DSP::u::FileOutput::InputExecute_float(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((float *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]=value;
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be written to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((float *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo] =
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

void DSP::u::FileOutput::InputExecute_scaled_float(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((float *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((float *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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

void DSP::u::FileOutput::InputExecute_uchar(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  short temp;

  temp = (short)((value*0x80)+0x80);
  if (temp < 0)
    temp = 0;
  if (temp > 0xff)
    temp = 0xff;

  ((uint8_t *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((uint8_t *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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

void DSP::u::FileOutput::InputExecute_uchar_no_scaling(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (value < 0)
    value = 0;
  if (value > 0xff)
    value = 0xff;

  ((uint8_t *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be written to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((uint8_t *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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

void DSP::u::FileOutput::InputExecute_short(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);
  int temp;

  temp = (int)(value*0x8000);
  if (temp < -0x8000)
    temp = -0x8000;
  if (temp > 0x7fff)
    temp = 0x7fff;

  ((short *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }


    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((short *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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

void DSP::u::FileOutput::InputExecute_short_no_scaling(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (value < -0x8000)
    value = -0x8000;
  if (value > 0x7fff)
    value = 0x7fff;

  ((short *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((short *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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

void DSP::u::FileOutput::InputExecute_int(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);
  long long temp;

  temp = (long long)(value*0x80000000L);
  if (temp < INT_MIN)
    temp = INT_MIN;
  if (temp > INT_MAX)
    temp = INT_MAX;

  ((int *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((int *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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

void DSP::u::FileOutput::InputExecute_int_no_scaling(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (value < (DSP::Float)INT_MIN)
    value = (DSP::Float)INT_MIN;
  if (value > (DSP::Float)INT_MAX)
    value = (DSP::Float)INT_MAX;

  ((int *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((int *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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

void DSP::u::FileOutput::InputExecute_bit_text(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((uint8_t *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->raw_FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((uint8_t *)DSP_THIS->RawBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
void DSP::u::FileOutput::InputExecute_bit(INPUT_EXECUTE_ARGS)
{ // we assume only one input
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((uint8_t *)DSP_THIS->TmpBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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
      if (DSP_THIS->BufferIndex == DSP::File_buffer_size)
      { // Data must be writen to file from buffer
        //First we need to convert data from RawBuffer
        //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
        DSP_THIS->FlushBuffer();
      }
      //DSP_THIS->BufferIndex %= DSP::File_buffer_size;
    }

    //NoOfInputsProcessed=0;
    if (DSP_THIS->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < DSP_THIS->NoOfInputs; ind++)
        if (DSP_THIS->IsConstantInput[ind])
        {
          ((uint8_t *)DSP_THIS->TmpBuffer.data())[DSP_THIS->BufferIndex * DSP_THIS->NoOfInputs + InputNo]
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

void DSP::u::FileOutput::InputExecute_blocked(INPUT_EXECUTE_ARGS)
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
void DSP::u::FileOutput::InputExecute(DSP::Block *block, int InputNo, DSP::Float value, DSP::Component_ptr Caller)
{ // we assume only one input
  ((DSP::u::FileOutput *)block)->Buffer[((DSP::u::FileOutput *)block)->BufferIndex * ((DSP::u::FileOutput *)block)->NoOfInputs + InputNo]=value;
  ((DSP::u::FileOutput *)block)->NoOfInputsProcessed++;

  if (((DSP::u::FileOutput *)block)->NoOfInputsProcessed == ((DSP::u::FileOutput *)block)->NoOfInputs)
  {
    if (((DSP::u::FileOutput *)block)->SamplesToSkipCounter > 0)
    {
      ((DSP::u::FileOutput *)block)->SamplesToSkipCounter--;
      ((DSP::u::FileOutput *)block)->NoOfInputsProcessed = ((DSP::u::FileOutput *)block)->InitialNoOfInputsProcessed;
      return;
    }

    ((DSP::u::FileOutput *)block)->BufferIndex++;
    ((DSP::u::FileOutput *)block)->BufferIndex %= DSP::File_buffer_size;

    if (((DSP::u::FileOutput *)block)->BufferIndex == 0)
    { // Data must be written to file from buffer
      //First we need to convert data from RawBuffer
      //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
      ((DSP::u::FileOutput *)block)->FlushBuffer();
    }

    //NoOfInputsProcessed=0;
    if (((DSP::u::FileOutput *)block)->IsUsingConstants)
    {
      for (int ind=0; ind < ((DSP::u::FileOutput *)block)->NoOfInputs; ind++)
        if (((DSP::u::FileOutput *)block)->IsConstantInput[ind])
        {
          ((DSP::u::FileOutput *)block)->Buffer[((DSP::u::FileOutput *)block)->BufferIndex * ((DSP::u::FileOutput *)block)->NoOfInputs + InputNo] =
             ((DSP::u::FileOutput *)block)->ConstantInputValues[ind];
          ((DSP::u::FileOutput *)block)->NoOfInputsProcessed++;
        }
    }
    ((DSP::u::FileOutput *)block)->NoOfInputsProcessed = ((DSP::u::FileOutput *)block)->InitialNoOfInputsProcessed;
  }

  #ifdef VerboseCompilation
    printf("%i: %5.3f\r\n", InputNo, value);
  #endif
};
*/

void DSP::u::FileOutput::FlushBuffer(void)
{
  uint8_t *tempUChar, mask;
  uint8_t *tempBuffer;
//#ifdef __DEBUG__
//  DSP::Float *tempBuffer_float;
//  short *tempShort;
//  float *tempFloat;
//#endif
  unsigned int ind;

  if (FileHandle != NULL)
  {

    if (BufferIndex == 0)
      return; //BufferIndex=DSP::File_buffer_size;

    switch (SampleType)
    {
    #ifdef __DEBUG__
      case DSP::e::SampleType::ST_short:
        DSP::log << "DSP::u::FileOutput::FlushBuffer" << DSP::e::LogMode::second << "DSP::e::SampleType::ST_short no longer supported" << endl;
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
        DSP::log << "DSP::u::FileOutput::FlushBuffer" << DSP::e::LogMode::second << "DSP::e::SampleType::ST_float no longer supported" << endl;
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
        DSP::log << "DSP::u::FileOutput::FlushBuffer" << DSP::e::LogMode::second << "DSP::e::SampleType::ST_tchar no longer supported" << endl;
        break;
      case DSP::e::SampleType::ST_uchar:
        DSP::log << "DSP::u::FileOutput::FlushBuffer" << DSP::e::LogMode::second << "DSP::e::SampleType::ST_uchar no longer supported" << endl;
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
        DSP::log << "DSP::u::FileOutput::FlushBuffer" << DSP::e::LogMode::second << "DSP::e::SampleType::ST_bit_text no longer supported" << endl;
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
        tempBuffer = TmpBuffer.data();
        tempUChar=(uint8_t *)RawBuffer.data();
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
        tempBuffer = TmpBuffer.data();
        tempUChar=(uint8_t *)RawBuffer.data();
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
          DSP::log << "DSP::u::FileOutput::FlushBuffer" << DSP::e::LogMode::second << "Unsupported format detected" << endl;
        #endif
        break;
    }

    fwrite(RawBuffer.data(), 1, (SampleSize*BufferIndex+7)/8, FileHandle);
    fflush(FileHandle);
    BufferIndex=0;
  }
}


void DSP::u::FileOutput::raw_FlushBuffer(void)
{
  if (FileHandle != NULL)
  {
    if (BufferIndex == 0)
      return; // BufferIndex=DSP::File_buffer_size;

    fwrite(RawBuffer.data(), 1, SampleSize/8*BufferIndex, FileHandle);
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
      DSP::log << DSP::e::LogMode::Error << "DSP::f::GetWAVEfileParams" << DSP::e::LogMode::second
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
DSP::u::WaveInput::WaveInput(DSP::Clock_ptr ParentClock,
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

  SegmentSize=DSP::File_buffer_size;
  SamplingRate=DSP::DefaultSamplingFrequency;

  Init();

  OutputExecute_ptr = &OutputExecute;
}


DSP::u::WaveInput::~WaveInput(void)
{
  CloseFile();

  ReadBuffer.clear();
  AudioBuffer.clear();
}

bool DSP::u::WaveInput::SetSkip(long long Offset)
{
  UNUSED_ARGUMENT(Offset);

  #ifdef __DEBUG__
    DSP::log << DSP::e::LogMode::Error << "DSP::u::FileInput::SetSkip" << DSP::e::LogMode::second << "not implemented yet" << endl;
  #endif
  return false;
}


//bool CWaveInput::StartCaptureAudio(void)
//To be used in constructor
bool DSP::u::WaveInput::Init(void)
{
  string tekst;
  int len;

  BufferIndex=0; //this means also that Buffer is empty

  ConvertionNeeded=true; //inner buffer in DSP::Float format
  AudioBufferLen = (uint32_t)(NoOfOutputs*SegmentSize*sizeof(DSP::Float));
  AudioBuffer.clear();
  AudioBuffer.resize(AudioBufferLen/sizeof(DSP::Float), 0.0);

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
      DSP::log << "DSP::u::WaveInput::Init" << DSP::e::LogMode::second
        << "(Input file \"" << tekst
        << "\" in block <" << GetName() << "> could not be opened" << endl;
    #endif

    FileEnd=true;
		WAVEchunk.nChannels=0;
    ReadBufferLen=0; ReadBuffer.clear();

    return false; //Error opening file
  }


  //Read WAVE file header
  if (WAVEchunk.WAVEinfo(FileHandle)==false)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "DSP::u::WaveInput::Init" << DSP::e::LogMode::second
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
    DSP::log << "DSP::u::WaveInput::Init" << DSP::e::LogMode::second << temp.str() << endl;
  }
  #endif
  //SampleSize = WAVEchunk.wBitsPerSample * WAVEchunk.nChannels;
  SampleSize = WAVEchunk.nBlockAlign * 8;

  if (WAVEchunk.FindDATA(FileHandle)==false)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "DSP::u::WaveInput::Init" << DSP::e::LogMode::second
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
  ReadBuffer.resize(ReadBufferLen);
  BytesRead=0;

  SamplingRate=WAVEchunk.nSamplesPerSec;
//  SegmentSize;

  return (FileHandle!=NULL);
}

bool DSP::u::WaveInput::CloseFile(void)
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
uint32_t DSP::u::WaveInput::ReadAudioSegment(void)
{
  if (FileEnd == true)
  { // AudioBuffer must be cleaned (reset)
    memset(AudioBuffer.data(), 0, AudioBufferLen);
    BytesRead=0;
    return 0x00000000;
  }
  else
  {
    //ReadFile(hIn, ReadBuffer, ReadBufferLen, &BytesRead, NULL);
    if (BytesRemainingInFile < ReadBufferLen)
      BytesRead=(uint32_t)fread(ReadBuffer.data(), 1, BytesRemainingInFile, FileHandle);
    else
      BytesRead=(uint32_t)fread(ReadBuffer.data(), 1, ReadBufferLen, FileHandle);
    BytesRemainingInFile-=BytesRead;
    if (BytesRead!=ReadBufferLen)
    {
      if (WAVEchunk.wBitsPerSample==8)
        memset(((uint8_t *)ReadBuffer.data())+BytesRead,0x80, ReadBufferLen-BytesRead);
      else
        memset(((uint8_t *)ReadBuffer.data())+BytesRead,0, ReadBufferLen-BytesRead);

//      if (BytesRead==0)
      FileEnd=true;
    }

    //Convert ReadBuffer to AudioBuffe;
    // with multichannel support
    if (ConvertionNeeded)
    {
      unsigned int ind, ind2;
      DSP::Float *temp_Audio;

      temp_Audio=AudioBuffer.data();

      if (WAVEchunk.wBitsPerSample==8)
        for (ind=0; ind<SegmentSize; ind++)
          for (ind2=0; ind2<NoOfOutputs; ind2++)
          {
            if (ind2<WAVEchunk.nChannels)
              *temp_Audio=(DSP::Float)(
                ((uint8_t *)ReadBuffer.data())[ind2+ind*WAVEchunk.nChannels]-0x80)
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
                *temp_Audio=DSP::Float(
                    ((short *)ReadBuffer.data())[ind2+ind*WAVEchunk.nChannels])
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
                  *temp_Audio=DSP::Float(
                      ((float *)ReadBuffer.data())[ind2+ind*WAVEchunk.nChannels])
                      / 0x8000;
                else
                  *temp_Audio=0.0;
                temp_Audio++;
              }
        #ifdef __DEBUG__
          else
          {
            DSP::log << DSP::e::LogMode::Error << "DSP::u::WaveInput::ReadAudioSegment" << DSP::e::LogMode::second << "unsupported PCM sample size" << endl;
          }
        #endif

    }
    return (uint32_t)(BytesRead/sizeof(DSP::Float)/WAVEchunk.nChannels);
  }
}

// returns number of bytes read during last file access
unsigned long DSP::u::WaveInput::GetBytesRead(void)
{
  return BytesRead;
}

// returns sampling rate of audio sample
unsigned long DSP::u::WaveInput::GetSamplingRate(void)
{
  return SamplingRate;
}

#define DSP_THIS ((DSP::u::WaveInput *)source)
bool DSP::u::WaveInput::OutputExecute(OUTPUT_EXECUTE_ARGS)
{ // we assume only one output
  UNUSED_DEBUG_ARGUMENT(clock);
  unsigned int ind;
  DSP::Float *temp;

  if (DSP_THIS->BufferIndex == 0)
  { // Data must be read from file to buffer and convert to DSP::Float
    DSP_THIS->ReadAudioSegment();
  }

  /* \todo_later The below calculation is done on the pointer stored in the object 
   * so it wouldn't need continous recalculation and just after the ReadAudioSegment above
   * sets the pointer to AudioBuffer and after each read increases just like temp.
   *
   * Check the above also in case of DSP::u::FileInput
   */
  temp = DSP_THIS->AudioBuffer.data() +
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
DSP::T_WAVEchunk::T_WAVEchunk(void)
{
  clear();
}
void DSP::T_WAVEchunk::clear() {
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

void DSP::T_WAVEchunk::PrepareHeader(
    uint32_t nSamplesPerSec_in,
    uint16_t  nChannels_in,
    uint16_t  wBitsPerSample_in)
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
      DSP::log << DSP::e::LogMode::Error << "T_WAVEchunk::PrepareHeader" << DSP::e::LogMode::second << "wBitsPerSample_in in not a multiple of 8" << endl;
  #endif
  nBlockAlign = (uint16_t)(wBitsPerSample / 8 * nChannels);
  nAvgBytesPerSec = nSamplesPerSec * nBlockAlign;

  memcpy(DataType, "data ", 4);
  DataSize = 0;

  HeaderSize = 44; // without any extra bytes
}

bool DSP::T_WAVEchunk::WriteHeader(FILE *hOut)
{
  if ((HeaderSize > 0) && (hOut != NULL))
  {
    BytesRead=(uint32_t)fwrite(this,1,HeaderSize,hOut);

    if ((int)BytesRead == HeaderSize)
      return true;
  }

  return false;
}

bool DSP::T_WAVEchunk::UpdateHeader(FILE *hOut)
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
    size = (uint32_t)(len - 8);
    DataSize = (uint32_t)(len - HeaderSize);
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
    BytesRead=(uint32_t)fwrite(this,1,HeaderSize,hOut);

    // 6. Restore position in the file
    fsetpos(hOut, &pos);

    if ((int)BytesRead == HeaderSize)
      return true;
  }

  return false;
}

bool DSP::T_WAVEchunk::WAVEinfo(FILE *hIn)
{
//4  read(hIn, Type, 4); "RIFF"
//8  read(hIn, size, 4);
//12  read(hIn, SubType, 4); "WAVE"
//16  read(hIn, FmtType, 4); "fmt "
//20  read(hIn, Fmt_size, 4);
//22(2)  uint16_t wFormatTag; // Data encoding format
//24(4)  uint16_t nChannels;  // Number of channels
//28(8)  uint32_t nSamplesPerSec;   // Samples per second
//32(12)  uint32_t nAvgBytesPerSec;  // Avg transfer rate
//34(14)  uint16_t  nBlockAlign;      // Block alignment
//36(16)  uint16_t  nBitsPerSample;   // Bits per sample

//  ReadFile(hIn, this, 36, &BytesRead, NULL);
  BytesRead=(uint32_t)fread(this,1,36,hIn);
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


int DSP::T_WAVEchunk::strncmpi(const char* str1, const char* str2, int N)
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

bool DSP::T_WAVEchunk::FindDATA(FILE *hIn)
{
  uint32_t BytesRead_temp;

//  ReadFile(hIn, DataType, 4, &BytesRead_temp, NULL);
  BytesRead_temp=(uint32_t)fread(DataType,1,4,hIn);
  BytesRead+=BytesRead_temp;
//  ReadFile(hIn, &DataSize, 4, &BytesRead_temp, NULL);
  BytesRead_temp=(uint32_t)fread(&DataSize,1,4,hIn);
  BytesRead+=BytesRead_temp;

  //  while (strncasecmp(DataType, "DATA", 4)!=0)
  while (strncmpi(DataType, "DATA", 4)!=0)
  {
//    SetFilePointer(hIn, DataSize+(DataSize%2), NULL, FILE_CURRENT);
//    fseeko64(hIn, DataSize+(DataSize%2), SEEK_CUR);
    fseek(hIn, DataSize+(DataSize%2), SEEK_CUR);
//    ReadFile(hIn, DataType, 4, &BytesRead_temp, NULL);
    BytesRead_temp=(uint32_t)fread(DataType,1,4,hIn);
    BytesRead+=BytesRead_temp;
//    ReadFile(hIn, &DataSize, 4, &BytesRead_temp, NULL);
    BytesRead_temp=(uint32_t)fread(&DataSize,1,4,hIn);
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
  uint32_t Wait_result, Ktory;
  LPVOID CaptureBuforLock;
  uint32_t CaptureBuforLockLen;

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
  WaveFormat.nBlockAlign=(uint16_t)(WaveFormat.nChannels*
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
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "Specified resource is already allocated." << endl;
          break;
        case MMSYSERR_BADDEVICEID:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "Specified device identifier is out of range." << endl;
          break;
        case MMSYSERR_NODRIVER:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "No device driver is present." << endl;
          break;
        case MMSYSERR_NOMEM:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "Unable to allocate or lock memory." << endl;
          break;
        case WAVERR_BADFORMAT:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "Attempted to open with an unsupported waveform-audio format." << endl;
          break;
        case WAVERR_SYNC:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "The device is synchronous but waveOutOpen was called without using the WAVE_ALLOWSYNC flag." << endl;
          break;
        case MMSYSERR_INVALFLAG:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "Invalid flag" << endl;
          break;
        case MMSYSERR_NOERROR:
          return false;
          //printf("DSP::f::AudioCheckError: ");
          //printf("No error.");
          //printf("\n"); getchar();
          break;
        case MMSYSERR_INVALHANDLE:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "Specified device handle is invalid." << endl;
          break;
        case WAVERR_UNPREPARED:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "The data block pointed to by the pwh parameter hasn't been prepared." << endl;
          break;
        case MMSYSERR_HANDLEBUSY:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "Handle busy." << endl;
          break;
        case WAVERR_STILLPLAYING:
          DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second <<
            "There are still buffers in the queue." << endl;
          break;
        default:
          {
            DSP::log << DSP::e::LogMode::Error << "DSP::f::AudioCheckError" << DSP::e::LogMode::second
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

uint32_t DSP::f::GetAudioBufferSize(const unsigned long &SamplingFreq, const DSP::e::AudioBufferType &type)
{
  uint32_t size;

  switch (type)
  {
    case DSP::e::AudioBufferType::input:
      size = DSP::Reference_audio_inbuffer_size;
      break;
    case DSP::e::AudioBufferType::output:
      size = DSP::Reference_audio_outbuffer_size;
      break;
    case DSP::e::AudioBufferType::none:
    default:
      size = 0;
      break;
  }

//  size / DSP::ReferenceFs == new_size / SamplingFreq
#ifdef __DEBUG__
  if ((SamplingFreq * size) / DSP::ReferenceFs > UINT32_MAX){
    DSP::log << DSP::e::LogMode::Error << "DSP::f::GetAudioBufferSize" << DSP::e::LogMode::second
       << "Buffer size (" << (SamplingFreq * size) / DSP::ReferenceFs << ") > UINT32_MAX" << endl;
  }
#endif
  size = (uint32_t)((SamplingFreq * size) / DSP::ReferenceFs);
  return size;
}

//#ifdef WINMMAPI

  // //! \bug allow user to select number of internal buffers
  // void CALLBACK DSP::u::AudioInput::waveInProc_short(HWAVEIN hwi, UINT uMsg,
  //   uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2)
  // {
  //   UNUSED_ARGUMENT(hwi);
  //   UNUSED_ARGUMENT(dwParam1);
  //   UNUSED_ARGUMENT(dwParam2);

  //   MMRESULT result;
  //   DSP::u::AudioInput *Current;
  //   short *temp16;
  //   DSP::Float_ptr Sample;
  //   unsigned int ind;
  // #ifdef __DEBUG__
  //   #ifdef AUDIO_DEBUG_MESSAGES_ON
  //     stringstream tekst;
  //   #endif
  // #endif

  //   Current = AudioObjects[dwInstance];

  //   switch (uMsg)
  //   {
  // #ifdef __DEBUG__
  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //     case WIM_OPEN:
  //       DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second
  //         << "WIM_OPEN(" << (int)dwInstance << ")" << endl;
  //       break;
  //     case WIM_CLOSE:
  //       DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second
  //         << "WIM_CLOSE(" << (int)dwInstance << ")" << endl;
  //       break;
  // #endif
  // #endif

  //     case WIM_DATA:
  // #ifdef __DEBUG__
  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //       DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second
  //         << "WIM_DATA(" << (int)dwInstance << ")" << endl;
  // #endif
  // #endif

  //       if (Current->StopRecording)
  //         return;
  //       else
  //       {
  //         if (Current->EmptyBufferIndex == Current->CurrentBufferIndex)
  //         {
  // #ifdef __DEBUG__
  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //           DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second << "All buffers had been used - skipping input audio frame" << endl;
  // #endif
  // #endif
  //           result=waveInUnprepareHeader(Current->hWaveIn,
  //             &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //           DSP::f::AudioCheckError(result);
  //           // ignore data

  //           //add put back into recording queue
  //           result=waveInPrepareHeader(Current->hWaveIn,
  //             &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //           DSP::f::AudioCheckError(result);
  //           result=waveInAddBuffer(Current->hWaveIn,
  //             &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //           DSP::f::AudioCheckError(result);

  //           Current->NextBufferInd++; Current->NextBufferInd %= 2; //just two buffers
  //         }
  //         else
  //         { //copy audio frame to buffer form audio frame NextBufferInd
  //           if (Current->waveHeaderIn[Current->NextBufferInd].dwFlags & WHDR_DONE)
  //           {
  //             //copy data
  //             result=waveInUnprepareHeader(Current->hWaveIn,
  //               &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //             DSP::f::AudioCheckError(result);

  //             Sample=Current->InBuffers[Current->EmptyBufferIndex].data();
  //             // ************************************************** //
  //             // Converts samples format to the one suitable for the audio device
  //             #ifdef __DEBUG__
  //               if (Current->InSampleType != DSP::e::SampleType::ST_short)
  //               {
  //                 DSP::log << "DSP::u::AudioInput::waveInProc_short" << DSP::e::LogMode::second << "Current->InSampleType != DSP::e::SampleType::ST_short" << endl;
  //               }
  //             #endif

  //             temp16=(short *)(Current->WaveInBuffers[Current->NextBufferInd].data());
  //             for (ind=0; ind<Current->InBufferLen; ind++)
  //             {
  //               *Sample = (DSP::Float)(*temp16) / SHRT_MAX;
  //               Sample++;
  //               temp16++;
  //             }
  //             Current->EmptyBufferIndex++; Current->EmptyBufferIndex %= DSP::NoOfAudioInputBuffers;

  //             //add put back into recording queue
  //             result=waveInPrepareHeader(Current->hWaveIn,
  //               &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //             DSP::f::AudioCheckError(result);
  //             result=waveInAddBuffer(Current->hWaveIn,
  //               &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //             DSP::f::AudioCheckError(result);

  //             Current->NextBufferInd++; Current->NextBufferInd %= 2; //just two buffers
  //           }
  //           else
  //           {
  // #ifdef __DEBUG__
  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //             DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second << "Wrong audio frame ready or other unexpected error" << endl;
  // #endif
  // #endif
  //           }
  //         }
  //       }
  //       break; // WIM_DATA
  //   }
  // }

  // //! \bug allow user to select number of internal buffers
  // void CALLBACK DSP::u::AudioInput::waveInProc_uchar(HWAVEIN hwi, UINT uMsg,
  //   uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2)
  // {
  //   UNUSED_ARGUMENT(hwi);
  //   UNUSED_ARGUMENT(dwParam1);
  //   UNUSED_ARGUMENT(dwParam2);

  //   MMRESULT result;
  //   DSP::u::AudioInput *Current;
  //   uint8_t *temp8;
  //   DSP::Float_ptr Sample;
  //   unsigned int ind;
  // #ifdef __DEBUG__
  //   #ifdef AUDIO_DEBUG_MESSAGES_ON
  //     stringstream tekst;
  //   #else
  //     UNUSED_DEBUG_ARGUMENT(uMsg);
  //   #endif
  // #endif

  //   Current = AudioObjects[dwInstance];

  //   switch (uMsg)
  //   {
  // #ifdef __DEBUG__
  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //     case WIM_OPEN:
  //       DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second
  //         << "WIM_OPEN(" << (int)dwInstance << ")" << end;
  //       break;
  //     case WIM_CLOSE:
  //       DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second
  //         << "WIM_CLOSE(" << (int)dwInstance << ")" << endl;
  //       break;
  // #endif
  // #endif

  //     case WIM_DATA:
  // #ifdef __DEBUG__
  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //       DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second
  //         << "WIM_DATA(" << (int)dwInstance << ")" << endl;
  // #endif
  // #endif

  //       if (Current->StopRecording)
  //         return;
  //       else
  //       {
  //         if (Current->EmptyBufferIndex == Current->CurrentBufferIndex)
  //         {
  // #ifdef __DEBUG__
  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //           DSP::log << "DSP::u::AudioInput::waveInProc"  << DSP::e::LogMode::second << "All buffers had been used - skipping input audio frame" << endl;
  // #endif
  // #endif
  //           result=waveInUnprepareHeader(Current->hWaveIn,
  //             &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //           DSP::f::AudioCheckError(result);
  //           // ignore data

  //           //add put back into recording queue
  //           result=waveInPrepareHeader(Current->hWaveIn,
  //             &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //           DSP::f::AudioCheckError(result);
  //           result=waveInAddBuffer(Current->hWaveIn,
  //             &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //           DSP::f::AudioCheckError(result);

  //           Current->NextBufferInd++; Current->NextBufferInd %= 2; //just two buffers
  //         }
  //         else
  //         { //copy audio frame to buffer form audio frame NextBufferInd
  //           if (Current->waveHeaderIn[Current->NextBufferInd].dwFlags & WHDR_DONE)
  //           {
  //             //copy data
  //             result=waveInUnprepareHeader(Current->hWaveIn,
  //               &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //             DSP::f::AudioCheckError(result);

  //             Sample=Current->InBuffers[Current->EmptyBufferIndex].data();
  //             // ************************************************** //
  //             // Converts samples format to the one suitable for the audio device
  //             #ifdef __DEBUG__
  //               if (Current->InSampleType != DSP::e::SampleType::ST_uchar)
  //               {
  //                 DSP::log << "DSP::u::AudioInput::waveInProc_uchar" << DSP::e::LogMode::second << "Current->InSampleType != DSP::e::SampleType::ST_uchar" << endl;
  //               }
  //             #endif

  //             temp8=(uint8_t *)(Current->WaveInBuffers[Current->NextBufferInd].data());
  //             for (ind=0; ind<Current->InBufferLen; ind++)
  //             {
  //               *Sample = (DSP::Float)(*temp8 - 128) / 128;
  //               Sample++;
  //               temp8++;
  //             }
  //             Current->EmptyBufferIndex++; Current->EmptyBufferIndex %= DSP::NoOfAudioInputBuffers;

  //             //add put back into recording queue
  //             result=waveInPrepareHeader(Current->hWaveIn,
  //               &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //             DSP::f::AudioCheckError(result);
  //             result=waveInAddBuffer(Current->hWaveIn,
  //               &(Current->waveHeaderIn[Current->NextBufferInd]), sizeof(WAVEHDR));
  //             DSP::f::AudioCheckError(result);

  //             Current->NextBufferInd++; Current->NextBufferInd %= 2; //just two buffers
  //           }
  //           else
  //           {
  // #ifdef __DEBUG__
  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //             DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second << "Wrong audio frame ready or other unexpected error" << endl;
  // #endif
  // #endif
  //           }
  //         }
  //         break;
  //       }

  //   }
  // }
//#endif // WINMMAPI

// unsigned long DSP::u::AudioOutput::Next_CallbackInstance=0;
// unsigned long DSP::u::AudioInput::Next_CallbackInstance =0;
// std::vector<DSP::u::AudioOutput *> DSP::u::AudioOutput::AudioObjects;
// std::vector<DSP::u::AudioInput  *> DSP::u::AudioInput::AudioObjects;

DSP::u::AudioOutput::AudioOutput(void)
  : DSP::Block()
{
  Init(8000);
}

/*
DSP::u::AudioInput::AudioInput(DSP::Clock_ptr ParentClock)
  : DSP::Source(ParentClock)
{
  Init(ParentClock, 8000);
}
*/

DSP::u::AudioOutput::AudioOutput(
                 unsigned long SamplingFreq,
                 unsigned int InputsNo, //just one channel
                 unsigned char BitPrec,
                 unsigned int WaveOutDevNo)
  : DSP::Block()
{
  Init(SamplingFreq, InputsNo, BitPrec, WaveOutDevNo);
}

/* Inputs and Outputs names:
 *   - Output: none
 *   - Input:
 *    -# "in" - real or complex
 *    -# "in.re" - first channel (real component)\n
 *       "in.im" - second channel (imag component if exists)
 *    -# "in1", "in2" - i-th channel input
 */
void DSP::u::AudioOutput::Init(unsigned long SamplingFreq,
                           unsigned int InputsNo, //just one channel
                           unsigned char BitPrec,
                           unsigned int WaveOutDevNo)
{
  #ifdef WINMMAPI
    // MMRESULT result;
    // DWORD_PTR Callback;
    //
    // //Rezerwacja pamięci dla formatu WAVE
    // //  WAVEFORMATEX wfx; //to wymaga korekty
    // PCMWAVEFORMAT wfx;
    //
    // unsigned long ind;
  #else
    UNUSED_ARGUMENT(WaveOutDevNo);
  #endif

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

//  #ifdef WINMMAPI
//    //! \bug in Debug mode this callback does nothing so it would be better just not use it
//    Callback = (DWORD_PTR)(&DSP::u::AudioOutput::waveOutProc);
//  #endif
  // Current_CallbackInstance=Next_CallbackInstance;
  // Next_CallbackInstance++;
  // AudioObjects.resize(Current_CallbackInstance+1);
  // AudioObjects[Current_CallbackInstance]=this;

  audio_outbuffer_size = DSP::f::GetAudioBufferSize(SamplingFreq, DSP::e::AudioBufferType::output);

  snd_object.select_output_device_by_number(WaveOutDevNo); // use default device
  snd_object.open_PCM_device_4_output(NoOfInputs, BitPrec, SamplingFreq, audio_outbuffer_size);

  OutBufferLen=NoOfInputs*audio_outbuffer_size;
  OutBuffer.clear(); OutBuffer.resize(OutBufferLen, 0.0);
  BufferIndex=0;

  // #ifdef WINMMAPI

  // #else

  //   WaveOutBufferLen = 0;
  //   WaveOutBuffers.clear();
  //   OutBuffer.clear(); OutBufferLen = 0;
  //   BufferIndex=0;

  // #endif  // WINMMAPI

  // IsPlayingNow = false;
  //NextBufferInd=0;

  Execute_ptr = &InputExecute;
}

/* ***************************************** */
/* ***************************************** */
DSP::u::AudioInput::AudioInput(
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

bool DSP::u::AudioInput::SOUND_object_callback(const DSP::e::SampleType &InSampleType, const std::vector<char> &wave_in_raw_buffer) {
  if (EmptyBufferIndex != CurrentBufferIndex) {
    DSP::Float_ptr Sample;
    unsigned int ind;

#ifdef __DEBUG__
    switch (InSampleType) {
      case DSP::e::SampleType::ST_short: {
          if (InBufferLen != wave_in_raw_buffer.size() / sizeof(short)) {
            DSP::log << "DSP::u::AudioInput::SOUND_object_callback" << DSP::e::LogMode::second << "incorrect wave_in_raw_buffer size: " <<
              wave_in_raw_buffer.size()  << " instead of " << InBufferLen * sizeof(short) << endl;
          }
        }
        break;
      case DSP::e::SampleType::ST_uchar: {
          if (InBufferLen != wave_in_raw_buffer.size() / sizeof(uint8_t)) {
            DSP::log << "DSP::u::AudioInput::SOUND_object_callback" << DSP::e::LogMode::second << "incorrect wave_in_raw_buffer size: " <<
              wave_in_raw_buffer.size()  << " instead of " << InBufferLen * sizeof(uint8_t) << endl;
          }
        }
        break;
      default:
        DSP::log << "DSP::u::AudioInput::SOUND_object_callback" << DSP::e::LogMode::second << "Unsupported Current->InSampleType" << endl;
        break;
    }
#endif // __DEBUG__

    //  reading sound card input buffer
    Sample=InBuffers[EmptyBufferIndex].data();
    // ************************************************** //
    // Converts samples format from the one used in the audio device
    switch (InSampleType) {
      case DSP::e::SampleType::ST_short: {
          short *temp16;

          temp16=(short *)(wave_in_raw_buffer.data());
          for (ind=0; ind<InBufferLen; ind++)
          {
            *Sample = (DSP::Float)(*temp16) / SHRT_MAX;
            Sample++;
            temp16++;
          }
        }
        break;

      case DSP::e::SampleType::ST_uchar: {
          uint8_t *temp8;
          
          temp8=(uint8_t *)(wave_in_raw_buffer.data());
          for (ind=0; ind<InBufferLen; ind++)
          {
            *Sample = (DSP::Float)(*temp8 - 128) / 128;
            Sample++;
            temp8++;
          }
        }
        break;

      default:
    #ifdef __DEBUG__
        DSP::log << "DSP::u::AudioInput::SOUND_object_callback" << DSP::e::LogMode::second << "Unsupported Current->InSampleType" << endl;
    #endif
        break;
    }
    EmptyBufferIndex++; EmptyBufferIndex %= DSP::NoOfAudioInputBuffers;

    return true;
  }
  else {
    return false;
  }
  return false;
}

/* Inputs and Outputs names:
 *   - Output:
 *    -# "out" - real or complex
 *    -# "out.re" - first channel (real component)\n
 *       "out.im" - second channel (imag component if exists)
 *    -# "out1", "out2" - i-th channel output
 *   - Input: none
 */
void DSP::u::AudioInput::Init(DSP::Clock_ptr ParentClock,
                           long int SamplingFreq,
                           unsigned int OutputsNo, //just one channel
                           char BitPrec,
                           unsigned int WaveInDevNo)
{
  unsigned long ind;
  string temp;

  // #ifdef WINMMAPI
  //   MMRESULT result;
  //   DWORD_PTR Callback;
  // //Rezerwacja pamici dla formatu WAVE
  // //  WAVEFORMATEX wfx; //to wymaga korekty
  //   PCMWAVEFORMAT wfx;
  // #elif defined(ALSA_support_H)
  //   UNUSED_ARGUMENT(WaveInDevNo);
  //   ALSA_object_t ALSA_object;

  // #else
  //   UNUSED_ARGUMENT(WaveInDevNo);
  // #endif

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

  // Current_CallbackInstance=Next_CallbackInstance;
  // Next_CallbackInstance++;
  // AudioObjects.resize(Current_CallbackInstance+1);
  // AudioObjects[Current_CallbackInstance]=this;
  // StopRecording = false;

  // switch (BitPrec)
  // {
  //   case 8:
  //     InSampleType=DSP::e::SampleType::ST_uchar;
  //     #ifdef WINMMAPI
  //       Callback = (DWORD_PTR)(&DSP::u::AudioInput::waveInProc_uchar);
  //     #endif
  //     break;
  //   case 16:
  //     InSampleType=DSP::e::SampleType::ST_short;
  //     #ifdef WINMMAPI
  //       Callback = (DWORD_PTR)(&DSP::u::AudioInput::waveInProc_short);
  //     #endif
  //     break;
  //   default:
  //     InSampleType=DSP::e::SampleType::ST_short;
  //     #ifdef WINMMAPI
  //       Callback = (DWORD_PTR)(&DSP::u::AudioInput::waveInProc_short);
  //     #endif
  //     BitPrec=16;
  //     break;
  // }

  audio_inbuffer_size = DSP::f::GetAudioBufferSize(SamplingFreq, DSP::e::AudioBufferType::input);

  snd_object.select_input_device_by_number(WaveInDevNo); // use default device
//  DSP::input_callback_function  cp = std::bind(&DSP::u::AudioInput::SOUND_object_callback, this, std::placeholders::_1, std::placeholders::_2);
  if (snd_object.is_input_callback_supported() == true) {
    // use callbacks if available
    DSP::input_callback_function  cp = &DSP::u::AudioInput::SOUND_object_callback;
    snd_object.register_input_callback_object(this, cp); // use default device
#ifdef __DEBUG__
    DSP::log << "DSP::u::AudioInput::Init" << DSP::e::LogMode::second << "using callback mode" << endl;
#endif // __DEBUG__
  }
#ifdef __DEBUG__
  else {
    DSP::log << "DSP::u::AudioInput::Init" << DSP::e::LogMode::second << "using non-callback mode" << endl;
  }
#endif // __DEBUG__
  if (snd_object.open_PCM_device_4_input(NoOfOutputs, BitPrec, SamplingFreq, audio_inbuffer_size) > 0) {
    InBufferLen=NoOfOutputs*audio_inbuffer_size;
    for (ind = 0; ind < DSP::NoOfAudioInputBuffers; ind++)
    {
      InBuffers[ind].clear();
      InBuffers[ind].resize(InBufferLen);
    }
    EmptyBufferIndex=0;
    CurrentBufferIndex=DSP::NoOfAudioInputBuffers-1;
    BufferIndex=InBufferLen;

  }
  else
  { // error creating audio object

    InBufferLen=0;
    for (ind = 0; ind < DSP::NoOfAudioInputBuffers; ind++)
    {
      InBuffers[ind].clear();
    }
    EmptyBufferIndex=0; CurrentBufferIndex=0;
    BufferIndex=0;
  }
}


DSP::u::AudioOutput::~AudioOutput()
{
  // #ifdef WINMMAPI
  //   MMRESULT result;
  //   unsigned long ind;
  // #endif

  if (OutBufferLen != 0)
  { // if device was opened successfully
    snd_object.stop_playback();

    snd_object.close_PCM_device_output(true);

    // #ifdef WINMMAPI
    //   result = waveOutReset(hWaveOut);
    //   DSP::f::AudioCheckError(result);
    //   for (ind=0; ind< DSP::NoOfAudioOutputBuffers; ind++)
    //   {
    //     result=waveOutUnprepareHeader(hWaveOut,
    //       &(waveHeaderOut[ind]), sizeof(WAVEHDR));
    //     DSP::f::AudioCheckError(result);
    //   }

    //   #ifdef AUDIO_DEBUG_MESSAGES_ON
    //     DSP::log << "DSP::u::AudioOutput" << DSP::e::LogMode::second << "Closing DSP::u::AudioOutput" << endl;
    //   #endif
    //   result=waveOutClose(hWaveOut);
    //   while (result==WAVERR_STILLPLAYING)
    //   {
    //   //    #ifdef WINBASEAPI
    //     DSP::f::Sleep(100);
    //   //    #else
    //   //      sleep(100);
    //   //    #endif
    //     #ifdef AUDIO_DEBUG_MESSAGES_ON
    //       DSP::log << "DSP::u::AudioOutput" << DSP::e::LogMode::second << "Closing DSP::u::AudioOutput" << endl;
    //     #endif
    //     result=waveOutClose(hWaveOut);
    //   }
    //   DSP::f::AudioCheckError(result);
    // #endif


    // // 2) Free buffers
    // WaveOutBuffers.clear();
    // #ifdef WINMMAPI
    //   waveHeaderOut.clear();
    // #endif

    OutBuffer.clear();
  }

  // *************************** //
  // Free local resourses
  // 3) remove this audio object from the list
  // if (AudioObjects.size() > 0)
  // {
  //   AudioObjects.erase(AudioObjects.begin() + Current_CallbackInstance);
  // }
  // Next_CallbackInstance--;
}

DSP::u::AudioInput::~AudioInput()
{
  unsigned long ind;

  if (InBufferLen != 0)
  { // if device was opened successfully
    // StopRecording=true;
    snd_object.stop_recording();

    snd_object.close_PCM_device_input();

    for (ind=0; ind<DSP::NoOfAudioInputBuffers; ind++)
      InBuffers[ind].clear();
  }

  // *************************** //
  // Free local resourses
  // 3) remove this audio object from the list
  // if (AudioObjects.size() > 0)
  // {
  //   AudioObjects.erase(AudioObjects.begin() + Current_CallbackInstance);
  // }
  // Next_CallbackInstance--;
}

void DSP::u::AudioOutput::FlushBuffer(void)
{
  snd_object.append_playback_buffer(OutBuffer);

  // #ifdef WINMMAPI
  //   MMRESULT result;
  //   uint8_t *temp8;
  //   short *temp16;
  //   DSP::Float_ptr Sample;
  //   short Znak;
  //   uint32_t ind;

  //   // ************************************************** //
  //   // Send buffer to the audio device

  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //   DSP::log << "DSP::u::AudioOutput" << DSP::e::LogMode::second << "Flushing output buffer" << endl;
  // #endif

  // while (1)
  // {
  //   if (waveHeaderOut[NextBufferInd].dwFlags & WHDR_DONE)
  //   {
  //     result=waveOutUnprepareHeader(hWaveOut,
  //       &(waveHeaderOut[NextBufferInd]), sizeof(WAVEHDR));
  //     DSP::f::AudioCheckError(result);

  //     Sample=OutBuffer.data();
  //     // ************************************************** //
  //     // Converts samples format to the one suitable for the audio device
  //     switch (OutSampleType)
  //     {
  //       case DSP::e::SampleType::ST_uchar:
  //         temp8=(uint8_t *)(WaveOutBuffers[NextBufferInd].data());
  //         for (ind=0; ind<OutBufferLen; ind++)
  //         {
  //           if (*Sample < 0)
  //             Znak=-1;
  //           else
  //             Znak=1;

  //           *Sample*=127;
  //           if ((*Sample)*Znak > 127)
  //             *temp8=(unsigned char)(128+Znak*127);
  //           else
  //             *temp8=(unsigned char)(128+*Sample+Znak*0.5);

  //           Sample++;
  //           temp8++;
  //         }
  //         break;
  //       case DSP::e::SampleType::ST_short:
  //         temp16=(short *)(WaveOutBuffers[NextBufferInd].data());
  //         for (ind=0; ind<OutBufferLen; ind++)
  //         {
  //           if (*Sample < 0)
  //             Znak=-1;
  //           else
  //             Znak=1;

  //           *Sample*=SHRT_MAX;
  //           if ((*Sample)*Znak > SHRT_MAX)
  //             *temp16=(short)(Znak*SHRT_MAX);
  //           else
  //             *temp16=(short)(*Sample+Znak*0.5);
  //           Sample++;
  //           temp16++;
  //         }
  //         break;
  //       default:
  //         break;
  //     }

  //     result=waveOutPrepareHeader(hWaveOut,
  //       &(waveHeaderOut[NextBufferInd]), sizeof(WAVEHDR));
  //     DSP::f::AudioCheckError(result);

  //     if (IsPlayingNow == false)
  //     {
  //       if (NextBufferInd == 1)
  //       {
  //         for (ind=0; ind < DSP::NoOfAudioOutputBuffers-1; ind++) //one spare buffer
  //         {
  //           result=waveOutWrite(hWaveOut,
  //             &(waveHeaderOut[ind]), sizeof(WAVEHDR));
  //           DSP::f::AudioCheckError(result);
  //         }
  //         IsPlayingNow = true;
  //       }

  //     }
  //     else
  //     {
  //       result=waveOutWrite(hWaveOut,
  //         &(waveHeaderOut[NextBufferInd]), sizeof(WAVEHDR));
  //       DSP::f::AudioCheckError(result);
  //     }
  //     NextBufferInd++;
  //     NextBufferInd %= DSP::NoOfAudioOutputBuffers;

  //     break;
  //   }
  //   else
  //   {
  // //    Sleep(10);
  // #ifdef AUDIO_DEBUG_MESSAGES_ON
  //     DSP::log << "DSP::u::AudioOutput" << DSP::e::LogMode::second << "Waiting for free output buffer" << endl;
  // #endif
  //     DSP::f::Sleep(0);
  //   }
  // }
  // #endif // WINMMAPI
}

void DSP::u::AudioOutput::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (((DSP::u::AudioOutput *)block)->OutBufferLen == 0)
    return; // no audio device opened

  ((DSP::u::AudioOutput *)block)->OutBuffer[((DSP::u::AudioOutput *)block)->BufferIndex * ((DSP::u::AudioOutput *)block)->NoOfInputs+InputNo]=value;
  ((DSP::u::AudioOutput *)block)->NoOfInputsProcessed++;

  if (((DSP::u::AudioOutput *)block)->NoOfInputsProcessed == ((DSP::u::AudioOutput *)block)->NoOfInputs)
  {
    ((DSP::u::AudioOutput *)block)->BufferIndex++;
    ((DSP::u::AudioOutput *)block)->BufferIndex %= ((DSP::u::AudioOutput *)block)->audio_outbuffer_size;

    if (((DSP::u::AudioOutput *)block)->BufferIndex == 0)
    { // Data must be written to file from buffer
      //First we need to convert data from RawBuffer
      //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
      ((DSP::u::AudioOutput *)block)->FlushBuffer();
    }

    //NoOfInputsProcessed=0;
    if (((DSP::u::AudioOutput *)block)->IsUsingConstants)
    {
      for (unsigned int ind=0; ind < ((DSP::u::AudioOutput *)block)->NoOfInputs; ind++)
        if (((DSP::u::AudioOutput *)block)->IsConstantInput[ind])
        {
          ((DSP::u::AudioOutput *)block)->OutBuffer[((DSP::u::AudioOutput *)block)->BufferIndex * ((DSP::u::AudioOutput *)block)->NoOfInputs + InputNo]=
            ((DSP::u::AudioOutput *)block)->ConstantInputValues[ind];
          // ((DSP::u::AudioOutput *)block)->NoOfInputsProcessed++;
        }
    }
    ((DSP::u::AudioOutput *)block)->NoOfInputsProcessed = ((DSP::u::AudioOutput *)block)->InitialNoOfInputsProcessed;
  }

  #ifdef __DEBUG__
  #ifdef VerboseCompilation
    DSP::log << "DSP::u::AudioOutput" << DSP::e::LogMode::second
      << InputNo << ": " << setw(5) << setprecision(3) << fixed << value << endl;
  #endif
  #endif
}

int DSP::u::AudioInput::GetNoOfFreeBuffers(void)
{
  return (CurrentBufferIndex - EmptyBufferIndex + DSP::NoOfAudioInputBuffers) % DSP::NoOfAudioInputBuffers;
}

int DSP::u::AudioInput::GetNoOfBuffers(void)
{
  return DSP::NoOfAudioInputBuffers;
}

#define DSP_THIS ((DSP::u::AudioInput *)source)
/*! Fixed <b>2005.10.21</b> cleared up start-up and congestion code
 */
bool DSP::u::AudioInput::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  unsigned int ind;

  if (DSP_THIS->InBufferLen == 0)
  { // no audio device opened
  	#ifdef AUDIO_DEBUG_MESSAGES_ON
      DSP::log << "DSP::u::AudioInput::Execute" << DSP::e::LogMode::second << "No audio device opened" << endl;
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

  // If there are free buffers check whether the sound card has any audio data already available
  if (DSP_THIS->snd_object.get_input_callback_object() == NULL) {
    // callbacks are not used thus audio data has to be obtained directly from snd_object
    while (DSP_THIS->GetNoOfFreeBuffers() > 0) {
      DSP::e::SampleType InSampleType;
      std::vector<char> wave_in_raw_buffer;
      if (DSP_THIS->snd_object.get_wave_in_raw_buffer(InSampleType, wave_in_raw_buffer)) {
        DSP_THIS->SOUND_object_callback(InSampleType, wave_in_raw_buffer);
      }
      else {
        break;
      }
    }
  }

  // if no input buffer is ready return false (system will later return here)
  if (DSP_THIS->snd_object.is_device_recording() == true)
  { // but must check if data ready
    if ((((DSP_THIS->CurrentBufferIndex + 1) % DSP::NoOfAudioInputBuffers) == DSP_THIS->EmptyBufferIndex)
        && (DSP_THIS->BufferIndex == DSP_THIS->InBufferLen))
    { //no input samples are available
      // so we must wait for some

      #ifdef AUDIO_DEBUG_MESSAGES_ON
        DSP::log << "DSP::u::AudioInput::Execute" << DSP::e::LogMode::second << "no data ready we must wait" << endl;
      #endif
		  DSP::Clock::InputNeedsMoreTime[DSP_THIS->my_clock->MasterClockIndex] = true;
		  return false;
    }
    else
    {
      if (DSP_THIS->BufferIndex == DSP_THIS->InBufferLen)
      { //should try to release buffer
        if (((DSP_THIS->CurrentBufferIndex + 1) % DSP::NoOfAudioInputBuffers) != DSP_THIS->EmptyBufferIndex)
        {
          DSP_THIS->BufferIndex = 0;

          DSP_THIS->CurrentBufferIndex++;
          DSP_THIS->CurrentBufferIndex %= DSP::NoOfAudioInputBuffers;

          #ifdef AUDIO_DEBUG_MESSAGES_ON
            DSP::log << "DSP::u::AudioInput::Execute" << DSP::e::LogMode::second << "fresh data has finally arrived" << endl;
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
    DSP_THIS->snd_object.start_recording();

    //DSP_THIS->NextBufferInd = 0; //first buffer first
    DSP_THIS->EmptyBufferIndex = 2; //simulate two full buffers
    for (ind = 0; ind < DSP_THIS->EmptyBufferIndex; ind++) {
      memset(DSP_THIS->InBuffers[ind].data(), 0, DSP_THIS->InBufferLen*sizeof(DSP::Float));
    }
    DSP_THIS->BufferIndex = 0;

    //We cannot wait until input buffers fill-up, some other processing
    // blocks like DSP::u::AudioOutput might wait for their time
    /*! _bug if there are no DSP::u::AudioOutput blocks nothing
      * will stop boost in processing speed
      *
      * we might want to simulated two full input buffers
      */
  //    #ifdef AUDIO_DEBUG_MESSAGES_ON
  //      DSP::log << "DSP::u::AudioInput::Execute", "Audio recording started !!!");
  //    #endif
  //
  //    for (ind=0; ind<NoOfOutputs; ind++)
  //    {
  //      OutputBlocks[ind]->Execute(OutputBlocks_InputNo[ind],
  //                                 0.0, this);
  //      BufferIndex++;
  //    }
  //    return true;

    // program will return to this function and then output should be dealt with
    DSP::Clock::InputNeedsMoreTime[DSP_THIS->my_clock->MasterClockIndex] = true;
    return false;
  }

//  DSP::log << "DSP::u::AudioInput::Execute" << "No input buffer ready yet" << endl;
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
DSP::u::InputBuffer::InputBuffer(DSP::Clock_ptr ParentClock, int BufferSize_in,
                                   unsigned int NoOfChannels, DSP::e::BufferType cyclic,
                                   int NotificationsStep_in, DSP::Notify_callback_ptr func_ptr,
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
  UserCallbackID = (CallbackIdentifier & DSP::CallbackID_mask);

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
  Buffer.clear(); Buffer.resize(BufferSize*NoOfOutputs, 0.0);
  BufferIndex=0;

  if (cyclic == DSP::e::BufferType::cyclic)
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
  	(*NotificationFunction_ptr)(this, DSP::CallbackID_signal_start | UserCallbackID);
}

DSP::u::InputBuffer::~InputBuffer(void)
{
  if (NotificationFunction_ptr != NULL)
  	(*NotificationFunction_ptr)(this, DSP::CallbackID_signal_stop | UserCallbackID);

  Buffer.clear();
}

void DSP::u::InputBuffer::Notify(DSP::Clock_ptr clock)
{
  UNUSED_ARGUMENT(clock);

  if (NotificationFunction_ptr != NULL)
    (*NotificationFunction_ptr)(this, UserCallbackID);
}

// copies source_size bytes from the source buffer
// to block's internal buffer (the rest is set to zero
void DSP::u::InputBuffer::WriteBuffer(void *source,
  const long int &source_size, const bool &reset_buffer, const DSP::e::SampleType &source_DataType)
{
  int InputSampleSize;
  long int NoOfSourceSamples;
  int ind;

  if (reset_buffer == true) {
    BufferIndex = 0;
  }

  //! \TODO add option to append writen data at current position or add method AppendBuffer

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
      DSP::log << DSP::e::LogMode::Error << "DSP::u::InputBuffer::WriteBuffer" << DSP::e::LogMode::second
        << "(" << this->GetName() << ") source_size ("
        << source_size << ") doesn't match source_DataType" << endl;
    }
    if (BufferSize < NoOfSourceSamples)
    {
      stringstream tekst;
      DSP::log << DSP::e::LogMode::Error << "DSP::u::InputBuffer::WriteBuffer" << DSP::e::LogMode::second
        << "(" << this->GetName() << ") source_size ("
        << source_size << ") larger then Buffer size\n" << endl;
    }
  #endif

  // Each sample component counts separately
  NoOfSourceSamples*=NoOfOutputs;
  if (BufferSize > NoOfSourceSamples)
  {
    memset(Buffer.data(), 0, sizeof(DSP::Float)*NoOfSourceSamples);
  }

  switch (source_DataType)
  {
    case DSP::e::SampleType::ST_uchar:
      //convertion
      for (ind=0; ind<NoOfSourceSamples; ind++)
        Buffer[ind]=(((DSP::Float)((unsigned char  *)source)[ind]-0x80)
                    )/0x80;
      break;
    case DSP::e::SampleType::ST_short:
      //convertion
      for (ind=0; ind<NoOfSourceSamples; ind++)
        Buffer[ind]=((DSP::Float)((short *)source)[ind])/0x8000;
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

#define DSP_THIS ((DSP::u::InputBuffer *)source)
bool DSP::u::InputBuffer::OutputExecute(OUTPUT_EXECUTE_ARGS)
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
    memset(DSP_THIS->Buffer.data(), 0,
           sizeof(DSP::Float) * DSP_THIS->BufferSize * DSP_THIS->NoOfOutputs);

  return true;
}

bool DSP::u::InputBuffer::OutputExecute_single_channel(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  DSP_THIS->OutputBlocks[0]->EXECUTE_PTR(
      DSP_THIS->OutputBlocks[0],
      DSP_THIS->OutputBlocks_InputNo[0],
      DSP_THIS->Buffer[DSP_THIS->BufferIndex], source);

  DSP_THIS->BufferIndex++;
  DSP_THIS->BufferIndex %= DSP_THIS->BufferSize;

  if (DSP_THIS->BufferIndex == 0)
    memset(DSP_THIS->Buffer.data(), 0, sizeof(DSP::Float) * DSP_THIS->BufferSize);

  return true;
}

bool DSP::u::InputBuffer::OutputExecute_cyclic(OUTPUT_EXECUTE_ARGS)
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

bool DSP::u::InputBuffer::OutputExecute_cyclic_single_channel(OUTPUT_EXECUTE_ARGS)
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
DSP::u::OutputBuffer::OutputBuffer(unsigned int BufferSize_in, unsigned int NoOfInputs_in, DSP::e::BufferType cyclic,
                                     DSP::Clock_ptr ParentClock, int NotificationsStep_in,
                                     DSP::Notify_callback_ptr func_ptr, unsigned int CallbackIdentifier)
  : DSP::Block(), DSP::Source()
{
  DSP::Clock_ptr NotificationClock;

  Init(BufferSize_in, NoOfInputs_in, cyclic, NotificationsStep_in);

  NotificationFunction_ptr = func_ptr;
  CallbackFunction_ptr = NULL;
  UserData_ptr = NULL;
  UserCallbackID = (CallbackIdentifier & DSP::CallbackID_mask);

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
  	(*NotificationFunction_ptr)(this, DSP::CallbackID_signal_start | UserCallbackID);
}

DSP::u::OutputBuffer::OutputBuffer(unsigned int BufferSize_in, unsigned int NoOfInputs_in,
                                     DSP::e::BufferType cyclic, DSP::Clock_ptr ParentClock,
                                     int NotificationsStep_in, unsigned int NoOfOutputs_in,
                                     DSP::Buffer_callback_ptr func_ptr, unsigned int CallbackIdentifier)
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
  OutputsValues.clear();
  OutputsValues.resize(NoOfOutputs, 0.0);

  NotificationFunction_ptr = NULL;
  CallbackFunction_ptr = func_ptr;
  UserData_ptr = NULL;
  UserCallbackID = CallbackIdentifier;

  Execute_ptr = &InputExecute_with_output;

  if (CallbackFunction_ptr != NULL)
  {
    (*CallbackFunction_ptr)(DSP::Callback_Init, NoOfOutputs, OutputsValues, &UserData_ptr, UserCallbackID, this);
  }
}

DSP::u::OutputBuffer::OutputBuffer(unsigned int BufferSize_in, unsigned int NoOfInputs_in, DSP::e::BufferType cyclic,
                                     DSP::Clock_ptr ParentClock, DSP::Clock_ptr NotificationsClock,
                                     unsigned int NoOfOutputs_in, DSP::Buffer_callback_ptr func_ptr, unsigned int CallbackIdentifier)
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
  OutputsValues.clear(); OutputsValues.resize(NoOfOutputs);

  NotificationFunction_ptr = NULL;
  CallbackFunction_ptr = func_ptr;
  UserData_ptr = NULL;
  UserCallbackID = (DSP::CallbackID_mask & CallbackIdentifier);

  Execute_ptr = &InputExecute;
  OutputExecute_ptr = &OutputExecute;

  if (CallbackFunction_ptr != NULL)
  {
    (*CallbackFunction_ptr)(DSP::Callback_Init, NoOfOutputs, OutputsValues, &UserData_ptr, UserCallbackID, this);
  }
}

DSP::u::OutputBuffer::~OutputBuffer(void)
{
  if (NotificationFunction_ptr != NULL)
  	(*NotificationFunction_ptr)(this, DSP::CallbackID_signal_stop | UserCallbackID);
  if (CallbackFunction_ptr != NULL) {
    (*CallbackFunction_ptr)(DSP::Callback_Delete, NoOfOutputs, OutputsValues, &UserData_ptr, UserCallbackID, this);
  }

  Buffer.clear();
  OutputsValues.clear();
}

void DSP::u::OutputBuffer::Init(unsigned int BufferSize_in, unsigned int NoOfChannels,
                             DSP::e::BufferType cyclic, int NotificationsStep_in)
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

  OutputsValues.clear();
  OutputSamples_ready = false;

  switch (cyclic)
  {
    case DSP::e::BufferType::cyclic:
      IsCyclic=true;
      StopWhenFull=false;
      break;
    case DSP::e::BufferType::stop_when_full:
      IsCyclic=false;
      StopWhenFull=true;
      break;
    case DSP::e::BufferType::standard:
    default:
      IsCyclic=false;
      StopWhenFull=false;
      break;
  }

  BufferSize=BufferSize_in;
  Buffer.clear();
  Buffer.resize(BufferSize*NoOfInputs, 0.0);
  BufferIndex=0;
}

// copies dest_size bytes to the dest buffer
// from block's internal buffer (the rest is set to zero)
unsigned long DSP::u::OutputBuffer::ReadBuffer(void *dest, long int dest_size,
                                       long int reset, DSP::e::SampleType dest_DataType)
{
  int OutputSampleSize;
  long int NoOfDestSamples;
  int ind;

  #ifdef __DEBUG__
    if (dest == NULL)
    {
      DSP::log << DSP::e::LogMode::Error << "DSP::u::OutputBuffer::ReadBuffer" << DSP::e::LogMode::second << "dest == NULL !!!" << endl;
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
      DSP::log << DSP::e::LogMode::Error << "DSP::u::OutputBuffer::ReadBuffer" << DSP::e::LogMode::second
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
        DSP::Float_ptr in, out;
        int size, step;

        // BufferIndex \ BufferSize
        //128       128                   256           256
        #ifdef __DEBUG__
          if (reset - (BufferSize - BufferIndex) > INT_MAX) {
            DSP::log << DSP::e::LogMode::Error << "DSP::u::OutputBuffer::ReadBuffer" << DSP::e::LogMode::second << "step > INT_MAX" << endl;
          }
        #endif // __DEBUG__
        step = (int)(reset - (BufferSize - BufferIndex));

        out = Buffer.data(); in = out + step*NoOfInputs;
        size = (int)(NoOfInputs*sizeof(DSP::Float));
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

const DSP::Float_vector &DSP::u::OutputBuffer::AccessBuffer(void)
{
  return Buffer;
}


unsigned long DSP::u::OutputBuffer::NoOfSamples(void)
{
  return  BufferIndex; // BufferIndex/NoOfInputs;
}

long int DSP::u::OutputBuffer::GetBufferSize(int mode)
{
  switch (mode)
  {
    case 2:
      return  (long int)(BufferSize*NoOfInputs*sizeof(DSP::Float));
    case 1:
      return  BufferSize*NoOfInputs;
    case 0:
    default:
      return  BufferSize;
  }
}

void DSP::u::OutputBuffer::Notify(DSP::Clock_ptr clock)
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

#define DSP_THIS ((DSP::u::OutputBuffer *)block)
void DSP::u::OutputBuffer::InputExecute(INPUT_EXECUTE_ARGS)
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
          memcpy(&(DSP_THIS->Buffer[0]), &(DSP_THIS->Buffer[DSP_THIS->NoOfInputs]), (DSP_THIS->BufferSize-1)*DSP_THIS->NoOfInputs*sizeof(DSP::Float));
          memset(&(DSP_THIS->Buffer[(DSP_THIS->BufferSize - 1) * DSP_THIS->NoOfInputs]), 0, DSP_THIS->NoOfInputs*sizeof(DSP::Float));
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

void DSP::u::OutputBuffer::InputExecute_with_output(INPUT_EXECUTE_ARGS)
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
          memcpy(&(DSP_THIS->Buffer[0]), &(DSP_THIS->Buffer[DSP_THIS->NoOfInputs]), (DSP_THIS->BufferSize-1)*DSP_THIS->NoOfInputs*sizeof(DSP::Float));
          memset(&(DSP_THIS->Buffer[(DSP_THIS->BufferSize - 1) * DSP_THIS->NoOfInputs]), 0, DSP_THIS->NoOfInputs*sizeof(DSP::Float));
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

#define  DSP_THIS  ((DSP::u::OutputBuffer *)source)
//Execution as a source block
bool DSP::u::OutputBuffer::OutputExecute(OUTPUT_EXECUTE_ARGS)
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
    DSP::log << DSP::e::LogMode::Error << "DSP::u::OutputBuffer::OutputExecute" << DSP::e::LogMode::second << "DSP_THIS->OutputSamples_ready == false" << endl;
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
    void Sleep(uint32_t time)
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
          	/*! Fixed <b>2005.10.05</b> function called itself instead ::Sleep()
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

void DSP::f::Sleep(uint32_t time)
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
    uint32_t result;

    result = MsgWaitForMultipleObjects(
      1, //uint32_t nCount,
      const HANDLE* pHandles,
      false, //BOOL bWaitAll,
      time, //uint32_t dwMilliseconds,
      QS_ALLEVENTS | QS_ALLINPUT | QS_ALLPOSTMESSAGE | QS_PAINT | QS_RAWINPUT | QS_SENDMESSAGE |QS_TIMER // uint32_t dwWakeMask
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
  uint32_t ValuesRead;
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
    ValuesRead=(uint32_t)fread(&(this->file_version), sizeof(unsigned char), 1, plik);
    this->header_size = int(ValuesRead * sizeof(unsigned char));

    switch (this->file_version)
    {
      case 0x00:
        ValuesRead=(uint32_t)fread(&(this->sample_dim), sizeof(unsigned char), 1, plik);
        this->header_size += ValuesRead * (int)sizeof(unsigned char);

        ValuesRead=(uint32_t)fread(&(temp_uchar), sizeof(unsigned char), 1, plik);
        this->sample_type = (DSP::e::SampleType)temp_uchar;
        this->header_size += ValuesRead * (int)sizeof(unsigned char);

        ValuesRead=(uint32_t)fread(&(temp_uchar), sizeof(unsigned char), 1, plik);
        this->NoOfVectors = temp_uchar;
        this->header_size += ValuesRead * (int)sizeof(unsigned char);
        break;
      case 0x01:
        ValuesRead=(uint32_t)fread(&(this->Fp),sizeof(unsigned int), 1, plik);
        this->header_size += ValuesRead * (int)sizeof(unsigned int);

        ValuesRead=(uint32_t)fread(&(this->sample_dim), sizeof(unsigned char), 1, plik);
        this->header_size += ValuesRead * (int)sizeof(unsigned char);

        ValuesRead=(uint32_t)fread(&(temp_uchar), sizeof(unsigned char), 1, plik);
        this->sample_type = (DSP::e::SampleType)temp_uchar;
        this->header_size += ValuesRead * (int)sizeof(unsigned char);

        ValuesRead=(uint32_t)fread(&(temp_uchar), sizeof(unsigned char), 1, plik);
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
    DSP::log << DSP::e::LogMode::Error << "DSP_LoadCoef::Open" << DSP::e::LogMode::second << "File doesn't exist or unsupported file format." << endl;
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
  uint32_t ValuesRead;
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
        ValuesRead=(uint32_t)fread(&(vector_size), sizeof(unsigned short), 1, plik);

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
bool DSP::LoadCoef::Load(DSP::Complex_vector &FIR_coef, int vector_index)
{
  FILE *plik;
  std::vector<uint8_t> buffer;
  uint32_t ValuesRead, Values2Read;
  int current_vector_no;

  current_vector_no = 0;
  Values2Read = 0;

  plik=fopen(this->filename.c_str(), "rb");
  if (plik != NULL)
  {
//    if (fseeko64(plik, this->header_size, SEEK_SET)==0)
    if (fseek(plik, this->header_size, SEEK_SET)==0)
    {
      ValuesRead=(uint32_t)fread(&(Values2Read), sizeof(unsigned short), 1, plik);
      if (ValuesRead == 0)
        return false;

      while (current_vector_no < vector_index)
      { // skip preceeding vectors
//        fseeko64(plik, vector_size * this->sample_size * this->sample_dim, SEEK_CUR);
        fseek(plik, Values2Read * this->sample_size * this->sample_dim, SEEK_CUR);
        current_vector_no++;

        ValuesRead=(uint32_t)fread(&(Values2Read), sizeof(unsigned short), 1, plik);
        if (ValuesRead == 0)
          return false;
      }

      FIR_coef.resize(Values2Read);
      buffer.resize(this->sample_size * this->sample_dim);
      for (unsigned int ind = 0; ind < Values2Read; ind++)
      {
        ValuesRead=(uint32_t)fread(buffer.data(), this->sample_size*sizeof(unsigned char), this->sample_dim, plik);

        switch (this->sample_type)
        {
          case DSP::e::SampleType::ST_float:
            FIR_coef[ind] = DSP::Complex(((float *)buffer.data())[0], ((float *)buffer.data())[1]);
            break;
          case DSP::e::SampleType::ST_double:
            FIR_coef[ind] = DSP::Complex(((double *)buffer.data())[0], ((double *)buffer.data())[1]);
            break;
          case DSP::e::SampleType::ST_long_double:
            FIR_coef[ind] = DSP::Complex(((long double *)buffer.data())[0], ((long double *)buffer.data())[1]);
            break;
          default:
            FIR_coef[ind] = DSP::Complex(0.0, 0.0);
            break;
        }
      }
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
bool DSP::LoadCoef::Load(DSP::Float_vector &FIR_coef, int vector_index)
{
  FILE *plik;
  std::vector<uint8_t> buffer;
  uint32_t ValuesRead, Values2Read;
  int current_vector_no;

  current_vector_no = 0;
  Values2Read = 0;

  plik=fopen(this->filename.c_str(), "rb");
  if (plik != NULL)
  {
//    if (fseeko64(plik, this->header_size, SEEK_SET)==0)
    if (fseek(plik, this->header_size, SEEK_SET)==0)
    {
      ValuesRead=(uint32_t)fread(&(Values2Read), sizeof(unsigned short), 1, plik);
      if (ValuesRead == 0)
        return false;

      while (current_vector_no < vector_index)
      { // skip preceding vectors
//        fseeko64(plik, vector_size * this->sample_size * this->sample_dim, SEEK_CUR);
        fseek(plik, Values2Read * this->sample_size * this->sample_dim, SEEK_CUR);
        current_vector_no++;

        ValuesRead=(uint32_t)fread(&(Values2Read), sizeof(unsigned short), 1, plik);
        if (ValuesRead == 0)
          return false;
      }

      FIR_coef.resize(Values2Read);
      buffer.resize(this->sample_size * this->sample_dim);
      for (unsigned int ind = 0; ind < Values2Read; ind++)
      {
        ValuesRead=(uint32_t)fread(buffer.data(), this->sample_size*sizeof(unsigned char), this->sample_dim, plik);

        switch (this->sample_type)
        {
          case DSP::e::SampleType::ST_float:
            FIR_coef[ind] = DSP::Float(((float *)buffer.data())[0]);
            break;
          case DSP::e::SampleType::ST_double:
            FIR_coef[ind] = DSP::Float(((double *)buffer.data())[0]);
            break;
          case DSP::e::SampleType::ST_long_double:
            FIR_coef[ind] = DSP::Float(((long double *)buffer.data())[0]);
            break;
          default:
            FIR_coef[ind] = DSP::Float(0.0);
            break;
        }
      }
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

  DSP::T_FLT_header header_flt;
  header_flt.version(0x00);
  header_flt.sample_type(0x0000); // floating point: float (4B)
  header_flt.sampling_rate(Fp);
  header_flt.no_of_channels((unsigned char)NoOfChannels);

  FileHandle=fopen(filename.c_str(), "wb");
  if (FileHandle != NULL)
    fwrite(&header_flt, sizeof(DSP::T_FLT_header), 1, FileHandle);

  return FileHandle;
}

// Saves to *.flt file len samples from real valued vector
bool DSP::f::SaveVector(const string &filename, const DSP::Float_vector &vector, const unsigned int &Fp)
{
  FILE *FileHandle = CreateFLTfile(filename, 1U, Fp);

  if (FileHandle != NULL)
  {
    fwrite(vector.data(), sizeof(DSP::Float), vector.size(), FileHandle);
//    fflush(FileHandle);
    fclose(FileHandle);

    return true;
  }
  return false;
}
// Saves to *.flt file len samples from complex valued vector
bool DSP::f::SaveVector(const string &filename, const DSP::Complex_vector &vector, const unsigned int &Fp)
{
  FILE *FileHandle = CreateFLTfile(filename, 2U, Fp);

  if (FileHandle != NULL)
  {
    fwrite(vector.data(), 2*sizeof(DSP::Float), vector.size(), FileHandle);
//    fflush(FileHandle);
    fclose(FileHandle);

    return true;
  }
  return false;
}


//unsigned long DSP::WMM_object_t::Next_CallbackInstance=0;
std::vector<DSP::SOUND_object_t *> DSP::SOUND_object_t::CallbackSoundObjects;

const DSP::SOUND_object_t * DSP::SOUND_object_t::get_CallbackSoundObject(const long int &instance_number) {
  return (CallbackSoundObjects[instance_number]);
}

DSP::SOUND_object_t::SOUND_object_t() {
  AudioOutput_object = NULL;
  AudioInput_object = NULL;

  Current_CallbackInstance=get_free_CallbackInstance();
  CallbackSoundObjects[Current_CallbackInstance]=this;
}

DSP::u::AudioInput *DSP::SOUND_object_t::get_input_callback_object() {
  return AudioInput_object;
}

bool DSP::SOUND_object_t::input_callback_call(const DSP::e::SampleType &InSampleType, const std::vector<char> &wave_in_raw_buffer) {
  if (AudioInput_object == NULL)
    return false;
  return (AudioInput_object->*AudioInput_callback)(InSampleType, wave_in_raw_buffer);
}

unsigned long DSP::SOUND_object_t::get_free_CallbackInstance(void) {
  bool is_free_slot_available = false;
  unsigned long free_slot_index;

  for (unsigned int ind = 0; ind < CallbackSoundObjects.size(); ind++) {
    if (CallbackSoundObjects[ind] == NULL) {
      is_free_slot_available = true;
      free_slot_index = ind;
      break;
    }
  }

  if (is_free_slot_available == false) {
    CallbackSoundObjects.push_back(NULL);
    free_slot_index = (unsigned long)(CallbackSoundObjects.size()-1);
  }

  return free_slot_index;
}


DSP::u::AudioOutput *DSP::SOUND_object_t::get_output_callback_object() {
  return AudioOutput_object;
}

bool DSP::SOUND_object_t::output_callback_call(const DSP::e::SampleType &OutSampleType, const std::vector<char> &wave_out_raw_buffer) {
//  return (AudioOutput_object->*AudioOutput_callback)();
  if (AudioOutput_object == NULL)
    return false;
  return (AudioOutput_object->*AudioOutput_callback)(OutSampleType, wave_out_raw_buffer);
}

//bool DSP::SOUND_object_t::register_input_callback_object(DSP::u::AudioInput *callback_object, bool(DSP::u::AudioInput::*cp)(const DSP::e::SampleType &, const std::vector<char> &)) {
bool DSP::SOUND_object_t::register_input_callback_object(DSP::u::AudioInput *callback_object, input_callback_function &cp) {
  if (is_input_callback_supported() == true) {
    AudioInput_object = callback_object;
    AudioInput_callback = cp;
    return true;
  }
  return false;
}

bool DSP::SOUND_object_t::register_output_callback_object(DSP::u::AudioOutput *callback_object, output_callback_function &cp) {
  if (is_output_callback_supported() == true) {
    AudioOutput_object = callback_object;
    AudioOutput_callback = cp;
    return true;
  }
  return false;
}


long int DSP::SOUND_object_t::get_current_CallbackInstance() {
  return Current_CallbackInstance;
}

DSP::SOUND_object_t::~SOUND_object_t() {
  if (CallbackSoundObjects.size() > 0)
  {
    //CallbackObjects.erase(CallbackObjects.begin() + Current_CallbackInstance);
    CallbackSoundObjects[Current_CallbackInstance] = NULL;
  }
}

