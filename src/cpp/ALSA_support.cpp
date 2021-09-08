/*! \file ALSA_support.cpp
 * ALSA support code file
 *
 * \author Marek Blok
 */

#include <ALSA_support.h>
#include <vector>
#include <cmath>
#include <DSP_lib.h> // for logging 

// ============================================================= //
// Based on: https://www.linuxjournal.com/article/6735
// ============================================================= //

DSP::ALSA_object_t::ALSA_object_t()
{
  alsa_handle = NULL;
  hw_params = NULL;

  std::string endianess;
  endianess = system("lscpu | grep \"Byte Order\" | egrep -o 'Little Endian|Big Endian'");

  if (endianess == "Big Endian")
    IsLittleEndian = false;
  else
    IsLittleEndian = true;

  IsHigherQualityMode = false;

  blocking_mode = false;

  IsPlayingNow = false;
  StopPlayback = false;

  StopRecording = false;
  IsRecordingNow = false;

  OutDevNo = -1;
  InDevNo = -1;

  NextBufferOutInd = 0;

  /* 44100 bits/second sampling rate (CD quality) */
  sampling_rate_alsa = 44100;
  no_of_channels_alsa = 2;
  no_of_bytes_in_channel = 2;

  frames = 8000;
  size_b = 32;

}

DSP::ALSA_object_t::~ALSA_object_t()
{
  if (hw_params != NULL)
  {
    snd_pcm_hw_params_free(hw_params);
    hw_params = NULL;
  }

  buffers_8bit.clear();
  buffers_16bit.clear();
  buffers_32bit.clear();
  buffers_64bit.clear();
  pcm_buffer.clear();

}

unsigned int DSP::ALSA_object_t::select_input_device_by_number(const unsigned int &device_number)
{
  InDevNo = (int) device_number;

  return InDevNo;
}

unsigned int DSP::ALSA_object_t::select_output_device_by_number(const unsigned int &device_number)
{
  OutDevNo = (int) device_number;

  return OutDevNo;
}

bool DSP::ALSA_object_t::is_output_callback_supported(void) 
{
  return false;
}

bool DSP::ALSA_object_t::is_input_callback_supported(void) 
{
  return false;
}

void DSP::ALSA_object_t::log_driver_data() 
{
  int val;

  DSP::log << "ALSA library version: " << SND_LIB_VERSION_STR << endl;

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
  DSP::log << "PCM states: " << endl;
  for (val = 0; val <= SND_PCM_STATE_LAST; val++)
    DSP::log << "  " << snd_pcm_state_name((snd_pcm_state_t)val) << endl;

}

