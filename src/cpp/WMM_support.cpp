/*! \file WMM_support.cpp
 * WMM support code file
 *
 * \author Marek Blok
 */

#include <WMM_support.h>

#include <DSP_lib.h> // for logging 

DSP::WMM_object_t::WMM_object_t() : DSP::SOUND_object_t()
{
  is_device_input_open = false;
  WaveInDevNo = UINT_MAX;
  is_device_output_open = false;
  WaveOutDevNo = UINT_MAX;

  NextBufferOutInd=0;
  StopPlayback = false;
  IsPlayingNow = false;

  NextBufferOutInd = 0;
  StopRecording = false;
  IsRecordingNow = false;
};

DSP::WMM_object_t::~WMM_object_t()
{
  if (is_device_input_open) {
    close_PCM_device_input();
  }
  if (is_device_output_open) {
    close_PCM_device_output(true);
  }

}

void CALLBACK DSP::WMM_object_t::waveOutProc(HWAVEOUT hwo, UINT uMsg,
  uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2)
{
  UNUSED_ARGUMENT(hwo);
  UNUSED_ARGUMENT(uMsg);
  UNUSED_ARGUMENT(dwInstance);
  UNUSED_ARGUMENT(dwParam1);
  UNUSED_ARGUMENT(dwParam2);
#ifdef __DEBUG__
#ifdef AUDIO_DEBUG_MESSAGES_ON
//  MMRESULT result;
  DSP::u::AudioOutput *Current;
  bool AllDone;
  int ind;
  std::string tekst;

  Current = AudioObjects[dwInstance];

  switch (uMsg)
  {
    case WOM_OPEN:
      DSP::log << "DSP::WMM_object_t::waveOutProc" << DSP::e::LogMode::second
        << "WOM_OPEN(" << (int)dwInstance << ")" << std::endl;
      break;
    case WOM_CLOSE:
      DSP::log << "DSP::WMM_object_t::waveOutProc" << DSP::e::LogMode::second
        << "WOM_CLOSE(" << (int)dwInstance << ")" << std::endl;
      break;
    case WOM_DONE:
      DSP::log << "DSP::WMM_object_t::waveOutProc" << DSP::e::LogMode::second
        << "WOM_DONE(" << (int)dwInstance << ")" << std::endl;

      if (Current->StopPlayback)
      {
        DSP::log << "DSP::WMM_object_t::waveOutProc" << DSP::e::LogMode::second << "StopPlayback is set" << std::endl;
        return;
      }
      else
      {
        AllDone=true;
        for (ind=0; ind < DSP_NoOfAudioOutputBuffers; ind++)
          AllDone &= (Current->waveHeaderOut[ind].dwFlags & WHDR_DONE);
        if (AllDone)
          DSP::log << "DSP::WMM_object_t::waveOutProc" << DSP::e::LogMode::second << "All buffers had been used - nothing to play" << std::endl;
      }
      break;
  }
#endif
#endif
}


void DSP::WMM_object_t::log_driver_data() {
  /*
  int val;

  DSP::log << "WMM library version: " << SND_LIB_VERSION_STR << std::endl;

  DSP::log << std::endl;
  DSP::log << "PCM stream types:" << std::endl;
  for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
    DSP::log << "  " << snd_pcm_stream_name((snd_pcm_stream_t)val) << std::endl;

  DSP::log << std::endl;
  DSP::log << "PCM access types:" << std::endl;
  for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
    DSP::log << "  " << snd_pcm_access_name((snd_pcm_access_t)val) << std::endl;

  DSP::log << std::endl;
  DSP::log << "PCM formats:" << std::endl;
  for (val = 0; val <= SND_PCM_FORMAT_LAST; val++)
    if (snd_pcm_format_name((snd_pcm_format_t)val) != NULL)
      DSP::log << "  " << snd_pcm_format_name((snd_pcm_format_t)val) <<
        "(" << snd_pcm_format_description((snd_pcm_format_t)val) << ")" << std::endl;

  DSP::log << std::endl;
  DSP::log << "PCM subformats:" << std::endl;
  for (val = 0; val <= SND_PCM_SUBFORMAT_LAST; val++)
    DSP::log << "  " << snd_pcm_subformat_name((snd_pcm_subformat_t)val) <<
      "(" << snd_pcm_subformat_description((snd_pcm_subformat_t)val) << ")" << std::endl;

  DSP::log << std::endl;
  DSP::log << "PCM states:" << std::endl;
  for (val = 0; val <= SND_PCM_STATE_LAST; val++)
    DSP::log << "  " << snd_pcm_state_name((snd_pcm_state_t)val) << std::endl;
  */
}

