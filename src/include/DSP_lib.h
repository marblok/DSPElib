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
#define DSP_VER_BUILD 22 // !!! without zeroes before, else this will be treated as octal number
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
 *    - DSP::Block::SetConstInput(char *input_name, DSP::Float value)
 *      function for real valued inputs
 *    - DSP::Block::SetConstInput(char *input_name, DSP::Float value_re, DSP::Float value_im)
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
 *   -# DSP::f::sinc(DSP::Float)
 *   -# DSP::f::sinc(int, DSP::Float_ptr)
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
