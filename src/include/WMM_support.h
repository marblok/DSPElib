/*! \file WMM_support.h
 * WMM support header file
 *
 * \author Marek Blok
 */
//---------------------------------------------------------------------------
#ifndef WMM_support_H
#define WMM_support_H

/* inluce WMM API */
////#ifdef __CYGWIN__
//#ifdef WIN32
#if defined(WIN32) || defined(WIN64)
  #include <windef.h>
  #include <mmsystem.h>
#else
  #error NO WIN32
#endif

#include <DSP_types.h> // for types

namespace DSP {
    class WMM_object_t : public DSP::SOUND_object_t {
    private:
      bool is_device_input_open;
      bool is_device_output_open;

      HWAVEIN  hWaveIn;
      HWAVEOUT hWaveOut;

      MMRESULT result;

      //Rezerwacja pamięci dla formatu WAVE
      //  WAVEFORMATEX wfx; //to wymaga korekty
      PCMWAVEFORMAT wfx;

      unsigned int WaveOutDevNo; // device numer used in next open operations

      std::vector<WAVEHDR> waveHeaderOut;
      uint32_t WaveOutBufferLen;  // in bytes
      //! Buffers for audio samples prepared for playing
      std::vector<std::vector<uint8_t>> WaveOutBuffers;
      //! Type of samples in WaveOutBuffers
      DSP::e::SampleType OutSampleType;
      //! Index of the buffer which must be used next time
      unsigned long NextBufferOutInd;
         
      bool StopPlayback;
      bool IsPlayingNow;

      static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg,
        uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2);


      unsigned int WaveInDevNo; // device numer used in next open operations

      //! Index of the buffer which is expected to filled next
      short NextBufferInInd;
      //! Type of samples in WaveInBuffers
      DSP::e::SampleType InSampleType;
      std::vector<WAVEHDR> waveHeaderIn;
      uint32_t WaveInBufferLen;  // in bytes
      //! Buffers for audio samples prepared for playing
      std::vector<std::vector<char>> WaveInBuffers;

      static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, uint32_t dwInstance, uint32_t dwParam1, uint32_t dwParam2);

      bool StopRecording;
      // indicates whether the recording started yet
      bool IsRecordingNow;

    public:

        //! log basic WMM information
        void log_driver_data();

        // returns false if device is already opened
        unsigned int select_input_device_by_number(const unsigned int &device_number=UINT_MAX);
        unsigned int select_output_device_by_number(const unsigned int &device_number=UINT_MAX);

        //! opens default PCM device for playback and returns selected sampling rate on success or negative error code
        long open_PCM_device_4_output(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_outbuffer_size = -1);
        //! opens default PCM device for capture and returns selected sampling rate on success or negative error code
        long open_PCM_device_4_input(const int &no_of_channels, int no_of_bits, const long &sampling_rate, const long &audio_inbuffer_size = -1);


        bool close_PCM_device_input(void);
        bool close_PCM_device_output(const bool &do_drain);

        bool stop_playback(void);
        bool stop_recording(void);
        //! returns true is the playback is on
        bool is_device_playing(void);
        //! returns true is the playback is on
        bool is_device_recording(void);

        bool is_input_callback_supported(void);
        bool is_output_callback_supported(void);

        //void get_params(snd_pcm_uframes_t &frames, unsigned int &period_time);
        //snd_pcm_sframes_t pcm_writei(const void *buffer, snd_pcm_uframes_t &frames);
        //! \note values stored in float_buffer might be altered
        long append_playback_buffer(DSP::Float_vector &float_buffer);
        bool start_recording(void);
        bool get_wave_in_raw_buffer(DSP::e::SampleType &InSampleType, std::vector<char> &wave_in_raw_buffer);

        //! object constructor \TODO check use of virtual in constructor and destructor
        WMM_object_t();
        ~WMM_object_t();
    };

}

#endif // WMM_support_H
