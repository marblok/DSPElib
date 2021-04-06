/*! \file DSP_AudioMixer.cpp
 * AudioMixer code file
 *
 * \author Marek Blok
 */
//#include <vcl.h>
#include <stdio.h>
#include <iomanip>

//#pragma hdrstop
#include <stdio.h>

#include <DSP_lib.h>
//#include "DSP_AudioMixer.h"

//---------------------------------------------------------------------------
//#pragma package(smart_init)
const string TAudioMixer::PCMwaveFileName= "PCM wave file";

#ifdef WIN32
  UINT TAudioMixer::WaveInCaps_size = 0;
  std::vector<WAVEINCAPS> TAudioMixer::WaveInCaps;
  UINT TAudioMixer::WaveOutCaps_size = 0;
  std::vector<WAVEOUTCAPS> TAudioMixer::WaveOutCaps;
#endif // WIN32

string TAudioMixer::GetWaveInDevName(UINT DevNo)
{
#ifdef WIN32
  UINT ile;
  UINT ind;

  ile = waveInGetNumDevs();
  if (ile > 0)
  {
    if (ile != WaveInCaps_size)
    {
      WaveInCaps.clear();
      WaveInCaps.resize(ile+1);

      for (ind=0; ind < ile; ind++)
        waveInGetDevCaps(ind, &(WaveInCaps[ind]), sizeof(WAVEINCAPS));
      WaveInCaps_size = ile;
      waveInGetDevCaps(WAVE_MAPPER, &(WaveInCaps[WaveInCaps_size]), sizeof(WAVEINCAPS));
    }
  }
  else
    return "";

  if (DevNo == WAVE_MAPPER)
    return WaveInCaps[WaveInCaps_size].szPname;
  if (DevNo < WaveInCaps_size)
    return WaveInCaps[DevNo].szPname;
#else
  (void)DevNo; // unused
#endif // WIN32
  return "";
}

string TAudioMixer::GetWaveOutDevName(UINT DevNo)
{
#ifdef WIN32
  UINT ile;
  UINT ind;

  ile = waveOutGetNumDevs();
  if (ile > 0)
  {
    if (ile != WaveOutCaps_size)
    {
      WaveOutCaps.clear();
      WaveOutCaps.resize(ile+1);

      for (ind=0; ind < ile; ind++)
        waveOutGetDevCaps(ind, &(WaveOutCaps[ind]), sizeof(WAVEOUTCAPS));
      WaveOutCaps_size = ile;
      waveOutGetDevCaps(WAVE_MAPPER, &(WaveOutCaps[WaveOutCaps_size]), sizeof(WAVEOUTCAPS));
    }
  }
  else
    return "";

  if (DevNo == WAVE_MAPPER)
    return WaveOutCaps[WaveOutCaps_size].szPname;
  if (DevNo < WaveOutCaps_size)
    return WaveOutCaps[DevNo].szPname;
#else
  (void)DevNo; // unused
#endif // WIN32
  return "";
}

//===============================================================//
//===============================================================//
//===============================================================//
/*! Fixed <b>2006.01.23</b> Added support for soundcards without global MIXER or MULTIPLEXER for input lines
 */
