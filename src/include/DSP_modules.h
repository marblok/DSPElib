/*! \file DSP_modules.h
 * This is DSP engine components and sources definition module header file.
 *
 * \author Marek Blok
 */
#ifndef DSPmodulesH
#define DSPmodulesH

//#include <string.h>
//#include <cmath>
//#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include <string>
#include <sstream>

//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------
#include <DSP_types.h>
#include <DSP_misc.h>

using namespace std;

//#include <DSPclocks.h>

namespace DSP {
  class Clock_trigger;
  class clocks_relation;
  class clock_groups;
  class input;
  class output;

  class name;

  class _connect_class; //private connect functions  

  class Randomization;
}

namespace DSP {
  class Component;
  class Block;
  class Source;

  class MacroStack;
}

namespace DSP {
  namespace u {
    class Copy;
    class Delay;
    class LoopDelay;
    class Splitter;
    class Amplifier;
    class Addition;
    class Power;
    class Multiplication;
    class RealMultiplication;
    class RawDecimator;
    class Zeroinserter;
    class Const;
    class Rand;
    class BinRand;
    class LFSR;
    class LFSR_tester;
    class COSpulse;
    class DDScos;
    class DCO;
    class IIR;
    class FIR;
    class Differator;
    class Accumulator;
    class SamplingRateConversion;
    class Maximum;
    class Selector;
    class SampleSelector;
    class ClockTrigger;
    class CrossSwitch;
    class Conjugation;
    class ABS;
    class Angle;
    class CMPO;
    class PCCC;
    class CCPC;
    class MyFunction;
    class Hold;
    class Demultiplexer;
    class Multiplexer;
    class Quantizer;
  }
}
bool operator>>( const DSP::output &output, const DSP::input &input);
bool operator<<( const DSP::input &input, const DSP::output  &output);

class DSP::_connect_class {
  friend bool ::operator>>( const DSP::output &output, const DSP::input &input);
  friend bool ::operator<<( const DSP::input &input, const DSP::output  &output);
  
  protected:
    static bool connect(const output &output, const input &input, bool AllowSplit = true);
    static bool splitconnect(const output &output, const input &input);
};

//! Object containing informations about specific output of a block
/*! That allows for outputs can be composed of several output lines
 */
class DSP::output
{
  private:
    bool _is_null = false;

    //! null output object
    static DSP::output _null;

    //! output name
    string _name;
  public:

    //! connects the given output to the given input
    /*! returns true if succeeds
     *  Makes use of DSP::Block::SetOutput function
     */
    friend bool ::operator>>( const DSP::output  &output, const DSP::input &input );

    //! read output name
    const string &get_name(void) const;
    //! set output name
    void set_name(const string &name);

    //! pointer to the component with the given output
    DSP::Component_ptr component;
    //! indexes of output lines
    vector <unsigned int> Outputs;

    //! returns null output object
    inline static DSP::output &null(){
      return _null;
    }

    inline bool is_null() const{
      return _is_null;
    }

    output(const bool &create_null = false);
    ~output(void);
};

//! Object containing informations about specific input of a block
/*! This allows for outputs that can be composed of several input lines.
 */
class DSP::input
{
  private:
    bool _is_null = false;

    //! null input object
    static DSP::input _null;

    //! input name
    string _name;

  public:
    //! connects the given output to the given input
    /*! returns true if succeeds
     *  Makes use of DSP::Block::SetOutput function
     */
    friend bool ::operator<<( const DSP::input &input, const DSP::output  &output);

    //! read input name
    const string &get_name(void) const;
    //! set input name
    void set_name(const string &name);

    //! pointer to the component with the given output
    DSP::Component_ptr component;
    //! indexes of input lines
    vector <unsigned int> Inputs;

    //! returns null input object
    inline static DSP::input &null(){
      return _null;
    }

    inline bool is_null() const {
      return _is_null;
    }

    input(const bool &create_null = false);
    ~input(void);
};

//! Object containing information about name of specific object
/*! Fixed <b>2005.10.30</b> This class is empty in release mode.
 *    And in such a case returns NULL as name.
 *
 *  \warning debug libraries must be created with -I../src/include/dbg
 *  \warning release libraries should be created with -I../src/include/rls
 */
class DSP::name
{
  private:
    #if __DEBUG__ == 1
      //! Name of the object
      string ObjectName;
    #endif
  public:
    name(void);
    name(const string &Name);

    //! Sets the name of the block
    void SetName(const string &Name, bool Append=true);
    //! Returns the block's name
    string GetName();
};

// ***************************************************** //
// ***************************************************** //
/*! \page DSP::Block_page DSP::Block and DSP::Source descendant components creation rules
 *
 *  All DSP units implemented in this engine are based on two classes:
 *  DSP::Source and DSP::Block. Both these classes are based on DSP::Component class.
 *  The first class (DSP::Source) has only outputs and its execution is
 *  controlled by one of user defined clocks (DSP::Clock). The second class
 *  (DSP::Block) has both inputs and outputs. The execution of DSP::Block object
 *  takes place when the input value is fed into the object. Strictly speaking
 *  block processing finishes after the last input value has been received
 *  in the current clock cycle.
 *
 *  \note
 *  <b>1.</b> Names of all DSP units classes should start from <b>DSP::u::</b> (e.g. DSP::u::ABS),
 *  where 'u' stands for unit.
 *  \note
 *  <b>2.</b> Names of enumeration types should begin with <b>DSP::e::</b>.
 *  \note
 *  <b>3.</b> Names of auxiliary functions should begin with <b>DSP::f::</b>.
 *  \note
 *  <b>4.</b> Names of auxiliary objects should begin with <b>DSP::o::</b>.
 *
 *
 *  Three types of DSP units can be implemented:
 *  -# Processing unit: processes input values and sends resulting value
 *     to the output (class based on DSP::Block). See: \ref devel_processing_block_ex.
 *  -# Source unit: generates (that also mean reading values from external
 *     sources like sound cards or files) value and sends it to the output
 *     (class based on DSP::Source). See: \ref devel_source_block_ex.
 *  -# Mixed blocks: units which process input samples like
 *   processing units but change sampling frequency or the output value
 *   might be needed before the input is ready. (class based on DSP::Block and DSP::Source).
 *   We can distinguish two types of such units:
 *   - resampling components: the input works with different clock
 *     than the output
 *   - asynchronous components: input and output work with independent clocks;
 *     at least one of them is the signal activated clock. See \ref alg_cr_clocks.
 *   - digital feedback loop separation components: output value might
 *     be needed before the input is received. To this category belong mainly
 *     delay units that are necessary to implement digital feedback loops.
 *   .
 *
 *
 *  \section main_recom_ Main recommendations
 *  The new DSP unit must be descendant class of DSP::Source or DSP::Block or both.
 *  In this class at least following elements must be implemented
 *   - \ref constr_ "constructor" and \ref destr_ "destructor"
 *   - in processing unit: static function <b>bool Execute(int InputNo, DSP::Float value);</b>.
 *     \ref proc_exec_ "See ..."
 *   - in source unit: static function <b>bool Execute(void);</b>
 *     \ref source_exec_ "See ..."
 *   - in mixed block both \b Execute static functions must be implemented
 *     \ref mixed_exec_ "See ..."
 *
 *
 *
 *  \section const_inputs_ Constant inputs support
 *  This engine has implemented basic support for constant inputs.
 *  The main support is provided by function DSP::Block::SetNoOfInputs.
 *  This function sets
 *   - IsConstantInput[InputNo]
 *   - ConstantInputValues[InputNo]
 *   - InitialNoOfInputsProcessed (equals number of constant inputs)
 *   .
 * 	If block supports use of the constant inputs it must:
 *  - In block constructor use DSP::Block::SetNoOfInputs with AllowForConstantInputs == <b>true</b>
 *    Such a block is initialized to support constant inputs and
 *    DSP::Block::IsUsingConstants variable is set to true.
 *  - Block input execution function <b>MUST</b> init DSP::Block::NoOfInputsProcessed variable
 *    with DSP::Block::InitialNoOfInputsProcessed.
 *  - In algorithm creation function use DSP::Block::SetConstInput
 *    to set values to inputs you want to have constant value.
 *    \warning At least one input must be left non-constant.
 *      If all input should be constant use DSP::u::Constant source
 *      block for at least one input.
 *  - If block requires additional processing when constant input is set,
 *    implement descendant function for void DSP::Block::RecalculateInitials (void).
 *    This function is called each time when DSP::Block::SetConstInput is called.
 *    Make use of DSP::Block::IsConstantInput and DSP::Block::ConstantInputValues.
 *  .
 *
 * \code
  void DSP::u::MyBlock_with_constant_inputs::RecalculateInitials (void)
  {
    int ind;

    for (ind=0; ind<NoOfInputs; ind++)
    {
      if (IsConstantInput[ind] == true)
      {
        State[ind] = ConstantInputValues[ind];
      }
    }
  }
  \endcode
 *
 *
 *  \section constr_ Constructor
 *  In constructor there should be:
 *  -# In both processing block and source
 *   - \ref DSP::Block::SetNoOfOutputs "SetNoOfOutputs(Number)" <- declaration that block has
 *    Number outputs. Apart from other initialisation actions this will set DSP::Block::NoOfOutputs variable.
 *    \note two outputs must be declared per one complex valued output.
 *   - \ref #DSP::Block::SetNoOfInputs "SetNoOfInputs(Number)" <- declaration that block has
 *    Number inputs. Apart from other initialisation actions this will set DSP::Block::NoOfInputs variable.
 *    \note two inputs must be declared per one complex valued input.
 *   .
 *  -# Only in source or mixed block
 *   - \link #DSP::Block::IsMultiClock IsMultiClock\endlink = \b false; <- When all outputs are controled
 *    by the same clock
 *   - \link #DSP::Block::IsMultiClock IsMultiClock\endlink = \b true; <- When some outputs are controled
 *    by different clocks (this option is not fully functional yet)
 *   - \link #DSP::Block::OutputClocks OutputClocks\endlink must be filled
 *     and the source must be \ref DSP::Source::RegisterOutputClock
 *     "registered" for this clock. For further details
 *     see \ref source_regist "Registering Source".
 *     If necessary also source notifications should be set here
 *     with DSP::Component::RegisterForNotifications.
 *   .
 *  -# Inputs and outputs names definitions
 *     - DSP::Block::DefineInput
 *     - DSP::Component::DefineOutput
 *     .
 *  .
 *
 *
 *  \section destr_ Destructor
 *   Destructor is optional because following are executed
 *   in \link DSP::Block::~DSP::Block parent destructor\endlink
 *   instead of descendant block destructor or after descendant
 *   block destructor is processed:
 *   - In mixed or source components: \link DSP::Source::UnregisterOutputClocks UnregisterOutputClocks\endlink ();
 *   - \link DSP::Block::SetNoOfOutputs SetNoOfOutputs\endlink (0);
 *
 *   It is only neccessary to free all resources allocated in
 *   descendant block constructor
 *
 *
 *  \section source_regist Registering Source
 *   Each source must register in the clock which sychronises
 *   its output(s) processing. To do this
 *   -# Proper clock must be obtained using DSP+clock::CreateMasterClock or
 *      DSP::Clock::GetClock function. Parent clock is usally passed as the
 *      parameter to the source block contructor.
 *   -# Pointer to this clock must be stored in \link #DSP::Block::OutputClocks
 *      OutputClocks\endlink. \n
 *      If DSP::Source::IsMultiClock == <b>false</b>, it is
 *      enough to set OutputClocks[0]. Otherwise OutputClocks table must be set
 *      for each output.
 *   -# The source must be register to the output clock using
 *      \link DSP::Source::RegisterOutputClock RegisterOutputClock \endlink function.
 *   -# DSP::Component::RegisterForNotification - register clock for source
 *     notifications and begining of the reference clock cycle. See also \ref asynch_block.
 *
 *   Example 1: (standard source or mixed blocks retaining
 *   sampling frequency)
 * \code
   if (ParentClock != NULL)
     OutputClocks[0]=ParentClock;
   else
     OutputClocks[0]=DSP::Clock::GetClock();
   RegisterOutputClock(OutputClocks[0]);
   \endcode
 *
 *   Example 2: (mixed blocks with sampling frequency change)
 * \code
   OutputClocks[0]=DSP::Clock::GetClock(ParentClock, L, M);
   RegisterOutputClock(OutputClocks[0]);
   \endcode
 *
 *
 * \section proc_exec_ Processing block execution function
 *
 * In processing block static member function of the type ::DSP::Block_Execute_ptr
 * for block's input values handling must be implemented.
 * Pointer to this function must be set to DSP::Block::Execute_ptr variable.
 * \code
 * class DSP_test : public DSP::Block
 * {
 *   private:
 *     // ...
 *     static void Execute_test(DSP::Block_ptr block, int InputNo, DSP::Float value, DSP::Component *Caller);
 *   public:
 *     // ...
 *     DSP_test(...);
 *
 * };
 *
 * void DSP_test::Execute_test(DSP::Block_ptr block, int InputNo, DSP::Float value, DSP::Component *Caller)
 * {
 *  / * ... * /
 * }
 *
 * DSP_test::DSP_test(...) : DSP::Block()
 * {
 *   / * ... * /
 *   Execute_ptr = &Execute_test;
 * }
 * \endcode
 *
 * This function is executed when input sample for input number
 * InputNo is ready and its value is given.
 * When all needed input values (if there are more inputs)
 * are ready, the function should calculate output sample
 * end send the output values to next blocks.
 *
 *
 * Example 1:
 * \code
  #define THIS_ ((DSP_test *)block)
  void DSP_test::Execute(DSP::Block_ptr block, int InputNo, DSP::Float value)
  {
    THIS_->OutputBlocks[0]->Execute_1(THIS_->OutputBlocks_InputNo[0], 0.5*value);
  }
  #undef THIS_
  \endcode
 *
 * In above example input value is multiplied by 0.5 and
 * send to first output. Output number OutputBlocks_InputNo[0]
 * of block OutputBlocks[0].
 *
 *
 * Example 2:
 * \code
  #define THIS_ ((DSP_test *)block)
  void DSP_test::Execute_2(int InputNo, DSP::Float value)
  {
    int ind;

    for (ind=0; ind<THIS_->NoOfOutputs; ind++)
      THIS_->OutputBlocks[ind]->Execute(THIS_->OutputBlocks_InputNo[ind], value);
  }
  #undef THIS_
  \endcode
 *
 * In example 2 input value is redistributed to several outputs.
 *
 *
 * Example 3:
 * \code
  #define THIS_ ((DSP_test *)block)
  void DSP_test::Execute_3(int InputNo, DSP::Float value)
  {
    //In general we should check whether each input is executed only once per cycle
    THIS_->State += value;
    THIS_->NoOfInputsProcessed++;

    if (THIS_->NoOfInputsProcessed == THIS_->NoOfInputs)
    {
      THIS_->OutputBlocks[0]->Execute(THIS_->OutputBlocks_InputNo[0], THIS_->State);
      THIS_->State=0.0;
      THIS_->NoOfInputsProcessed=THIS_->InitialNoOfInputsProcessed;
    }
  };
  #undef THIS_
  \endcode
 *
 * In example 3 values from several inputs are added up
 * and then sent to the output block.
 * Two additional variables declared in block are used:
 * \code
  DSP::Float State;
  unsigned int NoOfInputsProcessed;
   \endcode
 *
 * \b State stores temporary output result and \b NoOfInputsProcessed
 * stores number of input values already received.
 *
 * \note Each block can have more then one Execute function.
 *    That way input processing function can be selected
 *    depending on input parameters of block constructor
 *    or later e.g. when some of inputs are set to be constant
 *    or some block parameters are changed.
 *
 *
 *
 * \section source_exec_ Source execution function
 *
 * In source static member function of a type ::DSP::Source_Execute_ptr
 * for output values generation handling must be implemented.
 * Pointer to this function must be set to DSP::Source::OutputExecute_ptr.
 *
 * \code
 * class DSP_test : public DSP::Source
 * {
 *   private:
 *     // ...
 *     static bool OutputExecute_test(DSP::Source_ptr source, DSP::Clock_ptr clock);
 *   public:
 *     // ...
 *     DSP_test(...);
 *
 * };
 *
 * bool DSP_test::OutputExecute_test(DSP::Source_ptr source, DSP::Clock_ptr clock)
 * {
 *   / * ... * /
 * }
 *
 * DSP_test::DSP_test(...) : DSP::Source(ParentClock)
 * {
 *   / * ... * /
 *   OutputExecute_ptr = &OutputExecute_test;
 * }
 * \endcode
 * must be implemented. This function is executed when
 * the clock in which the source output is registered
 * is activated. The clock variable stores the pointer to
 * the clock which executed this function. This variable can
 * be used to distinguish different clocks in sources with mutiple
 * outputs working with different clocks.
 *
 * This function should return false when source is not ready yet,
 * and true when source was ready and sent the output value(s).
 *
 * Example:
 * \code
  #define THIS_ ((DSP_test *)source)
  bool DSP_test::OutputExecute_1(DSP::Source_ptr source, DSP::Clock_ptr clock)
  {
    THIS_->OutputBlocks[0]->Execute(THIS_->OutputBlocks_InputNo[0], 1.0);
    return true;
  }
  #undef THIS_
  \endcode
 *
 * In this example source simply sends 1.0 to first output
 * and returns true. In this case source doesn't need
 * to wait for external resources and is always ready.
 *
 * \note Just like in case of processing blocks each source
 *     can have more then one Execute function.
 *    That way output processing function can be selected
 *    depending on input parameters of source constructor
 *    or later e.g. when some of source parameters are changed.
 *
 * See also DSP::Component::Notify() which can be overloaded if notification
 * at begining of each input clock cycle is required (e.g. in
 * \ref asynch_block "asynchrounous blocks"). However, source must be
 * also registered for notifiations with DSP::Component::RegisterForNotification.
 *
 *
 * \section mixed_exec_  Mixed block execution functions
 *
 * Mixed blocks must have implemented both execution functions.
 * First, static member input processing function
 * of the type ::DSP::Block_Execute_ptr (see \ref proc_exec_)
 * which is neccessary to receive and manage storing input samples.
 * Second, static member output generation function
 * of the type ::DSP::Source_Execute_ptr (see \ref source_exec_)
 * which is used to provide output samples.
 *
 * \warning This two functions must be designed very carrefully.
 * When the block will serve as
 * \ref alg_cr_feedback "digital feedback loop separation component"
 * , it is crucial that the results are the same no mather which function is executed first.
 *
 * \note In case of <b>resampling components</b> we don't need
 * to be so strict, and we can force <b>DSP::Source::OutputExecute_ptr</b>
 * function to be executed second, by returning <b>false</b>,
 * when input values haven't been yet received. This will postpone
 * this function execution. That way we can ensure that function
 * <b>DSP::Block::InputExecute_ptr</b> is executed first.
 *
 * The following example demonstrate how to do it with the use of
 * DSP::Block::NoOfInputsProcessed variable. However, use might in
 * same case might use another variable for indication that
 * all required input samples are ready.
 *
 \code
  #define THIS ((DSP::u::DDScos *)block)
	void DSP::u::DDScos::InputExecute( DSP::Block *  block,
  					unsigned int  InputNo,
  					DSP::Float  value,
  					DSP::Component *  Caller )
 	{
		THIS->NoOfInputsProcessed++;
		switch (InputNo)
		{
			case 0: //amplitude
				THIS->A=value;
				break;
			case 1: //angular frequency
				THIS->frequency=value/DSP::M_PIx2;
				break;
			case 2: //initial phase
				THIS->phase=value/DSP::M_PIx2;
				break;
			default:
				break;
		}
	}

	bool DSP::u::DDScos::OutputExecute_real(DSP::Source_ptr source, DSP::Clock_ptr clock)
	{
		if (THIS->NoOfInputsProcessed < NoOfInputs)
		{ //Not all parameters are already read
			return false;
		}

		// This must be reset
		THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;

		// Generate output sample
		THIS->OutputBlocks[0]->Execute_ptr(
				THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
				THIS->A * cos(DSP::M_PIx2 * (THIS->CurrentPhase + THIS->phase)), source);

		//Update phase for next sample
		THIS->CurrentPhase += THIS->frequency;
		if (THIS->CurrentPhase<0)
		{
			THIS->CurrentPhase -= 0.5;
			THIS->CurrentPhase = fmod(THIS->CurrentPhase, 1.0);
			THIS->CurrentPhase += 0.5;
		}
		else
			if (THIS->CurrentPhase > 0)
			{
				THIS->CurrentPhase += 0.5;
				THIS->CurrentPhase = fmod(THIS->CurrentPhase, 1.0);
				THIS->CurrentPhase -= 0.5;
			}

		return true;
	}
  #undef THIS
 \endcode
 *
 * The overall idea is that InputExecute function
 *  - collects input samples
 *  - when all required samples are colleted, mark them as ready to be processed
 *  .
 * In OutputExecute function
 *  - if InputExecute have not yet collected all necessary samples,
 *    return <b>false</b>. OutputExecute function will be executed
 *    later when other sources will be processed.
 *  - if all required samples are ready
 *    -# generate output samples
 *    -# mark input samples as processed
 *    -# return <b>true</b> to indicate that source has been processed.
 *    .
 *  .
 *
 * \note In some cases source must wait for external events before it is ready
 *   to output samples, even though all other sources are ready. For example
 *   DSP::u::AudioInput source must wait until the soundcard colects audio samples.
 *   In such cases DSP::Clock::InputNeedsMoreTime table value must be set <b>true</b>
 *   for the master clock related to the source output clock before output execute
 *   function returns <b>false</b>.
 *
 * \code
   DSP::Clock::InputNeedsMoreTime [clock->MasterClockIndex] = true;
   return false;
   \endcode
 *
 *
 *
 *  \section multirate_block  Multirate mixed block
 *  	In multirate blocks output clock is L/M times
 *    faster than input clock whre L and M integer
 *    relatively prime factors.
 *
 * 		\note Output clock can be obtained on the basis of input clock
 *    \ref DSP::Clock::GetClock "DSP::Clock::GetClock(InputClock, L_factor, M_factor)".
 *
 *  	DSP::Block::IsMultirate MUST be set to true
 *  	and variables DSP::Block::L_factor and
 *  	DSP::Block::L_factor MUST be set properly.
 *    \note Function DSP::Block::GetMultirateFactorsFromClocks can compute
 *          this factors and determine proper state of DSP::Block::IsMultirate.
 *
 *
 *  \section asynch_block  Mixed block with asynchronous output clock
 * 		If mixed block can have signal activated output clock.
 *    Then input and output processing functions are no longer synchronous
 *    and output clock can no longer be determined on the basis of input clock.
 *
 * 		DSP::Block::IsMultirate MUST be set to true and
 * 		DSP::Block::L_factor and DSP::Block::L_factor variables
 *    should be set to -1.
 *    This will indicate that block is asynchronous.
 *    \note Function DSP::Block::GetMultirateFactorsFromClocks can compute
 *          this factors and determine proper state of DSP::Block::IsMultirate.
 *          If clocks will be detected to be synchronous correct values of L and
 *          M will be set instead of -1.
 *
 * 		For such blocks input samples might be invalid even if
 *    they have not been already processed. In such cases
 *    DSP::Component::RegisterForNotification should be called
 *    in addition to DSP::Source::RegisterClock.
 *    This also requires overloading of DSP::Component::Notify function
 *    which will be called for each clock cycle before any
 *    block processing. That way we can be informed that
 *    input samples are expected in current clock cycle.
 *
 *
 *
 * \section block_misc_ Miscellaneous
 * -# block with multiple real and complex valued
 * equivalent inputs (interchangeable)
 * should have real valued inputs first and then complex valued ones.
 * This does not apply to block where different inputs have
 * different interpretation.
 */

