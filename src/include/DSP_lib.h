/*! \file DSP_lib.h
 * DSP Engine library information header file.
 *
 * This is main header file.
 *
 * \author Marek Blok
 */

#ifndef DSP_lib_H
#define DSP_lib_H

#define DSP_VER_MAJOR 0
#define DSP_VER_MINOR 19
#define DSP_VER_BUILD 16 // !!! without zeroes before, else this will be treated as octal number
#define DSP_VER_YEAR  2021
#define DSP_VER       DSP_VER_MAJOR.DSP_VER_MINOR.DSP_VER_BUILD

//#define WINVER 0x0A00
//#define _WIN32_WINNT 0x0A00

#ifdef __DEBUG__
#ifdef __DEBUG_LEAKAGE__
  #include "../nvwa/debug_new.h"
#endif
#endif

#include <assert.h>
//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------
#include <DSP_IO.h>
#include <DSP_clocks.h>
#include <DSP_modules2.h>
//#include <DSP_Fourier.h>
#include <DSP_AudioMixer.h>


/*!
 *  \addtogroup ver_data
 */
//! Structure containing DSP version information. SEE details for changes info...
struct DSP_libver
{
  unsigned char  major;
  unsigned char  minor;
  unsigned short build;
};

//! Returns DSP Engine library version information
/*! <b>Major library updates</b>
 *
 *  \bug <b>2008.03.23</b> Check what will happen if all inputs are constant
 *    for blocks like DSPu_PCCC. Those should work like sources and have clock defined.
 *    ConvertConst2Source <== checks whether all inputs are constant,
 *        defines block clock and adds to list of constant input blocks for given clock.
 *  \todo <b>2008.04.06</b> DSPu_RawDecimator, DSPu_SampleRateConverter - zaimplementowa� w wersji bez
 *    rejestracji jako źródło (może nawet zaimplementować jako zwykły blok przetwarzania)
 *    - może przyśpieszyć przetwarzanie - mniej źródeł do przeszukiwania
 *  \todo <b>2008.04.06</b> DSP::Component::CheckInputsOfComponents(DSP_clock_ptr) - checking
 *     only for components of given clock. Also implement version in DSP_clock like
 *     DSP_clock::ListOfComponents
 *  \todo <b>2008.04.10</b> Przejrze� bloczki wieloszybkosciowe i asynchroniczne.
 *   -# dla bloczk�w multirate
 *     -# decymatory w stosunku wymiernym -> przerobi� na zwyk�e processing blocks,
 *        nie wymagaj� obs�ugi za pomoc� zegara
 *     -# interpolatory w stosunku wymiernym -> musz� by� bloczkami typu mixed.
 *        Pytanie nale�y korzysta� z Notifications czy lepiej kontrol�
 *        kompletno�ci pr�bek wej�ciowych pozostawi� InputExecute i OutputExecute.
 *  \todo <b>2008.04.13</b> When wav file does not exist, something
 *     more then "Unsupported input sampling rate" should be stated
 *    for DSPu_WaveInput
 *
 *  \todo Update docs with info about usage of DSP_Clock_trigger
 *
 *  \todo <b>2008.05.30</b> try if Multiplexer can be change into regular block
 *     instead of mixed one, output samples as soon as possible
 *     (implements as additional variant)
 *
 *  \todo <b>2008.05.30</b> implement Demultiplexer as a source (additional variant)
 *     output all samples together (outputs must be delayed)
 *
 *  \todo <b>2008.05.30</b> !!! implement offset clocks
 *   - normal clock but offset for some number of global cycles
 *   - needs rethinking of clock descent rules
 *   .
 *
 *  \todo <b>2008.07.05</b> add macro into library documentation
 *  \todo <b>2008.07.09</b> add DSPu_Quantizer into library documentation
 *
 *  \bug <b>2008.09.29</b> Increase automatically named outputs range (now it is up to 999 output).
 *    In some case when working with buffer outputs it might be to little (see OFDM implementation).
 *
 * \bug <b>2008.09.29</b> DOT file generation needs buffer overflow fix.
 *   DSP::Component::GetComponentNodeParams_DOTfile fixed but there are
 *   more similar functions.
 *
 * \todo <b>2008.10.29</b> DSPu_FIR introduce optimized InputExecute procedures instead of one universal
 *
 * \todo <b>2010.04.26</b> Integrate DSPu_RealMultiplication in DSPu_Multiplication as optimized implementation variant
 * \todo <b>2010.04.26</b> Integrate DSPu_LoopDelay in DSPu_Delay as implementation variant
 * \todo <b>2012.03.27</b> Test all variants of DSPu_FIR
 * \todo <b>2012.04.17</b> Check if DSPu_FIR variant for I-FIR shaping filter is implemented on the basis
 *                         of cyclic buffer and not based on memcpy
 *
 * - ver. 0.19.015 - <b>2021.03.12</b> Changed: type DSPf_ExternalSleep_ptr moved into DSP namespace:  DSP::ExternalSleep_ptr
 *                                     Changed: moved strncmpi inside the T_WAVEchunk class
 *                                     Changed: moved DSPf_* functions into DSP::f:: namespace: DSP::f::*
 * - ver. 0.19.014 - <b>2021.02.18</b> Changed: enum DSPe_SampleType moved into DSP::e namespace:  DSP::e::SampleType
 *                                     Changed: enum DSPe_FileType moved into DSP::e namespace:  DSP::e::FileType
 *                                     Changed: enum DSPe_AM_MutedState moved into DSP::e namespace:  DSP::e::AM_MutedState
 *                                     Changed: AM_MasterControl; AM_PCMwaveFile; AM_PCMwaveFileON; AM_PCMwaveFileOFF moved into DSP namespace
 * - ver. 0.19.013 - <b>2021.02.17</b> Changed: Class DSP::LoadCoef moved into DSP namespace: DSP::LoadCoef
 *                                     Changed: enum DSPe_LoadCoef_Type moved into DSP::e namespace:  DSP::e::LoadCoef_Type
 * - ver. 0.19.012 - <b>2021.01.28</b> Added: Added DSP::LogMode::pause and DSP::LogMode::pause_off
 *                                            Allows for pause in Info mode and skipping pause in Error mode.
 * - ver. 0.19.011 - <b>2021.01.20</b> Fixed: Main.cpp addapted to library changes
 * - ver. 0.19.010 - <b>2021.01.18</b> Changed: DSP_rand moved into DSP namespace: DSP::Rand
 *                                     Changed: DSP_file moved into DSP namespace: DSP:File
 * - ver. 0.19.009 - <b>2021.01.18</b> Changed: DSP_block moved into DSP namespace: DSP::Block
 *                                     Changed: DSP_source moved into DSP namespace: DSP:Source
 *                                     Changed: DSP_component moved into DSP namespace: DSP:Component
 *                                     Changed: DSP_macro moved into DSP namespace: DSP:Macro
 * - ver. 0.19.007 - <b>2020.09.08</b> Changed: DSP_connect  i DSP_splitconnect made unavailable for library user; now only << and >> operators can be used to connect blocks
 * - ver. 0.19.006 - <b>2020.09.08</b> Changed: DSP_name moved into DSP namespace: DSP::name
 * - ver. 0.19.005 - <b>2020.09.07</b> Changed: DSP_input and DSP_output moved into DSP namespace: DSP::input and DSP::output
 * - ver. 0.19.004 - <b>2020.09.07</b> Changed: DSP_clock moved into DSP namespace DSP::Clock 
 * - ver. 0.19.003 - <b>2020.09.07</b> Changed: Added DSP::logstream and DSP::log global object for logging management 
 *                                              DSP::f::InfoMessage is replaced with DSP::log << "source" << DSP::LogMode::second << "message" << endl;
 *                                              DSP::f::ErrorMessage is replaced with DSP::log << DSP::LogMode::Error << "source" << DSP::LogMode::second << "message" << endl;
 *                                              DSP::f::SetLogState and DSP::f::SetLogFileName are replaced with DSP::log.SetLogState and DSP::log.SetLogFileName
 *                                     Fixed: DSP::log Error message with no pause in console even if source and message are empty
 *                                     Changed: Started moving DSP_*, DSP::f::*, DSPu_* to DSP namespace. First is DSP_logstream.cpp/h with DSP::logstream class.
 * - ver. 0.19.001 - <b>2020.08.31</b> Changed: Unified source code filenames prefixes to "DSP_"
 * - ver. 0.19.000 - <b>2020.08.19</b> Changed: Linux compilation and code correction to eliminate compilation warnings
 * ======================================================================================================================================
 * - ver. 0.18.020 - <b>2020.04.22</b> Fixed: DSPu_Vacuum - meaning of "in" depends now on NoOfInputs
 * - ver. 0.18.019 - <b>2020.03.17</b> Fixed: DSPu_PSKencoder change input's names from "in0" and "in1" to "in1" and "in2"
 * - ver. 0.18.018 - <b>2020.03.01</b> Fixed: DSPu_MORSEkey::OutputExecute AsciiText update
 * - ver. 0.18.017 - <b>2020.02.29</b> Changed: Conditional WINVER and _WIN32_WINNT definition in DSPsockets.h
 * - ver. 0.18.016 - <b>2020.02.25</b> Added: Support for port number in DSP_socket, DSPu_SOCKETinput and DSPu_SOCKEToutput
 * - ver. 0.18.015 - <b>2020.02.23</b> Added: DSP::e::SampleType DSP::e::SampleType::ST_tchar for text output from DSPu_FILEoutput
 * - ver. 0.18.014 - <b>2020.02.22</b> Changed: DSPu_Farrow constructor as an input uses vector of vectors instead of array of arrays
 * - ver. 0.18.013 - <b>2020.02.15</b> Fixed: DSPu_FILEinput::ReadSegmentToBuffer raw_buffer type changed to uint8_t *
 *                                     Added: DSP::f::sinc<DSP_float>(const DSP_float_vector& arguments, DSP_float_vector& output_buffer);
 *                                     Added: DSP::f::sinc<DSP_complex>(const DSP_float_vector& arguments, DSP_complex_vector& output_buffer);
 * - ver. 0.18.012 - <b>2019.03.17</b> Fixed: DSP_clock::UpdateSamplingRates uses now GlobalSamplingRate which is set for current clock in DSP_clock::SetSamplingRate
 *                                     Added: DSP_clock::UpdateGlobalSamplingRate
 *                                     Fixed: DSP_clock::~DSP_clock() updates ParentClocks if necessary
 * - ver. 0.18.011 - <b>2018.11.20</b> Added: TAudioMixer::MemorizeMixerSettings_OUT - detailed logging in DEBUG mode
 * - ver. 0.18.010 - <b>2018.11.19</b> Fixed: DSP::Component::ListOfAllComponents - detects NoOfOutputs == 0 in source blocks
 * - ver. 0.18.009 - <b>2018.04.15</b> Changed: Added reversed_order parameter to DSPu_Parallel2Serial.
 * - ver. 0.18.008 - <b>2018.03.21</b> Changed: DSPu_SymbolMapper converted into single rate block, it can be used with DSPuSerial2Parallel block to get the original behavior of DSPu_SymbolMapper.
 * - ver. 0.18.008 - <b>2018.03.22</b> Changed: DSPu_SymbolDemapper converted into single rate block, it can be used with DSPuParallel2Serial block to get the original behavior of DSPu_SymbolDemapper.
 *
 * - ver. 0.18.007 - <b>2018.03.19</b> Added: DSPu_Parallel2Serial and DSPu_Serial2Parallel
 * - ver. 0.18.007 - <b>2018.03.20</b> Added: DSP::Component has now method Input() returning nullptr and issuing info message to log in DEBUG mode
 *                                     This allows to store DSP::Block and DSP::Source components in the same DSP::Component * table and connect then without need for conversion.
 * - ver. 0.18.006 - <b>2018.03.18</b> Fix: getConstellation - DSP_MT_PSK uses Gray coding
 * - ver. 0.18.006 - <b>2018.03.18</b> Fix: DSPu_SymbolDemapper - do not produce output bits before receiving first symbol
 * - ver. 0.18.005 - <b>2018.03.08</b> Added DSP_output::<< operator which can replace DSP::_connect_class::connect
 * - ver. 0.18.004 - <b>2018.03.08</b> Added DSP_output::>> operator which can replace DSP::_connect_class::connect
 * - ver. 0.18.003 - <b>2018.03.07</b> Converted to vectors: ConstantInputValues, IsConstantInput, InputClocks
 * - ver. 0.18.002 - <b>2018.03.07</b> Fix in DSP_clock::SchemeToDOTfile for clocks_group generation
 * - ver. 0.18.001 - <b>2018.03.07</b> Removed DSP_Input_ptr and DSP_Output_ptr; Added DSP_Input::is_null() and DSP_Output::is_null()
 * - ver. 0.18.001 - <b>2018.03.07</b> DSP_Input and DSP_Output store indexes in vectors
 * - ver. 0.18.000 - <b>2018.03.06</b> Removed limit for number of inputs and outputs
 * - ver. 0.18.000 - <b>2018.03.06</b> Use of string instead of char []
 * - ver. 0.18.000 - <b>2018.03.06</b> DefineInputs DefineOutputs: change from array to vector of line numbers
 * - ver. 0.17.23 - <b>2018.03.05</b> Migration from char * to string in DSP_AudioMixer, DSP_DOT, DSP_IO, ... modules
 * - ver. 0.17.22 - <b>2018.03.01</b> Using map.count instead of map.at with try/catch in DSP_clock_groups::AddOutputs2Group
 * - ver. 0.17.21 - <b>2018.03.01</b> Modified DSPu_SymbolMapper and DSPu_SymbolDemapper (added constellation_phase_offset).
 * - ver. 0.17.20 - <b>2017.05.23</b> Added DSPu_SymbolMapper and DSPu_SymbolDemapper.
 * - ver. 0.17.19 - <b>2017.03.22</b> Fixed DSPu_FILEinput filename as string.
 * - ver. 0.17.18 - <b>2017.03.22</b> Fixed DSPu_FILEoutput filename as string.
 * - ver. 0.17.17 - <b>2017.03.22</b> Added DSPu_FILEinput error message when file could not be opened correctly.
 * - ver. 0.17.16 - <b>2017.03.21</b> Changed DSP_LoadCoef now reads coefficients into DSP_*_vector.
 * - ver. 0.17.15 - <b>2017.03.21</b> Fix DSPu_SamplingRateConversion input for complex values.
 * - ver. 0.17.12 - <b>2017.03.18</b> Removed DSP_FILE_NAME_LEN; Use of string instead of char * in DSP_lib.h.
 * - ver. 0.17.11 - <b>2017.03.17</b> Use of string instead of char * in names and DSPu_MorseKey.
 *
 * - ver. 0.16.00 - <b>2016.04.22</b> DSP::Block::SetBlockInputClock with support for clocks groups in block.
 * - ver. 0.16.01 - <b>2016.04.22</b> Modernization of DSPu_SamplingRateConversion and DSPu_GardnerSampling.
 * - ver. 0.16.02 - <b>2016.06.02</b> DSPu_GardnerSampling - added support for OQPSK synchronization
 * - ver. 0.16.04 - <b>2016.06.03</b> DSPu_GardnerSampling - fix for better options support; A number of clocks groups definitions added.
 * - ver. 0.16.05 - <b>2016.06.07</b> DSPu_FILEoutput - added Flush() method.
 * - ver. 0.16.06 - <b>2016.06.10</b> Fix in DSP::Source::SetNoOfOutputs.
 * - ver. 0.16.07 - <b>2016.06.12</b> Fix DSP::Block::SetBlockInputClock - proper setting of GroupClock in ClockGroups in Release version
 * - ver. 0.16.08 - <b>2016.11.01</b> Fix DSPu_GardnerSampling - proper setting of GroupClock in ClockGroups
 *                                    Fix DSPu_Zeroinserter and DSPu_RawDecimator - corrected output clock group name
 * - ver. 0.16.09 - <b>2016.11.10</b> Added DSP::f::SaveVector.
 * - ver. 0.16.17 - <b>2017.02.01</b> Fix DSPu_PCCC - constant inputs support correction.
 *
 *
 * - ver. 0.15.38 - <b>2016.01.26</b> Small code corrections.
 *
 * - ver. 0.15.36 - <b>2012.05.04</b>
 *    - ADDED DBPSK and DEBPSK support for DSPu_PSKdecoder
 *    - FIXED DSPu_OutputBuffer::InputExecute and DSPu_OutputBuffer::InputExecute_with_outputs
 *    - FIXED DSPu_OutputBuffer::FlushBuffer for bit modes (RawBuffer requires one additional safety byte
 *    .
 * - ver. 0.15.35 - <b>2012.04.27</b>
 *    - CHANGED  DSP_Fourier
 *      - FFT uses FFTshiftON (needs testing)
 *      - added DFT
 *      .
 *    - ADDED DSPu_FFT - block computing DFT (or radix-2 FFT when applicable)
 *    - ADDED DSPu_Zeroinserter implementation for complex inputs
 *    - CHANGED DSP::Block::GetMultirateFactorsFromClocks
 *    - CHANGED Added mode parameter to DSPu_OutputBuffer:GetBufferSize
 *    - FIXED DSPu_OutputBuffer - mode with output and notification clock now works
 *       correctly as the mixed block.
 *    - FIXED DSPu_PSKdecoder - fix for QPSK modulations
 *    - FIXED DSPu_FILEoutput::FlashBuffer() - fix for sampel size < 8 in bits
 *    .
 * - ver. 0.15.34 - <b>2012.03.27</b>
 *    - CHANGED  DSPu_FIR - split InputExecute into several optimized procedures
 *    - ADDED DSPu_FIR::InputExecute_RI_RH_L - shaping filter for I-FIR filter implementation
 *    .
 * - ver. 0.15.33 - <b>2012.03.19</b>
 *    - ADDED  DSPu_OutputBuffer::AccessBuffer
 *    - ADDED  DSPu_OutputBuffer::GetBufferSize
 *    .
 * - ver. 0.15.32 - <b>2012.03.06</b>
 *    - ADDED  DSPu_FIR - support for polyphase decomposition of input impulse response
 *    - FIXED  DSPu_PSKencoder - corrected number of input for complex modulations
 *    .
 * - ver. 0.15.28 - <b>2010.05.21</b>
 *    - FIXED  DSPu_FILEinput::OpenFile
 *    - FIXED  DSP::f::MakeDir
 *    - ADDED  DSP::f::MakeDir_Ex
 *    - FIXED  DSPu_FILEinput, DSPu_FILEoutput - improved support in case of problems with file opening
 *    - ADDED  Macros: UNUSED_ARGUMENT, UNUSED_DEBUG_ARGUMENT and UNUSED_RELEASE_ARGUMENT
 *    - FIXED  DSP::MacroStack::RemoveMacroFromList
 *    - FIXED  DSPu_DCO support for max frequency deviation
 *    .
 * - ver. 0.15.014 - <b>2010.04.16</b>
 *   - CHANGED DSPu_Copy::SetCopyInput - added support for Copy chains
 *   - CHANGED DSPu_Copy::SetCopyOutput - added support for Copy chains
 *   - ADDED   DSPu_Copy::GetOutput
 *   - ADDED   DSP::Macro::GetMacroInputNo
 *   - CHANGED Improved DOT macro support
 *   .
 * - ver. 0.15.003 - <b>2010.04.12</b>
 *   - ADDED DSP::Macro::DefineInput and DSP::Macro::DefineOutput.
 *   - CHANGED Macro input will use internally and externally the same names. The same is with macro output.
 *   - CHANGED renamed DSP::Macro::MacroInput to DSP::Macro::MacroInput_block
 *   - CHANGED renamed DSP::Macro::MacroOutput to DSP::Macro::MacroOutput_block
 *   - CHANGED renamed DSP::Macro::InputOutput() to DSP::Macro::MacroInput()
 *   - CHANGED renamed DSP::Macro::OutputInput() to DSP::Macro::MacroOutput()
 *   - ADDED DSP::Component::UndefineInput(NULL) deletes all input definitions
 *   - ADDED DSP::Component::UndefineOutput(NULL) deletes all output definitions
 *   - CHANGED example makefile improvements
 *   - CHANGED macro_example.cpp updated
 * .
 * - ver. 0.15.001 - <b>2010.04.08</b>
 *   - ADDED Introduced macros: OUTPUT_EXECUTE_ARGS, OUTPUT_EXECUTE_PTR
 *   - CHANGED In debug mode OutputExecute callbacks skip the last parameter (calling clock pointer)
 * .
 * - ver. 0.15.000 - <b>2010.04.08</b>
 *   - ADDED Introduced macros: INPUT_EXECUTE_ARGS, EXECUTE_PTR, EXECUTE_INPUT_CALLBACK
 *   - CHANGED In debug mode InputExecute callbacks skip the last parameter (calling block pointer)
 * .
 *
 * - ver. 0.14.004 - <b>2010.04.02</b>
 *   - CHANGED Moved DOT file generation code into separate cpp module
 *   - FIXED Implemented minimal Macro for DOT support with merged autosplitters
 *   .
 * - ver. 0.14.001 - <b>2010.03.31</b>
 *   - CHANGED alpha support for new DSPu_Copy rules
 *   .
 * - ver. 0.14.000 - <b>2010.03.24</b>
 *   - CHANGED DSPu_Copy - rules modification - protection against double auto splitters
 *     - ADDED new DSP::Component::IsOutputConnectedToThisInput2 in addition to DSP::Component::IsOutputConnectedToThisInput
 *     - NEW new version of DSP::_connect_class::splitconnect (changed name of deprecated version DSP::_connect_class::splitconnect_old)
 *     .
 *   .
 * .
 *
 * - ver. 0.13.035 - <b>2010.03.15</b>
 *   - FIXED TAudioMixer - support for USB SBPlay!
 *   .
 * - ver. 0.13.033 - <b>2010.03.04</b>
 *   - FIXED DSPu_TimingErrorDetector::InputExecute_cplx_odd
 *   .
 * - ver. 0.13.032 - <b>2010.02.25</b>
 *   - CHANGED Renames DSP::f::prec_sinc to DSP::f::sinc_prec
 *   .
 * - ver. 0.13.031 - <b>2010.02.24</b>
 *   - FIXED ~DSP::Component - automatically remove output blocks after the component is unregistered
 *     (important if there is cascade of blocks with AutoFree on)
 *   - ADDED examples/macro_example.cpp
 *   .
 * - ver. 0.13.030 - <b>2010.02.24</b>
 *   - FIXED DSP::Macro DOT input lines numbering
 *   .
 * - ver. 0.13.029 - <b>2010.02.19</b>
 *   - CHANGED  DSP::Component::SetOutput - refactored error checking procedures
 *   .
 * - ver. 0.13.028 - <b>2010.02.19</b>
 *   - FIXED DSPu_Splitter::GetComponentEdgeParams_DOTfile - DOT plot output lines
 *     color generation:
 *     - for single input: color depends on output index
 *     - for multiple inputs: color depends on input index
 *     .
 *   .
 * - ver. 0.13.027 - <b>2010.02.08</b>
 *   - FIXED DSPu_SOCKEToutput - now waits if socket not ready to read
 *     (eg. if output reads data slower than we can generate them)
 *   .
 * - ver. 0.13.026 - <b>2009.06.15</b>
 *   - ADDED DSPu_FILEinput::SkipSamples
 *   .
 * - ver. 0.13.025 - <b>2008.12.15</b>
 *   - ADDED DSP::File::SetOffset
 *   - ADDED DSP::File::SetSkip
 *   - CHANGED parameters order in DSPu_FILEinput class constructor
 *   .
 * - ver. 0.13.022 - <b>2008.11.28</b>
 *   - ADDED DSPu_AudioInput::GetNoOfFreeBuffers and DSPu_AudioInput::GetNoOfBuffers
 *   - CHANGED callbacks and notification IDs now are unsigned int instead of int
 *   .
 * - ver. 0.13.019 - <b>2008.11.16</b>
 *   - FIXED Minor fixes in TAudioMixer set and get volume functions.
 *   - ADDED DSPu_MORSEkey::SetKeyState.
 *   - CHANGED DSPu_FILEoutput::WriteSegmentFromBuffer added skip parameter
 *   - CHANGED DSPu_FILEinput::ReadSegmentToBuffer added pad_size parameter
 *   - ADDED DSPu_FILEinput - option for autodetection of number of channels
 *   .
 * - ver. 0.13.014 - <b>2008.11.13</b>
 *   - FIXED TAudioMixer::SetSourceLineVolume now works for LineNo != AM_MasterControl
 *   - FIXED DSPu_MORSEkey::LoadCodeTable problem when file does not exist
 *   - CHANGED ::DSP_Message_callback_ptr prototype - no unprocessed messages are fed to this function
 *   - CHANGED ::DSP::f::InfoMessage, ::DSP::f::ErrorMessage - now log callback function can block logging to console and file
 *   - ADDED ::DSP::f::GetMessageLength, ::DSP::f::GetInfoMessage, ::DSP::f::GetErrorMessage
 *   .
 * - ver. 0.13.011 - <b>2008.11.06</b>
 *   - ADDED DSPu_FILEinput::OpenFile for changing or opening file during processing
 *   - CHANGED DSPu_FILEinput constructor parameters order
 *   - ADDED in DSP_lib.h definitions of DSP_VER_MAJOR, DSP_VER_MINOR, DSP_VER_BUILD and DSP_VER
 *   - ADDED DSPu_MORSEkey:GetDotLength
 *   .
 * - ver. 0.13.009 - <b>2008.11.03</b>
 *   - CHANGED DSPu_MORSEkey constructor
 *   - ADDED DSPu_MORSEkey::AddString
 *   - ADDED DSPu_MORSEkey::AddChar
 *   - ADDED DSPu_MORSEkey::SetKeyingSpeed
 *   - CHANGED DSP_socket::TryConnect - implemented nonblocking version
 *   .
 * - ver. 0.13.005 - <b>2008.10.31</b>
 *   - CHANGED DSPu_InputBuffer added cyclic parameter and optimized support for single channel buffers
 *   - ADDED DSPu_rand::randu
 *   - ADDED DSPu_rand::randn
 *   - FIXED DOT_plot generation for DSPu_ClockTrigger
 *   - ADDED DOT_plot supports now '"' in labels
 *   - FIXED DSPu_IIR should work correctly with Na = 1 and Nb = 0
 *   - CHANGED DSPu_IIR introduced four optimized InputExecute procedures instead of one universal
 *   - ADDED DSPu_IIR::SetCoefs
 *   .
 * - ver. 0.12.008 - <b>2008.10.24</b>
 *   - FIXED DSPu_SOCKEToutput DSP::e::SampleType::ST_short support
 *   .
 * - ver. 0.12.006 - <b>2008.10.19</b>
 *   - NEW DSP_socket::GetSocketStatus
 *   - FIXED DSP_socked on server objects deletion listen socked has
 *       been closed but it hasn't been marked as closed
 *   .
 * - ver. 0.12.003 - <b>2008.10.07</b>
 *   - NEW module DSPmodule_misc
 *   - NEW DSPu_MORSEkey
 *   .
 * - ver. 0.12.001 - <b>2008.10.06</b>
 *   - NEW DSP_socket base class for DSPu_SOCKETinput and DSPu_SOCKEToutput
 *   - NEW DSPu_SOCKETinput
 *   - NEW DSPu_SOCKEToutput
 *   - NEW optional library DSPsockets. See examples.
 *   - NEW examples socket_client.cpp and socket_server.cpp
 *   .
 * - ver. 0.11.033 - <b>2008.10.01</b>
 *   - NEW: functions
 *    - DSP::f::prec_sinc(DSP_prec_float)
 *   - CHANGED DSP::f::SolveMatrixEqu_prec - added use_pivote modes
 *   - FIXED DSP::f::LPF_LS use improved pivote mode in DSP::f::SolveMatrixEqu_prec
 *   .
 * - ver. 0.11.032 - <b>2008.09.29</b>
 *   - FIXED DSP::Component::GetComponentNodeParams_DOTfile buffer overflow problem
 *   - FIXED DSP::Component::ComponentToDOTfile automatically increases allocated
 *      buffer for ComponentNodeParams returned by DSP::Component::GetComponentNodeParams_DOTfile
 *   .
 * - ver. 0.11.031 - <b>2008.08.23</b>
 *   - ADDED DSP::e::FileType::FT_wav format support to DSPu_FILEinput
 *   - ADDED DSPu_FILEinput::GetHeader
 *   - ADDED DSP::f::FileExtToFileType function
 *   - CHANGED DSP_IO all ftell and tseek calls replaced with ftello64 and fseeko64
 *   .
 * - ver. 0.11.025 - <b>2008.07.27</b>
 *   - ADDED DSP::f::LPF_LS - LP FIR LS filter design
 *   - ADDED second variant of DSP::f::SolveMatrixEqu_prec
 *   - FIXED DSPu_FILEoutput: File type DSP::e::FileType::FT_flt now works correctly, previously it was the same as for DSP::e::FileType::FT_flt_no_scaling
 *   - ADDED DSPu_FILEoutput::WriteSegmentFromBuffer
 *   .
 * - ver. 0.11.021 - <b>2008.07.13</b>
 *   - ADDED New type: DSPu_buffer_callback_ptr - DSPu_OutputBuffer callback function
 *   - ADDED DSPu_FILEinput::GetRawBufferSize
 *   - ADDED DSPu_FILEinput::GetFltBufferSize
 *   - ADDED DSPu_FILEinput::ReadSegmentToBuffer
 *   .
 * - ver. 0.11.019 - <b>2008.07.09</b>
 *   - ADDED DSPu_Quantizer - only 1-bit version implemented
 *   .
 * - ver. 0.11.018 - <b>2008.07.08</b>
 *   - CHANGED type DSPu_callback_ptr used in MyFunction and DSPu_OutputBuffer
 *      number of input and outputs are now of the type <unsigned int>
 *   - ADDED new constants DSP::c::Callback_Init and DSP::c::Callback_Delete
 *     identifying initial and final call to callback function.
 *   - CHANGED example callbacks.cpp
 *   .
 * - ver. 0.11.017 - <b>2008.07.08</b>
 *   - CHANGED DSP::Component::DOT_DrawAsMacro now supports separate macro graphs
 *   - CHANGED DSP_clock::SchemeToDOTfile now supports separate macro graphs
 *   .
 * - ver. 0.11.015 - <b>2008.07.07</b>
 *   - ADDED DSP::Macro updated for macro support in DOT graph
 *   - CHANGED DSP_clock::SchemeToDOTfile now supports macros
 *   .
 * - ver. 0.11.013 - <b>2008.07.06</b>
 *   - CHANGED DSP_clock::SchemeToDOTfile update to prepare for macro support
 *   .
 * - ver. 0.11.012 - <b>2008.07.06</b>
 *   - CHANGED updated DSP::Macro concept
 *     - ADDED DSP::Macro::MacroInitStarted
 *     - ADDED DSP::Macro::MacroInitFinished
 *     - ADDED DSP::MacroStack::AddMacroToList
 *     - ADDED DSP::MacroStack::RemoveMacroFromList
 *     - CHANGED DSP::Component now stores information about macros to
 *           which given component belong. This is done only in DEBUG mode.
 *     .
 *   .
 * - ver. 0.11.009 - <b>2008.07.05</b>
 *   - FIXED DSPu_Copy - several fixes
 *   - FIXED DSP::Component::CheckInputsOfAllComponents - improved error detection code
 *   - CHANGED DSP::Component::LogInnerState
 *   - CHANGED DSP::Component::IsOutputConnectedToThisInput to incorporate DSPu_Copy component idea
 *   - CHANGED DSP::Component::SetOutput to incorporate DSPu_Copy component idea
 *   - ADDED DSP::Component::UndefineOutput and DSP::Block::UndefineInput
 *   - CHANGED DSP::Component::DefineOutput now redefines output when name conflict is detected
 *   - CHANGED DSP::Block::DefineInput now redefines input when name conflict is detected
 *   - ADDED basic support for macro components
 *     - DSP::Macro class
 *     - DSP::MacroStack class
 *     .
 *   .
 * - ver. 0.11.004 - <b>2008.07.04</b>
 *   - REMOVED obsolete function  DSP::_connect_class::connect_to_block
 *   - ADDED DSPu_Copy block
 *   - FIXED Error checking in DSP::Component::SetOutput
 *   - CHANGED DSP::Component::SetOutput - implemented support for DSPu_Copy transparent connections
 *   - ADDED new DSP::Component: DSP_CT_copy == DSPu_Copy block
 *   .
 * - ver. 0.11.001 - <b>2008.06.30</b> FIX equivalent to lost 0.11.000 version based on 0.10.018
 *   - FIXED all inputs and output indexing is now always of "unsigned int" type
 *   - CHANGED DSPu_Switch - removed UseOFFstate parameter
 *   - ADDED MaxOutputIndex, MaxInputIndex
 *   - CHANGED DSP::Block::FindOutputIndex_by_InputIndex - now uses FO_TheOnlyOutput and FO_NoOutput
 *   - FIXED DSPu_Vacuum with complex inputs Execute_ptr is now set correctly
 *   .
 * - ver. 0.10.018 - <b>2008.06.14</b>
 *   - ADDED DSPu_LFSR source block
 *   - ADDED DSPu_LFSR_tester processing block
 *   .
 * - ver. 0.10.015 - <b>2008.06.06</b>
 *   - ADDED DSP_clock::SetSamplingRate
 *   - ADDED DSP_clock::GetSamplingRate
 *   .
 * - ver. 0.10.014 - <b>2008.06.04</b>
 *   - ADDED  Notification are now present in dot graph generation
 *   - ADDED  multirate blocks that are not source nor registered
 *            for notification have the parent clock indicated
 *   .
 * - ver. 0.10.012 - <b>2008.05.31</b>
 *   - CHANGED DSPu_RawDecimator - redesigned processing
 *   - FIXED DSP_clock::Execute - corrected procedure for clocks swaping when sources are not ready
 *   .
 * - ver. 0.10.010 - <b>2008.05.30</b>
 *   - CHANGED DSPu_Demultiplexer is now a regular block instead of mixed block
 *   - FIXED   DSPu_Demultiplexer outputs names indexing
 *   - FIXED   DSPu_Multiplexer inputs and outputs names indexing
 *   - FIXED   DSPu_Demultiplexer can now work with DSPu_Multiplexer
 *   .
 * - ver. 0.10.009 - <b>2008.05.28</b>
 *   - FIXED DSPu_Multiplexer IsComplex is now interpreted correctly
 *   - FIXED DSPu_Multiplexer State now reserves place for all input samples instead of just one
 *   - FIXED DSPu_Multiplexer::OutputExecute <= now outputs all samples instead of just one
 *   .
 * - ver. 0.10.008 - <b>2008.05.10</b>
 *   - ADDED DSP::Rand - base class for components using random generator.
 *   .
 * - ver. 0.10.007 - <b>2008.05.07</b>
 *   - CHANGED When MasterClock is destroyed but clocks realted to this one still
 *             exist the new one is elected.
 *   - CHANGED Slots in MasterClocks table now can reused.
 *   - UPDATE Several minor updates to code and documentation.
 *   .
 * - ver. 0.10.006 - <b>2008.05.07</b>
 *   - CHANGED updated inputs and outputs names for DSPu_Multiplexer and DSPu_Demultiplexer.
 *   .
 * - ver. 0.10.005 - <b>2008.05.05</b>
 *   - CHANGED redesigned DOT file generation algorithm.
 *   - CHANGED DOT support: components use shape=records.
 *   .
 * - ver. 0.10.002 - <b>2008.04.30</b>
 *   - ADDED class DSP_Clock_trigger - must be used as a base class for components
 *           activating signal activated clocks.
 *   - CHANGED DSP_clock::GetAlgorithmClocks can now also list signal activated clocks.
 *   - ADDED DSP_clock::SchemeToDOTfile.
 *   .
 * - ver. 0.10.000 - <b>2008.04.26</b>
 *   - CHANGED updated clocks processing procedure in DSP_clock::Execute
 *   - CHANGED DSP_clock::Execute - if no source have been processed for active clocks
 *             signal activated clocks will be processed before next attempt instead of
 *             giving up.
 *   - CHANGED DSP_clock::Execute - preparations for next clock cycle only if
 *             previous processing have been succesfull.
 *   - CHANGED DSP_clock::Execute - signal activated clocks processing can be postponed
 *             until other clocks are processed.
 *   .
 * - ver. 0.09.006 - <b>2008.04.22</b>
 *   - CHANGED DSPu_MyFunction - redefined "out" and "in" inputs.
 *   .
 * - ver. 0.09.005 - <b>2008.04.21</b>
 *   - FIXED DSPu_OutputBuffer
 *     - buffer state was incorrectly updated when place for new sample was needed
 *     - with full buffer place for new sample was prepared before notification call
 *     - new place for input sample was prepared even DSPu_OutputBuffer::ReadBuffer
 *       was called in notification with reset = -2
 *     .
 *   .
 * - ver. 0.09.004 - <b>2008.04.20</b>
 *   - CHANGED DSP::Component::ListOfAllComponents additional info is displayed
 *     - component index
 *     - number of component inputs and outputs
 *     - "mixed" sources are now marked as mixed insted of "source"
 *     - automaticaly created blocks (DSPu_Splitter) are now marked as "auto" instead od "block"
 *     .
 *   .
 * - ver. 0.09.003 - <b>2008.04.17</b>
 *   - CHANGED renamed DSP_PSK_type and DSP_SampleType to DSPe_PSK_type and DSP::e::SampleType.
 *   - CHANGED DSPu_OutputBuffer class - now notification are based on clock notifications
 *     - constructors' parameters changed.
 *     - added possibility to use signal activated clock for callback function control.
 *     - DSPu_OutputBuffer::ReadBuffer reset parameter changed.
 *     - added additional calls to notification function (at the block construction and destruction).
 *     .
 *   - CHANGED DSPu_InputBuffer - added notifications support.
 *   .
 * - ver. 0.09.000 - <b>2008.04.16</b>
 *   - CHANGED Moved notification support from DSP::Source to DSP::Component.
 *             Now each block not only source can be registered for notifications.
 *   - CHANGED Blocks for notifications are no longer stored in the same table as sources for processing.
 *   - CHANGED Old DSP_clock::Execute removed and is replaced by DSP_clock::Execute2.
 *             There is no DSP_clock::Execute2 available use DSP_clock::Execute instead.
 *   - CHANGED Updated multirate.cpp example.
 *   .
 * - ver. 0.08.028 - <b>2008.04.15</b>
 *   - ADDED DSPu_ClockTrigger - now "act" signal can determine number of clock cycles
 *     for given clock activation.
 *   .
 * - ver. 0.08.027 - <b>2008.04.13</b>
 *   - CHANGED DSPu_FILEoutput removed samples to skip parameter from constructor.
 *      The same result can be optained with DSPu_FILEoutput::BlockOutput or
 *      with signal activated clock.
 *   - FIXED DSPu_FILEoutput::ReOpen and DSPu_FILEoutput::BlockOutput support fix.
 *   .
 * - ver. 0.08.026 - <b>2008.04.13</b>
 *   - CHANGED removed DSP::Component::ProtectOutputClock, new rules in clocks correctness checking apply now:
 *     -# output clocks of sources and mixed blocks won't be automaticly cleared
 *     -# output clock correctness checking is based on DSP::Block::L_factor and DSP::Block::M_factor
 *     -# DSP::Block::L_factor and DSP::Block::M_factor equal -1 mean that input and output clocks
 *        are asynchronous and output clocks cannot be determined based on input clocks.
 *     .
 *   - ADDED DSP::Block::GetMultirateFactorsFromClocks
 *   - CHANGED DSP::Block::SetBlockInputClock - if clocks differ
 *     MasterClockIndex is also displayed to help identifying
 *     asynchronous clocks.
 *   - ADDED DSPu_FILEoutput::ReOpen.
 *   - In release mode DSP_clock::ListOfAllComponents is completely silent now
 *   - CHANGED modifications to DSPu_FILEoutput
 *     - ADDED DSPu_FILEoutput::DSPu_FILEoutput(unsigned char NoOfChannels)
 *     - ADDED DSPu_FILEoutput::ReOpen
 *     - ADDED DSPu_FILEoutput::BlockOutput
 *     .
 *   .
 * - ver. 0.08.022 - <b>2008.04.10</b>
 *   - CHANGED Moved IsMultirate, L_factor and M_factor variables
 *      from DSP::Source to DSP::Block. This allows for multirate
 *      block without need of mixed block implementation, simple
 *      DSP::Block will suffice.
 *   - CHANGER Modified DSPu_callback_ptr function specification. Added Caller pointer argument.
 *   .
 * - ver. 0.08.020 - <b>2008.04.09</b>
 *   - ADDED DSPu_OutputBuffer can now be created with callback function
 *     which can generate output samples
 *   .
 * - ver. 0.08.019 - <b>2008.04.08</b>
 *   - ADDED static DSP::Component::CheckInputsOfAllComponents - check if all blocks inputs have been connected
 *   - ADDED DSPu_Delay - implemented processing for complex and multivalued inputs
 *   - FIX DSPu_Hold - fixed internal state swaping
 *   - UPDATED new implementation of DSPu_SampleSelector
 *   - UPDATED DSPu_DCO - added version with additional output: oscilatior instantaneous normalized frequency
 *   - UPDATED Some documentation updated - mainly user defined components creation rules updated.
 *   .
 * - ver. 0.08.014 - <b>2008.04.04</b>
 *   - ADDED new abstract class DSP::File for common virtual members definitions for file sources
 *   - UPDATED DSPu_FILEinput - added base class DSP::File
 *   - UPDATED DSPu_WaveInput - added base class DSP::File
 *   .
 * - ver. 0.08.013 - <b>2008.04.03</b>
 *   - UPDATED DSPu_WaveInput - added support for 32-bit wav files
 *   - UPDATED DSPu_FILEoutput - added support for 32-bit wav files
 *   .
 * - ver. 0.08.011 - <b>2008.04.01</b>
 *   - ADDED DSPu_Conjugation - complex conjugation block.
 *   - CHANGED DSPu_Amplifier
 *     - added possibility to use complex gain factor
 *      (this allows for amplitude and phase change)
 *     - changed inputs /outputs names
 *     .
 *   .
 * - ver. 0.08.010 - <b>2008.03.31</b>
 *   - ADDED DSPu_Power - given power of input signals (real and complex(only integer nonnegative factors)).
 *   .
 * - ver. 0.08.009 - <b>2008.03.25</b>
 *   - CHANGED DSP_clock::ProcessSources - now sources registered
 *     for notifications are ignored in processing.
 *     OutputExecute function won't be called anymore.
 *   - FIXED Notifications processing fixed
 *   - FIXED Signal Activated clocks processing fixed -> fix in Execute2()
 *   - ADDED Error message if source is registered for notification but the notification function
 *     is not defined corectly.
 *   - FIXED Notification function in DSPu_Hold fixed
 *   - UPDATED Some documentation updates
 *   - CHANGED Renamed DSP_LoadCoef_Info to DSP_LoadCoef
 *   .
 * - ver. 0.08.006 - <b>2008.03.24</b>
 *   - CHANGED "in1", "in2", ... input names for DSPu_Multiplication
 *   - CHANGED DSPu_LoopDelay - added support for multiple input/output lines
 *   - CHANGED DSPu_PCCC - added support for constant inputs
 *   - ADDED DSPu_LoopDelay::SetState
 *   - ADDED new mixed block DSPu_Farrow implementing Farrow structure.
 *   .
 * - ver. 0.08.003 - <b>2008.03.21</b>
 *   - ADDED DSPu_TimingErrorDetector block
 *   - ADDED FILEtype parameter to DSPu_FILEinput
 *   - CHANGED renamed DSP_FileType to DSP::e::FileType
 *   - FIXED DSPu_Addition - fixed numbering of complex inputs
 *   .
 * - ver. 0.08.001 - <b>2008.03.21</b>
 *   - CHANGED DSPu_Addition now can be created with real or complex weighting coefficients.
 *   .
 * - ver. 0.08.000 - <b>2008.03.20</b>
 *   - CHANGED functions SPf_LoadCoefficients_CheckType, DSP::f::LoadCoefficients_CheckSize
 *     and DSP::f::LoadCoefficients_FIR are now obsolete. Use members of DSP_LoadCoef_Info instead.
 *   .
 *
 * - ver. 0.06.xxx - <b>2006.07.10</b> Branches concept removed
 * - ver. 0.06.002 - <b>2006.07.10</b>
 *  - NEW: DSP_clock::Execute2
 *  - CHANGED: DSPu_Hold
 * - ver. 0.06.006 - <b>2006.07.27</b>
 *  - NEW: Signal activated clocks concept replaces obsolete Branches idea
 *  - NEW: Signal activated clocks support in DSP_clock::Execute2
 *  - NEW: DSPu_ClockTrigger
 * - ver. 0.07.000 - <b>2006.07.31</b>
 *  - NEW: new concept of block input execution method based on pointer to callback function
 *   instead of function overloading
 *  - NEW: new concept of source output execution (the same as for block input execution)
 * - ver. 0.07.003 - <b>2006.08.02</b>
 *  - CHANGED: DSPu_SampleSelector - is no longer a mixed block
 *   - OutputClock and ActivateOutputClock parameters
 *   - Can work with OR as DSPu_ClockTrigger
 *  - ADDED: DSPu_SampleSelector obsolete contructor compatible with previous version
 * - ver. 0.07.004 - <b>2006.08.04</b>
 *  - CHANGED: void DSP::Component::SetNoOfOutputs(int No, bool reset)
 *   - additional parameter: reset (default = true)
 *   - if reset is false previous entries in OutputClocks and OutputBlocks
 *     are preserved if possible
 *   .
 *  - CHANGED: DSPu_GardnerSampling
 *   - previous constructor version is now obsolete (left only for backward code compatibility)
 *   - new DSPu_GardnerSampling block properties
 *    -# optional output - input sample symbol time offset (for eyediagram)
 *    -# optional output signal for output clock activation
 *    -# optional output clock activation
 *    .
 *   .
 *  .
 * - ver. 0.07.010 - <b>2006.08.05</b>
 *  - CHANGED: DSPu_GardnerSampling
 *    -# new processing procedure
 *    -# "offset" output now works
 *    -# output samples are now generated half symbol rate earlier
 *  .
 * - ver. 0.07.012 - <b>2006.08.07</b>
 *   - ADDED: DSP_complex::multiply_by_conj
 *   .
 * - ver. 0.07.014 - <b>2006.08.08</b>
 *   - ADDED: DSPu_OutputBuffer::NoOfSamples
 *   .
 * - ver. 0.07.018 - <b>2006.08.11</b>
 *   - CHANGED: DSPu_Delay - added option IsBufferCyclic (default = true)
 *    -# DSPu_CyclicDelay is now OBSOLETE
 *    -# added macro DSPu_CyclicDelay to provide backward compatibility
 *   .
 * - ver. 0.07.022 - <b>2006.08.12</b>
 *   - CHANGED: constant inputs support concept
 *    -# DSP::Block::IsUsingConstants is set only when DSP::Block::SetConstInput is used
 *    -# DSP::Block::BlockAllowsForConstantInputs is set by DSP::Block::SetNoOfInputs
 *    -# constant specific memory is reserved only when any of inputs is actually constant
 *   - CHANGED: DSPu_DDScos::OutputExecute splited into separate case to improve performance
 *   .
 * - ver. 0.07.025 - <b>2006.08.12</b>
 *   - CHANGED: subsequent DSP::_connect_class::splitconnect call will increase number
 *    of auto createf DSP_Splitter outputs instead to adding new one.
 *    Should increase performance.
 *   .
 * - ver. 0.07.029 - <b>2006.08.13</b>
 *   - ADDED: DSPu_DynamicCompressor
 *   .
 * - ver. 0.07.031 - <b>2006.08.13</b>
 *   - CHANGED:
 *    -# DSP::Component::Convert2Block is now public function
 *    -# DSP::Component::Convert2Source is now public function
 *   .
 * - ver. 0.07.033 - <b>2006.08.13</b>
 *   - FIXED: DSPu_AudioInput - returning WAVEbuffer into the queue we skiping audio frame
 *   .
 * - ver. 0.07.037 - <b>2006.08.14</b>
 *   - CHANGED: Recoded DSPu_FILEoutput
 *   .
 * - ver. 0.07.040 - <b>2006.08.19</b>
 *   - CHANGED: Recoded DSPu_FILEoutput
 *    -# Added support for WAV files (DSP::e::FileType::FT_wav)
 *   .
 * - ver. 0.07.042 - <b>2006.08.19</b>
 *   - CHANGED: DSP_clock::ListComponents(void) lists now components with NULL InputClocks[0]
 *   - ADDED:   DSP_clock::ListOfAllComponents - lists all existing components (regardless of related clock)
 *   .
 * - ver. 0.07.047 - <b>2006.08.20</b>
 *   - CHANGED: DSP::f::InfoMessage - support for source == NULL.
 *     DSP::f::InfoMessage() enters new line now.
 *   - CHANGED: DSPu_Amplifier - added support complex and multivalued inputs.
 *   - CHANGED: DSPu_FILEoutput
 *    -# InputExecute_uchar - input values limited to <-1.0, +1.0>
 *    -# InputExecute_short - input values limited to <-1.0, +1.0>
 *    .
 * - ver. 0.07.054 - <b>2006.09.01</b>
 *   - ADDED: DSPu_WaveInput::GetSamplingRate(void)
 *   .
 * - ver. 0.07.055 - <b>2006.09.05</b>
 *   - ADDED: DSP_clock::FreeClocks(DSP_clock_ptr Reference clock) is now implemented
 *   .
 * - ver. 0.07.056 - <b>2006.09.06</b>
 *   - NEW: First version compiling under Linux. No support for
 *    -# DSPu_AudioOutput
 *    -# ASPu_AudioInput
 *    -# T_AudioMixer
 *   .
 * - ver. 0.07.058 - <b>2006.09.07</b>
 *   - FIXED: DSPu_WaveInput - info message in DEBUG mode when file does not exist
 *   - FIXED: DSP::f::ErroMessage - "Press ENTER" message in console mode
 *   .
 * - ver. 0.07.075 - <b>2006.09.08</b>
 *   - ADDED: DSPu_Accumulator
 *   - ADDED: DSPu_Differator::SetInitialState
 *   - ADDED: DSPu_Const
 *   - ADDED: DSP::f::NoOfErrors(bool Reset)
 *   - NEW: New simplified OutputClock setup and source's registration scheme
 *     - ADDED: DSP::Source::RegisterOutputClock(DSP_clock_ptr, int)
 *     - ADDED: DSP::Source::RegisterClockForNotification(DSP_clock_ptr)
 *     .
 *   - NEW: New DSPu_FILEoutput options
 *     - ADDED: DSP::e::FileType::FT_flt_no_scaling option (integer sample type are not scaled
 *         to cover range [-1.0, 1.0)
 *     - ADDED: DSP::e::SampleType::ST_int
 *   .
 * - ver. 0.07.079 - <b>2006.09.24</b>
 *   - ADDED: new time window generation funtions (see \ref wind_fnc)
 *   - CHANGED: time windows normalisation (window was normalised by mean value
 *              instead of sum of all samples)
 *   - NEW: functions
 *    - DSP::f::sinc(DSP_float)
 *    - DSP::f::sinc(int, DSP_float_ptr)
 *   - CHANGED: removed some remainings of branches concept
 *   .
 * - ver. 0.07.081 - <b>2007.05.12</b>
 *   - CHANGED: DSP_Fourier::absDFTR
 *   - CHANGED: DSP_Fourier::absFFTR
 *   .
 * - ver. 0.07.082 - <b>2007.05.13</b>
 *   - CHANGED: DSPu_OutputBuffer - added notification callback function support
 *   .
 * - ver. 0.07.083 - <b>2007.05.17</b>
 *   - CHANGED: DSPu_OutputBuffer
 *     - added CyclesToSkip_in in constructor
 *     - added SetCyclesToSkip() function
 *     - added SetCyclesToSkip_counter() function
 *     .
 *   .
 * - ver. 0.07.084 - <b>2007.05.17</b>
 *   - CHANGED: DSPu_OutputBuffer
 *     - changed CyclesToSkip_in into NotificationsStep_in in constructor
 *     - changed SetCyclesToSkip() into SetNotificationsStep() function
 *     - changed SetCyclesToSkip_counter() function, now it also updates NotificationsStep_counter
 *     - changed ReadBuffer for cyclic buffer in reset mode.
 *       Now it makes room only for new samples till next Notification
 *       preserving last previously read ones. (Good for overlapping processing).
 *     .
 *   .
 * - ver. 0.07.088 - <b>2007.05.18</b>
 *   - Fixed: DSPu_OutputBuffer::NoOfSamples function
 *   .
 * - ver. 0.07.090 - <b>2007.06.02</b>
 *   - CHANGED DSP::_connect_class::connect(DSP_output_ptr, DSP_input_ptr, bool)
 *    - Added DSP::_connect_class::splitconnect behaviour.
 *    - Changed default behaviour to DSP::_connect_class::splitconnect if output is used.
 *    - AllowSplit == false returns old behaviour where error is issued is output is already used.
 *    .
 *   - CHANGED DSP::_connect_class::connect:
 *     - bool DSP::_connect_class::connect(DSP_output_ptr output, DSP::Block_ptr DestinationBlock, int InputNo=0);
 *     .
 *    to
 *     - bool DSP::_connect_class::connect_block(DSP_output_ptr output,  DSP::Block_ptr DestinationBlock, int InputNo=0);
 *     .
 *   .
 *
 * - ver. 0.07.094 - <b>2007.07.31</b>
 *   - CHANGED DSPu_PSKencoder
 *    - Now complex output even for BPSK modulations.
 *    - Added  DSP_QPSK_A and DSP_QPSK_B modulations support.
 *   .
 *   - ADDED DSPu_PSKdecoder
 *    - Support for DSP_BPSK, DSP_QPSK_A and DSP_QPSK_B.
 *   .
 * .
 *
 * - ver. 0.07.100 - <b>2007.10.29</b>
 *   - CHANGED TAudioMixer
 *    - Provided support for audio cards, which use
 *      separate mixer devices for input and output lines.
 *    - Attempt to provide some support for audio cards with no input volume control.
 *   - FIXED TAudioMixer
 *    - When there is no input mixer control, mute for controls is converted into
 *      activity state correctly
 *   - ADDED TAudioMixer::SetSourceLineVolume function
 *   - ADDED Support for AM_MasterControl in recording controls
 *   - ADDED Memorizing master record control support in TAudioMixer::MemorizeMixerSettings_WAVEIN
 *          and in TAudioMixer::RestoreMixerSettings_WAVEIN (just single channel.
 *   .
 * .
 *
 * - ver. 0.07.101 - <b>2007.10.30</b>
 *   - ADDED Full debugging info logging in TAudioMixer constructor
 *   - FIXED Several fixes in TAudioMixer constructor
 *   .
 * .
 *
 * - ver. 0.07.103 - <b>2007.11.01</b>
 *   - ADDED Wave input/output device index can now be selected in TAudioMixer, DSPu_AudioInput, DSPu_AudioOutput
 *   - ADDED static char *GetWaveInDevName(UINT DevNo=WAVE_MAPPER );
 *   - ADDED static char *GetWaveOutDevName(UINT DevNo=WAVE_MAPPER );
 *   .
 *
 * - ver. 0.07.104 - <b>2008.03.19</b>
 *   - ADDED DSP::f::LoadCoefficients_FIR - added possibility to read files with
 *     multiple impulse responses stored inside.
 *   - FIXED typo in DSP_misc.h : DSP::f::LoadCoefficients_IIR - complex version
 *   .
 *
 *
 * \todo Add to the class DSP_clock
 *    - int RelatesToMasterClock[NoOfMasterClocks]
 *    .
 *  where index of MasterClock to which relates asynchronous clock will be stored.
 *  This information should be exploited in DSP_clock::FreeClocks(DSP_clock_ptr Reference clock)
 *  function to free related asynchronous clocks together with parent MasterClock.
 *   - find clocks related to ReferenceClocks's MasterClock
 *   - call FreeClocks for these clocks
 *   .
 *
 * \todo DSP_clock add relation to DSP_name
 *   - default name "%p" <- pointer
 *   - SetName <- adding user defined clock name
 *   - check if when DSP_clock is deleted all entries in
 *     InputClocks (?OutputClocks) of block are reset to NULL
 *   .
 *
 * \todo DSP_componet::Convert2Source() & DSP_componet::Convert2Block().
 *   Add version DSP_componet::Convert2Source(bool ErrorWhenNULL) & DSP_componet::Convert2Block(bool ErrorWhenNULL)
 *
 * \todo ListComponents(DSP_clock_ptr, bool list_with_NULL) <- additionally list components with NULL clock
 *
 * \todo DSP_clock::ListComponents(NULL) <- list only all orphaned components
 *
 * \todo DSP_clock::ListComponents - print clock infos (pointer, L and M)
 *
 * \todo consider omitting InputClocks memory reservation in release mode
 */