TAudioMixer::TAudioMixer(UINT WaveInDevNo, UINT WaveOutDevNo)
{
  #ifdef __DEBUG__
    string tekst;
    DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << "Mixer initialization start" << endl;
  #endif

  #ifdef WIN32
    int ind, ind_2;

    MIXERCAPS MixerCaps_in, MixerCaps_out;
  //  MIXERLINE MixerLine;
    MIXERLINECONTROLS MixerLineControl;
    std::vector<MIXERCONTROL> MixerControls;
    MIXERCONTROLDETAILS MixerControlDetails;

    Memorized_ControlWAVEIN_BOOLEAN.clear();
    Memorized_ControlWAVEIN_UNSIGNED.clear();
    Memorized_OUT_LinesStates.clear();
    Memorized_OUT_LinesVolumes.clear();

    // INPUTS
    MixerLinesWAVEIN.clear();
    MixerControlsWAVEIN_VOLUME.clear();
    MixerControlsWAVEIN_VOLUME_supported.clear();
    MixerControlsWAVEIN_MUTE.clear();
    MixerControlsWAVEIN_MUTE_supported.clear();
    MixerControlDetailsWAVEIN_LISTTEXT.clear();
    MixerSupported=false;

    // OUTPUTS
    MixerLinesOUT.clear();
    MixerControlsOUT_VOL.clear(); //pointer for controls for outputs volume
    MixerControlsOUT_VOL_supported.clear();
    MixerControlsOUT_MUTE.clear(); //pointer for controls for outputs MUTE
    MixerControlsOUT_MUTE_supported.clear();
    MixerSupportedOUT=false;


    // *********************************** //
    MixersNumber=mixerGetNumDevs();
    WaveInNumber=waveInGetNumDevs();
    WaveOutNumber=waveOutGetNumDevs();
    Mixer_InputLinesNumber = 0;

    #ifdef __DEBUG__
    {
      DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second
        << "No of mixer devices:" << to_string(MixersNumber)
        << ", No of WaveIn devices:" << to_string(WaveInNumber) << ", No of WaveOut devices:"
        << to_string(WaveOutNumber) << endl;
    }
    #endif

    if (WaveInNumber>0)
    { //we'll open first available mixer device
      //  Use the mixerGetNumDevs function to determine the number of audio
      //  mixer devices present in the system. The device identifier specified
      //  by uMxId varies from zero to one less than the number of devices present
//      rs=mixerOpen(&hMixer, 0,
//        0, //Handle - callback window handle
//        0, //DWORD dwInstance,
//        MIXER_OBJECTF_MIXER); //CALLBACK_WINDOW //DWORD fdwOpen
      rs=mixerOpen(&hMixer_in, WaveInDevNo,
        0, //Handle - callback window handle
        0, //DWORD dwInstance,
        MIXER_OBJECTF_WAVEIN); //CALLBACK_WINDOW //DWORD fdwOpen
      #ifdef __DEBUG__
      {
        stringstream ss;
        ss << setfill ('0') << setw(8) << hex << rs;
        tekst = "Opened mixer (MIXER_OBJECTF_WAVEIN (dev:" + to_string(WaveInDevNo)
            + "/" + to_string(WaveInNumber) + ")) / result:" + ss.str();
        DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
      }
      #endif
      rs=mixerOpen(&hMixer_out, WaveOutDevNo,
        0, //Handle - callback window handle
        0, //DWORD dwInstance,
        MIXER_OBJECTF_WAVEOUT); //CALLBACK_WINDOW //DWORD fdwOpen
      #ifdef __DEBUG__
      {
        stringstream ss;
        ss << setfill ('0') << setw(8) << hex << rs;
        tekst = "Opened mixer (MIXER_OBJECTF_WAVEOUT (dev:" + to_string(WaveOutDevNo)
            + "/" + to_string(WaveOutNumber) + ")) / result:" + ss.str();
        DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
      }
      #endif

      //waveInGetNumDevs / MIXER_OBJECTF_WAVEIN
      //waveOutGetNumDevs / MIXER_OBJECTF_WAVEOUT


      //Get input Mixer name
      //rs=mixerGetDevCaps(UINT(hMixer_in), &MixerCaps_in, sizeof(MIXERCAPS));
      rs=mixerGetDevCaps(UINT_PTR(hMixer_in), &MixerCaps_in, sizeof(MIXERCAPS));
      Input_MixerName = MixerCaps_in.szPname;
      #ifdef __DEBUG__
      {
        stringstream ss;
        ss << setfill ('0') << setw(8) << hex << rs;
        tekst = "Input mixer name: \"" + Input_MixerName + "\" / result:" + ss.str();
        DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
      }
      #endif
      //Get output Mixer name
      //rs=mixerGetDevCaps(UINT(hMixer_out), &MixerCaps_out, sizeof(MIXERCAPS));
      rs=mixerGetDevCaps(UINT_PTR(hMixer_out), &MixerCaps_out, sizeof(MIXERCAPS));
      Output_MixerName = MixerCaps_out.szPname;
      #ifdef __DEBUG__
      {
        stringstream ss;
        ss << setfill ('0') << setw(8) << hex << rs;
        tekst = "Output mixer name: \"" + Output_MixerName + "\" / result:" + ss.str();
        DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
      }
      #endif

      //Get first WAVEIN line //In the future it might be all WAVEIN lines
      MixerLineWAVEIN.cbStruct=sizeof(MIXERLINE);
      //MixerLineWAVEIN.dwDestination=0; // first destination lines
      //    rs=mixerGetLineInfo((void *)0, &MixerLineWAVEIN, MIXER_OBJECTF_WAVEIN);
      //rs=mixerGetLineInfo(hMixer_in, //the identifier of a waveform-audio input device in the range of zero to one less than the number of devices returned by the waveInGetNumDevs function
      //  &MixerLineWAVEIN, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER);
      MixerLineWAVEIN.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
      rs=mixerGetLineInfo((HMIXEROBJ)hMixer_in, &MixerLineWAVEIN, MIXER_GETLINEINFOF_COMPONENTTYPE | MIXER_OBJECTF_HMIXER);
      if (rs != 0)
      {
        //! \bug Problems might occur when there is no WAVEIN input device (MixerLineWAVEIN will be corrupt)
        #ifdef __DEBUG__
          DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << "No WAVEIN device found !!!" << endl;
        #endif
        MixerLineWAVEIN.cControls = 0;
        MixerLineWAVEIN.cConnections = 0;
      }

      #ifdef __DEBUG__
      {
        stringstream ss;
        ss << setfill ('0') << setw(8) << hex << rs;
        tekst = "Got info about first WAVEIN destination line / result:" + ss.str();
        DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
      }
      #endif

      MixerControls.clear();
      InputMixer_support = false;
      if (MixerLineWAVEIN.cControls > 0)
      {
        //Get WAVEIN line controls
        MixerControls.resize(MixerLineWAVEIN.cControls);
        MixerLineControl.cbStruct=sizeof(MIXERLINECONTROLS);
        MixerLineControl.dwLineID=MixerLineWAVEIN.dwLineID;
        MixerLineControl.cControls=MixerLineWAVEIN.cControls;
        MixerLineControl.pamxctrl=MixerControls.data();
        MixerLineControl.cbmxctrl=sizeof(MIXERCONTROL);
    //    rs=mixerGetLineControls((void *)hMixer, //HMIXEROBJ hmxobj,
    //      &MixerLineControl, MIXER_GETLINECONTROLSF_ALL);
//MIXER_OBJECTF_WAVEIN The hmxobj parameter is the identifier of a waveform-audio input device in the range of zero to one less than the number of devices returned by the waveInGetNumDevs function.
//MIXER_OBJECTF_WAVEOUT The hmxobj parameter is the identifier of a waveform-audio output device in the range of zero to one less than the number of devices returned by the waveOutGetNumDevs function.
//        rs=mixerGetLineControls((HMIXEROBJ)hMixer, //HMIXEROBJ hmxobj,
//          &MixerLineControl, MIXER_GETLINECONTROLSF_ALL);
        rs=mixerGetLineControls((HMIXEROBJ)hMixer_in, //first WAVEIN device,
          &MixerLineControl, MIXER_GETLINECONTROLSF_ALL | MIXER_OBJECTF_HMIXER);
        #ifdef __DEBUG__
        {
          stringstream ss;
          ss << setfill ('0') << setw(8) << hex << rs;
          tekst = "Got master controls of first WAVEIN destination line / result:" + ss.str();
          DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
        }
        #endif

        //Find first control of a type: MIXERCONTROL_CONTROLTYPE_MIXER or MIXERCONTROL_CONTROLTYPE_MUX
        //other not supported
        for (ind=0; ind<(int)MixerLineWAVEIN.cControls; ind++)
        {
          if ((MixerControls[ind].dwControlType==MIXERCONTROL_CONTROLTYPE_MIXER) ||
              (MixerControls[ind].dwControlType==MIXERCONTROL_CONTROLTYPE_MUX))
          {
            CopyMemory(&MixerControlWAVEIN,&(MixerControls[ind]),sizeof(MIXERCONTROL));
            InputMixer_support = true;
            MixerSupported=true;
            #ifdef __DEBUG__
            {
              stringstream ss;
              ss << setfill ('0') << setw(8) << hex << rs;
              tekst = "Found master input mixer or multiplexer control (no " +
                  to_string(ind) + ") / result: " + ss.str();
              DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
            }
            #endif
          }
          if (MixerControls[ind].dwControlType==MIXERCONTROL_CONTROLTYPE_VOLUME)
          {
            CopyMemory(&MixerControlWAVEIN_VOLUME,&(MixerControls[ind]),sizeof(MIXERCONTROL));
            MixerControlWAVEIN_VOLUME_supported = true;
            #ifdef __DEBUG__
            {
              stringstream ss;
              ss << setfill ('0') << setw(8) << hex << rs;
              tekst = "Found master input volume control (no " + to_string(ind)
                  + ") / result: " + ss.str();
              DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
            }
            #endif
            //break;
          }
          if (MixerControls[ind].dwControlType==MIXERCONTROL_CONTROLTYPE_MUTE)
          {
            CopyMemory(&MixerControlWAVEIN_MUTE,&(MixerControls[ind]),sizeof(MIXERCONTROL));
            MixerControlWAVEIN_MUTE_supported = true;
            #ifdef __DEBUG__
            {
              stringstream ss;
              ss << setfill ('0') << setw(8) << hex << rs;
              tekst = "Found master input mute control (no " + to_string(ind)
                  + ") / result: " + ss.str();
              DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
            }
            #endif
            //break;
          }
        }
      }

      /*! \todo 2006_01_23 Check if MixerControlWAVEIN.cMultipleItems should be replaced by Mixer_InputLinesNumber
       */
      if (InputMixer_support==true)
      { // Card supports global MIXER or MULTIPLEXER for input lines
        #ifdef __DEBUG__
          DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << "InputMixer_support==true" << endl;
        #endif
        Mixer_InputLinesNumber = MixerControlWAVEIN.cMultipleItems;

        //Get attached lines names and IDs //MixerControlDetails
        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
        MixerControlDetails.dwControlID=MixerControlWAVEIN.dwControlID;
        MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=Mixer_InputLinesNumber;
        MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_LISTTEXT);
        MixerControlDetailsWAVEIN_LISTTEXT.resize(Mixer_InputLinesNumber);
        MixerControlDetails.paDetails=&(MixerControlDetailsWAVEIN_LISTTEXT[0]);
        rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_LISTTEXT);
        #ifdef __DEBUG__
        {
          stringstream ss;
          ss << setfill ('0') << setw(8) << hex << rs;
          tekst = "Input mixer /multiplexer control info read (cChannels = 1) / result: " + ss.str();
          DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
        }
        #endif
        if (rs != 0)
        {
          InputMixer_support = false;
          // Correction: card doesn't support global MIXER or MULTIPLEXER for input lines
          #ifdef __DEBUG__
          {
            stringstream ss;
            ss << setfill ('0') << setw(8) << hex << rs;
            tekst = "Input mixer /multiplexer control info read (cChannels = 0) / result: " + ss.str();
            DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
          }
          #endif
        }
      }

      if (InputMixer_support==true)
      { // Card supports global MIXER or MULTIPLEXER for input lines
        //allocate memory for volume controls for each source line
        MixerControlsWAVEIN_VOLUME.resize(Mixer_InputLinesNumber);
        MixerControlsWAVEIN_VOLUME_supported.resize(Mixer_InputLinesNumber);
        MixerLinesWAVEIN.resize(Mixer_InputLinesNumber);

        //For each line get it's details and controls then find //MIXERCONTROL_CONTROLTYPE_VOLUME
        //mixerGetLineDetails
        //mixerGetLineControls / MIXER_GETLINECONTROLSF_ALL
        MixerLinesWAVEIN_MAXcChannels=0;
        for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
        {
          //get WAVEIN source lines properties
          MixerLinesWAVEIN[ind].cbStruct=sizeof(MIXERLINE);
          MixerLinesWAVEIN[ind].dwLineID=MixerControlDetailsWAVEIN_LISTTEXT[ind].dwParam1;
          rs=mixerGetLineInfo((HMIXEROBJ)hMixer_in, //the identifier of a waveform-audio input device in the range of zero to one less than the number of devices returned by the waveInGetNumDevs function
                              &(MixerLinesWAVEIN[ind]), MIXER_GETLINEINFOF_LINEID | MIXER_OBJECTF_HMIXER);
          #ifdef __DEBUG__
          {
            stringstream ss;
            ss << setfill ('0') << setw(8) << hex << rs;
            tekst = "Input mixer/multiplexer source line " + to_string(ind)
                + " info read / result: " + ss.str();
            DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
          }
          #endif
          if (MixerLinesWAVEIN_MAXcChannels<MixerLinesWAVEIN[ind].cChannels)
            MixerLinesWAVEIN_MAXcChannels=MixerLinesWAVEIN[ind].cChannels;

          //Get controls properties of particular WAVEIN line
          MixerControls.resize(MixerLinesWAVEIN[ind].cControls);
          MixerLineControl.cbStruct=sizeof(MIXERLINECONTROLS);
          MixerLineControl.dwLineID=MixerLinesWAVEIN[ind].dwLineID;
          MixerLineControl.cControls=MixerLinesWAVEIN[ind].cControls;
          MixerLineControl.pamxctrl=MixerControls.data();
          MixerLineControl.cbmxctrl=sizeof(MIXERCONTROL);
          rs=mixerGetLineControls((HMIXEROBJ)hMixer_in, //HMIXEROBJ hmxobj,
            &MixerLineControl, MIXER_GETLINECONTROLSF_ALL);
          #ifdef __DEBUG__
          {
            stringstream ss;
            ss << setfill ('0') << setw(8) << hex << rs;
            tekst = "Input mixer source line " + to_string(ind)
                + " controls info read / result: " + ss.str();
            DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
          }
          #endif
          //Find control MIXERCONTROL_CONTROLTYPE_VOLUME and copy it
          ZeroMemory(&(MixerControlsWAVEIN_VOLUME[ind]),sizeof(MIXERCONTROL));
          MixerControlsWAVEIN_VOLUME_supported[ind] = false;
          for (ind_2=0; ind_2<(int)MixerLinesWAVEIN[ind].cControls; ind_2++)
          {
            if (MixerControls[ind_2].dwControlType==MIXERCONTROL_CONTROLTYPE_VOLUME)
            {
              CopyMemory(&(MixerControlsWAVEIN_VOLUME[ind]),&(MixerControls[ind_2]),sizeof(MIXERCONTROL));
              MixerControlsWAVEIN_VOLUME_supported[ind] = true;
              #ifdef __DEBUG__
              {
                stringstream ss;
                ss << setfill ('0') << setw(8) << hex << rs;
                tekst = "Input mixer source line " + to_string(ind)
                    + " volume control found (no " + to_string(ind_2)
                    + ") / result: " + ss.str();
                DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
              }
              #endif
              break;
            }
          }
        }
      }
      else
      { // Card does not support global MIXER or MULTIPLEXER for input lines
        // Each of input lines is controlled separately
        #ifdef __DEBUG__
          DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second <<  "No input mixer/multiplexer device : processing source lines separately" << endl;
        #endif

        //Free memory
        MixerLinesWAVEIN.clear();

        // number of input lines
        Mixer_InputLinesNumber = MixerLineWAVEIN.cConnections;

        // get lines info MixerLineWAVEIN.cConnections
        MixerLinesWAVEIN.resize(Mixer_InputLinesNumber);
        MixerControlDetailsWAVEIN_LISTTEXT.resize(Mixer_InputLinesNumber);
        for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
        {
          //get WAVEIN source lines properties
          MixerLinesWAVEIN[ind].cbStruct=sizeof(MIXERLINE);
          MixerLinesWAVEIN[ind].dwDestination=MixerLineWAVEIN.dwDestination;
          MixerLinesWAVEIN[ind].dwSource=ind;
          // MixerLinesWAVEIN[ind].dwLineID=MixerControlDetailsWAVEIN_LISTTEXT[ind].dwParam1;
          rs=mixerGetLineInfo((HMIXEROBJ)hMixer_in, //the identifier of a waveform-audio input device in the range of zero to one less than the number of devices returned by the waveInGetNumDevs function
                              &(MixerLinesWAVEIN[ind]), MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER);
          #ifdef __DEBUG__
          {
            stringstream ss;
            ss << setfill ('0') << setw(8) << hex << rs;
            tekst = "Input source line " + to_string(ind) + " info read / result: " + ss.str();
            DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
          }
          #endif

          //Get input lines names and IDs //MixerControlDetails
          /*! \bug 2006_01_23 Sprawdzi�, czy MixerControlDetailsWAVEIN_LISTTEXT jest niezbdne
           *  (jest obs�ugiwane typowo tylko dla Mixer/Multiplexer) czy mo곿na zastpi
           *  polami ze struktury LineInfo
           */
          strncpy(MixerControlDetailsWAVEIN_LISTTEXT[ind].szName, MixerLinesWAVEIN[ind].szName, MIXER_LONG_NAME_CHARS);
          MixerControlDetailsWAVEIN_LISTTEXT[ind].szName[MIXER_LONG_NAME_CHARS-1] = 0;
          MixerControlDetailsWAVEIN_LISTTEXT[ind].dwParam1 = MixerLinesWAVEIN[ind].dwLineID;
        }


        // get volume & on/off controls
        //allocate memory for volume controls for each source line
        MixerControlsWAVEIN_VOLUME.resize(Mixer_InputLinesNumber);
        MixerControlsWAVEIN_VOLUME_supported.resize(Mixer_InputLinesNumber);
        MixerControlsWAVEIN_MUTE.resize(Mixer_InputLinesNumber);
        MixerControlsWAVEIN_MUTE_supported.resize(Mixer_InputLinesNumber);

        MixerLinesWAVEIN_MAXcChannels=0;
        for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
        {
          //get WAVEIN source lines properties
          if (MixerLinesWAVEIN_MAXcChannels<MixerLinesWAVEIN[ind].cChannels)
            MixerLinesWAVEIN_MAXcChannels=MixerLinesWAVEIN[ind].cChannels;

          //Get controls properties of particular WAVEIN line
          MixerControls.resize(MixerLinesWAVEIN[ind].cControls);
          MixerLineControl.cbStruct=sizeof(MIXERLINECONTROLS);
          MixerLineControl.dwLineID=MixerLinesWAVEIN[ind].dwLineID;
          MixerLineControl.cControls=MixerLinesWAVEIN[ind].cControls;
          MixerLineControl.pamxctrl=MixerControls.data();
          MixerLineControl.cbmxctrl=sizeof(MIXERCONTROL);
          rs=mixerGetLineControls((HMIXEROBJ)hMixer_in, //HMIXEROBJ hmxobj,
            &MixerLineControl, MIXER_GETLINECONTROLSF_ALL | MIXER_OBJECTF_HMIXER);
          #ifdef __DEBUG__
          {
            stringstream ss;
            ss << setfill ('0') << setw(8) << hex << rs;
            tekst = "Input mixer source line " + to_string(ind) + " controls info read / result: " + ss.str();
            DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
          }
          #endif
          //Find control MIXERCONTROL_CONTROLTYPE_VOLUME and copy it
          ZeroMemory(&(MixerControlsWAVEIN_VOLUME[ind]),sizeof(MIXERCONTROL));
          MixerControlsWAVEIN_VOLUME_supported[ind] = false;
          ZeroMemory(&(MixerControlsWAVEIN_MUTE[ind]),sizeof(MIXERCONTROL));
          MixerControlsWAVEIN_MUTE_supported[ind] = false;
          for (ind_2=0; ind_2<(int)MixerLinesWAVEIN[ind].cControls; ind_2++)
          {
            if (MixerControls[ind_2].dwControlType==MIXERCONTROL_CONTROLTYPE_VOLUME)
            {
              CopyMemory(&(MixerControlsWAVEIN_VOLUME[ind]),&(MixerControls[ind_2]),sizeof(MIXERCONTROL));
              MixerControlsWAVEIN_VOLUME_supported[ind] = true;
              #ifdef __DEBUG__
              {
                stringstream ss;
                ss << setfill ('0') << setw(8) << hex << rs;
                tekst = "Found input mixer source line " + to_string(ind)
                    + " volume control (no " + to_string(ind_2) + ") / result: " + ss.str();
                DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
              }
              #endif
              //break;
            }
      /*! Fixed 2006_01_23 Add support for MixerControlsWAVEIN_MUTE
       *  Fixed: 1) GetNumberOfSourceLines
       *  Fixed: 2) GetSourceLineName,
       *  Fixed: 3) GetSourceLineType,
       *  Fixed: 4) GetActiveSourceLine,
       *  Fixed: 5) SetActiveSourceLine,
       *  Fixed: 6a) MemorizeMixerSettings_WAVEIN,
       *  OK: 6b) ForgetMixerSettings_WAVEIN,
       *  Fixed: 6c) RestoreMixerSettings_WAVEIN
       *  OK: 7) GetSourceLineVolume,
       *  Fixed: 8) GetSourceLineState,
       *  OK: 9) SetActiveSourceLineVolume,
       *
       */
            if (MixerControls[ind_2].dwControlType==MIXERCONTROL_CONTROLTYPE_MUTE)
            {
              CopyMemory(&(MixerControlsWAVEIN_MUTE[ind]),&(MixerControls[ind_2]),sizeof(MIXERCONTROL));
              MixerControlsWAVEIN_MUTE_supported[ind] = true;
              #ifdef __DEBUG__
              {
                stringstream ss;
                ss << setfill ('0') << setw(8) << hex << rs;
                tekst = "Found input mixer source line " + to_string(ind)
                    + " mute control (no " + to_string(ind_2) + ") / result: " + ss.str();
                DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
              }
              #endif
              //break;
            }
          }
        }
      }

      //Free memory //Not needed any longer
      MixerControls.clear();

      PCMwaveFileActive=false;
      PCMwaveFileActiveValue=1.0;



      // ************************************************ //
      // ************************************************ //
      // Select destination where output is directed
      // Find first component of a type: MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
      // other not supported
      MixerLineOUT.cbStruct=sizeof(MIXERLINE);
      MixerSupportedOUT=false;
      for (ind = 0; ind < (int)MixerCaps_out.cDestinations; ind++)
      {
        MixerLineOUT.dwDestination=ind;
        rs=mixerGetLineInfo((HMIXEROBJ)hMixer_out, //the identifier of a waveform-audio input device in the range of zero to one less than the number of devices returned by the waveInGetNumDevs function
                            &MixerLineOUT, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_DESTINATION);
        #ifdef __DEBUG__
        {
          stringstream ss;
          ss << setfill ('0') << setw(8) << hex << rs;
          tekst = "Got output mixer device destination line " + to_string(ind) + " info / result: " + ss.str();
          DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
        }
        #endif

        if (MixerLineOUT.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)
        {
          MixerSupportedOUT=true;
          #ifdef __DEBUG__
            tekst = "Found output mixer device SPEAKER destination line " + to_string(ind);
            DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
          #endif
          break;
        }
        if (MixerLineOUT.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_HEADPHONES)
        {
          MixerSupportedOUT=true;
          #ifdef __DEBUG__
            tekst = "Found output mixer device HEADPHONES destination line " + to_string(ind) + " (looking still for SPEAKER)";
            DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
          #endif
          //break;
        }
      }

      // ************************************************ //
      // ************************************************ //
      // search for volume and mute controls
      if ( MixerSupportedOUT == true )
      {
        //Get controls properties of main output line
        MixerControls.resize(MixerLineOUT.cControls);
        MixerLineControl.cbStruct=sizeof(MIXERLINECONTROLS);
        MixerLineControl.dwLineID=MixerLineOUT.dwLineID;
        MixerLineControl.cControls=MixerLineOUT.cControls;
        MixerLineControl.pamxctrl=MixerControls.data();
        MixerLineControl.cbmxctrl=sizeof(MIXERCONTROL);
        rs=mixerGetLineControls((HMIXEROBJ)hMixer_out, //HMIXEROBJ hmxobj,
          &MixerLineControl, MIXER_GETLINECONTROLSF_ALL);
        #ifdef __DEBUG__
        {
          stringstream ss;
          ss << setfill ('0') << setw(8) << hex << rs;
          tekst = "Got controls of main output mixer device destination line / result: " + ss.str();
          DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
        }
        #endif

        // search for volume control
        MixerSupportedOUT = false;
        for (ind = 0; ind < (int) MixerLineControl.cControls; ind++)
        {
          if (MixerControls[ind].dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME)
          {
            CopyMemory(&MixerControlOUT_VOL,&(MixerControls[ind]),sizeof(MIXERCONTROL));
            MixerSupportedOUT = true;
            #ifdef __DEBUG__
            {
              DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second
                  << "Found main volume control (no " << ind
                  << ") of output mixer device destination line / result: "
                  << setfill ('0') << setw(8) << hex << rs << endl;
            }
            #endif
          }
        }

        if (MixerSupportedOUT == true)
        {
          MixerSupportedOUT = false;
          for (ind = 0; ind < (int)MixerLineControl.cControls; ind++)
          {
            if (MixerControls[ind].dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE)
            {
              CopyMemory(&MixerControlOUT_MUTE,&(MixerControls[ind]),sizeof(MIXERCONTROL));
              MixerSupportedOUT = true;
              #ifdef __DEBUG__
              {
                DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second
                  << "Found main mute control (no " << ind
                  << ") of output mixer device destination line / result: " << setfill ('0') << setw(8) << hex << rs << endl;
              }
              #endif
            }
          }
        }

        if (MixerSupportedOUT == true)
        {
          //Get attached lines
          //allocate memory for volume controls for each source line
          MixerLinesOUT.resize(MixerLineOUT.cConnections);

          //For each line get it's details and controls then find //MIXERCONTROL_CONTROLTYPE_VOLUME
          MixerLinesOUT_MAXcChannels=MixerLineOUT.cChannels;

          //allocate memory for volume controls for each source line
          MixerControlsOUT_VOL.resize(MixerLineOUT.cConnections);
          MixerControlsOUT_VOL_supported.resize(MixerLineOUT.cConnections);
          MixerControlsOUT_MUTE.resize(MixerLineOUT.cConnections);
          MixerControlsOUT_MUTE_supported.resize(MixerLineOUT.cConnections);


          for (ind=0; ind<(int)MixerLineOUT.cConnections; ind++)
          {
            //get OUTPUT source lines properties
            MixerLinesOUT[ind].cbStruct=sizeof(MIXERLINE);
            MixerLinesOUT[ind].dwDestination = MixerLineOUT.dwDestination;
            MixerLinesOUT[ind].dwSource=ind;
            rs=mixerGetLineInfo((HMIXEROBJ)hMixer_out, //the identifier of a waveform-audio input device in the range of zero to one less than the number of devices returned by the waveInGetNumDevs function
                                &(MixerLinesOUT[ind]), MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER);
            #ifdef __DEBUG__
            {
              stringstream ss;
              ss << setfill ('0') << setw(8) << hex << rs;
              tekst = "Got info of output source line (no " + to_string(ind)
                  + ") / result: " + ss.str();
              DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
            }
            #endif

            //Get controls properties of particular OUTPUT line
            MixerControls.resize(MixerLinesOUT[ind].cControls);
            MixerLineControl.cbStruct=sizeof(MIXERLINECONTROLS);
            MixerLineControl.dwLineID=MixerLinesOUT[ind].dwLineID;
            MixerLineControl.cControls=MixerLinesOUT[ind].cControls;
            MixerLineControl.pamxctrl=MixerControls.data();
            MixerLineControl.cbmxctrl=sizeof(MIXERCONTROL);
            rs=mixerGetLineControls((HMIXEROBJ)hMixer_out, //HMIXEROBJ hmxobj,
              &MixerLineControl, MIXER_GETLINECONTROLSF_ALL);
            #ifdef __DEBUG__
            {
              stringstream ss;
              ss << setfill ('0') << setw(8) << hex << rs;
              tekst = "Got controls of output source line (no " + to_string(ind)
                  + ") / result: " + ss.str();
              DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
            }
            #endif
            //Find control MIXERCONTROL_CONTROLTYPE_VOLUME and copy it
            ZeroMemory(&(MixerControlsOUT_VOL[ind]),sizeof(MIXERCONTROL));
            MixerControlsOUT_VOL_supported[ind] = false;
            ZeroMemory(&(MixerControlsOUT_MUTE[ind]),sizeof(MIXERCONTROL));
            MixerControlsOUT_MUTE_supported[ind] = false;
            for (ind_2=0; ind_2<(int)MixerLinesOUT[ind].cControls; ind_2++)
            {
              if (MixerControls[ind_2].dwControlType==MIXERCONTROL_CONTROLTYPE_VOLUME)
              {
                CopyMemory(&(MixerControlsOUT_VOL[ind]),&(MixerControls[ind_2]),sizeof(MIXERCONTROL));
                MixerControlsOUT_VOL_supported[ind] = true;
                #ifdef __DEBUG__
                {
                  stringstream ss;
                  ss << setfill ('0') << setw(8) << hex << rs;
                  tekst = "Found volume control (" + to_string(ind_2)
                      + ") of output source line (no " + to_string(ind)
                      + ") / result: " + ss.str();
                  DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
                }
                #endif
                //break;
              }
            }
            for (ind_2=0; ind_2<(int)MixerLinesOUT[ind].cControls; ind_2++)
            {
              if (MixerControls[ind_2].dwControlType==MIXERCONTROL_CONTROLTYPE_MUTE)
              {
                CopyMemory(&(MixerControlsOUT_MUTE[ind]),&(MixerControls[ind_2]),sizeof(MIXERCONTROL));
                MixerControlsOUT_MUTE_supported[ind] = true;
                #ifdef __DEBUG__
                {
                  stringstream ss;
                  ss << setfill ('0') << setw(8) << hex << rs;
                  tekst = "Found mute control (" + to_string(ind_2)
                      + ") of output source line (no " + to_string(ind)
                      + ") / result: " + ss.str();
                  DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << tekst << endl;
                }
                #endif
                //break;
              }
            }
            MixerControls.clear();
          }

        }



        // ************************************ //
        MixerControls.clear();

        // ****************************************************** //
        // Get names, types and control IDs  for separate ouput lines volume (and mute controls)
      }


      MixerSupported &= MixerSupportedOUT; //if MixerSupportedOUT false then MixerSupported also false

      //Free memory //Not needed any longer
      MixerControls.clear();
    }

  #else
    (void)WaveInDevNo; // unused
    (void)WaveOutDevNo; // unused
  
    MixerSupported = false;
    InputMixer_support = false;

    Input_MixerName[0] = 0;
    Output_MixerName[0] = 0;

    Mixer_InputLinesNumber = 0;

    //Memorized_ControlWAVEIN_BOOLEAN=NULL;
    //Memorized_ControlWAVEIN_UNSIGNED=NULL;
    Memorized_OUT_LinesStates.clear();
    Memorized_OUT_LinesVolumes.clear();

    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << "Not supported on this platform" << endl;
    #endif

  #endif

  #ifdef __DEBUG__
    DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << "Finished mixer configuration detection" << endl;
  #endif

  // ************************************************ //
  // ************************************************ //
  MixerSettingsMemorized_WAVEIN=false;
  MixerSettingsMemorized_OUT=false;

  //TEST
  #ifdef __DEBUG__
    DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << "MemorizeMixerSettings_WAVEIN" << endl;
  #endif
  MemorizeMixerSettings_WAVEIN();
  #ifdef __DEBUG__
    DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << "MemorizeMixerSettings_OUT" << endl;
  #endif
  MemorizeMixerSettings_OUT();
  #ifdef __DEBUG__
    DSP::log << "TAudioMixer::TAudioMixer" << DSP::e::LogMode::second << "Constructor finaly reached the end" << endl;
  #endif
}