#ifdef __DEBUG__
  namespace DSP {
    extern const vector<string> DOT_colors;
  }
#endif

// ***************************************************** //
// ***************************************************** //
//! This is main class for DSP units
/*! It serves as a base class for both processing blocks
 * and sources.
 *
 * \todo in DSP::Component::IsOutputConnectedToThisInput implement checking whether
 *  each input has only one output connected (with the exception of DSP::u::Vaccum)
 *
 */
class DSP::Component : public virtual DSP::name, public DSP::_connect_class
{
  friend class DSP::Block;
  friend class DSP::Clock;
  friend class DSP::Macro;

/*
  // connects the given output to the given input
  friend bool DSP::_connect_class::connect(const DSP::output &output, const DSP::input &input, bool AllowSplit);
  // connects the given already used output to the given input
  friend bool DSP::_connect_class::splitconnect(const DSP::output &output, const DSP::input &input);
*/
  friend class DSP::_connect_class;

  protected:
  	DSP::e::ComponentType Type;

  public:
    //! converts current object's pointer to DSP::Block if possible
    virtual DSP::Block_ptr Convert2Block(void)
    { return NULL; };
    //! converts current object's pointer to DSP::Source if possible
    virtual DSP::Source_ptr Convert2Source(void)
    { return NULL; };
    //! converts current object's pointer to DSP::File if possible
    virtual DSP::File_ptr Convert2File(void)
    { return NULL; };
    //! converts current object's pointer to DSP::Clock_trigger if possible
    virtual DSP::Clock_trigger_ptr Convert2ClockTrigger(void)
    { return NULL; };
    //! converts current object's pointer to DSP::u::Copy if possible
    virtual DSP::u::Copy_ptr Convert2Copy(void)
    { return NULL; };

  protected:
    //! Dummy block - for unconnected outputs
    /*! Every unconnected output defaults to this block.
     * This block does nothing or in __DEBUG__ mode
     * issues warnings.
     */
    static DSP::Block DummyBlock;


  //*********************************************************************//
  // COMPONENTS REGISTRATION STRUCTURES & FUNCTIONS
  private:
    //! Table in which all the blocks should be registered
    static DSP::Component_ptr *ComponentsTable;
    //! Number of slots reserved in ComponentsTable
    static long int ComponentsTableSize;
    //! Number of blocks registered in ComponentsTable
    static long int NoOfComponentsInTable;
    //! Base number of slots reserved in ComponentsTable
    static const unsigned long ComponentsTableSegmentSize;
    //! returns given component's index in ComponentsTable
    /*! Returns -1 if component not on the list
     */
    static long GetComponentIndexInTable(DSP::Component_ptr component);

  public:
    //! Returns number of blocks registered in ComponentsTable
    static long int GetNoOfComponentsInTable(void)
    {
      return NoOfComponentsInTable;
    }
    //! Returns component's pointer by its index
    static DSP::Component_ptr GetComponent(long int index)
    {
      if (index < NoOfComponentsInTable)
        return ComponentsTable[index];
      return NULL;
    }

    //! List all registered components
    /*! This is for safety precautions to indicate if any components
     * are still registered when we want to call FreeClocks
     *
     * \warning This function works only in DEBUG mode
     *
     * Fixed <b>2006.07.05</b> Added function listing to log all
     * blocks still on the list of the given master clock
     */
    static void ListComponents(void);
    //! Lists all components (if list_outputs == true also lists components outputs)
    static void ListOfAllComponents(bool list_outputs = false);
    //! List all registered components for algorithm related to given clock
    /*! This is for safety precautions to indicate if any components
     * are still registered when we want to call FreeClocks
     *
     * \warning This function works only in DEBUG mode
     */
    static void ListComponents(DSP::Clock_ptr InputClock);
    //! Checks if inputs of all blocks are connected
    static void CheckInputsOfAllComponents(void);
  protected:
    //! Place current block (this) on the list in ComponentsTable
    void RegisterComponent(void);
    //! Remove current block (this) from the list in ComponentsTable
    void UnregisterComponent(void);
  //*********************************************************************//

    //! true when different outputs use different clocks
    bool IsMultiClock;

  protected:
    //! Clocks at which block outputs work
    /*!Depending on IsMultiClock (only for sources) there is only one entry or NoOfOutputs entries valid,
     * however memory is allocated for NoOfOutputs entries. In case of common blocks
     * this is only for the sake of structure syntax checking
     */
    DSP::Clock_ptr *OutputClocks;

  protected:
    //Pointers for the output blocks
    DSP::Block_ptr *OutputBlocks; //!one block pointer for one output
    unsigned int *OutputBlocks_InputNo; //!Input number of the output block
    unsigned int NoOfOutputs; //!number of outputs

    //! breaks connection with output number No
    void ClearOutput(unsigned int No);
    //! Sets number of outputs and initializes component's internal structures accordingly
    /*! Should be used in component's constructor
     *  if <b>reset</b> if false previous entries in
     *    OutputClocks and OutputBlocks are preserved if possible
     */
    void SetNoOfOutputs(unsigned int No, bool reset = true);
    //! Adds additional output
    /*! Returns index of added output.
     *  Block connection is preserved.
     */
    int AddOutputLine(void);


    //! Should be set true if the block must be destroyed automatically
    /*! If AutoFree == true the block was created automatically
     *  (using >>new<<) by DSP_engine itself
     *  and must be freed automatically when the block last connected
     *  to this block is destroyed (in its destructor).
     *
     * Default: false
     *
     * \warning This variable is not set true automatically
     */
    bool AutoFree;

  private:
    //! Is set true is the component is DSP::u::Splitter created automatically
    bool IsAutoSplit;

  protected:
    //! Recalculates initial values
    virtual void RecalculateInitials(void) {};


    //! Checks whether there is any output connected to given block input
    /*! Returns true if there is an block with output connected to
     * input number InputNo of the block OutputBlock.
     *
     * tempBlock  is set to InputBlock
     * and tempNo to its output number.
     *
     */
    static bool IsOutputConnectedToThisInput(
               DSP::Component_ptr OutputBlock, unsigned int InputNo,
               DSP::Component_ptr &tempBlock, unsigned int &tempNo);
    static bool IsOutputConnectedToThisInput2(
               DSP::Component_ptr OutputBlock, unsigned int InputNo,
               DSP::Component_ptr &tempBlock, unsigned int &tempNo);

// ******************************************* //
//  Output connections managment               //
// ******************************************* //
  protected:
    // //! counter containing the number of defined outputs
    //unsigned int DefinedOutputsCounter;
    //! pointer the the array containing pointer to defined outputs
    vector <DSP::output> DefinedOutputs;

  public:
    //! creates output definition for the given block
    /*! Returns true if succeeds
     *  The same output line can be used in several DefinedOutputs.
     *  e.g. separate components of complex output
     * and complex output itself
     */
    bool DefineOutput(const string &Name, const unsigned int &OutputNo = 0);
    bool DefineOutput(const string &Name, const unsigned int &OutputNo_re, const unsigned int &OutputNo_im);
    bool DefineOutput(const string &Name, const vector<unsigned int> &Outputs);
    //! Deletes output definition
    /*! If Name.length() == 0 deletes all output definitions
     */
    bool UndefineOutput(const string &Name);
    //! returns output of the given name
    DSP::output &Output(const string &Name);
    //! returns input of the given name
    virtual DSP::input  &Input(const string &Name);

  protected:
    //! Connects DSP::Block input to output of the current block
    /*! Connects DSP::Block input number InputNo (default = 0)
     *  to first output of the current block.
     *  Inputs numbers start from 0.
     *
     *  When succesful function returns true. When given input
     *  doesn't exists, function does nothing and returns false.
     *
     *
     */
    bool SetOutput(DSP::Component_ptr Component, unsigned int InputNo=0U); //OutputNo==0;
    //! Connects DSP::Block input to output of the current block
    /*! Connects DSP::Block input number InputNo (default=0)
     *  to output number OutputNo of the current block.
     *  Inputs numbers start from 0.
     *
     *  When succesful function returns true. When given input
     *  doesn't exists, function does nothing and returns false.
     *
     *  Might be overloaded in descendant class
     *
     */
    bool SetOutput(unsigned int OutputNo, DSP::Component_ptr Block, unsigned int InputNo=0U);

  public:
    //! Returns clock assigned to this block's output number OutputNo
    DSP::Clock_ptr GetOutputClock(unsigned int OutputNo=0);

    unsigned int GetNoOfOutputs(void);

  protected:
    // ! True if the block output clock shouln't be modified
    /* ! should be true for all source blocks
     *
     * - default -> false
     * - RegisterSource -> sets to true
     * - other blocks such as SampleSelector -> sets to true
     * */
    /*
    bool ProtectOutputClock;
    friend class DSP::u::SampleSelector;
    */

  public:
    //! Resets the block to the initial state but leaves outputs and clocks untouched
    virtual bool Reset(void)
    { return true; };

  public:
    Component(void);
    //! DSP::Component destructor
    /*! Block is released from clocks' lists (only for sources)
     * and memory reserved for outputs is freed
     */
     virtual ~Component(void);

  public:

    #ifdef __DEBUG__
      //! Returns component name used in DOTfile (empty on failure)
      string GetComponentName_DOTfile();
      /*! output_index - index of the rendered output
      */
      virtual string GetComponentEdgeParams_DOTfile(const unsigned int &output_index = 0U);
      //! Returns component node parameters used in DOTfile
      virtual string GetComponentNodeParams_DOTfile();
      //! Returns true if ports should be used for edges
      virtual bool UsePorts_DOTfile(void);
      //! Writes component edges to file
      void ComponentEdgesToDOTfile(std::ofstream &dot_plik, const string &this_name,
          vector<bool> &UsedMacrosTable, vector<DSP::Macro_ptr> &MacrosList, 
          DSP::Macro_ptr DrawnMacro, unsigned int space_sep = 4);
      /*! Returns source name in first_source_name if first_source_name != NULL.
       *  User must reserve memory for it beforehand.
       *
       */
      void ComponentToDOTfile(std::ofstream &dot_plik,
            vector<bool> &ComponentDoneTable, long max_components_number,
            vector<bool> &UsedMacrosTable, vector<DSP::Macro_ptr> &MacrosList,
            vector<bool> &UsedClocksTable, vector<DSP::Clock_ptr> &ClocksList,
            DSP::Macro_ptr DrawnMacro = NULL,
            DSP::Clock_ptr clock_ptr = NULL);
  #endif

