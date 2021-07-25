/*! \file ALSA_support.cpp
 * ALSA support code file
 *
 * \author Marek Blok
 */

#include <ALSA_support.h>
#include <iostream> // w przyszłości pozamieniam na DSP::log
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
};
DSP::ALSA_object_t::~ALSA_object_t()
{
  if (hw_params != NULL){
    snd_pcm_hw_params_free(hw_params);
    hw_params = NULL;
  }
}

unsigned int DSP::ALSA_object_t::select_input_device_by_number(const unsigned int &device_number) {
  assert(!"ALSA_object_t::select_input_device_by_number not yet implemented");
  
}

unsigned int DSP::ALSA_object_t::select_output_device_by_number(const unsigned int &device_number) {
  assert(!"ALSA_object_t::select_output_device_by_number not yet implemented");
  
}

bool DSP::ALSA_object_t::is_output_callback_supported(void) {
  assert(!"ALSA_object_t::is_output_callback_supported not yet implemented");
  return false;
}

bool DSP::ALSA_object_t::is_input_callback_supported(void) {
  assert(!"ALSA_object_t::is_input_callback_supported not yet implemented");
  return false;
}

void DSP::ALSA_object_t::log_driver_data() {
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
  DSP::log << "PCM states:" << endl;
  for (val = 0; val <= SND_PCM_STATE_LAST; val++)
    DSP::log << "  " << snd_pcm_state_name((snd_pcm_state_t)val) << endl;
}