TAudioMixer::~TAudioMixer(void)
{
  //TEST
  RestoreMixerSettings_WAVEIN();
  RestoreMixerSettings_OUT();

  #ifdef WIN32
    WaveInNumber=0;
    MixersNumber=0;
    rs=mixerClose(hMixer_in);
    rs=mixerClose(hMixer_out);

    MixerLinesWAVEIN.clear();
    MixerControlsWAVEIN_VOLUME.clear();
    MixerControlsWAVEIN_VOLUME_supported.clear();
    MixerControlsWAVEIN_MUTE.clear();
    MixerControlsWAVEIN_MUTE_supported.clear();
    MixerControlDetailsWAVEIN_LISTTEXT.clear();

    MixerLinesOUT.clear();
    MixerControlsOUT_VOL.clear();
    MixerControlsOUT_VOL_supported.clear();
    MixerControlsOUT_MUTE.clear();
    MixerControlsOUT_MUTE_supported.clear();

  #endif

  // ************************************ //
  ForgetMixerSettings_WAVEIN();
  ForgetMixerSettings_OUT();

  // ************************************ //
#ifdef WIN32
  WaveInCaps.clear();
  WaveInCaps_size = 0;

  WaveOutCaps.clear();
  WaveOutCaps_size = 0;
#endif // WIN32
  // ************************************ //
}