  /****************************/
  /* Notifications support    */
  /****************************/
  private:
    vector<DSP::Clock_ptr> NotificationClocks;
  public:
    //! Function called by main clocks' processing function at the begining of each cycle
    /*! It is called only for registered components. It's executed for each
     *  component of the active clocks before any processing is performed.
     */
    virtual void Notify(DSP::Clock_ptr clock)
    {
      UNUSED_ARGUMENT(clock);

      #ifdef __DEBUG__
        DSP::log << DSP::e::LogMode::Error << "DSP::Component::Notify";
        DSP::log << DSP::e::LogMode::second << "Component >>" << GetName() << "<< registered for notifications but notification function not implemented !!!";
        DSP::log << endl;
      #endif
      return;
    }
    /*! Registers component for notifications with given clock.
     *
     * Overloaded functions DSP::Component::Notify() will be
     * called for every NotifyClock cycle before sources processing.
     *
     * Additionaly NotifyClock is stored in NotificationClocks table
     * for use in UnregisterNotifications.
     */
    void RegisterForNotification(DSP::Clock_ptr NotifyClock);
    //! removes block from clocks' notifications tables
    /*! Removes from clocks stored in NotificationClocks
     *  and frees that table.
     */
    void UnregisterNotifications(void);

#ifdef __DEBUG__
  private:
    //! table of macros to which this component belongs
    /*! The most external macro is on the top of the table.
     */
    DSP::Macro_ptr *MacrosStack;
    //! length of table of macros to which this component belongs
    unsigned int  MacrosStackLength;

  public:
    //! Check MacroStack if the components should be drawn or the macro will be drawn instead
    /*! Returns pointer to the macro if it will be drawn instead of the component.
     *  Otherwise returns DrawnMacro.
     *
     * All components belonging to the DrawnMacro
     * (this means they have DrawnMacro on their MacroStack)
     * will be taken into consideration. If DrawnMacro is not
     * on the MacroStack component must be ignored.
     *
     * \note If DrawnMacro == NULL, all components will be taken into
     *  consideration.
     *
     * Returns:
     *  - NULL - component must be drawn (is visible)
     *  - DrawnMacro - component must not be drawn (is invisible - outside the DrawnMacro)
     *  - macro pointer - returned macro must be drawn insted of the component
     *  .
     */
    DSP::Macro_ptr DOT_DrawAsMacro(DSP::Macro_ptr DrawnMacro = NULL);
#endif
};

//! Class for common member functions for processing blocks which use random generator
/*  This class is mainly responsible for random generator initialization.
 *  Prevents multiple generator initializations.
 */
class DSP::Randomization
{
  private:
    //! true if random generator has been initialized
    static bool Initialized;

  public:
    //! \warning this function thus not call InitRandGenerator
    static DSP::Float randu(void);
    //! \warning this function thus not call InitRandGenerator
    static void randu(unsigned int len, DSP::Float_ptr buffer);
    //! \warning this function thus not call InitRandGenerator
    static DSP::Float randn(void);
    //! \warning this function thus not call InitRandGenerator
    static void randn(unsigned int len, DSP::Float_ptr buffer);

    static void InitRandGenerator(bool force);
    Randomization(void);
};

//! Class for common member functions for file processing blocks
/*! \note Derived class not only must define virtual functions but
 *   also should overide implementation od DSP::Component::Convert2File()
 *   This makes posible acces to DSP::File members from pointer to DSP::Component.
 *
 *  \todo In file (input/output) blocks
 *  implement function GetErrorNumber
 *  and GetErrorName
 *  -> this would enable checking after creating
 *     block whether the file was opened
 *     successfully
 *
 */
class DSP::File
{
  protected:
    FILE *FileHandle;
    //! in bits (all channels together)
    unsigned long SampleSize;
    long long skip_counter;

  public:
    //! returns number of bytes read during last file access
    virtual unsigned long GetBytesRead(void) = 0;
    //! returns sampling rate of audio sample
    virtual unsigned long GetSamplingRate(void) = 0;

    DSP::File_ptr GetPointer2File(void)
    { return DSP::File_ptr(this); };

    //! Changes current file position
    /*!
     * @param Offset file position offset in samples
     * @param mode offset mode
     *
     * \todo add byte offset mode
     *
     * \note use only: ftello64, fseeko64. fgetpos, fsetpos
     *   (safe for files larger than 2GB)
     */
    bool SetOffset(long long Offset, DSP::e::OffsetMode mode = DSP::e::OffsetMode::standard);
    //! Set skip counter
    /*!
     * @param Offset number of samples to skip in file processing
     *
     *
     * \note derived classes should use DSP::File::skip_counter variable
     */
    virtual bool SetSkip(long long Offset) = 0;

    File(void)
    {
      FileHandle = NULL;
      skip_counter = 0;

      SampleSize = 0;
    };
    virtual ~File(void){};
};

//! Class for common members for block activating signal activated clocks
/*! \note Derived class not only must define virtual functions but
 *   also should overide implementation od DSP::Component::Convert2ClockTrigger()
 *   This makes posible acces to DSP::Clock_trigger members from pointer to DSP::Component.
 */
class DSP::Clock_trigger
{
  friend class DSP::Clock;

  #ifdef __DEBUG__
  //  friend void DSP::Component::ComponentToDOTfile(std::ofstream &dot_plik,
      //bool *ComponentDoneTable, long max_components_number,
      //DSP::Clock_ptr clock_ptr);
    friend string DSP::Component::GetComponentNodeParams_DOTfile();
  #endif

  protected:
    DSP::Clock_ptr SignalActivatedClock;
    int SignalActivatedClock_NoOfCycles;
    //! clocks index of the master clock for this block
    unsigned int MasterClockIndex;

    //! returns <b>this</b> or NULL if SignalActivatedClock == NULL
    Clock_trigger_ptr GetPointer2ClockTrigger(void);

  public:
    Clock_trigger(void);
    virtual ~Clock_trigger(void){};
};

//! Class containing definition of group of inputs and outputs of a block which operate at a same clock
class clock_group
{
private:
public:
  std::vector<int> InputsIndexes;
  std::vector<int> OutputsIndexes;

  DSP::Clock_ptr GroupClock;

  clock_group(void) : GroupClock(NULL) {};


  //Checks if the given input is in this group
  bool IsInputInGroup(const int &input_no);
};

//! defines resampling ratio between clocks (L==-1 identifies asynchronous clocks, L==0 defined undefined ratio)
class DSP::clocks_relation
{
public:
  long L;
  long M;

  //! default object's value indicates undefined ratio
  clocks_relation(void): L(0), M(0) {};
  clocks_relation(const long &L_in, const long &M_in): L(L_in), M(M_in) {};
};

/*!
 *  Example of use in block constructor:
 *   ClockGroups.AddInput2Group("input", Input("in"));
 *   ClockGroups.AddOutput2Group("output", Output("out"));
 *   ClockGroups.AddInput2Group("output", Input("eps"));
 *   ClockGroups.AddClockRelation("input", "output", L_factor, M_factor);
 */
class DSP::clock_groups
{
public:
  std::map<std::string, clock_group > groups;

//  std::vector<clocks_relation> clocks_relations;
  // clocks_relations[group_name_src, group_name_dest] = clocks_ratio
  std::map<std::string, std::map<std::string, DSP::clocks_relation > > clocks_relations;

  //! returns name of the group to which given input belongs (note: input can belong only to one group)
  std::string FindGroupName4Input(int input_index);
  std::string FindGroupName4Output(int output_index);

  //! returns clock for the given input based on the clock of the reference group with given name
  DSP::Clock_ptr FindClock4Input(const std::string &group_name, const int &input_index);

  //! returns clock for the given output based on the clock of the reference group with given name
  DSP::Clock_ptr FindClock4Output(const std::string &group_name, const int &output_index);

//  /*! dodanie grupy lub nadpisanie zegara istniej�cej grupy
//   */
//  void SetGroupClock(const std::string &name, const DSP::Clock_ptr &clock);

  /*! Adds inputs from index_start to index_end.
   *
   * If necessary creates new clocks group
   */
  void AddInputs2Group(const std::string &name, const int &index_start, const int &index_end);
  void AddInput2Group(const std::string &name, const int &index);
  void AddInput2Group(const std::string &name, const DSP::input &input);
  void AddOutputs2Group(const std::string &name, const int &index_start, const int &index_end);
  void AddOutput2Group(const std::string &name, const int &index);
  void AddOutput2Group(const std::string &name, const DSP::output &output);

  void AddClockRelation(const std::string &parent_group, const std::string &child_group, const long &L, const long &M);

//  /*! dodanie relacji pomi�dzy zegarami dw�ch grup lub nadpisanie istniej�cej relacji
//   */
//  void SetGroupsRelation(const std::string &parents_name, const std::string &childs_name, const int &L, const int &M);
//
//  DSP::Clock_ptr GetChildsClock(const std::string &parents_name);
//  DSP::Clock_ptr GetParentsClock(const std::string &childs_name);
};

// ***************************************************** //
// ***************************************************** //
//! This is main class for DSP processing blocks
/*! \todo_later Source blocks must always check wether the given Clock pointer
 * is valid.
 *
 * \todo_later No NULL clock pointers should be allowed at construction time.
 * User clock should explicitly be defined by the user.
 *
 * Fixed <b>2005.02.13</b> while creating error messages dynamicaly
 * allocated strings (tekst) should be used instead of static ones
 *
 *
 * \todo_later Stworzyc macro CreateDSPblock(Nazwa, parameters);
 * Ktore tworzylo by blok o takiej nazwie oraz funkcje SetName
 * jednoczesnie ustawialoby mu taka nazwe (pytanie co z kontrola poprawnosci
 * parametrow oraz ronymi listami parametrow
 *
 * \todo Precise control other clocks correctness should be established when
 * blocks are connected
 *
 * \todo_later Vectorized complex or real valued input and outputs in blocks
 * Create descendant class from DSP::Blocks
 *
 *
 *
 * \todo_later Check whether this is still actual:
 * DSP_Buffor <- bufor wydajcy dane zgodnie z zegarem wyjsciowym
 *    przyjmujacy dane asynchronicznie (niezalenie od zegara wyjciowego
 *    jezeli wystpi adanie prbki wyjciowej po pobraniu ostatniej
 *    probki wyjciowej na wyjscie powinna by podana ta sama probka co
 *    poprzednio (PROBLEM: zegar wyjciowy moze by wywolany
 *    przed wywolaniem bloczka podajacego warto wejciowa,
 *    ale rowniez moze by tak, ze wartosc wejciowa nie bedzie
 *    podana na wejcie w tym cyklu zegara)
 */
class DSP::Block : public virtual DSP::Component
{
  friend class DSP::Component;
//  friend class DSP::Clock;

//  friend bool DSP::_connect_class::splitconnect(const DSP::output &output, const DSP::input &input);
  friend class _connect_class;

  friend class DSP::Macro;
//  friend void DSP::Macro::MacroEdgesToDOTfile(std::ofstream &, char *, unsigned int);


  public:
    //! converts current object's pointer to DSP::Source is possible
    Block_ptr Convert2Block(void)
    { return (Block_ptr)this; }; //???

// ****************************************** //
//  Input connections management               //
// ****************************************** //
  protected:
    // //! counter containing the number of defined inputs
    //unsigned int DefinedInputsCounter;
    //! pointer the the array containing pointer to defined inputs
    vector<DSP::input> DefinedInputs;

  public:
    //! creates input definition for the given block
    /*! Returns true if succeeds.
     */
    bool DefineInput(const string &Name, const unsigned int &InputNo = 0);
    bool DefineInput(const string &Name, const unsigned int &InputNo_re, const unsigned int &InputNo_im);
    bool DefineInput(const string &Name, const vector <unsigned int> &Inputs);
    //! Deletes input definition
    /*! If Name == NULL deletes all input definitions
     */
    bool UndefineInput(const string &Name);
    //! returns input of the given name
    DSP::input  &Input(const string &Name);

  protected:
    //! Clocks at which each input works
    /*! This is only for the sake of structure syntax checking */
    vector<DSP::Clock_ptr> InputClocks;

    DSP::clock_groups ClockGroups;

    //! Stores for the current block the clock associated with its given input.
    /*! The clock is then compared with clock for the other inputs or outputs
     * (especially mixed blocks outputs). If it is also possible (if the output
     *  clock can be determined) the output clock should be also stored.
     * If the output Clock is updated Input clock of blocks connected to
     * that output should also be updated.
     *
     * This function in general is block dependent.
     */
    virtual void SetBlockInputClock(unsigned int InputNo, DSP::Clock_ptr InputClock);
    DSP::Clock_ptr GetBlockInputClock(unsigned int InputNo);

    //! Number of inputs
    /*! This variable should not be set directly!
     *  Use DSP::Block::SetNoOfInputs instead.
     */
    unsigned int NoOfInputs;

    //! Number if inputs processed in current cycle
    unsigned int NoOfInputsProcessed;

    //! Number of real inputs (these are first inputs)
    unsigned int NoOfRealInputs;
    //! Is first complex input's number odd
    bool IsComplex_real_odd;

    //! Contains number of inputs connected using DSP::Block::SetOutput
    unsigned int NoOfInputsConnected;
    //! Initial number if inputs processed = NoOfInputs - number of constant inputs
    unsigned int InitialNoOfInputsProcessed;
    //! Returns true when the block must be destroyed automaticaly
    /*! True if AutoFree == true and  last input was cleared
     *  or all inputs were allready cleared (NoOfInputs-InitialNoOfInputsProcessed).
     *
     * \warning Must be called only once per input, and only when
     * output connected to the input is cleared
     */
    bool ClearInput(unsigned int InputNo);

    //! True if block is using constant input variables
    /*! DSP::Block::ConstantInputValues and
     * DSP::Block::IsConstantInput
     *
     * \note This variable is set by DSP::Block::SetConstInput.
     *       See also DSP::Block::BlockAllowsForConstantInputs.
     */
    bool IsUsingConstants;
    //! finds index of the input connected to given output of this block
    /*! returns
     *  - output index if block had more than one output
     *  - FO_TheOnlyOutput if it is the only output
     *  - FO_NoOutput if no block was found
     *  .
     */
    unsigned int FindOutputIndex_by_InputIndex(unsigned int InputIndex);
    //! True is block supports use of constant inputs.
    /*! \note This variable is set by DSP::Block::SetNoOfInputs.
     *    See also DSP::Block::IsUsingConstants.
     */
    bool BlockAllowsForConstantInputs;
    //! Values for constant inputs
    vector<DSP::Float> ConstantInputValues;
    //! true is given value is constant
    vector<bool> IsConstantInput;

    //!Sets number of inputs
    /*!Parameters are: number of real inputs, number of complex inputs
     * and bool constant telling whether the block allows constant inputs.
     * This function should be used instead of setting the DSP::Block::NoOfInputs
     * directly.
     */
    void SetNoOfInputs(unsigned int No_real, unsigned int No_complex, bool AllowForConstantInputs);
    //!Sets number of inputs (see: void SetNoOfInputs(int, int, bool))
    void SetNoOfInputs(unsigned int No_real, bool AllowForConstantInputs);


    //! Set if the block is multirate block (output clock differs from input clock)
    bool IsMultirate;
//    //! Interpolation (L) and decimation (M) factor
//    int L_factor, M_factor;

    //! input and output clocks groups definitions with relations between these clocks
    std::vector<clock_group> clocks_groups;

    //! returns true if block is Multirate and fills L and M with proper interpolation and decimation factors
    /*! If clocks are asynchronous L and M will be set -1.
     *  On the other hand is clocks are detected as synchronous
     *  correct values of L and L will be set.
     *
     *  Function output should be set to IsMultirate variable.
     *
     *  If ClocksShouldBeSynchronous is <b>true</b>
     *  error message will be generated if asynchronous
     *  clocks are detected.
     */
    bool GetMultirateFactorsFromClocks(DSP::Clock_ptr InputClock, DSP::Clock_ptr OutputClock,
                                       long &L, long &M, bool ClocksShouldBeSynchronous);


  public:
  	//    bool Get_IsMultiClock(void)
  	//    { return IsMultiClock; }
//  	int Get_L_factor()
//  	{ return L_factor; }
//  	int Get_M_factor()
//  	{ return M_factor; }

  	unsigned int GetNoOfInputs(void);

