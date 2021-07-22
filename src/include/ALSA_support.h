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

#include <iostream> // w przyszłości pozamieniam na DSP::log
#include <vector>
#include <cmath>
#include <DSP_types.h> // for types

namespace DSP {
    
    class ALSA_object_t : public DSP::SOUND_object_t {
    private:
        snd_pcm_t *alsa_handle;
        snd_pcm_hw_params_t *hw_params;

        unsigned int sampling_rate; // M.B. lepiej korzystać z nazw oddających przeznaczenie zmiennej

        unsigned int period_time_ms; // M.B. i ograniczać wykorzystywanie jednej zmiennej do przechowywania wartości różniących się interpretacją i przeznaczeniem
        unsigned int no_of_channels;
        unsigned int no_of_bytes_in_channel;

        int dir;
        snd_pcm_uframes_t frames;

        std::vector<unsigned char> buffer_8bit; // M.B. lepiej korzystać z kontenerów STD, są wygodniejsze i oznaczają mniej problemów z wyciekami pamięci
        std::vector<int16_t> buffer_16bit; // M.B. dla odtwarzania 16-bitowego (int to dłuższe słowa)
        std::vector<int32_t> buffer_32bit; // D.K. dla odtwarzania 24 i 32-bitowego
        std::vector<double> buffer_64bit; // D.K. dla odtwarzania 64-bitowego

        unsigned char *pcm_buffer = NULL;

        bool blocking_mode = false; // M.B. na potrzeby realizacji odtwarzania w trybie non-blocking

        std::string endianess;
        endianess = system("lscpu | grep \"Byte Order\" | egrep -o 'Little Endian|Big Endian'"); // gdzies to trzeba umiescic

        //! open default PCM device and return 1 on success or negative error code
        /*! stream_type = SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURE
        */
        int open_alsa_device(snd_pcm_stream_t stream_type, unsigned int no_of_channels, unsigned int no_of_bytes_in_channel, unsigned int &sampling_rate);
        void close_alsa_device(bool do_drain = false, bool use_log = false);

        void get_period_size(snd_pcm_uframes_t &frames, unsigned int &period_time);
        snd_pcm_sframes_t pcm_writei(const void *buffer, snd_pcm_uframes_t &frames);

    public:

        //! log basic ALSA information
        void log_driver_data();

        unsigned int select_input_device_by_number(const unsigned int &device_number=UINT_MAX);
        unsigned int select_output_device_by_number(const unsigned int &device_number=UINT_MAX);

        long open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size = -1);
        long open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size = -1);
        bool close_PCM_device_input(void);
        bool close_PCM_device_output(void);

        //! returns true is the playback is on
        bool is_device_playing(void);
        //! initializes playback stopping
        bool stop_playback(void);
        //! returns true is the sound capture is on
        bool is_device_recording(void);
        //! returns true is the sound capture is on
        bool stop_recording(void);

        //! \note values stored in float_buffer might be altered
        long append_playback_buffer(DSP::Float_vector &float_buffer);
        //! Starts sound capture
        bool start_recording(void);
        bool get_wave_in_raw_buffer(DSP::e::SampleType &InSampleType, std::vector<char> &wave_in_raw_buffer);

        //! Returns false if callbacks are not supported of recording
        bool is_input_callback_supported(void);

        //! Returns false if callbacks are not supported of playback
        bool is_output_callback_supported(void);

        //! object constructor
        ALSA_object_t();
        ~ALSA_object_t();
    };

}

#endif // ALSA_support_H
