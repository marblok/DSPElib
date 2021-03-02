/*! \file DSP_AudioMixer.h
 * AudioMixer header file
 *
 * \author Marek Blok
 */
//---------------------------------------------------------------------------
#ifndef AudioMixerH
#define AudioMixerH

#if __BORLANDC__ == 0x0520
  #define _ANONYMOUS_STRUCT
#else
#endif

#ifdef WIN32
  #include <windows.h>
  #include <mmsystem.h>
#else
  #define DWORD unsigned int
  #define UINT unsigned int
  #define MAXPNAMELEN 256

  //! \TODO to be removed/replaced with linux specific implementation
  #define WAVE_MAPPER ((UINT)-1)

#endif

#include <string>
using namespace std;

//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------

namespace DSP {
  const int AM_MasterControl = -1;
  const int AM_PCMwaveFile = -2;
  const int AM_PCMwaveFileON = -2;
  const int AM_PCMwaveFileOFF = -3;

  namespace e {
    enum struct AM_MutedState {
      MUTED_INACTIVE = -1,
      MUTED_YES = 1, MUTED_NO = 0
    };
  }
}

class TAudioMixer
{
  private:
#ifdef WIN32
    static UINT WaveInCaps_size;
    static WAVEINCAPS  *WaveInCaps;
    static UINT WaveOutCaps_size;
    static WAVEOUTCAPS *WaveOutCaps;
#endif // WIN32

  public:
    static string GetWaveInDevName(UINT DevNo=WAVE_MAPPER );
    static string GetWaveOutDevName(UINT DevNo=WAVE_MAPPER );

    bool MixerSupported;
    //! true if there is global mixer or multiplexer for input lines
    bool InputMixer_support;

    static const string PCMwaveFileName;

    static const DWORD Types[];
    static const string TypesNames[];
    static const string ComponentNames[];
    static const DWORD ComponentTypes[];

//    AnsiString MixerName;
    string Input_MixerName;
    string Output_MixerName;
    string Input_Output_MixerName;

    int Mixer_InputLinesNumber; //! Number of input lines supported by soundcard

    #ifdef WIN32
      MMRESULT rs;

      UINT MixersNumber; //Number of available mixers
      UINT WaveInNumber; //Number of wave in audio devices
      UINT WaveOutNumber; //Number of wave out audio devices
      HMIXER hMixer_out; //Handler of the opened output mixer
      HMIXER hMixer_in;  //Handler of the opened input mixer

      // INPUTS
      MIXERLINE MixerLineWAVEIN; //WAVEIN line currently in use
      MIXERCONTROL MixerControlWAVEIN; //Main Mixer control for WAVEIN line
      MIXERCONTROL MixerControlWAVEIN_VOLUME; //Main Mixer volume control for WAVEIN line
      bool MixerControlWAVEIN_VOLUME_supported;
      MIXERCONTROL MixerControlWAVEIN_MUTE; //Main Mixer MUTE control for WAVEIN line
      bool MixerControlWAVEIN_MUTE_supported;

      //Number of items: MixerControlWAVEIN.cMultipleItems
      MIXERCONTROLDETAILS_LISTTEXT *MixerControlDetailsWAVEIN_LISTTEXT;
      //Details for WAVEIN mixer control attached lines (sources) names and IDs (dwParam1)
      MIXERLINE *MixerLinesWAVEIN; //pointer for lines of WAVEIN sources
      DWORD MixerLinesWAVEIN_MAXcChannels;
      MIXERCONTROL *MixerControlsWAVEIN_VOLUME; //pointer for controls for WAVEIN sources
      bool         *MixerControlsWAVEIN_VOLUME_supported; //pointer table with entries indicating if volume control is supported
      MIXERCONTROL *MixerControlsWAVEIN_MUTE; //pointer for controls for WAVEIN sources
      bool         *MixerControlsWAVEIN_MUTE_supported;



      // OUTPUTS
      bool MixerSupportedOUT;
      MIXERLINE MixerLineOUT; //output mixer master line
      MIXERCONTROL MixerControlOUT_VOL; //Main Mixer volume control for output line
      MIXERCONTROL MixerControlOUT_MUTE; //Main Mixer MUTE control for output line

      //Number of items: MixerLineOUT.cConnections
  //    MIXERCONTROLDETAILS_LISTTEXT *MixerControlDetailsOUT_LISTTEXT;
      //Details for output mixer control attached lines (destinations) names and IDs (dwParam1)
      MIXERLINE *MixerLinesOUT; //pointer for lines of outputs
      DWORD MixerLinesOUT_MAXcChannels;
      MIXERCONTROL *MixerControlsOUT_VOL; //pointer for controls for outputs volume
      bool         *MixerControlsOUT_VOL_supported;
      MIXERCONTROL *MixerControlsOUT_MUTE; //pointer for controls for outputs MUTE
      bool         *MixerControlsOUT_MUTE_supported;