  	/*! Assigns constant values to selected inputs
     * (output connections to this inputs shouldn't be allowed)
     *  Initial sum value is calculated on this basis
     *  and NoOfInputs processed is also initiated acordingly.
     *
     *  Should be used before connecting any outputs
     *
     * Replaces obsolete function:
     *   bool SetConstInput(int InputNo, DSP::Float value);
     */
    bool SetConstInput(const string &InputName, DSP::Float value);
    /*! Indicates given input as constant value
     *  InputNo   -> real_value
     *  InputNo+1 -> imag_value
     *  (no coutput can be connected to it)
     *
     *  Should be used before connecting any outputs
     *
     * Replaces obsolete function:
     *   bool SetConstInput(int InputNo, DSP::Float real_value, DSP::Float imag_value);
     */
    bool SetConstInput(const string &InputName, DSP::Float real_value, DSP::Float imag_value);
    bool SetConstInput(const string &InputName, DSP::Complex value)
    {
      return SetConstInput(InputName, value.re, value.im);
    }

    //! Checks whether to the given input we can connect output
    /*! Prevents from connecting outputs to constant inputs,
     *  nonexistent inputs and inputs already used by outputs
     *
     * \todo_later add this to description
     */
    virtual bool IsInputAvailable(unsigned int InputNo=0);

  public:
    //! Input processing callback funtion pointer for Processing blocks
    DSP::Block_Execute_ptr Execute_ptr;

    //!Processing command for Processing blocks
    /*! value is put on input number InputNo.
     * If no output connected value redirected to DSP::Block::DummyBlock
     */
    static void DummyExecute(INPUT_EXECUTE_ARGS)
    {
      UNUSED_ARGUMENT(InputNo);
      UNUSED_ARGUMENT(value);

      #ifndef __DEBUG__
        UNUSED_ARGUMENT(block);
      #else
        stringstream tekst;
        if (block == &DummyBlock)
        {
          tekst << "WARNING: Unconnected output detected (" << Caller->GetName() << ")";
        }
        else
        {
          tekst << "WARNING: Block uses DummyExecute. Check block constructor if Execute_ptr is set correctly ("
                << Caller->GetName() << ")";
        }
        DSP::log << DSP::e::LogMode::Error << "DSP::Block::Execute" << DSP::e::LogMode::second <<  tekst.str() << endl;
      #endif
    }

  public:
    //! Basic DSP::Block constructor
    /*! Outputs and input numbers are set to zero.
     */
    Block(void);
    //! Basic DSP::Block destructor
    /*! Block is released from clocks' lists (only for sources)
     * and memory reserved for outputs is freed
     */
    virtual ~Block(void);
};

// ***************************************************** //
// ***************************************************** //
//! This is main class for DSP source blocks
/*! \todo_later Move basic source registration in ParentrClock
 *  from descendant block into DSP::Source itself
 *  so only more complicated cases will need it
 *  done explicitly
 */
class DSP::Source : public virtual DSP::Component
{
  friend class DSP::Component;
  friend class DSP::Clock;

  public:
    //! converts current object's pointer to DSP::Source is possible
    Source_ptr Convert2Source(void)
    { return Source_ptr(this); };

  private:
    // // ! Indicates that source is always ON
    // /* ! This field is taken into account in DSP::Clock::ProcessSources method
    //  */
    // bool IsActive;
    // // ! Indicates that this source is controlled by DSP::u::BranchSelector
    // bool IsControlled;
    // // ! Indicates that this source knows its current state and doesn't need to wait for state update.
    // /* ! It is only valid if IsControlled == true
    //  *
    //  *  \note must be updated after notifycation from DSP::u::BranchSelector
    //  *  and after each source execution
    //  */
    // bool IsSourceStateReady;
    // // ! number of fundamental clock cycles of SourceState validity time
    // /* ! Must be updated each time Controlled source is executed successfully
    //  *  together with IsSourceStateReady.
    //  */
    // long int activation_time_length;
    // // ! Indicates that source is ON when it IsControlled
    // /* ! This field is taken into account in DSP::Clock::ProcessSources method
    //  */
    // bool IsControlledActive;
    // // ! Updates IsSourceStateReady and activation_time_length
    // /* ! \note Must be called after the Controlled source has just been processed
    //  */
    // void UpdateSourceState(void);

  protected:
    //! calls DSP::Component::SetNoOfOutputs and after that performs source specific actions
    void SetNoOfOutputs(unsigned int No);
    //one SourceReady and one OutputClocks pointer per one output
    //If unit is not source OutputClocks[0] == NULL
    bool *SourceReady;

    //! Unregister source from OutputClocks lists
    void UnregisterOutputClocks(void);
    void RegisterOutputClock(DSP::Clock_ptr OutputClock, unsigned int output_index = 0U);

  public:
    //! Output processing callback function pointer for sources
    Source_Execute_ptr OutputExecute_ptr;

    //! Processing command for Source blocks
    /*! returns true if source was ready and processed all commands.
     *  Input variable clock gives pointer to the clock revoking
     *  this function.
     */
    static bool DummyExecute(OUTPUT_EXECUTE_ARGS)
    {
      UNUSED_RELEASE_ARGUMENT(source);
      UNUSED_DEBUG_ARGUMENT(clock);

      #ifdef __DEBUG__
        DSP::log << DSP::e::LogMode::Error << "DSP::Block::Execute"
          << DSP::e::LogMode::Error
          << "WARNING: Source uses DummyExecute. Check source constructor if OutputExecute_ptr is set correctly ("
          << source->GetName() << ")" << endl;
      #endif
      return true;
    }

    // // ! Sets Activates/Deactivates source.
    // /* ! If TurnOn == true the field is set to true
    //  *  and the source will be processed in subsequent clock cycles.
    //  *
    //  * If ParentClockOfTheControler != NULL this source activation is controlled
    //  * by DSP::u::BranchSelector descendant and given state is valid only
    //  * for duration of the ParentClockOfTheControler fundamental cycle.
    //  */
    // void SetState(bool TurnOn=true, DSP::Clock_ptr ParentClockOfTheControler=NULL);

  public:
    //! Basic DSP::Source constructor
    /*! Outputs and input numbers are set to zero.
     */
    Source(void);
    //DSP::Source(DSP::Clock_ptr ParentClock);
    virtual ~Source(void);
};

//! Class storing current stack of macros
/*! On the basis of this stack user
 *  can determine how the macros are nested
 *  at the moment.
 *
 *  \note Macro stack copy is stored with every component.
 *    This information determines to which macros given component belongs.
 *    It is done only in debug mode.
 */
class DSP::MacroStack
{
  friend class DSP::Macro;

  private:
    //! table of active macros
    /*! The most external macro is on the top of the table.
     *
     * Active macro is the macro which is in initialization state.
     */
    static DSP::Macro_ptr *Stack;
    //! length of table of active macros
    static unsigned int  Length;

    //! List of all existing macros
    static DSP::Macro_ptr *List;
    //! length of list of existing macros
    static unsigned int  ListLength;

    //! adds this macro at the bottom of Stack table
    static void AddMacroToStack(DSP::Macro_ptr macro);
    //! removes this macro from Stack table
    /*! macro is expected at the bottom of the stack
     */
    static void RemoveMacroFromStack(DSP::Macro_ptr macro);

    //! adds this macro at the bottom of List table
    static void AddMacroToList(DSP::Macro_ptr macro);
    //! removes this macro from List table
    /*! macro can be anywhere on the list
     */
    static void RemoveMacroFromList(DSP::Macro_ptr macro);

  public:
    //! Returns current macro stack
    /*! Used in DSP::Component constructor to determine
     *  to which macros given component belongs.
     *
     * \note MacrosStack will be overwritten with new value
     *   which will point to the macros table allocated
     *   inside this function.
     *
     * \warning The user must free MacroStack table allocated
     *  by this function on his own.
     */
    static unsigned int GetCurrentMacroStack(DSP::Macro_ptr *&MacrosStack);

    //! Returns current macro list in MacroList
    /*!
     * \note MacrosList will be overwritten with new value
     *   which will point to the macros table allocated
     *   inside this function.
     *
     * \warning The user must free MacroList table allocated
     *  by this function on his own.
     */
    static unsigned int GetCurrentMacroList(vector<DSP::Macro_ptr> &MacrosList);
};

//! User can derive class from this block to group several DSP components into single macro component
/*! Macro component can be used in the same way as any over component,
 *  but it will be implemented as the group of DSP components
 *  defined inside the decendant to DSP::Macro class.
 *
 *  All macro external inputs and outputs must be done
 *  through MacroInput and MacroOutput objects.
 *  User can create his own naming convention.
 *
 *  Macro inputs and outputs can be accesed by
 *    -# External macro input ==> DSP::Macro::Output
 *    -# External macro output ==> DSP::Macro::Input
 *    -# Internal output of the macro input ==> DSP::Macro::InputOutput
 *    -# Internal input of the macro output ==> DSP::Macro::OuputInput
 *    .
 *
 *  \note Creating inputs and outputs naming convention remember that
 *    -# External macro input == input of the MacroInput component
 *    -# External macro output == output of the MacroOutput component
 *    -# Internal output of the macro input == output of the MacroInput component
 *    -# Internal input of the macro output == input of the MacroOutput component
 *    .
 *
 * \todo DOT mode to create scheme with macro as the single component
 *   or as the separate objects being part of the macro object.
 */
class DSP::Macro : public virtual DSP::name
{
  #ifdef __DEBUG__
    friend void DSP::Component::ComponentEdgesToDOTfile(std::ofstream &, const string &,
                      vector<bool> &, vector<DSP::Macro_ptr> &, DSP::Macro_ptr, unsigned int);
  #endif

  private:
    //! true if macro initialization is in progress
    /*! \todo DSP::Component::CheckInputsOfAllComponents must also check
     *   if all macro initialization is finished for all macros
     *   in MacroStack.
     */
    bool MacroInitializationOn;

    unsigned int NoOfInputs;
    unsigned int NoOfOutputs;

  #ifdef __DEBUG__
    private:
      //! determines how macro will be represented on DOT graph
      /*! Default value == DSP::e::DOTmode::DOT_macro_wrap.
       */
      DSP::e::DOTmode DOTmode;
    public:
      //! true if macro must be drawn instead of macro components
      bool DOT_DrawMacro(void);
      //! writes macro's node and edges to the DOT file
      /*! If this == DrawnMacro draw macro input and output blocks
       */
      void MacroToDOTfile(std::ofstream &dot_plik, DSP::Macro_ptr DrawnMacro);
  #endif


  protected:
    DSP::u::Copy_ptr MacroInput_block;
    DSP::u::Copy_ptr MacroOutput_block;

    //! returns internal output of the macro input of the given name
    DSP::output &MacroInput(const string &Name);
    //! returns internal input of the macro output of the given name
    DSP::input &MacroOutput(const string &Name);

  public:
    //! returns macro output of the given name
    DSP::output &Output(const string &Name);
    //! returns external macro input of the given name
    DSP::input &Input(const string &Name);

    //! Returns macro input line number connected to the given macro block input
    /*! Returns FO_NoInput if not connected to the macro input
     */
    unsigned int GetMacroInputNo(DSP::Component_ptr output_block, unsigned int output_block_input_no);

    //! creates input definition for macro input (internal and external)
    /*! Returns true if succeeds.
     */
    bool DefineInput(const string &Name, const unsigned int &InputNo = 0);
    bool DefineInput(const string &Name, const unsigned int &InputNo_re, const unsigned int &InputNo_im);
    bool DefineInput(const string &Name, const vector<unsigned int> &Inputs);
    //! Deletes input definition
    bool UndefineInput(const string &Name = "");

    //! creates output definition for macro output (internal and external)
    /*! Returns true if succeeds
     */
    bool DefineOutput(const string &Name, const unsigned int &OutputNo = 0);
    bool DefineOutput(const string &Name, const unsigned int &OutputNo_re, const unsigned int &OutputNo_im);
    bool DefineOutput(const string &Name, vector<unsigned int> &Outputs);
    //! Deletes output definition
    bool UndefineOutput(const string &Name = "");

    //! Returns clock assigned to macro external output number OutputNo
    DSP::Clock_ptr GetOutputClock(unsigned int OutputNo=0);

  protected:
    //! Function call from macro constructor
    /*! Creates MacroInput and MacroOutput objects
     *  with given input and output lines number.
     *
     *  All user macro components and connections MUST
     *  be after call to this function and before
     *  call to DSP::Macro::MacroInitFinished.
     *
     *  Order of the execution <b>MacroInitStarted</b>
     *    -# Macro stack update (add macro to stack)
     *    -# MacroInput and MacroOutput creation
     *    -# Mark macro initialization as in progress
     *    .
     */
    void MacroInitStarted(void);
    //! Function call from macro initialization finish
    /*!
     *  Order of the execution <b>OnMacroDelete</b>
     *    -# Macro stack update (remove macro from stack)
     *    -# Mark macro initialization as finished
     *    .
     *
     * \note No macro components should be created after call to this function.
     */
    void MacroInitFinished(void);

  public:
    //! Creates macro component basic container
    /*! Macro list update (add macro to the list)
     *
     * \note All macro components should be created in derived class destructor
     *   between calls to DSP::Macro::MacroInitStarted and DSP::Macro::MacroInitFinished.
     */
    Macro(const string &macro_name,
          unsigned int NoOfInputs_in,
          unsigned int NoOfOutputs_in);
    //! Macro container clean up
    /*! Destroys MacroInput and MacroOutput objects.
     *
     *  Macro list update (remove macro from the list)
     *
     * \note All macro components should be deleted in derived class destructor.
     */
    virtual ~Macro(void);

    public:
      void SetDOTmode(DSP::e::DOTmode mode = DSP::e::DOTmode::DOT_macro_wrap)
      {
        UNUSED_RELEASE_ARGUMENT(mode);

        #ifdef __DEBUG__
          DOTmode = mode;
        #endif
      }
      DSP::e::DOTmode GetDOTmode(void)
      {
        #ifdef __DEBUG__
          return DOTmode;
        #else
          return DSP::e::DOTmode::DOT_macro_inactive;
        #endif
      }

  #ifdef __DEBUG__
    private:
      //! Returns macro node parames used in DOTfile
      string GetMacroNodeParams_DOTfile();
      //! used in macro graph to draw input node
      string GetMacroInputNodeParams_DOTfile();
      //! used in macro graph to draw output node
      string GetMacroOutputNodeParams_DOTfile();
      //! Draws macro node edges
      string GetMacroEdgeParams_DOTfile(const unsigned int &output_index);
      bool UsePorts_DOTfile(void);
      //! Writes macro outgoing edges to file
      void MacroEdgesToDOTfile(std::ofstream &dot_plik, const string &macro_name,
          DSP::Macro_ptr DrawnMacro, unsigned int space_sep = 4);
      void MacroInputEdgesToDOTfile(std::ofstream &dot_plik, const string &macro_input_name,
          DSP::Macro_ptr DrawnMacro, unsigned int space_sep = 4);
      void MacroOutputEdgesToDOTfile(std::ofstream &dot_plik, const string &macro_output_name,
          DSP::Macro_ptr DrawnMacro, unsigned int space_sep = 4);

      //! Returns macro name used in DOTfile
      string GetMacroName_DOTfile();
  #endif
};

/**************************************************/
//! Delay element implemented in mixed mode (unit-source)
/*! \warning Necessary for digital feedback loop !!!
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" (vector of all outputs)
 *    -# "out1", "out2", ...
 *    -# "out.re" == "out1"
 *    -# "out.im" == "out2"
 *    .
 *   - Input:
 *    -# "in" (vector of all inputs)
 *    -# "in1", "in2", ...
 *    -# "in.re" == "in1"
 *    -# "in.im" == "in2"
 *    .
 */
class DSP::u::LoopDelay  : public DSP::Block, public DSP::Source
{
  private:
    unsigned int *Delay;
    DSP::Float **State;
    bool *IsOutputProcessed, *IsInputProcessed;
    DSP::Float *tempInput;

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);
    static bool OutputExecute_multi(OUTPUT_EXECUTE_ARGS);

    static void InputExecute(INPUT_EXECUTE_ARGS);
    static void InputExecute_multi(INPUT_EXECUTE_ARGS);

  public:
    LoopDelay(DSP::Clock_ptr ParentClock, unsigned int delay=1U, unsigned int inputs_no = 1U);
    virtual ~LoopDelay(void);

    //! Sets internal state for delay line related to given input
    /*! \param InputName - must be name of real valued input line
     *  \param size - number of elements in state_buffer; must be
     *    equal to size of the delay
     *  \param state_buffer - buffer with initial values
     */
    bool SetState(const string &InputName, unsigned int size, DSP::Float_ptr state_buffer);
    //! Sets internal state for delay line related to given input
    /*! State buffer size (block delay) must equal one.
     * \param InputName - must be name of real valued input line
     *  \param state_buffer_value - buffer with initial values
     */
    bool SetState(const string &InputName, DSP::Float state_buffer_value);
};