/*

This example opens the default PCM device, sets
some parameters, and then displays the value
of most of the hardware parameters. It does not
perform any sound playback or recording.

returns actually selected sampling_rate

*/
int DSP::ALSA_object_t::open_alsa_device(snd_pcm_stream_t stream_type, unsigned int no_of_channels, unsigned int no_of_bytes_in_channel, unsigned int &sampling_rate) 
{
  long loops;
  int rc;
  int errc;
  int size_b;

  int mode;

  snd_pcm_t *handle;
  unsigned int val, val2;
  int dir;
  snd_pcm_uframes_t frames;

  std::vector<unsigned char> buffer_8bit; // M.B. lepiej korzystać z kontenerów STD, są wygodniejsze i oznaczają mniej problemów z wyciekami pamięci
  std::vector<int16_t> buffer_16bit; // M.B. dla odtwarzania 16-bitowego (int to dłuższe słowa)
  std::vector<int32_t> buffer_32bit; // D.K. dla odtwarzania 24 i 32-bitowego
  std::vector<float> buffer_32bit_f; // natywny dla 32 bitow
  std::vector<double> buffer_64bit; // D.K. dla odtwarzania 64-bitowego

  std::string endianess;
  endianess = system("lscpu | grep \"Byte Order\" | egrep -o 'Little Endian|Big Endian'");

  if (alsa_handle != NULL)
    close_alsa_device();

  // ==================================================== //
  DSP::log << "Opening ALSA device" << endl;

  {
    //! \TODO Test mode:	Open mode (see SND_PCM_NONBLOCK, SND_PCM_ASYNC)
    
    DSP::log << "Opening PCM device for playback." << endl;
    rc = snd_pcm_open(&handle, "default",
                      stream_type, SND_PCM_NONBLOCK);

    if (rc < 0) 
    {
      DSP::log << "Unable to open pcm device: " << snd_strerror(rc) << endl;
      // exit(1);
      return -1;
    }
    alsa_handle = handle;
  }

  {
    snd_pcm_hw_params_t *params;

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(alsa_handle, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(alsa_handle, params,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);

    DSP::log << "Setting the SND PCM FORMAT." << endl;
    DSP::log << "Something less than 0 means an error occurance." << endl;


    // W tym miejscu wywołać metodę int set_snd_pcm_format - te ify ponizej się w niej znajdą   
    set_snd_pcm_format(errc, no_of_bytes_in_channel, endianess, params, alsa_handle, mode); 

    snd_pcm_hw_params_set_channels(alsa_handle, params, no_of_channels);

    snd_pcm_hw_params_set_rate_near(alsa_handle, params, &sampling_rate, &dir);

    rc = snd_pcm_hw_params_set_buffer_size(alsa_handle,
                                           params, 2*frames);
    DSP::log << "Set buffer size: " << rc << endl;

    snd_pcm_hw_params_set_period_size_near(alsa_handle,
                                           params, &frames, &dir);

    
    /* Write the parameters to the driver */

    rc = snd_pcm_hw_params(alsa_handle, params);

    if (rc < 0) 
    {
      DSP::log << "Unable to set hw parameters: " << snd_strerror(rc) << endl;
      
      close_alsa_device();
      
      return -2;
    }
    // make a copy of hardware parameters
    snd_pcm_hw_params_malloc(&hw_params);
    snd_pcm_hw_params_copy(hw_params, params);
  }


  /* Display information about the PCM interface */
  
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


//snd_pcm_hw_params_free(params);
//  There are two ways of allocating such structures:
//1) Use snd_xxx_malloc() and snd_xxx_free() to allocate memory from the
//heap, or
//2) use snd_xxx_alloca() to allocate memory from the stack.
//
//The snd_xxx_alloca() functions behave just like alloca(): their memory
//is automatically freed when the function returns.

 // return 1;
// }

// void DSP::ALSA_object_t::get_period_size(snd_pcm_uframes_t &frames, unsigned int &period_time) {
  // int dir;

  // snd_pcm_hw_params_current() // Retreive current PCM hardware configuration chosen with snd_pcm_hw_params.
  //snd_pcm_hw_params_current(alsa_handle, hw_params);

  // https://www.alsa-project.org/wiki/FramesPeriods
  // frame - size of sample in byts
  // A period is the number of frames in between each hardware interrupt

  /* Use a buffer large enough to hold one period */

  snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);

  size_b = frames * no_of_channels * no_of_bytes_in_channel; /* 2 bytes/sample, 2 channels */ // M.B. warto być gotowym na wardziej uniwersalne warianty
  switch (no_of_bytes_in_channel)
  {
    case 1:
      buffer_8bit.resize(size_b / no_of_bytes_in_channel); // M.B. wygodniejsze niż malloc
      pcm_buffer = (unsigned char *)(buffer_8bit.data());
      break;
    case 2:
      buffer_16bit.resize(size_b / no_of_bytes_in_channel); // M.B. wygodniejsze niż malloc
      pcm_buffer = (unsigned char *)(buffer_16bit.data());
      break;
    case 3:
    case 4:

      if (mode == 1)
      {
        buffer_32bit.resize(size_b / no_of_bytes_in_channel); // M.B. wygodniejsze niż malloc
        pcm_buffer = (unsigned char *)(buffer_32bit.data());
      }

      else // D.K. native mode
      {
        buffer_32bit_f.resize(size_b / no_of_bytes_in_channel);
        pcm_buffer = (unsigned char *)(buffer_32bit_f.data());
      }
      break;
    case 8:
      buffer_64bit.resize(size_b / no_of_bytes_in_channel); // M.B. wygodniejsze niż malloc
      pcm_buffer = (unsigned char *)(buffer_64bit.data());
        break;
    default:
      std::cerr << "Unsupported no_of_bytes_in_channel" << std::endl;
      exit(1);
  }

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(hw_params, &period_time_ms, &dir);

 // TODO:
  // M.B. wybór trybu synchroniczanego lub asynchronicznego
  if (blocking_mode == false)
  {
      rc = snd_pcm_nonblock(handle, 0);

      if (rc < 0)
      {

          std::cerr << "Unable to set blocking mode" << std::endl;
          exit(1);
      }
  }
  else
  {
      rc = snd_pcm_nonblock(handle, 1);
      if (rc < 0)
      {
          std::cerr << "Unable to set non blocking mode" << std::endl;
          exit(1);
      }
  }

  /* requested number of microseconds divided by period time */
  loops = 15000000 / period_time_ms;

  /* sinus generator */
  // std::cout << "generuje sinusa" << std::endl;
  double phase_0 = 0.0, phase_0_2 = 0.0;
  double Freq = 500, Freq_2 = 200; // M.B. częstotliwość początkowa [Hz]
  while (loops > 0)
  {
    loops--;

  /*
  if (rc == 0)
  {
      std::cerr << "end of file on input" << std::endl;
      break;
  }
  else if (rc != size_b)
  {
      std::cerr << "short read: read " << rc << " bytes" << std::endl;
  }
 */

    // M.B. wariant dla stałej częstotliwości
    for (unsigned int n = 0; n < size_b / no_of_bytes_in_channel / no_of_channels; n++)
    {
        if (no_of_bytes_in_channel == 1)
        {
            buffer_8bit[no_of_channels*n] = 128 + 127 * sin(2 * M_PI * Freq / sampling_rate * n + phase_0);
            if (no_of_channels == 2)
                buffer_8bit[no_of_channels * n + 1] = 128 + 127 * sin(2 * M_PI * (Freq_2) / sampling_rate * n + phase_0_2);

        }
        else if (no_of_bytes_in_channel == 2)
        {
            buffer_16bit[no_of_channels * n] = INT16_MAX * sin(2 * M_PI * Freq / sampling_rate * n + phase_0);
            if (no_of_channels == 2)
                buffer_16bit[no_of_channels * n + 1 ] = INT16_MAX * sin(2 * M_PI * (Freq_2) / sampling_rate * n + phase_0_2);

        }
        else if (no_of_bytes_in_channel == 3)
        {
            buffer_32bit[no_of_channels * n] = INT32_MAX * sin(2 * M_PI * Freq / sampling_rate * n + phase_0);
            if (no_of_channels == 2)
                buffer_32bit[no_of_channels * n + 1 ] = INT32_MAX * sin(2 * M_PI * (Freq_2) / sampling_rate * n + phase_0_2);
        }
        else if (no_of_bytes_in_channel == 4)
        {
            if (mode == 1)
            {
                buffer_32bit[no_of_channels * n] = INT32_MAX * sin(2 * M_PI * Freq / sampling_rate * n + phase_0);
                if (no_of_channels == 2)
                    buffer_32bit[no_of_channels * n + 1 ] = INT32_MAX * sin(2 * M_PI * (Freq_2) / sampling_rate * n + phase_0_2);
            }

            else
            {
                buffer_32bit_f[no_of_channels * n] = sin(2 * M_PI * Freq / sampling_rate * n + phase_0);
                if (no_of_channels == 2)
                    buffer_32bit_f[no_of_channels * n + 1 ] = sin(2 * M_PI * (Freq_2) / sampling_rate * n + phase_0_2);
            }
        }

        else if (no_of_bytes_in_channel == 8)
        {
            buffer_64bit[no_of_channels * n] = sin(2 * M_PI * Freq / sampling_rate * n + phase_0);
            if (no_of_channels == 2)
                buffer_64bit[no_of_channels * n + 1 ] = sin(2 * M_PI * (Freq_2) / sampling_rate * n + phase_0_2);

        }

    }
    phase_0 +=  2 * M_PI * Freq / sampling_rate * size_b / no_of_bytes_in_channel / no_of_channels;
    phase_0_2 +=  2 * M_PI * Freq_2 / sampling_rate * size_b / no_of_bytes_in_channel / no_of_channels;
    std::cout << Freq << ", " << Freq_2 << std::endl;

    //std::cout << "Before snd_pcm_writei (" << loops << ")" << std::endl;
    rc = snd_pcm_writei(handle, pcm_buffer, frames);
    std::cout << "Wrote" << std::endl;

    if (rc == -EPIPE)
    {
        /* EPIPE means underrun */
        std::cerr << "Underrun occurred" << std::endl;
        snd_pcm_prepare(handle);

    }
    else if (rc < 0)
    {
        std::cerr << "Error from writei: " << snd_strerror(rc) << std::endl;

    }
    else if (rc != (int)frames)
    {
        std::cerr << "short write, write " << rc << " frames" << std::endl;
    }
  }
  std::cout << "The end of the playback" << std::endl;

  //snd_pcm_nonblock(handle, 0);
  snd_pcm_drain(handle);
  std::cout << "Closing the PCM device" << std::endl;
  snd_pcm_close(handle);
  buffer_8bit.clear(); // M.B. można nawet pominąć, bo i tak będzie wykonana przy zwalnianiu pamięci zmiennej
  buffer_16bit.clear(); // M.B. można nawet pominąć, bo i tak będzie wykonana przy zwalnianiu pamięci zmiennej
  buffer_32bit.clear(); // D.K. it will be depricated in DSPElib
  buffer_64bit.clear();

  std::cout << "Press ENTER" << std::endl;
  std::cin.get(); // tutaj oczekuję na wciśnięcie entera

  return 0;

}