      //Memorized
      bool                Memorized_WAVEIN_MasterState[1];
      double              Memorized_WAVEIN_MasterVolume[1];
      MIXERCONTROLDETAILS_BOOLEAN *Memorized_ControlWAVEIN_BOOLEAN; //Memorized ControlWAVEIN state
      MIXERCONTROLDETAILS_UNSIGNED *Memorized_ControlWAVEIN_UNSIGNED; //Memorized source lines volume values

    #endif

    DSP::e::AM_MutedState    Memorized_OUT_MasterState[2];
    double                   Memorized_OUT_MasterVolume[2];
    DSP::e::AM_MutedState    *Memorized_OUT_LinesStates;
    double                   *Memorized_OUT_LinesVolumes;

    bool MixerSettingsMemorized_WAVEIN;
    bool MixerSettingsMemorized_OUT;

    static const string GetMixerControlType(DWORD dwControlType);
    static const string GetMixerComponentType(DWORD dwComponentType);

    bool PCMwaveFileActive;
    double PCMwaveFileActiveValue;
  public:
    //! Opens mixer to use with selected WaveIn and WaveOut devices
    /*!  WaveInDevNo  - waveIn device number
     *   WaveOutDevNo - waveOut device number
     */
    TAudioMixer(UINT WaveInDevNo = 0, UINT WaveOutDevNo = 0);
    ~TAudioMixer(void);

//    void TestInfo(TStrings *Lines);

//    AnsiString GetMixerName(void);
    string GetMixerName(void);

    //! Returns empty string on failure
    string GetSourceLineName(int ind);
    DWORD GetSourceLineType(int ind);
    int GetNumberOfSourceLines(void);

    //! Returns empty string on failure
    string GetDestLineName(int ind);
    DWORD GetDestLineType(int ind);
    int GetNumberOfDestLines(void);

    int  GetActiveSourceLine(void);
    void SetActiveSourceLine(int ActiveNo);
    void SetActiveSourceLine(string ActiveName);
    void SetSourceLineState(int LineNo, bool IsActive);
    //! gets active source line volume
    /*! - returns vol in range [0, 1] on success
     *  - returns -1.0 if failed
     *  .
     */
    double GetActiveSourceLineVolume(void);
    //! gets given source line volume
    /*! - LineNo
     *   - AM_PCMwaveFile - set gain factor for wave file input
     *   - AM_MasterControl - set input master volume
     *   - 0..NoOfSourceLines-1 - set volume for given input
     *   .
     *  - returns vol in range [0, 1] on success
     *  - returns -1.0 if failed
     *  .
     */
    double GetSourceLineVolume(int LineNo);
    bool GetSourceLineState(int LineNo); // SELECTED : ON/OFF
    //! sets active source line volume
    /*!
     *  - vol in range [0, 1]
     *  - Returns true on success
     *  .
     */
    bool SetActiveSourceLineVolume(double Vol);
    //! sets source line volume
    /*!
     *  - vol in range [0, 1]
     *  - LineNo
     *    - AM_PCMwaveFile - set gain factor for wave file input
     *    - AM_MasterControl - set input master volume
     *    - 0..NoOfSourceLines-1 - set volume for given input
     *    .
     *  - Returns true on success
     *  .
     */
    bool SetSourceLineVolume(int LineNo, double Vol);

    //! Channel:
    /*!  - "-1" - is any ON
     *  - "0" - is left ON
     *  - "1" - is right ON
     */
    DSP::e::AM_MutedState GetDestLineState(int LineNo, int Channel = -1); // 0/1 - MUTE : ON/OFF; -1 - unavailable
//    bool SetDestLineState(int LineNo, bool IsMuted);
    bool SetDestLineState(int LineNo, DSP::e::AM_MutedState IsMuted_Left, DSP::e::AM_MutedState IsMuted_Right=DSP::e::AM_MutedState::MUTED_INACTIVE);
//    void SetDestLineState(char *LineName, bool IsMuted);
    //! Channel:
    /*!  - "-1" - mean value
     *  - "0" - left channel
     *  - "1" - right channel
     *
     * Line:
     *  "-1" - master volume
     *  "0","1"... output lines
     *
     *  returns:
     *   - < 0 on error
     *   - -2.0 if volume control is not supported for given line
     *   .
     */
    double GetDestLineVolume(int LineNo, int Channel = -1); // 0/1 - MUTE : ON/OFF
//    bool   SetDestLineVolume(int LineNo, double Vol);
    //! Sets destination line volume
    /*!
     *  - Vol_Left, Vol_Right in range [0, 1]
     *  - LineNo
     *    - AM_PCMwaveFile - set gain factor for wave file input
     *    - AM_MasterControl - set input master volume
     *    - 0..NoOfSourceLines-1 - set volume for given input
     *    .
     *  - Returns true on success
     *  .
     */
    bool SetDestLineVolume(int LineNo, double Vol_Left, double Vol_Right=-1.0);

    void MemorizeMixerSettings_WAVEIN(void);
    void ForgetMixerSettings_WAVEIN(void);
    void RestoreMixerSettings_WAVEIN(void);

    void MemorizeMixerSettings_OUT(void);
    void ForgetMixerSettings_OUT(void);
    void RestoreMixerSettings_OUT(void);
};
#endif