//! Delay element implemented in processing mode
/*! \warning Cannot separate processing in digital feedback loop !!!
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" (vector of all outputs)
 *    -# "out1", "out2", ...
 *    -# "out.re" == "out1"
 *    -# "out.im" == "out2"
 *    .
 *   - Input:
 *    -# "in" (vector of all inputs)
 *    -# "in1", "in2", ...
 *    -# "in.re" == "in1"
 *    -# "in.im" == "in2"
 *    .
 */
class DSP::u::Delay : public DSP::Block
{
  private:
    unsigned int delay;
    DSP::Float **State;
    //! current index in buffer
    unsigned int *index;

    //! version for Delay == 0
    static void InputExecute_D0(INPUT_EXECUTE_ARGS);
    static void InputExecute_D0_multi(INPUT_EXECUTE_ARGS);
    //! version for Delay == 1
    static void InputExecute_D1(INPUT_EXECUTE_ARGS);
    static void InputExecute_D1_multi(INPUT_EXECUTE_ARGS);
    //! version with memcpy
    static void InputExecute(INPUT_EXECUTE_ARGS);
    static void InputExecute_multi(INPUT_EXECUTE_ARGS);
    //! version with cyclic buffer
    static void InputExecute_with_cyclic_buffer(INPUT_EXECUTE_ARGS);
    static void InputExecute_with_cyclic_buffer_multi(INPUT_EXECUTE_ARGS);
  public:
    //! DSP::u::Delay block constructor
    Delay(unsigned int delay_in = 1, unsigned int InputsNo = 1, bool IsBufferCyclic = true);
    virtual ~Delay(void);
};

/**************************************************/
//! Outputs input value to multiple outputs
/*! Inputs and Outputs names:
 *  - Output:
 *   -# "out1", "out2", ... (real or complex valued)
 *   -# "out1.re", "out2.re", ... (real components)\n
 *      "out1.im", "out2.im", ... (imag components if exists)
 *  - Input:
 *   -# "in" (real or complex valued)
 *   -# "in.re" (real component)\n
 *      "in.im" (imag component if exists)
 */
class DSP::u::Splitter : public DSP::Block
{
  private:
    static void InputExecute(INPUT_EXECUTE_ARGS);

  #ifdef __DEBUG__
    private:
      bool UsePorts_DOTfile(void);
      string GetComponentNodeParams_DOTfile();
      string GetComponentEdgeParams_DOTfile(const unsigned int &output_index);
  #endif

  public:
    Splitter(unsigned int No=2); //number of outputs
    Splitter(bool IsInputComplex, unsigned int No=2); //number of outputs
    ~Splitter(void);
};

/**************************************************/
//! Gets value from selected input and sends it to the selected output
/*! There can be only one input and/or output. In addition
 *  this unit can be used to break processing line - no output
 *  would be processed.
 *
 *  If UseSelectorInputs is true additional inputs are available:
 *  "input_selector" & "output_selector". Values from these inputs
 *  indicate currently selected input/output.
 *  Inputs/outputs are selected by numbers (floor from the actual input value)
 *  0, 1, 2, ... selects input/ouput number 1, 2, 3 ... (0 means "in1"/"out1" and so on).
 *
 *  \note When input and/or output index is out of range
 *        output sample will not be generated.
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out1", "out2", ... (real or complex valued)
 *   -# "out1.re", "out2.re", ... (real components)\n
 *      "out1.im", "out2.im", ... (imag components if exists)
 *  - Input:
 *   -# "in1", "in2", ... (real or complex valued)
 *   -# "in1.re", "in2.re", ... (real components)\n
 *      "in1.im", "in2.im", ... (imag components if exists)
 *
 * Input/Output numbers must be selected directly using DSP::u::Switch::Select
 *
 * \todo Implement inputs:
 *   -# "input_selector" index of the selected input
 *   -# "output_selector" index of the selected output
 *   .
 */
class DSP::u::Switch : public DSP::Block
{
  private:
    void Init(bool IsInputComplex, //! if true inputs are complex, otherwise real
              unsigned int InputsNo=1U,  //! number of inputs to select from
              unsigned int OutputsNo=1U //! number of outputs to select from
//              bool UseSelectorInputs=false //! if true additional inputs "input_selector" & "output_selector" are available
              );

    //! Number of signal components per input/output (for real == 1, for complex == 2)
    unsigned int ValuesPerOutput;
    //! Index of additional input for input/output selection (FO_NoInput and FO_NoOutput means not available)
    unsigned int InputSelectionInd, OutputSelectionInd;
    //! Index of the last input line (with the exeption of additional inputs for input/output selection)
    unsigned int LastInputInd;
    //! Index of the selected input and output
    unsigned int SelectedInputNo, SelectedOutputNo;
    //! Maximum allowed index of the selected input and output (index start from zero!)
    unsigned int MaxSelectedInputNo, MaxSelectedOutputNo;
    //! Minimum allowed index of the selected input and output (zero or FO_NoInput)
    unsigned int MinSelectedIndexNo;

    // vector for storing input values before all are available and output is generated
    DSP::Float_ptr State;

    /*! \todo_later Consider processing Output as soon as SelectedInputs are avaiable
     * \todo_later Consider storing only SelectedInputs the rest could be ignored
     */
    static void InputExecute(INPUT_EXECUTE_ARGS);

  public:
    Switch(unsigned int InputsNo=1U,  //! number of inputs to select from
                unsigned int OutputsNo=1U //! number of outputs to select from
//                bool UseSelectorInputs=false //! if true additional inputs "input_selector" & "output_selector" are available
                );
    Switch(bool IsInputComplex, //! if true inputs are complex, otherwise real
                unsigned int InputsNo=1U,  //! number of inputs to select from
                unsigned int OutputsNo=1U //! number of outputs to select from
//                bool UseSelectorInputs=false //! if true additional inputs "input_selector" & "output_selector" are available
                );
    ~Switch(void);

    void Select(unsigned int InputIndex, unsigned int OutputIndex);
};

/**************************************************/
//! Multiplies input value by given constant
/*! Inputs and Outputs names:
 *  - Output:
 *   -# "out_all" - all output lines
 *   -# "out" - first output (real or complex)
 *   -# "out.re", /n
 *      "out.im"
 *   -# "out1", "out2", ...  - consecutive outputs (real or complex)
 *   -# "out1.re", "out2.re", .../n
 *      "out1.im", "out2.im", ...
 *  - Input:
 *   -# "in_all" - all input lines
 *   -# "in" - first input (real or complex)
 *   -# "in.re", /n
 *      "in.im"
 *   -# "in1", "in2", ...  - consecutive inputs (real or complex)
 *   -# "in1.re", "in2.re", .../n
 *      "in1.im", "in2.im", ...
 */
class DSP::u::Amplifier : public DSP::Block
{
  private:
    //! index variable for input execute
    unsigned int ind;
    //! temporary variable for storing input for InputExecute_cplx_inputs_with_cplx_factor
    DSP::Float_ptr temp_inputs;

    bool IsGainComplex;
    DSP::Float Coeficient;
    DSP::Complex CplxCoeficient;

    static void InputExecute_one_real_input_with_real_factor(INPUT_EXECUTE_ARGS);
    static void InputExecute_real_factor(INPUT_EXECUTE_ARGS);
    static void InputExecute_real_inputs_with_cplx_factor(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx_inputs_with_cplx_factor(INPUT_EXECUTE_ARGS);

  public:
    Amplifier(DSP::Float alfa, unsigned int NoOfInputs_in = 1, bool AreInputsComplex = false);
    //! Amplifier with complex gain factor (changes amplitude and phase)
    /*! \param  NoOfInputs_in - number of real or complex inputs
     *  \param  AreInputsComplex - if true, inputs are complex
     *  \param alfa - amplification coefficient
     */
    Amplifier(DSP::Complex alfa, unsigned int NoOfInputs_in = 1, bool AreInputsComplex = false);
    ~Amplifier(void);

    //! changes amplification factor
    void SetGain(DSP::Float gain);
    //! changes amplification factor
    void SetGain(DSP::Complex gain);
};

/**************************************************/
//! Calculates given power of the input signal
/*! Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real or complex valued)
 *   -# "out.re", "out.im"
 *   .
 *  - Input:
 *   -# "in" (real or complex valued)
 *   -# "in.re", "in.im"
 *   .
 *  .
 * There are different implementation for
 *  - power 2
 *  - integer power (other than 2)
 *  - power with the real factor (only for real input signals)
 *  .
 */
class DSP::u::Power : public DSP::Block
{
  private:
    int IntFactor;
    DSP::Float RealFactor;

    //! index variable for InputExecute functions
    int ind;
    //! temporary variable for complex input signal
    DSP::Complex in_value;
    //! temporary variable for complex output signal
    DSP::Complex out_value;

    static void InputExecute_Power2_real(INPUT_EXECUTE_ARGS);
    static void InputExecute_Power2_cplx(INPUT_EXECUTE_ARGS);
    static void InputExecute_PowerInt_real(INPUT_EXECUTE_ARGS);
    static void InputExecute_PowerInt_cplx(INPUT_EXECUTE_ARGS);
    static void InputExecute_PowerReal_real(INPUT_EXECUTE_ARGS);
    //static void InputExecute_PowerReal_cplx(DSP::Block *block, int InputNo, DSP::Float value, DSP::Component *Caller);

  public:
    Power(int factor);
    Power(bool IsComplex, int factor);
    Power(DSP::Float factor);
    Power(bool IsComplex, DSP::Float factor);
    ~Power(void);
};

/**************************************************/
//! Addition block
/*! Adds up all input values and outputs the result
 *  Real and complex valued inputs are allowed.
 *  If there is any complex valued input the output is
 *  also complex valued , otherwise the output is
 *  real valued.
 *
 *  Constant inputs can also be defined
 *  using DSP_Addition::SetConstInput().
 *  This should be done before any outputs are connected
 *  to this block inputs.
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real or complex valued)
 *   -# "out.re" (real component)\n
 *      "out.im" (imag component if exists)
 *  - Input:
 *   -# "real_in1", "real_in2", ... - real inputs
 *   -# "cplx_in1", "cplx_in2", ... - complex inputs
 *   -# "cplx_in1.re", ... - real part of complex input
 *   -# "cplx_in1.im", ... - imaginary part of complex input
 *   -# "in1", "in2", ... - real or complex inputs
 *   -# "in1.re", "in2.re", ... - real components of inputs
 *   -# "in1.im", "in2.im", ... - imaginary components of inputs (if they exist)
 *   .
 *   For block that has one real input and two complex inputs
 *   we will have following predefined names:
 *     - "real_in1", "cplx_in1", "cplx_in2"
 *     - "cplx_in1.re", "cplx_in1.im", "cplx_in2.re", "cplx_in2.im"
 *     - "in1", "in2", "in3"
 *     - "in1.re", "in2.re", "in2.im", "in3.re", "in3.im"
 *     .
 */
class DSP::u::Addition : public DSP::Block
{
  private:
//    int NoOfInputsProcessed;
//    int NoOfInputs;
    DSP::Float State_real, State_imag;

    DSP::Float_ptr   RealWeights;
    DSP::Complex_ptr CplxWeights;

    //! Initial Sum value = 0.0 if no constant inputs (real part)
    DSP::Float InitialSum_real;
    //! Initial Sum value = 0.0 if no constant inputs (imaginary part)
    DSP::Float InitialSum_imag;

    //! Standard initialization
    /*! \param ForceCplxOutput - if true forces block output to be complex
     *   even though all inputs are real. This should be used with
     *   complex weights.
     *  \param NoOfComplexInputs_in number of complex valued inputs
     *  \param NoOfRealInputs_in number of real valued inputs
     */
    void Init(unsigned int NoOfRealInputs_in, unsigned int NoOfComplexInputs_in, bool ForceCplxOutput = false);

    //! Recalculates initial values
    void RecalculateInitials(void);

    static void InputExecute(INPUT_EXECUTE_ARGS);
    static void InputExecute_RealWeights(INPUT_EXECUTE_ARGS);
    static void InputExecute_CplxWeights(INPUT_EXECUTE_ARGS);

  public:
    /*! NoOfRealInputs <- number of real valued inputs
     *  first NoOfRealInputs are treated as real valued inputs
     *  this real inputs are followed by complex valued ones
     */
    Addition(unsigned int NoOfRealInputs_in=2, unsigned int NoOfComplexInputs_in=0); //number of inputs
    Addition(unsigned int NoOfRealInputs_in, DSP::Float_ptr weights);
    Addition(unsigned int NoOfRealInputs_in, DSP::Complex_ptr weights);
    Addition(unsigned int NoOfRealInputs_in, unsigned int NoOfComplexInputs_in, DSP::Float_ptr weights);
    Addition(unsigned int NoOfRealInputs_in, unsigned int NoOfComplexInputs_in, DSP::Complex_ptr weights);
    ~Addition(void);
};

/**************************************************/
//! Multiplication block
/*! Multiplies all input values and outputs the result
 *  \todo_later Implement constant inputs (like in DSP_Addition)
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real or complex valued)
 *   -# "out.re" (real component)\n
 *      "out.im" (imag component if exists)
 *  - Input:
 *   -# "in1", "in2", ... - real/complex input (first are real valued inputs)
 *   -# "real_in1", "real_in2", ... - real inputs
 *   -# "cplx_in1", "cplx_in2", ... - complex inputs
 *   -# "cplx_in1.re", ... - real part of complex input
 *   -# "cplx_in1.im", ... - imaginary part of complex input
 *   -# "in1.re", "in1.im", ... - real/complex part of the input
 *     (sequence as for "in1", "in2", ...)
 *   .
 *   For block has one real input and two complex inputs
 *   we will have following predefined names: "real_in1",
 *   "cplx_in1", "cplx_in2" and "cplx_in1.re", "cplx_in1.im",
 *   "cplx_in2.re", "cplx_in2.im".
 *
 */
class DSP::u::Multiplication : public DSP::Block
{
  private:
    DSP::Float_ptr State;
    DSP::Float State_Re, State_Im;
//    DSP::Float temp_re;

    //! Recalculates initial value when constant input is set for this block
    void RecalculateInitials(void);

    static void InputExecute(INPUT_EXECUTE_ARGS);

  public:
    Multiplication(unsigned int NoOfRealInputs_in=2,
        unsigned int NoOfComplexInputs_in=0); //numbers of inputs
    ~Multiplication(void);
};

/**************************************************/
//! Real multiplication block
/*! Multiplies all input values and outputs the result
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real valued)
 *  - Input:
 *   -# "real_in1", "real_in2", ... - real inputs
 *   -# "in1" == "real_in1", ...
 *   .
 *  .
 *
 * \note This block supports constant inputs (see DSP::Block::SetConstInput).
 */
class DSP::u::RealMultiplication : public DSP::Block
{
  private:
    DSP::Float State;
    DSP::Float RealInitialValue;

    //! Recalculates initial value when constant input is set for this block
    void RecalculateInitials(void);

    static void InputExecute(INPUT_EXECUTE_ARGS);

  public:
    RealMultiplication(unsigned int NoOfRealInputs_in=2U); //numbers of inputs
    ~RealMultiplication(void);
};

/**************************************************/
//! Decimator without antialias filter
/*! Inputs and Outputs names:
 *  - Output:
 *   -# "out1", "out2", ... (real valued)
 *  - Input:
 *   -# "in1", "in2", ... (real valued)
 *
 * \note It seems that decimation blocks do not need to
 * be registered to the output clock, they just should know
 * the output clock for error detection purpose. THIS MUST BE SORTED OUT.
 */
class DSP::u::RawDecimator  : public DSP::Block, public DSP::Source
{
  private:
    int  M;
    bool IsReady;
    int  InnerCounter;
    DSP::Float *State;

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);
    static void InputExecute(INPUT_EXECUTE_ARGS);

  public:
    /*! \todo_later OutputClocks should be updated for each output
     * not only first
     */
    RawDecimator(DSP::Clock_ptr ParentClock, unsigned int M_in=2, unsigned int InputsNo=1);
    ~RawDecimator(void);
};

//! Time expansion block: zeroinserter (+ hold)
/*! Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real valued)
 *  - Input:
 *   -# "in" (real valued)
 *
 * \warning ParentClock is the clock with which works input not output.
 *   Output's clock works L times faster. To get output's clock use
 *   DSP::Component::GetOutputClock.
 *
 * \todo Complex or multivalued input/output.
 */
class DSP::u::Zeroinserter  : public DSP::Block, public DSP::Source
{
  private:
    unsigned int  L;
    bool IsInputReady, IsReady;
    unsigned int  InnerCounter;
    DSP::Float tempInput_re, tempInput_im, State_re, State_im;

    bool IsHold;