DSP_libver DSP_lib_version(void);
//! Return DSP Engine library version information string.
string DSP_lib_version_string();

/* @} ver_data */


// Examples list
/*!
 *  \example hello.cpp
 *  This example outputs to audio card first channel from test.wav file.
 */
/*!
 *  \example multirate.cpp
 *  This example implements simple multirate DSP algorithm.
 */
/*!
 *  \example callbacks.cpp
 *  This example implements output buffer with callback processing.
 */
/*!
 *  \example processing_block_example.cpp
 *  This code shows exemplary processing block class structure and implementation.
 */
/*!
 *  \example source_block_example.cpp
 *  This code shows exemplary source block class structure and implementation.
 */


/*! \mainpage Digital Signal Processing Engine Documentation
 *
 *  <b>Digital Signal Processing Engine</b> 2005-2021
 *
 *  This project is licensed under GNU General Public License v3.0. 
 *  \ref lib_license
 *
 * \section lib_lic_author Engine author
 *   The author of the <b>Digital Signal Processing Engine</b> library is Marek Blok.
 *
 *   Author's email addresses: \par
 *    Marek.Blok@pg.edu.pl \n
 *    Marek.Blok@eti.pg.edu.pl
 *
 * <b>Digital Signal Processing Engine</b> is the library
 * designed to help in easy and fast DSP multirate algorithms
 * implementation as standalone computer applications.
 *
 * The main idea is to reduce algorithm implementation
 * just to selecting DSP blocks, specifying blocks connections
 * and in case of multirate or asynchronous
 * algorithms, defining clocks relations.
 * In order to make development process even easier
 * and less error prone, in debug mode a range of problems,
 * such as unconnected outputs and inputs or clocks mismatch,
 * is detected and reported to the user.
 *
 * This library is the expansion of the concept of program
 * <b>miCePS</b> (1993-1994) I've created during my last year of studies
 * at Faculty of Electronics, Telecommunications and Informatics
 * (www.eti.pg.gda.pl). The program allowed simple implementation
 * of in-line signal processing with digital feedback loops
 * defined in simple scripts with specifications of
 * processing blocks and their connections.
 *
 * Since my graduation I work at Faculty of Electronics,
 * Telecommunications and Informatics teaching mainly digital signal processing
 * and doing research in that area. My main interest is in digital filters
 * (mainly FIR)
 * and multirate signal processing (mainly for communications applications),
 * however programming is still my hobby. In teaching and research I usually
 * use the MathWorks' MATLAB, however I'm not a fan of Simulink.
 * After getting my PhD I've found some time to rethink the concept
 * of <b>miCePS</b> and have come up with the library that allows
 * for development and debugging of multirate in-line signal processing
 * algorithms with digital feedback loops in stand alone applications.
 * At that moment I think the library is at such a state that
 * could be also useful for other people so I've decided to
 * publish it.
 *
 * Main DSP Engine library advantages
 *  - scheme like implementations of algorithms
 *  - extensive debug info at development stage
 *  - simplicity of algorithm change and expansion (important at development stage)
 *  - openness (library frame work allows for fast addition of new blocks)
 *  - easy interfacing with Octave/MATLAB (with addons)
 *  .
 *
 * If you find this library useful I will be glad to hear about you experience from you.
 * Generally, feel free to send me any propositions for improvements and new features.
 * In the title line add [DSPElib]. \par
 * e-mail: Marek.Blok@eti.pg.edu.pl
 *
 * Future plans
 *  - additional DSP blocks
 *  - built-in filter design routines
 *  - addon for DSP algorithms implementation based on scripts (like in <b>miCePS</b>)
 *  - DSP algorithm automatic C code generation
 *  - visual DSP algorithm presentation
 *  - visual DSP algorithm edition
 *  .
 *
 * \section lib_license Library lincensing information
 *  This project is licensed under GNU General Public License v3.0. 
 *  \ref lib_license_text
 * 
 * \section main_lib_users For library users
 *  - \ref lib_linking
 *  - \ref lib_use_alg
 *  - \ref DSP_units_list_page
 *  - \ref DSP_fnc_list_page
 *  - \ref lib_use_LOG
 *  - Audio input/output (::DSP::f::Sleep, ::DSP::f::SetSleepFunction and wxWidgets)
 *  - Audio mixer (TAudioMixer)
 *  - T_WAVEchunk
 *  - Examples
 *   - \link lib_use_hello_ex Hello world ;-) \endlink
 *   - \link lib_use_mult_ex Multirate algorithm \endlink
 *   - \link lib_use_callback_ex Output buffer callback \endlink
 *   - Asynchronous algorithm
 *   - Separate algorithms
 *   .
 *  - MATLAB addons
 *  .
 *
 * \section main_lib_info About library
 *  - \ref lib_license_text
 *  - \link ver_data  Library version and major updates information \endlink
 *  - \ref DSP_units_list_page
 *  - \ref DSP_fnc_list_page
 *  .
 *
 * \section main_lib_devel For library addons developers
 *  - \ref devel_processing_block_ex
 *  - \ref devel_source_block_ex
 *  - \ref DSP::Block_page
 *  .
 *
 * \page lib_license_text DSPE library license
 *   This project is licensed under GNU General Public License v3.0. 
 * 
 *   \include{doc} "../LICENSE"
 *
 * \page lib_linking Linking with Digital Signal Processing Engine (MinGW)
 *
 *  Here you can find information about compiling and linking with
 *  <b>Digital Signal Processing Engine</b> MinGW  (http://www.mingw.org)
 *  version.
 *
 *  I use Eclipse (http://www.eclipse.org) with CDT (http://www.eclipse.org/cdt)
 *  as development platform and directories are selected for use with
 *  "Managed Make C++ Project".
 *
 *  Linking options given below assume
 *  the following directory structure:
 * \code
 *   .../workspace/project                <<- user project directory
 *   .../workspace/project/_DSP_lib_minGW <<- Digital Signal Processing Engine directory
 * \endcode
 *
 * \section lib_ln_cpp User's source file
 *  \code
 *   #include <DSP_lib.h>
 *  \endcode
 *
 *  Library version can be checked using
 *   - ::DSP_lib_version function which returns DSP_libver structure
 *   - ::DSP_lib_version_string function which returns string with version and copyright
 *     information
 *   .
 *
 * \section lib_ln_base Linking in release mode
 *
 *  \subsection lib_ln_comp Compiler options
 *   \par
 *   -DWIN32 \n
 *   \n
 *   -I../_DSP_lib_minGW/include \n
 *   -I../_DSP_lib_minGW/rls
 *
 *  \subsection lib_ln_linker Linker options
 *   \par
 *   -L../_DSP_lib_minGW/rls \n
 *   \n
 *   -lwinmm \n
 *   -lDSP \n
 *
 * \section lib_ln_base_db Linking in debug mode
 *
 *  \subsection lib_ln_comp_db Compiler options
 *   \par
 *   -DWIN32 \n
 *   \n
 *   -I../_DSP_lib_minGW/include \n
 *   -I../_DSP_lib_minGW/dbg
 *
 *  \subsection lib_ln_linker_db Linker options
 *   \par
 *   -L../_DSP_lib_minGW/dbg \n
 *   \n
 *   -lwinmm \n
 *   -lDSP \n
 *
 *  \note In debug mode \p __DEBUG__ preprocessing option is defined.
 *
 * \section lib_ln_base_wx Using wxWidgets with DSP Engine
 *  In several of my project to present in real time
 *  processing results I've used wxWidgets (http://www.wxwidgets.org)
 *  with OpenGL.
 * \note This is only example at your actual needs might require different set of options.
 *
 *  \subsection lib_ln_wx_comp Compiling wxWidgets with OpenGL enabled.
 *
 *   \code
      path=E:\mingw\bin;E:\msys\1.0\bin\;%path%

      mkdir build-yyyy_mm_dd
      cd build-yyyy_mm_dd
      sh  ../configure --with-msw --disable-compat24 --with-opengl --disable-precomp-headers --enable-optimise --disable-shared --enable-sockets
      make
 *   \endcode
 *
 *  Linking options given below assume
 *  the following directory structure:
 * \code
 *   .../workspace/_wxWidgets_2.6.3_minGW/lib     <<- wxWidgets library directory
 *   .../workspace/_wxWidgets_2.6.3_minGW/include <<- wxWidgets include directory
 *   .../workspace/project                        <<- user project directory
 *   .../workspace/project/_DSP_lib_minGW         <<- Digital Signal Processing Engine directory
 * \endcode
 *  \subsection lib_ln_comp_wx Compiler options
 *   \par
 *   -D__WINDOWS__ \n
 *   -DHAVE_W32API_H \n
 *   -D__GNUWIN32__ \n
 *   -D__WXMSW__  \n
 *   \n
 *   -I../../_wxWidgets_2.6.3_minGW/lib/wx/include/msw-ansi-release-static-2.6 \n
 *   -I../../_wxWidgets_2.6.3_minGW/include
 *
 *  \subsection lib_ln_linker_wx Linker options
 *   \par
 *   -L../../_wxWidgets_2.6.3_minGW/lib \n
 *   \n
 *   -lwx_msw_core-2.6 \n
 *   -lwx_base-2.6 \n
 *   -lwx_msw_gl-2.6 \n
 *   -ladvapi32 \n
 *   -lshell32 \n
 *   -lole32 \n
 *   -loleaut32 \n
 *   -lopengl32 \n
 *   -luuid \n
 *   -lglu32 \n
 *   -lcomctl32 \n
 *   \n
 *   -mwindows
 *
 * \page lib_use_alg Algorithm creation rules
 *
 *  Algorithm creation rules index:
 *   - \ref alg_cr_stages
 *   - \ref alg_cr_clocks
 *   - \ref alg_cr_connect
 *   - \ref alg_cr_unconnected
 *   - \ref alg_cr_clearing
 *   - \ref alg_cr_consts
 *   - \ref alg_cr_feedback
 *   - \ref alg_cr_adv
 *   .
 *
 *   \section alg_cr_stages Algorithm creation stages:
 *    -# The Master Clock creation,
 *    -# Algorithm blocks creation and connections definitions.
 *       At this stage additional clocks related to the Master Clock
 *       might be created,
 *    -# Algorithm execution,
 *    -# Deleting blocks,
 *    -# Freeing clocks.
 *    .
 *    \note Clocks might be freed before blocks deletion (e.g. if
 *    blocks are not created dynamically) but such situation should be
 *    generally avoided.
 *
 *  \section alg_cr_clocks Algorithm clocks
 *    Processing of outputs in sources and mixed blocks is
 *    trigered at corresponding clock's cycle. This means that when
 *    such a block is created its clock must be defined.
 *
 *    For each separate algorithm Master Clock must be created.
 *    If the algorithm is asynchronous or multirate, additional
 *    clocks related to Master Clock will be created.
 *
 *
 *    - <b>Master Clock</b> \n
 *      The first to do thing when new algorithm is defined is
 *      Master Clock creation. This is done by calling function
 *      DSP_clock::CreateMasterClock. This function returns
 *      Master Clock pointer (DSP_clock_ptr) which can be further used
 *      in sources or mixed blocks creation or in related clocks
 *      creation.
 *      \n \n
 *    - <b>Acquiring clocks related to Master Clock</b> \n
 *     Clock related to Master Clock working
 *     L/M times faster then Master Clock are created
 *     created automaticaly when multirate blocks are defined
 *     or can be also defined by user.
 *     - For asynchronous or multirate blocks DSP::Component::GetOutputClock
 *       function returns output clock.
 *       \warning This function should be used with care
 *         because in case of standard blocks, which does not create
 *         their output clocks automaticaly, this function might
 *         return NULL. Note that not at each DSP algorithm creation
 *         stage block's output clock can be resolved.
 *     - When clock in needed before multirate block is defined
 *       user can create it using DSP_clock::GetClock function.
 *     .
 *     \note Conversion using DSP_clock::GetClock
 *           is not possible between asynchrounous clocks.
 *      \n \n
 *    - <b>Signal activated clocks</b> \n
 *      In some situations two parts of the algorithm work asynchronously.
 *      For example in digital demodulators symbol sampling is not
 *      strictly related to the input clock. In such cases signal activated clocks
 *      must be used. To make use of such a clock several things must be taken care of
 *      -# Two asynchronous parts of the algorithm must be connected with special
 *         asynchronous blocks like: DSPu_Hold, DSPu_Farrow.
 *      -# The both parts must have separate master clocks. The signal activated clock
 *         must be generated using DSP_clock::CreateMasterClock (different from that for
 *         main algorithm part) or be related to this clock.
 *      -# Signal activated clock must be activated by special processing block
 *         DSPu_ClockTrigger.
 *      .
 *      \n \n
 *    - <b>DSP algorithm execution</b> \n
 *     DSP algorithm processing is started using following functions:
 *      - \link DSP_clock::Execute DSP_clock::Execute(DSP_clock_ptr ReferenceClock, unsigned long NoOfCyclesToProcess) \endlink
 *      .
 *     One long algorithm execution can me split into several consecutive execute function executions
 *     allowing user to do something else during breaks, e.g. signals visualisation, spectral analysis,
 *     algorithm parameters update, checking finish conditions, etc.
 *     \note
 *       -# As ReferenceClock any clock related to Master Clock can used, not just Master Clock.
 *       -# If NoOfCyclesToProcess equals 0 or is unspecified algorithm will run in infinite loop.
 *          This can be usefull if DSP processing is run as a separate thread.
 *       .
 *     \n
 *    - <b>Freeing clocks</b> \n
 *      When processing is finished and blocks deleted,
 *      all clocks should also be deleted. There are two possibilities:
 *      -# DSP_clock::FreeClocks(void) function which deletes all clocks
 *      -# DSP_clock::FreeClocks(DSP_clock_ptr ReferenceClock) function which deletes all clocks
 *       related to ReferenceClock, e.g. all clocks with the same Master Clock
 *       as ReferenceClock.
 *      .
 *      \note In case of signal activated asynchronous clocks if
 *        DSP_clock::FreeClocks(void) canot be used
 *        Reference clock must be aquired manualy using DSP::Component::GetClock()
 *        get function and freed separately with
 *        DSP_clock::FreeClocks(DSP_clock_ptr ReferenceClock) function.
 *
 *
 *
 * \section alg_cr_connect Connecting blocks
 *  Connections between blocks can be defined using
 *   - bool operator>>( const DSP_output  &output, const DSP_input &input )
 *   - bool operator<<( const DSP_input &input, const DSP_output  &output)
 *   - ::DSP::_connect_class::connect(DSP_output_ptr output, DSP_input_ptr input, bool AllowSplit)
 *   .
 *
 *  In most situations <b>DSP::_connect_class::connect</b> function with AllowSplit == true
 *  (default behaviour) should be used to connect blocks output to other block's input.
 *  Several inputs can be connected using this function to the same output.
 *
 *  <b>DSP::_connect_class::connect</b> function with AllowSplit == false can be only used
 *  to join unconnected output line to unconnected input line. This can help debug
 *  situations when several inputs are connected to single output.
 *  When output line must be connected to several input lines and <b>DSP::_connect_class::connect</b>
 *  with AllowSplit == false is used for the first connection,
 *  for consecutive connections <b>DSP::_connect_class::splitconnect</b> or
 *  <b>DSP::_connect_class::connect</b> with AllowSplit == true must be used.
 *
 *  \note For obvious reasons it's not possible to connect several output lines
 *   to one input line. When such an attempt is detected in the DEBUG mode the
 *   appropriate error message is reported to LOG.
 *
 *  Pointers defining output and input are obtained using
 *   - DSP::Component::Output(char *output_name)
 *   - DSP::Block::Input(char *input_name)
 *   .
 *  where block output/input is identified by its name (see \ref lib_use_hello_ex).
 *  Inputs and outputs names can be found in each block documentation
 *  (in detailed description).
 *  For example see DSPu_Addition where you can find that quite often
 *  inputs can be addressed in more than one way. For example instead
 *  addressing complex input with "cplx_in1", you can access its
 *  real and imaginary parts separately using "cplx_in1.re" and
 *  "cplx_in1.im" names.
 *
 *  \note Inputs and outputs are numbered from 1, e.g. "in1", "in2", ...
 *   (there is no "in0" input).
 *
 *
 * \section alg_cr_unconnected Dealing with unconnected outputs
 *  In DEBUG mode DSP Engine will report unconnected outputs
 *  each time output sample is generated. This helps to detect
 *  situations when somthing has been forgotten. However, quite
 *  often you migth want to discard some of the output lines of
 *  blocks with more then one output line. The way to suppress
 *  error messages is to connect unneeded outputs to
 *  special block DSPu_Vacuum.
 *
 *
 * \section alg_cr_clearing Verification if all components have been deleted
 *  In general it is adviced to created processing blocks dynamically
 *  with C++ <b>new</b> operator. This means that when the processign is
 *  finished all blocks must be freed with <b>delete</b> operator.
 *  During algorithm creation process some of the blocks can be easily
 *  omitted at deletion stage. Therefore the functions DSP_clock::ListOfAllComponents
 *  and DSP_clock::ListComponents have been introduced to help user check
 *  if any of the blocks have been left behind.
 *  \note The best place to call DSP_clock::ListOfAllComponents or DSP_clock::ListComponents
 *    is just before freeing clocks (see \ref alg_cr_clocks).
 *
 *
 * \section alg_cr_consts Constant inputs
 *   Some of the blocks allow to set constat value to the input
 *   instead of connecting some block's output to it.
 *   Use
 *    - DSP::Block::SetConstInput(char *input_name, DSP_float value)
 *      function for real valued inputs
 *    - DSP::Block::SetConstInput(char *input_name, DSP_float value_re, DSP_float value_im)
 *      function for complex valued inputs.
 *    .
 *   In the following example only frequency of DDS will be changed
 *   during processing with amplitude and initial phase constant.
 *   \code
 *     DSPu_DDScos SinGen(Clock1, true);
 *     SinGen.SetConstInput("ampl",1.0); // Set constant amplitude
 *     SinGen.SetConstInput("phase",0.0); // Initial phase set to zero
 *     DSP::_connect_class::connect(Freq.Output("out"), SinGen.Input("in"));
 *   \endcode
 *
 *
 * \section alg_cr_feedback Feedback loops
 *   When creating DSP algorithm special care must be taken of
 *  feedback loops. Generally output cannot be directly connected
 *  to input so special blocks must be used. These blocks are
 *  called here mixed block because they work like a source
 *  but they have also inputs.
 *
 *  Basic block for separating input form output in feedback loops
 *  is the delay block DSPu_LoopDelay. Note that there is also
 *  DSPu_Delay block but it cannot be used in the feedback loop.
 *  However DSPu_Delay should be used in all other cases
 *  because it is implemented in more efficient way than
 *  DSPu_LoopDelay.
 *
 *
 * \section alg_cr_adv Advanced
 *    - DSP::Component vs DSP::Block and DSP::Source
 *     - DSP::Component::Convert2Block
 *     - DSP::Component::Convert2Source
 *     .
 *    .
 */
