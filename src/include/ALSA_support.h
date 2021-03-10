/*! \file ALSA_support.h
 * ALSA support header file
 *
 * \author Marek Blok
 */
//---------------------------------------------------------------------------
#ifndef ALSA_support_H
#define ALSA_support_H

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

/* All of the ALSA library API is defined in this header */
#include <alsa/asoundlib.h>

#include <DSP_lib.h> // for logging

class ALSA_object_t {
private:
    snd_pcm_t *alsa_handle;
    snd_pcm_hw_params_t *hw_params;

public:

    //! log basic ALSA information
    void log_alsa_data();

    //! open default PCM device and return 1 on success or negative error code
    /*! stream_type = SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURE
     */
    int open_alsa_device(snd_pcm_stream_t stream_type, int no_of_channels, unsigned int &sampling_rate);
    void close_alsa_device(bool do_drain = false, bool use_log = false);

    void get_params(snd_pcm_uframes_t &frames, unsigned int &period_time);
    snd_pcm_sframes_t pcm_writei(const void *buffer, snd_pcm_uframes_t &frames);

    //! object constructor
    ALSA_object_t();
    ~ALSA_object_t();
};

#endif // ALSA_support_H