    //!Execution as a source block
    static bool OutputExecute_real(OUTPUT_EXECUTE_ARGS);
    static bool OutputExecute_cplx(OUTPUT_EXECUTE_ARGS);
    //!Execution as an processing block
    static void InputExecute_real(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx(INPUT_EXECUTE_ARGS);

    void Init(bool IsInputComplex, DSP::Clock_ptr ParentClock, unsigned int L_in, bool Hold);
public:
    //if Hold == true, holds input value instead of inserting zeros
    Zeroinserter(DSP::Clock_ptr ParentClock, unsigned int L_in=2, bool Hold=false);
    Zeroinserter(bool IsInputComplex, DSP::Clock_ptr ParentClock, unsigned int L_in=2, bool Hold=false);
    ~Zeroinserter(void);
};

// ***************************************************** //
//! Generates constant value
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real, complex or multivalued valued)
 *   -# "out.re" (real component)\n
 *      "out.im" (imag component)
 *   -# "out1", "out2", ... (real or complex)
 *   -# "out1.re", "out2.re", ... (real part)
 *   -# "out1.im", "out2.im", ... (complex part)
 *  - Input: none
 */
class DSP::u::Const : public DSP::Source
{
  private:
    DSP::Float const_val;
    DSP::Float_ptr const_state;

    static bool OutputExecute_one(OUTPUT_EXECUTE_ARGS);
    static bool OutputExecute_many(OUTPUT_EXECUTE_ARGS);

  public:
    Const(DSP::Clock_ptr ParentClock,
          DSP::Float value);
    Const(DSP::Clock_ptr ParentClock,
          DSP::Float value_re, DSP::Float value_im);
    Const(DSP::Clock_ptr ParentClock,
          DSP::Complex value);
    Const(DSP::Clock_ptr ParentClock,
          unsigned int NoOfInputs_in, DSP::Float_ptr values);
    Const(DSP::Clock_ptr ParentClock,
          unsigned int NoOfInputs_in, DSP::Complex_ptr values);
    ~Const(void);
};

// ***************************************************** //
//! Generates pulse train
/*!
 * -# N0 > N1:
 *   y[n]=A*exp(alfa*n)*cos(omega*n+phase)*(u[n-N0]-u[n-N1])
 * -# N0 <= N1:
 *   y[n]=A*exp(alfa*n)*cos(omega*n+phase)*u[n-N0]
 * -# if period > 0 -> n = (n+1) \% period;
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real or complex valued)
 *   -# "out.re" (real component)\n
 *      "out.im" (imag component)
 *  - Input: none
 */
class DSP::u::COSpulse : public DSP::Source
{
  private:
    DSP::Float A;
    DSP::Float alfa;
    DSP::Float omega, phase;
    unsigned long N0, N1;
    unsigned long period;

    unsigned long n;


    //! Initiates inner variables
    /*! Only for use with constructor
     */
    void Init(DSP::Float A_in, DSP::Float alfa_in,
              DSP::Float omega_in, DSP::Float phase_in,
              unsigned long N0_in, unsigned long N1_in,
              unsigned long period_in,
              DSP::Clock_ptr ParentClock);

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

  public:
    COSpulse(DSP::Clock_ptr ParentClock,
                 DSP::Float A_in, DSP::Float alfa_in=0.0,
                 DSP::Float omega_in=0.0, DSP::Float phase_in=0.0,
                 unsigned long N0_in=0, unsigned long N1_in=0,
                 unsigned long period_in=0);
    COSpulse(DSP::Clock_ptr ParentClock,
                 bool IsComplex, DSP::Float A_in, DSP::Float alfa_in=0.0,
                 DSP::Float omega_in=0.0, DSP::Float phase_in=0.0,
                 unsigned long N0_in=0, unsigned long N1_in=0,
                 unsigned long period_in=0);
    ~COSpulse(void);

    //! changes amplitude parameter
    void SetAmplitude(DSP::Float new_amplitude);
    //! Changes angular frequency
    void SetAngularFrequency(DSP::Float omega_in);
    //! Changes pulse length in samples
    void SetPulseLength(int pulse_length);
    //! Changes pulse train period in samples
    void SetPulsePeriod(int pulse_period);
};

// ***************************************************** //
//! Generates uniform noise
/*!
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real or complex valued)
 *   -# "out.re" (real component)\n
 *      "out.im" (imag component)
 *  - Input: none
 */
class DSP::u::Rand : public DSP::Source, public DSP::Randomization
{
  private:


    //! Initiates random generator
    void Init(DSP::Clock_ptr ParentClock);

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

  public:
    Rand(DSP::Clock_ptr ParentClock);
    Rand(DSP::Clock_ptr ParentClock,
         bool IsComplex);
    ~Rand(void);
};

// ***************************************************** //
//! Generates random binary streams.
/*! Output value can be only  L_value_in or U_value_in.
 *  Default: 0.0 and 1.0.
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real valued)
 *  - Input: none
 */
class DSP::u::BinRand : public DSP::Source, public DSP::Randomization
{
  private:
    DSP::Float L_value; //! value corresponding to binary 0
    DSP::Float U_value; //! value corresponding to binary 1

    //! Initiates random generator
    void Init(DSP::Clock_ptr ParentClock,
              DSP::Float L_value_in = 0.0, DSP::Float U_value_in = 1.0);

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

  public:
    BinRand(DSP::Clock_ptr ParentClock,
                 DSP::Float L_value_in = 0.0, DSP::Float U_value_in = 1.0);
    ~BinRand(void);
};

// ***************************************************** //
//! Generates LFSR binary streams.
/*! LFSR - linear feedback shift register.
 *
 * \todo test with register lengths >= 64
 *
 * \todo_later <b>12.06.2008</b> For random initial state, output will be m-sequence
 *    after register length cycles. Consider outputing modulo 2 sum of
 *    taps instead of LSB of the register.
 *
 * \note Output value can be only  L_value_in or U_value_in.
 *  Default: 0.0 and 1.0.
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real valued)
 *  - Input: none
 *
 * \note division by ULL_MSB can be changed into binary shift >>
 *   ! check if its value is correct (if it should not be larger).
 */
class DSP::u::LFSR : public DSP::Source, public DSP::Randomization
{
  private:
    DSP::Float L_value; //! value corresponding to binary 0
    DSP::Float U_value; //! value corresponding to binary 1
    unsigned int reg_len; //! register length
    unsigned int taps_no; //! number of feedback taps
    unsigned int *taps;   //! feedback taps indexes - 1 (index in buffer)
    unsigned int buffer_size; // buffer size in ULL ints
    unsigned long long int* buffer; //! buffer with internal register state
    unsigned long long int buffer_MSB_mask; //! mask for MSB bit (the most left hand bit)
    // LSB bit - the most right hand bit - mask == 0x0000..001
    unsigned long long int ULL_MSB; //! mask for MSB bit (the most right hand bit of unsigned long long int)

    //! Initiates shift register
    void Init(DSP::Clock_ptr ParentClock, unsigned int reg_length,
      	      unsigned int no_of_taps, unsigned int *taps_idx,
    		      bool *state = NULL,
              DSP::Float L_value_in = 0.0, DSP::Float U_value_in = 1.0);

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

  public:
  	/*! - reg_length - register length
  	 *  - no_of_taps - number of feedback taps
  	 *  - taps_idx - feedback taps indexes (1 .. reg_len)
  	 *  - state - initial register state (reg_len elements)
  	 *  .
  	 *
  	 *  \note if state == NULL register will be initiated with
  	 *   none zero random sequence.
  	 */
    LFSR(DSP::Clock_ptr ParentClock, unsigned int reg_length,
              unsigned int no_of_taps, unsigned int *taps_idx,
    		      bool *state = NULL,
              DSP::Float L_value_in = 0.0, DSP::Float U_value_in = 1.0);
    ~LFSR(void);
};

// ***************************************************** //
//! Tests LFSR binary streams.
/*! Tests LFSR - linear feedback shift register.
 *
 * \todo test with register lengths >= 64
 *
 * \note Input value must be only  L_value_in or U_value_in.
 *  Default: 0.0 and 1.0.
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real valued)
 *  - Input:
 *   -# "in" (real valued)
 *
 * \todo change division by ULL_MSB into binary shift >>
 *   ! check if it is correct value (if it should not be larger).
 */
class DSP::u::LFSR_tester : public DSP::Block, public DSP::Randomization
{
  private:
    DSP::Float L_value; //! value corresponding to binary 0
    DSP::Float U_value; //! value corresponding to binary 1
    unsigned int reg_len; //! register length
    unsigned int taps_no; //! number of feedback taps
    unsigned int *taps;   //! feedback taps indexes - 1 (index in buffer)
    unsigned int buffer_size; // buffer size in ULL ints
    unsigned long long int* buffer; //! buffer with internal register state
    unsigned long long int ULL_MSB; //! mask for MSB bit (the most right hand bit of unsigned long long int)

    unsigned long long int buffer_MSB_mask; //! mask for MSB bit (the most left hand bit)
    // LSB bit - the most right hand bit - mask == 0x0000..001

    //! Initiates taps mask
    void Init(unsigned int reg_length, unsigned int no_of_taps, unsigned int *taps_idx,
              DSP::Float L_value_in = 0.0, DSP::Float U_value_in = 1.0);

    static void InputExecute(INPUT_EXECUTE_ARGS);

  public:
  	/*! - reg_length - register length
  	 *  - no_of_taps - number of feedback taps
  	 *  - taps_idx - feedback taps indexes (1 .. reg_len)
  	 *  - state - initial register state (reg_len elements)
  	 *  .
  	 *
  	 *  \note if state == NULL register will be initiated with
  	 *   none zero random sequence.
  	 */
  	LFSR_tester(unsigned int reg_length, unsigned int no_of_taps, unsigned int *taps_idx,
                DSP::Float L_value_in = 0.0, DSP::Float U_value_in = 1.0);
    ~LFSR_tester(void);
};

// ***************************************************** //
//! Generates real/complex cosinusoid on the basis of DDS
/*! Uses phase acumulated modulo pi, thus can work forever
 * \f[
   x[n]=A \cdot \cos(\sum_{n=0}^{+\infty}{\omega[n]+\varphi[n]})
   \f]
 *
 * Inputs and Outputs names:
 *  - Constant parameters version:
 *   - Output:
 *    -# "out" (real or complex valued)
 *    -# "out.re" (real component)\n
 *       "out.im" (imag component if exists)
 *   - Input: none
 *  - Runtime changeable parameters version:
 *   - Output:
 *    -# "out" (real or complex valued)
 *    -# "out.re" (real component)\n
 *       "out.im" (imag component if exists)
 *   - Input:
 *    -# "ampl" - cosinusoid amplitude
 *    -# "puls" - cosinusoid angular frequency
 *    -# "phase" - cosinusoid initial phase
 *
 */
class DSP::u::DDScos : public DSP::Block, public DSP::Source
{
  private:
    //! cosinusoid amplitude
    DSP::Float A;
    //! initial phase divided by DSP::M_PIx2
    DSP::Float phase;
    //! cosinusoid normalized frequency (angular frequency divided by DSP::M_PIx2)
    DSP::Float frequency;

    //! True if DDS generator parameters are ready (constant or read from inputs in current cycle)
    bool InputParamsReady;
//    //! Number of inputs read in current cycle
//    int NoOfInputsRead;

    //! Cosinusoid instantaneous phase divided by DSP::M_PIx2
    /*! Strictly speaking it's just cumulated instantaneous frequency
     * so it doesn't include initial phase (which in fact might be
     * changing in time too)
     */
    DSP::Float CurrentPhase;

    //! Initiation - only to be used in constructor
    /*! frequency is given in [rad/cycle]
     *  phase is given in [rad]
     */
    void Init(DSP::Float A_in,
              DSP::Float frequency_in, DSP::Float phase_in,
              DSP::Clock_ptr ParentClock);

    //! Recalculates initial values
    void RecalculateInitials(void);

//    // ! Source execution function
//    static bool OutputExecute(DSP::Source_ptr source, DSP::Clock_ptr clock=NULL);
    //! Source execution function: real output / no inputs
    static bool OutputExecute_real_no_inputs(OUTPUT_EXECUTE_ARGS);
    //! Source execution function: complex output / no inputs
    static bool OutputExecute_complex_no_inputs(OUTPUT_EXECUTE_ARGS);
    //! Source execution function: real output & inputs
    static bool OutputExecute_real(OUTPUT_EXECUTE_ARGS);
    //! Source execution function: complex output & inputs
    static bool OutputExecute_complex(OUTPUT_EXECUTE_ARGS);
    //! Processing block execution function
    /*! This function provides input parameters
     *  reception and processing
     */
    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    //! DDS cosinusoid generator with constant parameters and real output
    /*! Parameters are defined at block definition (creation) time.
     *
     *  Frequency is given in [rad/cycle],
     *  phase is given in [rad]
     */
    DDScos(DSP::Clock_ptr ParentClock,
           DSP::Float A_in,
           DSP::Float frequency_in=0.0,
           DSP::Float phase_in=0.0);
    //! DDS cosinusoid generator with constant parameters and real or complex output
    /*! Parameters are defined at block definition (creation) time
     *
     *  Frequency is given in [rad/cycle],
     *  phase is given in [rad]
     */
    DDScos(DSP::Clock_ptr ParentClock,
           bool IsComplex,
           DSP::Float A_in,
           DSP::Float frequency_in=0.0,
           DSP::Float phase_in=0.0);

    //! DDS cosinusoid generator with runtime changeable parameters read from inputs and real output
    /*! Parameters are read from inputs at runtime
     *  - input no 0 : cosinusoid amplitude
     *  - input no 1 : cosinusoid angular frequency
     *  - input no 2 : cosinusoid initial phase
     *
     *  Frequency input should be given in [rad/cycle],
     *  phase input should be given in [rad]
     *
     * Inputs: Amplitude, angular frequency, initial phase
     */
    DDScos(DSP::Clock_ptr ParentClock);

    //! DDS cosinusoid generator with runtime changeable parameters read from inputs and real or complex output
    /*! Parameters are read from inputs at runtime
     *  - input no 0 : cosinusoid amplitude
     *  - input no 1 : cosinusoid angular frequency
     *  - input no 2 : cosinusoid initial phase
     *
     *  Frequency input should be given in [rad/cycle] ,
     *  phase input should be given in [rad]
     *
     * Inputs: Amplitude, angular frequency, initial phase
     */
    DDScos(DSP::Clock_ptr ParentClock, bool IsComplex);

    ~DDScos(void);

    //! Changes angular frequency (if not associated with input)
    void SetAngularFrequency(DSP::Float omega);
    //! Changes amplitude (if not associated with input)
    void SetAmplitude(DSP::Float amplitude);
    DSP::Float GetFrequency(DSP::Float Fp)
    {return frequency*Fp;}
};

/**************************************************/
//! FIR filter implementation
/*! N_in - impulse response length (number of samples in h_in)\n
 * h_in - impulse response samples
 * n0 - index of the first sample to take
 * M - get samples of impulse response decimated by M\n
 *     (extraction of polyphase filters, M > 1)\n
 * Take samples: n0, n0+M, ... < N_in
 * L - delay between consecutive filter taps. Used to implement shaping filters in I-FIR.
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" (real or complex valued)
 *    -# "out.re" (real component)\n
 *       "out.im" (imag component if exists)
 *   - Input:
 *    -# "in" (real or complex valued)
 *    -# "in.re" (real component)\n
 *       "in.im" (imag component if exists)
 */
class DSP::u::FIR : public DSP::Block
{
  private:
    long N;
    //! size of state buffer part to move in shaping filter implementation
    long int memmove_size;
    //! memory cell size in shaping filter implementation
    int L_step;

    // if h == NULL, coefficients are complex
    DSP::Float_vector h;
    DSP::Complex_vector hC;
    DSP::Float_vector State;
    DSP::Complex in_value;

    void Init(bool IsInputComplex, bool AreCoeficientsComplex,
              unsigned long N_in, const void *h_in, int n0, int M, int L);

    static void InputExecute_RI_RH1(INPUT_EXECUTE_ARGS);
    static void InputExecute_RI_RH(INPUT_EXECUTE_ARGS);
    static void InputExecute_RI_CH1(INPUT_EXECUTE_ARGS);
    static void InputExecute_RI_CH(INPUT_EXECUTE_ARGS);
    static void InputExecute_CI_RH1(INPUT_EXECUTE_ARGS);
    static void InputExecute_CI_RH(INPUT_EXECUTE_ARGS);
    static void InputExecute_CI_CH1(INPUT_EXECUTE_ARGS);
    static void InputExecute_CI_CH(INPUT_EXECUTE_ARGS);

    static void InputExecute_RI_RH_L(INPUT_EXECUTE_ARGS);

    //static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    FIR(const DSP::Float_vector &h_in, int n0 = 0, int M = 1, int L = 1);
    FIR(const DSP::Complex_vector &h_in, int n0 = 0, int M = 1, int L = 1);