int set_snd_pcm_format(int errc, int no_of_bytes_in_channel, string endianess, snd_pcm_hw_params_t *params, snd_pcm_t *alsa_handle, int mode) {
    // M.B. docelowo dodać przynajmniej obsługę 8-bitów
  if (no_of_bytes_in_channel == 1)
  {
      /* Signed 8-bit format */

      errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                          SND_PCM_FORMAT_U8);

       std::cout << "Format set with error code: " << errc << std::endl;
  }
  else if (no_of_bytes_in_channel == 2)
  {
      if (endianess == "Big Endian")
      {
        /* Signed 16-bit big-endian format */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_S16_BE);
      }

      else
      {
        /* Signed 16-bit little-endian format */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_S16_LE);
      }

      std::cout << "Format set with error code: " << errc << std::endl;
  }
  else if (no_of_bytes_in_channel == 3) // D.K. 32-bits buffer can be used
  {
      if (endianess == "Big Endian")
      {
        /* Signed 24-bit big-endian low three bytes in 32-bit word format */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_S32_BE);
      }

      else
      {
        /* Signed 24-bit little-endian low three bytes in 32-bit word format */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_S32_LE);
      }

      mode = 1;

      std::cout << "Format set with error code: " << errc << std::endl;
  }
  else if (no_of_bytes_in_channel == 4)
  {
      if (endianess == "Big Endian")
      {
        /* Float Little Endian, Range -1.0 to 1.0 */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_FLOAT_BE);
        mode = 0; // native

        if(errc < 0)
        {
            /* Signed 32-bit little-endian format */
            errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                                SND_PCM_FORMAT_S32_BE);
            mode = 1; // higher quality
        }
      }

      else
      {
        /* Float Little Endian, Range -1.0 to 1.0 */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_FLOAT_LE);
        mode = 0; // native

        if(errc < 0)
        {
            /* Signed 32-bit little-endian format */
            errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                                SND_PCM_FORMAT_S32_LE);
            mode = 1; // higher quality
        }
      }

      std::cout << "Format set with error code: " << errc << std::endl;
  }
  else if (no_of_bytes_in_channel == 8)
  {
      if (endianess == "Big Endian")
      {
        /* Float Big Endian, Range -1.0 to 1.0 */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_FLOAT64_BE);
      }

      else
      {
        /* Float Little Endian, Range -1.0 to 1.0 */
        errc = snd_pcm_hw_params_set_format(alsa_handle, params,
                                            SND_PCM_FORMAT_FLOAT64_LE);
      }

      std::cout << "Format set with error code: " << errc << std::endl;
  }
}