int DSP::ALSA_object_t::open_alsa_device(snd_pcm_stream_t stream_type)
{ 
  //! Errors controllers
  int rc;
  int errc;
  
  snd_pcm_hw_params_t *params;

  //! For logging
  unsigned int val, val2;
  int dir;

  if (alsa_handle != NULL)
    close_alsa_device();

  // ==================================================== //
  DSP::log << "Opening ALSA device" << endl;

    //! \TODO Test mode:	Open mode (see SND_PCM_NONBLOCK, SND_PCM_ASYNC)
    
    if (stream_type == SND_PCM_STREAM_PLAYBACK)    
      DSP::log << "Opening PCM device for playback." << endl;
    
    else
      DSP::log << "Opening PCM device for recording (capture)." << endl;

    // What "name" actually means?
    rc = snd_pcm_open(&alsa_handle, "default", stream_type, SND_PCM_NONBLOCK);

    if (rc < 0)
    {
      DSP::log << "Unable to open pcm device: " << snd_strerror(rc) << endl;
      return -1;
    }

    /*! Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /*! Fill it in with default values. */
    snd_pcm_hw_params_any(alsa_handle, params);

    /*! Set the desired hardware parameters. */

    /*! Interleaved mode */
    snd_pcm_hw_params_set_access(alsa_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    DSP::log << "Setting the SND PCM FORMAT." << endl;
    DSP::log << "Something less than 0 means an error occurance." << endl;
    
    errc = DSP::ALSA_object_t::set_snd_pcm_format(params);

    snd_pcm_hw_params_set_channels(alsa_handle, params, no_of_channels_alsa);

    snd_pcm_hw_params_set_rate_near(alsa_handle, params, &sampling_rate_alsa, &dir);

    if (frames <= 0)
      frames = 8000;

    rc = snd_pcm_hw_params_set_buffer_size(alsa_handle, params, DSP::NoOfAudioOutputBuffers*frames);
    
    snd_pcm_uframes_t tmp_frames = frames;

    DSP::log << "Buffer size set with error code: " << rc << endl;

    /*! Set period size to desired number of frames. */
    snd_pcm_hw_params_set_period_size_near(alsa_handle, params, &frames, &dir);

    if (frames != tmp_frames)
    {
      DSP::log << "Current frames value should be equal: " << tmp_frames << endl;
      DSP::log << "Frames is not equal to tmp_frames! Frames: " << frames << endl;    
    }
    else
      DSP::log << "Frames has been set correctly." << endl;

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(alsa_handle, params);

    if (rc < 0)
    {
      DSP::log << "Unable to set hw parameters: " << snd_strerror(rc) << endl;
      
      close_alsa_device();
      
      return -2;
    }
    /*! Make a copy of hardware parameters. */
    snd_pcm_hw_params_malloc(&hw_params);
    snd_pcm_hw_params_copy(hw_params, params);
  

  /*! Display information about the PCM interface */
  
    DSP::log << "PCM handle name = '" << snd_pcm_name(alsa_handle) << "'" << endl;

    DSP::log << "PCM state = " << snd_pcm_state_name(snd_pcm_state(alsa_handle)) << endl;

    snd_pcm_hw_params_get_access(hw_params, (snd_pcm_access_t *) &val);
    DSP::log << "access type = " << snd_pcm_access_name((snd_pcm_access_t)val) << endl;

    snd_pcm_format_t format;
    snd_pcm_hw_params_get_format(hw_params, &format);
    DSP::log << "format = '" << snd_pcm_format_name(format) << "' "
      << "(" << snd_pcm_format_description(format) << ")" << endl;

    snd_pcm_hw_params_get_subformat(hw_params, (snd_pcm_subformat_t *)&val);
    DSP::log << "subformat = '" << snd_pcm_subformat_name((snd_pcm_subformat_t)val) << "' "
      << "(" << snd_pcm_subformat_description((snd_pcm_subformat_t)val) << ")" << endl;

    snd_pcm_hw_params_get_channels(hw_params, &val);
    DSP::log << "channels = " << val << endl;

    snd_pcm_hw_params_get_rate(hw_params, &val, &dir);
    DSP::log << "rate = " << val << " bps" << endl;

    snd_pcm_hw_params_get_period_time(hw_params, &val, &dir);
    DSP::log << "period time = " << val << " us" << endl;

    snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);
    DSP::log << "period size = " << (int) frames << " frames" << endl;

    snd_pcm_hw_params_get_buffer_time(hw_params, &val, &dir);
    DSP::log << "buffer time = " << val << " us" << endl;

    snd_pcm_hw_params_get_buffer_size(hw_params, (snd_pcm_uframes_t *) &val);
    DSP::log << "buffer size = " << val << " frames" << endl;

    snd_pcm_hw_params_get_periods(hw_params, &val, &dir);
    DSP::log << "periods per buffer = " << val << " frames" << endl;

    snd_pcm_hw_params_get_rate_numden(hw_params, &val, &val2);
    DSP::log << "exact rate = " << val << "/" << val2 << " bps" << endl;

    val = snd_pcm_hw_params_get_sbits(hw_params);
    DSP::log << "significant bits = " << val << endl;

    snd_pcm_hw_params_get_tick_time(hw_params, &val, &dir); // deprecated !!!
    DSP::log << "tick time = " << val << " us" << endl;

    val = snd_pcm_hw_params_is_batch(hw_params);
    DSP::log << "is batch = " << val << endl;

    val = snd_pcm_hw_params_is_block_transfer(hw_params);
    DSP::log << "is block transfer = " << val << endl;

    val = snd_pcm_hw_params_is_double(hw_params);
    DSP::log << "is double = " << val << endl;

    val = snd_pcm_hw_params_is_half_duplex(hw_params);
    DSP::log << "is half duplex = " << val << endl;

    val = snd_pcm_hw_params_is_joint_duplex(hw_params);
    DSP::log <<"is joint duplex = " << val << endl;

    val = snd_pcm_hw_params_can_overrange(hw_params);
    DSP::log << "can overrange = " << val << endl;

    val = snd_pcm_hw_params_can_mmap_sample_resolution(hw_params);
    DSP::log << "can mmap = " << val << endl;

    val = snd_pcm_hw_params_can_pause(hw_params);
    DSP::log << "can pause = " << val << endl;

    val = snd_pcm_hw_params_can_resume(hw_params);
    DSP::log << "can resume = " << val << endl;

    val = snd_pcm_hw_params_can_sync_start(hw_params);
    DSP::log << "can sync start = " << val << endl;
  

  //  snd_pcm_hw_params_free(params);
  //  There are two ways of allocating such structures:
  //  1) Use snd_xxx_malloc() and snd_xxx_free() to allocate memory from the
  //     heap, or
  //  2) use snd_xxx_alloca() to allocate memory from the stack.
  
  //  The snd_xxx_alloca() functions behave just like alloca(): their memory
  //  is automatically freed when the function returns.

  //  snd_pcm_hw_params_current() // Retreive current PCM hardware configuration chosen with snd_pcm_hw_params.
  //  snd_pcm_hw_params_current(alsa_handle, hw_params);

  // https://www.alsa-project.org/wiki/FramesPeriods
  // frame - size of sample in byts
  // A period is the number of frames in between each hardware interrupt

  /*! Use a buffer large enough to hold one period */

  // Nalezy przemyśleć
  // snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);
  
   size_b = frames * no_of_channels_alsa * no_of_bytes_in_channel;

   pcm_buffer.resize(DSP::NoOfAudioOutputBuffers);

   switch (no_of_bytes_in_channel)
   {
     case 1:
        buffers_8bit.resize(DSP::NoOfAudioOutputBuffers);

        for(unsigned int ind = 0; ind < DSP::NoOfAudioOutputBuffers; ind++)
        {
           buffers_8bit[ind].resize(size_b / no_of_bytes_in_channel);
           pcm_buffer[ind] = (unsigned char *)(buffers_8bit[ind].data());
        }
        break;

     case 2:
        buffers_16bit.resize(DSP::NoOfAudioOutputBuffers);

        for(unsigned int ind = 0; ind < DSP::NoOfAudioOutputBuffers; ind++)
        {
           buffers_16bit[ind].resize(size_b / no_of_bytes_in_channel);
           pcm_buffer[ind] = (unsigned char *)(buffers_16bit[ind].data());
        }
        break;

     case 3:
     case 4:
        if (IsHigherQualityMode)
        {
          buffers_32bit.resize(DSP::NoOfAudioOutputBuffers);

          for(unsigned int ind = 0; ind < DSP::NoOfAudioOutputBuffers; ind++)
          {
             buffers_32bit[ind].resize(size_b / no_of_bytes_in_channel);
             pcm_buffer[ind] = (unsigned char *)(buffers_32bit[ind].data());
          }
        }
 
        else //! native mode
        {
            buffers_32bit_f.resize(DSP::NoOfAudioOutputBuffers);

            for(unsigned int ind = 0; ind < DSP::NoOfAudioOutputBuffers; ind++)
            {
               buffers_32bit_f[ind].resize(size_b / no_of_bytes_in_channel);
               pcm_buffer[ind] = (unsigned char *)(buffers_32bit_f[ind].data());
            }
        }
        break;

     case 8:
        buffers_64bit.resize(DSP::NoOfAudioOutputBuffers);

        for(unsigned int ind = 0; ind < DSP::NoOfAudioOutputBuffers; ind++)
        {
           buffers_64bit[ind].resize(size_b / no_of_bytes_in_channel);
           pcm_buffer[ind] = (unsigned char *)(buffers_64bit[ind].data());
        }
        break;

     default:
       DSP::log << "Unsupported no of bytes in channel" << endl;
       return -1;
    }

  if (blocking_mode == false)
  {
      rc = snd_pcm_nonblock(alsa_handle, 0);

      if (rc < 0)
      {
          DSP::log << "Unable to set blocking mode" << endl;
          return -1;
      }
  }
  else
  {
      rc = snd_pcm_nonblock(alsa_handle, 1);
      if (rc < 0)
      {
          DSP::log << "Unable to set non blocking mode" << endl;
          return -1;
      }
  }

  //if (stream_type == SND_PCM_STREAM_PLAYBACK)
  // {

    /* Sinus experimental
    // M.B. wariant dla stałej częstotliwości
    for (unsigned int n = 0; n < size_b / no_of_bytes_in_channel / no_of_channels_alsa; n++)
    {
        if (no_of_bytes_in_channel == 1)
        {
            buffer_8bit[no_of_channels_alsa*n] = 128 + 127 * sin(2 * M_PI * Freq / sampling_rate_alsa * n + phase_0);
            if (no_of_channels_alsa == 2)
                buffer_8bit[no_of_channels_alsa * n + 1] = 128 + 127 * sin(2 * M_PI * (Freq_2) / sampling_rate_alsa * n + phase_0_2);

        }
        else if (no_of_bytes_in_channel == 2)
        {
            buffer_16bit[no_of_channels_alsa * n] = INT16_MAX * sin(2 * M_PI * Freq / sampling_rate_alsa * n + phase_0);
            if (no_of_channels_alsa == 2)
                buffer_16bit[no_of_channels_alsa * n + 1 ] = INT16_MAX * sin(2 * M_PI * (Freq_2) / sampling_rate_alsa * n + phase_0_2);

        }
        else if (no_of_bytes_in_channel == 3)
        {
            buffer_32bit[no_of_channels_alsa * n] = INT32_MAX * sin(2 * M_PI * Freq / sampling_rate_alsa * n + phase_0);
            if (no_of_channels_alsa == 2)
                buffer_32bit[no_of_channels_alsa * n + 1 ] = INT32_MAX * sin(2 * M_PI * (Freq_2) / sampling_rate_alsa * n + phase_0_2);
        }
        else if (no_of_bytes_in_channel == 4)
        {
            if (IsHigherQualityMode)
            {
                buffer_32bit[no_of_channels_alsa * n] = INT32_MAX * sin(2 * M_PI * Freq / sampling_rate_alsa * n + phase_0);
                if (no_of_channels_alsa == 2)
                    buffer_32bit[no_of_channels_alsa * n + 1 ] = INT32_MAX * sin(2 * M_PI * (Freq_2) / sampling_rate_alsa * n + phase_0_2);
            }

            else
            {
                buffer_32bit_f[no_of_channels_alsa * n] = sin(2 * M_PI * Freq / sampling_rate_alsa * n + phase_0);
                if (no_of_channels_alsa == 2)
                    buffer_32bit_f[no_of_channels_alsa * n + 1 ] = sin(2 * M_PI * (Freq_2) / sampling_rate_alsa * n + phase_0_2);
            }
        }

        else if (no_of_bytes_in_channel == 8)
        {
            buffer_64bit[no_of_channels_alsa * n] = sin(2 * M_PI * Freq / sampling_rate_alsa * n + phase_0);
            if (no_of_channels_alsa == 2)
                buffer_64bit[no_of_channels_alsa * n + 1 ] = sin(2 * M_PI * (Freq_2) / sampling_rate_alsa * n + phase_0_2);

        }
    }

    phase_0 +=  2 * M_PI * Freq / sampling_rate_alsa * size_b / no_of_bytes_in_channel / no_of_channels_alsa;
    phase_0_2 +=  2 * M_PI * Freq_2 / sampling_rate_alsa * size_b / no_of_bytes_in_channel / no_of_channels_alsa;
    DSP::log << Freq << ", " << Freq_2 << endl;
    */
   
   // do innych metod
    // DSP::log << "Before snd_pcm_writei (" << loops << ")" << endl;
  /*

  if (stream_type == SND_PCM_STREAM_PLAYBACK)
    snd_pcm_sframes_t DSP::ALSA_object_t::pcm_writei(alsa_handle, const void *buffer, frames)

  else
  {
    
    rc = snd_pcm_readi(alsa_handle, pcm_buffer, frames);
    DSP::log << "Read" << endl;

  }
  */

  return 1;

}

