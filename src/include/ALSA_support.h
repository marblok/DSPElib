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
#include <vector>
#include <cmath>
#include <DSP_types.h> // for types

namespace DSP {
    
    class ALSA_object_t : public DSP::SOUND_object_t {
    private:
        //! handlers
        snd_pcm_t *handle;
        snd_pcm_t *alsa_handle;
        snd_pcm_hw_params_t *hw_params;
        unsigned char *pcm_buffer;

        //! device number used in next open operations
        unsigned int OutDevNo; 
        unsigned int InDevNo;

        //! variables holding values ​​from the external interface
        unsigned int sampling_rate_alsa; // M.B. lepiej korzystać z nazw oddających przeznaczenie zmiennej
        unsigned int no_of_channels_alsa;
        unsigned int no_of_bytes_in_channel;

        //! buffers depending on samples type
        std::vector<uint8_t> buffer_8bit; 
        std::vector<int16_t> buffer_16bit;
        std::vector<int32_t> buffer_32bit;
        std::vector<float> buffer_32bit_f;
        std::vector<double> buffer_64bit;

        /*! It is better to use STD containers - they are more convenient, 
            and they mean fewer problems with memory leaks.
        */
        
        //! samples are integers rather than float values  
        bool IsHigherQualityMode;
        
        //! CPU architecture
        bool IsLittleEndian;

        //! just samples
        snd_pcm_uframes_t frames;

        //! We always use the non-blocking mode in DSPElib
        bool blocking_mode;
        
        //! buffer capacity
        int size_b;

        /*! Open default PCM device and return 1 on success or negative error code
            stream_type = SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURE
            the rest of the parameters are set in the class
        */ 
        int open_alsa_device(snd_pcm_stream_t stream_type);
        void close_alsa_device(bool do_drain = false, bool use_log = false);

        void get_period_size(snd_pcm_uframes_t &frames, unsigned int &period_time);
        
        //! playback
        snd_pcm_sframes_t pcm_writei(const void *buffer);
        
        //! Set SND PCM format depending on no of bytes in channel and CPU endianness
        int set_snd_pcm_format(snd_pcm_hw_params_t *params);

    public:

        //! Log basic ALSA information
        void log_driver_data();

        //! Select the desired device from the interface
        unsigned int select_input_device_by_number(const unsigned int &device_number=UINT_MAX);
        unsigned int select_output_device_by_number(const unsigned int &device_number=UINT_MAX);

        //! Deciding wheter PCM device is opening for playback or capture
        long open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size = -1, long playback_time);
        long open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_inbuffer_size = -1, long playback_time);
        
        //! Closes PCM device
        bool close_PCM_device_input(void);
        bool close_PCM_device_output(void);

        //! Returns true is the playback is on
        bool is_device_playing(void);

        //! Initializes playback stopping
        bool stop_playback(void);

        //! Returns true is the sound capture is on
        bool is_device_recording(void);

        //! Returns true is the sound capture is on
        bool stop_recording(void);

        //! \note values stored in float_buffer might be altered
        long append_playback_buffer(DSP::Float_vector &float_buffer);

        //! Starts sound capture
        bool start_recording(void);

        //! For sound capture
        bool get_wave_in_raw_buffer(DSP::e::SampleType &InSampleType, std::vector<char> &wave_in_raw_buffer);

        //! Returns false if callbacks are not supported of recording
        bool is_input_callback_supported(void);

        //! Returns false if callbacks are not supported of playback
        bool is_output_callback_supported(void);

        //! object constructor and destructor
        ALSA_object_t();
        ~ALSA_object_t();
    };

}

#endif // ALSA_support_H