string TAudioMixer::GetMixerName(void)
{
  Input_Output_MixerName = Input_MixerName;
  //if (strcmp(Input_MixerName, Output_MixerName) != 0)
  if (Input_MixerName.compare(Output_MixerName) != 0)
  {
//    Input_Output_MixerName[strlen(Input_Output_MixerName)+1] = 0;
//    Input_Output_MixerName[strlen(Input_Output_MixerName)] = '/';
//    strcpy(Input_Output_MixerName+strlen(Input_Output_MixerName), Output_MixerName);
    Input_Output_MixerName += '/' + Output_MixerName;
  }

  return Input_Output_MixerName;
};

/*! Fixed <b>2006.01.23</b> Added support for soundcards without global MIXER or MULTIPLEXER for input lines
 */
void TAudioMixer::MemorizeMixerSettings_WAVEIN(void)
{
  #ifdef WIN32
    if (MixerSupported == false)
      return;

    int ind;
    MIXERCONTROLDETAILS MixerControlDetails;

    MixerSettingsMemorized_WAVEIN=true;

    //Master Line Controls
    Memorized_WAVEIN_MasterState[0]=GetSourceLineState(DSP::AM_MasterControl);
    Memorized_WAVEIN_MasterVolume[0]=GetSourceLineVolume(DSP::AM_MasterControl);

    // Other controls
    Memorized_ControlWAVEIN_BOOLEAN.clear();
    Memorized_ControlWAVEIN_BOOLEAN.resize(Mixer_InputLinesNumber);
    Memorized_ControlWAVEIN_UNSIGNED.clear();
    Memorized_ControlWAVEIN_UNSIGNED.resize(MixerLinesWAVEIN_MAXcChannels*Mixer_InputLinesNumber);

    if (InputMixer_support == true)
    { //Mixer/Multiplexer Controls
      //mixerGetControlDetails //MixerControlWAVEIN // cMultipleItems
      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlWAVEIN.dwControlID;
      MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=Mixer_InputLinesNumber;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      MixerControlDetails.paDetails=&(Memorized_ControlWAVEIN_BOOLEAN[0]);
      rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
    }
    else
    { //Input lines Controls
      for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
      {
        if (MixerControlsWAVEIN_MUTE_supported[ind] == true)
        {
          MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
          MixerControlDetails.dwControlID=MixerControlsWAVEIN_MUTE[ind].dwControlID;
          MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
          MixerControlDetails.cMultipleItems=0; // single line control
          MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
          MixerControlDetails.paDetails=&(Memorized_ControlWAVEIN_BOOLEAN[ind]);

          rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
        }
        else
        {
          Memorized_ControlWAVEIN_BOOLEAN[ind].fValue = false;
          rs = MIXERR_INVALCONTROL;
        }
      }
    }

    //mixerGetControlDetails / MIXER_GETCONTROLDETAILSF_VALUE
    for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
    {
      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      if (MixerControlsWAVEIN_VOLUME_supported[ind] == false)
        rs = MIXERR_INVALCONTROL;
      else
      {
        MixerControlDetails.dwControlID=MixerControlsWAVEIN_VOLUME[ind].dwControlID;
        MixerControlDetails.cChannels=MixerLinesWAVEIN[ind].cChannels; //one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=0; //MixerControlsWAVEIN[ind].cMultipleItems;
        MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
        MixerControlDetails.paDetails=&(Memorized_ControlWAVEIN_UNSIGNED[MixerLinesWAVEIN_MAXcChannels*ind]);
        rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
      }
    }

  #else

    MixerSettingsMemorized_WAVEIN=false;

    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "TAudioMixer::MemorizeMixerSettings_WAVEIN" << DSP::e::LogMode::second << "Not yet implemented on this platform" << endl;
    #endif

  #endif
}

void TAudioMixer::ForgetMixerSettings_WAVEIN(void)
{
  MixerSettingsMemorized_WAVEIN=false;

  #ifdef WIN32
    Memorized_ControlWAVEIN_BOOLEAN.clear();
    Memorized_ControlWAVEIN_UNSIGNED.clear();
  #else

    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "TAudioMixer::ForgetMixerSettings_WAVEIN" << DSP::e::LogMode::second << "Not yet implemented on this platform" << endl;
    #endif

  #endif
}

/*! Fixed <b>2006.01.23</b> MixerControlDetailsWAVEIN_LISTTEXT was allocated (overriding previous allocation without freeing it) but it should not
 *
 * Fixed <b>2006.01.23</b> Added support for soundcards without global MIXER or MULTIPLEXER for input lines
 */
void TAudioMixer::RestoreMixerSettings_WAVEIN(void)
{
  #ifdef WIN32
    int ind;
    MIXERCONTROLDETAILS MixerControlDetails;

    if (MixerSettingsMemorized_WAVEIN == true)
    {
      //Master Line Controls
      SetSourceLineState(-1, Memorized_WAVEIN_MasterState[0]);
      SetSourceLineVolume(-1, Memorized_WAVEIN_MasterVolume[0]);

      if (InputMixer_support == true)
      { //Mixer/Multiplexer Controls
        //mixerGetControlDetails //MixerControlWAVEIN // cMultipleItems
        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
        MixerControlDetails.dwControlID=MixerControlWAVEIN.dwControlID;
        MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=Mixer_InputLinesNumber;
        MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    //    MixerControlDetailsWAVEIN_LISTTEXT=new MIXERCONTROLDETAILS_LISTTEXT[MixerControlWAVEIN.cMultipleItems];
        MixerControlDetails.paDetails=&(Memorized_ControlWAVEIN_BOOLEAN[0]); //???
        rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      }
      else
      { //Input lines Controls
        for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
        {
          if (MixerControlsWAVEIN_MUTE_supported[ind] == true)
          {
            MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
            MixerControlDetails.dwControlID=MixerControlsWAVEIN_MUTE[ind].dwControlID;
            MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
            MixerControlDetails.cMultipleItems=0; // single line control
            MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
            MixerControlDetails.paDetails=&(Memorized_ControlWAVEIN_BOOLEAN[ind]);

            rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
          }
          else
          {
            Memorized_ControlWAVEIN_BOOLEAN[ind].fValue = false;
            rs = MIXERR_INVALCONTROL;
          }
        }
      }


      //mixerGetControlDetails / MIXER_GETCONTROLDETAILSF_VALUE
      for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
      {
        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
        if (MixerControlsWAVEIN_VOLUME_supported[ind] == false)
          rs = MIXERR_INVALCONTROL;
        else
        {
          MixerControlDetails.dwControlID=MixerControlsWAVEIN_VOLUME[ind].dwControlID;
          MixerControlDetails.cChannels=MixerLinesWAVEIN[ind].cChannels; //one mixer ON/OFF control per channel
          MixerControlDetails.cMultipleItems=0; //MixerControlsWAVEIN[ind].cMultipleItems;
          MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
          MixerControlDetails.paDetails=&(Memorized_ControlWAVEIN_UNSIGNED[MixerLinesWAVEIN_MAXcChannels*ind]);
          rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
        }
      }
    }

  #else

    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "TAudioMixer::RestoreMixerSettings_WAVEIN" << DSP::e::LogMode::second << "Not yet implemented on this platform" << endl;
    #endif

  #endif
}

void TAudioMixer::MemorizeMixerSettings_OUT(void)
{
  int ind, ile;

  MixerSettingsMemorized_OUT=true;

  //Master Line Controls
#ifdef __DEBUG__
  DSP::log << "TAudioMixer::MemorizeMixerSettings_OUT" << DSP::e::LogMode::second << "Saving Master Line Controls State" << endl;
#endif
  Memorized_OUT_MasterState[0]=GetDestLineState(DSP::AM_MasterControl, 0);
  Memorized_OUT_MasterState[1]=GetDestLineState(DSP::AM_MasterControl, 1);
#ifdef __DEBUG__
  DSP::log << "TAudioMixer::MemorizeMixerSettings_OUT" << DSP::e::LogMode::second << "Saving Master Line Controls Volume" << endl;
#endif
  Memorized_OUT_MasterVolume[0]=GetDestLineVolume(DSP::AM_MasterControl, 0);
  Memorized_OUT_MasterVolume[1]=GetDestLineVolume(DSP::AM_MasterControl, 1);

  //Master Line connections Controls
#ifdef __DEBUG__
  DSP::log << "TAudioMixer::MemorizeMixerSettings_OUT" << DSP::e::LogMode::second << "Saving Master Line connections Controls" << endl;
#endif
  ile = GetNumberOfDestLines();
#ifdef __DEBUG__
  {
    DSP::log << "TAudioMixer::MemorizeMixerSettings_OUT" << DSP::e::LogMode::second << "GetNumberOfDestLines() = " << ile << endl;
  }
#endif
  Memorized_OUT_LinesStates.clear();
  Memorized_OUT_LinesStates.resize(2*ile);
  Memorized_OUT_LinesVolumes.clear();
  Memorized_OUT_LinesVolumes.resize(2*ile);

  for (ind=0; ind<ile; ind++)
  {
#ifdef __DEBUG__
    {
      DSP::log << "TAudioMixer::MemorizeMixerSettings_OUT" << DSP::e::LogMode::second << "Saving line #" << ind << " State" << endl;
    }
#endif
    Memorized_OUT_LinesStates[ind*2+0]=GetDestLineState(ind, 0);
    Memorized_OUT_LinesStates[ind*2+1]=GetDestLineState(ind, 1);
#ifdef __DEBUG__
    {
      DSP::log << "TAudioMixer::MemorizeMixerSettings_OUT" << DSP::e::LogMode::second << "Saving line #" << ind << " Volume" << endl;
    }
#endif
    Memorized_OUT_LinesVolumes[ind*2+0]=GetDestLineVolume(ind, 0);
    Memorized_OUT_LinesVolumes[ind*2+1]=GetDestLineVolume(ind, 1);
  }
#ifdef __DEBUG__
  DSP::log << "TAudioMixer::MemorizeMixerSettings_OUT" << DSP::e::LogMode::second << "Finished Saving Master Line connections Controls" << endl;
#endif
}