int DSP::ALSA_object_t::set_snd_pcm_format(snd_pcm_hw_params_t *params) 
{
  int errc;
  
  if (no_of_bytes_in_channel == 1)
  {
      /*! Signed 8-bit format */
      errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                          SND_PCM_FORMAT_U8);

       DSP::log << "Format set with error code: " << errc << endl;
  }
  else if (no_of_bytes_in_channel == 2)
  {
      if (IsLittleEndian == false)
      {
        /*! Signed 16-bit big-endian format */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_S16_BE);
      }

      else
      {
        /*! Signed 16-bit little-endian format */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_S16_LE);
      }

      DSP::log << "Format set with error code: " << errc << endl;
  }
  else if (no_of_bytes_in_channel == 3) // 32-bits buffer can be used
  {
      if (IsLittleEndian == false)
      {
        /*! Signed 24-bit big-endian low three bytes in 32-bit word format */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_S32_BE);
      }

      else
      {
        /*! Signed 24-bit little-endian low three bytes in 32-bit word format */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_S32_LE);
      }

      IsHigherQualityMode = true;

      DSP::log << "Format set with error code: " << errc << std::endl;
  }
  else if (no_of_bytes_in_channel == 4)
  {
      if (IsLittleEndian == false)
      {
        /*! Float Little Endian, Range -1.0 to 1.0 */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_FLOAT_BE);
        IsHigherQualityMode = false; // native

        if(errc < 0)
        {
            /*! Signed 32-bit little-endian format */
            errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                                SND_PCM_FORMAT_S32_BE);
            IsHigherQualityMode = true;
        }
      }

      else
      {
        /*! Float Little Endian, Range -1.0 to 1.0 */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_FLOAT_LE);
        IsHigherQualityMode = false; // native

        if(errc < 0)
        {
            /*! Signed 32-bit little-endian format */
            errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                                SND_PCM_FORMAT_S32_LE);
            IsHigherQualityMode = true;
        }
      }

      DSP::log << "Format set with error code: " << errc << endl;
  }
  else if (no_of_bytes_in_channel == 8)
  {
      if (IsLittleEndian == false)
      {
        /*! Float Big Endian, Range -1.0 to 1.0 */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_FLOAT64_BE);
      }

      else
      {
        /*! Float Little Endian, Range -1.0 to 1.0 */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_FLOAT64_LE);
      }

      DSP::log << "Format set with error code: " << errc << endl;
  }
  return errc;
}