    FIR(bool IsInputComplex, const DSP::Float_vector &h_in, int n0 = 0, int M = 1, int L = 1);
    FIR(bool IsInputComplex, const DSP::Complex_vector &h_in, int n0 = 0, int M = 1, int L = 1);
    ~FIR(void);
};

/**************************************************/
//! IIR filter implementation
/*! Na_in - number of feedback coefficients
 * a_in - feedback coefficients\n
 * Nb_in - number of feedforward coefficients (if it's 1 or not specified
 *        all-pole IIR filter is created)\n
 * b_in - feedforward coefficients (if it's NULL or not specified
 *        all-pole IIR filter is created)
 *
 * \note a_in and b_in coefficients are coppied to internal vectors
 *  thus user can free them as soon as the object is constructed.
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" (real or complex valued)
 *    -# "out.re" (real component)\n
 *       "out.im" (imag component if exists)
 *   - Input:
 *    -# "in" (real or complex valued)
 *    -# "in.re" (real component)\n
 *       "in.im" (imag component if exists)
 */
class DSP::u::IIR : public DSP::Block
{
  private:
    long FilterOrder;
//    int Na, Nb;

    // bool AreCoeficientsComplex; If a == NULL, coeficients are complex
    DSP::Float_vector a, b;
    DSP::Complex_vector aC, bC;

    DSP::Float_vector State;
    DSP::Complex in_value;

    template <typename T>
    void Init(bool IsInputComplex, bool AreCoeficientsComplex, T &a_in, T &b_in);

    static void InputExecute_real_coefs_cplx_input_zero_order(INPUT_EXECUTE_ARGS);
    static void InputExecute_real_coefs_real_input_zero_order(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx_coefs_cplx_input_zero_order(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx_coefs_real_input_zero_order(INPUT_EXECUTE_ARGS);

    static void InputExecute_real_coefs_cplx_input(INPUT_EXECUTE_ARGS);
    static void InputExecute_real_coefs_real_input(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx_coefs_cplx_input(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx_coefs_real_input(INPUT_EXECUTE_ARGS);

  public:
    //! Allows for filter coefficients change.
    /*! Sets new set of real valued coefficients.
     *
     * \note This version can be called only if
     *   real valued coefficients where used in
     *   block constructor.
     *
     *  \note Number of coefficients must be identical to
     *   that set in block constructor. If filter order must be
     *   lowered then a_in or b_in vectors must be padded at the end
     *   with zeroes.
     */
    bool SetCoefs(DSP::Float_vector &a_in, DSP::Float_vector &b_in);
    //! Allows for filter coefficients change.
    /*! Sets new set of complex valued coefficients.
     *
     * \note This version can be called only if
     *   complex valued coefficients where used in
     *   block constructor.
     *
     * \note Number of coefficients must be identical to
     *   that set in block constructor. If filter order must be
     *   lowered then a_in or b_in vectors must be padded at the end
     *   with zeroes.
     */
    bool SetCoefs(DSP::Complex_vector &a_in, DSP::Complex_vector &b_in);

    //! DSP::Float_vector &b_in = {1}
    IIR(DSP::Float_vector &a_in);
    IIR(DSP::Float_vector &a_in, DSP::Float_vector &b_in);
    //! DSP::Float_vector &b_in = {1}
    IIR(bool IsInputComplex, DSP::Float_vector &a_in);
    IIR(bool IsInputComplex, DSP::Float_vector &a_in, DSP::Float_vector &b_in);

    //! DSP::Float_vector &b_in = {1}
    IIR(DSP::Complex_vector &a_in);
    IIR(DSP::Complex_vector &a_in, DSP::Complex_vector &b_in);
    //! DSP::Float_vector &b_in = {1}
    IIR(bool IsInputComplex, DSP::Complex_vector &a_in);
    IIR(bool IsInputComplex, DSP::Complex_vector &a_in, DSP::Complex_vector &b_in);

    ~IIR(void);
};

/**************************************************/
//! Differator - first order backward difference operator
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out1", "out2", ... (real or complex valued)
 *   -# "out1.re", "out2.re", ... - real component\n
 *      "out1.im", "out2.im", ... - imag component if exists
 *   -# "out" == "out1" (real or complex valued)
 *   -# "out.re" == "out1.re" - real component\n
 *      "out.im" == "out1.im" - imag component if exists
 *  - Input:
 *   -# "in1", "in2", ... (real or complex valued)
 *   -# "in1.re", "in2.re", ... - real component\n
 *      "in1.im", "in2.im", ... - imag component if exists
 *   -# "in" == "in1" (real or complex valued)
 *   -# "in.re" == "in1.re" - real component\n
 *      "in.im" == "in1.im" - imag component if exists
 */
class DSP::u::Differator : public DSP::Block
{
  private:
    std::vector <DSP::Float> State;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    //! Setting up internal state
    void SetInitialState(const DSP::Float_vector &State_init);
    void SetInitialState(DSP::Float State_init);
    void SetInitialState(DSP::Float State_init_re, DSP::Float State_init_im);
    void SetInitialState(DSP::Complex State_init);

    Differator(int NoOfInputs_in, bool IsInputComplex=false);
    ~Differator(void);
};

/**************************************************/
//! Accumulator - first order operator
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out1", "out2", ... (real or complex valued)
 *   -# "out1.re", "out2.re", ... - real component\n
 *      "out1.im", "out2.im", ... - imag component if exists
 *   -# "out" == "out1" (real or complex valued)
 *   -# "out.re" == "out1.re" - real component\n
 *      "out.im" == "out1.im" - imag component if exists
 *  - Input:
 *   -# "in1", "in2", ... (real or complex valued)
 *   -# "in1.re", "in2.re", ... - real component\n
 *      "in1.im", "in2.im", ... - imag component if exists
 *   -# "in" == "in1" (real or complex valued)
 *   -# "in.re" == "in1.re" - real component\n
 *      "in.im" == "in1.im" - imag component if exists
 */
class DSP::u::Accumulator : public DSP::Block
{
  private:
    DSP::Float lambda, one_minus_lambda;
    DSP::Float_vector State;

    void Init(int NoOfInputs_in, DSP::Float lambda_in = 0.5, bool IsInputComplex=false);

    static void InputExecute_classic(INPUT_EXECUTE_ARGS);
    static void InputExecute_leakage(INPUT_EXECUTE_ARGS);
  public:
    //! Setting up internal state
    void SetInitialState(const DSP::Float_vector &State_init);
    void SetInitialState(DSP::Float State_init);
    void SetInitialState(DSP::Float State_init_re, DSP::Float State_init_im);
    void SetInitialState(DSP::Complex State_init);

    //! Classic accumulator
    Accumulator(int NoOfInputs_in = 1, bool IsInputComplex=false);
    //! Accumulator with leakage
    Accumulator(DSP::Float lambda_in, int NoOfInputs_in = 1, bool IsInputComplex=false);
    ~Accumulator(void);
};

/**************************************************/
//! Sampling rate conversion block
/*! N_in - impulse response length
 *  h_in - interpolation/decimation filter impulse response samples
 *
 * One state buffer working at input sampling rate and several polyphase filters
 *  - L filters \f$ h_i[n] = h[i+nL]\f$ where \f$i = 0 .. L-1\f$
 *  - output samples are evaluated for every \f$M^{th}\f$ filter (modulo L)
 *  - if L > M  for some input sample there will be no output sample
 *  - if L < M we need output samples buffer larger then for one sample,
 *   possibly for ceil(L/M) samples
 *
 * StateBuffer length = ceil(N_in/L) = floor((N_in+L-1)/L) (including current input sample)\n
 * OuputBuffer length = ceil(L/M) = floor ((L+M-1)/M)
 *
 * <b> Implementation should be input driven </b>:
 *  this means: output samples should be evaluated when input
 *  sample is available, and stored in OutputBuffer
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out" (real valued)
 *   - Input:
 *    -# "in" (real valued)
 *
 *
 * \todo_later Implement version with complex interpolation filter impulse respose
 */
class DSP::u::SamplingRateConversion : public DSP::Block, public DSP::Source
{
  private:
    //int N;
    //! interpolation/decimation filter impulse response
    DSP::Float_vector h_real;
    DSP::Complex_vector h_cplx;

    unsigned int CurrentFilterIndex, dn;
    unsigned int L, M;


    //! Number of samples available in output buffer
    unsigned int NoOfSamplesReady;
    //! Buffer for the output samples
    DSP::Float_vector OutputBuffer_real;
    DSP::Complex_vector OutputBuffer_cplx;

    //! State buffer for polyphase filters
    DSP::Float_vector StateBuffer_real;
    DSP::Complex_vector StateBuffer_cplx;

    //! Variable for storing components of the input sample (in case of complex inputs)
    DSP::Complex in_value;

    void Init(bool IsInputComplex, unsigned int L_in, unsigned int M_in,
              const DSP::Float_vector &h_in,
              DSP::Clock_ptr ParentClock);
    void Init(bool IsInputComplex, unsigned int L_in, unsigned int M_in,
              const DSP::Complex_vector &h_in,
              DSP::Clock_ptr ParentClock);

    static void InputExecute_real_in_real_h(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx_in_real_h(INPUT_EXECUTE_ARGS);
    static void InputExecute_real_in_cplx_h(INPUT_EXECUTE_ARGS);
    static void InputExecute_cplx_in_cplx_h(INPUT_EXECUTE_ARGS);
    static bool OutputExecute_real_out(OUTPUT_EXECUTE_ARGS);
    static bool OutputExecute_cplx_out(OUTPUT_EXECUTE_ARGS);

  public:
    //! Variant assumming IsInputComplex = false
    SamplingRateConversion(DSP::Clock_ptr ParentClock, unsigned int L_in, unsigned int M_in,
        const DSP::Float_vector &h_in);
    //! Variant assumming IsInputComplex = false
    SamplingRateConversion(DSP::Clock_ptr ParentClock, unsigned int L_in, unsigned int M_in,
        const DSP::Complex_vector &h_in);
    SamplingRateConversion(bool IsInputComplex,
        DSP::Clock_ptr ParentClock, unsigned int L_in, unsigned int M_in,
        const DSP::Float_vector &h_in);
    SamplingRateConversion(bool IsInputComplex,
        DSP::Clock_ptr ParentClock, unsigned int L_in, unsigned int M_in,
        const DSP::Complex_vector &h_in);
    ~SamplingRateConversion(void);
};

/**************************************************/
//! Maximum selector. Outputs: (1) maximum value (2) number of input where maximum is observed
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" maximum value (real valued)
 *   -# "max" == "out"
 *   -# "ind" - input index where the maximum value has been observed.
 *     This is the first input with such value. Indeks value starts
 *     from 1.
 *  - Input:
 *   -# "in1", "in2", ...  - real i-th input
 */
class DSP::u::Maximum : public DSP::Block
{
    DSP::Float temp_max;
    unsigned int max_ind;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    Maximum(unsigned int NumberOfInputs=2);
    ~Maximum(void);
};

// ***************************************************** //
//! Outputs selected input (given by the number)
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" value of the selected input (real or complex valued)
 *   -# "out.re" - real component
 *   -# "out.im" - imag component (if exists)
 *  - Input:
 *   -# "ind" - indeks of selected input (result of floor() from the given value).
 *              By default indexes start from 1. Can be changed by setting IndexOffset.
 *   -# "in1", "in2", ...  - real or complex i-th input
 *   -# "in1.re", "in2.re", ...  - real component of the i-th input
 *   -# "in1.im", "in2.im", ...  - imag component of the i-th input
 */
class DSP::u::Selector : public DSP::Block
{
    unsigned int in_values_len;
    DSP::Complex_ptr in_values;
    unsigned int index;
    int index_offset;
  private:
    void Init(bool AreInputsComplex,
        unsigned int NumberOfInputs,
        int IndexOffset);

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    Selector(unsigned int NumberOfInputs=2u,
             int IndexOffset=1);
    Selector(bool AreInputsComplex, unsigned int NumberOfInputs=2U,
             int IndexOffset=1);
    ~Selector(void);
};

/**************************************************/
//! Block calculating absolute value of real or complex sample
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real valued)
 *  - Input:
 *   -# "in" - real or complex input
 *   -# "in.re" - real component
 *      "in.im" - imag component (if exists)
 */
class DSP::u::ABS : public DSP::Block
{
  private:
    DSP::Complex in_value;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    ABS(bool IsInputComplex=true);
    ~ABS(void);
};

/**************************************************/
//! Block calculating complex conjugation of complex sample
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" - complex output
 *   -# "out.re" - real component /n
 *      "out.im" - imag component
 *  - Input:
 *   -# "in" - complex input
 *   -# "in.re" - real component /n
 *      "in.im" - imag component
 */
class DSP::u::Conjugation : public DSP::Block
{
  private:
    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    Conjugation(void);
    ~Conjugation(void);
};

/**************************************************/
//! Block calculating the phase of a complex sample
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real valued)
 *  - Input:
 *   -# "in" - complex input
 *   -# "in.re" - real component
 *      "in.im" - imag component
 */
class DSP::u::Angle : public DSP::Block
{
  private:
    DSP::Complex in_value;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    Angle(void);
    ~Angle(void);
};

/**************************************************/
//! CMPO - complex mutual power operator
/*!
 *
 * \f$ b[n]=u[n] \cdot u^*[n-1] \f$
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (complex valued)
 *   -# "out.re" - real component\n
 *      "out.im" - imag component
 *  - Input:
 *   -# "in" - complex input
 *   -# "in.re" - real component\n
 *      "in.im" - imag component
 */
class DSP::u::CMPO : public DSP::Block
{
  private:
    DSP::Complex last_value;
    DSP::Complex in_value;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    CMPO(void);
    ~CMPO(void);
};

/**************************************************/
//! CCPC - cartesian coordinated to polar coordinates converter
/*!
 *
 * \f$ p[n]=|u[n]| + j \cdot \arg(u[n]) \f$
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (complex valued)
 *   -# "out.abs" - absolute value (real component)\n
 *      "out.arg" - phase (imag component)
 *   -# "out.re" == "out.abs"\n
 *      "out.im" == "out.arg
 *  - Input:
 *   -# "in" - complex input
 *   -# "in.re" - real component\n
 *      "in.im" - imag component
 */
class DSP::u::CCPC : public DSP::Block
{
  private:
    DSP::Complex in_value;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    CCPC(void);
    ~CCPC(void);
};

/**************************************************/
//! PCCC - polar coordinated to cartesian coordinates converter
/*!
 *
 * \f$ u[n]=a[n] \cdot e^{j \cdot \phi[n]} \f$
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (complex valued)
 *   -# "out.re" - real component\n
 *      "out.im" - imag component
 *  - Input:
 *   -# "in" - complex input
 *   -# "in.abs" - absolute value (real component)\n
 *      "in.arg" - phase (imag component)
 *   -# "in.re" == "in.abs"\n
 *      "in.im" == "in.arg
 */
class DSP::u::PCCC : public DSP::Block
{
  private:
    DSP::Float in_value_abs;
    DSP::Float in_value_phase;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    PCCC(void);
    ~PCCC(void);
};

/**************************************************/
//! User defined function block
/*!
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" - all output lines
 *   -# "out1", "out2" - i-th output (real valued)
 *   .
 *   Output "out1" ->  OutputSamples[0]\n
 *   Output "out2" ->  OutputSamples[1]\n
 *   ...
 *  - Input:
 *   -# "in" - all input lines
 *   -# "in1", "in2" - i-th input (real valued)
 *   .
 *   Input "in1" ->  InputSamples[0]\n
 *   Input "in2" ->  InputSamples[1]\n
 *   ...
 *
 * \warning Inputs and Output interpretation depends on the user.
 *
 * Callback funtion:
 * void func(int NoOfInputs,  DSP::Float_ptr InputSamples,
 *           int NoOfOutputs, DSP::Float_ptr OutputSamples,
 *           DSP::void_ptr *UserDataPtr, int UserDefinedIdentifier)
 *
 * UserDataPtr - default value is NULL, in this variable user can store
 *   the pointer to his own data structure
 * UserDefinedIdentifier - value specified in class constructor: CallbackIdentifier
 *
 * This callback function is in addtion to calls when input samples are
 * to be processed, is called twice more times:
 *  -# call from block constructor with NoOfInputs = -1;
 *  this is when the user can initiate UserData structure
 *  -# call from block destructor with NoOfInputs = -2;
 *  this is when the user can free UserData structure
 */
class DSP::u::MyFunction : public DSP::Block
{
  private:
    int UserCallbackID;
    DSP::Callback_ptr UserFunction_ptr;
    DSP::void_ptr UserData;

    DSP::Float_ptr InputData;
    DSP::Float_ptr OutputData;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    MyFunction(unsigned int NumberOfInputs, unsigned int NumberOfOutputs,
               DSP::Callback_ptr func_ptr, int CallbackIdentifier=0);
    ~MyFunction(void);
};

/**************************************************/
//! Outputs some inputs samples on the basis of activation signal
/*! If the activation signal is > 0 then the input values are
 * send to the output
 *
 * Output samples are processed when all input values (including activation
 * signal) are ready. When State[0] > 0 then process outputs otherwise do nothing.
 *
 * State[0] - activation signal
 * State[1-...] - input signals
 *
 * \note Output clock must be activated separetly with DSP::u::ClockTrigger
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out1", "out2", ...
 *    -# "out.re" == "out1", "out.im" == "out2"
 *    -# "out" == all output lines
 *   - Input:
 *    -# "in1", "in2", ...
 *    -# "in.re" == "in1", "in.im" == "in2"
 *    -# "in" == all output lines with the exception of "act"
 *    -# "act" activation signal <= exists only when ActivateOutputClock is <b>true</b>
 *    .
 *   .
 */
class DSP::u::SampleSelector  : public DSP::Block, public DSP::Source, public DSP::Clock_trigger
{
  private:
    DSP::Float_ptr State;

//  protected:
//    void SetBlockInputClock(int InputNo, DSP::Clock_ptr InputClock);
  public:
    DSP::Clock_trigger_ptr Convert2ClockTrigger(void)
    { return GetPointer2ClockTrigger(); };

    //! SampleSelector constructor
    /*!
     * Parameters:
     * - ParentClock - Clock with which inputs work. Required for syntax
     *   checking and clock activation. Might be NULL only if
     *   ActivateOutputClock is set to <b>false</b>.
     * - OutputClock
     *   - output clock determines when the output samples are generated.
     *   - if ActivateOutputClock is <b>true</b> this is clock which will
     *     be activated for each sample selection.
     *   - <b>NULL</b> if undefined. This should be used only if
     *     output brach has no sources at all and ActivateOutputClock
     *     is set <b>false</b>
     * - ActivateOutputClock
     *   - <b>true</b> if block have to work as DSP::u::ClockTriger
     *     activating output clock for each sample selection.
     *   - <b>false</b> if output clock is not defined or is activated
     *     by other block, e.g. with several DSP::u::SampleSelector blocks
     *     working synchronous.
     * - NumberOfInputs - means number of inputs and equals number of outputs.
     *
     * \warning Activation signal input doesn't count to the NumberOfInputs.
     *
     */
    SampleSelector(DSP::Clock_ptr ParentClock, DSP::Clock_ptr OutputClock,
                   bool ActivateOutputClock, int NumberOfInputs=1);
    //! SampleSelector backward compatibility constructor <b>[OBSOLETE]</b>
    /*! \note This version is provided only for backward compatibility
     *    with DSP_lib version older than 0.07.003.
     *    OutputClock is assumed NULL and ActivateOutputClock is assumed false.
     */
    SampleSelector(DSP::Clock_ptr ParentClock, int NumberOfInputs=1);
    //! SampleSelector destructor
    ~SampleSelector(void);

    //! true if all input samples in current cycle are ready
    bool SamplesReady;

    //! Output samples are generated when "act" signal is in active state
    static void InputExecute_without_source_output(INPUT_EXECUTE_ARGS);
    //! Output samples are generated when OutputClock is active
    static void InputExecute(INPUT_EXECUTE_ARGS);
    //! InputExecute version which activates output clock. Output samples are generated when OutputClock is active.
    static void InputExecute_with_Activation(INPUT_EXECUTE_ARGS);
    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);
    void Notify(DSP::Clock_ptr clock);
  private:
    //! Block initiation procedure
    /*! For use with block constructors
     */
    void Init(DSP::Clock_ptr ParentClock, DSP::Clock_ptr OutputClock,
                        bool ActivateOutputClock, unsigned int NumberOfInputs);
};

//! Reads samples acording to one clocks' group and outputs acording to other clocks' group
/*! Generaly Input & Output clocks should have different output clocks\n
 * Last sample seen on input is send to the output if required.
 * Some samples can be lost, some might be repeated several times.
 *
 * \note In most cases you would rather use DSP::u::Zeroinserter with IsHold set to true.
 * \note This block is an example of using notifications.
 * \warning because Input & Output clocks are independent to some extend then
 *   time hazards can occur.
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real, complex or multivalued)
 *   -# "out.re" (real component)\n
 *      "out.im" (imag component if exists)
 *   -# "out1", "out2", "out3", ...
 *  - Input:
 *   -# "in" (real, complex or multivalued)
 *   -# "in.re" (real component)\n
 *      "in.im" (imag component if exists)
 *   -# "in1", "in2", "in3", ...
 *   .
 *
 * \todo <b>IMPORTANT</b>
 *  - issue warning when Input & Output have the same MasterClock
 */
class DSP::u::Hold  : public DSP::Block, public DSP::Source
{
  private:
    DSP::Float_ptr currentState;
    DSP::Float_ptr newState;
    /*! false if Input and Output clocks have the
     *  same MasterClock and Input is Expected in
     *  the given clock cycle.
     */
    bool SamplesReady;

    /*! if IsHold == false the current_State will be filled with zeros
     * after it was sent do outputs */
    bool IsHold;

    //! stores output clock in case we must stall it for some time
    DSP::Clock *my_clock;

//  protected:
//    void SetBlockInputClock(int InputNo, DSP::Clock_ptr InputClock);

    static void InputExecute(INPUT_EXECUTE_ARGS);
    void Notify(DSP::Clock_ptr clock);
    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);
  public:

    //! InputClock == NULL <- auto detection at connection time
    /*! if UseZeros == true the output will be filled with zeros
     * otherwise with last sample
     */
    Hold(DSP::Clock_ptr InputClock, DSP::Clock_ptr OutputClock, bool UseZeros=false, unsigned int NumberOfInputs=1);
    ~Hold(void);
};


/**************************************************/
//! Demultiplexer block (y1[n]=x[L*n], y2[n]=x[L*n+1], yL[n]=x[L*n+L-1])
/*! Inputs and Outputs names:
 *  - Output:
 *   -# "out1", "out2", ... (real or complex valued)
 *   -# "out1.re", "out2.re", ... real components
 *   -# "out1.im", "out2.im", ... imaginary components (if they exist)
 *   -# "out" all output lines
 *   .
 *  - Input:
 *   -# "in" - (real or complex valued)
 *   -# "in.re" - real component
 *   -# "in.im" - imaginary component (if exist)
 *   .
 */
class DSP::u::Demultiplexer  : public DSP::Block // , public DSP::Source
{
  private:
    //! Number of the output where the current sample should be forwarded
    int  CurrentOutputNo;
    DSP::Float State[2];

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    Demultiplexer(bool IsComplex, unsigned int OutputsNo=2);
    ~Demultiplexer(void);
};


//! Multiplexer block (y[L*n]=x1[n], y[L*n+1]=x2[n], y[L*n+L-1]=xL[n])
/*! Inputs and Outputs names:
 *  - Output:
 *   -# "out" - (real or complex valued)
 *   -# "out.re" - real component
 *   -# "out.im" - imaginary component (if exist)
 *   .
 *  - Input:
 *   -# "in1", "in2", ... (real or complex valued)
 *   -# "in1.re", "in2.re", ... real components
 *   -# "in1.im", "in2.im", ... imaginary components (if they exist)
 *   -# "in" all input lines
 *   .
 *  .
 */
class DSP::u::Multiplexer  : public DSP::Block, public DSP::Source
{
  private:
    //! Number of current the output sample
    int  CurrentOutputSampleNo;
    DSP::Float_ptr State;
    //! true if given state slot is ready
    bool         *StateReady;