void TAudioMixer::ForgetMixerSettings_OUT(void)
{
  MixerSettingsMemorized_OUT=false;

  Memorized_OUT_LinesStates.clear();
  Memorized_OUT_LinesVolumes.clear();
}


string TAudioMixer::GetSourceLineName(int ind)
{
  string LineName;

  LineName= "";

  #ifdef WIN32
    if (ind == DSP::AM_MasterControl)
      LineName=MixerLineWAVEIN.szName;
    else
      if (MixerControlDetailsWAVEIN_LISTTEXT.size() > 0)
  	    if ((ind>=0) && (ind<(int)Mixer_InputLinesNumber))
    	    LineName=MixerControlDetailsWAVEIN_LISTTEXT[ind].szName;
  #endif

  if (ind == DSP::AM_PCMwaveFile)
  	LineName = PCMwaveFileName;

  return LineName;
}

string TAudioMixer::GetDestLineName(int ind)
{
  string LineName;

  LineName= "";

  #ifdef WIN32
    if (MixerLinesOUT.size() > 0)
    {
      if (ind == DSP::AM_MasterControl)
        LineName=MixerLineOUT.szName;
      else
        if ((ind>=0) && (ind<(int)MixerLineOUT.cConnections))
          LineName=MixerLinesOUT[ind].szName;
    }
  #else
    (void)ind; // unused
  #endif

  return LineName;
}

void TAudioMixer::RestoreMixerSettings_OUT(void)
{
  int ind, ile;

  if (MixerSettingsMemorized_OUT == false)
    return;

  //Master Line Controls
  if (Memorized_OUT_MasterState[1] == DSP::e::AM_MutedState::MUTED_INACTIVE)
  {
    SetDestLineState(-1, Memorized_OUT_MasterState[0]);
    SetDestLineVolume(-1, Memorized_OUT_MasterVolume[0]);
  }
  else
  {
    SetDestLineState(-1, Memorized_OUT_MasterState[0], Memorized_OUT_MasterState[1]);
    SetDestLineVolume(-1, Memorized_OUT_MasterVolume[0], Memorized_OUT_MasterVolume[1]);
  }

  //Master Line connections Controls
  ile = GetNumberOfDestLines();
  if (Memorized_OUT_LinesStates.size() > 0)
    for (ind=0; ind<ile; ind++)
    {
      if (Memorized_OUT_LinesStates[ind*2+1] == DSP::e::AM_MutedState::MUTED_INACTIVE)
      {
        SetDestLineState(ind, Memorized_OUT_LinesStates[ind*2+0]);
      }
      else
      {
        SetDestLineState(ind, Memorized_OUT_LinesStates[ind*2+0], Memorized_OUT_LinesStates[ind*2+1]);
      }
    }

  if (Memorized_OUT_LinesVolumes.size() > 0)
    for (ind=0; ind<ile; ind++)
    {
      if (Memorized_OUT_LinesVolumes[ind*2+1] < 0)
      {
        SetDestLineVolume(ind, Memorized_OUT_LinesVolumes[ind*2+0]);
      }
      else
      {
        SetDestLineVolume(ind, Memorized_OUT_LinesVolumes[ind*2+0], Memorized_OUT_LinesVolumes[ind*2+1]);
      }
    }

}


DWORD TAudioMixer::GetSourceLineType(int ind)
{
  DWORD LineType;

  LineType = 0xffffffff;

  #ifdef WIN32
    if (ind == DSP::AM_MasterControl)
      LineType=MixerLineWAVEIN.dwComponentType;
    else
      if (MixerLinesWAVEIN.size() > 0)
        if ((ind>=0) && (ind<(int)Mixer_InputLinesNumber))
          LineType=MixerLinesWAVEIN[ind].dwComponentType;
  #else
    (void)ind; // unused
  #endif

  //  if (ind==AM_PCMwaveFile)
  //    LineType = PCMwaveFileName;

  return LineType;
}

DWORD TAudioMixer::GetDestLineType(int ind)
{
  DWORD LineType;

  LineType = 0xffffffff;

  #ifdef WIN32
    if (MixerLinesOUT.size() > 0)
      if ((ind>=0) && (ind<(int)MixerLineOUT.cConnections))
        LineType=MixerLinesOUT[ind].dwComponentType;
  #else
    (void)ind; // unused
  #endif

  return LineType;
}


int TAudioMixer::GetNumberOfSourceLines(void)
{
//  return MixerControlWAVEIN.cMultipleItems;
  return Mixer_InputLinesNumber;
}

int TAudioMixer::GetNumberOfDestLines(void)
{
  #ifdef WIN32
    return MixerLineOUT.cConnections;

  #else
    return 0;

  #endif
}

/*! Fixed <b>2006.01.23</b> Added support for soundcards without global MIXER or MULTIPLEXER for input lines
 * Fixed <b>2006.01.23</b> reversed status in input lines controls mode: converting mute into line activity
 */
int TAudioMixer::GetActiveSourceLine(void)
{
  if (PCMwaveFileActive)
  	return DSP::AM_PCMwaveFile;

  #ifdef WIN32
    int ind;
    MIXERCONTROLDETAILS MixerControlDetails;

    vector<MIXERCONTROLDETAILS_BOOLEAN> temp(Mixer_InputLinesNumber);

    //We should reread lines state
    if (InputMixer_support == true)
    { //Mixer/Multiplexer Controls
      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlWAVEIN.dwControlID;
      MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=Mixer_InputLinesNumber;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      MixerControlDetails.paDetails=temp.data();
      rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);

      for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
        if (temp[ind].fValue)
          break;
    }
    else
    { //Input lines Controls
      for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
      {
        if (MixerControlsWAVEIN_MUTE_supported[ind] == true)
        {
          MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
          MixerControlDetails.dwControlID= MixerControlsWAVEIN_MUTE[ind].dwControlID; //??? MixerControlWAVEIN.dwControlID;
          MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
          MixerControlDetails.cMultipleItems=0; //single item
          MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
          MixerControlDetails.paDetails=temp.data()+ind;
          rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
        }
        else
        {
          temp[ind].fValue = false;
          rs = MIXERR_INVALCONTROL;
        }
      }

      // line is active if mute is off
      for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
        if (temp[ind].fValue == 0)
          break;
    }

    temp.clear();

    if (ind==(int)Mixer_InputLinesNumber)
      ind=-1;
    return ind;

  #else

    return -1;

  #endif
}

/*! Fixed <b>2006.01.23</b> Added support for soundcards without global MIXER or MULTIPLEXER for input lines
 *
 * Fixed <b>2006.01.23</b> reversing status in input lines controls mode: converting mute into line activity
 * \bug <b>2007.10.25</b> It is not necessary to read all line states in input lines controls mode
 */
bool TAudioMixer::GetSourceLineState(int LineNo)
{
  if (LineNo == DSP::AM_PCMwaveFile)
    return PCMwaveFileActive;

  #ifdef WIN32
    int ind;
    MIXERCONTROLDETAILS MixerControlDetails;
    std::vector<MIXERCONTROLDETAILS_BOOLEAN> temp;
    bool result;

    // Master control
    if (LineNo == DSP::AM_MasterControl)
    {
      temp.resize(1);

      if (MixerControlWAVEIN_MUTE_supported == true)
      {
        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
        MixerControlDetails.dwControlID=MixerControlWAVEIN_MUTE.dwControlID;
        MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=0; // single line control
        MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
        MixerControlDetails.paDetails=temp.data();

        rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);

        // reverse status: mute into line activity
        if (temp[0].fValue > 0)
          temp[0].fValue = 0;
        else
          temp[0].fValue = 1;
      }
      else
      {
        temp[0].fValue = 0;
        rs = MIXERR_INVALCONTROL;
      }

      result = (temp[0].fValue > 0);
      return result;
    }


    // other lines
    temp.resize(Mixer_InputLinesNumber);

    //We should reread lines state
    if (InputMixer_support == true)
    { //Mixer/Multiplexer Controls
      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlWAVEIN.dwControlID;
      MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=Mixer_InputLinesNumber;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      MixerControlDetails.paDetails=temp.data();
      rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
    }
    else
    { //Input lines Controls
      for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
      {
        if (MixerControlsWAVEIN_MUTE_supported[ind] == true)
        {
          MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
          MixerControlDetails.dwControlID=MixerControlsWAVEIN_MUTE[ind].dwControlID;
          MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
          MixerControlDetails.cMultipleItems=0; // single line control
          MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
          MixerControlDetails.paDetails=&(temp[ind]);

          rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);

          // reverse status: mute into line activity
          if (temp[LineNo].fValue > 0)
            temp[LineNo].fValue = 0;
          else
            temp[LineNo].fValue = 1;
        }
        else
        {
          temp[ind].fValue = 0;
          rs = MIXERR_INVALCONTROL;
        }
      }
    }

    if ((LineNo < 0) || (LineNo >= (int)Mixer_InputLinesNumber))
    {
      return false;
    }

    result = (temp[LineNo].fValue > 0);
    return result;

  #else
    (void)LineNo; // unused

    return false;

  #endif
}

DSP::e::AM_MutedState TAudioMixer::GetDestLineState(int LineNo, int Channel)
{
//  if (PCMwaveFileActive)
//    return AM_PCMwaveFile;

  #ifdef WIN32
    int ile;
    MIXERCONTROLDETAILS MixerControlDetails;
    std::vector<MIXERCONTROLDETAILS_BOOLEAN> temp;
    DSP::e::AM_MutedState left, right, any_channel;

    if (MixerSupportedOUT == false)
      return DSP::e::AM_MutedState::MUTED_INACTIVE; // no output mixer device

    if (LineNo==DSP::AM_MasterControl)
    {
      ile = MixerLineOUT.cChannels;

      temp.resize(ile);

      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlOUT_MUTE.dwControlID;
      MixerControlDetails.cChannels=ile;
      MixerControlDetails.cMultipleItems=0;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      MixerControlDetails.paDetails=temp.data();
      rs=mixerGetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
      if (rs != MMSYSERR_NOERROR)
      {
        MixerControlDetails.cChannels--;
        ile--;
        rs=mixerGetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
      }

      switch (ile)
      {
        case 1:
          if (temp[0].fValue==0)
            left  = DSP::e::AM_MutedState::MUTED_NO;
          else
            left  = DSP::e::AM_MutedState::MUTED_YES;
          right = DSP::e::AM_MutedState::MUTED_INACTIVE;
          break;
        case 2:
          if (temp[0].fValue==0)
            left  = DSP::e::AM_MutedState::MUTED_NO;
          else
            left  = DSP::e::AM_MutedState::MUTED_YES;
          if (temp[1].fValue==0)
            right  = DSP::e::AM_MutedState::MUTED_NO;
          else
            right = DSP::e::AM_MutedState::MUTED_YES;
          break;
        default:
          left = DSP::e::AM_MutedState::MUTED_INACTIVE;
          right = DSP::e::AM_MutedState::MUTED_INACTIVE;
          break;
      }
      any_channel = left;
      if (right == DSP::e::AM_MutedState::MUTED_NO)
        any_channel = DSP::e::AM_MutedState::MUTED_NO;

      temp.clear();

      switch (Channel)
      {
        case -1:
          return any_channel;
        case 0:
          return left;
        case 1:
          return right;
        default:
          return DSP::e::AM_MutedState::MUTED_INACTIVE;
      }
    }

    ile = GetNumberOfDestLines();
    if ((LineNo >= ile) || (LineNo < 0))
      return DSP::e::AM_MutedState::MUTED_INACTIVE;

    if (MixerControlsOUT_MUTE_supported[LineNo] == true)
    {
      ile = MixerControlsOUT_MUTE[LineNo].cMultipleItems;
      if (ile <= 0)
        ile = 1;
      ile *= MixerLinesOUT[LineNo].cChannels;
      temp.resize(ile);

      //Otczytanie stanu kontrolki
      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlsOUT_MUTE[LineNo].dwControlID;
      MixerControlDetails.cChannels=MixerLinesOUT[LineNo].cChannels;
      //If you stuff here too much rs==11
      MixerControlDetails.cMultipleItems=MixerControlsOUT_MUTE[LineNo].cMultipleItems;

      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      MixerControlDetails.paDetails=temp.data();
      rs=mixerGetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
      if (rs != MMSYSERR_NOERROR)
      {
        MixerControlDetails.cChannels--;
        rs=mixerGetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
      }

      if (MixerControlDetails.cMultipleItems<2)
        MixerControlDetails.cMultipleItems=1;
      ile = MixerControlDetails.cMultipleItems*MixerControlDetails.cChannels;
      switch (ile)
      {
        case 1:
          if (temp[0].fValue==0) {
            left  = DSP::e::AM_MutedState::MUTED_NO;
          }
          else {
            left  = DSP::e::AM_MutedState::MUTED_YES;
          }
          right = DSP::e::AM_MutedState::MUTED_INACTIVE; 
          break;
        case 2:
          if (temp[0].fValue==0)
            left  = DSP::e::AM_MutedState::MUTED_NO;
          else
            left = DSP::e::AM_MutedState::MUTED_YES;
          if (temp[1].fValue==0)
            right  = DSP::e::AM_MutedState::MUTED_NO;
          else
            right = DSP::e::AM_MutedState::MUTED_YES;
          break;
        default:
          left = DSP::e::AM_MutedState::MUTED_INACTIVE;
          right = DSP::e::AM_MutedState::MUTED_INACTIVE;
          break;
      }
      any_channel = left;
      if (right == DSP::e::AM_MutedState::MUTED_NO)
        any_channel = DSP::e::AM_MutedState::MUTED_NO;

      // free temp
      temp.clear();
    }
    else // if (MixerControlsOUT_MUTE_supported[LineNo] == true)
    {
      Channel = -1;
      any_channel = DSP::e::AM_MutedState::MUTED_INACTIVE;
    }


    switch (Channel)
    {
      case -1:
        return any_channel;
      case 0:
        return left;
      case 1:
        return right;
      default:
        return DSP::e::AM_MutedState::MUTED_INACTIVE;
    }

  #else
    (void)LineNo; // unused
    (void)Channel; // unused

    return DSP::e::AM_MutedState::MUTED_INACTIVE;

  #endif
}

