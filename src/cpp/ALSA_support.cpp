/*! \file ALSA_support.cpp
 * ALSA support code file
 *
 * \author Marek Blok
 */

#include <ALSA_support.h>

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
int DSP::ALSA_object_t::open_alsa_device(snd_pcm_stream_t stream_type, int no_of_channels, unsigned int &sampling_rate) {
  int rc;
  snd_pcm_t *handle;
  unsigned int val, val2;
  int dir;
  snd_pcm_uframes_t frames;

  if (alsa_handle != NULL)
    close_alsa_device();

  // ==================================================== //
  DSP::log << "Opening ALSA device" << endl;

  {
    //! \TODO Test mode:	Open mode (see SND_PCM_NONBLOCK, SND_PCM_ASYNC)
    /* Open PCM device for playback. */
    rc = snd_pcm_open(&handle, "default",
                      stream_type, SND_PCM_NONBLOCK);
    if (rc < 0) {
      DSP::log << "unable to open pcm device: " << snd_strerror(rc) << endl;
      //exit(1);
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
    snd_pcm_hw_params_set_access(handle, params,
                        SND_PCM_ACCESS_RW_INTERLEAVED);

    //! \TODO Format selection based on input parameters
    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(alsa_handle, params, SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(alsa_handle, params, no_of_channels);

    /* 44100 bits/second sampling rate (CD quality) */
    snd_pcm_hw_params_set_rate_near(alsa_handle, params, &sampling_rate, &dir);

    //snd_pcm_hw_params_set_buffer_size();

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(alsa_handle, params);
    if (rc < 0) {
      DSP::log << "unable to set hw parameters: " << snd_strerror(rc) << endl;
      //snd_pcm_hw_params_free(params);
      close_alsa_device();
      //exit(1);
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

  return 1;
}

void DSP::ALSA_object_t::get_period_size(snd_pcm_uframes_t &frames, unsigned int &period_time) {
  int dir;

  // snd_pcm_hw_params_current() // Retreive current PCM hardware configuration chosen with snd_pcm_hw_params.
  //snd_pcm_hw_params_current(alsa_handle, hw_params);

  // https://www.alsa-project.org/wiki/FramesPeriods
  // frame - size of sample in byts
  // A period is the number of frames in between each hardware interrupt

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(hw_params, &period_time, &dir);
}

long DSP::ALSA_object_t::open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size) {
  assert(!"DSP::ALSA_object_t::open_PCM_device_4_output not implemented yet");
}

long DSP::ALSA_object_t::open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size) {
  assert(!"DSP::ALSA_object_t::open_PCM_device_4_input not implemented yet");
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