/*!
 * \page lib_use_hello_ex Hello world example
 *
 *  This example outputs to audio card first channel from test.wav file.
 *
 *  \note It is better idea to dynamically create objects DSPu_WaveInput
 *        and DSPu_AudioOutput.
 *
 *  \include hello.cpp
 *
 *
 * \page lib_use_callback_ex Output buffer callback
 *
 *  This example sends audio input to output buffer
 *  which sends out samples with optional spectrum inversion
 *  to the multiplexer. Then signal is demultiplexed.
 *  odd samples change sign (spectral inversion) and
 *  multiplexed again. Finally signal is send to audio output.
 *
 *  \include callbacks.cpp
 *
 *
 * \page lib_use_mult_ex Multirate algorithm example
 *
 *  This example adds echo to the input audio.
 *
 *  Input works with Fp1 = 22050 Sa/s. \n
 *  Output works with Fp2 = 8000 Sa/s. \n
 *
 *  \warning Before running this example >>LPF_22050_8000.coef<< file
 *    must be generated in >>matlab<< subdirectory. This can be done with
 *    MATLAB script >>multirate_filters.m<<.
 *
 *  \note Too make this example more sophisticated sampling
 *  rate conversion is done twice. First time for sound with echo and
 *  then again in echo loopback.
 *
 *  Input:
 *    -# test.wav file
 *    -# soundcard
 *    -# chirp generator
 *    .
 *
 *  \include multirate.cpp
 *
 *
 * \page devel_processing_block_ex Processing block implementation example
 *
 *  This example show processing block class construction and implementation.
 *
 *  \include processing_block_example.cpp
 *
 *
  * \page devel_source_block_ex Source block implementation example
 *
 *  This example show source block class construction and implementation.
 *
 *  \include source_block_example.cpp
 *
 *
 * \page lib_use_LOG LOG functions and DEBUG info
 *   \section lib_LOG_setup Setting up LOG
 *     - DSP::f::SetLogState(DSPe_LS_Mode Mode)
 *     - DSP::f::GetLogState()
 *     - ::DSPe_LS_Mode
 *     - DSP::f::NoOfErrors(bool reset)
 *     - DSP::f::SetLogState(DSPe_LS_Mode mode)
 *     - DSP::f::SetLogFileName(char *LOG_filename);
 *     - DSP::f::SetLogFunctionPtr(DSP_Message_callback_ptr function_ptr)
 *     - ::DSP_Message_callback_ptr
 *     .
 *   \section lib_LOG_dbg DEBUG info
 *     In DEBUG mode wide range of information about problems encountered
 *     during algorithm setup and running. For example:
 *      - attempt to connect to already used input
 *      - unconnected outputs
 *      - problems like attempt to connect real input to complex output, etc.
 *      - inputs' and outputs' clocks mismatch
 *      - can list all existing blocks (for example to check if there are some not deleted blocks after clean up process)
 *      .
 *   \section lib_LOG_usr User LOG messages
 *     - DSP::f::InfoMessage(char *source, char *message)
 *     - DSP::f::ErrorMessage(char *source, char *message)
 *     - DSP::f::Message(int IsError, char *source, char *message)
 *     .
 *   \section lib_LOG_wx Working with wxWidgets
 *   .
 */