void TAudioMixer::SetSourceLineState(int LineNo, bool IsActive)
{
  if (LineNo == DSP::AM_PCMwaveFile)
  {
    PCMwaveFileActive = IsActive;
    return;
  }

  #ifdef WIN32
    MIXERCONTROLDETAILS MixerControlDetails;
    std::vector<MIXERCONTROLDETAILS_BOOLEAN> temp;

    temp.resize(Mixer_InputLinesNumber);

    if (LineNo == DSP::AM_MasterControl)
    {
      // ************************************ //
      if (MixerControlWAVEIN_MUTE_supported == true)
      { // master mute control
        if (IsActive)
          temp[0].fValue=0; // not muted
        else
          temp[0].fValue=1; // muted

        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
        MixerControlDetails.dwControlID=MixerControlWAVEIN_MUTE.dwControlID;
        MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=0; // single line control
        MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
        MixerControlDetails.paDetails=temp.data();

        rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      }
      return;
    }

    if ((LineNo >= 0) || (LineNo < (int)Mixer_InputLinesNumber))
    {
      // ************************************ //
      //We should write lines state
      if (InputMixer_support == true)
      { //Mixer/Multiplexer Controls
        //for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
        //  temp[ind].fValue = GetSourceLineState(ind);
        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
        MixerControlDetails.dwControlID=MixerControlWAVEIN.dwControlID;
        MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=Mixer_InputLinesNumber;
        MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
        MixerControlDetails.paDetails=temp.data();
        rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);

        if (IsActive == true)
          temp[LineNo].fValue=1; // selected
        else
          temp[LineNo].fValue=0; // not selected

        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
        MixerControlDetails.dwControlID=MixerControlWAVEIN.dwControlID;
        MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=Mixer_InputLinesNumber;
        MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
        MixerControlDetails.paDetails=temp.data();

        rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      }
      else
      { //Input lines Controls
        // line is active if mute is off
        if (IsActive == true)
          temp[0].fValue=0; // muted
        else
          temp[0].fValue=1; // not muted

        if (MixerControlsWAVEIN_MUTE_supported[LineNo] == true)
        {
          MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
          MixerControlDetails.dwControlID=MixerControlsWAVEIN_MUTE[LineNo].dwControlID;
          MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
          MixerControlDetails.cMultipleItems=0; // single line control
          MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
          MixerControlDetails.paDetails=temp.data();

          rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
        }
      }
    }

    temp.clear();

  #else

  #endif

  return;
}

/*! Fixed <b>2006.01.23</b> Added support for soundcards without global MIXER or MULTIPLEXER for input lines
  * Fixed <b>2006.01.23</b> reversed status in input lines controls mode: converting mute into line activity
 */
void TAudioMixer::SetActiveSourceLine(int ActiveNo)
{
  if (ActiveNo==DSP::AM_PCMwaveFileON)
  {
    PCMwaveFileActive=true;
    return;
  }
  if (ActiveNo==DSP::AM_PCMwaveFileOFF)
  {
    PCMwaveFileActive=false;
    ActiveNo=GetActiveSourceLine();
  }

  #ifdef WIN32
    int ind;
    MIXERCONTROLDETAILS MixerControlDetails;
    std::vector<MIXERCONTROLDETAILS_BOOLEAN> temp;

    temp.resize(Mixer_InputLinesNumber);

    // ************************************ //
    if (MixerControlWAVEIN_MUTE_supported == true)
    { // just in case turn off master mute control
      temp[0].fValue=0;

      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlWAVEIN_MUTE.dwControlID;
      MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=0; // single line control
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      MixerControlDetails.paDetails=temp.data();

      rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
    }

    // ************************************ //
    //We should write lines state
    if (InputMixer_support == true)
    { //Mixer/Multiplexer Controls
      for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
        if (ind==ActiveNo)
          temp[ind].fValue=1;
        else
          temp[ind].fValue=0;

      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlWAVEIN.dwControlID;
      MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=Mixer_InputLinesNumber;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      MixerControlDetails.paDetails=temp.data();

      rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
    }
    else
    { //Input lines Controls
      // line is active if mute is off
      for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
        if (ind==ActiveNo)
          temp[ind].fValue=0;
       else
          temp[ind].fValue=1;

      for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
      {
        if (MixerControlsWAVEIN_MUTE_supported[ind] == true)
        {
          MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
          MixerControlDetails.dwControlID=MixerControlsWAVEIN_MUTE[ind].dwControlID;
          MixerControlDetails.cChannels=1; //one mixer ON/OFF control per channel
          MixerControlDetails.cMultipleItems=0; // single line control
          MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
          MixerControlDetails.paDetails=&(temp[ind]);

          rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
        }
        else
        {
          temp[ind].fValue = false;
          rs = MIXERR_INVALCONTROL;
        }
      }
    }

    temp.clear();

  #else

  #endif

  return;
}

/*! Fixed <b>2006.01.23</b> Added support for soundcards without global MIXER or MULTIPLEXER for input lines
 */
void TAudioMixer::SetActiveSourceLine(string ActiveName)
{
  // if (strcmp("PCMwave file",ActiveName)==0)
  if (ActiveName.compare("PCMwave file")==0)
  {
  	PCMwaveFileActive=true;
  	return;
  }

  #ifdef WIN32
    int ind;

    for (ind=0; ind<(int)Mixer_InputLinesNumber; ind++)
      //if (strcmp(MixerControlDetailsWAVEIN_LISTTEXT[ind].szName,ActiveName)==0)
      if (ActiveName.compare(MixerControlDetailsWAVEIN_LISTTEXT[ind].szName)==0)
      {
        SetActiveSourceLine(ind);
        break;
      }
  #else

  #endif

  return;
}

bool TAudioMixer::SetActiveSourceLineVolume(double Vol)
{
  int Active;

  Active=GetActiveSourceLine();
  if (Active == DSP::AM_MasterControl)
    return false;
  if (Active==DSP::AM_PCMwaveFile)
  {
    PCMwaveFileActiveValue=Vol;
    return true;
  }

  #ifdef WIN32
    MIXERCONTROLDETAILS MixerControlDetails;
    MIXERCONTROLDETAILS_UNSIGNED tmp_UNSIGNED;
    DWORD output;

    MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    if (MixerControlsWAVEIN_VOLUME_supported[Active] == false)
    {
      rs = MIXERR_INVALCONTROL;
      return false;
    }
    else
    {
      MixerControlDetails.dwControlID=MixerControlsWAVEIN_VOLUME[Active].dwControlID;
      MixerControlDetails.cChannels=1; //WaveIn_cChannels; one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=0;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
      MixerControlDetails.paDetails=&(tmp_UNSIGNED);

      output=(DWORD)(Vol*(MixerControlsWAVEIN_VOLUME[Active].Bounds.dwMaximum -   //union
                          MixerControlsWAVEIN_VOLUME[Active].Bounds.dwMinimum));
      output+=MixerControlsWAVEIN_VOLUME[Active].Bounds.dwMinimum;
      tmp_UNSIGNED.dwValue=output;

      rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      return true;
    }
  #else
    return false;

  #endif
}

bool TAudioMixer::SetSourceLineVolume(int LineNo, double Vol)
{
  if (LineNo==DSP::AM_PCMwaveFile)
  {
    PCMwaveFileActiveValue=Vol;
    return true;
  }

  #ifdef WIN32
    MIXERCONTROLDETAILS MixerControlDetails;
    MIXERCONTROLDETAILS_UNSIGNED tmp_UNSIGNED;
    DWORD output;

    if (LineNo==DSP::AM_MasterControl)
    {
      if (MixerControlWAVEIN_VOLUME_supported == false)
        return false;
      else
      {
        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);

        MixerControlDetails.dwControlID=MixerControlWAVEIN_VOLUME.dwControlID;
        MixerControlDetails.cChannels=1; //WaveIn_cChannels; one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=0;
        MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
        MixerControlDetails.paDetails=&(tmp_UNSIGNED);

        output=(DWORD)(Vol*(MixerControlWAVEIN_VOLUME.Bounds.dwMaximum -   //union
                            MixerControlWAVEIN_VOLUME.Bounds.dwMinimum));
        output+=MixerControlWAVEIN_VOLUME.Bounds.dwMinimum;
        tmp_UNSIGNED.dwValue=output;

        rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      }
      return true;
    }

    if ((LineNo<0) || (LineNo >= Mixer_InputLinesNumber))
      return false; // Wrong input parameter

    MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    if (MixerControlsWAVEIN_VOLUME_supported[LineNo] == false)
    {
      rs = MIXERR_INVALCONTROL;
      return false;
    }
    else
    {
      MixerControlDetails.dwControlID=MixerControlsWAVEIN_VOLUME[LineNo].dwControlID;
      MixerControlDetails.cChannels=1; //WaveIn_cChannels; one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=0;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
      MixerControlDetails.paDetails=&(tmp_UNSIGNED);

      output=(DWORD)(Vol*(MixerControlsWAVEIN_VOLUME[LineNo].Bounds.dwMaximum -   //union
                          MixerControlsWAVEIN_VOLUME[LineNo].Bounds.dwMinimum));
      output+=MixerControlsWAVEIN_VOLUME[LineNo].Bounds.dwMinimum;
      tmp_UNSIGNED.dwValue=output;

      rs=mixerSetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      return true;
    }
  #else
    return false;

  #endif
}

/*! Fixed <b>2006.01.23</b> returns false if no volume control is supported
 */