long DSP::ALSA_object_t::open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size) {
  
  int rc;
  no_of_bytes_in_channel = (unsigned int) no_of_bits / 8;
  sampling_rate_alsa = (unsigned int) sampling_rate;
  no_of_channels_alsa = (unsigned int) no_of_channels;
  frames = (snd_pcm_uframes_t) audio_outbuffer_size;

  rc = open_alsa_device(SND_PCM_STREAM_PLAYBACK);

  if(rc > 0)
  { 
    return 1;
  }  
  else 
  {
    return -1;
  }
}

long DSP::ALSA_object_t::open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_inbuffer_size) {
  
  int rc;
  no_of_bytes_in_channel = (unsigned int) no_of_bits / 8;
  sampling_rate_alsa = (unsigned int) sampling_rate;
  no_of_channels_alsa = (unsigned int) no_of_channels;
  frames = (snd_pcm_uframes_t) audio_inbuffer_size;

  rc = open_alsa_device(SND_PCM_STREAM_CAPTURE);
  
  if(rc > 0)
  { 
    return 1;
  }  
  else 
  {
    return -1;
  }
}

long DSP::ALSA_object_t::append_playback_buffer(DSP::Float_vector &float_buffer)
{
 // assert(!"DSP::ALSA_object_t::append_playback_buffer not implemented yet");

  snd_pcm_sframes_t rc = -1;

  long buffer_size;

  while (1)
  {
    if (rc < 0)
    {
      for (unsigned int m = 0; m < float_buffer.size(); m++)
      {
        if (float_buffer[m] < -1)
          float_buffer[m] = -1;
        
        else if (float_buffer[m] > 1) 
          float_buffer[m] = 1;
      }
      // Converts samples format to the one suitable for the audio device
      for (unsigned int n = 0; n < size_b / no_of_bytes_in_channel / no_of_channels_alsa; n++)
      {
            if (no_of_bytes_in_channel == 1)
            {
                buffers_8bit[NextBufferOutInd][no_of_channels_alsa * n] = 128 + 127 * (uint8_t) float_buffer[no_of_channels_alsa * n];
                  if (no_of_channels_alsa == 2)
                    buffers_8bit[NextBufferOutInd][no_of_channels_alsa * n + 1] = 128 + 127 * (uint8_t) float_buffer[no_of_channels_alsa * n + 1];

            }
            else if (no_of_bytes_in_channel == 2)
            {
                buffers_16bit[NextBufferOutInd][no_of_channels_alsa * n] = INT16_MAX * (int16_t) float_buffer[no_of_channels_alsa * n];
                if (no_of_channels_alsa == 2)
                  buffers_16bit[NextBufferOutInd][no_of_channels_alsa * n + 1 ] = INT16_MAX * (int16_t) float_buffer[no_of_channels_alsa * n + 1];

            }
            else if (no_of_bytes_in_channel == 3)
            {
                buffers_32bit[NextBufferOutInd][no_of_channels_alsa * n] = INT32_MAX * (int32_t) float_buffer[no_of_channels_alsa * n];
                if (no_of_channels_alsa == 2)
                  buffers_32bit[NextBufferOutInd][no_of_channels_alsa * n + 1 ] = INT32_MAX * (int32_t) float_buffer[no_of_channels_alsa * n + 1];  

            }
            else if (no_of_bytes_in_channel == 4)
            {
                if (IsHigherQualityMode)
                {
                    buffers_32bit[NextBufferOutInd][no_of_channels_alsa * n] = INT32_MAX * (int32_t) float_buffer[no_of_channels_alsa * n];
                    if (no_of_channels_alsa == 2)
                      buffers_32bit[NextBufferOutInd][no_of_channels_alsa * n + 1 ] = INT32_MAX * (int32_t) float_buffer[no_of_channels_alsa * n + 1];

                }
                else
                {
                    buffers_32bit_f[NextBufferOutInd][no_of_channels_alsa * n] = float_buffer[no_of_channels_alsa * n];
                    if (no_of_channels_alsa == 2)
                      buffers_32bit_f[NextBufferOutInd][no_of_channels_alsa * n + 1 ] = float_buffer[no_of_channels_alsa * n + 1];

                }
            }

            else if (no_of_bytes_in_channel == 8)
            {
                buffers_64bit[NextBufferOutInd][no_of_channels_alsa * n] = float_buffer[no_of_channels_alsa * n];
                if (no_of_channels_alsa == 2)
                  buffers_64bit[NextBufferOutInd][no_of_channels_alsa * n + 1 ] = float_buffer[no_of_channels_alsa * n + 1];

            }      
      // converting samples ends
      }

      // set the pointer to the right buffer and store the buffer size
      switch (no_of_bytes_in_channel)
      {
        case 1:
          pcm_buffer[NextBufferOutInd] = (uint8_t *) buffers_8bit[NextBufferOutInd].data();
          buffer_size = (long) buffers_8bit.size();
          break;

        case 2:
          pcm_buffer[NextBufferOutInd] = (uint8_t *) buffers_16bit[NextBufferOutInd].data();
          buffer_size = (long) buffers_16bit.size();
          break;
    
          case 3:
          pcm_buffer[NextBufferOutInd] = (uint8_t *) buffers_32bit[NextBufferOutInd].data();
          buffer_size = (long) buffers_32bit.size();
          break;

        case 4:
          if (IsHigherQualityMode)
          {
            pcm_buffer[NextBufferOutInd] = (uint8_t *) buffers_32bit[NextBufferOutInd].data();
            buffer_size = (long) buffers_32bit.size();
          }
          else
          {
            pcm_buffer[NextBufferOutInd] = (uint8_t *) buffers_32bit_f[NextBufferOutInd].data();
            buffer_size = (long) buffers_32bit_f.size();
          }
          break;

        case 8:
          pcm_buffer[NextBufferOutInd] = (uint8_t *) buffers_64bit[NextBufferOutInd].data();
          buffer_size = (long) buffers_64bit.size();
          break;

        default:
          buffer_size = (long) size_b;
          break;
      }

      if (IsPlayingNow == false)
      {
        if (NextBufferOutInd == DSP::NoOfAudioOutputBuffers - 2) //all but one spare buffer are filled up
        { 
          // send all data from buffers to soundcard to start playback
          for (unsigned int ind = 0; ind < DSP::NoOfAudioOutputBuffers - 1; ind++) //one spare buffer
          {
            rc = DSP::ALSA_object_t::pcm_writei(pcm_buffer[ind]);
          }
          IsPlayingNow = true;
        }

      }
      else
      {
        rc = DSP::ALSA_object_t::pcm_writei(pcm_buffer[NextBufferOutInd]);
      }
      NextBufferOutInd++;
      NextBufferOutInd %= DSP::NoOfAudioOutputBuffers;

      break;
    }

    IsPlayingNow = false;
 }
  return buffer_size;
}