//    Callback = (DWORD_PTR)(&DSP::u::AudioOutput::waveOutProc);

unsigned int DSP::WMM_object_t::select_output_device_by_number(const unsigned int &device_number) {
  WaveOutDevNo = device_number;

  return WaveOutDevNo;
}

unsigned int DSP::WMM_object_t::select_input_device_by_number(const unsigned int &device_number) {
  WaveInDevNo = device_number;

  return WaveInDevNo;
}

bool DSP::WMM_object_t::stop_playback(void) {
  StopPlayback = true;

  // if there are still buffers that haven't been yet sent to sound card then do it now
  if (IsPlayingNow == false)
  {
    if (NextBufferOutInd > 0) //all but one spare buffer are filled up
    { // send all data from buffers to soundcard to start playback
      for (unsigned int ind=0; ind < NextBufferOutInd; ind++) //one spare buffer
      {
        result=waveOutWrite(hWaveOut,
          &(waveHeaderOut[ind]), sizeof(WAVEHDR));
        DSP::f::AudioCheckError(result);
      }
      IsPlayingNow = true;
    }
  }

  return true;
}

bool DSP::WMM_object_t::stop_recording(void) {
  StopRecording = true;
  //! \TODO can we do more ?

  return true;
}

bool DSP::WMM_object_t::is_device_playing(void) {
  return IsPlayingNow;
}

bool DSP::WMM_object_t::is_device_recording(void) {
  return IsRecordingNow;
}

long DSP::WMM_object_t::open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_inbuffer_size) {
  if (is_device_input_open)
  {
    DSP::log << "DSP::WMM_object_t::open_PCM_device_4_input" << DSP::e::LogMode::second << "Device has been already opened: closing device before reopening" << std::endl;
    close_PCM_device_input();
  }

  switch (no_of_bits)
  {
    case 8:
      InSampleType=DSP::e::SampleType::ST_uchar;
      break;
    case 16:
      InSampleType=DSP::e::SampleType::ST_short;
      break;
    default:
      InSampleType=DSP::e::SampleType::ST_short;
      no_of_bits=16;
      break;
  }

  DWORD_PTR Callback;
  DWORD_PTR dwInstance;
  uint32_t fdwOpen;
  if (get_input_callback_object() != NULL) {
    Callback = (DWORD_PTR)(&DSP::WMM_object_t::waveInProc);
    dwInstance = get_current_CallbackInstance();
    fdwOpen = CALLBACK_FUNCTION;
  }
  else {
    Callback = (DWORD_PTR)(NULL);
    dwInstance = (DWORD_PTR)(0);
    fdwOpen = 0;
  }

  //Wypeniamy struktur wfx
  wfx.wf.wFormatTag=WAVE_FORMAT_PCM;
  wfx.wf.nChannels=(uint16_t)no_of_channels;
  wfx.wf.nSamplesPerSec=(UINT)sampling_rate;
  wfx.wBitsPerSample=(uint16_t)no_of_bits;
  wfx.wf.nAvgBytesPerSec=wfx.wf.nSamplesPerSec*(wfx.wBitsPerSample/8);
  wfx.wf.nBlockAlign=(uint16_t)(wfx.wf.nChannels*(wfx.wBitsPerSample/8));

  fdwOpen |= WAVE_FORMAT_DIRECT;
  if (waveInGetNumDevs() <= WaveInDevNo)
    result=waveInOpen(&hWaveIn,
      WAVE_MAPPER, //&DeviceID,
      (WAVEFORMATEX *)(&wfx),
      Callback,
      dwInstance, //CallbackInstance,
      fdwOpen //| WAVE_MAPPED //CALLBACK_NULL
      );
  else
    result=waveInOpen(&hWaveIn,
      WaveInDevNo, //&DeviceID,
      (WAVEFORMATEX *)(&wfx),
      Callback,
      dwInstance, //CallbackInstance,
      fdwOpen //| WAVE_MAPPED //CALLBACK_NULL
      );

  if (DSP::f::AudioCheckError(result) == false)
  { // no errors
    is_device_input_open = true;

    waveHeaderIn.resize(2);
    WaveInBufferLen=wfx.wf.nBlockAlign*audio_inbuffer_size;
    WaveInBuffers.resize(2);
    /*! \bug <b>2006.08.13</b> when 8bit audio stream is created initial values should be 0x80 or 0x79 not 0x00
      */
    WaveInBuffers[0].clear(); WaveInBuffers[0].resize(WaveInBufferLen, 0);
    WaveInBuffers[1].clear(); WaveInBuffers[1].resize(WaveInBufferLen, 0);

    waveHeaderIn[0].lpData=(char *)(WaveInBuffers[0].data());
    waveHeaderIn[0].dwBufferLength=no_of_channels*audio_inbuffer_size*(no_of_bits/8); //sizeof(short);
    waveHeaderIn[0].dwFlags= 0; // WHDR_BEGINLOOP | WHDR_ENDLOOP;
    waveHeaderIn[0].dwLoops=0;

    result=waveInPrepareHeader(hWaveIn,
      &(waveHeaderIn[0]), sizeof(WAVEHDR));
    DSP::f::AudioCheckError(result);
  //  waveHeaderIn[0].dwFlags= WHDR_DONE; // WHDR_BEGINLOOP | WHDR_ENDLOOP;

    waveHeaderIn[1].lpData=(char *)(WaveInBuffers[1].data());
    waveHeaderIn[1].dwBufferLength=no_of_channels*audio_inbuffer_size*(no_of_bits/8); //sizeof(short);
    waveHeaderIn[1].dwFlags= 0; // WHDR_BEGINLOOP | WHDR_ENDLOOP;
    waveHeaderIn[1].dwLoops=0;

    result=waveInPrepareHeader(hWaveIn,
      &(waveHeaderIn[1]), sizeof(WAVEHDR));
    DSP::f::AudioCheckError(result);
    //  waveHeaderIn[1].dwFlags= WHDR_DONE; // WHDR_BEGINLOOP | WHDR_ENDLOOP;

    IsRecordingNow = false;
    NextBufferInInd = 0;

    return sampling_rate; 
  }

  // error creating audio object
  waveHeaderIn.clear(); WaveInBufferLen=0;
  WaveInBuffers.clear();

  return -1;
}