bool TAudioMixer::SetDestLineVolume(int LineNo, double Vol_Left, double Vol_Right)
{
  if (Vol_Left < 0)
    return false;

  if (Vol_Right < 0)
    Vol_Right = Vol_Left;

  #ifdef WIN32
    MIXERCONTROLDETAILS MixerControlDetails;
    std::vector<MIXERCONTROLDETAILS_UNSIGNED> tmp_UNSIGNED;
  //  int Active;
    DWORD output_L, output_R;
    int ind, ile;

    if (LineNo==DSP::AM_MasterControl)
    {
      ile = MixerLineOUT.cChannels;

      tmp_UNSIGNED.resize(ile);

      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlOUT_VOL.dwControlID;
      MixerControlDetails.cChannels=ile; //WaveIn_cChannels; one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=0;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
      MixerControlDetails.paDetails=tmp_UNSIGNED.data();

      output_L=(DWORD)(Vol_Left* (MixerControlOUT_VOL.Bounds.dwMaximum -   //union
                                  MixerControlOUT_VOL.Bounds.dwMinimum));
      output_R=(DWORD)(Vol_Right*(MixerControlOUT_VOL.Bounds.dwMaximum -   //union
                                  MixerControlOUT_VOL.Bounds.dwMinimum));
      output_L+=MixerControlOUT_VOL.Bounds.dwMinimum;
      output_R+=MixerControlOUT_VOL.Bounds.dwMinimum;
      for (ind=0; ind < ile; ind++)
      {
        switch (ind)
        {
          case 0:
            tmp_UNSIGNED[ind].dwValue=output_L;
            break;
          case 1:
          default:
            tmp_UNSIGNED[ind].dwValue=output_R;
            break;
        }
      }

      rs=mixerSetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      if (rs != MMSYSERR_NOERROR)
      {
        MixerControlDetails.cChannels--;
        rs=mixerSetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      }
      return true;
    }

    ile = GetNumberOfDestLines();
    if ((LineNo >= ile) || (LineNo < 0))
      return false;

    if (MixerLinesOUT[LineNo].cControls == 0)
      return false; // NO CONTROLS

    if (MixerControlsOUT_VOL_supported[LineNo] == false)
      return false; // NO VOLUME CONTROL

    ile =MixerLinesOUT[LineNo].cChannels;
  //  if (Channel >= ile)
  //    return -1.0;

    tmp_UNSIGNED.resize(ile);

    MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    MixerControlDetails.dwControlID=MixerControlsOUT_VOL[LineNo].dwControlID;
    MixerControlDetails.cChannels=ile; //WaveIn_cChannels; one mixer ON/OFF control per channel
    MixerControlDetails.cMultipleItems=0;
    MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
    MixerControlDetails.paDetails=tmp_UNSIGNED.data();

    output_L=(DWORD)(Vol_Left* (MixerControlsOUT_VOL[LineNo].Bounds.dwMaximum -   //union
                                MixerControlsOUT_VOL[LineNo].Bounds.dwMinimum));
    output_R=(DWORD)(Vol_Right*(MixerControlsOUT_VOL[LineNo].Bounds.dwMaximum -   //union
                                MixerControlsOUT_VOL[LineNo].Bounds.dwMinimum));
    output_L+=MixerControlsOUT_VOL[LineNo].Bounds.dwMinimum;
    output_R+=MixerControlsOUT_VOL[LineNo].Bounds.dwMinimum;
    for (ind=0; ind < ile; ind++)
    {
      switch (ind)
      {
        case 0:
          tmp_UNSIGNED[ind].dwValue=output_L;
          break;
        case 1:
        default:
          tmp_UNSIGNED[ind].dwValue=output_R;
          break;
      }
    }

    rs=mixerSetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
    if (rs != MMSYSERR_NOERROR)
    {
      MixerControlDetails.cChannels--;
      rs=mixerSetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
    }

    return true;

  #else
    (void)LineNo; // unused

    return false;

  #endif
}

bool TAudioMixer::SetDestLineState(int LineNo, DSP::e::AM_MutedState IsMuted_Left, DSP::e::AM_MutedState IsMuted_Right)
{
  if (IsMuted_Left == DSP::e::AM_MutedState::MUTED_INACTIVE)
    return false;

  if (IsMuted_Right == DSP::e::AM_MutedState::MUTED_INACTIVE)
    IsMuted_Right = IsMuted_Left;

  #ifdef WIN32
    MIXERCONTROLDETAILS MixerControlDetails;
    std::vector<MIXERCONTROLDETAILS_BOOLEAN> tmp_BOOL;
//  int Active;
    int ind, ile;

    if (LineNo==DSP::AM_MasterControl)
    {
      ile = MixerLineOUT.cChannels;

      tmp_BOOL.resize(ile);

      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlOUT_MUTE.dwControlID;
      MixerControlDetails.cChannels=ile; //WaveIn_cChannels; one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=0;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      MixerControlDetails.paDetails=tmp_BOOL.data();

      for (ind=0; ind < ile; ind++)
      {
        switch (ind)
        {
          case 0:
            if (IsMuted_Left == DSP::e::AM_MutedState::MUTED_YES)
              tmp_BOOL[ind].fValue=1;
            else
              tmp_BOOL[ind].fValue=0;
            break;
          case 1:
          default:
            if (IsMuted_Right == DSP::e::AM_MutedState::MUTED_YES)
              tmp_BOOL[ind].fValue=1;
            else
              tmp_BOOL[ind].fValue=0;
            break;
        }
      }

      rs=mixerSetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      if (rs != MMSYSERR_NOERROR)
      {
        MixerControlDetails.cChannels--;
        rs=mixerSetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      }

      return true;
    }

    ile = GetNumberOfDestLines();
    if ((LineNo >= ile) || (LineNo < 0))
      return false;

    if (MixerLinesOUT[LineNo].cControls == 0)
      return false; // NO CONTROLS

    if (MixerControlsOUT_MUTE_supported[LineNo] == false)
      return false; // No MUTE control

    ile =MixerLinesOUT[LineNo].cChannels;
  //  if (Channel >= ile)
  //    return -1.0;

    tmp_BOOL.resize(ile);

    MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    MixerControlDetails.dwControlID=MixerControlsOUT_MUTE[LineNo].dwControlID;
    MixerControlDetails.cChannels=ile; //WaveIn_cChannels; one mixer ON/OFF control per channel
    MixerControlDetails.cMultipleItems=0;
    MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    MixerControlDetails.paDetails=tmp_BOOL.data();

    for (ind=0; ind < ile; ind++)
    {
      switch (ind)
      {
        case 0:
          if (IsMuted_Left == DSP::e::AM_MutedState::MUTED_YES)
            tmp_BOOL[ind].fValue=1;
          else
            tmp_BOOL[ind].fValue=0;
          break;
        case 1:
        default:
          if (IsMuted_Right == DSP::e::AM_MutedState::MUTED_YES)
            tmp_BOOL[ind].fValue=1;
          else
            tmp_BOOL[ind].fValue=0;
          break;
      }
    }

    rs=mixerSetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
    if (rs != MMSYSERR_NOERROR)
    {
      MixerControlDetails.cChannels--;
      rs=mixerSetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
    }

    return true;

  #else
    (void)LineNo; // unused

    return false;

  #endif
}


double TAudioMixer::GetActiveSourceLineVolume(void)
{
//  MIXERCONTROLDETAILS MixerControlDetails;
//  MIXERCONTROLDETAILS_UNSIGNED tmp_UNSIGNED;
  int Active;
//  double output;

  Active=GetActiveSourceLine();
  if (Active==-1)
    return -1.0;
//  if (Active==AM_PCMwaveFile)
//		return PCMwaveFileActiveValue;

  return GetSourceLineVolume(Active);

/*
  MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
  MixerControlDetails.dwControlID=MixerControlsWAVEIN[Active].dwControlID;
  MixerControlDetails.cChannels=1; //WaveIn_cChannels; one mixer ON/OFF control per channel
  MixerControlDetails.cMultipleItems=0;
  MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
  MixerControlDetails.paDetails=&(tmp_UNSIGNED);
  rs=mixerGetControlDetails((HMIXEROBJ)hMixer, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);

  output=(tmp_UNSIGNED.dwValue-MixerControlsWAVEIN[Active].Bounds.dwMinimum);
  output/=(MixerControlsWAVEIN[Active].Bounds.dwMaximum -
           MixerControlsWAVEIN[Active].Bounds.dwMinimum);

  return output;
*/
}

double TAudioMixer::GetSourceLineVolume(int LineNo)
{
  if (LineNo==DSP::AM_PCMwaveFile)
    return PCMwaveFileActiveValue;

  #ifdef WIN32
    MIXERCONTROLDETAILS MixerControlDetails;
    MIXERCONTROLDETAILS_UNSIGNED tmp_UNSIGNED;
    double output;

    int ile;

    // Master Recording Control
    if (LineNo == DSP::AM_MasterControl)
    {
      if (MixerControlWAVEIN_VOLUME_supported == false)
      {
        rs = MIXERR_INVALCONTROL;
        output = -1.0;
      }
      else
      {
        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);

        MixerControlDetails.dwControlID=MixerControlWAVEIN_VOLUME.dwControlID;
        MixerControlDetails.cChannels=1; //WaveIn_cChannels; one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=0;
        MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
        MixerControlDetails.paDetails=&(tmp_UNSIGNED);
        rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);

        output=(tmp_UNSIGNED.dwValue-MixerControlWAVEIN_VOLUME.Bounds.dwMinimum);
        output/=(MixerControlWAVEIN_VOLUME.Bounds.dwMaximum -
                MixerControlWAVEIN_VOLUME.Bounds.dwMinimum);
      }
      return output;
    }

    // Other controls
    ile = GetNumberOfSourceLines();
    if ((LineNo >= ile) || (LineNo < 0))
      return -1.0;

  //  ile =MixerControlsWAVEIN[LineNo].cChannels;
  //  if (Channel >= ile)
  //    return -1.0;

    MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    if (MixerControlsWAVEIN_VOLUME_supported[LineNo] == false)
    {
      rs = MIXERR_INVALCONTROL;
      output = -1.0;
    }
    else
    {
      MixerControlDetails.dwControlID=MixerControlsWAVEIN_VOLUME[LineNo].dwControlID;
      MixerControlDetails.cChannels=1; //WaveIn_cChannels; one mixer ON/OFF control per channel
      MixerControlDetails.cMultipleItems=0;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
      MixerControlDetails.paDetails=&(tmp_UNSIGNED);
      rs=mixerGetControlDetails((HMIXEROBJ)hMixer_in, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);

      output=(tmp_UNSIGNED.dwValue-MixerControlsWAVEIN_VOLUME[LineNo].Bounds.dwMinimum);
      output/=(MixerControlsWAVEIN_VOLUME[LineNo].Bounds.dwMaximum -
              MixerControlsWAVEIN_VOLUME[LineNo].Bounds.dwMinimum);
    }

    return output;

  #else

    return -1.0;

  #endif
}

/*! Fixed <b>2006.01.23</b> returns -2.0 if no volume control is supported
 */