bool DSP::ALSA_object_t::close_PCM_device_input(void) {
  //assert(!"DSP::ALSA_object_t::close_PCM_device_input not implemented yet");
  close_alsa_device(true);
  return true;
}

bool DSP::ALSA_object_t::close_PCM_device_output(void) {
  //assert(!"DSP::ALSA_object_t::close_PCM_device_output not implemented yet");
  close_alsa_device(true);
  return true;
}

bool DSP::ALSA_object_t::is_device_playing(void) {
  assert(!"DSP::ALSA_object_t::is_device_playing not implemented yet");
  return false;
}

bool DSP::ALSA_object_t::is_device_recording(void) {
  assert(!"DSP::ALSA_object_t::is_device_recording not implemented yet");
  return false;
}

bool DSP::ALSA_object_t::stop_playback(void) {
  assert(!"DSP::ALSA_object_t::stop_playback not implemented yet");
  return true;
}

bool DSP::ALSA_object_t::stop_recording(void) {
  assert(!"DSP::ALSA_object_t::stop_recording not implemented yet");
}

bool DSP::ALSA_object_t::start_recording(void) {
  assert(!"DSP::ALSA_object_t::stop_recording not implemented yet");
  
}

bool DSP::ALSA_object_t::get_wave_in_raw_buffer(DSP::e::SampleType &InSampleType, std::vector<char> &wave_in_raw_buffer) {
  assert(!"DSP::ALSA_object_t::get_wave_in_raw_buffer not implemented yet");
}