long DSP::WMM_object_t::open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size) {
  if (is_device_output_open)
  {
    DSP::log << "DSP::WMM_object_t::open_PCM_device_4_output" << DSP::e::LogMode::second << "Device has been already opened: closing device before reopening" << std::endl;
    close_PCM_device_output(false);
  }

  switch (no_of_bits)
  {
    case 8:
      OutSampleType=DSP::e::SampleType::ST_uchar;
      break;
    case 16:
      OutSampleType=DSP::e::SampleType::ST_short;
      break;
    default:
      OutSampleType=DSP::e::SampleType::ST_short;
      no_of_bits=16;
      break;
  }

  //Wypeniamy struktur wfx
  wfx.wf.wFormatTag=WAVE_FORMAT_PCM;
  wfx.wf.nChannels=(uint16_t)no_of_channels;
  wfx.wf.nSamplesPerSec=(UINT)sampling_rate;
  wfx.wBitsPerSample=(uint16_t)no_of_bits;
  wfx.wf.nAvgBytesPerSec=wfx.wf.nSamplesPerSec*(wfx.wBitsPerSample/8);
  wfx.wf.nBlockAlign=(uint16_t)(wfx.wf.nChannels*(wfx.wBitsPerSample/8));

  if (WaveOutDevNo >= (UINT)waveOutGetNumDevs())
    result=waveOutOpen(&hWaveOut,
      WAVE_MAPPER, //&DeviceID,
      (WAVEFORMATEX *)(&wfx),
      DWORD_PTR(NULL), //Callback,
      0, //Current_CallbackInstance, 
      WAVE_ALLOWSYNC | WAVE_FORMAT_DIRECT //| WAVE_MAPPED //CALLBACK_NULL
      //CALLBACK_FUNCTION | WAVE_ALLOWSYNC | WAVE_FORMAT_DIRECT //| WAVE_MAPPED //CALLBACK_NULL
      );
  else
    result=waveOutOpen(&hWaveOut,
      WaveOutDevNo, //&DeviceID,
      (WAVEFORMATEX *)(&wfx),
      DWORD_PTR(NULL), //Callback,
      0, // Current_CallbackInstance, //CallbackInstance,
      WAVE_ALLOWSYNC | WAVE_FORMAT_DIRECT //| WAVE_MAPPED //CALLBACK_NULL
//      CALLBACK_FUNCTION | WAVE_ALLOWSYNC | WAVE_FORMAT_DIRECT //| WAVE_MAPPED //CALLBACK_NULL
      );

  if (DSP::f::AudioCheckError(result) == false)
  { // everything  is ok
    is_device_output_open = true;

    waveHeaderOut.resize(DSP::NoOfAudioOutputBuffers);
    WaveOutBufferLen=wfx.wf.nBlockAlign*audio_outbuffer_size;
    WaveOutBuffers.resize(DSP::NoOfAudioOutputBuffers);
    for (unsigned int ind=0; ind< DSP::NoOfAudioOutputBuffers; ind++)
    {
      WaveOutBuffers[ind].clear();
      WaveOutBuffers[ind].resize(WaveOutBufferLen, 0);
    }


    for (unsigned int ind=0; ind< DSP::NoOfAudioOutputBuffers; ind++)
    {
      waveHeaderOut[ind].lpData=(char *)(WaveOutBuffers[ind].data());
      waveHeaderOut[ind].dwBufferLength=no_of_channels*audio_outbuffer_size*(no_of_bits/8); //sizeof(short);
      waveHeaderOut[ind].dwFlags= 0; // WHDR_BEGINLOOP | WHDR_ENDLOOP;
      waveHeaderOut[ind].dwLoops=0;

      result=waveOutPrepareHeader(hWaveOut,
        &(waveHeaderOut[ind]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);
      waveHeaderOut[ind].dwFlags= WHDR_DONE; // WHDR_BEGINLOOP | WHDR_ENDLOOP;
    }

    return sampling_rate;
  }
  else
  { //error while creating audio output
    waveHeaderOut.clear();
    WaveOutBuffers.clear();
    WaveOutBufferLen = 0;
  }

  return -1;
}

bool DSP::WMM_object_t::close_PCM_device_input(void) {
  result = waveInReset(hWaveIn);
  DSP::f::AudioCheckError(result);
  result=waveInUnprepareHeader(hWaveIn,
    &(waveHeaderIn[0]), sizeof(WAVEHDR));
  DSP::f::AudioCheckError(result);
  result=waveInUnprepareHeader(hWaveIn,
    &(waveHeaderIn[1]), sizeof(WAVEHDR));
  DSP::f::AudioCheckError(result);

  #ifdef AUDIO_DEBUG_MESSAGES_ON
    DSP::log << "DSP::u::AudioInput" << DSP::e::LogMode::second << "Closing DSP::u::AudioInput" << std::endl;
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
      DSP::log << "DSP::u::AudioInput" << DSP::e::LogMode::second << "Closing DSP::u::AudioInput" << std::endl;
    #endif
    result=waveInClose(hWaveIn);
  }
  DSP::f::AudioCheckError(result);

  is_device_input_open = false;

  // 2) Free buffers
  WaveInBuffers[0].clear();
  WaveInBuffers[1].clear();
  WaveInBuffers.clear();

  waveHeaderIn.clear();

  return true;
}

