/*! \file WMM_support.cpp
 * WMM support code file
 *
 * \author Marek Blok
 */

#include <WMM_support.h>

#include <DSP_lib.h> // for logging 

DSP::WMM_object_t::WMM_object_t()
{
  is_device_openned = false;
  WaveOutDevNo = UINT_MAX;

  NextBufferInd=0;
  IsPlayingNow = false;
};
DSP::WMM_object_t::~WMM_object_t()
{
  if (is_device_openned) {
    close_PCM_device();
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
  string tekst;

  Current = AudioObjects[dwInstance];

  switch (uMsg)
  {
    case WOM_OPEN:
      DSP::log << "DSP::u::AudioOutput::waveOutProc" << DSP::e::LogMode::second
        << "WOM_OPEN(" << (int)dwInstance << ")" << endl;
      break;
    case WOM_CLOSE:
      DSP::log << "DSP::u::AudioOutput::waveOutProc" << DSP::e::LogMode::second
        << "WOM_CLOSE(" << (int)dwInstance << ")" << endl;
      break;
    case WOM_DONE:
      DSP::log << "DSP::u::AudioOutput::waveOutProc" << DSP::e::LogMode::second
        << "WOM_DONE(" << (int)dwInstance << ")" << endl;

      if (Current->StopPlaying)
      {
        DSP::log << "DSP::u::AudioOutput::waveOutProc" << DSP::e::LogMode::second << "StopPlaying is set" << endl;
        return;
      }
      else
      {
        AllDone=true;
        for (ind=0; ind < DSP_NoOfAudioOutputBuffers; ind++)
          AllDone &= (Current->waveHeaderOut[ind].dwFlags & WHDR_DONE);
        if (AllDone)
          DSP::log << "DSP::u::AudioOutput::waveOutProc" << DSP::e::LogMode::second << "All buffers had been used - nothing to play" << endl;
      }
      break;
  }
#endif
#endif
}


void DSP::WMM_object_t::log_driver_data() {
  /*
  int val;

  DSP::log << "WMM library version: " << SND_LIB_VERSION_STR << endl;

  DSP::log << endl;
  DSP::log << "PCM stream types:" << endl;
  for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
    DSP::log << "  " << snd_pcm_stream_name((snd_pcm_stream_t)val) << endl;

  DSP::log << endl;
  DSP::log << "PCM access types:" << endl;
  for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
    DSP::log << "  " << snd_pcm_access_name((snd_pcm_access_t)val) << endl;

  DSP::log << endl;
  DSP::log << "PCM formats:" << endl;
  for (val = 0; val <= SND_PCM_FORMAT_LAST; val++)
    if (snd_pcm_format_name((snd_pcm_format_t)val) != NULL)
      DSP::log << "  " << snd_pcm_format_name((snd_pcm_format_t)val) <<
        "(" << snd_pcm_format_description((snd_pcm_format_t)val) << ")" << endl;

  DSP::log << endl;
  DSP::log << "PCM subformats:" << endl;
  for (val = 0; val <= SND_PCM_SUBFORMAT_LAST; val++)
    DSP::log << "  " << snd_pcm_subformat_name((snd_pcm_subformat_t)val) <<
      "(" << snd_pcm_subformat_description((snd_pcm_subformat_t)val) << ")" << endl;

  DSP::log << endl;
  DSP::log << "PCM states:" << endl;
  for (val = 0; val <= SND_PCM_STATE_LAST; val++)
    DSP::log << "  " << snd_pcm_state_name((snd_pcm_state_t)val) << endl;
  */
}

//    Callback = (DWORD_PTR)(&DSP::u::AudioOutput::waveOutProc);

bool DSP::WMM_object_t::select_device_by_number(const unsigned int &device_number) {
  WaveOutDevNo = device_number;

  return WaveOutDevNo;
}

bool DSP::WMM_object_t::is_device_playing(void) {
  return IsPlayingNow;
}

long DSP::WMM_object_t::open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size) {
  if (is_device_openned)
  {
    DSP::log << "DSP::WMM_object_t::open_PCM_device_4_output" << DSP::e::LogMode::second << "Device has been already opened: closing device before reopening" << endl;
    close_PCM_device();
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
  }
  else
  { //error while creating audio output
    waveHeaderOut.clear();
    WaveOutBuffers.clear();
    WaveOutBufferLen = 0;
  }

  return sampling_rate;
}