snd_pcm_sframes_t DSP::ALSA_object_t::pcm_writei(const void *buffer) {

  int rc = -EAGAIN;
  
  while (rc == -EAGAIN)
  {
    rc = snd_pcm_writei(alsa_handle, buffer, frames);
    DSP::log << "EAGAIN occured. Waiting for free buffer." << endl;
    DSP::f::Sleep(10);
  }
    DSP::log << "Wrote" << endl;
    
    if (rc == -EPIPE)
    {
        // EPIPE means underrun
        DSP::log << "Underrun occurred" << endl;
        snd_pcm_prepare(alsa_handle);

    }
    else if (rc < 0)
    {
      DSP::log << "Error from writei: " << snd_strerror(rc) << endl;
      snd_pcm_sframes_t err = (snd_pcm_sframes_t) rc;
      return rc;
    }
    else if (rc != (int)frames)
    {
      DSP::log << "short write, write " << rc << " frames" << endl;
    }
  
    DSP::log << "The end of the playback" << endl;

  return 1;
}

void DSP::ALSA_object_t::close_alsa_device(bool do_drain, bool use_log) {
  if (alsa_handle != NULL) 
  {
    if (do_drain == true)
    {
      snd_pcm_drain(alsa_handle);
    }
    
    DSP::log << "Closing PCM device." << endl;
    snd_pcm_close(alsa_handle);
    alsa_handle = NULL;

    if (hw_params != NULL)
    {
      snd_pcm_hw_params_free(hw_params);
      hw_params = NULL;
    }
  }

  if (use_log == true)
  {
    DSP::log << "ALSA PCM sound closed" << endl;
  }
}