bool DSP::WMM_object_t::close_PCM_device_output(const bool &do_drain) {
  stop_playback(); // just to be sure that all prepared buffershave been sent to sound card

  if (do_drain == true) {
    bool still_playing = true;
    while (still_playing) {
      int counter = 0;
      for (unsigned int ind=0; ind < DSP::NoOfAudioOutputBuffers; ind++) //one spare buffer
      {
        if (waveHeaderOut[NextBufferOutInd].dwFlags & WHDR_DONE)
          counter++;
      }
      if (counter == DSP::NoOfAudioOutputBuffers) {
        still_playing = false;
      }
      else {
        // let system process others and then check again
        DSP::f::Sleep(0);
      }
    }
  }

  result = waveOutReset(hWaveOut);
  DSP::f::AudioCheckError(result);
  
  for (unsigned int ind=0; ind< DSP::NoOfAudioOutputBuffers; ind++)
  {
    result=waveOutUnprepareHeader(hWaveOut,
      &(waveHeaderOut[ind]), sizeof(WAVEHDR));
    DSP::f::AudioCheckError(result);
  }

  #ifdef AUDIO_DEBUG_MESSAGES_ON
    DSP::log << "DSP::u::AudioOutput" << DSP::e::LogMode::second << "Closing DSP::u::AudioOutput" << std::endl;
  #endif
  result=waveOutClose(hWaveOut);
  while (result==WAVERR_STILLPLAYING)
  {
    DSP::f::Sleep(100);
    #ifdef AUDIO_DEBUG_MESSAGES_ON
      DSP::log << "DSP::u::AudioOutput" << DSP::e::LogMode::second << "Closing DSP::u::AudioOutput" << std::endl;
    #endif
    result=waveOutClose(hWaveOut);
  }  
  DSP::f::AudioCheckError(result);

  is_device_output_open = false;

  // 2) Free buffers
  WaveOutBuffers.clear();
  #ifdef WINMMAPI
    waveHeaderOut.clear();
  #endif

  #ifdef AUDIO_DEBUG_MESSAGES_ON
    DSP::log << "WMM PCM sound closed" << std::endl;
  #endif

  is_device_input_open = false;
  NextBufferOutInd=0;
  StopPlayback = false;
  IsPlayingNow = false;

  return true; // device has been closed
}

