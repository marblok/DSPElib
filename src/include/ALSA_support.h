/*! \file ALSA_support.h
 * ALSA support header file
 *
 * \authors Damian Karaś, Marek Blok
 */
//---------------------------------------------------------------------------
#ifndef ALSA_support_H
#define ALSA_support_H

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

/* All of the ALSA library API is defined in this header */
#include <alsa/asoundlib.h>
#include <vector>
// #include <cmath>
#include <DSP_types.h> // for types

namespace DSP {
    
    class ALSA_object_t : public DSP::SOUND_object_t {
    private:
        //! PCM device handler
        snd_pcm_t *alsa_handle;
        //! copy of set configuration space handler 
        snd_pcm_hw_params_t *hw_params;

        //! stores pointers to audio buffers
        std::vector<uint8_t *> pcm_buffer;
        //! stores size of audio buffers
        std::vector<snd_pcm_sframes_t> pcm_buffer_size_in_frames;

        //! output device number used in next open operations
        unsigned int OutDevNo;
        //! input device number used in next open operations 
        unsigned int InDevNo;

        //! keeping track of which outbuffer is currently being filled
        unsigned int NextBufferOutInd;

        //! sampling rate
        unsigned int sampling_rate_alsa;
        //! number of channels
        unsigned int no_of_channels_alsa;
        //! number of bytes in channel
        unsigned int no_of_bytes_in_channel;

        /*! 
            It is better to use STD containers - they are more convenient, 
            and they mean fewer problems with memory leaks.
        */
        //! outbuffers depending on samples type
        std::vector<std::vector<uint8_t>> buffers_8bit; 
        std::vector<std::vector<int16_t>> buffers_16bit;
        std::vector<std::vector<int32_t>> buffers_32bit;
        std::vector<std::vector<float>> buffers_32bit_f;
        std::vector<std::vector<double>> buffers_64bit;

        //! inbuffer
        std::vector<char> capture_buffer;
        
        //! samples have integer values instead of float ones  
        bool IsHigherQualityMode;
        
        //! CPU architecture - endianness
        bool IsLittleEndian;
        
        //! Is the PCM device opened for recording?
        bool IsDeviceInputOpen;
        //! Is the PCM device opened for playback?
        bool IsDeviceOutputOpen;

        //! Has playback already started?
        bool IsPlayingNow;
        //! Is playback stopped?
        bool StopPlayback;

        //! Has recording already started?
        bool StopRecording;
        //! Has recording already started?
        bool IsRecordingNow;

        //! audio inbuffer size in samples
        snd_pcm_uframes_t audio_inbuffer_size_in_frames;
        //! audio outbuffer size in samples
        snd_pcm_uframes_t audio_outbuffer_size_in_frames;

        //! type of samples in WaveInBuffers
        DSP::e::SampleType InSampleTypeALSA;
        
        //! We always use the non-blocking mode in DSPElib
        bool non_blocking_mode;
        
        //! buffer capacity
        int size_b;

        /*! Open default PCM device and return 1 on success or negative error code
            stream_type = SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURE
            the rest of the parameters are set in the class
        */ 
        int open_alsa_device(snd_pcm_stream_t stream_type);
        //! Close opened PCM device
        void close_alsa_device(bool do_drain = false, bool use_log = false);

        //void get_period_size(snd_pcm_uframes_t &frames, unsigned int &period_time);
        
        //! playback
        snd_pcm_sframes_t pcm_writei(const void *buffer, const snd_pcm_uframes_t &frames); // M.B. this will be more transparent
        
        //! Set SND PCM format depending on no of bytes in channel and CPU endianness
        int set_snd_pcm_format(snd_pcm_hw_params_t *params);

    public:

        //! Log basic ALSA information
        void log_driver_data();

        //! Select the desired device from the interface for recording
        unsigned int select_input_device_by_number(const unsigned int &device_number=UINT_MAX);
        //! Select the desired device from the interface for playback
        unsigned int select_output_device_by_number(const unsigned int &device_number=UINT_MAX);

        //! Open PCM device for playback
        long open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size = -1);
        //! Open PCM device for recording
        long open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_inbuffer_size = -1);
        
        //! Closes PCM device from recording
        bool close_PCM_device_input(void);
        //! Closes PCM device from playback
        bool close_PCM_device_output(const bool &do_drain);

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

        //! ALSA_obcject_t class constructor
        ALSA_object_t();
        //! ALSA_obcject_t class destructor
        ~ALSA_object_t();
    };

}

#endif // ALSA_support_H
