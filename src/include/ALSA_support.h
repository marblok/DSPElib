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
#include <cmath>
#include <DSP_types.h> // for types

namespace DSP {
    
    class ALSA_object_t : public DSP::SOUND_object_t {
    private:
        //! handlers
        snd_pcm_t *alsa_handle;
        snd_pcm_hw_params_t *hw_params;
    
        std::vector<uint8_t *> pcm_buffer;
        std::vector<snd_pcm_sframes_t> pcm_buffer_size_in_frames;

        //! output device number used in next open operations
        unsigned int OutDevNo;
        //! input device number used in next open operations 
        unsigned int InDevNo;

        //! keeping track of which buffer is currently being filled
        unsigned int NextBufferOutInd;
        unsigned int NextBufferInInd;

        //! variables holding values ​​from the external interface
        unsigned int sampling_rate_alsa;
        unsigned int no_of_channels_alsa;
        unsigned int no_of_bytes_in_channel;

        /*! It is better to use STD containers - they are more convenient, 
            and they mean fewer problems with memory leaks.
        */
        //! buffers depending on samples type
        std::vector<std::vector<uint8_t>> buffers_8bit; 
        std::vector<std::vector<int16_t>> buffers_16bit;
        std::vector<std::vector<int32_t>> buffers_32bit;
        std::vector<std::vector<float>> buffers_32bit_f;
        std::vector<std::vector<double>> buffers_64bit;
        
        //! samples are integers rather than float values  
        bool IsHigherQualityMode;
        
        //! CPU architecture
        bool IsLittleEndian;

        bool IsDeviceInputOpen;
        bool IsDeviceOutputOpen;

        //! Has playback already started?
        bool IsPlayingNow;
        bool StopPlayback;

        //! Has recording already started?
        bool StopRecording;
        bool IsRecordingNow;

        //! just samples
        snd_pcm_uframes_t audio_inbuffer_size_in_frames; // M.B. more meaningful variable name

        //! We always use the non-blocking mode in DSPElib
        bool blocking_mode;
        
        //! buffer capacity
        int size_b;

        /***************************************************/
        /****************AUDIO INPUT TEST*******************/
        //DSP::e::SampleType InSampleType;
        //static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2);
        //std::vector<std::vector<int8_t>> input_buffers;
        /**********************END**************************/
        /***************************************************/

        /*! Open default PCM device and return 1 on success or negative error code
            stream_type = SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURE
            the rest of the parameters are set in the class
        */ 
        int open_alsa_device(snd_pcm_stream_t stream_type);
        void close_alsa_device(bool do_drain = false, bool use_log = false);

        void get_period_size(snd_pcm_uframes_t &frames, unsigned int &period_time);
        
        //! playback
        snd_pcm_sframes_t pcm_writei(const void *buffer, const snd_pcm_uframes_t &frames); // M.B. this will be more transparent
        
        //! Set SND PCM format depending on no of bytes in channel and CPU endianness
        int set_snd_pcm_format(snd_pcm_hw_params_t *params);

    public:

        //! Log basic ALSA information
        void log_driver_data();

        //! Select the desired device from the interface
        unsigned int select_input_device_by_number(const unsigned int &device_number=UINT_MAX);
        unsigned int select_output_device_by_number(const unsigned int &device_number=UINT_MAX);

        //! Deciding wheter PCM device is opening for playback or capture
        long open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size = -1);
        long open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_inbuffer_size = -1);
        
        //! Closes PCM device
        bool close_PCM_device_input(void);
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

        //! object constructor and destructor
        ALSA_object_t();
        ~ALSA_object_t();
    };

}

#endif // ALSA_support_H