bool DSP::WMM_object_t::start_recording(void) {
  #ifdef AUDIO_DEBUG_MESSAGES_ON
    DSP::log << "DSP::u::AudioInput::Execute" << DSP::e::LogMode::second << "Starting recording using two wave buffers" << std::endl;
  #endif

  // if (get_input_callback_object() == NULL) {
  //   DSP::log << DSP::e::LogMode::Error << "DSP::WMM_object_t::start_recording" << DSP::e::LogMode::second << "No AudioInput object registered for callbacks" << std::endl;
  //   return false;
  // }

  result=waveInAddBuffer(hWaveIn, &(waveHeaderIn[0]), sizeof(WAVEHDR));
  DSP::f::AudioCheckError(result);
  result=waveInAddBuffer(hWaveIn, &(waveHeaderIn[1]), sizeof(WAVEHDR));
  DSP::f::AudioCheckError(result);
  NextBufferInInd = 0;

  result=waveInStart(hWaveIn);
  DSP::f::AudioCheckError(result);
  IsRecordingNow = true;
  //now just wait for buffer to fill up
  // which should call waveInProc

  return true;
}

bool DSP::WMM_object_t::is_input_callback_supported(void) {
  return true;
}

bool DSP::WMM_object_t::is_output_callback_supported(void) {
  return false;
}