double TAudioMixer::GetDestLineVolume(int LineNo, int Channel)
{
  #ifdef WIN32
    MIXERCONTROLDETAILS MixerControlDetails;
    std::vector<MIXERCONTROLDETAILS_UNSIGNED> tmp_UNSIGNED;
    double output;
    int ile;

    if (LineNo==DSP::AM_MasterControl)
    {
      ile = MixerLineOUT.cChannels;

      tmp_UNSIGNED.resize(ile);

      MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
      MixerControlDetails.dwControlID=MixerControlOUT_VOL.dwControlID;
      MixerControlDetails.cChannels=ile;
      MixerControlDetails.cMultipleItems=0;
      MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
      MixerControlDetails.paDetails=tmp_UNSIGNED.data();
      rs=mixerGetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
      if (rs != MMSYSERR_NOERROR)
      {
        MixerControlDetails.cChannels--;
        ile--;
        rs=mixerGetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
      }

      switch (Channel)
      {
        case 0:
          output=(tmp_UNSIGNED[0].dwValue-MixerControlOUT_VOL.Bounds.dwMinimum);
          break;
        case 1:
          output=(tmp_UNSIGNED[1].dwValue-MixerControlOUT_VOL.Bounds.dwMinimum);
          break;
        case -1:
        default:
          output=(tmp_UNSIGNED[0].dwValue-MixerControlOUT_VOL.Bounds.dwMinimum);
          if (ile > 1)
          {
            output+=(tmp_UNSIGNED[1].dwValue-MixerControlOUT_VOL.Bounds.dwMinimum);
            output /= 2;
          }
          break;
      }
      output/=(MixerControlOUT_VOL.Bounds.dwMaximum -
               MixerControlOUT_VOL.Bounds.dwMinimum);
      //if (ile < Channel)
      if ((ile <= 0) || (ile-1 < Channel))
        output = -1.0;

      return output;
    }


    ile = GetNumberOfDestLines();
    if ((LineNo >= ile) || (LineNo < 0))
      return -1.0;

    if (MixerLinesOUT[LineNo].cControls == 0)
      return -2.0; // NO CONTROLS

    if (MixerControlsOUT_VOL_supported[LineNo] == false)
      return -2.0; // NO VOLUME CONTROL SUPPORTED

    ile =MixerLinesOUT[LineNo].cChannels;
    if (Channel >= ile)
      return -1.0;

    tmp_UNSIGNED.resize(ile);

    MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    MixerControlDetails.dwControlID=MixerControlsOUT_VOL[LineNo].dwControlID;
    MixerControlDetails.cChannels=ile;
    MixerControlDetails.cMultipleItems=0;
    MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
    MixerControlDetails.paDetails=tmp_UNSIGNED.data();
    rs=mixerGetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
    if (rs != MMSYSERR_NOERROR)
    {
      MixerControlDetails.cChannels--;
      ile--;
      rs=mixerGetControlDetails((HMIXEROBJ)hMixer_out, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
    }

    switch (Channel)
    {
      case 0:
        output=(tmp_UNSIGNED[0].dwValue-MixerControlsOUT_VOL[LineNo].Bounds.dwMinimum);
        break;
      case 1:
        output=(tmp_UNSIGNED[1].dwValue-MixerControlsOUT_VOL[LineNo].Bounds.dwMinimum);
        break;
      case -1:
      default:
        output=(tmp_UNSIGNED[0].dwValue-MixerControlsOUT_VOL[LineNo].Bounds.dwMinimum);
        if (ile > 1)
        {
          output+=(tmp_UNSIGNED[1].dwValue-MixerControlsOUT_VOL[LineNo].Bounds.dwMinimum);
          output /= 2;
        }
        break;
    }
    output/=(MixerControlsOUT_VOL[LineNo].Bounds.dwMaximum -
             MixerControlsOUT_VOL[LineNo].Bounds.dwMinimum);
    if (ile < Channel)
      output = -1.0;

    return output;

  #else
    (void)LineNo; // unused 
    (void)Channel; // unused 

    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "TAudioMixer::GetDestLineVolume" << DSP::e::LogMode::second << "Not yet implemented on this platform" << endl;
    #endif

    return -2.0; // NO CONTROLS
  #endif
}



const DWORD TAudioMixer::Types[]={
  #ifdef WIN32
    MIXERCONTROL_CT_CLASS_CUSTOM,
      MIXERCONTROL_CONTROLTYPE_CUSTOM,

    MIXERCONTROL_CT_CLASS_FADER,
      MIXERCONTROL_CONTROLTYPE_BASS,
      MIXERCONTROL_CONTROLTYPE_EQUALIZER,
      MIXERCONTROL_CONTROLTYPE_FADER,
      MIXERCONTROL_CONTROLTYPE_TREBLE,
      MIXERCONTROL_CONTROLTYPE_VOLUME,

    MIXERCONTROL_CT_CLASS_LIST,
      MIXERCONTROL_CONTROLTYPE_MIXER,
      MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT,
      MIXERCONTROL_CONTROLTYPE_MUX,
      MIXERCONTROL_CONTROLTYPE_SINGLESELECT,

    MIXERCONTROL_CT_CLASS_METER,
      MIXERCONTROL_CONTROLTYPE_BOOLEANMETER,
      MIXERCONTROL_CONTROLTYPE_PEAKMETER,
      MIXERCONTROL_CONTROLTYPE_SIGNEDMETER,
      MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER,

    MIXERCONTROL_CT_CLASS_NUMBER,
      MIXERCONTROL_CONTROLTYPE_DECIBELS,
      MIXERCONTROL_CONTROLTYPE_PERCENT,
      MIXERCONTROL_CONTROLTYPE_SIGNED,
      MIXERCONTROL_CONTROLTYPE_UNSIGNED,

    MIXERCONTROL_CT_CLASS_SLIDER,
      MIXERCONTROL_CONTROLTYPE_PAN,
      MIXERCONTROL_CONTROLTYPE_QSOUNDPAN,
      MIXERCONTROL_CONTROLTYPE_SLIDER,

    MIXERCONTROL_CT_CLASS_SWITCH,
      MIXERCONTROL_CONTROLTYPE_BOOLEAN,
      MIXERCONTROL_CONTROLTYPE_BUTTON,
      MIXERCONTROL_CONTROLTYPE_LOUDNESS,
      MIXERCONTROL_CONTROLTYPE_MONO,
      MIXERCONTROL_CONTROLTYPE_MUTE,
      MIXERCONTROL_CONTROLTYPE_ONOFF,
      MIXERCONTROL_CONTROLTYPE_STEREOENH,

    MIXERCONTROL_CT_CLASS_TIME,
      MIXERCONTROL_CONTROLTYPE_MICROTIME,
      MIXERCONTROL_CONTROLTYPE_MILLITIME
  #endif
};

const string TAudioMixer::TypesNames[]={
  #ifdef WIN32
    "MIXERCONTROL_CT_CLASS_CUSTOM",
      "MIXERCONTROL_CONTROLTYPE_CUSTOM",

    "MIXERCONTROL_CT_CLASS_FADER",
      "MIXERCONTROL_CONTROLTYPE_BASS",
      "MIXERCONTROL_CONTROLTYPE_EQUALIZER",
      "MIXERCONTROL_CONTROLTYPE_FADER",
      "MIXERCONTROL_CONTROLTYPE_TREBLE",
      "MIXERCONTROL_CONTROLTYPE_VOLUME",

    "MIXERCONTROL_CT_CLASS_LIST",
      "MIXERCONTROL_CONTROLTYPE_MIXER",
      "MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT",
      "MIXERCONTROL_CONTROLTYPE_MUX",
      "MIXERCONTROL_CONTROLTYPE_SINGLESELECT",

    "MIXERCONTROL_CT_CLASS_METER",
      "MIXERCONTROL_CONTROLTYPE_BOOLEANMETER",
      "MIXERCONTROL_CONTROLTYPE_PEAKMETER",
      "MIXERCONTROL_CONTROLTYPE_SIGNEDMETER",
      "MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER",

    "MIXERCONTROL_CT_CLASS_NUMBER",
      "MIXERCONTROL_CONTROLTYPE_DECIBELS",
      "MIXERCONTROL_CONTROLTYPE_PERCENT",
      "MIXERCONTROL_CONTROLTYPE_SIGNED",
      "MIXERCONTROL_CONTROLTYPE_UNSIGNED",

    "MIXERCONTROL_CT_CLASS_SLIDER",
      "MIXERCONTROL_CONTROLTYPE_PAN",
      "MIXERCONTROL_CONTROLTYPE_QSOUNDPAN",
      "MIXERCONTROL_CONTROLTYPE_SLIDER",

    "MIXERCONTROL_CT_CLASS_SWITCH",
      "MIXERCONTROL_CONTROLTYPE_BOOLEAN",
      "MIXERCONTROL_CONTROLTYPE_BUTTON",
      "MIXERCONTROL_CONTROLTYPE_LOUDNESS",
      "MIXERCONTROL_CONTROLTYPE_MONO",
      "MIXERCONTROL_CONTROLTYPE_MUTE",
      "MIXERCONTROL_CONTROLTYPE_ONOFF",
      "MIXERCONTROL_CONTROLTYPE_STEREOENH",

    "MIXERCONTROL_CT_CLASS_TIME",
      "MIXERCONTROL_CONTROLTYPE_MICROTIME",
      "MIXERCONTROL_CONTROLTYPE_MILLITIME",

  #endif
  "END"};

const DWORD TAudioMixer::ComponentTypes[]={
  #ifdef WIN32
    MIXERLINE_COMPONENTTYPE_DST_DIGITAL,
    MIXERLINE_COMPONENTTYPE_DST_HEADPHONES,
    MIXERLINE_COMPONENTTYPE_DST_LINE,
    MIXERLINE_COMPONENTTYPE_DST_MONITOR,
    MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,
    MIXERLINE_COMPONENTTYPE_DST_TELEPHONE,
    MIXERLINE_COMPONENTTYPE_DST_UNDEFINED,
    MIXERLINE_COMPONENTTYPE_DST_VOICEIN,
    MIXERLINE_COMPONENTTYPE_DST_WAVEIN,
    MIXERLINE_COMPONENTTYPE_SRC_ANALOG,
    MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY,
    MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC,
    MIXERLINE_COMPONENTTYPE_SRC_DIGITAL,
    MIXERLINE_COMPONENTTYPE_SRC_LINE,
    MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE,
    MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER,
    MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER,
    MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE,
    MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED,
    MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT,
  #endif
  0xffffffff
  };

const string TAudioMixer::ComponentNames[]={
  #ifdef WIN32
    "MIXERLINE_COMPONENTTYPE_DST_DIGITAL",
    "MIXERLINE_COMPONENTTYPE_DST_HEADPHONES",
    "MIXERLINE_COMPONENTTYPE_DST_LINE",
    "MIXERLINE_COMPONENTTYPE_DST_MONITOR",
    "MIXERLINE_COMPONENTTYPE_DST_SPEAKERS",
    "MIXERLINE_COMPONENTTYPE_DST_TELEPHONE",
    "MIXERLINE_COMPONENTTYPE_DST_UNDEFINED",
    "MIXERLINE_COMPONENTTYPE_DST_VOICEIN",
    "MIXERLINE_COMPONENTTYPE_DST_WAVEIN",
    "MIXERLINE_COMPONENTTYPE_SRC_ANALOG",
    "MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY",
    "MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC",
    "MIXERLINE_COMPONENTTYPE_SRC_DIGITAL",
    "MIXERLINE_COMPONENTTYPE_SRC_LINE",
    "MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE",
    "MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER",
    "MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER",
    "MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE",
    "MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED",
    "MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT",
  #endif
  "0xffffffff"
  };

const string TAudioMixer::GetMixerControlType(DWORD dwControlType)
{
  #ifdef WIN32
    int indType;

    indType=0;
    while (dwControlType!=Types[indType])
    {
      if (Types[indType]==MIXERCONTROL_CONTROLTYPE_MILLITIME)
      {
        indType++;
        break;
      }
      indType++;
    }
    return TypesNames[indType];
  #else
    (void)dwControlType; // unused 

    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "TAudioMixer::GetMixerControlType" << DSP::e::LogMode::second << "Not yet implemented on this platform" << endl;
    #endif

    return NULL;
  #endif
}

const string TAudioMixer::GetMixerComponentType(DWORD dwComponentType)
{
  int indType;

  indType=0;
  while (dwComponentType!=ComponentTypes[indType])
  {
    if (ComponentTypes[indType]==0xffffffff)
      break;
    indType++;
  }
  return ComponentNames[indType];
}