long DSP::WMM_object_t::open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size) {
  return -1;
}


bool DSP::WMM_object_t::close_PCM_device(void) {
  result = waveOutReset(hWaveOut);
  DSP::f::AudioCheckError(result);
  
  for (unsigned int ind=0; ind< DSP::NoOfAudioOutputBuffers; ind++)
  {
    result=waveOutUnprepareHeader(hWaveOut,
      &(waveHeaderOut[ind]), sizeof(WAVEHDR));
    DSP::f::AudioCheckError(result);
  }

  #ifdef AUDIO_DEBUG_MESSAGES_ON
    DSP::log << "DSP::u::AudioOutput" << DSP::e::LogMode::second << "Closing DSP::u::AudioOutput" << endl;
  #endif
  result=waveOutClose(hWaveOut);
  while (result==WAVERR_STILLPLAYING)
  {
    DSP::f::Sleep(100);
    #ifdef AUDIO_DEBUG_MESSAGES_ON
      DSP::log << "DSP::u::AudioOutput" << DSP::e::LogMode::second << "Closing DSP::u::AudioOutput" << endl;
    #endif
    result=waveOutClose(hWaveOut);
  }  
  DSP::f::AudioCheckError(result);

  // 2) Free buffers
  WaveOutBuffers.clear();
  #ifdef WINMMAPI
    waveHeaderOut.clear();
  #endif

  #ifdef AUDIO_DEBUG_MESSAGES_ON
    DSP::log << "WMM PCM sound closed" << endl;
  #endif

  return true; // device has been closed
}


long DSP::WMM_object_t::append_playback_buffer(DSP::Float_vector &float_buffer) {
  uint8_t *temp8;
  short *temp16;
  short Znak;
  uint32_t ind;

  // ************************************************** //
  // Send float_buffer to the audio device

  #ifdef AUDIO_DEBUG_MESSAGES_ON
    DSP::log << "DSP::WMM_object_t::append_playback_buffer" << DSP::e::LogMode::second << "Flushing output buffer" << endl;
  #endif

  while (1)
  {
    if (waveHeaderOut[NextBufferInd].dwFlags & WHDR_DONE)
    {
      result=waveOutUnprepareHeader(hWaveOut,
        &(waveHeaderOut[NextBufferInd]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);

      DSP::Float_ptr Sample=float_buffer.data();
      // ************************************************** //
      // Converts samples format to the one suitable for the audio device
      switch (OutSampleType)
      {
        case DSP::e::SampleType::ST_uchar:
          temp8=(uint8_t *)(WaveOutBuffers[NextBufferInd].data());
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
          temp16=(short *)(WaveOutBuffers[NextBufferInd].data());
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
        &(waveHeaderOut[NextBufferInd]), sizeof(WAVEHDR));
      DSP::f::AudioCheckError(result);

      if (IsPlayingNow == false)
      {
        if (NextBufferInd == 1)
        {
          for (ind=0; ind < DSP::NoOfAudioOutputBuffers-1; ind++) //one spare buffer
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
      NextBufferInd %= DSP::NoOfAudioOutputBuffers;

      break;
    }
    else
    {
  //    Sleep(10);
  #ifdef AUDIO_DEBUG_MESSAGES_ON
      DSP::log << "DSP::WMM_object_t::append_playback_buffer" << DSP::e::LogMode::second << "Waiting for free output buffer" << endl;
  #endif
      DSP::f::Sleep(0);
    }
  }

  return long(float_buffer.size());
}