    // //!Execution as a source block
    // static bool OutputExecute(DSP::Source_ptr source, DSP::Clock_ptr clock=NULL);
    //!Execution as a source block for real input
    static bool OutputExecute_real(OUTPUT_EXECUTE_ARGS);
    //!Execution as a source block for complex input
    static bool OutputExecute_cplx(OUTPUT_EXECUTE_ARGS);
    //!Execution as an processing block
    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    Multiplexer(DSP::Clock_ptr ParentClock, bool IsComplex, unsigned int InputsNo=2);
    ~Multiplexer(void);
};

/**************************************************/
//! DCO - digitaly controled oscilator
/*!
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (complex valued)
 *   -# "out.re" - real component\n
 *      "out.im" - imag component
 *   -# "freq" available only if output_current_frequency == <b>true</b>
 *     (instantaneous normalized frequency of the oscillator)
 *   .
 *  - Input:
 *   -# "in.freq_err" - angular frequency error correction input
 *   -# "in.phase_err" - phase error correction input
 *   .
 */
class DSP::u::DCO : public DSP::Block
{
  private:
    DSP::Float fo;
    DSP::Float freq_factor;
    DSP::Float phase_factor;
    //DSP::Float max_freq_deviation;
    DSP::Float freq_dev_min, freq_dev_max;

    DSP::Float freq_memo, phase_memo;

    //temporary input variables
    DSP::Float in_freq_err;
    DSP::Float in_phase_err;

    //! DCO main routine
    /*!
     *  \image html DCO_scheme.jpg
     *
     */
    static void InputExecute(INPUT_EXECUTE_ARGS);
    DSP::Float current_frequ;
    static void InputExecute_with_Freq(INPUT_EXECUTE_ARGS);
  public:
    DCO(DSP::Float wo //!initial normalized angular frequency of the oscilator [rad/Sa]
          , DSP::Float d_wo //! maximum allowed frequency deviation (ignored if < 0.0)
          , DSP::Float freq_alfa  //! angular frequency error correction input scaling factor [rad/Sa]
          , DSP::Float phase_alfa //! phase error correction input scaling factor [rad]
          , bool output_current_frequency = false //! if true additional output with current frequency value is available
          );
    ~DCO(void);

    //! returns current estimated signal frequency in Hz for given sampling frequency
    DSP::Float GetFrequency(DSP::Float Fp);
};

/**************************************************/
//! CrossSwitch - sends input signals stright or crossed to outputs
/*!
 * \todo_later Implement multi-valued inputs/outputs
 *
 * Stright connection: in1 -> out1; in2 -> out2
 *
 * Crossed connection: in1 -> out2; in2 -> out1
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out1" (real or complex valued)
 *   -# "out1.re" - real component\n
 *      "out1.im" - imag component (if there is one)
 *   -# "out2" (real or complex valued)
 *   -# "out2.re" - real component\n
 *      "out2.im" - imag component (if there is one)
 *  - Input:
 *   -# "state" - 0 - stright connection, 1 - crossed connection
 *   -# "in1" (real or complex valued)
 *   -# "in1.re" - real component\n
 *      "in1.im" - imag component (if there is one)
 *   -# "in2" (real or complex valued)
 *   -# "in2.re" - real component\n
 *      "in2.im" - imag component (if there is one)
 */
class DSP::u::CrossSwitch : public DSP::Block
{
  private:
    int state; //! ??? false if stright, true if crossed
    int default_state;

    DSP::Float_ptr inputs;

    static void InputExecute(INPUT_EXECUTE_ARGS);
  public:
    //! if IsComplex is true inputs are complex
    CrossSwitch(bool IsComplex = false);
    ~CrossSwitch(void);
};

/**************************************************/
//! ClockTrigger - activates given clock based on activation signal
/*!
 * Inputs and Outputs names:
 *  - Output:
 *  - Input:
 *   -# "act" - activation signal
 *     - if NoOfCycles >= 1, input > 0.0 triggers clock for NoOfCycles,
 *       \note it is best to use activation signal with values from set {-1.0, +1.0}
 *        where +1.0 activates clock.
 *     - if NoOfCycles == -1, input > 0.0 triggers clock for
 *       (int)(input + 0.5) == round(input) cycles
 *       - input < 0.0 -> no activation
 *       - input = (0, 0.5) -> one cycle
 *       - input = [0.5, 1.5) -> two cycles
 *       - input = [1.5, 2.5) -> three cycles
 *       - ...
 *       .
 *     .
 *   .
 * .
 *  \warning InputClock MUST be different from OutputClock
 */
class DSP::u::ClockTrigger : public DSP::Block, public DSP::Clock_trigger
{
  private:
    static void InputExecute(INPUT_EXECUTE_ARGS);
    static void InputExecute_multivalue(INPUT_EXECUTE_ARGS);

  public:
    DSP::Clock_trigger_ptr Convert2ClockTrigger(void)
    { return GetPointer2ClockTrigger(); };

    /*! MasterClockIndex - index of MasterClock with which "act" signal works
     *  SignalActivatedClock - clock to trigger
     *  NoOfCycles - number of OutputClock cycles per clock activation
     *    - if (NoOfCycles == -1) - number of OutputClock cycles
     *      per clock activation is given by "act" signal
     */
    ClockTrigger(DSP::Clock_ptr ParentClock, DSP::Clock_ptr SignalActivatedClock_in, int NoOfCycles_in = 1);
    ~ClockTrigger(void);
};



/**************************************************/
//! See through type copy block
/*! Simply copies input to output thus
 *  number of outputs is equal to
 *  number of inputs.
 *
 *  This block has been designed to be used internaly in DSP::Macro
 *
 * \note This block is the abstract object.
 *   All connections are made directly
 *   between block's input and output components.
 *
 * \note This block can be freed
 *   after all connections are made.
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" - all output lines
 *   .
 *  - Input:
 *   -# "in" - all input lines
 *   .
 *
 * The general idea for this block is to provide possibility
 * to create names for input and outputs spaning
 * through several different blocks.
 *
 * \note Other inputs and outputs must be defined by user.
 */
class DSP::u::Copy : public DSP::Block
{
  friend class DSP::Component;
  friend class DSP::Macro;

  //friend bool DSP::_connect_class::splitconnect(const DSP::output &output, const DSP::input &input);
  friend class _connect_class;

  private:
    //! Raw output block reading function
    bool GetOutput(unsigned int OutputNo, DSP::Component_ptr &output_block, unsigned int &output_block_InputNo);

  private:
    //static void InputExecute(INPUT_EXECUTE_ARGS);

    //! DSP::u::Copy output info update
    /*! DSP::u::Copy component's output with index OutputNo must be
     *  connected to block's input with index block_InputNo.
     *
     *  DSP::u::Copy must store this info and use it to
     *  help DSP::Component::DSP::_connect_class::connect directly connect
     *  block which user connects through DSP::u::copy component.
     *
     *  Returns false if input block is still unknown.
     */
		bool SetCopyOutput(unsigned int OutputNo, DSP::Block_ptr block, unsigned int block_InputNo);
    //! Returns block and its input number to which is connected given DSP::u::Copy block output
    /*! If the requested data is no available function returns false and
     *  - output_block = NULL
     *  - output_block_InputNo = FO_NoInput
     *  .
     */
    bool GetCopyOutput(unsigned int OutputNo, DSP::Block_ptr &output_block, unsigned int &output_block_InputNo);
    //! DSP::u::Copy input info update
    /*! DSP::u::Copy component's input with index InputNo must be
     *  connected to block's output with index block_OutputNo.
     *
     *  DSP::u::Copy must store this info and use it to
     *  help DSP::Component::DSP::_connect_class::connect directly connect
     *  block which user connects through DSP::u::copy component.
     *
     *  Returns false if output block is still unknown.
     */
		bool SetCopyInput(unsigned int InputNo, DSP::Component_ptr block, unsigned int block_OutputNo);
    //! Returns component and its output number connected to given DSP::u::Copy block input
    /*! If the requested data is no available function returns false and
     *  - input_block = NULL
     *  - input_block_OutputNo = FO_NoOutput
     *  .
     */
    bool GetCopyInput(unsigned int InputNo, DSP::Component_ptr &input_block, unsigned int &input_block_OutputNo);

    DSP::u::Copy_ptr Convert2Copy(void)
    { return (DSP::u::Copy_ptr)this; };

    DSP::Component_ptr *InputBlocks; //!one block pointer per one output
    unsigned int *InputBlocks_OutputNo; //!Input number of the output block

  public:
    Copy(unsigned int NoOfInputs_in);
    ~Copy(void);
};



/**************************************************/
//! Quantizer
/*!
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" - real valued output
 *  - Input:
 *   -# "in" - real valued input
 *  .
 *
 * \todo implement version for mo�e quantization levels
 */
class DSP::u::Quantizer : public DSP::Block
{
  private:
    //! number of bits - with sign bit
    unsigned int B;
    //! quantization thresholds
    DSP::Float_vector thresholds;
    //! values corresponding to quantization levels
    DSP::Float_vector q_levels;

    DSP::Float output_val;

    static void InputExecute_1bit(INPUT_EXECUTE_ARGS);
  public:
    //! 1-bit quantizer
    /*! - value <= threshold ==> returns L_value
     *  - value > threshold ==> returns U_value
     *  .
     */
    Quantizer(DSP::Float threshold = 0.0, DSP::Float L_value = -1.0, DSP::Float U_value = +1.0);
    //DSP::u::Quantizer(unsigned int B_in);
    ~Quantizer(void);
};


#endif