bool DSP::WMM_object_t::get_wave_in_raw_buffer(DSP::e::SampleType &InSampleType_in, std::vector<char> &wave_in_raw_buffer) {
  if (StopRecording)
    return false;
  else
  {
    if (waveHeaderIn[NextBufferInInd].dwFlags & WHDR_DONE)
    {
      result=waveInUnprepareHeader(hWaveIn, &(waveHeaderIn[NextBufferInInd]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);
    
      InSampleType_in = this->InSampleType;
      std::swap(wave_in_raw_buffer, WaveInBuffers[NextBufferInInd]);
      WaveInBuffers[NextBufferInInd].resize(WaveInBufferLen, 0);
      waveHeaderIn[NextBufferInInd].lpData=(char *)(WaveInBuffers[NextBufferInInd].data());

      //add put back into recording queue
      result=waveInPrepareHeader(hWaveIn, &(waveHeaderIn[NextBufferInInd]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);
      result=waveInAddBuffer(hWaveIn, &(waveHeaderIn[NextBufferInInd]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);

      NextBufferInInd++; NextBufferInInd %= 2; //just two buffers
    }
    else
    {
#ifdef __DEBUG__
#ifdef AUDIO_DEBUG_MESSAGES_ON
        DSP::log << "DSP::WMM_object_t::get_wave_in_raw_buffer" << DSP::e::LogMode::second << "Wrong audio frame ready or other unexpected error" << std::endl;
#endif
#endif
      return false;
    }
  }
  return true;
}

long DSP::WMM_object_t::append_playback_buffer(DSP::Float_vector &float_buffer) {
  uint8_t *temp8;
  short *temp16;
  short Znak;
  uint32_t ind;

  // ************************************************** //
  // Send float_buffer to the audio device

  #ifdef AUDIO_DEBUG_MESSAGES_ON
    DSP::log << "DSP::WMM_object_t::append_playback_buffer" << DSP::e::LogMode::second << "Flushing output buffer" << std::endl;
  #endif

  while (1)
  {
    if (waveHeaderOut[NextBufferOutInd].dwFlags & WHDR_DONE)
    {
      result=waveOutUnprepareHeader(hWaveOut,
        &(waveHeaderOut[NextBufferOutInd]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);

      DSP::Float_ptr Sample=float_buffer.data();
      // ************************************************** //
      // Converts samples format to the one suitable for the audio device
      switch (OutSampleType)
      {
        case DSP::e::SampleType::ST_uchar:
          temp8=(uint8_t *)(WaveOutBuffers[NextBufferOutInd].data());
          for (ind=0; ind<float_buffer.size(); ind++)
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
          temp16=(short *)(WaveOutBuffers[NextBufferOutInd].data());
          for (ind=0; ind<float_buffer.size(); ind++)
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
        &(waveHeaderOut[NextBufferOutInd]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);

      if (IsPlayingNow == false)
      {
        if (NextBufferOutInd == DSP::NoOfAudioOutputBuffers - 2) //all but one spare buffer are filled up
        { // send all data from buffers to soundcard to start playback
          for (ind=0; ind < DSP::NoOfAudioOutputBuffers-1; ind++) //one spare buffer
          { // send all data from buffers to soundcard to start playback
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
          &(waveHeaderOut[NextBufferOutInd]), sizeof(WAVEHDR));
        DSP::f::AudioCheckError(result);
      }
      NextBufferOutInd++;
      NextBufferOutInd %= DSP::NoOfAudioOutputBuffers;

      break;
    }
    else
    {
  //    Sleep(10);
  #ifdef AUDIO_DEBUG_MESSAGES_ON
      DSP::log << "DSP::WMM_object_t::append_playback_buffer" << DSP::e::LogMode::second << "Waiting for free output buffer" << std::endl;
  #endif
      DSP::f::Sleep(0);
    }
  }

  return long(float_buffer.size());
}


//! \bug allow user to select number of internal buffers
void CALLBACK DSP::WMM_object_t::waveInProc(HWAVEIN hwi, UINT uMsg,
  uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2)
{
  UNUSED_ARGUMENT(hwi);
  UNUSED_ARGUMENT(dwParam1);
  UNUSED_ARGUMENT(dwParam2);

  MMRESULT result;
//  DSP::u::AudioInput *Current;
  DSP::WMM_object_t *Current;
#ifdef __DEBUG__
  #ifdef AUDIO_DEBUG_MESSAGES_ON
    std::stringstream tekst;
  #endif
#endif

//  Current = CallbackObjects[dwInstance]->AudioInput_object;
  Current = (DSP::WMM_object_t *)(DSP::SOUND_object_t::get_CallbackSoundObject(dwInstance));
  assert(Current);

  switch (uMsg)
  {
#ifdef __DEBUG__
#ifdef AUDIO_DEBUG_MESSAGES_ON
    case WIM_OPEN:
      DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second
        << "WIM_OPEN(" << (int)dwInstance << ")" << std::endl;
      break;
    case WIM_CLOSE:
      DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second
        << "WIM_CLOSE(" << (int)dwInstance << ")" << std::endl;
      break;
#endif
#endif

    case WIM_DATA:
#ifdef __DEBUG__
#ifdef AUDIO_DEBUG_MESSAGES_ON
      DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second
        << "WIM_DATA(" << (int)dwInstance << ")" << std::endl;
#endif
#endif
      if (Current->StopRecording)
        return;
      else
      {
        if (Current->waveHeaderIn[Current->NextBufferInInd].dwFlags & WHDR_DONE)
        {
          result=waveInUnprepareHeader(Current->hWaveIn,
            &(Current->waveHeaderIn[Current->NextBufferInInd]), sizeof(WAVEHDR));
          DSP::f::AudioCheckError(result);
        
          if (Current->input_callback_call(Current->InSampleType, Current->WaveInBuffers[Current->NextBufferInInd]) == false)
          //if (Current->EmptyBufferIndex == Current->CurrentBufferIndex)
          {
            // ignore data
#ifdef __DEBUG__
#ifdef AUDIO_DEBUG_MESSAGES_ON
            DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second << "All buffers had been used - skipping input audio frame" << std::endl;
#endif
#endif
          }

          //add put back into recording queue
          result=waveInPrepareHeader(Current->hWaveIn,
            &(Current->waveHeaderIn[Current->NextBufferInInd]), sizeof(WAVEHDR));
          DSP::f::AudioCheckError(result);
          result=waveInAddBuffer(Current->hWaveIn,
            &(Current->waveHeaderIn[Current->NextBufferInInd]), sizeof(WAVEHDR));
          DSP::f::AudioCheckError(result);

          Current->NextBufferInInd++; Current->NextBufferInInd %= 2; //just two buffers
        }
        else
        {
#ifdef __DEBUG__
#ifdef AUDIO_DEBUG_MESSAGES_ON
            DSP::log << "DSP::u::AudioInput::waveInProc" << DSP::e::LogMode::second << "Wrong audio frame ready or other unexpected error" << std::endl;
#endif
#endif
        }
      }
      break; // WIM_DATA
  }
}