// ***************************************************** //
// ***************************************************** //
/*! \page DSP_units_list_page List of DSP units available in DSP Engine
 *
 *  Last update: DSP_lib ver. 0.08.009 <b>2008.03.25</b> file DSP_lib.h
 *
 *  DSP units are listed in several categories:
 *  - \ref standard_units
 *  - \ref standard_src_units
 *  - \ref inout_units
 *  - \ref multirate_units
 *  - \ref misc_units
 *  - \ref specialised_units
 *
 *  \section standard_units Standard processing units (18)
 *   -# DSPu_ABS calculates absolute value of real or complex sample
 *   -# DSPu_Accumulator implements standard accumulator or accumulator with leakage
 *   -# DSPu_Addition addition block (also weighted summation)
 *   -# DSPu_Amplifier multiplies input value by given constant
 *   -# DSPu_Angle calculates the phase of a complex sample
 *   -# DSPu_CCPC CCPC - Cartesian coordinates to polar coordinates converter
 *   -# DSPu_CMPO CMPO - complex mutual power operator
 *   -# DSPu_Conjugation calculates complex conjugation
 *   -# DSPu_CyclicDelay this block is OBSOLETE use DSPu_Delay instead.
 *   -# DSPu_Delay delay element implemented in processing block mode. Inefficient for large buffer (implemented using memcopy). See DSPu_CyclicDelay.
 *   -# DSPu_Differator Differator - first order backward difference operator
 *   -# DSPu_FIR FIR filter implementation
 *   -# DSPu_IIR IIR filter implementation
 *   -# DSPu_LoopDelay delay element implemented in mixed mode (unit-source). Must be used in digital loopbacks.
 *   -# DSPu_Multiplication multiplication block
 *   -# DSPu_PCCC PCCC - polar coordinated to Cartesian coordinates converter
 *   -# DSPu_Power calculates given power of the input signal (real and complex(only integer nonnegative factors))
 *   -# DSPu_RealMultiplication real multiplication block
 *   .
 *
 *  \section standard_src_units Standard sources (7)
 *   -# DSPu_binrand Generates random binary streams
 *   -# DSPu_Const Generates constant signal
 *   -# DSPu_COSpulse Generates pulse train
 *   -# DSPu_DCO DCO - digitaly controled oscilator
 *   -# DSPu_DDScos Generates real/complex cosinusoid on the basis of DDS
 *   -# DSPu_rand Generates uniform noise
 *   -# DSPu_LFSR Generates binary sequence with linear feedback shift register
 *   .
 *
 *  \section inout_units Input/Output units (8)
 *   -# DSPu_AudioInput Creates object for recording audio
 *   -# DSPu_AudioOutput Creates object for playing audio
 *   -# DSPu_FILEinput Multichannel file input block - sample format can be specified
 *   -# DSPu_FILEoutput Multichannel file output block - sample format can be specified
 *   -# DSPu_InputBuffer Source block providing input from the memory buffer
 *   -# DSPu_OutputBuffer Block providing output to the memory buffer
 *   -# DSPu_Vacuum Block for connecting loose outputs
 *   -# DSPu_WaveInput Creates object for *.wav files reading
 *   .
 *
 *  \section multirate_units Multirate units (6)
 *   -# DSPu_Demultiplexer Demultiplexer block (y1[n]=x[L*n], y2[n]=x[L*n+1], yL[n]=x[L*n+L-1])
 *   -# DSPu_Multiplexer Multiplexer block (y[L*n]=x1[n], y[L*n+1]=x2[n], y[L*n+L-1]=xL[n])
 *   -# DSPu_RawDecimator Decimator without antialias filter
 *   -# DSPu_SampleSelector Outputs some inputs samples on the basis of activation signal
 *   -# DSPu_SamplingRateConversion Sampling rate conversion block
 *   -# DSPu_Zeroinserter Time expansion block: zeroinserter (+ hold)
 *   .
 *
 *  \section async_units Asynchronous units (5)
 *   -# DSPu_ClockTrigger Clock activation based on input activation signal
 *   -# DSPu_GardnerSampling GardnerSampling - sample selection based on Gadner sampling time recovery algorithm
 *   -# DSPu_Hold Reads samples from signal activated clocks (asynchronous) and outputs in synchro with standard clock (synchronous)
 *   -# DSPu_SampleSelector Outputs samples based on input activation signal, can also activate output clock like DSPu_ClockTrigger
 *   -# DSPu_Farrow Implements Farrow structure based FSD filter. Outputs samples delayed by given fractional delay based
 *     asynchronously (based on signal activated clock)
 *   .
 *
 *  \section misc_units Misc units (6)
 *   -# DSPu_CrossSwitch CrossSwitch - sends input signals stright or crossed to outputs
 *   -# DSPu_Maximum Maximum selector. Outputs: (1) maximum value (2) number of input where maximum is observed
 *   -# DSPu_MyFunction User defined function block
 *   -# DSPu_Selector Outputs selected input (given by the number)
 *   -# DSPu_Splitter Outputs input value to multiple outputs
 *   -# DSPu_Switch Gets value from selected input and sends it to the selected output
 *   .
 *
 *  \section specialised_units Specialised units (6)
 *   -# DSPu_AGC AGC - automatic gain control
 *   -# DSPu_BPSK_SNR_estimator Signal to noise ratio estimation for BPSK modulation
 *   -# DSPu_DynamicCompressor Dynamic compressor/decompressor
 *   -# DSPu_PSKdecoder PSK encoder - prepares symbols for PSK modulations
 *   -# DSPu_PSKencoder PSK encoder - decodes symbols for PSK modulations
 *   -# DSPu_TimingErrorDetector Garder timing error detector
 *   .
 *
 * \page DSP_fnc_list_page List of DSP functions available in DSP Engine
 *
 *  Last update: DSP_lib ver. 0.08.009 <b>2008.03.25</b> file DSP_lib.h
 *
 *  \section LOG_fnc LOG related functions
 *   -# DSP::f::GetLogState()
 *   -# DSP::f::ErrorMessage()
 *   -# DSP::f::InfoMessage()
 *   -# DSP::f::Message()
 *   -# DSP::f::NoOfErrors()
 *   -# DSP::f::SetLogFileName()
 *   -# DSP::f::SetLogFunctionPtr()
 *   -# DSP::f::SetLogState()
 *   .
 *  \section time_fnc Timing functions
 *   -# DSP::f::SetSleepFunction()
 *   -# DSP::f::Sleep()
 *   .
 *  \section wind_fnc Time windows
 *   -# DSP::f::Bartlett()
 *   -# DSP::f::Bartlett_Hann()
 *   -# DSP::f::Blackman()
 *   -# DSP::f::Blackman_Harris()
 *   -# DSP::f::Blackman_Nuttall()
 *   -# DSP::f::Flat_top()
 *   -# DSP::f::Gauss()
 *   -# DSP::f::Hamming()
 *   -# DSP::f::Hann()
 *   -# DSP::f::Nuttall()
 *   -# DSP::f::Rectangular()
 *   -# DSP::f::Triangular()
 *   .
 *  \section misc_DSP_fnc Miscellaneous DSP functions
 *   -# DSP::f::sinc(DSP_float)
 *   -# DSP::f::sinc(int, DSP_float_ptr)
 *   .
 *  \section load_fnc Loading coefficients
 *   -# DSP_LoadCoef class for loading coeffients from files
 *     created with MATLAB script >>save_filter_coef.m<<
 *     located in >>toolbox<< subdirectory.
 *     Coefficients file can be accessed with functions:
 *     -# DSP_LoadCoef::Open
 *     -# DSP_LoadCoef::GetNoOfVectors
 *     -# DSP_LoadCoef::GetSize
 *     -# DSP_LoadCoef::Load
 *     .
 *     \note Functions listed below are now OBSOLETE
 *      -# DSP::f::LoadCoeffi1cients_IIR()
 *      -# DSP::f::LoadCoefficients_CheckSize()
 *      -# DSP::f::LoadCoefficients_CheckType()
 *      -# DSP::f::LoadCoefficients_FIR()
 *      -# DSP::f::LoadCoefficients_IIR()
 *      .
 *   -# DSP::f::ReadCoefficientsFromFile()
 *   .
 *  \section math_fnc Math functions
 *   -# DSP::f::gcd()
 *   -# DSP::f::SolveMatrixEqu()
 *   -# DSP::f::SolveMatrixEqu_prec()
 *   .
 *  \section file_fnc Files related functions
 *   -# DSP::f::MakeDir()
 *   -# DSP::f::GetAudioBufferSize()
 *   -# DSP::f::GetWAVEfileParams()
 *   -# DSP::f::SaveVector()
 *   .
 *  \section mod_fnc Modulations related functions
 *   -# DSP::f::BER4BPSK()
 *   -# DSP::f::SER4QPSK()
 *   -# DSP::f::PSK_SNR_estimator()
 *   -# DSP::f::PSK_SNR_estimator2()
 *   .
 *
 */
#endif