long DSP::ALSA_object_t::open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, unsigned int &sampling_rate, const long &audio_outbuffer_size) {
  assert(!"DSP::ALSA_object_t::open_PCM_device_4_output not implemented yet");
  int rc;
  unsigned int sampling_rate_alsa = sampling_rate; 
  rc = open_alsa_device(SND_PCM_STREAM_PLAYBACK, no_of_channels, sampling_rate_alsa); // pierwszy parametr ewentualnie 0

  if(rc > 0) return 1;
  else return -1;
}

long DSP::ALSA_object_t::open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, unsigned int &sampling_rate, const long &audio_outbuffer_size) {
  assert(!"DSP::ALSA_object_t::open_PCM_device_4_input not implemented yet");
  int rc;
  unsigned int sampling_rate_alsa = sampling_rate; 
  rc = open_alsa_device(SND_PCM_STREAM_CAPTURE, no_of_channels, sampling_rate_alsa);
  
  if(rc > 0) return 1;
  else return -1;
}

long DSP::ALSA_object_t::append_playback_buffer(DSP::Float_vector &float_buffer) {
  assert(!"DSP::ALSA_object_t::append_playback_buffer not implemented yet");
}

bool DSP::ALSA_object_t::close_PCM_device_input(void) {
  assert(!"DSP::ALSA_object_t::close_PCM_device_input not implemented yet");
  close_alsa_device(true);
  return true;
}

bool DSP::ALSA_object_t::close_PCM_device_output(void) {
  assert(!"DSP::ALSA_object_t::close_PCM_device_output not implemented yet");
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

snd_pcm_sframes_t DSP::ALSA_object_t::pcm_writei(const void *buffer, snd_pcm_uframes_t &frames) {
  snd_pcm_sframes_t rc = snd_pcm_writei(alsa_handle, buffer, frames);
  if (rc == -EPIPE) {
    /* EPIPE means underrun */
    fprintf(stderr, "underrun occurred\n");
    snd_pcm_prepare(alsa_handle);
  } else if (rc < 0) {
    fprintf(stderr,
            "error from writei: %s\n",
            snd_strerror(int(rc)));
  }  else if (rc != (int)frames) {
    fprintf(stderr,
            "short write, write %d frames\n", int(rc));
  }
  return rc;
}

void DSP::ALSA_object_t::close_alsa_device(bool do_drain, bool use_log) {
  if (alsa_handle != NULL) {
    if (do_drain == true) {
      snd_pcm_drain(alsa_handle);
    }
    snd_pcm_close(alsa_handle);
    alsa_handle = NULL;

    if (hw_params != NULL){
      snd_pcm_hw_params_free(hw_params);
      hw_params = NULL;
    }
  }

  if (use_log == true) {
    DSP::log << "ALSA PCM sound closed" << endl;
  }
}




