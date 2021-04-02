/*! \file DSPmodules.cpp
 * This is DSP engine components and sources definition module main file.
 *
 * \author Marek Blok
 */
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <iomanip>

#include <DSP_modules.h>
#include <DSP_clocks.h>

#include <DSP_DOT.h>

//#include <conio.h>
#include <string.h>

DSP::Component_ptr *DSP::Component::ComponentsTable=NULL;
long int DSP::Component::ComponentsTableSize=0;
long int DSP::Component::NoOfComponentsInTable=0;
const unsigned long DSP::Component::ComponentsTableSegmentSize = 1024;


//**************************************//
/*! \Fixed <b>2006.04.30</b> In release mode do not store component name (to free more memory)
 *    It would be best not to create objects storing names.
 *
 *  Saving algorithms structure requires Names, so they are inactive
 *  in Release mode
 */
DSP::name::name(void)
{
  #ifdef __DEBUG__
    ObjectName = "unknown object";
  #endif
}

DSP::name::name(const string &Name)
{
  UNUSED_RELEASE_ARGUMENT(Name);

  #ifdef __DEBUG__
    ObjectName = Name;
  #endif
}

void DSP::name::SetName(const string &Name, bool Append)
{
  #ifndef __DEBUG__
    UNUSED_ARGUMENT(Name);
    UNUSED_ARGUMENT(Append);
  #else
    if (Append)
    {
      ObjectName += "<";
      ObjectName += Name;
      ObjectName += ">";
    }
    else
      ObjectName = Name;
  #endif
}

string DSP::name::GetName()
{
  #ifdef __DEBUG__
    return ObjectName;
  #endif
  return "";
}

/* ****************************************** */
/* ****************************************** */
bool DSP::Randomization::Initialized = false;
void DSP::Randomization::InitRandGenerator(bool forced)
{
  if ((Initialized == false) || (forced == true))
  {
    time_t now;

    now = time(NULL);
    srand((unsigned int)now);
    Initialized = true;

    #ifdef TestCompilation
      DSP::log << "DSP::Randomization::InitRandGenerator", "Random generator initialized");
    #endif
  }
}

DSP::Float DSP::Randomization::randu(void)
{
  DSP::Float value;

  value =  DSP::Float(rand());
  value /= DSP::Float(RAND_MAX);

  return value;
}

void DSP::Randomization::randu(unsigned int len, DSP::Float_ptr buffer)
{
  unsigned int ind;

  for (ind = 0; ind < len; ind ++)
  {
    buffer[ind] =  DSP::Float(rand());
    buffer[ind] /= DSP::Float(RAND_MAX);
  }
}

DSP::Float DSP::Randomization::randn(void)
{
  DSP::Float value;

  value  =  (DSP::Float)rand();  // 1
  value +=  (DSP::Float)rand(); // 2
  value +=  (DSP::Float)rand(); // 3
  value +=  (DSP::Float)rand(); // 4
  value +=  (DSP::Float)rand(); // 5
  value +=  (DSP::Float)rand(); // 6
  value +=  (DSP::Float)rand(); // 7
  value +=  (DSP::Float)rand(); // 8
  value +=  (DSP::Float)rand(); // 9
  value +=  (DSP::Float)rand(); // 10
  value +=  (DSP::Float)rand(); // 11
  value +=  (DSP::Float)rand(); // 12
  value /= DSP::Float(RAND_MAX);
  value -= 6;

  return value;
}

void DSP::Randomization::randn(unsigned int len, DSP::Float_ptr buffer)
{
  unsigned int ind;

  for (ind = 0; ind < len; ind ++)
  {
    buffer[ind]  =  (DSP::Float)rand();  // 1
    buffer[ind] +=  (DSP::Float)rand(); // 2
    buffer[ind] +=  (DSP::Float)rand(); // 3
    buffer[ind] +=  (DSP::Float)rand(); // 4
    buffer[ind] +=  (DSP::Float)rand(); // 5
    buffer[ind] +=  (DSP::Float)rand(); // 6
    buffer[ind] +=  (DSP::Float)rand(); // 7
    buffer[ind] +=  (DSP::Float)rand(); // 8
    buffer[ind] +=  (DSP::Float)rand(); // 9
    buffer[ind] +=  (DSP::Float)rand(); // 10
    buffer[ind] +=  (DSP::Float)rand(); // 11
    buffer[ind] +=  (DSP::Float)rand(); // 12
    buffer[ind] /= DSP::Float(RAND_MAX);
    buffer[ind] -= 6;
  }
}

DSP::Randomization::Randomization(void)
{
  InitRandGenerator(false);
}

/* ****************************************** */
/* ****************************************** */
DSP::Clock_trigger::Clock_trigger(void)
{
  SignalActivatedClock = NULL;
  SignalActivatedClock_NoOfCycles = 1;
  MasterClockIndex = 0;
};

DSP::Clock_trigger_ptr DSP::Clock_trigger::GetPointer2ClockTrigger(void)
{
  if (SignalActivatedClock != NULL)
    return this;
  return NULL;
};


//**************************************//
DSP::Component::Component(void)
{
  Type=DSP_CT_none;
//  NoOfInputs=0;

  IsMultiClock=false;

  AutoFree = false;
  IsAutoSplit = false;

  NotificationClocks.clear();

  NoOfOutputs = 0;
  OutputBlocks = NULL;
  OutputBlocks_InputNo = NULL;

  #ifdef __DEBUG__
    MacrosStack = NULL; MacrosStackLength = 0;
    MacrosStackLength = DSP::MacroStack::GetCurrentMacroStack(MacrosStack);
  #endif

  RegisterComponent();
}

DSP::Component::~Component(void)
{
  unsigned int ind;

  std::vector<DSP::Component_ptr> deprecated_list;
  unsigned int NoOfDeprecated = 0;

  for (ind=0; ind<NoOfOutputs; ind++)
  {
    if ((OutputBlocks[ind] != NULL) && (OutputBlocks[ind] != &DSP::Block::DummyBlock))
      if (OutputBlocks[ind]->ClearInput(OutputBlocks_InputNo[ind]) == true)
      { //It is our responsibility to free this output block
        // !!! but not if we are the DSP::u::Copy component
        /*! \bug 2010.03.31 Auto blocks probably should also be deleted
         *   if Type == DSP_CP_Copy when nothing is connected to the output auto block
         */
        if (Type != DSP_CT_copy)
        {
          // create list of blocks for automatic deletion - deleting blocks here can lead to cascade of deletion procedure calls without prior call to UnregisterComponent();
          //  ??? check repetitions
          if (NoOfDeprecated == 0)
            deprecated_list.resize(NoOfOutputs, NULL);

          bool ignore = false;
          for (unsigned int ind2=0; ind2<NoOfDeprecated; ind2++)
            if (deprecated_list[ind2] == OutputBlocks[ind])
              ignore = true; // ignore - already on the list

          if (ignore == false)
          {
            deprecated_list[NoOfDeprecated] = OutputBlocks[ind];
            NoOfDeprecated++;
          }

          OutputBlocks[ind] = NULL;
        }
      }
  }

  //The clock might be released already
//  UnregisterOutputClocks();
  SetNoOfOutputs(0);
//  if (this->Convert2Block() != NULL)
//    this->Convert2Block()->SetNoOfInputs(0,0,false);

  if (DefinedOutputs.empty() == false)
  {
    DefinedOutputs.clear();
  }

  UnregisterNotifications();
  UnregisterComponent();

  if (NotificationClocks.size() > 0)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Component::~DSP::Component" << DSP::LogMode::second
        << "Notifications have not been correctly unregistered for component: >>" << GetName() << "<<" << endl;
    #endif
    NotificationClocks.clear();
  }

  #ifdef __DEBUG__
    if (MacrosStack != NULL)
    {
      delete [] MacrosStack;
      MacrosStack = NULL;

      MacrosStackLength = 0;
    }
  #endif

  // delete blocks previously marked for automatic deletion
  if (deprecated_list.size() > 0)
  {
    for (ind=0; ind<NoOfDeprecated; ind++)
      delete deprecated_list[ind];
    deprecated_list.clear();
  }
}

//***************************************************//
//  DSP::output & DSP::input
//***************************************************//
//! null output object
DSP::output DSP::output::_null(true);

//! read output name
const string &DSP::output::get_name(void) const {
  return _name;
}
//! set output name
void DSP::output::set_name(const string &name) {
  if (_is_null == true) {
    DSP::log << DSP::LogMode::Error << "DSP::output::set_name" << DSP::LogMode::second << "Attempt to set name to null output object" << endl;
    return;
  }
  _name = name;
}

DSP::output::output(const bool &create_null)
{
  if (create_null == true)
    _is_null = true;

  _name="";

  component = NULL;

  Outputs.resize(0);
}

DSP::output::~output(void)
{
  Outputs.resize(0);
}

//! null input object
DSP::input DSP::input::_null(true);

//! read input name
const string &DSP::input::get_name(void) const {
  return _name;
}
//! set input name
void DSP::input::set_name(const string &name) {
  if (_is_null == true) {
    DSP::log << DSP::LogMode::Error << "DSP::input::set_name" << DSP::LogMode::second << "Attempt to set name to null input object" << endl;
    return;
  }
  _name = name;
}

DSP::input::input(const bool &create_null)
{
  if (create_null == true)
    _is_null = true;

  _name="";

  component = NULL;

  Inputs.resize(0);
}

DSP::input::~input(void)
{
  Inputs.resize(0);
}

bool operator>>( const DSP::output  &output, const DSP::input &input ) {
  //! \TODO move DSP::_connect_class::connect code to this method and remove DSP::_connect_class::connect
  return DSP::_connect_class::connect(output, input);
}

bool operator<<( const DSP::input &input, const DSP::output  &output) {
  //! \TODO move DSP::_connect_class::connect code to this method and remove DSP::_connect_class::connect
  return DSP::_connect_class::connect(output, input);
}

bool DSP::Component::UndefineOutput(const string &Name)
{
  unsigned int ind;
  bool done;

  done = false;
  if (Name.length() == 0)
  {
    //clear all inputs
    if (DefinedOutputs.empty() == false)
    {
      DefinedOutputs.clear();
    }

    done = true;
  }
  else
  {
    //check whether the name already exists
    for (ind=0; ind<DefinedOutputs.size(); ind++)
    {
      if (Name.compare(DefinedOutputs[ind].get_name())==0)
      {
        // Remove Input
        DefinedOutputs.erase (DefinedOutputs.begin()+ind);

        done = true;
      }
    }
  }

#ifdef __DEBUG__
  if (done == false)
  {
    DSP::log << "UndefineOutput" << DSP::LogMode::second
      << "Name >" << Name << "< was not defined for block: >" << GetName() << "<" << endl;
  }
#endif

  return done;
}

bool DSP::Component::DefineOutput(const string &Name, const unsigned int &OutputNo)
{
  unsigned int ind;

  if (OutputNo>=NoOfOutputs)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DefineOutput" << DSP::LogMode::second
         << "Output number out too large for block: >" << GetName() << "<" << endl;
    #endif
    return false;
  }

  //check whether the name already exists
  for (ind=0; ind<DefinedOutputs.size(); ind++)
  {
    if (Name.compare(DefinedOutputs[ind].get_name())==0)
    {
      #ifdef __DEBUG__
        DSP::log << "DefineOutput" << DSP::LogMode::second
           << "Name >" << Name << "< already defined for block: >" << GetName() << "<" << endl;
        DSP::log << "            " << DSP::LogMode::second << "Name will be deleted before redefining" << endl;
      #endif

      UndefineOutput(Name);
    }
  }

  DSP::output tempOut;
  tempOut.component=this;
  if (Name.length() == 0)
  {
    tempOut.set_name(to_string(DefinedOutputs.size()));
  }
  else
  {
    tempOut.set_name(Name);
  }

  tempOut.Outputs.resize(1);
  tempOut.Outputs[0]=OutputNo;

  DefinedOutputs.push_back(std::move(tempOut));

  return true;
}

bool DSP::Component::DefineOutput(const string &Name,
    const unsigned int &OutputNo_re, const unsigned int &OutputNo_im)
{
  unsigned int ind;

  if (OutputNo_re>=NoOfOutputs)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DefineOutput" << DSP::LogMode::second
        << "Output number (realis) out too large for block: " << GetName() << endl;
    #endif
    return false;
  }
  if (OutputNo_im>=NoOfOutputs)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DefineOutput" << DSP::LogMode::second
        << "Output number (imaginaris) out too large for block: " << GetName() << endl;
    #endif
    return false;
  }

  //check whether the name already exists
  for (ind=0; ind<DefinedOutputs.size(); ind++)
  {
    if (Name.compare(DefinedOutputs[ind].get_name()) == 0)
    {
      #ifdef __DEBUG__
        DSP::log << "DefineOutput" << DSP::LogMode::second
          << "Name >" << Name << "< already defined for block: >" << GetName() << "<" << endl;
        DSP::log << "            " << DSP::LogMode::second << "Name will be deleted before redefining" << endl;
      #endif

      UndefineOutput(Name);
    }
  }

  DSP::output tempOut;
  tempOut.component=this;
  if (Name.length() == 0)
  {
    tempOut.set_name(to_string(DefinedOutputs.size()));
  }
  else
  {
    tempOut.set_name(Name);
  }

  tempOut.Outputs.resize(2);
  tempOut.Outputs[0]=OutputNo_re;
  tempOut.Outputs[1]=OutputNo_im;

  DefinedOutputs.push_back(std::move(tempOut));

  return true;
}

bool DSP::Component::DefineOutput(const string &Name, const vector<unsigned int> &Outputs)
{
  unsigned int ind;

  for (ind=0; ind<Outputs.size(); ind++)
    if (Outputs[ind]>=NoOfOutputs)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DefineOutput" << DSP::LogMode::second
          << "Output number (" << ind << ") out too large for block: " << GetName() << endl;
      #endif
      return false;
    }

  //check whether the name already exists
  for (ind=0; ind<DefinedOutputs.size(); ind++)
  {
    if (Name.compare(DefinedOutputs[ind].get_name()) == 0)
    {
      #ifdef __DEBUG__
        stringstream tekst;
        DSP::log << "DefineOutput" << DSP::LogMode::second
          << "Name >" << Name << "< already defined for block: >" << GetName() << "<" << endl;
        DSP::log << "            "  << DSP::LogMode::second << "Name will be deleted before redefining" << endl;
      #endif

      UndefineOutput(Name);
    }
  }

  DSP::output tempOut;
  tempOut.component=this;
  if (Name.length() == 0)
  {
    tempOut.set_name(to_string(DefinedOutputs.size()));
  }
  else
  {
    tempOut.set_name(Name);
  }

  tempOut.Outputs.resize(Outputs.size());
  for (ind=0; ind<Outputs.size(); ind++)
    tempOut.Outputs[ind]=Outputs[ind];

  DefinedOutputs.push_back(std::move(tempOut));

  return true;
}

bool DSP::Block::UndefineInput(const string &Name)
{
  unsigned int ind;
  bool done;

  done = false;
  if (Name.length() == 0)
  { // delete all input definitions
    if (DefinedInputs.empty() == false)
    {
      DefinedInputs.clear();
    }
    done = true;
  }
  else
  {
    //check whether the name already exists
    for (ind=0; ind<DefinedInputs.size(); ind++)
    {
      if (Name.compare(DefinedInputs[ind].get_name()) == 0)
      {
        // Remove Input
        DefinedInputs.erase(DefinedInputs.begin() + ind);

        done = true;
      }
    }
  }

#ifdef __DEBUG__
  if (done == false)
  {
    DSP::log << "UndefineInput" << DSP::LogMode::second
      << "Name >" << Name << "< was not defined for block: >" << GetName() << "<" << endl;
  }
#endif

  return done;
}

bool DSP::Block::DefineInput(const string &Name, const unsigned int &InputNo)
{
  unsigned int ind;

  if (InputNo>=NoOfInputs)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DefineInput(\""+Name+"\")" << DSP::LogMode::second
        << "Input number:" << InputNo << ", too large for block: " << GetName() << endl;
    #endif
    return false;
  }

  //check whether the name already exists
  for (ind=0; ind<DefinedInputs.size(); ind++)
  {
    if (Name.compare(DefinedInputs[ind].get_name()) == 0)
    {
      #ifdef __DEBUG__
        DSP::log << "DefineInput" << DSP::LogMode::second
          << "Name >" << Name << "< already defined for block: >" << GetName() << "<" << endl;
        DSP::log << "           " << DSP::LogMode::second << "Name will be deleted before redefining" << endl;
      #endif

      UndefineInput(Name);
    }
  }

  DSP::input tempIn;
  tempIn.component=this;
  if (Name.length() == 0)
  {
    tempIn.set_name(to_string(DefinedInputs.size()));
  }
  else
  {
    tempIn.set_name(Name);
  }

  tempIn.Inputs.resize(1);
  tempIn.Inputs[0]=InputNo;

  DefinedInputs.push_back(std::move(tempIn));

  return true;
}

bool DSP::Block::DefineInput(const string &Name,
    const unsigned int &InputNo_re, const unsigned int &InputNo_im)
{
  unsigned int ind;

  if (InputNo_re>=NoOfInputs)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DefineInput(\"" << Name << "\")" << DSP::LogMode::second
        << "Input number:" << InputNo_re << ", (realis) too large for block: " << GetName() << endl;
    #endif
    return false;
  }
  if (InputNo_im>=NoOfInputs)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DefineInput(\"" << Name << "\")" << DSP::LogMode::second
        << "Input number " << InputNo_re << " (imaginaris) too large for block: " << GetName() << endl;
    #endif
    return false;
  }

  //check whether the name already exists
  for (ind=0; ind<DefinedInputs.size(); ind++)
  {
    if (Name.compare(DefinedInputs[ind].get_name()) == 0)
    {
      #ifdef __DEBUG__
        DSP::log << "DefineInput" << DSP::LogMode::second
          << "Name >" << Name << "< already defined for block: >" << GetName() << "<" << endl;
        DSP::log << "           " << DSP::LogMode::second << "Name will be deleted before redefining" << endl;
      #endif

      UndefineInput(Name);
    }
  }

  DSP::input tempIn;
  tempIn.component=this;
  if (Name.length() == 0)
  {
    tempIn.set_name(to_string(DefinedInputs.size()));
  }
  else
  {
    tempIn.set_name(Name);
  }

  tempIn.Inputs.resize(2);
  tempIn.Inputs[0]=InputNo_re;
  tempIn.Inputs[1]=InputNo_im;

  DefinedInputs.push_back(std::move(tempIn));

  return true;
}

bool DSP::Block::DefineInput(const string &Name,
    const vector<unsigned int> &Inputs)
{
  unsigned int ind;

 for (ind=0; ind<Inputs.size(); ind++)
    if (Inputs[ind]>=NoOfInputs)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DefineInput(\"" << Name << "\")" << DSP::LogMode::second
          << "Input number:" << ind << ", too large for block: " << GetName() << endl;
      #endif
      return false;
    }

  //check whether the name already exists
  for (ind=0; ind<DefinedInputs.size(); ind++)
  {
    if (Name.compare(DefinedInputs[ind].get_name()) == 0)
    {
      #ifdef __DEBUG__
        DSP::log << "DefineInput" << DSP::LogMode::second
          << "Name >" << Name << "< already defined for block: >" << GetName() << "<" << endl;
        DSP::log << "           " << DSP::LogMode::second << "Name will be deleted before redefining" << endl;
      #endif

      UndefineInput(Name);
    }
  }

  DSP::input tempIn;
  tempIn.component=this;
  if (Name.length() == 0)
  {
    tempIn.set_name(to_string(DefinedInputs.size()));
  }
  else
  {
    tempIn.set_name(Name);
  }

  tempIn.Inputs.resize(Inputs.size());
  for (ind=0; ind<Inputs.size(); ind++)
    tempIn.Inputs[ind]=Inputs[ind];

  DefinedInputs.push_back(std::move(tempIn));

  return true;
}

DSP::output &DSP::Component::Output(const string &Name)
{
  unsigned int ind;

  for (ind=0; ind<DefinedOutputs.size(); ind++)
  {
    if (Name.compare(DefinedOutputs[ind].get_name()) == 0)
      return DefinedOutputs[ind];
  }

  #ifdef __DEBUG__
    DSP::log << DSP::LogMode::Error << "DSP::Component::Output" << DSP::LogMode::second
      << "Block<" << this->GetName() << "> has no output named <" << Name << ">" << endl;
    DSP::log << "DSP::Component::Output" << DSP::LogMode::second << "Available output names:" << endl;
    for (ind=0; ind<DefinedOutputs.size(); ind++)
    {
      DSP::log << "DSP::Component::Output" << DSP::LogMode::second
        << "   \"" << DefinedOutputs[ind].get_name() << "\"" << endl;
    }
    DSP::log << DSP::LogMode::Error << "DSP::Component::Output" << DSP::LogMode::second
      << "Use one of defined outputs or define new one using DSP::Component::DefineOutput function" << endl;
  #endif
  return DSP::output::null();
}

DSP::input &DSP::Component::Input(const string &Name)
{
  UNUSED_ARGUMENT(Name);

  #ifdef __DEBUG__
    DSP::log << DSP::LogMode::Error << "DSP::Block::Input";
    DSP::log << DSP::LogMode::second << "Block<" << this->GetName() << "> should not have Input() method called." << endl;
  #endif
  return DSP::input::null();
}

DSP::input &DSP::Block::Input(const string &Name)
{
  unsigned int ind;

  for (ind=0; ind<DefinedInputs.size(); ind++)
  {
    if (Name.compare(DefinedInputs[ind].get_name()) == 0)
      return DefinedInputs[ind];
  }

  #ifdef __DEBUG__
    DSP::log << "DSP::Block::Input" << DSP::LogMode::second
      << "Block<" << this->GetName() << "> has no input named <" << Name << ">" << endl;
    DSP::log << "DSP::Block::Input" << DSP::LogMode::second << "Available input names:" << endl;
    for (ind=0; ind<DefinedInputs.size(); ind++)
    {
      DSP::log << "DSP::Block::Input" << DSP::LogMode::second
        << "   \"" << DefinedInputs[ind].get_name() << "\"" << endl;
    }
    DSP::log << DSP::LogMode::Error << "DSP::Block::Input" << DSP::LogMode::second
      << "Use one of defined inputs or define new one using DSP::Block::DefineInput function" << endl;
  #endif
  return DSP::input::null();
}

//! connects the given output to the given input
/*! returns true if succeeds
 *  Makes use of DSP::Block::SetOutput function
 *
 * \Fixed 2007.06.02 Added DSP::_connect_class::splitconnect behavior.
 *    - Changed default behavior to DSP::_connect_class::splitconnect if output is used.
 *    - AllowSplit == false returns old behavior where error is issued is output is already used.
 *    .
 */
bool DSP::_connect_class::connect(const DSP::output &output, const DSP::input &input, bool AllowSplit)
{
  unsigned int ind;
  bool ReturnStatus;

  //  if ((output == NULL) || (input == NULL))
  if (output.is_null() || input.is_null())
  {
    #ifdef __DEBUG__
      stringstream tekst;
      if (output.is_null() && input.is_null())
        tekst << "NULL output -> NULL input";
      if (output.is_null() && (!input.is_null()))
      {
        tekst << "NULL output -> Block:" << input.component->GetName()
              << ", input:" << input.get_name() << "(" << input.Inputs.size() << ")";
      }
      if ((!output.is_null()) && input.is_null())
      {
        tekst << "Block:" << output.component->GetName()
              << ", output:" << output.get_name() << "(" << output.Outputs.size() << ") -> NULL input";
      }

      DSP::log << "DSP::_connect_class::connect"  << DSP::LogMode::second << tekst.str() << endl;
      DSP::log << DSP::LogMode::Error << "DSP::_connect_class::connect"  << DSP::LogMode::second << "At least one of the input parameters is NULL" << endl;
    #endif
    return false;
  }

  if (output.Outputs.size() != input.Inputs.size())
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::_connect_class::connect" << DSP::LogMode::second
        << "block <" << output.component->GetName()
        << "> has output <" << output.get_name()
        << "> with different number of output lines (=" << output.Outputs.size()
        << ") from block <" << input.component->GetName()
        << "> input <" << input.get_name() << "> (=" << input.Inputs.size() << ")" << endl;
    #endif
    return false;
  }

  if (AllowSplit == true)
  {
    ReturnStatus = DSP::_connect_class::splitconnect(output, input);
  }
  else
  {
    ReturnStatus = true;
    for (ind=0; ind<output.Outputs.size(); ind++)
      ReturnStatus &= output.component->SetOutput(output.Outputs[ind],
                                                   input.component, input.Inputs[ind]);
  }

  return ReturnStatus;
}



//#define __DEBUG__SPLIT__
/*
 * DSP::_connect_class::splitconnect(Blok1.Out, Blok3.In) \n
 * before: Blok1.Out -> Blok2->In \n
 * after: Blok1.Out->AutoSplit(2).In\n
 * AutoSplit(2).Out1 -> Blok2.In
 * AutoSplit(2).Out2 -> Blok3.In
 */
bool DSP::_connect_class::splitconnect(const DSP::output &output, const DSP::input &input)
{
  unsigned int ind;

  #ifdef __DEBUG__SPLIT__
    DSP::log << "DSP::_connect_class::splitconnect", "Entering");
  #endif

//  if ((output == NULL) || (input == NULL))
//  {
//    #ifdef __DEBUG__
//      DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "DSP::_connect_class::splitconnect error: at least one of inputs doesn't exist");
//    #endif
//    return false;
//  }

  if (output.Outputs.size() != input.Inputs.size())
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect" << DSP::LogMode::second
        << "block >" << output.component->GetName()
        << "< has output >" << output.get_name()
        << "< with different number of output lines (" << output.Outputs.size()
        << ") from block >" << input.component->GetName()
        << "< output >" << input.get_name() << "< (" << input.Inputs.size() << ")" << endl;
    #endif
    return false;
  }

  DSP::u::Copy *sourceCopy = NULL, *destCopy = NULL;
  bool is_source_output_clear;
  DSP::Component_ptr source_block;    unsigned int  source_block_output_no;
  DSP::Block_ptr source_output_block; unsigned int  source_output_block_input_no;
  DSP::Block_ptr dest_block;          unsigned int  dest_block_input_no;

  bool is_source_auto, is_source_output_auto, is_dest_auto;
  //DSP::Block_ptr tempOut;
  int SplitterOutInd;
  DSP::u::Splitter *AutoSplitter; //, *tempSplitter;
  unsigned int tmp_ind;

  for (ind=0; ind<output.Outputs.size(); ind++)
  {
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // get source block and the block which is connected to its output
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    source_block = output.component;    source_block_output_no = output.Outputs[ind];
    sourceCopy = source_block->Convert2Copy();
    source_output_block = NULL; source_output_block_input_no = DSP::c::FO_NoOutput;
    is_source_output_clear = true;

    if (sourceCopy != NULL)
    {

      sourceCopy->GetCopyInput(output.Outputs[ind], source_block, source_block_output_no);

      if (source_block == NULL)
      {
        // nothing connected to the input but maybe we know what is at the output
        sourceCopy->GetCopyOutput(output.Outputs[ind], source_output_block, source_output_block_input_no);
      }
      else
      {
        // >> input block connection could have been updated outside without use of Copy block
        // something could have been connected to the output of the Copy's input block without use of Copy
        source_output_block = source_block->OutputBlocks[source_block_output_no];
        source_output_block_input_no = source_block->OutputBlocks_InputNo[source_block_output_no];
      }
    }
    else
    {
      source_output_block = source_block->OutputBlocks[source_block_output_no];
      source_output_block_input_no = source_block->OutputBlocks_InputNo[source_block_output_no];
    }
    if (source_output_block != NULL)
    {
      is_source_output_clear = (source_output_block == &DSP::Block::DummyBlock);
      is_source_output_auto = source_output_block->IsAutoSplit;
    }
    else
    {
      is_source_output_clear = true;
      is_source_output_auto = false;
    }
    if (source_block != NULL)
      is_source_auto = source_block->IsAutoSplit;
    else
      is_source_auto = false;
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // get destination block
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    dest_block = input.component->Convert2Block(); dest_block_input_no = input.Inputs[ind];
    destCopy = dest_block->Convert2Copy();
    if (destCopy != NULL)
    {
      destCopy->GetCopyOutput(input.Inputs[ind], dest_block, dest_block_input_no);
    }
    if (dest_block != NULL)
      is_dest_auto = dest_block->IsAutoSplit;
    else
      is_dest_auto = false;
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //




    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // Output reconfiguration if necessary
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    if ( (is_source_auto == false) && (is_source_output_clear == false) && (is_source_output_auto == false) &&
         (destCopy == NULL) )
    { // IN -> ANY(block)    ==> OUT.with_clear_input
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "Reconfiguration: 5c, 11a");
      #endif


      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      //erase connection details from block
      // ClearInput(ANY)
      source_output_block->ClearInput(source_output_block_input_no);
      // ClearOutput(IN)
      if (source_block != NULL) // non Copy block
        source_block->OutputBlocks[source_block_output_no]=&DSP::Block::DummyBlock;
      is_source_output_clear = true;

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      // Auto = AddAuto(ANY, OUT)
      // create splitter
      AutoSplitter = new DSP::u::Splitter(2U);
      AutoSplitter->AutoFree = true;
      AutoSplitter->IsAutoSplit = true;

      AutoSplitter->SetOutput(1, source_output_block, source_output_block_input_no);
      AutoSplitter->SetOutput(0, dest_block, dest_block_input_no);

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      // redefine output block ==> source_block->SetOutput(source_block_output_no, AutoSplitter, 0);
      dest_block = AutoSplitter;
      dest_block_input_no = 0;
      is_dest_auto = true;

      //continue;
    }

    if ( (is_source_auto == false) && (is_source_output_clear == false) && (is_source_output_auto == false) &&
         (destCopy == NULL) && (is_dest_auto == true) )
    { // INauto    ==> OUTauto
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "Reconfiguration: 5c2, 11b");
      #endif

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      //erase connection details from block
      // ClearInput(ANY)
      source_output_block->ClearInput(source_output_block_input_no);
      // ClearOutput(IN)
      source_block->OutputBlocks[source_block_output_no]=&DSP::Block::DummyBlock;
      is_source_output_clear = true;

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      // AddLine(OUTauto)
      AutoSplitter = ((DSP::u::Splitter *)dest_block);
      SplitterOutInd = AutoSplitter->AddOutputLine();
      // connect autosplitter new line ==> ANY
      AutoSplitter->SetOutput(SplitterOutInd,
                              source_output_block, source_output_block_input_no);

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      // redefine output block ==> source_block->SetOutput(source_block_output_no, AutoSplitter, 0);
      dest_block = AutoSplitter;
      dest_block_input_no = 0;
      is_dest_auto = true;

      //continue;
    }

    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //



    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // Update Copys' outputs and inputs if necessary
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    if ((sourceCopy == NULL) && (destCopy != NULL))
    {
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "CopyUpdate: 3, 6, 4");
      #endif

      // update destCOPY input
      destCopy->SetCopyInput(input.Inputs[ind],
                             source_block, source_block_output_no);
    }

    if ((sourceCopy != NULL) && (destCopy == NULL))
    {
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "CopyUpdate: 8, 9");
      #endif

      // update sourceCOPY output
      if ((is_dest_auto == false) && (is_source_output_auto == true))
      { // source_copy output does not need update
      }
      else
      {
        sourceCopy->SetCopyOutput(output.Outputs[ind],
                                  dest_block, dest_block_input_no);
      }
    }

    if ((sourceCopy != NULL) && (destCopy != NULL))
    {
      #ifdef __DEBUG__SPLIT__
         DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "CopyUpdate: 12, 13");
      #endif

      // update destCOPY input
      destCopy->SetCopyInput(input.Inputs[ind],
                             sourceCopy, output.Outputs[ind]);
      // update sourceCOPY output
      sourceCopy->SetCopyOutput(output.Outputs[ind],
                                destCopy, input.Inputs[ind]);
    }
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //



    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    #ifdef __DEBUG__SPLIT__
      DSP::log << "DSP::_connect_class::splitconnect", " Start");
    #endif

    //if ( (sourceCopy == NULL) && (is_source_output_clear == true) && (destCopy == NULL) )
    if ( (source_block != NULL) && (is_source_output_clear == true) &&
         (dest_block != NULL) )
    { // 2. IN.with_clear_output    ==> OUT.with_clear_input    : just connect

      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "2");
      #endif

      // Connect(IN, OUT)
      source_block->SetOutput(source_block_output_no,
                              dest_block, dest_block_input_no);

      continue;
    }

    //if ( (sourceCopy == NULL) && (is_source_auto == true) && (destCopy == NULL) )
    if ( (source_block != NULL) && (is_source_auto == true) &&
         (dest_block != NULL) )
    { // INauto    ==> OUT.with_clear_input
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "5a");
      #endif

      // AddLine(INauto, OUT)
      // +++++++++++++++++++++++++++++++++++++++++++++++++++ //
      // resize splitter
      SplitterOutInd = ((DSP::u::Splitter *)(source_block->Convert2Block()))->AddOutputLine();
      // +++++++++++++++++++++++++++++++++++++++++++++++++++ //
      // connect autosplitter new line ==> OUT
      source_block->SetOutput(SplitterOutInd,
                              dest_block, dest_block_input_no);

      continue;
    }


    //if ( (is_source_auto == false) && (is_source_output_auto == true) && (destCopy == NULL) )
    if ( (is_source_auto == false) && (is_source_output_auto == true) &&
         (dest_block != NULL) )
    { // IN -> Auto    ==> OUT.with_clear_input
      // 5b : (sourceCopy == NULL)
      // 11c : (sourceCopy != NULL)
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "5b, 11c");
      #endif

      // AddLine(Auto, OUT)
      // +++++++++++++++++++++++++++++++++++++++++++++++++++ //
      // resize splitter
      SplitterOutInd = ((DSP::u::Splitter *)source_output_block)->AddOutputLine();
      // +++++++++++++++++++++++++++++++++++++++++++++++++++ //
      // connect autosplitter new line ==> OUT
      source_output_block->SetOutput(SplitterOutInd,
                                     dest_block, dest_block_input_no);

      continue;
    }


    //if ( (sourceCopy == NULL) && (is_source_auto == false) && (is_source_output_auto == false) && (destCopy == NULL) )
    if ( (source_block != NULL) && (is_source_auto == false) && (is_source_output_auto == false) &&
         (dest_block != NULL) )
    { // IN -> ANY(block)    ==> OUT.with_clear_input
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "5c");
      #endif


      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      //erase connection details from block
      // ClearInput(ANY)
      source_output_block->ClearInput(source_output_block_input_no);
      // ClearOutput(IN)
      source_block->OutputBlocks[source_block_output_no]=&DSP::Block::DummyBlock;

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      // Auto = AddAuto(ANY, OUT)
      // create splitter
      AutoSplitter = new DSP::u::Splitter(2U);
      AutoSplitter->AutoFree = true;
      AutoSplitter->IsAutoSplit = true;

      AutoSplitter->SetOutput(1, source_output_block, source_output_block_input_no);
      AutoSplitter->SetOutput(0, dest_block, dest_block_input_no);

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      // recreate connection with additional splitter
      // Connect(IN, Auto)
      source_block->SetOutput(source_block_output_no, AutoSplitter, 0);

      continue;
    }


    //if ( (sourceCopy == NULL) && (is_source_auto == true) && (destCopy == NULL) && (is_dest_auto == true) )
    if ( (source_block != NULL) && (is_source_auto == true) &&
         (dest_block != NULL) && (is_dest_auto == true) )
    { // INauto    ==> OUTauto
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "5a2");
      #endif

      // Merge(INauto, OUTauto)
      // +++++++++++++++++++++++++++++++++++++++++++++++++++ //
      AutoSplitter = ((DSP::u::Splitter *)(source_block->Convert2Block()));
      //tempSplitter = ((DSP::u::Splitter *)dest_block);
      // resize splitter
      for (tmp_ind = 0; tmp_ind < dest_block->NoOfOutputs; tmp_ind++)
      {
        SplitterOutInd = AutoSplitter->AddOutputLine();
        // +++++++++++++++++++++++++++++++++++++++++++++++++++ //
        // connect autosplitter new line ==> OUT
        AutoSplitter->SetOutput(SplitterOutInd,
                                dest_block->OutputBlocks[tmp_ind], dest_block->OutputBlocks_InputNo[tmp_ind]);
      }
      delete dest_block;

      continue;
    }


    //if ( (is_source_auto == false) && (is_source_output_auto == true) && (destCopy == NULL) && (is_dest_auto == true) )
    if ( (is_source_auto == false) && (is_source_output_auto == true) &&
         (dest_block == NULL) && (is_dest_auto == true) )
    { // INauto    ==> OUTauto
      // 5b2: (sourceCopy == NULL)
      // 11d: (sourceCopy != NULL)
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "5b2, 11d");
      #endif

      // Merge(INauto, OUTauto)
      // +++++++++++++++++++++++++++++++++++++++++++++++++++ //
      AutoSplitter = ((DSP::u::Splitter *)source_output_block);
      //tempSplitter = ((DSP::u::Splitter *)dest_block);
      // resize splitter
      for (tmp_ind = 0; tmp_ind < dest_block->NoOfOutputs; tmp_ind++)
      {
        SplitterOutInd = AutoSplitter->AddOutputLine();
        // +++++++++++++++++++++++++++++++++++++++++++++++++++ //
        // connect autosplitter new line ==> OUT
        AutoSplitter->SetOutput(SplitterOutInd,
                                dest_block->OutputBlocks[tmp_ind], dest_block->OutputBlocks_InputNo[tmp_ind]);
      }
      delete dest_block;

      continue;
    }


    //if ( (sourceCopy == NULL) && (is_source_auto == false) && (is_source_output_auto == false) && (destCopy == NULL) && (is_dest_auto == true) )
    if ( (source_block != NULL) && (is_source_auto == false) && (is_source_output_auto == false) &&
         (dest_block != NULL) && (is_dest_auto == true) )
    { // INauto    ==> OUTauto
      #ifdef __DEBUG__SPLIT__
        DSP::log << DSP::LogMode::Error << "DSP::_connect_class::splitconnect", "5c2");
      #endif

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      //erase connection details from block
      // ClearInput(ANY)
      source_output_block->ClearInput(source_output_block_input_no);
      // ClearOutput(IN)
      source_block->OutputBlocks[source_block_output_no]=&DSP::Block::DummyBlock;

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      // AddLine(OUTauto)
      AutoSplitter = ((DSP::u::Splitter *)dest_block);
      SplitterOutInd = AutoSplitter->AddOutputLine();
      // connect autosplitter new line ==> ANY
      AutoSplitter->SetOutput(SplitterOutInd,
                              source_output_block, source_output_block_input_no);

      // +++++++++++++++++++++++++++++++++++++++++++++++++ //
      // recreate connection with additional splitter
      // Connect(INblock, Auto)
      source_block->SetOutput(source_block_output_no, AutoSplitter, 0);

      continue;
    }

    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


    #ifdef __DEBUG__SPLIT__
      DSP::log << "DSP::_connect_class::splitconnect", "unsupported configuration ???");
    #endif

  }
  #ifdef __DEBUG__SPLIT__
    DSP::log << "DSP::_connect_class::splitconnect", "end");
  #endif

  return true;
}

void DSP::Component::ClearOutput(unsigned int No)
{
  OutputBlocks[No]=&DSP::Block::DummyBlock;
}


//***************************************************//
//  DSP::Block
//***************************************************//
DSP::Block DSP::Component::DummyBlock;

DSP::Block::Block(void) : DSP::Component()
{
  Type = DSP::Component::Component_type(Type | DSP_CT_block); //might be mixed

//  RegisterComponent();

//  EnableNotifycations=false;

  IsMultirate=false;
//  L_factor=1; M_factor=1;

  //ProtectOutputClock = false;

  AutoFree=false;
  NoOfInputsConnected=0;

  SetName("Noname block", false);
//  SourceReady=NULL;
  OutputBlocks=NULL; OutputClocks=NULL;
  OutputBlocks_InputNo=NULL;

  InputClocks.resize(0);
  ConstantInputValues.resize(0);
  IsConstantInput.resize(0);


  IsUsingConstants = false;
  BlockAllowsForConstantInputs = false;
  NoOfOutputs=0; // SetNoOfOutputs initiation depends on it
  NoOfInputs=0;  // SetNoOfInputs  initiation depends on it
  NoOfInputsProcessed=0;
  InitialNoOfInputsProcessed=0;
//  IsMultiClock=false;

  SetNoOfOutputs(0); //initiation
  SetNoOfInputs(0, 0, false);

  DefinedOutputs.clear();
  DefinedInputs.clear();

  // Set default Execute function
  Execute_ptr = &DummyExecute;
}

DSP::Source::Source(void) : DSP::Component()
{
  Type = DSP::Component::Component_type(Type | DSP_CT_source); //might be mixed

//  RegisterComponent();

//  IsActive = true;

//  IsControlled = false;
//  IsSourceStateReady = false; // activation_time_length=0;
//  IsControlledActive = false;

//  EnableNotifycations=false;
//  EnableProcessing = false; // not registered for processing yet

  //ProtectOutputClock = false;

  AutoFree=false;
//  NoOfInputsConnected=0;

  SetName("Noname source", false);
  SourceReady=NULL;
  OutputBlocks=NULL; OutputClocks=NULL;
  OutputBlocks_InputNo=NULL;
//  ConstantInputValues=NULL;
//  IsConstantInput=NULL;


  NoOfOutputs=0; // SetNoOfOutputs initiation depends on it
//  NoOfInputs=0;  // SetNoOfInputs  initiation depends on it
//  NoOfInputsProcessed=0;

  SetNoOfOutputs(0); //initiation
//  SetNoOfInputs(0, 0, false);

  DefinedOutputs.clear();
//  DefinedInputsCounter=0;
//  DefinedInputs=NULL;

  OutputExecute_ptr = &DummyExecute;
}

DSP::Block::~Block(void)
{
  DSP::Component_ptr tempComponent;
  unsigned int ind;
  unsigned int tempNo;

  for (ind=0; ind<NoOfInputs; ind++)
  {
    if (IsOutputConnectedToThisInput2((DSP::Component_ptr)this, ind,
                        tempComponent, tempNo) == true)
    { // if something points at this block delete that connection
      //tempComponent->OutputBlocks[tempNo]=NULL;
      if (tempComponent != NULL) // non constant input
      {
        //### delete blocks' (block => this) outputs
        tempComponent->ClearOutput(tempNo);
      }
    }
    IsOutputConnectedToThisInput2((DSP::Component_ptr)this, ind,
                                  tempComponent, tempNo); // should be false
    while (tempComponent != NULL) // non constant COPY input
    {
      //### delete Copys' (Copy => this) outputs
      tempComponent->ClearOutput(tempNo);

      IsOutputConnectedToThisInput2((DSP::Component_ptr)this, ind,
                                    tempComponent, tempNo); // should be false
    }
  }

  // delete Copys' (this <= Copy) inputs
  //! \bug 31.03.2010 NOT implemented: delete Copys' (this <= Copy) inputs

  SetNoOfInputs(0, 0, false);

  if (DefinedInputs.empty() == false)
  {
    DefinedInputs.clear();
  }
}

DSP::Source::~Source(void)
{
  // This is already done in DSP::Component destructor
  // SetNoOfOutputs(0);
  UnregisterOutputClocks();
}

/* \test Test this function when it is used together with
 *  IsControlled and IsStateReady
 * /
void DSP::Source::SetState(bool TurnOn, DSP::Clock_ptr ParentClockOfTheControler)
{
  if (ParentClockOfTheControler != NULL)
  {
    IsActive = false;
    IsControlled = true;

    activation_time_length = ParentClockOfTheControler->cycle_length;
    IsControlledActive = TurnOn;
    IsSourceStateReady = true;
  }
  else
  {
    IsActive = TurnOn;
    IsControlled = false;
  }
}

void DSP::Source::UpdateSourceState(void)
{
  if (activation_time_length > 0)
  {
    // IsSourceStateReady should be >true< so no need to change
    activation_time_length -= OutputClocks[0]->cycle_length;
  }
  else
    IsSourceStateReady = false;

}
*/

void DSP::Component::RegisterComponent(void)
{
  DSP::Component_ptr *temp;

  if (ComponentsTableSize == NoOfComponentsInTable)
  //if ((NoOfComponentsInTable % ComponentsTableSegmentSize)==0)
  { // We need more space in the table
    temp=ComponentsTable;
    ComponentsTableSize += ComponentsTableSegmentSize;
    ComponentsTable = new DSP::Component_ptr[ComponentsTableSize];
    if (temp != NULL)
    {
      memcpy(ComponentsTable, temp, NoOfComponentsInTable*sizeof(DSP::Component_ptr));
      delete [] temp;
    }
  }
  ComponentsTable[NoOfComponentsInTable++]=this;
}

// returns given component's index in ComponentsTable
long DSP::Component::GetComponentIndexInTable(DSP::Component_ptr component)
{
  long int ind, current_index;

  current_index=-1;
  //find where the block is on the list
  for (ind=0; ind<NoOfComponentsInTable; ind++)
  {
    if (ComponentsTable[ind]==component)
    {
      current_index=ind;
      break;
    }
  }
  return current_index;
}

void DSP::Component::UnregisterComponent(void)
{
  long int ind, current_component;

  current_component=-1;
  //find where the block is on the list
  for (ind=0; ind<NoOfComponentsInTable; ind++)
  {
    if (ComponentsTable[ind]==this)
    {
      current_component=ind;
      break;
    }
  }
  if (current_component != -1)
  { //remove block from the list
    if (current_component<(NoOfComponentsInTable-1))
    { //move all following blocks
      for (ind=current_component+1; ind<NoOfComponentsInTable; ind++)
        ComponentsTable[ind-1]=ComponentsTable[ind];
    }
    NoOfComponentsInTable--;

    //shrink list if necessary : might be omitted
    // RegisterBlock will deal with it
  }
  #ifdef __DEBUG__
    else
    {
      DSP::log << DSP::LogMode::Error << "UnregisterComponent" << DSP::LogMode::second
        << "WARNING: trying unregister component (" << this->GetName() << ") which is not in the ComponentsTable." << endl;
    }
  #endif

/*
  DSP::Block_ptr *temp;

  if ((NoOfBlocksInTable % BlocksTableSegmentSize)==0)
  { // We need more space in the table
    temp=BlocksTable;
    BlocksTable= new DSP::Block_ptr[NoOfBlocksInTable + BlocksTableSegmentSize];
    if (temp != NULL)
    {
      memcpy(BlocksTable, temp, NoOfBlocksInTable*sizeof(DSP::Block_ptr));
      delete [] temp;
    }
  }
  BlocksTable[NoOfBlocksInTable++]=this;
*/
}

void DSP::Block::SetNoOfInputs(unsigned int No_real, bool AllowForConstantInputs)
{
  SetNoOfInputs(No_real, 0, AllowForConstantInputs);
}
/*! \Fixed <b>2005.11.02</b> InputClocks reserved only if NoOfInputs != 0
 */
void DSP::Block::SetNoOfInputs(unsigned int No_real, unsigned int No_complex, bool AllowForConstantInputs)
{
  NoOfInputsConnected=0;

//  if (NoOfInputs > 0)
//  { //free memory
  ConstantInputValues.resize(0);
  IsConstantInput.resize(0);
  InputClocks.resize(0);
//  }

  NoOfRealInputs=No_real;
  NoOfInputs=No_real + No_complex*2;
  //first complex input has number: NoOfRealInputs
  IsComplex_real_odd= ((NoOfRealInputs % 2) == 1);

  if (NoOfInputs==0)
  {
    AllowForConstantInputs=false;
    InputClocks.resize(0);
  }
  else
  {
    InputClocks.resize(NoOfInputs, NULL);
  }
  IsUsingConstants=false;
  BlockAllowsForConstantInputs = AllowForConstantInputs;

  InitialNoOfInputsProcessed = 0;
  NoOfInputsProcessed = InitialNoOfInputsProcessed;
}

unsigned int DSP::Block::GetNoOfInputs(void)
{
  return NoOfInputs;
}

/* Indicates given input as constant value (no coutput can
 * be connected to it)
 */
bool DSP::Block::SetConstInput(const string &InputName, DSP::Float value)
//bool DSP::Block::SetConstInput(int InputNo, DSP::Float value)
{
  unsigned int ind;
  unsigned int InputNo;

  if (BlockAllowsForConstantInputs == false)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Block::SetConstInput" << DSP::LogMode::second
        << "Block >>" << GetName() << "<< doesn't support constant inputs !!!" << endl;
    #endif
    return false;
  }

  DSP::input &temp_input = Input(InputName);
  if (temp_input.is_null() == false)
    InputNo = temp_input.Inputs[0];
  else
    InputNo = NoOfInputs;

  if (InputNo < NoOfInputs)
  {
    if (ConstantInputValues.empty() == true)
    {
      ConstantInputValues.resize(NoOfInputs, 0.0);
    }
    if (IsConstantInput.empty() == true)
    {
      IsConstantInput.resize(NoOfInputs, false);
    }

    ConstantInputValues[InputNo] = value;
    IsConstantInput[InputNo] = true;

    InitialNoOfInputsProcessed=0;
    for (ind=0; ind<NoOfInputs; ind++)
      if (IsConstantInput[ind] == true)
        InitialNoOfInputsProcessed++;
    NoOfInputsProcessed=InitialNoOfInputsProcessed;

    IsUsingConstants = true;

    this->RecalculateInitials();

    return true;
  }

  return false;
}

/* Indicates given input as constant value
 * InputNo   -> real_value
 * InputNo+1 -> imag_value
 * (no coutput can be connected to it)
 */
bool DSP::Block::SetConstInput(const string &InputName, DSP::Float real_value, DSP::Float imag_value)
{
  unsigned int ind;
  unsigned int InputNo;

  if (BlockAllowsForConstantInputs == false)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Block::SetConstInput" << DSP::LogMode::second
        << "Block >>" << GetName() << "<< doesn't support for constant inputs !!!" << endl;
    #endif
    return false;
  }

  DSP::input &temp_input = Input(InputName);
  if (temp_input.is_null() == false)
    InputNo = temp_input.Inputs[0];
  else
    InputNo = NoOfInputs;

  if (InputNo+1 < NoOfInputs)
  {
    if (ConstantInputValues.empty() == true)
    {
      ConstantInputValues.resize(NoOfInputs, 0.0);
    }
    if (IsConstantInput.empty() == true)
    {
      IsConstantInput.resize(NoOfInputs, false);
    }

    ConstantInputValues[InputNo] = real_value;
    IsConstantInput[InputNo] = true;
    ConstantInputValues[InputNo+1] = imag_value;
    IsConstantInput[InputNo+1] = true;

    InitialNoOfInputsProcessed=0;
    for (ind=0; ind<NoOfInputs; ind++)
      if (IsConstantInput[ind] == true)
        InitialNoOfInputsProcessed++;
    NoOfInputsProcessed=InitialNoOfInputsProcessed;

    IsUsingConstants = true;

    this->RecalculateInitials();

    return true;
  }

  return false;
}

void DSP::Component::SetNoOfOutputs(unsigned int No, bool reset)
{
  unsigned int ind;
  unsigned int previous_NoOfOutputs;
  DSP::Clock_ptr *tmp_clocks;
  DSP::Block_ptr *tmp_block;
  unsigned int *tmp_input_no;

  //one SourceReady and one OutputClocks pointer per one output
  //If unit is not source OutputClocks[0] == NULL
  previous_NoOfOutputs = NoOfOutputs;
  tmp_clocks = OutputClocks;
  tmp_block = OutputBlocks;
  tmp_input_no = OutputBlocks_InputNo;

  if (NoOfOutputs == 0)
    reset = true; // there are no previous entries

  if (No>0)
  {
    NoOfOutputs=No;
    OutputClocks = new DSP::Clock_ptr [No];
    OutputBlocks = new DSP::Block_ptr [No];
    OutputBlocks_InputNo = new unsigned int [No];
  }
  else
  {
    NoOfOutputs=0;
    OutputClocks = NULL;
    OutputBlocks = NULL;
    OutputBlocks_InputNo = NULL;
  }

  if (reset == true)
  {
    for (ind=0; ind < NoOfOutputs; ind++)
    {
      OutputClocks[ind] = NULL;

      OutputBlocks[ind] = &DummyBlock;
      OutputBlocks_InputNo[ind] = 0;
    }
  }
  else
  {
    for (ind=0; ind < NoOfOutputs; ind++)
    {

      if (ind < previous_NoOfOutputs)
      {
        OutputClocks[ind] = tmp_clocks[ind];
        OutputBlocks[ind] = tmp_block[ind];
        OutputBlocks_InputNo[ind] = tmp_input_no[ind];
      }
      else
      {
        OutputClocks[ind] = NULL;
        OutputBlocks[ind] = &DummyBlock;
        OutputBlocks_InputNo[ind] = 0;
      }
    }
  }

  // free previously reserved memory
  if (previous_NoOfOutputs > 0)
  {
    delete [] tmp_clocks;
    delete [] tmp_block;
    delete [] tmp_input_no;
  }
}

int DSP::Component::AddOutputLine(void)
{
  SetNoOfOutputs(NoOfOutputs+1, false);
  return NoOfOutputs-1;
}



void DSP::Source::SetNoOfOutputs(unsigned int No)
{
  unsigned int ind;

  //one SourceReady and one OutputClocks pointer per one output
  //If unit is not source OutputClocks[0] == NULL
  if (NoOfOutputs>0)
  {
    delete [] SourceReady;
  }

  DSP::Component::SetNoOfOutputs(No);
  if (NoOfOutputs>0)
  {
    SourceReady = new bool [NoOfOutputs];
    for (ind=0; ind < NoOfOutputs; ind++)
    {
      SourceReady[ind] = false;
    }
  }
  else
  {
    SourceReady = NULL;
  }

  IsMultiClock=false;
  for (ind=1; ind < NoOfOutputs; ind++)
  {
    if (OutputClocks[ind] != NULL)
    {
      IsMultiClock=true;
      break;
    }
  }

}

/*! should only perform unregister if source is being destroyed
 */
void DSP::Source::UnregisterOutputClocks(void)
{
  unsigned int ind;

  if (NoOfOutputs > 0)
  {
    if (IsMultiClock)
    {
      for (ind=0; ind < NoOfOutputs; ind++)
      {
        if (OutputClocks[ind] != NULL)
          DSP::Clock::UnregisterSource(OutputClocks[ind], this);
        OutputClocks[ind] = NULL;
      }
    }
    else
    {
      if (OutputClocks[0] != NULL)
        DSP::Clock::UnregisterSource(OutputClocks[0], this);
      OutputClocks[0] = NULL;
    }
  }
}

void DSP::Source::RegisterOutputClock(DSP::Clock_ptr OutputClock, unsigned int output_index)
{
  if (OutputClock == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Source::RegisterOutputClock" << DSP::LogMode::second
        << "Attempt to register source <" << GetName()
        << "> to NULL clock (output_index = " << output_index  << ")" << endl;
    #endif
    return;
  }
  if (output_index >= NoOfOutputs)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Source::RegisterOutputClock" << DSP::LogMode::second
        << "Attempt to register source output with index out of range = " << output_index << endl;
    #endif
    return;
  }

  OutputClocks[output_index] = OutputClock;

  if (output_index > 0)
    IsMultiClock = true;
  else
    IsMultiClock = false;

  OutputClock->RegisterSource(this);
}

void DSP::Component::UnregisterNotifications(void)
{
  unsigned int ind;

  for (ind = 0; ind < NotificationClocks.size(); ind++)
    DSP::Clock::UnregisterNotification(NotificationClocks[ind], this);

  NotificationClocks.clear();
}

void DSP::Component::RegisterForNotification(DSP::Clock_ptr NotifyClock)
{
  if (NotifyClock == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Source::RegisterClockForNotification" << DSP::LogMode::second
        << "Attempt to register source <" << GetName() << "> to NULL clock" << endl;
    #endif
    return;
  }

  NotificationClocks.push_back(NotifyClock);

  NotifyClock->RegisterNotification(this); // only for notifications
}
//Connect the given DSP::Block input number InputNo
//to the output number OutputNo of the current DSP::Block
bool DSP::Component::SetOutput(DSP::Component_ptr Block, unsigned int InputNo)
{
  return SetOutput(0, Block, InputNo);
}

bool DSP::Component::SetOutput(unsigned int OutputNo, DSP::Component_ptr component, unsigned int InputNo)
{ //OutputNo==0;
  DSP::Block_ptr  tempBlock;
  DSP::u::Copy_ptr  tempCopy;
  DSP::Component_ptr input_block;
  unsigned int input_block_output_no;
  DSP::Block_ptr output_block;
  unsigned int output_block_input_no;
  bool do_connect;

  if (OutputNo >= NoOfOutputs)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second
        << "Can't connect output (" << GetName() << ":" << OutputNo
        << ") to input (" << component->GetName() << ":" << InputNo << "). "
           "Such an output doesn't exist - too large output number." << endl;
    #endif
    return false;
  }

  if (component == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second << "Can't connect to input. Output block not specified (NULL)." << endl;
    #endif
    return false;
  }

  tempBlock = component->Convert2Block();
  if (tempBlock == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second << "Can't connect to input. Output component is the source (no inputs)." << endl;
    #endif
    return false;
  }

  if (InputNo >= tempBlock->NoOfInputs)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second
        << "Can't connect output (" << component->GetName() << ":" << InputNo
        << ") to input (" << GetName() << ":"<< OutputNo
        << "). Input (" << component->GetName() << ":" << InputNo
        << ") does not exist - too large input number." << endl;
    #endif
    return false;
  }

  if (OutputBlocks[OutputNo] != &DSP::Block::DummyBlock)
  {
    if (OutputBlocks[OutputNo]->Type == DSP_CT_copy)
    {
      tempCopy = OutputBlocks[OutputNo]->Convert2Copy();
      if (tempCopy->GetCopyOutput(InputNo, output_block, output_block_input_no) == true)
      {
        #ifdef __DEBUG__
          DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second
            << "Can't connect output (" << GetName() << ":" << OutputNo
            << ") to input(DSP::u::Copy) (" << component->GetName() << ":" << InputNo
            << "). DSP::u::Copy Output (" << tempCopy->GetName() << ":" << InputNo
            << ") is already connected to (" << output_block->GetName() << ":" << output_block_input_no << ")." << endl;
        #endif
      }
    }
    else
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second
          << "Can't connect output (" << GetName() << ":" << OutputNo
          << ") to input (" << component->GetName() << ":" << InputNo
          << "). Output (" << GetName() << ":" << OutputNo
          << ") is already connected to ("
          << OutputBlocks[OutputNo]->GetName() << ":" << OutputBlocks_InputNo[OutputNo] << ")." << endl;
      #endif
      return false;
    }
  }

  if (tempBlock->NoOfInputs == 0)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second
        << "Cant't connect output (" << GetName() << ":" << OutputNo
        << ") to input (" << component->GetName() << ":" << InputNo << "). "
           "Destination component has no inputs!" << endl;
    #endif
    return false;
  }

  if (!(tempBlock->IsInputAvailable(InputNo)))
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second
        << "Cant't connect output (" << GetName() << ":" << OutputNo
        << ") to input (" << component->GetName() << ":" << InputNo
        << "). Something is already connected to input ("
        << component->GetName() << ":" << InputNo << ")." << endl;
    #endif
    return false;
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  do_connect = true;

  input_block = this;
  input_block_output_no = OutputNo;

  if (Type == DSP_CT_copy)
  { // DSP::u::Copy output info update
    tempCopy = this->Convert2Copy();
    if (tempCopy->SetCopyOutput(OutputNo, tempBlock, InputNo) == true)
    {
      if (tempCopy->GetCopyInput(OutputNo, input_block, input_block_output_no) == false)
      {
        #ifdef __DEBUG__
          DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second
            << "GetCopyInput(" << tempCopy->GetName() << ":" << InputNo << ") failed!" << endl;
        #endif
        return false;
      }
    }
//    else
//      do_connect = false;
  }
  output_block = tempBlock;
  output_block_input_no = InputNo;

  if (tempBlock->Type == DSP_CT_copy)
  { // DSP::u::Copy input info update
    tempCopy = tempBlock->Convert2Copy();
    if (tempCopy->SetCopyInput(InputNo, this, OutputNo) == true)
    {
      //bool GetCopyOutput(unsigned int OutputNo, DSP::Block_ptr &output_block, unsigned int &output_block_InputNo);
      if (tempCopy->GetCopyOutput(InputNo, output_block, output_block_input_no) == false)
      {
        #ifdef __DEBUG__
          DSP::log << DSP::LogMode::Error << "DSP::Component::SetOutput" << DSP::LogMode::second
            << "GetCopyOutput(" << tempCopy->GetName() << ":" << OutputNo << ") failed!" << endl;
        #endif
        return false;
      }
    }
//    else
//      do_connect = false;
  }

  // if do_connect == false DSP::u::Copy only stored connection data
  // but there is no enough data to make actual connection
  if (do_connect == true)
  {
    input_block->OutputBlocks[input_block_output_no]=output_block;
    input_block->OutputBlocks_InputNo[input_block_output_no]=output_block_input_no;

    if (output_block->IsMultiClock)
      output_block->SetBlockInputClock(output_block_input_no,
            input_block->OutputClocks[input_block_output_no]);
    else
      output_block->SetBlockInputClock(output_block_input_no,
            input_block->OutputClocks[0]);
    output_block->NoOfInputsConnected++;
  }

  return true;
}

bool DSP::Block::ClearInput(unsigned int InputNo)
{
  if (NoOfInputsConnected <= 0)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Block::ClearInput" << DSP::LogMode::second
        << "ClearInput: Input (" << GetName() << ":" << InputNo << ") must have already been cleared !!!" << endl;
    #endif
  }
  else
  {
    DSP::Component_ptr temp;
    unsigned int tempNo;

//    if (GetBlockInputClock(InputNo) != NULL)
    if (IsOutputConnectedToThisInput2(this, InputNo, temp, tempNo) == true)
    {
      SetBlockInputClock(InputNo, NULL);
      NoOfInputsConnected--;
    }
  }

  return ((NoOfInputsConnected == InitialNoOfInputsProcessed) &&
          (AutoFree == true));
}

DSP::Clock_ptr DSP::Block::GetBlockInputClock(unsigned int InputNo)
{
  if (InputClocks.size() > InputNo)
  {
    return InputClocks[InputNo];
  }

  return NULL;
}

bool clock_group::IsInputInGroup(const int &input_no)
{
  for (unsigned int ind=0; ind < InputsIndexes.size(); ind++)
  {
    if (InputsIndexes[ind] == input_no)
      return true;
  }
  return false;
}

std::string DSP::clock_groups::FindGroupName4Input(int input_index)
{
  for(auto &entry : groups)
  { // entry.first is the key
    // entry.second is the data
    std::vector<int> &input_indexes = entry.second.InputsIndexes;
    for (unsigned int n=0; n<input_indexes.size(); n++)
    {
      if (input_indexes[n] == input_index)
        return entry.first;
    }
  }
  return std::string("");
}

std::string DSP::clock_groups::FindGroupName4Output(int output_index)
{
  for(auto &entry : groups)
  { // entry.first is the key
    // entry.second is the data
    std::vector<int> &output_indexes = entry.second.OutputsIndexes;
    for (unsigned int m=0; m<output_indexes.size(); m++)
    {
      if (output_indexes[m] == output_index)
        return entry.first;
    }
  }
  return std::string("");
}

//! returns clock for the given input based on the clock of the reference group with given name
DSP::Clock_ptr DSP::clock_groups::FindClock4Input(const std::string &group_name, const int &input_index)
{
  DSP::Clock_ptr reference_clock = groups[group_name].GroupClock;

  std::string input_group_name = FindGroupName4Input(input_index);

  if (group_name == input_group_name)
  { // input belongs to the reference group
    return reference_clock;
  }

//#ifdef __DEBUG__
//  DSP::log << "DSP::clock_groups::FindClock4Input");
//  DSP::log << "group_name", group_name);
//  DSP::log << "input_group_name", input_group_name);
//#endif // __DEBUG__

  long L = clocks_relations[group_name][input_group_name].L;
  long M = clocks_relations[group_name][input_group_name].M;
  if (L == 0)
  {
    // reverse clocks' relation
    M = clocks_relations[input_group_name][group_name].L;
    L = clocks_relations[input_group_name][group_name].M;
  }

  if (L == 0)
  {
    DSP::log << DSP::LogMode::Error << "DSP::clock_groups::FindClock4Input" << DSP::LogMode::second
      << "there is no clocks' groups relation defined for given reference group and input index" << endl;
    return NULL;
  }

  if ((L == -1) || (M == -1))
  { // clocks are asynchronous
    return NULL;
  }

  return DSP::Clock::GetClock(reference_clock, L,M);
}

//! returns clock for the given input based on the clock of the reference group with given name
DSP::Clock_ptr DSP::clock_groups::FindClock4Output(const std::string &group_name, const int &output_index)
{
  std::stringstream ss;

  DSP::Clock_ptr reference_clock = groups[group_name].GroupClock;

  ss << "reference_clock=" << reference_clock << std::endl;

  std::string output_group_name = FindGroupName4Output(output_index);

  if (group_name == output_group_name)
  { // output belongs to the reference group
    return reference_clock;
  }

  long L = clocks_relations[group_name][output_group_name].L;
  long M = clocks_relations[group_name][output_group_name].M;
  if (L == 0)
  {
    // reverse clocks' relation
    M = clocks_relations[output_group_name][group_name].L;
    L = clocks_relations[output_group_name][group_name].M;
  }

  if (L == 0)
  {
    DSP::log << DSP::LogMode::Error << "DSP::clock_groups::FindClock4Input" << DSP::LogMode::second
      << "there is no clocks' groups relation defined for given reference group and input index" << endl;
    return reference_clock;
  }

  if ((L == -1) || (M == -1))
  { // clocks are asynchronous
    return NULL;
  }

  return DSP::Clock::GetClock(reference_clock, L,M);
}

void DSP::clock_groups::AddInput2Group(const std::string &name, const DSP::input &input)
{
  for (unsigned int ind = 0; ind < input.Inputs.size(); ind++)
  {
    AddInput2Group(name, input.Inputs[ind]);
  }
}

void DSP::clock_groups::AddInput2Group(const std::string &name, const int &index)
{
  if (index < 0)
    return;

  if (groups.count(name) > 0)
  {
    // attempt to add to existing group
    std::vector<int> &input_indexes = groups.at(name).InputsIndexes;

    bool exists = false;

    // check is given inputs is not yet on the list
    for (unsigned int m=0; m < input_indexes.size(); m++)
    {
      if (input_indexes[m]==index)
      {
        exists = true;
        break;
      }
    }

    if (exists == false)
      input_indexes.push_back(index);
  }
  else
  {
    //std::cerr << "Out of Range error: " << oor.what() << '\n';
    // add new group
    std::vector<int> &input_indexes = groups[name].InputsIndexes;

    input_indexes.push_back(index);
  }
}

void DSP::clock_groups::AddInputs2Group(const std::string &name, const int &index_start, const int &index_end)
{
  if (index_end < 0)
    return;

  if (groups.count(name) > 0) {
    // attempt to add to existing group
    std::vector<int> &input_indexes = groups.at(name).InputsIndexes;

    for (int ind = index_start; ind <= index_end; ind++)
    {
      bool exists = false;

      // check is given inputs is not yet on the list
      for (unsigned int m=0; m < input_indexes.size(); m++)
      {
        if (input_indexes[m]==ind)
        {
          exists = true;
          break;
        }
      }

      if (exists == false)
        input_indexes.push_back(ind);
    }
  }
  else
  {
    //std::cerr << "Out of Range error: " << oor.what() << '\n';
    // add new group
    std::vector<int> &input_indexes = groups[name].InputsIndexes;

    for (int ind = index_start; ind <= index_end; ind++)
    {
      input_indexes.push_back(ind);
    }
  }
}

void DSP::clock_groups::AddOutput2Group(const std::string &name, const DSP::output &output)
{
  for (unsigned int ind = 0; ind < output.Outputs.size(); ind++)
  {
    AddOutput2Group(name, output.Outputs[ind]);
  }
}

void DSP::clock_groups::AddOutput2Group(const std::string &name, const int &index)
{
  if (index < 0)
    return;

  if (groups.count(name) > 0)
  {
    // attempt to add to existing group
    std::vector<int> &output_indexes = groups.at(name).OutputsIndexes;

    bool exists = false;

    // check is given inputs is not yet on the list
    for (unsigned int m=0; m < output_indexes.size(); m++)
    {
      if (output_indexes[m]==index)
      {
        exists = true;
        break;
      }
    }

    if (exists == false)
      output_indexes.push_back(index);
  }
  else
  {
    //std::cerr << "Out of Range error: " << oor.what() << '\n';
    // add new group
    std::vector<int> &output_indexes = groups[name].OutputsIndexes;

    output_indexes.push_back(index);
  }
}

void DSP::clock_groups::AddOutputs2Group(const std::string &name, const int &index_start, const int &index_end)
{
  if (index_end < 0)
    return;

  if (groups.count(name) > 0)
  {
    // attempt to add to existing group
    std::vector<int> &output_indexes = groups.at(name).OutputsIndexes;

    for (int ind = index_start; ind <= index_end; ind++)
    {
      bool exists = false;

      // check is given inputs is not yet on the list
      for (unsigned int m=0; m < output_indexes.size(); m++)
      {
        if (output_indexes[m]==ind)
        {
          exists = true;
          break;
        }
      }

      if (exists == false)
        output_indexes.push_back(ind);
    }
  }
  else
  {
    //std::cerr << "Out of Range error: " << oor.what() << '\n';
    // add new group
    std::vector<int> &output_indexes = groups[name].OutputsIndexes;

    for (int ind = index_start; ind <= index_end; ind++)
    {
      output_indexes.push_back(ind);
    }
  }
}

void DSP::clock_groups::AddClockRelation(const std::string &parent_group, const std::string &child_group, const long &L, const long &M)
{
  clocks_relations[parent_group][child_group] = DSP::clocks_relation(L, M);
}

/* The clock is then compared with clock for the other inputs
 * or outputs (especially mixed blocks outputs).
 * If it is also possible (if the output
 *  clock can be determined) the output clock should be also stored.
 * If the output Clock is updated Input clock of blocks connected to
 * that output should also be updated.
 *
 * This function in general is block dependent.
 */
void DSP::Block::SetBlockInputClock(unsigned int InputNo, DSP::Clock_ptr InputClock)
{
  unsigned int ind;
  DSP::Clock_ptr tmp_OutputClock;

  if (ClockGroups.groups.empty() == true)
  { // create group "all" containing all blocks' inputs and outputs
#ifdef __DEBUG__
    DSP::log << "DSP::Block::SetBlockInputClock" << DSP::LogMode::second
      << "creating clock group >>all<< for block " << GetName() << endl;
#endif // __DEBUG__
    if (NoOfInputs > 0)
    {
      ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
    }
    if (NoOfOutputs > 0)
    {
      ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);
    }
  }

  if (InputClock==NULL)
  { // Input clock must be cleared and if necessary also output blocks input clocks

    // clears the clock associated with given input.
    InputClocks[InputNo]=NULL;
    tmp_OutputClock=NULL; //Output clocks must be also cleared !!!

    // If it is also possible (that means if the output clock can be determined)
    // the output clock should be also stored.
    //if ((NoOfOutputs > 0) && (ProtectOutputClock == false))
    if ((NoOfOutputs > 0) && (this->Convert2Source() == NULL))
    { // if block is source block output clocks must not be NULL
      if (IsMultiClock)
      {
        for (ind=0; ind<NoOfOutputs; ind++)
        {
          if (OutputClocks[ind] != NULL)
          {
            OutputClocks[ind] = NULL;
            // If the output Clock is updated Input clock of blocks connected to
            // that output should also be updated.

            if (OutputBlocks[ind] != &DSP::Block::DummyBlock)
              OutputBlocks[ind]->SetBlockInputClock(OutputBlocks_InputNo[ind], NULL);
          }
        }
      }
      else
      {
        if (OutputClocks[0] != NULL)
        {
          OutputClocks[0] = NULL;
          // If the output Clock is updated Input clock of blocks connected to
          // that output should also be updated.
          for (ind=0; ind<NoOfOutputs; ind++)
          {
            if (OutputBlocks[ind] != NULL)
              if (OutputBlocks[ind] != &DummyBlock)
                OutputBlocks[ind]->SetBlockInputClock(OutputBlocks_InputNo[ind], NULL);
          }
        }
      }
    }
    return;
  }

  if (InputNo<NoOfInputs)
  {
    #ifdef __DEBUG__
      if (InputClocks[InputNo] != NULL)
      {
        DSP::log << DSP::LogMode::Error << "Input clock has been already set" << endl;
      }
    #endif // __DEBUG__

    // Stores for the current block the clock associated with given input.
    InputClocks[InputNo]=InputClock;
    std::string reference_input_group_name = ClockGroups.FindGroupName4Input(InputNo);


    // The clock is then compared with clocks for the other inputs
    if (ClockGroups.groups[reference_input_group_name].GroupClock  != NULL)
    {
#ifdef __DEBUG__
      if (ClockGroups.groups[reference_input_group_name].GroupClock  != InputClock)
      {
        DSP::log << DSP::LogMode::Error << GetName() << DSP::LogMode::second << "Group clock has been already set to different value." << endl;
      }
#endif
    }
    else
    {
      ClockGroups.groups[reference_input_group_name].GroupClock = InputClock;
    }

    #ifdef __DEBUG__
      DSP::Clock_ptr tmp_InputClock = InputClock;
      bool AllOK;

      #ifdef VerboseCompilation
        printf ("Testing Input: (%s:%i)\n", BlockName,InputNo);
        //getchar();
      #endif

      AllOK = true;
      for (ind=0; ind<NoOfInputs; ind++)
      {
        if (ClockGroups.groups[reference_input_group_name].IsInputInGroup(ind) == true)
        {
          // 1. all clocks for inputs in the group must have the same clock
          tmp_InputClock = InputClock;
        }
        else
        {
          if (ClockGroups.FindGroupName4Input(ind).size() > 0)
          {
            // 2. all clocks for inputs from outside the group must have the clock as defined in groups clocks'relations
            tmp_InputClock = ClockGroups.FindClock4Input(reference_input_group_name, ind);
          }
          else
          {
            DSP::log << "DSP::Block::SetBlockInputClock" << DSP::LogMode::second
              << "In block <" << GetName() << "> There is no group defined for input number " << ind << endl;
            tmp_InputClock = InputClock;
          }
        }
        // if tmp_InputClock == NULL then clocks relation is either undefined or clocks arre asynchronous
        if ((InputClocks[ind]!=NULL) && (tmp_InputClock != NULL))
          if (InputClocks[ind]!=tmp_InputClock)
            AllOK = false;
      }


      if (AllOK == false)
      {
        DSP::log << DSP::LogMode::Error << "SetBlockInputClock" << DSP::LogMode::second
          << "Input clocks mismatch in block (" << GetName()
          << ") possibly at input (" << InputNo << ")" << endl;
      }
    #endif


    // podobnie dla zegar�w wyj�ciowych
    if (NoOfOutputs > 0)
    {
      if (IsMultiClock == true)
      {
        for (ind=0; ind<NoOfOutputs; ind++)
        {
          //std::string output_group_name = ClockGroups.FindGroupName4Input(m);
          if (ClockGroups.FindGroupName4Output(ind).size() > 0)
          {
            tmp_OutputClock = ClockGroups.FindClock4Output(reference_input_group_name, ind);
          }
          else
          {
            DSP::log << "DSP::Block::SetBlockInputClock" << DSP::LogMode::second
              << "In block <" << GetName() << "> There is no group defined for output number " << ind << endl;
            tmp_OutputClock = InputClock;
          }

          if (tmp_OutputClock != NULL)
          {
            // 1.  Check if OutputClock is already set
            if (OutputClocks[ind]!=NULL)
            {
              // 2a.  If it is set then check if it is correct
              if (OutputClocks[ind]!=tmp_OutputClock)
              {
                #ifdef __DEBUG__
                  DSP::log << DSP::LogMode::Error << "SetBlockInputClock" << DSP::LogMode::second
                    << "Input clock differs from output clock for ("
                    << GetName() << ":" << ind << ") probably at input ("
                    << InputNo << ")!!!" << endl;
                #endif
              }
              else
              { // clock is already set and it is OK
                // Do nothing
              }
            }
            else
            {
              // 2b.  If not then update it
              OutputClocks[ind] = tmp_OutputClock;
              // If the output Clock is updated Input clock of blocks connected to
              // that output should also be updated.
              if (OutputBlocks[ind] != &DSP::Block::DummyBlock)
                OutputBlocks[ind]->SetBlockInputClock(OutputBlocks_InputNo[ind], tmp_OutputClock);
            }
          }
        }
      }
      else
      { // all outputs have the same clock
        tmp_OutputClock = ClockGroups.FindClock4Output(reference_input_group_name, 0);

        if (tmp_OutputClock != NULL)
        {
          // 1.  Check if OutputClock is already set
          if (OutputClocks[0]!=NULL)
          {
            // 2a.  If it is set then check if it is correct
            if (OutputClocks[0]!=tmp_OutputClock)
            {
              #ifdef __DEBUG__
                DSP::log << DSP::LogMode::Error << "SetBlockInputClock" << DSP::LogMode::second
                  << "Input clock differs from output clock for (" << GetName()
                  << ") probably at input (" << InputNo << ") \n"
                     " >> Master clock index: " << tmp_OutputClock->MasterClockIndex
                  << "; Output clock cycle_length based on input clock: " << tmp_OutputClock->cycle_length << "\n"
                     " >> Master clock index: " << OutputClocks[0]->MasterClockIndex
                  << "; Actual output clock cycle_length: " << OutputClocks[0]->cycle_length << endl;
              #endif
            }
            else
            { // clock is already set and it is OK
              // Do nothing
            }
          }
          else
          {
            OutputClocks[0] = tmp_OutputClock;

            // If the output Clock is updated Input clock of blocks connected to
            // that output should also be updated.
            for (ind=0; ind<NoOfOutputs; ind++)
            {
              if (OutputBlocks[ind] != &DSP::Block::DummyBlock)
                OutputBlocks[ind]->SetBlockInputClock(OutputBlocks_InputNo[ind], tmp_OutputClock);
            }

          }
        }
      }
    }

  }
}


//return clock assigned to this block's output
//number OutputNo
DSP::Clock_ptr DSP::Component::GetOutputClock(unsigned int OutputNo)
{
  if (NoOfOutputs < 1)
    return NULL;
  if (OutputNo >= NoOfOutputs)
    return NULL;

  if (IsMultiClock)
    return OutputClocks[OutputNo];
  else
    return OutputClocks[0];
}

unsigned int DSP::Component::GetNoOfOutputs(void)
{
  return NoOfOutputs;
}

bool DSP::Block::IsInputAvailable(unsigned int InputNo)
{
  DSP::Component_ptr tempComponent;
  unsigned int tempNo;

  if (InputNo < NoOfInputs)
  {
    if ((IsUsingConstants == true) && (IsConstantInput.empty() == false))
      if (IsConstantInput[InputNo] == true)
      {
        #ifdef __DEBUG__
          DSP::log << DSP::LogMode::Error << "IsInputAvailable" << DSP::LogMode::second
            << "tried to connect to constant input ("
            << this->GetName() << ":" << InputNo << ")" << endl;
        #endif
        return false;
      }

    if (IsOutputConnectedToThisInput2((DSP::Component_ptr)this, InputNo,
                                       tempComponent, tempNo) == false)
      return true;
    else
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "IsInputAvailable" << DSP::LogMode::second
          << "There is output (" << tempComponent->GetName()
          << ":" << tempNo << ") already connected to this input ("
          << this->GetName() << ":" << InputNo << ")" << endl;
      #endif
      return false;
    }
  }
  return false;
}

/*! For DSP::u::Copy component
 * - NULL <== Copy ==> Block2
 *  -# OutputBlock == Block2 : return true & Copy
 *  -# OutputBlock == Copy   : return false
 *  .
 * - Block1 <== Copy ==> Block2  and  Block1 ==> Block2
 *  -# OutputBlock == Block2 : return true & Block1
 *  -# OutputBlock == Copy   : return true & Block1
 *  .
 * - NULL <== Copy ==> NULL
 *  -# OutputBlock == Copy   : return false
 *  .
 * .
 *
 * - OutputBlock == Copy
 *  -# Block1 <== Copy ==> Block2  and  Block1 ==> Block2  : return true & Block1
 *  -# Block1 <== Copy ==> NULL : return true & Block1
 *  -# NULL <== Copy ==> Block2 : return false
 *  -# NULL <== Copy ==> NULL : return false
 *  .
 * - OutputBlock == Block2
 *  -# Block1 ==> Block2 : return true & Block1
 *  -# Block1 <== Copy ==> Block2  and  Block1 ==> Block2  : return true & Block1
 *  -# NULL <== Copy ==> Block2 : return true & Copy
 *  -# NULL ==> Block2 : return false
 *  .
 * .
 *
 */
bool DSP::Component::IsOutputConnectedToThisInput(
              DSP::Component_ptr OutputBlock, unsigned int InputNo,
              DSP::Component_ptr &tempBlock, unsigned int &tempNo)
{
  long int ind;
  unsigned int output_ind;
  DSP::Component_ptr temp;
  DSP::u::Copy_ptr temp_Copy;

//  NoOfBlocksInTable
//  BlocksTable
//
//  DSP::Block_ptr *OutputBlocks; //  one block pointer for one output
//  int *OutputBlocks_InputNo; // Input number of the output block
//  int NoOfOutputs; // number of outputs

  tempBlock=NULL; tempNo=0;
  if (OutputBlock->Convert2Block() != NULL)
    if (OutputBlock->Convert2Block()->IsUsingConstants == true)
      if (OutputBlock->Convert2Block()->IsConstantInput[InputNo] == true)
        return true; // constant input

  temp_Copy = OutputBlock->Convert2Copy();
  if (temp_Copy != NULL)
  { // output block is DSP::u::Copy
    if (temp_Copy->InputBlocks[InputNo] != NULL)
    {
      tempBlock = temp_Copy->InputBlocks[InputNo];
      tempNo = temp_Copy->InputBlocks_OutputNo[InputNo];
      //  -# Block1 <== Copy ==> Block2  and  Block1 ==> Block2  : return true & Block1
      //  -# Block1 <== Copy ==> NULL : return true & Block1
      return true;
    }
    // -# NULL <== Copy ==> Block2 : return false
    // -# NULL <== Copy ==> NULL : return false
    return false;
  }

  temp_Copy = NULL;
  //for each block
  for (ind=0; ind<NoOfComponentsInTable; ind++)
  {
    temp=ComponentsTable[ind];
    // and for each output
    for (output_ind=0; output_ind<temp->NoOfOutputs; output_ind++)
    {
      //check if the output block is the same
      if (temp->OutputBlocks[output_ind]==OutputBlock)
      {
        //and output block input number is also the same
        if (temp->OutputBlocks_InputNo[output_ind]==InputNo)
        {
          temp_Copy = temp->Convert2Copy();

          tempBlock=temp;
          tempNo=output_ind;

          if (temp_Copy == NULL)
          {
            // -# Block1 ==> Block2 : return true & Block1
            // -# Block1 <== Copy ==> Block2  and  Block1 ==> Block2  : return true & Block1
            return true;
          }
        }
      }
    }
  }

  if (temp_Copy != NULL)
  {
    // -# NULL <== Copy ==> Block2 : return true & Copy
    return true;
  }

  // -# NULL ==> Block2 : return false
  return false;
}

/*! Modified version: if only DSP::u::Copy is connected then return false
 * For DSP::u::Copy component
 * - NULL <== Copy ==> Block2
 *  -# OutputBlock == Block2 : return false (WAS: true & Copy)
 *  -# OutputBlock == Copy   : return false
 *  .
 * - Block1 <== Copy ==> Block2  and  Block1 ==> Block2
 *  -# OutputBlock == Block2 : return true & Block1
 *  -# OutputBlock == Copy   : return true & Block1
 *  .
 * - NULL <== Copy ==> NULL
 *  -# OutputBlock == Copy   : return false
 *  .
 * .
 *
 * - OutputBlock == Copy
 *  -# Block1 <== Copy ==> Block2  and  Block1 ==> Block2  : return true & Block1
 *  -# Block1 <== Copy ==> NULL : return true & Block1
 *  -# NULL <== Copy ==> Block2 : return false
 *  -# NULL <== Copy ==> NULL : return false
 *  .
 * - OutputBlock == Block2
 *  -# Block1 ==> Block2 : return true & Block1
 *  -# Block1 <== Copy ==> Block2  and  Block1 ==> Block2  : return true & Block1
 *  -# NULL <== Copy ==> Block2 : return false (WAS: true & Copy)
 *  -# NULL ==> Block2 : return false
 *  .
 * .
 */
bool DSP::Component::IsOutputConnectedToThisInput2(
              DSP::Component_ptr OutputBlock, unsigned int InputNo,
              DSP::Component_ptr &tempBlock, unsigned int &tempNo)
{
  long int ind;
  unsigned int output_ind;
  DSP::Component_ptr temp;
  DSP::u::Copy_ptr temp_Copy;

//  NoOfBlocksInTable
//  BlocksTable
//
//  DSP::Block_ptr *OutputBlocks; //  one block pointer for one output
//  int *OutputBlocks_InputNo; // Input number of the output block
//  int NoOfOutputs; // number of outputs

  tempBlock=NULL; tempNo=0;
  if (OutputBlock->Convert2Block() != NULL)
    if (OutputBlock->Convert2Block()->IsUsingConstants == true)
      if (OutputBlock->Convert2Block()->IsConstantInput[InputNo] == true)
        return true; // constant input

  temp_Copy = OutputBlock->Convert2Copy();
  if (temp_Copy != NULL)
  { // output block is DSP::u::Copy
    if (temp_Copy->GetCopyInput(InputNo, tempBlock, tempNo) == true)
    //if (temp_Copy->InputBlocks[InputNo] != NULL)
    {
      //tempBlock = temp_Copy->InputBlocks[InputNo];
      //tempNo = temp_Copy->InputBlocks_OutputNo[InputNo];
      //  -# Block1 <== OUT.Copy ==> Block2  and  Block1 ==> Block2  : return true & Block1
      //  -# Block1 <== OUT.Copy ==> NULL : return true & Block1
      return true;
    }
    // -# NULL <== OUT.Copy ==> Block2 : return false
    // -# NULL <== OUT.Copy ==> NULL : return false
    return false;
  }

  temp_Copy = NULL;
  //for each block
  for (ind=0; ind<NoOfComponentsInTable; ind++)
  {
    temp=ComponentsTable[ind];
    // and for each output
    for (output_ind=0; output_ind<temp->NoOfOutputs; output_ind++)
    {
      //check if the output block is the same
      if (temp->OutputBlocks[output_ind]==OutputBlock)
      {
        //and output block input number is also the same
        if (temp->OutputBlocks_InputNo[output_ind]==InputNo)
        {
          temp_Copy = temp->Convert2Copy();

          tempBlock=temp;
          tempNo=output_ind;

          if (temp_Copy == NULL)
          {
            // -# Block1 ==> OUT.Block2 : return true & Block1
            // -# Block1 <== Copy ==> OUT.Block2  and  Block1 ==> OUT.Block2  : return true & Block1
            return true;
          }
        }
      }
    }
  }

//  if (temp_Copy != NULL)
//  {
//    // -# NULL <== Copy ==> Block2 : return true & Copy
//    return true;
//  }

  // -# NULL ==> OUT.Block2 : return false
  // -# NULL <== Copy ==> OUT.Block2 : return false & Copy
  return false;
}

// returns true if block is Multirate and fills L and M with proper interpolation and decimation factors
bool DSP::Block::GetMultirateFactorsFromClocks(
    DSP::Clock_ptr InputClock, DSP::Clock_ptr OutputClock,
    long &L, long &M, bool ClocksShouldBeSynchronous)
{
  UNUSED_RELEASE_ARGUMENT(ClocksShouldBeSynchronous);
  //int in_L, in_M, out_L, out_M;

  if (InputClock->MasterClockIndex != OutputClock->MasterClockIndex)
  { // asynchronous clocks
    L = -1;
    M = -1;
    #ifdef __DEBUG__
      if (ClocksShouldBeSynchronous == true)
      {
        DSP::log << DSP::LogMode::Error << "DSP::Block::GetMultirateFactorsFromClocks" << DSP::LogMode::second
          << "Detected asynchronous clock in component >>" << GetName() << "<< !!!" << endl;
      }
    #endif
  }
  else
  {
    /*
    in_L  = InputClock->L;  in_M  = InputClock->M;
    #ifdef __DEBUG__
      if ((in_L == 0) || (in_M == 0))
        DSP::log << DSP::LogMode::Error << "DSP::Block::GetMultirateFactorsFromClocks", "(in_L == 0) || (in_M == 0)");
    #endif
    if (in_L == 0) in_L = 1; if (in_M == 0) in_M = 1; // Input clock might be the master clock
    out_L = OutputClock->L; out_M = OutputClock->M;
    #ifdef __DEBUG__
      if ((out_L == 0) || (out_M == 0))
        DSP::log << DSP::LogMode::Error << "DSP::Block::GetMultirateFactorsFromClocks", "(out_L == 0) || (out_M == 0)");
    #endif
    if (out_L == 0) out_L = 1; if (out_M == 0) out_M = 1; // Output clock might be the master clock

    //L = out_L / in_L;  M = out_M / in_M;
    L = out_L * in_M ; M = out_M * in_L;
    gcd = DSP::f::gcd(L, M);
    L /= gcd; M /= gcd;
    */

    //! \Fixed <b>2012.04.27</b> This must be computed based on ->cycle_length
    unsigned long cycle_length_in, cycle_length_out;
    unsigned long int gcd;
    cycle_length_in = InputClock->cycle_length;
    cycle_length_out = OutputClock->cycle_length;
    gcd = DSP::f::gcd(cycle_length_out, cycle_length_in);
    L = cycle_length_in/gcd;
    M = cycle_length_out/gcd;
  }
  if ((L == 1) && (M == 1))
    return false;
  return true;
}

/**************************************************/
// Delay element implemented in mixed mode (unit-source)
// neccessary for digital feedback loop !!!
#define  THIS  ((DSP::u::LoopDelay *)block)

DSP::u::LoopDelay::LoopDelay(DSP::Clock_ptr ParentClock, unsigned int delay, unsigned int inputs_no)
  : DSP::Block(), DSP::Source()
{
  unsigned int ind;
  string temp;
  vector<unsigned int> indexes;

  SetName("LoopDelay", false);

  if (inputs_no <= 0)
    inputs_no = 1;
  SetNoOfOutputs(inputs_no);
  SetNoOfInputs(inputs_no, false);

  //DefineOutput("out", 0);
  //DefineInput("in", 0);
  DefineInput("in.re", 0);
  DefineOutput("out.re", 0);
  if (NoOfInputs >= 2)
  {
    DefineInput("in.im", 1);
    DefineOutput("out.im", 1);
  }
  indexes.resize(NoOfInputs);
  for (ind=0; ind<NoOfInputs; ind++)
  {
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind);

    indexes[ind] = ind;
  }
  DefineInput("in", indexes);
  DefineOutput("out", indexes);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  RegisterOutputClock(ParentClock);

  if (delay<1)
    delay=1; //in feedback loop can't be zero

  Delay = new unsigned int[NoOfInputs];
  tempInput = new DSP::Float[NoOfInputs];
  IsOutputProcessed = new bool[NoOfInputs];
  IsInputProcessed  = new bool[NoOfInputs];
  State = new DSP::Float_ptr[NoOfInputs];

  for (ind = 0; ind < NoOfInputs; ind++)
  {
    Delay[ind]=delay;
    tempInput[ind]=0.0;
    IsOutputProcessed[ind]=false;
    IsInputProcessed[ind]=false;
    if (Delay[ind] > 0)
    {
      State[ind]=new DSP::Float[Delay[ind]];
      for (unsigned int ind2=0; ind2<Delay[ind]; ind2++)
        State[ind][ind2]=0.0;
    }
    else
      State[ind] = NULL;
  }

  if (NoOfInputs == 1)
  {
    Execute_ptr = &InputExecute;
    OutputExecute_ptr = &OutputExecute;
  }
  else
  {
    Execute_ptr = &InputExecute_multi;
    OutputExecute_ptr = &OutputExecute_multi;
  }
}

bool DSP::u::LoopDelay::SetState(const string &InputName, DSP::Float state_buffer_value)
{
  return SetState(InputName, 1, &state_buffer_value);
}
bool DSP::u::LoopDelay::SetState(const string &InputName, unsigned int size, DSP::Float_ptr state_buffer)
{
  unsigned int InputNo;

  DSP::input &temp_input = Input(InputName);
  if (temp_input.is_null() == false)
    InputNo = temp_input.Inputs[0];
  else
    InputNo = NoOfInputs;

  if (InputNo < NoOfInputs)
  {
    if (size == Delay[InputNo])
    {
      if (State[InputNo] != NULL)
      {
        memcpy(State[InputNo], state_buffer, size*sizeof(DSP::Float));
      }
      else
      {
        #ifdef __DEBUG__
          DSP::log << DSP::LogMode::Error << "DSP::u::LoopDelay::SetState" << DSP::LogMode::second
            << "Block >>" << GetName() << "<<  - state buffer == NULL" << endl;
        #endif
        return false;
      }
    }
    else
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSP::u::LoopDelay::SetState" << DSP::LogMode::second
          << "Block >>" << GetName() << "<<  - wrong state buffer size: " << size  << " instead of " << Delay[InputNo] << endl;
      #endif
      return false;
    }
  }
  else
  {
    if (BlockAllowsForConstantInputs == false)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSP::u::LoopDelay::SetState" << DSP::LogMode::second
          << "Block >>" << GetName() << "<< doesn't support input >>" << InputName << "<< !!!" << endl;
      #endif
      return false;
    }
  }


  return true;
}

DSP::u::LoopDelay::~LoopDelay(void)
{
//  SetNoOfOutputs(0);
  if (State != NULL)
  {
    for (unsigned int ind = 0; ind < NoOfInputs; ind++)
      if (State[ind] != NULL)
      {
        delete [] State[ind];
        State[ind] = NULL;
      }
  }

  delete []State; State = NULL;
  delete []Delay; Delay = NULL;
  delete []tempInput; tempInput = NULL;;
  delete []IsOutputProcessed; IsOutputProcessed = NULL;
  delete []IsInputProcessed; IsInputProcessed = NULL;
}

//Execution as an processing block
void DSP::u::LoopDelay::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (*(THIS->IsOutputProcessed))
  {
    if (*(THIS->Delay) > 1)
      memcpy(*(THIS->State), (*(THIS->State))+1,
             sizeof(DSP::Float) * ((*(THIS->Delay))-1));
    (*(THIS->State))[(*(THIS->Delay))-1]=value;

    //reseting for next cycle
    *(THIS->IsInputProcessed)=false;
    *(THIS->IsOutputProcessed)=false;
  }
  else
  {
    *(THIS->tempInput)=value;
    *(THIS->IsInputProcessed)=true;
  }
}
//Execution as an processing block
void DSP::u::LoopDelay::InputExecute_multi(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (THIS->IsOutputProcessed[InputNo])
  {
    if (THIS->Delay[InputNo] > 1)
      memcpy(THIS->State[InputNo], THIS->State[InputNo]+1,
             sizeof(DSP::Float)*(THIS->Delay[InputNo]-1));
    THIS->State[InputNo][THIS->Delay[InputNo]-1]=value;

    //reseting for next cycle
    THIS->IsInputProcessed[InputNo]=false;
    THIS->IsOutputProcessed[InputNo]=false;
  }
  else
  {
    THIS->tempInput[InputNo]=value;
    THIS->IsInputProcessed[InputNo]=true;
  }
}
#undef THIS

#define  THIS  ((DSP::u::LoopDelay *)source)
//Execution as a source block
bool DSP::u::LoopDelay::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  *(THIS->IsOutputProcessed) = true; //This must be here because
    //The line below in case of digital loop may cause execution
    //of this block's Input processing function
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      *(THIS->State)[0], source);
  if (*(THIS->IsInputProcessed))
  {
    if (*(THIS->Delay) == 1)
    {
      *(THIS->State)[0] = *(THIS->tempInput);
    }
    else
    {
      memcpy(*(THIS->State), *(THIS->State) + 1,
             sizeof(DSP::Float) * (*(THIS->Delay) - 1));
      *(THIS->State)[*(THIS->Delay) - 1] = *(THIS->tempInput);
    }

    //reseting for next cycle
    *(THIS->IsInputProcessed) = false;
    *(THIS->IsOutputProcessed) = false;
  }

  return true;
}
bool DSP::u::LoopDelay::OutputExecute_multi(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  for (unsigned int ind = 0; ind < THIS->NoOfOutputs; ind++)
  {
    THIS->IsOutputProcessed[ind] = true; //This must be here because
      //The line below in case of digital loop may cause execution
      //of this block's Input processing function
    THIS->OutputBlocks[ind]->EXECUTE_PTR(
        THIS->OutputBlocks[ind], THIS->OutputBlocks_InputNo[ind],
        THIS->State[ind][0], source);
    if (THIS->IsInputProcessed[ind])
    {
      if (THIS->Delay[ind] == 1)
      {
        THIS->State[ind][0] = THIS->tempInput[ind];
      }
      else
      {
        memcpy(THIS->State[ind], THIS->State[ind] + 1,
               sizeof(DSP::Float)*(THIS->Delay[ind] - 1));
        THIS->State[ind][THIS->Delay[ind] - 1] = THIS->tempInput[ind];
      }

      //reseting for next cycle
      THIS->IsInputProcessed[ind] = false;
      THIS->IsOutputProcessed[ind] = false;
    }
  }

  return true;
}
#undef THIS

// Delay element implemented in processing mode
// cannot separate processing in digital feedback loop !!!
DSP::u::Delay::Delay(unsigned int delay_in, unsigned int InputsNo, bool IsBufferCyclic) : DSP::Block()
{
  unsigned int ind;
  string temp;
  vector<unsigned int> indexes;

  if (IsBufferCyclic == true)
    SetName("Delay(cyclic)", false);
  else
    SetName("Delay", false);

  if (InputsNo <= 0)
    InputsNo = 1;
  SetNoOfOutputs(InputsNo);
  SetNoOfInputs(InputsNo, false);

  //DefineOutput("out", 0);
  //DefineInput("in", 0);
  DefineInput("in.re", 0);
  DefineOutput("out.re", 0);
  if (NoOfInputs >= 2)
  {
    DefineInput("in.im", 1);
    DefineOutput("out.im", 1);
  }
  indexes.resize(NoOfInputs);
  for (ind=0; ind<NoOfInputs; ind++)
  {
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind);

    indexes[ind] = ind;
  }
  DefineInput("in", indexes);
  DefineOutput("out", indexes);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  //if (delay<0)  delay=0;

  delay=delay_in;
  if (delay > 0)
  {
    State = new DSP::Float_ptr[NoOfInputs];
    index = new unsigned int[NoOfInputs];
    for (ind = 0; ind < NoOfInputs; ind++)
    {
      State[ind]=new DSP::Float[delay];
      memset(State[ind], 0, sizeof(DSP::Float) * delay);
      index[ind]=0;
    }
  }
  else
  {
    State = NULL;
    index = NULL;
  }

  if (InputsNo == 1)
  {
    Execute_ptr = &InputExecute;
    if (IsBufferCyclic == true)
      Execute_ptr = &InputExecute_with_cyclic_buffer;
    if (delay == 0)
      Execute_ptr = &InputExecute_D0;
    if (delay == 1)
      Execute_ptr = &InputExecute_D1;
  }
  else
  {
    Execute_ptr = &InputExecute_multi;
    if (IsBufferCyclic == true)
      Execute_ptr = &InputExecute_with_cyclic_buffer_multi;
    if (delay == 0)
      Execute_ptr = &InputExecute_D0_multi;
    if (delay == 1)
      Execute_ptr = &InputExecute_D1_multi;
  }
}

DSP::u::Delay::~Delay(void)
{
  unsigned int ind;

//  SetNoOfOutputs(0);
  if (State != NULL)
  {
    for (ind = 0; ind < NoOfInputs; ind++)
    {
      if (State[ind] != NULL)
      {
        delete [] State[ind];
        State[ind] = NULL;
      }
    }
    delete [] State;
    State = NULL;
  }
  if (index != NULL)
  {
    delete [] index;
    index = NULL;
  }
}

#define  THIS  ((DSP::u::Delay *)block)
void DSP::u::Delay::InputExecute_D0(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  // if (((DSP::u::Delay *)block)->Delay == 0)
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      value, block);
};

void DSP::u::Delay::InputExecute_D0_multi(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  // if (((DSP::u::Delay *)block)->Delay == 0)
  THIS->OutputBlocks[InputNo]->EXECUTE_PTR(
      THIS->OutputBlocks[InputNo], THIS->OutputBlocks_InputNo[InputNo],
      value, block);
};

void DSP::u::Delay::InputExecute_D1(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  // if (THIS == 1)
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      *(THIS->State[0]), block);
  *(THIS->State[0])=value;
};

void DSP::u::Delay::InputExecute_D1_multi(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  // if (THIS == 1)
  THIS->OutputBlocks[InputNo]->EXECUTE_PTR(
      THIS->OutputBlocks[InputNo], THIS->OutputBlocks_InputNo[InputNo],
      *(THIS->State[InputNo]), block);
  *(THIS->State[InputNo])=value;
};

void DSP::u::Delay::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State[0], this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      THIS->State[0][0], block);
  memcpy(THIS->State[0], THIS->State[0]+1, sizeof(DSP::Float)*(THIS->delay-1));
  THIS->State[0][THIS->delay-1]=value;
};

void DSP::u::Delay::InputExecute_multi(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State[0], this);
  THIS->OutputBlocks[InputNo]->EXECUTE_PTR(
      THIS->OutputBlocks[InputNo], THIS->OutputBlocks_InputNo[InputNo],
      THIS->State[InputNo][0], block);
  memcpy(THIS->State[InputNo], THIS->State[InputNo]+1, sizeof(DSP::Float)*(THIS->delay-1));
  THIS->State[InputNo][THIS->delay-1]=value;
};

void DSP::u::Delay::InputExecute_with_cyclic_buffer(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State[index], this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      THIS->State[0][THIS->index[0]], block);
  THIS->State[0][THIS->index[0]]=value;

  THIS->index[0]++;
  THIS->index[0] %= THIS->delay;
};

void DSP::u::Delay::InputExecute_with_cyclic_buffer_multi(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State[index], this);
  THIS->OutputBlocks[InputNo]->EXECUTE_PTR(
      THIS->OutputBlocks[InputNo], THIS->OutputBlocks_InputNo[InputNo],
      THIS->State[InputNo][THIS->index[InputNo]], block);
  THIS->State[InputNo][THIS->index[InputNo]]=value;

  THIS->index[InputNo]++;
  THIS->index[InputNo] %= THIS->delay;
}
#undef THIS

/**************************************************/
// addition node
DSP::u::Addition::Addition(unsigned int NoOfRealInputs_in, unsigned int NoOfComplexInputs_in)
  : DSP::Block()
{
  Init(NoOfRealInputs_in, NoOfComplexInputs_in, false);

  Execute_ptr = &InputExecute;
}

DSP::u::Addition::Addition(unsigned int NoOfRealInputs_in, DSP::Float_ptr weights)
  : DSP::Block()
{
  Init(NoOfRealInputs_in, 0, false);

  RealWeights = new DSP::Float[NoOfRealInputs_in];
  memcpy(RealWeights, weights, NoOfRealInputs_in*sizeof(DSP::Float));

  Execute_ptr = &InputExecute_RealWeights;
}

DSP::u::Addition::Addition(unsigned int NoOfRealInputs_in, DSP::Complex_ptr weights)
  : DSP::Block()
{
  unsigned int ind;

  Init(NoOfRealInputs_in, 0, true);

  CplxWeights = new DSP::Complex[NoOfRealInputs_in];
  //memcpy(CplxWeights, weights, NoOfRealInputs_in*sizeof(DSP::Complex));
  for (ind = 0; ind < NoOfRealInputs_in; ind++)
    CplxWeights[ind] = weights[ind];

  Execute_ptr = &InputExecute_CplxWeights;
}

DSP::u::Addition::Addition(unsigned int NoOfRealInputs_in, unsigned int NoOfComplexInputs_in, DSP::Float_ptr weights)
  : DSP::Block()
{
  unsigned int ind;

  Init(NoOfRealInputs_in, NoOfComplexInputs_in, false);

  RealWeights = new DSP::Float[NoOfRealInputs_in+NoOfComplexInputs_in*2];
  for (ind = 0; ind < NoOfRealInputs_in; ind++)
    RealWeights[ind] = weights[ind];
  for (ind = 0; ind < NoOfComplexInputs_in; ind++)
  {
    // duplicating coefficients for complex inputs
    // in order to avoid input line to input number
    // convertions during execution
    RealWeights[NoOfRealInputs_in+ind*2] = weights[NoOfRealInputs_in+ind];
    RealWeights[NoOfRealInputs_in+ind*2+1] = weights[NoOfRealInputs_in+ind];
  }

  Execute_ptr = &InputExecute_RealWeights;
}

DSP::u::Addition::Addition(unsigned int NoOfRealInputs_in, unsigned int NoOfComplexInputs_in, DSP::Complex_ptr weights)
  : DSP::Block()
{
  unsigned int ind;

  Init(NoOfRealInputs_in, NoOfComplexInputs_in, true);

  CplxWeights = new DSP::Complex[NoOfRealInputs_in+NoOfComplexInputs_in];
  //memcpy(CplxWeights, weights, (NoOfRealInputs_in+NoOfComplexInputs_in)*sizeof(DSP::Complex));
  for (ind = 0; ind < NoOfRealInputs_in; ind++)
    CplxWeights[ind] = weights[ind];
  for (ind = 0; ind < NoOfComplexInputs_in; ind++)
  {
    // duplicating coefficients for complex inputs
    // in order to avoid input line to input number
    // convertions during execution
    CplxWeights[NoOfRealInputs_in+ind*2] = weights[NoOfRealInputs_in+ind];
    CplxWeights[NoOfRealInputs_in+ind*2+1] = weights[NoOfRealInputs_in+ind];
  }

  Execute_ptr = &InputExecute_CplxWeights;
}

DSP::u::Addition::~Addition(void)
{
//  SetNoOfOutputs(0);
  if (RealWeights != NULL)
  {
    delete [] RealWeights;
    RealWeights = NULL;
  }
  if (CplxWeights != NULL)
  {
    delete [] CplxWeights;
    CplxWeights = NULL;
  }
}

// Standard initialisation
void DSP::u::Addition::Init(unsigned int NoOfRealInputs_in, unsigned int NoOfComplexInputs_in, bool ForceCplxOutput)
{
  string temp;
  unsigned int ind;

  SetName("Addition", false);
  if ((NoOfComplexInputs_in > 0) | (ForceCplxOutput == true))
  // there is at least one complex input
  {
    SetNoOfOutputs(2); //complex valued output
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    SetNoOfOutputs(1);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }

  SetNoOfInputs(NoOfRealInputs_in, NoOfComplexInputs_in, true);
  for (ind=0; ind<NoOfRealInputs_in; ind++)
  {
    temp = "real_in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "in" + to_string(ind+1) + ".re";
    DefineInput(temp, ind);
  }
  for (ind=0; ind<NoOfComplexInputs_in; ind++)
  {
    temp = "cplx_in" + to_string(ind+1);
    DefineInput(temp, NoOfRealInputs_in+ind*2, NoOfRealInputs_in+ind*2+1);
    temp = "in" + to_string(NoOfRealInputs_in+ind+1);
    DefineInput(temp, NoOfRealInputs_in+ind*2, NoOfRealInputs_in+ind*2+1);

    temp = "cplx_in" + to_string(ind+1) + ".re";
    DefineInput(temp, NoOfRealInputs_in+ind*2);
    temp = "cplx_in" + to_string(ind+1) + ".im";
    DefineInput(temp, NoOfRealInputs_in+ind*2+1);
    temp = "in" + to_string(NoOfRealInputs_in+ind+1) + ".re";
    DefineInput(temp, NoOfRealInputs_in+ind*2);
    temp = "in" + to_string(NoOfRealInputs_in+ind+1) + ".im";
    DefineInput(temp, NoOfRealInputs_in+ind*2+1);
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  InitialSum_real = 0.0;  InitialSum_imag = 0.0;

  State_real=InitialSum_real;
  State_imag=InitialSum_imag;
  NoOfInputsProcessed=InitialNoOfInputsProcessed;

  RealWeights = NULL;
  CplxWeights = NULL;
}

#define THIS ((DSP::u::Addition *)block)
void DSP::u::Addition::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  //In general we should check whether each input
  //is executed only once per cycle
  if (InputNo < THIS->NoOfRealInputs)
    THIS->State_real += value;
  else
  {
    if ((InputNo % 2) == THIS->IsComplex_real_odd)
      THIS->State_real += value;
    else
      THIS->State_imag += value;
  }
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    //we assume one output
//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State_real, this);
    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
        THIS->State_real, block);
    THIS->State_real=THIS->InitialSum_real;
    if (THIS->NoOfOutputs == 2)
    {
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], State_imag, this);
      THIS->OutputBlocks[1]->EXECUTE_PTR(
          THIS->OutputBlocks[1], THIS->OutputBlocks_InputNo[1],
          THIS->State_imag, block);
      THIS->State_imag=THIS->InitialSum_imag;
    }
    THIS->NoOfInputsProcessed=THIS->InitialNoOfInputsProcessed;
  }
};

void DSP::u::Addition::InputExecute_RealWeights(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  //In general we should check whether each input
  //is executed only once per cycle
  if (InputNo < THIS->NoOfRealInputs)
    THIS->State_real += value * THIS->RealWeights[InputNo];
  else
  {
    if ((InputNo % 2) == THIS->IsComplex_real_odd)
      THIS->State_real += value * THIS->RealWeights[InputNo];
    else
      THIS->State_imag += value * THIS->RealWeights[InputNo];
  }
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    //we assume one output
//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State_real, this);
    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0],
        THIS->OutputBlocks_InputNo[0],
        THIS->State_real, block);
    THIS->State_real=THIS->InitialSum_real;
    if (THIS->NoOfOutputs == 2)
    {
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], State_imag, this);
      THIS->OutputBlocks[1]->EXECUTE_PTR(
          THIS->OutputBlocks[1],
          THIS->OutputBlocks_InputNo[1],
          THIS->State_imag, block);
      THIS->State_imag=THIS->InitialSum_imag;
    }
    THIS->NoOfInputsProcessed=THIS->InitialNoOfInputsProcessed;
  }
}


void DSP::u::Addition::InputExecute_CplxWeights(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  //In general we should check whether each input
  //is executed only once per cycle
  if (InputNo < THIS->NoOfRealInputs)
  {
    THIS->State_real += value * THIS->CplxWeights[InputNo].re;
    THIS->State_imag += value * THIS->CplxWeights[InputNo].im;
  }
  else
  {
    if ((InputNo % 2) == THIS->IsComplex_real_odd)
    {  // a * (c+jd) = ac + j*ad
      THIS->State_real += value * THIS->CplxWeights[InputNo].re;
      THIS->State_imag += value * THIS->CplxWeights[InputNo].im;
    }
    else
    {  // j*b * (c+jd) = j*bc - bd
      THIS->State_imag += value * THIS->CplxWeights[InputNo].re;
      THIS->State_real -= value * THIS->CplxWeights[InputNo].im;
    }
  }
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    //we assume one output
//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State_real, this);
    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0],
        THIS->OutputBlocks_InputNo[0],
        THIS->State_real, block);
    THIS->State_real=THIS->InitialSum_real;
    if (THIS->NoOfOutputs == 2)
    {
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], State_imag, this);
      THIS->OutputBlocks[1]->EXECUTE_PTR(
          THIS->OutputBlocks[1],
          THIS->OutputBlocks_InputNo[1],
          THIS->State_imag, block);
      THIS->State_imag=THIS->InitialSum_imag;
    }
    THIS->NoOfInputsProcessed=THIS->InitialNoOfInputsProcessed;
  }
}
#undef THIS


void DSP::u::Addition::RecalculateInitials(void)
{
  unsigned int ind;

  InitialSum_real = 0.0;
  InitialSum_imag = 0.0;

  if ((RealWeights == NULL) && (CplxWeights == NULL))
    for (ind=0; ind<NoOfInputs; ind++)
    {
      if (IsConstantInput[ind] == true)
      {
        if (ind < NoOfRealInputs)
          InitialSum_real += ConstantInputValues[ind];
        else
        {
          if ((ind % 2) == IsComplex_real_odd)
            InitialSum_real += ConstantInputValues[ind];
          else
            InitialSum_imag += ConstantInputValues[ind];
        }
      }
    }
  else
  {
    if (RealWeights != NULL)
      for (ind=0; ind<NoOfInputs; ind++)
      {
        if (IsConstantInput[ind] == true)
        {
          if (ind < NoOfRealInputs)
            InitialSum_real += (ConstantInputValues[ind] * RealWeights[ind]);
          else
          {
            if ((ind % 2) == IsComplex_real_odd)
              InitialSum_real += (ConstantInputValues[ind] * RealWeights[ind]);
            else
              InitialSum_imag += (ConstantInputValues[ind] * RealWeights[ind]);
          }
        }
      }

    if (CplxWeights != NULL)
      for (ind=0; ind<NoOfInputs; ind++)
      {
        if (IsConstantInput[ind] == true)
        {
          if (ind < NoOfRealInputs)
          {
            InitialSum_real += (ConstantInputValues[ind] * CplxWeights[ind].re);
            InitialSum_imag += (ConstantInputValues[ind] * CplxWeights[ind].im);
          }
          else
          {
            if ((ind % 2) == IsComplex_real_odd)
            { // a * (c+j*d) = ac + j*ad
              InitialSum_real += (ConstantInputValues[ind] * CplxWeights[ind].re);
              InitialSum_imag += (ConstantInputValues[ind] * CplxWeights[ind].im);
            }
            else
            { // jb * (c+j*d) = jbc - bd
              InitialSum_imag += (ConstantInputValues[ind] * CplxWeights[ind].re);
              InitialSum_real -= (ConstantInputValues[ind] * CplxWeights[ind].im);
            }
          }
        }
      }

  }

  // set inner state so first cycle is OK
  State_real=InitialSum_real;
  State_imag=InitialSum_imag;
}

/**************************************************/
// multiplication node
DSP::u::Multiplication::Multiplication(unsigned int NoOfRealInputs_in, unsigned int NoOfComplexInputs_in)
  : DSP::Block()
{
  string temp;
  unsigned int ind;

  SetName("Multiplication", false);
  if (NoOfComplexInputs_in > 0)
  {
    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    SetNoOfOutputs(1);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }

  SetNoOfInputs(NoOfRealInputs_in, NoOfComplexInputs_in, true);
  for (ind=0; ind<NoOfRealInputs_in; ind++)
  {
    temp = "real_in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "in" + to_string(ind+1) + ".re";
    DefineInput(temp, ind);
  }
  for (ind=0; ind<NoOfComplexInputs_in; ind++)
  {
    temp = "cplx_in" + to_string(ind+1);
    DefineInput(temp, NoOfRealInputs_in+ind*2, NoOfRealInputs_in+ind*2+1);

    temp = "in" + to_string(NoOfRealInputs_in+ind+1);
    DefineInput(temp, NoOfRealInputs_in+ind*2, NoOfRealInputs_in+ind*2+1);


    temp = "cplx_in" + to_string(ind+1) + ".re";
    DefineInput(temp, NoOfRealInputs_in+ind*2);
    temp = "cplx_in" + to_string(ind+1)+ ".im";
    DefineInput(temp, NoOfRealInputs_in+ind*2+1);
    temp = "in" + to_string(NoOfRealInputs_in+ind+1) + ".re";
    DefineInput(temp, NoOfRealInputs_in+ind*2);
    temp = "in" + to_string(NoOfRealInputs_in+ind+1) + ".im";
    DefineInput(temp, NoOfRealInputs_in+ind*2+1);
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

//  State=1.0;
  State_Re = 0.0; State_Im = 0.0;

  State = new DSP::Float[NoOfInputs];

  Execute_ptr = &InputExecute;
}

DSP::u::Multiplication::~Multiplication(void)
{
//  SetNoOfOutputs(0);
  if (State != NULL)
  {
    delete [] State;
    State = NULL;
  }
}

void DSP::u::Multiplication::RecalculateInitials(void)
{
  unsigned int ind;

  for (ind=0; ind<NoOfInputs; ind++)
  {
    if (IsConstantInput[ind] == true)
    {
      State[ind] = ConstantInputValues[ind];
    }
  }
}

void DSP::u::Multiplication::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  unsigned int current_ind;
  DSP::Float temp_re;

  //In general we should check whether each input
  //is executed only once per cycle
  ((DSP::u::Multiplication *)block)->State[InputNo]=value;
  ((DSP::u::Multiplication *)block)->NoOfInputsProcessed++;

  if (((DSP::u::Multiplication *)block)->NoOfInputsProcessed == ((DSP::u::Multiplication *)block)->NoOfInputs)
  {
    ((DSP::u::Multiplication *)block)->State_Im=0.0;
    current_ind=0;
    if (((DSP::u::Multiplication *)block)->NoOfRealInputs>0)
    {
      ((DSP::u::Multiplication *)block)->State_Re=((DSP::u::Multiplication *)block)->State[current_ind];
      current_ind++;
      while (current_ind<((DSP::u::Multiplication *)block)->NoOfRealInputs)
      {
        ((DSP::u::Multiplication *)block)->State_Re*=((DSP::u::Multiplication *)block)->State[current_ind];
        current_ind++;
      }
    }
    else
    {
      ((DSP::u::Multiplication *)block)->State_Re=1.0;
    }
    if (((DSP::u::Multiplication *)block)->NoOfRealInputs<((DSP::u::Multiplication *)block)->NoOfInputs)
    {
      ((DSP::u::Multiplication *)block)->State_Im =
          ((DSP::u::Multiplication *)block)->State_Re * ((DSP::u::Multiplication *)block)->State[current_ind+1];
      ((DSP::u::Multiplication *)block)->State_Re *=
          ((DSP::u::Multiplication *)block)->State[current_ind];
      current_ind+=2;
      while (current_ind<((DSP::u::Multiplication *)block)->NoOfInputs)
      {
        temp_re = ((DSP::u::Multiplication *)block)->State_Re *
                    ((DSP::u::Multiplication *)block)->State[current_ind] -
                  ((DSP::u::Multiplication *)block)->State_Im *
                    ((DSP::u::Multiplication *)block)->State[current_ind+1];
        ((DSP::u::Multiplication *)block)->State_Im =
                  ((DSP::u::Multiplication *)block)->State_Re *
                    ((DSP::u::Multiplication *)block)->State[current_ind+1] +
                  ((DSP::u::Multiplication *)block)->State_Im *
                    ((DSP::u::Multiplication *)block)->State[current_ind];
        ((DSP::u::Multiplication *)block)->State_Re = temp_re;
        current_ind+=2;
      }
    }


    //we assume one output
//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State_Re, this);
    ((DSP::u::Multiplication *)block)->OutputBlocks[0]->EXECUTE_PTR(
        ((DSP::u::Multiplication *)block)->OutputBlocks[0],
        ((DSP::u::Multiplication *)block)->OutputBlocks_InputNo[0],
        ((DSP::u::Multiplication *)block)->State_Re, block);
    if (((DSP::u::Multiplication *)block)->NoOfOutputs==2)
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], State_Im, this);
      ((DSP::u::Multiplication *)block)->OutputBlocks[1]->EXECUTE_PTR(
          ((DSP::u::Multiplication *)block)->OutputBlocks[1],
          ((DSP::u::Multiplication *)block)->OutputBlocks_InputNo[1],
          ((DSP::u::Multiplication *)block)->State_Im, block);
    ((DSP::u::Multiplication *)block)->NoOfInputsProcessed =
        ((DSP::u::Multiplication *)block)->InitialNoOfInputsProcessed;
  }
};

/**************************************************/
// Real multiplication node
DSP::u::RealMultiplication::RealMultiplication(unsigned int NoOfRealInputs_in)
  : DSP::Block()
{
  string temp;
  unsigned int ind;

  SetName("Real multiplication", false);
  SetNoOfOutputs(1);
  DefineOutput("out", 0);

  SetNoOfInputs(NoOfRealInputs_in, 0, true);
  for (ind=0; ind<NoOfRealInputs_in; ind++)
  {
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "real_in" + to_string(ind+1);
    DefineInput(temp, ind);
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  RealInitialValue = 1.0;
  State = RealInitialValue;

  Execute_ptr = &InputExecute;
}

DSP::u::RealMultiplication::~RealMultiplication(void)
{
//  SetNoOfOutputs(0);
}

void DSP::u::RealMultiplication::RecalculateInitials(void)
{
  unsigned int ind;

  RealInitialValue = 1.0;

  for (ind=0; ind<NoOfInputs; ind++)
  {
    if (IsConstantInput[ind] == true)
    {
      RealInitialValue *= ConstantInputValues[ind];
    }
  }

  // set inner state so the first cycle is OK
  State = RealInitialValue;
}

#define  THIS  ((DSP::u::RealMultiplication *)block)
void DSP::u::RealMultiplication::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->State*=value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    //we assume one output
//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State, this);
    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
        THIS->State, block);
    THIS->NoOfInputsProcessed=0; // no constant inputs //InitialNoOfInputsProcessed;
    THIS->State = THIS->RealInitialValue;
  }
};
#undef THIS

/**************************************************/
// Multiplies input value by constant
DSP::u::Amplifier::Amplifier(DSP::Float alfa, unsigned int NoOfInputs_in, bool AreInputsComplex)
  : DSP::Block()
{
  vector <unsigned int> indexes;
  //unsigned int ind;
  string temp;

  IsGainComplex = false;
  SetName("Amplifier", false);

  if (AreInputsComplex == true)
  {
    SetNoOfOutputs(2*NoOfInputs_in);
    SetNoOfInputs(2*NoOfInputs_in, false);

    if (NoOfInputs_in == 1)
    {
      DefineInput("in", 0, 1);
      DefineInput("in.re", 0);
      DefineInput("in.im", 0);
      DefineOutput("out", 0, 1);
      DefineOutput("out.re", 0);
      DefineOutput("out.im", 0);
    }
    for (ind=0; ind<NoOfInputs_in; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, 2*ind, 2*ind+1);
      temp = "in" + to_string(ind+1) + ".re";
      DefineInput(temp, 2*ind);
      temp = "in" + to_string(ind+1) + ".im";
      DefineInput(temp, 2*ind+1);
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, 2*ind, 2*ind+1);
      temp = "out" + to_string(ind+1) + ".re";
      DefineOutput(temp, 2*ind);
      temp = "out" + to_string(ind+1) + ".im";
      DefineOutput(temp, 2*ind+1);
    }
  }
  else
  {
    SetNoOfOutputs(NoOfInputs_in);
    SetNoOfInputs(NoOfInputs_in, false);

    if (NoOfInputs == 1)
    {
      DefineInput("in", 0);
      DefineInput("in.re", 0);
      DefineOutput("out", 0);
      DefineOutput("out.re", 0);
    }
    for (ind=0; ind<NoOfInputs_in; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, ind);
      temp = "in" + to_string(ind+1) + ".re";
      DefineInput(temp, ind);
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, ind);
      temp = "out" + to_string(ind+1) + ".re";
      DefineOutput(temp, ind);
    }
  }
  indexes.resize(NoOfInputs);
  for (ind=0; ind<NoOfInputs; ind++)
    indexes[ind] = ind;
  DefineInput("in_all", indexes);

  indexes.resize(NoOfOutputs);
  for (ind=0; ind<NoOfOutputs; ind++)
    indexes[ind] = ind;
  DefineOutput("out_all", indexes);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  Coeficient=alfa;

  temp_inputs = NULL;
  Execute_ptr = &InputExecute_real_factor;
  if (NoOfInputs == 1)
    Execute_ptr = &InputExecute_one_real_input_with_real_factor;
};

  DSP::u::Amplifier::Amplifier(DSP::Complex alfa, unsigned int NoOfInputs_in, bool AreInputsComplex)
    : DSP::Block()
  {
    vector <unsigned int> indexes;
    //unsigned int ind;
    string temp;

    SetName("Amplifier", false);

    IsGainComplex = true;
    if (AreInputsComplex == false)
    {
      SetNoOfOutputs(2*NoOfInputs_in); // complex outputs
      SetNoOfInputs(NoOfInputs_in, false); // real inputs

      DefineInput("in", 0);
      DefineInput("in.re", 0);
      DefineOutput("out", 0, 1);
      DefineOutput("out.re", 0);
      DefineOutput("out.im", 1);
      for (ind=0; ind<NoOfInputs_in; ind++)
      {
        temp = "in" + to_string(ind+1);
        DefineInput(temp, ind);
        temp = "in" + to_string(ind+1) + ".re";
        DefineInput(temp, ind);

        temp = "out" + to_string(ind+1);
        DefineOutput(temp, 2*ind, 2*ind+1);
        temp = "out" + to_string(ind+1) + ".re";
        DefineOutput(temp, 2*ind);
        temp = "out" + to_string(ind+1) + ".im";
        DefineOutput(temp, 2*ind+1);
      }
    }
    else
    {
      SetNoOfOutputs(2*NoOfInputs_in); // complex outputs
      SetNoOfInputs(2*NoOfInputs_in, false); // complex inputs

      DefineInput("in", 0, 1);
      DefineInput("in.re", 0);
      DefineInput("in.im", 1);
      DefineOutput("out", 0, 1);
      DefineOutput("out.re", 0);
      DefineOutput("out.im", 1);
      for (ind=0; ind<NoOfInputs_in; ind++)
      {
        temp = "in" + to_string(ind+1);
        DefineInput(temp, 2*ind, 2*ind+1);
        temp = "in" + to_string(ind+1) + ".re";
        DefineInput(temp, 2*ind);
        temp = "in" + to_string(ind+1) + ".im";
        DefineInput(temp, 2*ind+1);

        temp = "out" + to_string(ind+1);
        DefineOutput(temp, 2*ind, 2*ind+1);
        temp = "out" + to_string(ind+1) + ".re";
        DefineOutput(temp, 2*ind);
        temp = "out" + to_string(ind+1) + ".im";
        DefineOutput(temp, 2*ind+1);
      }
    }
    indexes.resize(NoOfInputs);
    for (ind=0; ind<NoOfInputs; ind++)
      indexes[ind] = ind;
    DefineInput("in_all", indexes);

    indexes.resize(NoOfOutputs);
    for (ind=0; ind<NoOfOutputs; ind++)
      indexes[ind] = ind;
    DefineOutput("out_all", indexes);

    ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
    ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

    Coeficient=0; // unused for complex coefficient
    CplxCoeficient=alfa;

    temp_inputs = NULL;
    if (AreInputsComplex == true)
    {
      temp_inputs = new DSP::Float[NoOfInputs];
      Execute_ptr = &InputExecute_cplx_inputs_with_cplx_factor;
    }
    else
      Execute_ptr = &InputExecute_real_inputs_with_cplx_factor;
  };

DSP::u::Amplifier::~Amplifier(void)
{
//  SetNoOfOutputs(0);
  if (temp_inputs != NULL)
  {
    delete [] temp_inputs;
    temp_inputs = NULL;
  }
};

// changes amplification factor
void DSP::u::Amplifier::SetGain(DSP::Float gain)
{
  if (IsGainComplex == true)
    CplxCoeficient=gain;
  else
    Coeficient=gain;
}

// changes amplification factor
void DSP::u::Amplifier::SetGain(DSP::Complex gain)
{
  if (IsGainComplex == true)
    CplxCoeficient=gain;
  else
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::u::Amplifier::SetGain" << DSP::LogMode::second << "Attemt to set complex gain factor for block with real gain factor" << endl;
    #endif
    Coeficient=gain.re;
  }
}

#define  THIS  ((DSP::u::Amplifier *)block)
void DSP::u::Amplifier::InputExecute_one_real_input_with_real_factor(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0],
//                           Coeficient*value, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
        THIS->Coeficient*value, block);
};

void DSP::u::Amplifier::InputExecute_real_factor(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->OutputBlocks[InputNo]->EXECUTE_PTR(
        THIS->OutputBlocks[InputNo], THIS->OutputBlocks_InputNo[InputNo],
        THIS->Coeficient*value, block);
};

void DSP::u::Amplifier::InputExecute_real_inputs_with_cplx_factor(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->OutputBlocks[2*InputNo]->EXECUTE_PTR(
        THIS->OutputBlocks[2*InputNo], THIS->OutputBlocks_InputNo[2*InputNo],
        THIS->CplxCoeficient.re*value, block);
  THIS->OutputBlocks[2*InputNo+1]->EXECUTE_PTR(
        THIS->OutputBlocks[2*InputNo+1], THIS->OutputBlocks_InputNo[2*InputNo+1],
        THIS->CplxCoeficient.im*value, block);
};

void DSP::u::Amplifier::InputExecute_cplx_inputs_with_cplx_factor(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->temp_inputs[InputNo] = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  for (THIS->ind = 0; THIS->ind <  (THIS->NoOfInputs >> 1); THIS->ind++)
  {
    // (a+jb)*(c+jd) = ac-bd + j(ad+bc)
    THIS->OutputBlocks[2*THIS->ind]->EXECUTE_PTR(
          THIS->OutputBlocks[2*THIS->ind], THIS->OutputBlocks_InputNo[2*THIS->ind],
          THIS->CplxCoeficient.re*THIS->temp_inputs[2*THIS->ind] -
          THIS->CplxCoeficient.im*THIS->temp_inputs[2*THIS->ind+1], block);
    THIS->OutputBlocks[2*THIS->ind+1]->EXECUTE_PTR(
          THIS->OutputBlocks[2*THIS->ind+1], THIS->OutputBlocks_InputNo[2*THIS->ind+1],
          THIS->CplxCoeficient.re*THIS->temp_inputs[2*THIS->ind+1] +
          THIS->CplxCoeficient.im*THIS->temp_inputs[2*THIS->ind], block);
  }
};
#undef THIS


/**************************************************/
// Calculates giver power of the input signal
// - real input
DSP::u::Power::Power(int factor)
  : DSP::Block()
{
  SetName("Power", false);

  SetNoOfOutputs(1);
  SetNoOfInputs(1, false);

  DefineInput("in", 0);
  DefineInput("in.re", 0);
  DefineOutput("out", 0);
  DefineOutput("out.re", 0);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  ind = 0;

  if (factor == 2)
    Execute_ptr = &InputExecute_Power2_real;
  else
  {
    if (factor >=0)
    {
      IntFactor = factor;
      Execute_ptr = &InputExecute_PowerInt_real;
    }
    else
    {
      RealFactor = (DSP::Float)factor;
      Execute_ptr = &InputExecute_PowerReal_real;
    }
  }
};

DSP::u::Power::Power(bool IsComplex, int factor)
    : DSP::Block()
{
  SetName("Power", false);

  ind = 0;

  if (IsComplex == false)
  {
    SetNoOfOutputs(1);
    SetNoOfInputs(1, false);

    DefineInput("in", 0);
    DefineInput("in.re", 0);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);

    ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
    ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

    if (factor == 2)
      Execute_ptr = &InputExecute_Power2_real;
    else
    {
      if (factor >=0)
      {
        IntFactor = factor;
        Execute_ptr = &InputExecute_PowerInt_real;
      }
      else
      {
        RealFactor = DSP::Float(factor);
        Execute_ptr = &InputExecute_PowerReal_real;
      }
    }
  }
  else
  {
    SetNoOfOutputs(2);
    SetNoOfInputs(2, false);

    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);

    ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
    ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

    if (factor < 0)
    {
      factor = 0;
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSP::u::Power::Power(true,int)" << DSP::LogMode::second << "negative power factor not allowed" << endl;
      #endif
    }

    if (factor == 2)
      Execute_ptr = &InputExecute_Power2_cplx;
    else
    {
      IntFactor = factor;
      Execute_ptr = &InputExecute_PowerInt_cplx;
    }
  }
};

DSP::u::Power::Power(DSP::Float factor)
  : DSP::Block()
{
  SetName("Power", false);

  SetNoOfOutputs(1);
  SetNoOfInputs(1, false);

  DefineInput("in", 0);
  DefineInput("in.re", 0);
  DefineOutput("out", 0);
  DefineOutput("out.re", 0);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  ind = 0;
  IntFactor = 0; // not used in this variant
  RealFactor = factor;
  Execute_ptr = &InputExecute_PowerReal_real;
};

DSP::u::Power::Power(bool IsComplex, DSP::Float factor)
    : DSP::Block()
{
  SetName("Power", false);
  ind = 0;

  if (IsComplex == false)
  {
    SetNoOfOutputs(1);
    SetNoOfInputs(1, false);

    DefineInput("in", 0);
    DefineInput("in.re", 0);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);

    ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
    ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

    RealFactor = factor;
    Execute_ptr = &InputExecute_PowerReal_real;
  }
  else
  {
    SetNoOfOutputs(2);
    SetNoOfInputs(2, false);

    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);

    ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
    ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::u::Power::Power(true,DSP::Float)" << DSP::LogMode::second << "real power factor for complex input is not allowed" << endl;
    #endif

    IntFactor = (int)factor;
    if (IntFactor < 0)
      IntFactor = 0;
    Execute_ptr = &InputExecute_PowerInt_cplx;
  }
};

DSP::u::Power::~Power(void)
{
//  SetNoOfOutputs(0);
};

#define  THIS  ((DSP::u::Power *)block)
void DSP::u::Power::InputExecute_Power2_real(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
        value*value, block);
};

void DSP::u::Power::InputExecute_Power2_cplx(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->in_value.re = value;
  else
    THIS->in_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  // (a+jb)^2 = a^2-b^2 + j2ab
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->in_value.re*THIS->in_value.re - THIS->in_value.im*THIS->in_value.im, block);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      2*THIS->in_value.re*THIS->in_value.im, block);
};

void DSP::u::Power::InputExecute_PowerInt_real(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->out_value.re = 1.0;
  for (THIS->ind = 0; THIS->ind < THIS->IntFactor; THIS->ind++)
    THIS->out_value.re *= value;
  THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
        THIS->out_value.re, block);
};

void DSP::u::Power::InputExecute_PowerInt_cplx(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->in_value.re = value;
  else
    THIS->in_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  THIS->out_value = 1.0;
  for (THIS->ind = 0; THIS->ind < THIS->IntFactor; THIS->ind++)
    THIS->out_value.multiply_by(value);

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->out_value.re, block);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      THIS->out_value.im, block);
};

void DSP::u::Power::InputExecute_PowerReal_real(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
        (DSP::Float)pow(value, THIS->RealFactor), block);
};

#undef THIS

/**************************************************/
// splits one input to several outputs
DSP::u::Splitter::Splitter(unsigned int No) : DSP::Block()
{
  string temp;
  unsigned int ind;

  SetName("Splitter", false);

  SetNoOfOutputs(No);
  SetNoOfInputs(1,false);
  for (ind=0; ind<No; ind++)
  {
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind);
    temp = "out" + to_string(ind+1) + ".re";
    DefineOutput(temp, ind);
  }
  DefineInput("in", 0);
  DefineInput("in.re", 0);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  Execute_ptr = &InputExecute;
}

DSP::u::Splitter::Splitter(bool IsInputComplex, unsigned int No)
  : DSP::Block()
{
  unsigned int ind;
  string temp;

  SetName("Splitter", false);

  if (IsInputComplex == true)
  {
    SetNoOfOutputs(2*No);
    SetNoOfInputs(2,false);
    for (ind=0; ind<No; ind++)
    {
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, 2*ind, 2*ind+1);
      temp = "out" + to_string(ind+1) + ".re";
      DefineOutput(temp, 2*ind);
      temp = "out" + to_string(ind+1) + ".im";
      DefineOutput(temp, 2*ind+1);
    }
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineInput("in", 0, 1);
  }
  else
  {
    SetNoOfOutputs(No);
    SetNoOfInputs(1,false);
    for (ind=0; ind<No; ind++)
    {
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, ind);
      temp = "out" + to_string(ind+1) + ".re";
      DefineOutput(temp, ind);
    }
    DefineInput("in.re", 0);
    DefineInput("in", 0);
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  Execute_ptr = &InputExecute;
}

DSP::u::Splitter::~Splitter(void)
{
//  SetNoOfOutputs(0);
}

void DSP::u::Splitter::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  unsigned int ind;

  //! start from output with the same input the skip by NoOfInputs
  for (ind=InputNo; ind<((DSP::u::Splitter *)block)->NoOfOutputs; ind+=((DSP::u::Splitter *)block)->NoOfInputs)
//    OutputBlocks[ind]->Execute(OutputBlocks_InputNo[ind], value, this);
    ((DSP::u::Splitter *)block)->OutputBlocks[ind]->EXECUTE_PTR(
        ((DSP::u::Splitter *)block)->OutputBlocks[ind],
        ((DSP::u::Splitter *)block)->OutputBlocks_InputNo[ind], value, block);
};


/**************************************************/
// Gets value from selected input and sends it to the selected output
DSP::u::Switch::Switch(unsigned int InputsNo, unsigned int OutputsNo) : DSP::Block()
{
  State = NULL;
  Init(false, InputsNo, OutputsNo); //, UseSelectorInputs);

  Execute_ptr = &InputExecute;
}

DSP::u::Switch::Switch(bool IsInputComplex,
    unsigned int InputsNo, unsigned int OutputsNo) : DSP::Block()
{
  State = NULL;
  Init(IsInputComplex, InputsNo, OutputsNo); //, UseSelectorInputs);

  Execute_ptr = &InputExecute;
}

DSP::u::Switch::~Switch(void)
{
//  SetNoOfOutputs(0);
  if (State != NULL)
    delete [] State;
}

void DSP::u::Switch::Init(bool IsInputComplex,
    unsigned int InputsNo, unsigned int OutputsNo)
{
  unsigned int ind;
  string temp;

  SetName("Switch", false);

  if (IsInputComplex == false)
  {
    ValuesPerOutput =1;
  }
  else
  {
    ValuesPerOutput =2;
  }
  if (OutputsNo<1) OutputsNo=1;
  SetNoOfOutputs(ValuesPerOutput*OutputsNo);
  if (InputsNo<1) InputsNo=1;

  State = new DSP::Float[InputsNo*ValuesPerOutput];

  LastInputInd=ValuesPerOutput*InputsNo-1; //index starts from 0
  InputSelectionInd = DSP::c::FO_NoInput;
  OutputSelectionInd = DSP::c::FO_NoOutput;
  /*
  if (UseSelectorInputs == true)
  {
    if ((InputsNo==1) || (OutputsNo==1))
    {
      SetNoOfInputs(ValuesPerOutput*InputsNo+1,false);
      if (OutputsNo==1)
      {
        InputSelectionInd=ValuesPerOutput*InputsNo;
        DefineOutput("input_selector", ValuesPerOutput*InputsNo);
      }
      else
      {
        OutputSelectionInd=ValuesPerOutput*InputsNo;
        DefineOutput("output_selector", OutputSelectionInd);
      }
    }
    else
    {
      InputSelectionInd=ValuesPerOutput*InputsNo;
      OutputSelectionInd=ValuesPerOutput*InputsNo+1;
      SetNoOfInputs(ValuesPerOutput*InputsNo+2,false);
      DefineOutput("input_selector", InputSelectionInd);
      DefineOutput("output_selector", OutputSelectionInd);
    }
  }
  else
  {
*/
    SetNoOfInputs(ValuesPerOutput*InputsNo,false);
/*  } */
  SelectedInputNo  = DSP::c::FO_NoInput;
  SelectedOutputNo = DSP::c::FO_NoOutput;
  MaxSelectedInputNo=InputsNo-1;
  MaxSelectedOutputNo=OutputsNo-1;

  if (IsInputComplex == false)
  {
    for (ind=0; ind<InputsNo; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineOutput(temp, ind);
      temp = "in" + to_string(ind+1) + ".re";
      DefineOutput(temp, ind);
    }
    for (ind=0; ind<OutputsNo; ind++)
    {
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, ind);
      temp = "out" + to_string(ind+1) + ".re";
      DefineOutput(temp, ind);
    }
  }
  else
  {
    for (ind=0; ind<InputsNo; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineOutput(temp, 2*ind, 2*ind+1);
      temp = "in" + to_string(ind+1) + ".re";
      DefineOutput(temp, 2*ind);
      temp = "in" + to_string(ind+1) + ".im";
      DefineOutput(temp, 2*ind+1);
    }
    for (ind=0; ind<OutputsNo; ind++)
    {
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, 2*ind, 2*ind+1);
      temp = "out" + to_string(ind+1) + ".re";
      DefineOutput(temp, 2*ind);
      temp = "out" + to_string(ind+1) + ".im";
      DefineOutput(temp, 2*ind+1);
    }
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);
}

void DSP::u::Switch::Select(unsigned int InputIndex, unsigned int OutputIndex)
{
  SelectedInputNo  = InputIndex;
  SelectedOutputNo = OutputIndex;

  if (SelectedInputNo > MaxSelectedInputNo)
    SelectedInputNo = DSP::c::FO_NoInput;
  if (SelectedOutputNo > MaxSelectedOutputNo)
    SelectedOutputNo = DSP::c::FO_NoOutput;
  /*
  if (SelectedInputNo < MinSelectedIndexNo)
    SelectedInputNo = MinSelectedIndexNo;
  if (SelectedOutputNo < MinSelectedIndexNo)
    SelectedOutputNo = MinSelectedIndexNo;
    */
}

void DSP::u::Switch::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  /* \todo_later Consider processing Output as soon as SelectedInputs are avaiable
   * \todo_later Consider storing only SelectedInputs the rest could be ignored
   *
   */
  unsigned int index;

/*
  if (InputNo > LastInputInd)
  {
    if (InputNo == InputSelectionInd)
      SelectedInputNo=((int)(floor(value)));
    if (InputNo == OutputSelectionInd)
      SelectedOutputNo=((int)(floor(value)));
  }
  else
  {
*/
    index=InputNo / ((DSP::u::Switch *)block)->ValuesPerOutput;
    ((DSP::u::Switch *)block)->State[index]=value;
/*  } */
  ((DSP::u::Switch *)block)->NoOfInputsProcessed++;

  if (((DSP::u::Switch *)block)->NoOfInputsProcessed == ((DSP::u::Switch *)block)->NoOfInputs)
  {
/*
   if (SelectedInputNo > MaxSelectedInputNo)
     SelectedInputNo = MaxSelectedInputNo;
   if (SelectedOutputNo > MaxSelectedOutputNo)
     SelectedOutputNo = MaxSelectedOutputNo;
   if (SelectedInputNo < MinSelectedIndexNo)
     SelectedInputNo = MinSelectedIndexNo;
   if (SelectedOutputNo < MinSelectedIndexNo)
     SelectedOutputNo = MinSelectedIndexNo;
*/
    if ((((DSP::u::Switch *)block)->SelectedInputNo != DSP::c::FO_NoInput) && (((DSP::u::Switch *)block)->SelectedOutputNo != DSP::c::FO_NoOutput))
    {
      if (((DSP::u::Switch *)block)->ValuesPerOutput == 1)
      {
//        OutputBlocks[SelectedOutputNo]->
//          Execute(OutputBlocks_InputNo[SelectedOutputNo], State[SelectedInputNo], this);
        ((DSP::u::Switch *)block)->OutputBlocks[((DSP::u::Switch *)block)->SelectedOutputNo]->EXECUTE_PTR(
            ((DSP::u::Switch *)block)->OutputBlocks[((DSP::u::Switch *)block)->SelectedOutputNo],
            ((DSP::u::Switch *)block)->OutputBlocks_InputNo[((DSP::u::Switch *)block)->SelectedOutputNo],
            ((DSP::u::Switch *)block)->State[((DSP::u::Switch *)block)->SelectedInputNo], block);
      }
      else // if (ValuesPerOutput == 2)
      {
//        OutputBlocks[SelectedOutputNo*2]->
//          Execute(OutputBlocks_InputNo[SelectedOutputNo*2], State[SelectedInputNo*2], this);
        ((DSP::u::Switch *)block)->OutputBlocks[((DSP::u::Switch *)block)->SelectedOutputNo*2]->EXECUTE_PTR(
            ((DSP::u::Switch *)block)->OutputBlocks[((DSP::u::Switch *)block)->SelectedOutputNo*2],
            ((DSP::u::Switch *)block)->OutputBlocks_InputNo[((DSP::u::Switch *)block)->SelectedOutputNo*2],
            ((DSP::u::Switch *)block)->State[((DSP::u::Switch *)block)->SelectedInputNo*2], block);
//        OutputBlocks[SelectedOutputNo*2+1]->
//          Execute(OutputBlocks_InputNo[SelectedOutputNo*2+1], State[SelectedInputNo*2+1], this);
        ((DSP::u::Switch *)block)->OutputBlocks[((DSP::u::Switch *)block)->SelectedOutputNo*2+1]->EXECUTE_PTR(
            ((DSP::u::Switch *)block)->OutputBlocks[((DSP::u::Switch *)block)->SelectedOutputNo*2+1],
            ((DSP::u::Switch *)block)->OutputBlocks_InputNo[((DSP::u::Switch *)block)->SelectedOutputNo*2+1],
            ((DSP::u::Switch *)block)->State[((DSP::u::Switch *)block)->SelectedInputNo*2+1], block);
      }
    } // otherwise simply ignore input (NO output)
    ((DSP::u::Switch *)block)->NoOfInputsProcessed = ((DSP::u::Switch *)block)->InitialNoOfInputsProcessed;
  }
};

/**************************************************/
// Decimator without antialias filter
DSP::u::RawDecimator::RawDecimator(DSP::Clock_ptr ParentClock,
                  unsigned int M_in,
                  unsigned int InputsNo)
  : DSP::Block(), DSP::Source()
{
  unsigned int ind;
  vector <unsigned int> inds;
  string temp;


  SetName("Raw decimator", false);
  if (InputsNo == 0)
    InputsNo=0;
  SetNoOfOutputs(InputsNo);
  SetNoOfInputs(InputsNo,false);
  inds.resize(InputsNo);
  for (ind=0; ind<InputsNo; ind++)
  {
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind);
    inds[ind]=ind;
  }
  DefineInput("in", inds);
  DefineOutput("out", inds);


  if (M_in > 0)
    M=M_in;
  else
    M=1; //without decimation

  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));
  ClockGroups.AddClockRelation("input", "output", 1, M);
  IsMultirate=true;
//  L_factor=1; M_factor=M; //set basic decimation factor

  if (ParentClock != NULL)
  {
    RegisterOutputClock(DSP::Clock::GetClock(ParentClock, 1, M));
  }
  else
  {
    DSP::log << DSP::LogMode::Error << "DSP::u::RawDecimator" << DSP::LogMode::second << "Undefined ParentClock" << endl;
    return;
  }

  State=new DSP::Float[NoOfInputs];
  IsReady=false; //Input value not yet received
  InnerCounter=0; //Input received would be stored as an output value
  for (ind=0; ind<NoOfInputs; ind++)
  {
    State[ind]=0.0;
  }

  Execute_ptr = &InputExecute;
  OutputExecute_ptr = &OutputExecute;
}

DSP::u::RawDecimator::~RawDecimator(void)
{
//  SetNoOfOutputs(0);
  delete [] State;
}

//Execution as an processing block
#define THIS ((DSP::u::RawDecimator *)block)
void DSP::u::RawDecimator::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->NoOfInputsProcessed++;
  if (THIS->InnerCounter == 0)
  {
    THIS->State[InputNo]=value;
    if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
      THIS->IsReady=true;
  }

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    THIS->InnerCounter++;
    THIS->InnerCounter %= THIS->M;

    THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;
  }
}
#undef THIS

//Execution as a source block
#define THIS ((DSP::u::RawDecimator *)source)
bool DSP::u::RawDecimator::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);
  unsigned int ind;

  if (THIS->IsReady == false)
    return false; //output samples not ready yet

  for (ind=0; ind < THIS->NoOfOutputs; ind++)
  {
    THIS->OutputBlocks[ind]->EXECUTE_PTR(
        THIS->OutputBlocks[ind],
        THIS->OutputBlocks_InputNo[ind],
        THIS->State[ind], source);
    THIS->IsReady = false;  //output sample just processed
  }

  return true;
}
#undef THIS

/**************************************************/
// Time expansion block: zeroinserter (+ hold)
DSP::u::Zeroinserter::Zeroinserter(DSP::Clock_ptr ParentClock, unsigned int L_in, bool Hold)
  : DSP::Block(), DSP::Source()
{ //if Hold == true, holds input value instead of inserting zeros
  Init(false, ParentClock, L_in, Hold);
};

DSP::u::Zeroinserter::Zeroinserter(bool IsInputComplex, DSP::Clock_ptr ParentClock, unsigned int L_in, bool Hold)
  : DSP::Block(), DSP::Source()
{ //if Hold == true, holds input value instead of inserting zeros
  Init(IsInputComplex, ParentClock, L_in, Hold);
};

void DSP::u::Zeroinserter::Init(bool IsInputComplex, DSP::Clock_ptr ParentClock, unsigned int L_in, bool Hold)
{ //if Hold == true, holds input value instead of inserting zeros
  SetName("Zeroinserter", false);

  if (IsInputComplex == false)
  {
    SetNoOfOutputs(1);
    SetNoOfInputs(1,false);
    DefineInput("in",0);
    DefineOutput("out", 0);
  }
  else
  {
    SetNoOfOutputs(2);
    SetNoOfInputs(2,false);
    DefineInput("in",0,1);
    DefineInput("in.re",0);
    DefineInput("in.im",1);
    DefineOutput("out", 0,1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }

  IsHold=Hold;
  if (L_in > 0)
    L=L_in;
  else
    L=1; //without decimation

  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));
  ClockGroups.AddClockRelation("input", "output", L, 1);
  IsMultirate=true;
//  L_factor=L; M_factor=1; //set basic interpolation factor

  if (ParentClock != NULL)
  {
    RegisterOutputClock(DSP::Clock::GetClock(ParentClock, L, 1));
  }
  else
  {
    // OutputClocks[0]=DSP::Clock::GetClock(L, 1);
    DSP::log << DSP::LogMode::Error << "DSP::u::Zeroinserter" << DSP::LogMode::second << "Undefined ParentClock" << endl;
    return;
  }

  State_re=0.0; State_im=0.0;
  IsInputReady=false; IsReady=false; //Input value not yet received
  InnerCounter=0; //Input received would be stored as an output value

  if (IsInputComplex == false)
  {
    Execute_ptr = &InputExecute_real;
    OutputExecute_ptr = &OutputExecute_real;
  }
  else
  {
    Execute_ptr = &InputExecute_cplx;
    OutputExecute_ptr = &OutputExecute_cplx;
  }
};

DSP::u::Zeroinserter::~Zeroinserter(void)
{
//  SetNoOfOutputs(0);
};

#define THIS ((DSP::u::Zeroinserter *)block)

void DSP::u::Zeroinserter::InputExecute_real(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->tempInput_re = value;
  THIS->IsInputReady=true;
  THIS->IsReady=true;
};

void DSP::u::Zeroinserter::InputExecute_cplx(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo == 0)
    THIS->tempInput_re = value;
  else
    THIS->tempInput_im = value;
  THIS->NoOfInputsProcessed++;
  if (THIS->NoOfInputsProcessed == 2)
  {
    THIS->IsInputReady=true;
    THIS->IsReady=true;

    THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;
  }
};
#undef THIS

#define THIS ((DSP::u::Zeroinserter *)source)
bool DSP::u::Zeroinserter::OutputExecute_real(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS->IsReady == false)
    return false; //output sample not ready yet

  if (THIS->InnerCounter == 0)
  {
    if (THIS->IsInputReady == true)
    {
      THIS->State_re = THIS->tempInput_re;
      THIS->IsInputReady = false;
//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State, this);
      THIS->OutputBlocks[0]->EXECUTE_PTR(
          THIS->OutputBlocks[0],
          THIS->OutputBlocks_InputNo[0],
          THIS->State_re, source);
      if (THIS->IsHold == false)
        THIS->State_re = 0.0;
    }
    else
      return false; //Input sample not ready yet
  }
  else
  {
//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State, this);
    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0],
        THIS->OutputBlocks_InputNo[0],
        THIS->State_re, source);
  }
  THIS->InnerCounter++;
  THIS->InnerCounter %= THIS->L;
  if (THIS->InnerCounter == 0)
    THIS->IsReady=false;  //output sample completely processed

  return true;
}

bool DSP::u::Zeroinserter::OutputExecute_cplx(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS->IsReady == false)
    return false; //output sample not ready yet

  if (THIS->InnerCounter == 0)
  {
    if (THIS->IsInputReady == true)
    {
      THIS->State_re = THIS->tempInput_re;
      THIS->State_im = THIS->tempInput_im;
      THIS->IsInputReady = false;
//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State, this);
      THIS->OutputBlocks[0]->EXECUTE_PTR(
          THIS->OutputBlocks[0],
          THIS->OutputBlocks_InputNo[0],
          THIS->State_re, source);
      THIS->OutputBlocks[1]->EXECUTE_PTR(
          THIS->OutputBlocks[1],
          THIS->OutputBlocks_InputNo[1],
          THIS->State_im, source);
      if (THIS->IsHold == false)
      {
        THIS->State_re = 0.0;
        THIS->State_im = 0.0;
      }
    }
    else
      return false; //Input sample not ready yet
  }
  else
  {
//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], State, this);
    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0],
        THIS->OutputBlocks_InputNo[0],
        THIS->State_re, source);
    THIS->OutputBlocks[1]->EXECUTE_PTR(
        THIS->OutputBlocks[1],
        THIS->OutputBlocks_InputNo[1],
        THIS->State_im, source);
  }
  THIS->InnerCounter++;
  THIS->InnerCounter %= THIS->L;
  if (THIS->InnerCounter == 0)
    THIS->IsReady=false;  //output sample completely processed

  return true;
}
#undef THIS


// ***************************************************** //
// generates const values
/*
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
DSP::u::Const::Const(DSP::Clock_ptr ParentClock,
                       DSP::Float value)
  : DSP::Source()
{
  SetName("Const", false);

  SetNoOfOutputs(1);
  //SetNoOfInputs(0,false);
  DefineOutput("out", 0);
  DefineOutput("out.re", 0);

  RegisterOutputClock(ParentClock);

  const_val = value;
  const_state = NULL;

  OutputExecute_ptr = &OutputExecute_one;
}

DSP::u::Const::Const(DSP::Clock_ptr ParentClock,
                       DSP::Float value_re, DSP::Float value_im)
  : DSP::Source()
{
  SetName("Const", false);

  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  RegisterOutputClock(ParentClock);

  const_val = 0; // unused in this version of a block

  const_state = new DSP::Float[2];
  const_state[0] = value_re;
  const_state[1] = value_im;

  OutputExecute_ptr = &OutputExecute_many;
}

DSP::u::Const::Const(DSP::Clock_ptr ParentClock,
                       DSP::Complex value)
  : DSP::Source()
{
  SetName("Const", false);

  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  RegisterOutputClock(ParentClock);

  const_val = 0; // unused in this version of a block

  const_state = new DSP::Float[2];
  const_state[0] = value.re;
  const_state[1] = value.im;

  OutputExecute_ptr = &OutputExecute_many;
}

DSP::u::Const::Const(DSP::Clock_ptr ParentClock,
    unsigned int NoOfOutputs_in, DSP::Float_ptr values)
  : DSP::Source()
{
  string tekst;
  unsigned int ind;
  vector <unsigned int> temp_int;

  SetName("Const", false);

  SetNoOfOutputs(NoOfOutputs_in);
  temp_int.resize(NoOfOutputs_in);
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    tekst = "out" + to_string(ind+1);
    DefineOutput(tekst, ind);
    tekst = "out" + to_string(ind+1) + ".re";
    DefineOutput(tekst, ind);

    temp_int[ind] = ind;
  }
  DefineOutput("out", temp_int);

  RegisterOutputClock(ParentClock);

  const_val = 0; // unused in this version of a block

  const_state = new DSP::Float[NoOfOutputs_in];
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    const_state[ind] = values[ind];
  }

  OutputExecute_ptr = &OutputExecute_many;
}

DSP::u::Const::Const(DSP::Clock_ptr ParentClock,
    unsigned int NoOfOutputs_in, DSP::Complex_ptr values)
  : DSP::Source()
{
  string tekst;
  unsigned int ind;
  vector <unsigned int> temp_int;

  SetName("Const", false);

  SetNoOfOutputs(2*NoOfOutputs_in);
  temp_int.resize(2*NoOfOutputs_in);
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    tekst = "out" + to_string(ind+1);
    DefineOutput(tekst, 2*ind, 2*ind+1);
    tekst = "out" + to_string(ind+1) + ".re";
    DefineOutput(tekst, 2*ind);
    tekst = "out" + to_string(ind+1) + ".im";
    DefineOutput(tekst, 2*ind+1);

    temp_int[ind] = ind;
  }
  DefineOutput("out", temp_int);

  RegisterOutputClock(ParentClock);

  const_val = 0; // unused in this version of a block

  const_state = new DSP::Float[2*NoOfOutputs_in];
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    const_state[2*ind] = values[ind].re;
    const_state[2*ind+1] = values[ind].im;
  }

  OutputExecute_ptr = &OutputExecute_many;
}

DSP::u::Const::~Const(void)
{
  if (const_state != NULL)
  {
    delete [] const_state;
    const_state = NULL;
  }
}

#define THIS_ ((DSP::u::Const *)source)
bool DSP::u::Const::OutputExecute_one(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  THIS_->OutputBlocks[0]->EXECUTE_PTR(
      THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0],
      THIS_->const_val, source);

  return true;
};

bool DSP::u::Const::OutputExecute_many(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);
  unsigned int ind;

  for (ind = 0; ind < THIS_->NoOfOutputs; ind++)
    THIS_->OutputBlocks[ind]->EXECUTE_PTR(
          THIS_->OutputBlocks[ind], THIS_->OutputBlocks_InputNo[ind],
          THIS_->const_state[ind], source);

  return true;
};
#undef THIS_


// ***************************************************** //
// generates pulse train
// a) N0 > N1:
//   y[n]=exp(alfa*n)*cos(omega*n+phase)*(u[n-N0]-u[n-N1])
// b) N0 <= N1:
//   y[n]=exp(alfa*n)*cos(omega*n+phase)*u[n-N0]
// if period > 0 -> n = (n+1) % period;
DSP::u::COSpulse::COSpulse(DSP::Clock_ptr ParentClock,
                  DSP::Float A_in, DSP::Float alfa_in,
                  DSP::Float omega_in, DSP::Float phase_in,
                  unsigned long N0_in, unsigned long N1_in,
                  unsigned long period_in)
  : DSP::Source()
{
  SetName("COSpulse", false);

  SetNoOfOutputs(1);
  //SetNoOfInputs(0,false);
  DefineOutput("out", 0);
  DefineOutput("out.re", 0);
  IsMultiClock=false;

  Init(A_in, alfa_in, omega_in, phase_in,
       N0_in, N1_in, period_in, ParentClock);

  OutputExecute_ptr = &OutputExecute;
}

DSP::u::COSpulse::COSpulse(DSP::Clock_ptr ParentClock,
                  bool IsComplex,
                  DSP::Float A_in, DSP::Float alfa_in,
                  DSP::Float omega_in, DSP::Float phase_in,
                  unsigned long N0_in, unsigned long N1_in,
                  unsigned long period_in)
  : DSP::Source()
{
  SetName("COSpulse", false);

  if (IsComplex==true)
  {
    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    SetNoOfOutputs(1);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }
  //SetNoOfInputs(0,false);
  IsMultiClock=false;

  Init(A_in, alfa_in, omega_in, phase_in,
       N0_in, N1_in, period_in, ParentClock);

  OutputExecute_ptr = &OutputExecute;
}

void DSP::u::COSpulse::Init(DSP::Float A_in, DSP::Float alfa_in,
                  DSP::Float omega_in, DSP::Float phase_in,
                  unsigned long N0_in, unsigned long N1_in,
                  unsigned long period_in,
                  DSP::Clock_ptr ParentClock)
{
  RegisterOutputClock(ParentClock);

  A= A_in;
  alfa=alfa_in;
  omega=omega_in; phase=phase_in;
  N0=N0_in; N1=N1_in;
  period=period_in;

  n=0;
};

// changes amplitude parameter
void DSP::u::COSpulse::SetAmplitude(DSP::Float new_amplitude)
{
  A = new_amplitude;
}

// Changes angular frequency
void DSP::u::COSpulse::SetAngularFrequency(DSP::Float omega_in)
{
  omega=omega_in;
}

// Changes pulse length in samples
void DSP::u::COSpulse::SetPulseLength(int pulse_length)
{
  N1 = N0 + pulse_length;
}

// Changes pulse train period in samples
void DSP::u::COSpulse::SetPulsePeriod(int pulse_period)
{
  period = pulse_period;
}

DSP::u::COSpulse::~COSpulse(void)
{
//  SetNoOfOutputs(0);
};

bool DSP::u::COSpulse::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);
  DSP::Float value, value_im;

  value=0.0;
  if (((DSP::u::COSpulse *)source)->n >= ((DSP::u::COSpulse *)source)->N0)
    if (!((((DSP::u::COSpulse *)source)->N1 > ((DSP::u::COSpulse *)source)->N0) &&
          (((DSP::u::COSpulse *)source)->n >= ((DSP::u::COSpulse *)source)->N1)))
      value = ((DSP::u::COSpulse *)source)->A *
              EXP(
                  ((DSP::u::COSpulse *)source)->alfa * (DSP::Float)(((DSP::u::COSpulse *)source)->n)) *
                  COS(((DSP::u::COSpulse *)source)->omega * (DSP::Float)(((DSP::u::COSpulse *)source)->n) + ((DSP::u::COSpulse *)source)->phase);
//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], value, this);
  ((DSP::u::COSpulse *)source)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::COSpulse *)source)->OutputBlocks[0],
      ((DSP::u::COSpulse *)source)->OutputBlocks_InputNo[0],
      value, source);

  if (((DSP::u::COSpulse *)source)->NoOfOutputs == 2)
  {
    value_im=0.0;
    if (((DSP::u::COSpulse *)source)->n > ((DSP::u::COSpulse *)source)->N0)
      if (!((((DSP::u::COSpulse *)source)->N1 > ((DSP::u::COSpulse *)source)->N0) &&
            (((DSP::u::COSpulse *)source)->n >= ((DSP::u::COSpulse *)source)->N1)))
        value_im = ((DSP::u::COSpulse *)source)->A *
                   EXP(((DSP::u::COSpulse *)source)->alfa * (DSP::Float)(((DSP::u::COSpulse *)source)->n)) *
                   SIN(((DSP::u::COSpulse *)source)->omega * (DSP::Float)(((DSP::u::COSpulse *)source)->n) + ((DSP::u::COSpulse *)source)->phase);
//    OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], value_im, this);
    ((DSP::u::COSpulse *)source)->OutputBlocks[1]->EXECUTE_PTR(
        ((DSP::u::COSpulse *)source)->OutputBlocks[1],
        ((DSP::u::COSpulse *)source)->OutputBlocks_InputNo[1],
        value_im, source);
  }

  ((DSP::u::COSpulse *)source)->n++;
  if (((DSP::u::COSpulse *)source)->period > 0)
    ((DSP::u::COSpulse *)source)->n %= ((DSP::u::COSpulse *)source)->period;

  return true;
};

// ***************************************************** //
// generates uniform noise
DSP::u::Rand::Rand(DSP::Clock_ptr ParentClock)
  : DSP::Source()
{
  SetName("rand", false);

  SetNoOfOutputs(1);
  DefineOutput("out", 0);
  DefineOutput("out.re", 0);
  IsMultiClock=false;

  Init(ParentClock);

  OutputExecute_ptr = &OutputExecute;
}

DSP::u::Rand::Rand(DSP::Clock_ptr ParentClock,
                  bool IsComplex)
  : DSP::Source()
{
  SetName("rand", false);

  if (IsComplex==true)
  {
    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    SetNoOfOutputs(1);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }
  IsMultiClock=false;

  Init(ParentClock);

  OutputExecute_ptr = &OutputExecute;
}

void DSP::u::Rand::Init(DSP::Clock_ptr ParentClock)
{
  RegisterOutputClock(ParentClock);
 };

DSP::u::Rand::~Rand(void)
{
};

bool DSP::u::Rand::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);
  DSP::Float value, value_im;

  value =  (DSP::Float)rand();
  value /= DSP::Float(RAND_MAX);
//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], value, this);
  ((DSP::u::Rand *)source)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::Rand *)source)->OutputBlocks[0],
      ((DSP::u::Rand *)source)->OutputBlocks_InputNo[0],
      value, source);

  if (((DSP::u::Rand *)source)->NoOfOutputs == 2)
  {
    value_im =  (DSP::Float)rand();
    value_im /= DSP::Float(RAND_MAX);
//    OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], value_im, this);
    ((DSP::u::Rand *)source)->OutputBlocks[1]->EXECUTE_PTR(
        ((DSP::u::Rand *)source)->OutputBlocks[1],
        ((DSP::u::Rand *)source)->OutputBlocks_InputNo[1],
        value_im, source);
  }

  return true;
};

// ***************************************************** //
// Generates random binary streams.
DSP::u::BinRand::BinRand(DSP::Clock_ptr ParentClock,
                           DSP::Float L_value_in, DSP::Float U_value_in)
  : DSP::Source()
{
  SetName("binrand", false);

  SetNoOfOutputs(1);
  DefineOutput("out", 0);
  IsMultiClock=false;

  Init(ParentClock, L_value_in, U_value_in);

  OutputExecute_ptr = &OutputExecute;
}

void DSP::u::BinRand::Init(DSP::Clock_ptr ParentClock,
                        DSP::Float L_value_in, DSP::Float U_value_in)
{
  RegisterOutputClock(ParentClock);

  L_value = L_value_in;
  U_value = U_value_in;
};

DSP::u::BinRand::~BinRand(void)
{
};

bool DSP::u::BinRand::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);
  DSP::Float value;

  value = (rand() < (RAND_MAX/2)) ? ((DSP::u::BinRand *)source)->L_value : ((DSP::u::BinRand *)source)->U_value;
//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], value, this);
  ((DSP::u::BinRand *)source)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::BinRand *)source)->OutputBlocks[0],
      ((DSP::u::BinRand *)source)->OutputBlocks_InputNo[0],
      value, source);

  return true;
};


/* ***************************************************** //
 * Generates LFSR binary streams.
 * LFSR - linear feedback shift register.
 *
 * \note Output value can be only  L_value_in or U_value_in.
 *  Default: 0.0 and 1.0.
 *
 * Inputs and Outputs names:
 *  - Output:
 *   -# "out" (real valued)
 *  - Input: none
 */
DSP::u::LFSR::LFSR(DSP::Clock_ptr ParentClock,
                     unsigned int reg_length,
                     unsigned int no_of_taps, unsigned int *taps_idx,
		    		         bool *state,
		                 DSP::Float L_value_in, DSP::Float U_value_in)
  : DSP::Source()
{
  SetName("LFSR", false);

  SetNoOfOutputs(1);
  DefineOutput("out", 0);
  IsMultiClock=false;

  Init(ParentClock, reg_length, no_of_taps, taps_idx,
  		 state, L_value_in, U_value_in);

  OutputExecute_ptr = &OutputExecute;
}

void DSP::u::LFSR::Init(DSP::Clock_ptr ParentClock,
                     unsigned int reg_length,
		                 unsigned int no_of_taps, unsigned int *taps_idx,
		    		         bool *state,
		                 DSP::Float L_value_in, DSP::Float U_value_in)
{
	unsigned int ind, ind_1;
	unsigned long long int sum;
	unsigned long long mask;
	unsigned int ULL_size;

	ULL_size = sizeof(unsigned long long int) * 8; // ULL int in bits

  RegisterOutputClock(ParentClock);

  reg_len = reg_length;

  taps_no = no_of_taps;
  taps = new unsigned int[taps_no];
  for (ind = 0; ind < taps_no; ind++)
  	taps[ind] = reg_len - taps_idx[ind];
    //taps[ind] = taps_idx[ind] - 1;

  buffer_size = ((reg_len-1) / ULL_size);
  buffer_size++; // at least one slot
  buffer = new unsigned long long int[buffer_size];

  buffer_MSB_mask = 1ULL << ((reg_len - 1) % ULL_size);
  ULL_MSB = 1LL << (ULL_size-1);

  for (ind = 0; ind < buffer_size; ind++)
    buffer[ind] = 0ULL;
  if (state == NULL)
  { // random
  	// nonzero elements, 1 .. reg_len
  	do
  	{
  		ind_1 = 0; mask = buffer_MSB_mask;
  		for (ind = 0; ind < reg_len; ind++)
  		{
  		  if (rand() < (RAND_MAX/2))
  		    buffer[ind_1] |=  mask;
  		  mask >>= 1;
  		  if (mask == (unsigned long long int)(0))
  		  {
  		    mask = ULL_MSB;
  		    ind_1++;
  		  }
  		}
      sum = 0;
      for (ind = 0; ind < buffer_size; ind++)
        sum |= buffer[ind];
  	}
  	while (sum == 0);

  }
  else
  { // from state
    ind_1 = 0; mask = buffer_MSB_mask;
    for (ind = 0; ind < reg_len; ind++)
    {
      if (state[ind] == true)
        buffer[ind_1] |=  mask;
      mask >>= 1;
      if (mask == (unsigned long long int)(0))
      {
        mask = ULL_MSB;
        ind_1++;
      }
    }
  }

  L_value = L_value_in;
  U_value = U_value_in;
};

DSP::u::LFSR::~LFSR(void)
{
	if (buffer != NULL)
	{
		buffer_size = 0;
		delete [] buffer;
		buffer = NULL;
	}
};

#define THIS ((DSP::u::LFSR *)source)
bool DSP::u::LFSR::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);
  DSP::Float value;
  unsigned char sum;
  unsigned int ind;
  unsigned int ind_1;
  unsigned int mask_shift;

  value = ((THIS->buffer[THIS->buffer_size-1] & 1ULL) == 0) ? THIS->L_value : THIS->U_value;
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      value, source);

  // get taps modulo 2 sum
  sum = 0;
  for (ind = 0; ind < THIS->taps_no; ind++)
  {
	//! \TODO Zweryfikowa� t� p�tl� i ew. poprawi�  (THIS->ULL_MSB <==> THIS->ULL_size ?)
    ind_1 = THIS->buffer_size-1 - (unsigned int)(THIS->taps[ind] / THIS->ULL_MSB);
    mask_shift = (unsigned int)(THIS->taps[ind] % THIS->ULL_MSB);

  	sum ^= (unsigned char)((THIS->buffer[ind_1] >> mask_shift) & 1ULL);
  }

  // shift register and update MSB
  for (ind = THIS->buffer_size-1; ind > 0; ind --)
  {
  	THIS->buffer[ind] >>= 1;
  	if ((THIS->buffer[ind-1] & 1ULL) != 0)
      THIS->buffer[ind] |= THIS->ULL_MSB;
  }
  THIS->buffer[0] >>= 1;

  if (sum != 0)
    THIS->buffer[0] |= THIS->buffer_MSB_mask;

  return true;
};
#undef THIS

/* ************************************************ */
DSP::u::LFSR_tester::LFSR_tester(unsigned int reg_length,
                     unsigned int no_of_taps, unsigned int *taps_idx,
		                 DSP::Float L_value_in, DSP::Float U_value_in)
  : DSP::Block()
{
  SetName("LFSR tester", false);

  SetNoOfOutputs(1);
  DefineOutput("out", 0);
  SetNoOfInputs(1, false);
  DefineInput("in", 0);
  IsMultiClock=false;

  Init(reg_length, no_of_taps, taps_idx,
  		 L_value_in, U_value_in);

  Execute_ptr = &InputExecute;
}

void DSP::u::LFSR_tester::Init(unsigned int reg_length,
		                 unsigned int no_of_taps, unsigned int *taps_idx,
		                 DSP::Float L_value_in, DSP::Float U_value_in)
{
	unsigned int ind;
	unsigned int ULL_size;

  reg_len = reg_length;

	ULL_size = sizeof(unsigned long long int) * 8; // ULL int in bits
  ULL_MSB = 1LL << (ULL_size-1);
  buffer_MSB_mask = 1ULL << ((reg_len - 1) % ULL_size);

  taps_no = no_of_taps;
  taps = new unsigned int[taps_no];
  for (ind = 0; ind < taps_no; ind++)
    taps[ind] = reg_len - taps_idx[ind];
    //taps[ind] = taps_idx[ind] - 1;


  buffer_size = ((reg_len-1) / ULL_size);
  buffer_size++; // at least one slot
  buffer = new unsigned long long int[buffer_size];
  for (ind = 0; ind < buffer_size; ind++)
  {
    buffer[ind] = 0ULL;
  }

  L_value = L_value_in;
  U_value = U_value_in;
};

DSP::u::LFSR_tester::~LFSR_tester(void)
{
  delete [] buffer;
};

#define THIS ((DSP::u::LFSR_tester *)block)
void DSP::u::LFSR_tester::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Float out_value;
  unsigned int ind;
  unsigned int ind_1;
  unsigned int sum, mask_shift;

  // get taps modulo 2 sum
  sum = 0;
  for (ind = 0; ind < THIS->taps_no; ind++)
  { //! \TODO Zweryfikowa� t� p�tl� i ew. poprawi�  (THIS->ULL_MSB <==> THIS->ULL_size ?)
    ind_1 = THIS->buffer_size-1 - (unsigned int)(THIS->taps[ind] / THIS->ULL_MSB);
    mask_shift = (unsigned int)(THIS->taps[ind] % THIS->ULL_MSB);

    sum ^= (unsigned char)((THIS->buffer[ind_1] >> mask_shift) & 1ULL);
  }

  // shift register and update MSB
  for (ind = THIS->buffer_size-1; ind > 0; ind --)
  {
    THIS->buffer[ind] >>= 1;
    if ((THIS->buffer[ind-1] & 1ULL) != 0)
      THIS->buffer[ind] |= THIS->ULL_MSB;
  }
  THIS->buffer[0] >>= 1;
  if (value == THIS->U_value)
  {
    THIS->buffer[0] |= THIS->buffer_MSB_mask;
    sum ^= 1;
  }

  out_value = (sum == 0) ? THIS->L_value : THIS->U_value;
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value, block);
};
#undef THIS

/**************************************************/
// FIR filter implementation
// N_in - impulse response length
// h_in - impulse response samples
DSP::u::FIR::FIR(const DSP::Float_vector &h_in, int n0, int M, int L)
  : DSP::Block()
{ //real valued input & real valued coefficients
  Init(false, false, (unsigned long)(h_in.size()), h_in.data(), n0, M, L);
}

DSP::u::FIR::FIR(const DSP::Complex_vector &h_in, int n0, int M, int L)
  : DSP::Block()
{ //real valued input & complex valued coefficients
  Init(false, true, (unsigned long)(h_in.size()), h_in.data(), n0, M, L);
}

DSP::u::FIR::FIR(bool IsInputComplex, const DSP::Float_vector &h_in, int n0, int M, int L)
  : DSP::Block()
{ //real or complex valued input & real valued coefficients
  Init(IsInputComplex, false, (unsigned long)(h_in.size()), h_in.data(), n0, M, L);
}

DSP::u::FIR::FIR(bool IsInputComplex, const DSP::Complex_vector &h_in, int n0, int M, int L)
  : DSP::Block()
{ //real or complex valued input & complex valued coefficients
  Init(IsInputComplex, true, (unsigned long)(h_in.size()), h_in.data(), n0, M, L);
}

/*! \Fixed <b>2006.06.28</b> Problem with State allocation when N_in = 1
 *  \Added  <b>2012.02.28</b> Added option for extraction of impulse response of
 *   polyphase filter from prototype filter (step parameter)
 */
void DSP::u::FIR::Init(bool IsInputComplex, bool AreCoeficientsComplex,
                   unsigned long N_in, const void *h_in, int n0, int M, int L)
{
  int ind;

  SetName("FIR", false);

  if (IsInputComplex)
  {
    SetNoOfOutputs(2);
    SetNoOfInputs(2,false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    SetNoOfInputs(1,false);
    DefineInput("in", 0);
    DefineInput("in.re", 0);
    if (AreCoeficientsComplex)
    {
      SetNoOfOutputs(2);
      DefineOutput("out", 0, 1);
      DefineOutput("out.re", 0);
      DefineOutput("out.im", 1);
    }
    else
    {
      SetNoOfOutputs(1);
      DefineOutput("out", 0);
      DefineOutput("out.re", 0);
    }
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  if (n0 < 0)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::u::FIR::Init" << DSP::LogMode::second
        << "n0 (=" << n0 << ") must be in range <0, N_in-1>" << endl;
    #endif
    n0 = 0;
  }
  if (M < 1)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::u::FIR::Init" << DSP::LogMode::second
        << "M (=" << M << ") must be in range > 0" << endl;
    #endif
    M = 1;
  }

  N = N_in - n0;
  if (M > 1)
  {
    // 0, 1, 2, 3, 4
    // N_in = 5;

    // n0 = 1; ==> N = 4
    // M = 3; ==> (4+2)/3 = 2 ==> 1, 4

    // n0 = 2; ==> N = 3
    // M = 3; ==> (3+2)/3 = 1 ==> 2
    N = (N+M-1) / M;
  }

  L_step = L;
  if (L_step <= 0)
  {
    L_step = 1;
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::u::FIR::Init" << DSP::LogMode::second << "L must be >= 1" << endl;
    #endif
  }

  if (L_step == 1)
  { // standard filter in the transposed implementation (state updating instead of buffer shifting)
    if (NoOfOutputs*(N-1) < 1)
    {
      State.clear();
    }
    else
    {
      State.clear();
      State.resize(NoOfOutputs*(N-1), 0.0);
    }
  }
  else
  { // shaping filter implementation with buffer shifting
    if (NoOfOutputs*(N-1)*L_step < 1)
    {
      State.clear();
    }
    else
    {
      State.clear();
      State.resize(NoOfOutputs*(N-1)*L_step, 0.0);
      memmove_size = (long int)(sizeof(DSP::Float)*(NoOfOutputs*(N-1)*L_step - 1));
    }
  }


  h.clear(); hC.clear();

  if (AreCoeficientsComplex)
  {
    if (N > 0)
    {
      hC.resize(N);
      for (ind=0; ind<N; ind++)
        hC[ind]=((DSP::Complex_ptr)h_in)[n0+M*ind];
    }
  }
  else
  {
    if (N > 0)
    {
      h.resize(N);
      for (ind=0; ind<N; ind++)
        h[ind]=((DSP::Float_ptr)h_in)[n0+M*ind];
    }
  }

//  NoOfInputsProcessed=0;
  if (L_step == 1) // standard filter
  {
    if (NoOfInputs == 1)
    {
      if (N==1)
      {
        if (AreCoeficientsComplex == false)
          Execute_ptr = &InputExecute_RI_RH1;
        else
          Execute_ptr = &InputExecute_RI_CH1;
      }
      else
      {
        if (AreCoeficientsComplex == false)
          Execute_ptr = &InputExecute_RI_RH;
        else
          Execute_ptr = &InputExecute_RI_CH;
      }
    }
    else
    {
      if (N==1)
      {
        if (AreCoeficientsComplex == false)
          Execute_ptr = &InputExecute_CI_RH1;
        else
          Execute_ptr = &InputExecute_CI_CH1;
      }
      else
      {
        if (AreCoeficientsComplex == false)
          Execute_ptr = &InputExecute_CI_RH;
        else
          Execute_ptr = &InputExecute_CI_CH;
      }
    }
  }
  else
  { // shaping filter
    if ((NoOfInputs == 1) && (N > 1))
    {
      Execute_ptr = InputExecute_RI_RH_L;
    }
    else
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSP::u::FIR:Init" << DSP::LogMode::second << "unsupported shaping filter mode" << endl;
      #endif
    }
  }
}

DSP::u::FIR::~FIR(void)
{
//  SetNoOfOutputs(0);

  State.clear();
  h.clear();
  hC.clear();
}

#define  THIS  ((DSP::u::FIR *)block)
/* ! \Fixed <b>2006.06.28</b> When N == 1 State was used although it was NULL
void DSP::u::FIR::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value, temp_value;
  DSP::Float *temp_h;
  DSP::Complex *temp_hC;
  int ind;

  if (InputNo==0)
    THIS->in_value.re=value;
  else
    THIS->in_value.im=value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed=0;

  if (THIS->NoOfInputs==1)
  {
    THIS->in_value.im=0.0;
    if (THIS->h!=NULL) //real input + real filter response == real output
    {
      if (THIS->N == 1)
      {
        out_value.re=THIS->in_value.re*(*THIS->h);
      }
      else
      {
        temp_h=THIS->h;
        out_value.re=(*temp_h)*THIS->in_value.re + THIS->State[0];
        temp_h++;
        for (ind=0; ind<THIS->N-2; ind++)
        {
          THIS->State[ind] =
              (*temp_h)*THIS->in_value.re + THIS->State[ind+1];
          temp_h++;
        }
        THIS->State[THIS->N-2] =
            (*temp_h) * THIS->in_value.re;
      }

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  	  THIS->OutputBlocks[0]->EXECUTE_PTR(
          THIS->OutputBlocks[0],
          THIS->OutputBlocks_InputNo[0],
          out_value.re, block);
    }
    else //real input + complex filter response == complex output
    {
      if (THIS->N == 1)
      {
        out_value.set(THIS->in_value.re);
        out_value.multiply_by(*THIS->hC);
      }
      else
      {
        temp_hC = THIS->hC;
        out_value.re=(*temp_hC).re * THIS->in_value.re + THIS->State[0];
        out_value.im=(*temp_hC).im * THIS->in_value.re + THIS->State[1];
        temp_hC++;
        for (ind=0; ind < THIS->N-2; ind++)
        {
          THIS->State[2*ind]   = (*temp_hC).re * THIS->in_value.re + THIS->State[2*ind+2];
          THIS->State[2*ind+1] = (*temp_hC).im * THIS->in_value.re + THIS->State[2*ind+3];
          temp_hC++;
        }
        THIS->State[2*(THIS->N-2)] =
            (*temp_hC).re * THIS->in_value.re;
        THIS->State[2*(THIS->N-2)+1] =
            (*temp_hC).im * THIS->in_value.re;
      }

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
      THIS->OutputBlocks[0]->EXECUTE_PTR(
          THIS->OutputBlocks[0],
          THIS->OutputBlocks_InputNo[0],
          out_value.re, block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
      THIS->OutputBlocks[1]->EXECUTE_PTR(
          THIS->OutputBlocks[1],
          THIS->OutputBlocks_InputNo[1],
          out_value.im, block);
    }
  }
  else //complex input
  {
    if (THIS->h != NULL) //complex input + real filter response == complex output
    {
      if (THIS->N == 1)
      {
        out_value.re = THIS->in_value.re * (*THIS->h);
        out_value.im = THIS->in_value.im * (*THIS->h);
      }
      else
      {
        temp_h = THIS->h;
        out_value.re = THIS->in_value.re * (*temp_h) + THIS->State[0];
        out_value.im = THIS->in_value.im * (*temp_h) + THIS->State[1];
        temp_h++;
        for (ind=0; ind < THIS->N-2; ind++)
        {
          THIS->State[2*ind]  =
              (*temp_h) * THIS->in_value.re + THIS->State[2*ind+2];
          THIS->State[2*ind+1]=
              (*temp_h) * THIS->in_value.im + THIS->State[2*ind+3];
          temp_h++;
        }
        THIS->State[2*(THIS->N-2)]  =
            (*temp_h) * THIS->in_value.re;
        THIS->State[2*(THIS->N-2)+1]=
            (*temp_h) * THIS->in_value.im;
      }
    }
    else  //complex input + complex filter response == complex output
    {
      if (THIS->N == 1)
      {
        out_value.set(THIS->in_value);
        out_value.multiply_by(*THIS->hC);
      }
      else
      {
        temp_hC = THIS->hC;
        out_value.set(THIS->in_value);
        out_value.multiply_by(*temp_hC);
        out_value.re += THIS->State[0];
        out_value.im += THIS->State[1];
        temp_hC++;
        for (ind=0; ind < THIS->N-2; ind++)
        {
          temp_value.set(THIS->in_value);
          temp_value.multiply_by(*temp_hC);
          THIS->State[2*ind]  = temp_value.re + THIS->State[2*ind+2];
          THIS->State[2*ind+1]= temp_value.im + THIS->State[2*ind+3];
          temp_hC++;
        }
        temp_value.set(THIS->in_value);
        temp_value.multiply_by(*temp_hC);
        THIS->State[2*(THIS->N-2)]  =temp_value.re;
        THIS->State[2*(THIS->N-2)+1]=temp_value.im;
      }
    }
//	  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0],
        THIS->OutputBlocks_InputNo[0],
        out_value.re, block);
//	  OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
    THIS->OutputBlocks[1]->EXECUTE_PTR(
        THIS->OutputBlocks[1],
        THIS->OutputBlocks_InputNo[1],
        out_value.im, block);
  }
};
*/

//! real input - real impulse response of length N == 1
void DSP::u::FIR::InputExecute_RI_RH1(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      value*(THIS->h[0]), block);
};

//! real input - real impulse response
void DSP::u::FIR::InputExecute_RI_RH(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Float out_value;
  DSP::Float *temp_h;
  int ind;

  temp_h=THIS->h.data();
  out_value=(*temp_h)*value + THIS->State[0];
  temp_h++;
  for (ind=0; ind<THIS->N-2; ind++)
  {
    THIS->State[ind] =
        (*temp_h)*value + THIS->State[ind+1];
    temp_h++;
  }
  THIS->State[THIS->N-2] =
      (*temp_h) * value;

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value, block);
};


//! real input / complex impulse response of length N == 1
void DSP::u::FIR::InputExecute_RI_CH1(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;

  out_value.set(value);
  out_value.multiply_by(THIS->hC[0]);

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

//! real input / complex impulse response of length N > 1
void DSP::u::FIR::InputExecute_RI_CH(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;
  DSP::Complex *temp_hC;
  int ind;

  //THIS->in_value.im=0.0;
  temp_hC = THIS->hC.data();
  out_value.re=(*temp_hC).re * value + THIS->State[0];
  out_value.im=(*temp_hC).im * value + THIS->State[1];
  temp_hC++;
  for (ind=0; ind < THIS->N-2; ind++)
  {
    THIS->State[2*ind]   = (*temp_hC).re * value + THIS->State[2*ind+2];
    THIS->State[2*ind+1] = (*temp_hC).im * value + THIS->State[2*ind+3];
    temp_hC++;
  }
  THIS->State[2*(THIS->N-2)] =
      (*temp_hC).re * value;
  THIS->State[2*(THIS->N-2)+1] =
      (*temp_hC).im * value;

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

//! complex input - real impulse response of length N == 1
void DSP::u::FIR::InputExecute_CI_RH1(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;

  if (InputNo==0)
    THIS->in_value.re=value;
  else
    THIS->in_value.im=value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed=0;

  out_value.re = THIS->in_value.re * (THIS->h[0]);
  out_value.im = THIS->in_value.im * (THIS->h[0]);

//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//    OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

//! complex input - real impulse response of length N > 1
void DSP::u::FIR::InputExecute_CI_RH(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;
  DSP::Float *temp_h;
  int ind;

  if (InputNo==0)
    THIS->in_value.re=value;
  else
    THIS->in_value.im=value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed=0;

  temp_h = THIS->h.data();
  out_value.re = THIS->in_value.re * (*temp_h) + THIS->State[0];
  out_value.im = THIS->in_value.im * (*temp_h) + THIS->State[1];
  temp_h++;
  for (ind=0; ind < THIS->N-2; ind++)
  {
    THIS->State[2*ind]  =
        (*temp_h) * THIS->in_value.re + THIS->State[2*ind+2];
    THIS->State[2*ind+1]=
        (*temp_h) * THIS->in_value.im + THIS->State[2*ind+3];
    temp_h++;
  }
  THIS->State[2*(THIS->N-2)]  =
      (*temp_h) * THIS->in_value.re;
  THIS->State[2*(THIS->N-2)+1]=
      (*temp_h) * THIS->in_value.im;

//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//    OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

//! complex input - complex impulse response of length N == 1
void DSP::u::FIR::InputExecute_CI_CH1(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;

  if (InputNo==0)
    THIS->in_value.re=value;
  else
    THIS->in_value.im=value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed=0;

  out_value.set(THIS->in_value);
  out_value.multiply_by(THIS->hC[0]);

//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//    OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

//! complex input - complex impulse response of length N > 1
void DSP::u::FIR::InputExecute_CI_CH(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value, temp_value;
  DSP::Complex *temp_hC;
  int ind;

  if (InputNo==0)
    THIS->in_value.re=value;
  else
    THIS->in_value.im=value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed=0;

  temp_hC = THIS->hC.data();
  out_value.set(THIS->in_value);
  out_value.multiply_by(*temp_hC);
  out_value.re += THIS->State[0];
  out_value.im += THIS->State[1];
  temp_hC++;
  for (ind=0; ind < THIS->N-2; ind++)
  {
    temp_value.set(THIS->in_value);
    temp_value.multiply_by(*temp_hC);
    THIS->State[2*ind]  = temp_value.re + THIS->State[2*ind+2];
    THIS->State[2*ind+1]= temp_value.im + THIS->State[2*ind+3];
    temp_hC++;
  }
  temp_value.set(THIS->in_value);
  temp_value.multiply_by(*temp_hC);
  THIS->State[2*(THIS->N-2)]  =temp_value.re;
  THIS->State[2*(THIS->N-2)+1]=temp_value.im;

//    OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//    OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};


//! real input - real impulse response for shaping filter with Z^-L elements instead of Z^-1
void DSP::u::FIR::InputExecute_RI_RH_L(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Float out_value;
  DSP::Float *temp_h, *temp_state;
  int ind;

  // ++++++++++++++++++++++++++++
  // compute output value
  temp_h=THIS->h.data();
  temp_state = THIS->State.data()+(THIS->L_step-1);

  out_value=(*temp_h)*value;
  for (ind=1; ind<THIS->N; ind++)
  {
    temp_h++;
    out_value += (*temp_h)*(*temp_state);
    temp_state += THIS->L_step;
  }

  // ++++++++++++++++++++++++++++
  // update state: memmove_size = (N-1)*L
  memmove(&(THIS->State[1]), &(THIS->State[0]), THIS->memmove_size);
  THIS->State[0] = value;

  // ++++++++++++++++++++++++++++
//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value, block);
};

#undef THIS

/**************************************************/
// IIR filter implementation
DSP::u::IIR::IIR(DSP::Float_vector &a_in)
  : DSP::Block()
{
  DSP::Float_vector b_in = {1.0};

  Init(false, false, a_in, b_in);

//  Execute_ptr = &InputExecute;
}
DSP::u::IIR::IIR(DSP::Float_vector &a_in, DSP::Float_vector &b_in)
  : DSP::Block()
{
  Init(false, false, a_in, b_in);

//  Execute_ptr = &InputExecute;
}

DSP::u::IIR::IIR(bool IsInputComplex, DSP::Float_vector &a_in, DSP::Float_vector &b_in)
  : DSP::Block()
{
  Init(IsInputComplex, false, a_in, b_in);

//  Execute_ptr = &InputExecute;
}
DSP::u::IIR::IIR(bool IsInputComplex, DSP::Float_vector &a_in)
  : DSP::Block()
{
  DSP::Float_vector b_in = {1.0};
  Init(IsInputComplex, false, a_in, b_in);

//  Execute_ptr = &InputExecute;
}

DSP::u::IIR::IIR(DSP::Complex_vector &a_in)
  : DSP::Block()
{
  DSP::Complex_vector b_in = {DSP::Complex(1.0,1.0)};
  Init(false, true, a_in, b_in);

//  Execute_ptr = &InputExecute;
}
DSP::u::IIR::IIR(DSP::Complex_vector &a_in, DSP::Complex_vector &b_in)
  : DSP::Block()
{
  Init(false, true, a_in, b_in);

//  Execute_ptr = &InputExecute;
}

DSP::u::IIR::IIR(bool IsInputComplex, DSP::Complex_vector &a_in, DSP::Complex_vector &b_in)
  : DSP::Block()
{
  Init(IsInputComplex, true, a_in, b_in);

//  Execute_ptr = &InputExecute;
}
DSP::u::IIR::IIR(bool IsInputComplex, DSP::Complex_vector &a_in)
  : DSP::Block()
{
  DSP::Complex_vector b_in = {DSP::Complex(1.0, 1.0)};
  Init(IsInputComplex, true, a_in, b_in);

//  Execute_ptr = &InputExecute;
}

template <typename T>
void DSP::u::IIR::Init(bool IsInputComplex,
                   bool AreCoeficientsComplex,
                   T &a_in, T &b_in)
{
  SetName("IIR", false);

  if (IsInputComplex)
  {
    SetNoOfOutputs(2);
    SetNoOfInputs(2,false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    if (AreCoeficientsComplex)
    {
      SetNoOfOutputs(2);
      DefineOutput("out", 0, 1);
      DefineOutput("out.re", 0);
      DefineOutput("out.im", 1);
    }
    else
    {
      SetNoOfOutputs(1);
      DefineOutput("out", 0);
      DefineOutput("out.re", 0);
    }
    SetNoOfInputs(1,false);
    DefineInput("in", 0);
    DefineInput("in.re", 0);
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  FilterOrder=long(a_in.size())-1;
  if (FilterOrder<long(b_in.size())-1)
    FilterOrder=long(b_in.size())-1;
  if (FilterOrder<0)
    FilterOrder=0;

  State.clear();
  State.resize(NoOfOutputs*FilterOrder, 0.0);

  if (AreCoeficientsComplex)
  {
    a.clear(); b.clear();
    aC.resize(FilterOrder+1);
    bC.resize(FilterOrder+1);
  }
  else
  {
    a.resize(FilterOrder+1);
    b.resize(FilterOrder+1);
    aC.clear(); bC.clear();
  }

  //SetCoefs(AreCoeficientsComplex, a_in, b_in);
  SetCoefs(a_in, b_in);

  if (NoOfInputs == 1)
  {
    if (AreCoeficientsComplex == false)
    {
      if (FilterOrder == 0)
        Execute_ptr = &InputExecute_real_coefs_real_input_zero_order;
      else
        Execute_ptr = &InputExecute_real_coefs_real_input;
    }
    else
    {
      if (FilterOrder == 0)
        Execute_ptr = &InputExecute_cplx_coefs_real_input_zero_order;
      else
        Execute_ptr = &InputExecute_cplx_coefs_real_input;
    }
  }
  else
  {
    if (AreCoeficientsComplex == false)
    {
      if (FilterOrder == 0)
        Execute_ptr = &InputExecute_real_coefs_cplx_input_zero_order;
      else
        Execute_ptr = &InputExecute_real_coefs_cplx_input;
    }
    else
    {
      if (FilterOrder == 0)
        Execute_ptr = &InputExecute_cplx_coefs_cplx_input_zero_order;
      else
        Execute_ptr = &InputExecute_cplx_coefs_cplx_input;
    }
  }

//  NoOfInputsProcessed=0;
}

DSP::u::IIR::~IIR(void)
{
//  SetNoOfOutputs(0);

  State.clear();
  a.clear();
  b.clear();
  aC.clear();
  bC.clear();
}

bool DSP::u::IIR::SetCoefs(DSP::Float_vector &a_in,
                        DSP::Float_vector &b_in)
{
  long ind;
  long required_FilterOrder;
  DSP::Float a0;

  required_FilterOrder=long(a_in.size())-1;
  if (required_FilterOrder<long(b_in.size())-1)
    required_FilterOrder=long(b_in.size())-1;
  if (required_FilterOrder<0)
    required_FilterOrder=0;

  if (required_FilterOrder > FilterOrder)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::u::IIR::SetCoefs<DSP::Float_vector>" << DSP::LogMode::second << "Too many coefficients for current filter order" << endl;
    #endif
    return false;
  }

  a = a_in;
  if (a.size()==0)
  {
    a={1.0};
  }
  a.resize(FilterOrder, 0.0);

  b = b_in;
  if (b.size() ==  0)
  {
    b={1.0};
  }
  b.resize(FilterOrder, 0.0);
  if (a[0]==0.0)
    a[0]=1.0;

  a0=a[0];
  for (ind=0; ind<=FilterOrder; ind++)
  {
    a[ind]=a[ind]/a0;
    b[ind]=b[ind]/a0;
  }
  return true;
}

bool DSP::u::IIR::SetCoefs(DSP::Complex_vector &a_in,
                        DSP::Complex_vector &b_in)
{
  long ind;
  long required_FilterOrder;
  DSP::Complex aC_0;

  required_FilterOrder=long(a_in.size())-1;
  if (required_FilterOrder<long(b_in.size())-1)
    required_FilterOrder=long(b_in.size())-1;
  if (required_FilterOrder<0)
    required_FilterOrder=0;

  if (required_FilterOrder > FilterOrder)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::u::IIR::SetCoefs<DSP::Complex_vector>" << DSP::LogMode::second << "Too many coefficients for current filter order" << endl;
    #endif
    return false;
  }

  aC = a_in;
  aC.resize(FilterOrder, 0.0);
  if (aC.size()==0)
  {
    aC = {DSP::Complex(1.0, 0.0)};
  }

  bC = b_in;
  bC.resize(FilterOrder, 0.0);
  if (bC.size()==0)
  {
    bC = {DSP::Complex(1.0, 0.0)};
  }

  if ((aC[0].re==0.0) && (aC[0].im==0.0))
    aC[0].set(1.0);

  aC_0=aC[0];
  for (ind=0; ind<=FilterOrder; ind++)
  {
    aC[ind].divide_by(aC_0);
    bC[ind].divide_by(aC_0);
  }
  return true;
}


#define THIS ((DSP::u::IIR *)block)
void DSP::u::IIR::InputExecute_cplx_coefs_cplx_input_zero_order(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;

  if (InputNo==0)
    THIS->in_value.re = value;
  else
    THIS->in_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  //temp_a = THIS->aC; aC[0] == 1.0
  //y[n]=(w[0]+b0*x[n])*a0
  // 1) b0*x[n] (x[n] - complex)
  out_value.set(THIS->in_value);
  out_value.multiply_by(THIS->bC[0]);

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

void DSP::u::IIR::InputExecute_cplx_coefs_real_input_zero_order(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;

  //temp_a = THIS->aC; aC[0] == 1.0
  out_value.set(THIS->bC[0]);
  out_value.multiply_by(value);

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

void DSP::u::IIR::InputExecute_cplx_coefs_cplx_input(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;
  int ind;

  if (InputNo==0)
    THIS->in_value.re = value;
  else
    THIS->in_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  DSP::Complex *temp_a, *temp_b;
  DSP::Complex temp_val1, temp_val2;

  temp_a = THIS->aC.data();
  temp_b = THIS->bC.data();
  //y[n]=(w[0]+b0*x[n])*a0
  // 1) b0*x[n] (x[n] - complex)
  out_value.set(THIS->in_value);
  out_value.multiply_by(*temp_b);
  // 2) w[0]+last
  out_value.re += THIS->State[0];
  out_value.im += THIS->State[1];
  // 3) last*a0
  out_value.multiply_by(*temp_a);
  temp_a++; temp_b++;

  for (ind=0; ind < THIS->FilterOrder-1; ind++)
  {
    //w[k-1]=-ak*y[n]+w[k]+bk*x[n]
    // ak*y[n]
    temp_val1.set(*temp_a);
    temp_val1.multiply_by(out_value);
    // w[k]
    temp_val2.set(THIS->State[2*ind+2], THIS->State[2*ind+3]);
    // w[k]-ak*y[n]
    temp_val2.sub(temp_val1);
    // bk*x[n]
    temp_val1.set(THIS->in_value);
    temp_val1.multiply_by(*temp_b);
    // w[k-1]=bk*x[n]+(w[k]-ak*y[n])
    THIS->State[2*ind]=temp_val1.re+temp_val2.re;
    THIS->State[2*ind+1]=temp_val1.im+temp_val2.im;

    temp_a++; temp_b++;
  }

  ind = THIS->FilterOrder-1;
  //w[k-1]=-ak*y[n]+bk*x[n]
  // ak*y[n]
  temp_val2.set(*temp_a);
  temp_val2.multiply_by(out_value);
  // bk*x[n]
  temp_val1.set(THIS->in_value);
  temp_val1.multiply_by(*temp_b);
  // w[k-1]=bk*x[n]-ak*y[n]
  THIS->State[2*ind]   = temp_val1.re-temp_val2.re;
  THIS->State[2*ind+1] = temp_val1.im-temp_val2.im;

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

void DSP::u::IIR::InputExecute_cplx_coefs_real_input(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;
  int ind;

  THIS->in_value.re = value;
  //THIS->NoOfInputsProcessed++;
  //if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
  //  return;
  //THIS->NoOfInputsProcessed = 0;

  DSP::Complex *temp_a, *temp_b;
  DSP::Complex temp_val1, temp_val2;

  temp_a = THIS->aC.data();
  temp_b = THIS->bC.data();
  //y[n]=(w[0]+b0*x[n])*a0
  // 1) b0*x[n] (x[n] - real)
  out_value.re = (*temp_b).re * THIS->in_value.re;
  out_value.im = (*temp_b).im * THIS->in_value.re;
  // 2) w[0]+last
  out_value.re += THIS->State[0];
  out_value.im += THIS->State[1];
  // 3) last*a0
  out_value.multiply_by(*temp_a);
  temp_a++; temp_b++;

  for (ind=0; ind < THIS->FilterOrder-1; ind++)
  {
    //w[k-1]=-ak*y[n]+w[k]+bk*x[n]
    // ak*y[n]
    temp_val1.set(*temp_a);
    temp_val1.multiply_by(out_value);
    // w[k]
    temp_val2.set(THIS->State[2*ind+2], THIS->State[2*ind+3]);
    // w[k]-ak*y[n]
    temp_val2.sub(temp_val1);
    // bk*x[n] <- x[n] - real
    temp_val1.set(THIS->in_value.re);
    temp_val1.multiply_by(*temp_b);
    // w[k-1]=bk*x[n]+(w[k]-ak*y[n])
    THIS->State[2*ind]   = temp_val1.re + temp_val2.re;
    THIS->State[2*ind+1] = temp_val1.im + temp_val2.im;

    temp_a++; temp_b++;
  }

  ind = THIS->FilterOrder-1;
  //w[k-1]=-ak*y[n]+bk*x[n]
  // ak*y[n]
  temp_val2.set(*temp_a);
  temp_val2.multiply_by(out_value);
  // bk*x[n] <- x[n] - real
  temp_val1.set(THIS->in_value.re);
  temp_val1.multiply_by(*temp_b);
  // w[k-1]=bk*x[n]-ak*y[n]
  THIS->State[2*ind]   = temp_val1.re-temp_val2.re;
  THIS->State[2*ind+1] = temp_val1.im-temp_val2.im;

/*
  for (ind=0; ind<2*FilterOrder; ind++)
  {
    if (State[ind]>0)
    {
      if (State[ind]<1.0E-38)
        State[ind]=0.0;
    }
    else
      if ((-State[ind])<1.0E-38)
        State[ind]=0.0;
  }
*/
//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

void DSP::u::IIR::InputExecute_real_coefs_cplx_input_zero_order(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;

  if (InputNo==0)
    THIS->in_value.re = value;
  else
    THIS->in_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  out_value.re = (THIS->b[0]) * THIS->in_value.re;
  out_value.im = (THIS->b[0]) * THIS->in_value.im;

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

void DSP::u::IIR::InputExecute_real_coefs_real_input_zero_order(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  UNUSED_ARGUMENT(InputNo);
  DSP::Float out_value;

  out_value = (THIS->b[0]) * value;

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value, block);
};


void DSP::u::IIR::InputExecute_real_coefs_cplx_input(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;
  int ind;

  if (InputNo==0)
    THIS->in_value.re = value;
  else
    THIS->in_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  DSP::Float *temp_a, *temp_b;

  temp_a = THIS->a.data();
  temp_b = THIS->b.data();
  out_value.re = (*temp_a) * ((*temp_b) * THIS->in_value.re + THIS->State[0]);
  out_value.im = (*temp_a) * ((*temp_b) * THIS->in_value.im + THIS->State[1]);
  temp_a++; temp_b++;
  for (ind=0; ind < THIS->FilterOrder-1; ind++)
  {
    THIS->State[2*ind] =
        (*temp_b) * THIS->in_value.re - (*temp_a) * out_value.re + THIS->State[2*ind+2];
    THIS->State[2*ind+1]=
        (*temp_b) * THIS->in_value.im - (*temp_a) * out_value.im + THIS->State[2*ind+3];
    temp_a++; temp_b++;
  }
  THIS->State[2*(THIS->FilterOrder-1)]  =
      (*temp_b) * THIS->in_value.re - (*temp_a) * out_value.re;
  THIS->State[2*(THIS->FilterOrder-1)+1]=
      (*temp_b) * THIS->in_value.im - (*temp_a) * out_value.im;

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], out_value.im, this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      out_value.im, block);
};

void DSP::u::IIR::InputExecute_real_coefs_real_input(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex out_value;
  int ind;

  THIS->in_value.re = value;
  //THIS->NoOfInputsProcessed++;
  //if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
  //  return;
  //THIS->NoOfInputsProcessed = 0;

  DSP::Float *temp_a, *temp_b;

  temp_a = THIS->a.data();
  temp_b = THIS->b.data();
  out_value.re = (*temp_a) * ((*temp_b) * THIS->in_value.re + THIS->State[0]);
  temp_a++; temp_b++;
  for (ind=0; ind < THIS->FilterOrder-1; ind++)
  {
    THIS->State[ind] =
        (*temp_b) * THIS->in_value.re - (*temp_a)*out_value.re + THIS->State[ind+1];
    temp_a++; temp_b++;
  }
  THIS->State[THIS->FilterOrder-1] =
      (*temp_b) * THIS->in_value.re - (*temp_a) * out_value.re;

//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], out_value.re, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      out_value.re, block);
};
#undef THIS

// ***************************************************** //
// ***************************************************** //
// ***************************************************** //
void DSP::u::DDScos::Init(DSP::Float A_in,
                      DSP::Float frequency_in, DSP::Float phase_in,
                      DSP::Clock_ptr ParentClock)
{
  A=A_in;
  frequency= frequency_in/DSP_M_PIx2;
  phase= phase_in/DSP_M_PIx2;

  CurrentPhase=0.0;

  RegisterOutputClock(ParentClock);
}


// DDS cosinusoid generator with constant parameters and real output
DSP::u::DDScos::DDScos(DSP::Clock_ptr ParentClock,
                       DSP::Float A_in,
                       DSP::Float frequency_in, DSP::Float phase_in)
  : DSP::Block(), DSP::Source()
{
  SetName("DDScos", false);
  SetNoOfOutputs(1); //one real valued output
  DefineOutput("out", 0);
  DefineOutput("out.re", 0);
  SetNoOfInputs(0,false);
  IsMultiClock=false;

  Init(A_in, frequency_in, phase_in, ParentClock);
  InputParamsReady=true; //Always true

  //Execute_ptr = &InputExecute; // no inputs: use DSP::Block::InputExecute as default
  //OutputExecute_ptr = &OutputExecute;
  OutputExecute_ptr = &OutputExecute_real_no_inputs;
}

// DDS cosinusoid generator with constant parameters and complex output
DSP::u::DDScos::DDScos(DSP::Clock_ptr ParentClock,
               bool IsComplex,
               DSP::Float A_in,
               DSP::Float frequency_in, DSP::Float phase_in)
  : DSP::Block(), DSP::Source()
{
  SetName("DDScos", false);
  if (IsComplex == true)
  {
    SetNoOfOutputs(2); //one complex valued output
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    SetNoOfOutputs(1); //one real valued output
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }
  SetNoOfInputs(0,false);

//  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);
  IsMultiClock=false;

  Init(A_in, frequency_in, phase_in, ParentClock);
  InputParamsReady=true; //Always true

  Execute_ptr = &InputExecute;
  if (IsComplex == true)
    OutputExecute_ptr = &OutputExecute_complex_no_inputs;
  else
    OutputExecute_ptr = &OutputExecute_real_no_inputs;
}

// DDS cosinusoid generator with runtime changeable parameters read from inputs and real output
DSP::u::DDScos::DDScos(DSP::Clock_ptr ParentClock)
  : DSP::Block(), DSP::Source()
{
  SetName("DDScos", false);
  SetNoOfOutputs(1); //one real valued output
  DefineOutput("out", 0);
  DefineOutput("out.re", 0);
  SetNoOfInputs(3, true); // three real valued inputs and allow constants
  DefineInput("ampl", 0);
  DefineInput("puls", 1);
  DefineInput("phase", 2);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);
  IsMultiClock=false;

  Init(1.0, 0.0, 0.0, ParentClock);
  InputParamsReady=false; //should be set to false after output sample generation

  Execute_ptr = &InputExecute;
  OutputExecute_ptr = &OutputExecute_real;
}

// DDS cosinusoid generator with runtime changeable parameters read from inputs and real or complex output
DSP::u::DDScos::DDScos(DSP::Clock_ptr ParentClock, bool IsComplex)
  : DSP::Block(), DSP::Source()
{
  SetName("DDScos", false);
  if (IsComplex == true)
  {
    SetNoOfOutputs(2); //one complex valued output
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    SetNoOfOutputs(1); //one real valued output
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }
  SetNoOfInputs(3,true); // three real valued inputs allowing constant values
  DefineInput("ampl", 0);
  DefineInput("puls", 1);
  DefineInput("phase", 2);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);
  IsMultiClock=false;

  Init(1.0, 0.0, 0.0, ParentClock);
  InputParamsReady=false; //should be set to false after output sample generation

  Execute_ptr = &InputExecute;
  if (IsComplex == true)
    OutputExecute_ptr = &OutputExecute_complex;
  else
    OutputExecute_ptr = &OutputExecute_real;
}

DSP::u::DDScos::~DDScos(void)
{
}

void DSP::u::DDScos::RecalculateInitials(void)
{
  A=1.0; frequency=0.0; phase=0.0;

  if (IsUsingConstants==true)
  {
    if (IsConstantInput[0] == true)
      A = ConstantInputValues[0];
    if (IsConstantInput[1] == true)
      frequency = ConstantInputValues[1]/DSP_M_PIx2;
    if (IsConstantInput[2] == true)
      phase = ConstantInputValues[2]/DSP_M_PIx2;
  }
}

#define  THIS  ((DSP::u::DDScos *)source)
/*
bool DSP::u::DDScos::OutputExecute(DSP::Source_ptr source, DSP::Clock_ptr clock)
{
  if (THIS->NoOfInputs != 0)
  {
    if (THIS->NoOfInputsProcessed < 3)
    { //Not all parameters are already read
      return false;
    }

    // When this block has any inputs this
    // must be set to false after output sample generation
    THIS->InputParamsReady = false;
    // This must be reset
    THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;
  }

  // Generate output sample
//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0],
//                           A*cos(M_PIx2*(CurrentPhase+phase)), this);
  THIS->OutputBlocks[0]->Execute_ptr(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      THIS->A * cos(M_PIx2 * (THIS->CurrentPhase + THIS->phase)), source);
  if (THIS->NoOfOutputs == 2) // complex valued output case
//    OutputBlocks[1]->Execute(OutputBlocks_InputNo[1],
//                           A*sin(M_PIx2*(CurrentPhase+phase)), this);
    THIS->OutputBlocks[1]->Execute_ptr(
        THIS->OutputBlocks[1], THIS->OutputBlocks_InputNo[1],
        THIS->A * sin(M_PIx2 * (THIS->CurrentPhase + THIS->phase)), source);

  //Update phase for next sample
  THIS->CurrentPhase += THIS->frequency;

/ *
  CurrentPhase=fmod(CurrentPhase+0.5, 1.0);
  if (CurrentPhase<0)
    CurrentPhase+=1.0;
  CurrentPhase-=0.5;
* /

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
*/

bool DSP::u::DDScos::OutputExecute_complex(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS->NoOfInputsProcessed < 3)
  { //Not all parameters are already read
    return false;
  }

  // When this block has any inputs this
  // must be set to false after output sample generation
  THIS->InputParamsReady = false;
  // This must be reset
  THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;


  // Generate output sample
  // complex valued output case
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      THIS->A * COS(DSP_M_PIx2 * (THIS->CurrentPhase + THIS->phase)), source);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1], THIS->OutputBlocks_InputNo[1],
      THIS->A * SIN(DSP_M_PIx2 * (THIS->CurrentPhase + THIS->phase)), source);

  //Update phase for next sample
  THIS->CurrentPhase += THIS->frequency;

  if (THIS->CurrentPhase<0)
  {
    THIS->CurrentPhase -= DSP::Float(0.5);
    THIS->CurrentPhase = FMOD(THIS->CurrentPhase, 1.0);
    THIS->CurrentPhase += DSP::Float(0.5);
  }
  else
    if (THIS->CurrentPhase > 0)
    {
      THIS->CurrentPhase += DSP::Float(0.5);
      THIS->CurrentPhase = FMOD(THIS->CurrentPhase, 1.0);
      THIS->CurrentPhase -= DSP::Float(0.5);
    }

  return true;
}

bool DSP::u::DDScos::OutputExecute_real(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS->NoOfInputsProcessed < 3)
  { //Not all parameters are already read
    return false;
  }

  // When this block has any inputs this
  // must be set to false after output sample generation
  THIS->InputParamsReady = false;
  // This must be reset
  THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;


  // Generate output sample
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      THIS->A * COS(DSP_M_PIx2 * (THIS->CurrentPhase + THIS->phase)), source);

  //Update phase for next sample
  THIS->CurrentPhase += THIS->frequency;

  if (THIS->CurrentPhase<0)
  {
    THIS->CurrentPhase -= DSP::Float(0.5);
    THIS->CurrentPhase = FMOD(THIS->CurrentPhase, 1.0);
    THIS->CurrentPhase += DSP::Float(0.5);
  }
  else
    if (THIS->CurrentPhase > 0)
    {
      THIS->CurrentPhase += DSP::Float(0.5);
      THIS->CurrentPhase = FMOD(THIS->CurrentPhase, 1.0);
      THIS->CurrentPhase -= DSP::Float(0.5);
    }

  return true;
}

bool DSP::u::DDScos::OutputExecute_complex_no_inputs(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  // Generate output sample
  // complex valued output case
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      THIS->A * COS(DSP_M_PIx2 * (THIS->CurrentPhase + THIS->phase)), source);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1], THIS->OutputBlocks_InputNo[1],
      THIS->A * SIN(DSP_M_PIx2 * (THIS->CurrentPhase + THIS->phase)), source);

  //Update phase for next sample
  THIS->CurrentPhase += THIS->frequency;

  if (THIS->CurrentPhase<0)
  {
    THIS->CurrentPhase -= DSP::Float(0.5);
    THIS->CurrentPhase = FMOD(THIS->CurrentPhase, 1.0);
    THIS->CurrentPhase += DSP::Float(0.5);
  }
  else
    if (THIS->CurrentPhase > 0)
    {
      THIS->CurrentPhase += DSP::Float(0.5);
      THIS->CurrentPhase = FMOD(THIS->CurrentPhase, 1.0);
      THIS->CurrentPhase -= DSP::Float(0.5);
    }

  return true;
}

bool DSP::u::DDScos::OutputExecute_real_no_inputs(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  // Generate output sample
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0],
      THIS->A * COS(DSP_M_PIx2 * (THIS->CurrentPhase + THIS->phase)), source);

  //Update phase for next sample
  THIS->CurrentPhase += THIS->frequency;

  if (THIS->CurrentPhase<0)
  {
    THIS->CurrentPhase -= DSP::Float(0.5);
    THIS->CurrentPhase = FMOD(THIS->CurrentPhase, 1.0);
    THIS->CurrentPhase += DSP::Float(0.5);
  }
  else
    if (THIS->CurrentPhase > 0)
    {
      THIS->CurrentPhase += DSP::Float(0.5);
      THIS->CurrentPhase = FMOD(THIS->CurrentPhase, 1.0);
      THIS->CurrentPhase -= DSP::Float(0.5);
    }

  return true;
}
#undef  THIS

// Changes angular frequency (if not associated with input)
void DSP::u::DDScos::SetAngularFrequency(DSP::Float omega)
{
  if (NoOfInputs == 0)
  {
    frequency= omega/DSP_M_PIx2;
  }
#ifdef __DEBUG__
  else
    DSP::log << "DSP::u::DDScos::SetAngularFrequency" << DSP::LogMode::second << "Cannot change frequency - associated with input" << endl;
#endif
}

// Changes amplitude (if not associated with input)
void DSP::u::DDScos::SetAmplitude(DSP::Float amplitude)
{
  if (NoOfInputs == 0)
  {
    A=amplitude;
  }
#ifdef __DEBUG__
  else
    DSP::log << "DSP::u::DDScos::SetAmplitude" << DSP::LogMode::second << "Cannot change amplitude - associated with input" << endl;
#endif
}

void DSP::u::DDScos::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((DSP::u::DDScos *)block)->NoOfInputsProcessed++;
  switch (InputNo)
  {
    case 0: //amplitude
      ((DSP::u::DDScos *)block)->A=value;
      break;
    case 1: //angular frequency
      ((DSP::u::DDScos *)block)->frequency= value/DSP_M_PIx2;
      break;
    case 2: //initial phase
      ((DSP::u::DDScos *)block)->phase= value/DSP_M_PIx2;
      break;
    default:
      break;
  }
}

// ***************************************************** //
// ***************************************************** //
// ***************************************************** //
DSP::u::SamplingRateConversion::SamplingRateConversion(
    DSP::Clock_ptr ParentClock, unsigned int L_in, unsigned int M_in,
    const DSP::Complex_vector &h_in)
  : DSP::Block(), DSP::Source()
{
  Init(false, L_in, M_in, h_in, ParentClock);

  Execute_ptr = &InputExecute_real_in_cplx_h;
  OutputExecute_ptr = &OutputExecute_cplx_out;
}

DSP::u::SamplingRateConversion::SamplingRateConversion(
    bool IsInputComplex,
    DSP::Clock_ptr ParentClock, unsigned int L_in, unsigned int M_in,
    const DSP::Complex_vector &h_in)
  : DSP::Block(), DSP::Source()
{
  Init(IsInputComplex, L_in, M_in, h_in, ParentClock);

  if (IsInputComplex == false)
  {
    Execute_ptr = &InputExecute_real_in_cplx_h;
    OutputExecute_ptr = &OutputExecute_cplx_out;
  }
  else
  {
    Execute_ptr = &InputExecute_cplx_in_cplx_h;
    OutputExecute_ptr = &OutputExecute_cplx_out;
  }
}

// ***************************************************** //
DSP::u::SamplingRateConversion::SamplingRateConversion(
    DSP::Clock_ptr ParentClock, unsigned int L_in, unsigned int M_in,
    const DSP::Float_vector &h_in)
  : DSP::Block(), DSP::Source()
{
  Init(false, L_in, M_in, h_in, ParentClock);

  Execute_ptr = &InputExecute_real_in_real_h;
  OutputExecute_ptr = &OutputExecute_real_out;
}

DSP::u::SamplingRateConversion::SamplingRateConversion(
    bool IsInputComplex,
    DSP::Clock_ptr ParentClock, unsigned int L_in, unsigned int M_in,
    const DSP::Float_vector &h_in)
  : DSP::Block(), DSP::Source()
{
  Init(IsInputComplex, L_in, M_in, h_in, ParentClock);

  if (IsInputComplex == false)
  {
    Execute_ptr = &InputExecute_real_in_real_h;
    OutputExecute_ptr = &OutputExecute_real_out;
  }
  else
  {
    Execute_ptr = &InputExecute_cplx_in_real_h;
    OutputExecute_ptr = &OutputExecute_cplx_out;
  }
}

void DSP::u::SamplingRateConversion::Init(bool IsInputComplex,
    unsigned int L_in, unsigned int M_in, const DSP::Float_vector &h_in,
  DSP::Clock_ptr ParentClock)
{
  SetName("SamplingRateConversion", false);

  L=L_in; M=M_in;

//  L_factor=L; M_factor=M; //set basic interpolation factor

  if (IsInputComplex==true)
  { //complex input and complex output (real filter coefficients)
    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    SetNoOfInputs(2,false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
  }
  else
  { //real input and real output (real filter coefficients)
    SetNoOfOutputs(1);
    DefineOutput("out", 0);
    SetNoOfInputs(1,false);
    DefineInput("in", 0);
  }

  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));
  ClockGroups.AddClockRelation("input", "output", L, M);
  IsMultirate=true;

  if (ParentClock != NULL)
  {
    RegisterOutputClock(DSP::Clock::GetClock(ParentClock, L, M));
  }
  else
  {
    // OutputClocks[0]=DSP::Clock::GetClock(L, M);
    DSP::log << DSP::LogMode::Error << "DSP::u::SamplingRateConversion" << DSP::LogMode::second << "Undefined ParentClock" << endl;
    return;
  }


  long StateBufferLen = ((long)(h_in.size())+L-1)/L;
  if (IsInputComplex == false)
  {
    StateBuffer_real.resize(StateBufferLen, 0.0);
  }
  else
  {
    StateBuffer_cplx.resize(StateBufferLen, DSP::Complex(0.0, 0.0));
  }

  // make a copy of impulse response
  h_real = h_in;
  //appending zeros at the end of impulse response if necessary
  h_real.resize(StateBufferLen*L, 0.0);
  CurrentFilterIndex = 0;
  dn = 1; // start from processing first input sample

  int OutputBufferLen = (L+M-1)/M;
  if (IsInputComplex == false) // impulse response is real so the output is the same type as the input
  {
    OutputBuffer_real.resize(OutputBufferLen, 0.0);
  }
  else
  {
    OutputBuffer_cplx.resize(OutputBufferLen, DSP::Complex(0.0, 0.0));
  }
  NoOfSamplesReady=0;
}

void DSP::u::SamplingRateConversion::Init(bool IsInputComplex,
    unsigned int L_in, unsigned int M_in, const DSP::Complex_vector &h_in,
    DSP::Clock_ptr ParentClock)
{
  SetName("SamplingRateConversion", false);

  L=L_in; M=M_in;
  IsMultirate=true;
//  L_factor=L; M_factor=M; //set basic interpolation factor

  if (IsInputComplex==true)
  { //complex input and complex output (real filter coefficients)
    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    SetNoOfInputs(2,false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
  }
  else
  { //real input and complex output (complex filter coefficients)
    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    SetNoOfInputs(1,false);
    DefineInput("in", 0);
  }

  if (ParentClock != NULL)
  {
    RegisterOutputClock(DSP::Clock::GetClock(ParentClock, L, M));
  }
  else
  {
    // OutputClocks[0]=DSP::Clock::GetClock(L, M);
    DSP::log << DSP::LogMode::Error << "DSP::u::SamplingRateConversion" << DSP::LogMode::second << "Undefined ParentClock" << endl;
    return;
  }

  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));
  ClockGroups.AddClockRelation("input", "output", L, M);

  long StateBufferLen = ((long)(h_in.size())+L-1)/L;
  if (IsInputComplex == false)
  {
    StateBuffer_real.resize(StateBufferLen, 0.0);
  }
  else
  {
    StateBuffer_cplx.resize(StateBufferLen, DSP::Complex(0.0, 0.0));
  }

  // make a copy of impulse response
  h_cplx = h_in;

  //appending zeros at the end of impulse response if necessary
  h_cplx.resize(StateBufferLen*L, 0.0);

  CurrentFilterIndex = 0;
  dn = 1; // start from processing first input sample

  int OutputBufferLen = (L+M-1)/M;
  // output is always complex valued
  OutputBuffer_cplx.resize(OutputBufferLen, DSP::Complex(0.0, 0.0));
  NoOfSamplesReady=0;
}

DSP::u::SamplingRateConversion::~SamplingRateConversion(void)
{
}

#define  THIS  ((DSP::u::SamplingRateConversion *)block)
/*
 * - Push input sample into the StateBuffer
 * - While CurrentFilterIndex is < L
 *   -# compute output sample for polyphase filter number CurrentFilterIndex
 *   -# store it in OuputBuffer
 *   -# update CurrentFilterIndex: CurrentFilterIndex += M;
 * - Prepare CurrentFilterIndex for next input cycle:
 *   CurrentFilterIndex -= L;
 */
void DSP::u::SamplingRateConversion::InputExecute_real_in_real_h(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  UNUSED_ARGUMENT(InputNo);
  DSP::Float tempOut;
  unsigned int n_h, ind;

  // Push input sample into the StateBuffer
  for (int n = (int)(THIS->StateBuffer_real.size())-1; n >0; n--)
    THIS->StateBuffer_real[n] = THIS->StateBuffer_real[n-1];
  THIS->StateBuffer_real[0] = value;

  // While CurrentFilterIndex is < L
  while (THIS->CurrentFilterIndex < THIS->L)
  {
    // compute output sample for polyphase filter number CurrentFilterIndex
    n_h = THIS->CurrentFilterIndex;

    tempOut = THIS->h_real[n_h] * THIS->StateBuffer_real[0];
    //for (ind=0; ind<StateBufferLen-1; ind++)
    for (ind=1; ind < THIS->StateBuffer_real.size(); ind++)
    {
      n_h += THIS->L; //skip L-1 impulse response samples
      tempOut += THIS->h_real[n_h] * THIS->StateBuffer_real[ind];
    }

    // store it in OuputBuffer
    THIS->OutputBuffer_real[THIS->NoOfSamplesReady] = tempOut;
    THIS->NoOfSamplesReady++;

    // update CurrentFilterIndex
    THIS->CurrentFilterIndex += THIS->M;
  }

  // Prepare CurrentFilterIndex for next input cycle:
  THIS->CurrentFilterIndex -= THIS->L;
}

void DSP::u::SamplingRateConversion::InputExecute_cplx_in_real_h(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex tempOut;
  unsigned int n_h, ind;

  if (InputNo==0) {
    THIS->in_value.re = value;
  }
  else {
    THIS->in_value.im = value;
  }
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  // Push input sample into the StateBuffer
  for (long n = (long)(THIS->StateBuffer_cplx.size())-1; n > 0; n--)
    THIS->StateBuffer_cplx[n] = THIS->StateBuffer_cplx[n-1];
  THIS->StateBuffer_cplx[0] = THIS->in_value;

  // While CurrentFilterIndex is < L
  while (THIS->CurrentFilterIndex < THIS->L)
  {
    // compute output sample for polyphase filter number CurrentFilterIndex
    n_h = THIS->CurrentFilterIndex;

    {
//      tempOut.re = h_real[n_h] * StateBuffer_cplx[0].re;
//      tempOut.im = h_real[n_h] * StateBuffer_cplx[0].im;
      tempOut = THIS->h_real[n_h] * THIS->StateBuffer_cplx[0];
      //for (ind=0; ind<StateBufferLen-1; ind++)
      for (ind=1; ind < THIS->StateBuffer_cplx.size(); ind++)
      {
        n_h += THIS->L; //skip L-1 impulse response samples
//        tempOut.re += h_real[n_h] * StateBuffer_cplx[ind].re;
//        tempOut.im += h_real[n_h] * StateBuffer_cplx[ind].im;
        tempOut += THIS->h_real[n_h] * THIS->StateBuffer_cplx[ind];
      }

      // store it in OuputBuffer
      THIS->OutputBuffer_cplx[THIS->NoOfSamplesReady] = tempOut;
      THIS->NoOfSamplesReady++;
    }

    // update CurrentFilterIndex
    THIS->CurrentFilterIndex += THIS->M;
  }

  // Prepare CurrentFilterIndex for next input cycle:
  THIS->CurrentFilterIndex -= THIS->L;
}

void DSP::u::SamplingRateConversion::InputExecute_real_in_cplx_h(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  UNUSED_ARGUMENT(InputNo);
  DSP::Complex tempOut;
  unsigned int n_h, ind;

  // Push input sample into the StateBuffer
//  for (unsigned int n = 1; n <THIS->StateBuffer_real.size(); n++)
//    THIS->StateBuffer_real[n] = THIS->StateBuffer_real[n-1];
  for (int n = (int)(THIS->StateBuffer_real.size())-1; n >0; n--)
    THIS->StateBuffer_real[n] = THIS->StateBuffer_real[n-1];
  THIS->StateBuffer_real[0] = value;

  // filter_index = rem(filter_index + M, L)
  // // filter_delay = 0.5-(filter_index+0.5)/L
  // filter_dn = floor((M+filter_index+0.5)/L)
  // filter_dn = floor((M+filter_index)/L)

  // mark that the new sample have been pushed into buffer
  THIS->dn--;
  if (THIS->dn > 0)
  { // we wait for next input sample
    return;
  }

  // THIS->dn == 0 => we need to compute next output sample
  while (THIS->dn == 0)
  {
    // compute output sample for polyphase filter number CurrentFilterIndex
    n_h = THIS->CurrentFilterIndex;

//    tempOut = THIS->h_cplx[n_h] * THIS->StateBuffer_real[0];
    tempOut.re = THIS->h_cplx[n_h].re * THIS->StateBuffer_real[0];
    tempOut.im = THIS->h_cplx[n_h].im * THIS->StateBuffer_real[0];
    //for (ind=0; ind<StateBufferLen-1; ind++)
    for (ind=1; ind < THIS->StateBuffer_real.size(); ind++)
    {
      n_h += THIS->L; //skip L-1 impulse response samples
//      tempOut += THIS->h_cplx[n_h] * THIS->StateBuffer_real[ind];
      tempOut.re += THIS->h_cplx[n_h].re * THIS->StateBuffer_real[ind];
      tempOut.im += THIS->h_cplx[n_h].im * THIS->StateBuffer_real[ind];
    }

    // store it in OuputBuffer
    THIS->OutputBuffer_cplx[THIS->NoOfSamplesReady] = tempOut;
    THIS->NoOfSamplesReady++;

    // and update THIS->dn and THIS->CurrentFilterIndex
    THIS->CurrentFilterIndex = (THIS->CurrentFilterIndex + THIS->M) % THIS->L;
    THIS->dn = (THIS->M + THIS->CurrentFilterIndex)/THIS->L;
  }
}

void DSP::u::SamplingRateConversion::InputExecute_cplx_in_cplx_h(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  DSP::Complex tempOut;
  unsigned int n_h, ind;

  if (InputNo==0)
    THIS->in_value.re = value;
  else
    THIS->in_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  // Push input sample into the StateBuffer
//  for (unsigned int n = 1; n <THIS->StateBuffer_cplx.size(); n++)
//    THIS->StateBuffer_cplx[n] = THIS->StateBuffer_cplx[n-1];
  for (int n = (int)(THIS->StateBuffer_cplx.size())-1; n >0; n--)
    THIS->StateBuffer_cplx[n] = THIS->StateBuffer_cplx[n-1];
  THIS->StateBuffer_cplx[0] = THIS->in_value;

  // mark that the new sample have been pushed into buffer
  THIS->dn--;
  if (THIS->dn > 0)
  { // we wait for next input sample
    return;
  }

  // THIS->dn == 0 => we need to compute next output sample
  while (THIS->dn == 0)
  {
    // compute output sample for polyphase filter number CurrentFilterIndex
    n_h = THIS->CurrentFilterIndex;

    tempOut = THIS->h_cplx[n_h] * THIS->StateBuffer_cplx[0];
    //for (ind=0; ind<StateBufferLen-1; ind++)
    for (ind=1; ind < THIS->StateBuffer_real.size(); ind++)
    {
      n_h += THIS->L; //skip L-1 impulse response samples
      tempOut += THIS->h_cplx[n_h] * THIS->StateBuffer_cplx[ind];
    }

    // store it in OuputBuffer
    THIS->OutputBuffer_cplx[THIS->NoOfSamplesReady] = tempOut;
    THIS->NoOfSamplesReady++;

    // and update THIS->dn and THIS->CurrentFilterIndex
    THIS->CurrentFilterIndex = (THIS->CurrentFilterIndex + THIS->M) % THIS->L;
    THIS->dn = (THIS->M + THIS->CurrentFilterIndex)/THIS->L;
  }
}

#undef THIS // DSP::u::SamplingRateConversion (block)

#define  THIS  ((DSP::u::SamplingRateConversion *)source)
bool DSP::u::SamplingRateConversion::OutputExecute_real_out(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS->NoOfSamplesReady == 0)
    return false;

//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], *OutputBuffer, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->OutputBuffer_real[0], source);
  THIS->NoOfSamplesReady--;
  if (THIS->NoOfSamplesReady != 0)
    memcpy(THIS->OutputBuffer_real.data(), THIS->OutputBuffer_real.data()+1,
           THIS->NoOfSamplesReady * sizeof(DSP::Float));

  return true;
};

bool DSP::u::SamplingRateConversion::OutputExecute_cplx_out(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS->NoOfSamplesReady == 0)
    return false;

  //  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], *OutputBuffer, this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->OutputBuffer_cplx[0].re, source);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      THIS->OutputBuffer_cplx[0].im, source);
  THIS->NoOfSamplesReady--;
  if (THIS->NoOfSamplesReady != 0)
    memcpy(THIS->OutputBuffer_cplx.data(), THIS->OutputBuffer_cplx.data() + 1,
           THIS->NoOfSamplesReady * sizeof(DSP::Complex));

  return true;
};

#undef THIS // DSP::u::SamplingRateConversion (source)

/**************************************************/
// Block calculating absolute value of real or complex sample
DSP::u::ABS::ABS(bool IsInputComplex)
  : DSP::Block()
{
  SetName("ABS", false);

  if (IsInputComplex==true)
  {
    SetNoOfInputs(2, false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
  }
  else
  {
    SetNoOfInputs(1, false);
    DefineInput("in", 0);
    DefineInput("in.re", 0);
  }
  SetNoOfOutputs(1);
  DefineOutput("out", 0);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  in_value.set(0.0);
//  NoOfInputsProcessed=0;

  Execute_ptr = &InputExecute;
}

DSP::u::ABS::~ABS(void)
{
}

void DSP::u::ABS::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    ((DSP::u::ABS *)block)->in_value.re = value;
  else
    ((DSP::u::ABS *)block)->in_value.im = value;
  ((DSP::u::ABS *)block)->NoOfInputsProcessed++;

  if (((DSP::u::ABS *)block)->NoOfInputsProcessed < ((DSP::u::ABS *)block)->NoOfInputs)
    return;
  ((DSP::u::ABS *)block)->NoOfInputsProcessed=0;

//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], in_value.abs(), this);
  ((DSP::u::ABS *)block)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::ABS *)block)->OutputBlocks[0],
      ((DSP::u::ABS *)block)->OutputBlocks_InputNo[0],
      ((DSP::u::ABS *)block)->in_value.abs(), block);
}

/**************************************************/
// Block calculating complex conjugation of complex sample
DSP::u::Conjugation::Conjugation(void)
  : DSP::Block()
{
  SetName("Conjugation", false);

  SetNoOfInputs(2, false);
  DefineInput("in", 0, 1);
  DefineInput("in.re", 0);
  DefineInput("in.im", 1);

  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);
//  NoOfInputsProcessed=0;

  Execute_ptr = &InputExecute;
}

DSP::u::Conjugation::~Conjugation(void)
{
}

#define THIS ((DSP::u::Conjugation *)block)
void DSP::u::Conjugation::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->OutputBlocks[0]->EXECUTE_PTR(
        THIS->OutputBlocks[0],
        THIS->OutputBlocks_InputNo[0],
        value, block);
  else // if (InputNo==1)
    THIS->OutputBlocks[1]->EXECUTE_PTR(
        THIS->OutputBlocks[1],
        THIS->OutputBlocks_InputNo[1],
        -value, block);
}
#undef THIS

/**************************************************/
// Block calculating the phase of a complex sample
DSP::u::Angle::Angle(void)
  : DSP::Block()
{
  SetName("Angle", false);

  SetNoOfInputs(2, false);
  DefineInput("in", 0, 1);
  DefineInput("in.re", 0);
  DefineInput("in.im", 1);
  SetNoOfOutputs(1);
  DefineOutput("out", 0);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  in_value.set(0.0);
//  NoOfInputsProcessed=0;

  Execute_ptr = &InputExecute;
}

DSP::u::Angle::~Angle(void)
{
}

void DSP::u::Angle::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    ((DSP::u::Angle *)block)->in_value.re = value;
  else
    ((DSP::u::Angle *)block)->in_value.im = value;
  ((DSP::u::Angle *)block)->NoOfInputsProcessed++;

  if (((DSP::u::Angle *)block)->NoOfInputsProcessed < ((DSP::u::Angle *)block)->NoOfInputs)
    return;
  ((DSP::u::Angle *)block)->NoOfInputsProcessed=0;


//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], in_value.angle(), this);
  ((DSP::u::Angle *)block)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::Angle *)block)->OutputBlocks[0],
      ((DSP::u::Angle *)block)->OutputBlocks_InputNo[0],
      ((DSP::u::Angle *)block)->in_value.angle(), block);
}

// ****************************************************** //
// Maximum selector
DSP::u::Maximum::Maximum(unsigned int NumberOfInputs)
  : DSP::Block()
{
  string temp;
  unsigned int ind;

  SetName("Maximum", false);

  SetNoOfInputs(NumberOfInputs, false);
  for (ind=0; ind<NumberOfInputs; ind++)
  {
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
  }

  SetNoOfOutputs(2);
  DefineOutput("out", 0);
  DefineOutput("max", 0);
  DefineOutput("ind", 1);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  temp_max=0.0; max_ind=0;
//  NoOfInputsProcessed=0;

  Execute_ptr = &InputExecute;
}

DSP::u::Maximum::~Maximum(void)
{
}

void DSP::u::Maximum::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (((DSP::u::Maximum *)block)->NoOfInputsProcessed == 0)
  {
    ((DSP::u::Maximum *)block)->temp_max = value;
    ((DSP::u::Maximum *)block)->max_ind = InputNo;
  }
  else
  {
    if (((DSP::u::Maximum *)block)->temp_max < value)
    {
      ((DSP::u::Maximum *)block)->temp_max = value;
      ((DSP::u::Maximum *)block)->max_ind  = InputNo;
    }
  }
  ((DSP::u::Maximum *)block)->NoOfInputsProcessed++;

  if (((DSP::u::Maximum *)block)->NoOfInputsProcessed < ((DSP::u::Maximum *)block)->NoOfInputs)
    return;
  ((DSP::u::Maximum *)block)->NoOfInputsProcessed = 0;

//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], temp_max, this);
  ((DSP::u::Maximum *)block)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::Maximum *)block)->OutputBlocks[0],
      ((DSP::u::Maximum *)block)->OutputBlocks_InputNo[0],
      ((DSP::u::Maximum *)block)->temp_max, block);
//  OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], max_ind+1, this);
  ((DSP::u::Maximum *)block)->OutputBlocks[1]->EXECUTE_PTR(
      ((DSP::u::Maximum *)block)->OutputBlocks[1],
      ((DSP::u::Maximum *)block)->OutputBlocks_InputNo[1],
      DSP::Float(((DSP::u::Maximum *)block)->max_ind+1), block);
}

// **************************************************** //
// Outputs selected input (given by the number)
DSP::u::Selector::Selector(unsigned int NumberOfInputs, int IndexOffset)
  : DSP::Block()
{
  SetName("Selector", false);
  Init(false, NumberOfInputs, IndexOffset);

  Execute_ptr = &InputExecute;
}

DSP::u::Selector::Selector(bool AreInputsComplex,
    unsigned int NumberOfInputs, int IndexOffset)
  : DSP::Block()
{
  Init(AreInputsComplex, NumberOfInputs, IndexOffset);

  Execute_ptr = &InputExecute;
}

void DSP::u::Selector::Init(bool AreInputsComplex,
    unsigned int NumberOfInputs, int IndexOffset)
{
  unsigned int ind;
  string temp;

  if (AreInputsComplex==true)
  {
    SetNoOfInputs(2*NumberOfInputs+1, false);
    DefineInput("ind", 0);
    for (ind=0; ind<NumberOfInputs; ind++)
    {
      temp = "in" + to_string(1+ind);
      DefineInput(temp, 1+2*ind, 2+2*ind);
      temp = "in" + to_string(1+ind) + ".re";
      DefineInput(temp, 1+2*ind);
      temp = "in" + to_string(1+ind) + ".im";
      DefineInput(temp, 2+2*ind);
    }
    SetNoOfOutputs(2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  else
  {
    SetNoOfInputs(NumberOfInputs+1, false);
    DefineInput("ind", 0);
    for (ind=0; ind<NumberOfInputs; ind++)
    {
      temp = "in" + to_string(1+ind);
      DefineInput(temp, 1+ind);
      temp = "in" + to_string(1+ind) + ".re";
      DefineInput(temp, 1+ind);
    }
    SetNoOfOutputs(1);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  index_offset=IndexOffset;
  in_values_len=NumberOfInputs;
  in_values= new DSP::Complex[in_values_len];
}

DSP::u::Selector::~Selector(void)
{
  if (in_values != NULL)
  {
    delete [] in_values;
    in_values=NULL;
  }
}

void DSP::u::Selector::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
  {
    ((DSP::u::Selector *)block)->index = (int)(floor(value));
  }
  else
  {
    if (((DSP::u::Selector *)block)->NoOfOutputs==1)
      ((DSP::u::Selector *)block)->in_values[InputNo-1].re = value;
    else
    {
      if ((InputNo % 2)== 1)
      { // 1 -> 0, 3 -> 1, 5 -> 2
        ((DSP::u::Selector *)block)->in_values[InputNo >> 1].re = value;
      }
      else
      { // 2 -> 0, 4 -> 1, 6 -> 2
        ((DSP::u::Selector *)block)->in_values[(InputNo >> 1) - 1].im = value;
      }
    }
  }
  ((DSP::u::Selector *)block)->NoOfInputsProcessed++;

  if (((DSP::u::Selector *)block)->NoOfInputsProcessed < ((DSP::u::Selector *)block)->NoOfInputs)
    return;
  ((DSP::u::Selector *)block)->NoOfInputsProcessed=0;

  ((DSP::u::Selector *)block)->index -= ((DSP::u::Selector *)block)->index_offset;
  //if (((DSP::u::Selector *)block)->index < 0)
  //  ((DSP::u::Selector *)block)->index = 0;
  //else
  {
    if (((DSP::u::Selector *)block)->index >= ((DSP::u::Selector *)block)->in_values_len)
      ((DSP::u::Selector *)block)->index = ((DSP::u::Selector *)block)->in_values_len - 1;
  }

//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], in_values[index].re, this);
  ((DSP::u::Selector *)block)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::Selector *)block)->OutputBlocks[0],
      ((DSP::u::Selector *)block)->OutputBlocks_InputNo[0],
      ((DSP::u::Selector *)block)->in_values[((DSP::u::Selector *)block)->index].re, block);
  if (((DSP::u::Selector *)block)->NoOfOutputs == 2)
//    OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], in_values[index].im, this);
    ((DSP::u::Selector *)block)->OutputBlocks[1]->EXECUTE_PTR(
        ((DSP::u::Selector *)block)->OutputBlocks[1],
        ((DSP::u::Selector *)block)->OutputBlocks_InputNo[1],
        ((DSP::u::Selector *)block)->in_values[((DSP::u::Selector *)block)->index].im, block);
};

/**************************************************/
// CMPO - complex mutual power operator
DSP::u::CMPO::CMPO()
  : DSP::Block()
{
  SetName("CMPO", false);

  SetNoOfInputs(2, false);
  DefineInput("in", 0, 1);
  DefineInput("in.re", 0);
  DefineInput("in.im", 1);
  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  last_value.set(0.0);

  Execute_ptr = &InputExecute;
}

DSP::u::CMPO::~CMPO()
{
}

void DSP::u::CMPO::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
   ((DSP::u::CMPO *)block)->in_value.re = value;
  else
    ((DSP::u::CMPO *)block)->in_value.im = value;
  ((DSP::u::CMPO *)block)->NoOfInputsProcessed++;

  if (((DSP::u::CMPO *)block)->NoOfInputsProcessed < ((DSP::u::CMPO *)block)->NoOfInputs)
    return;
  ((DSP::u::CMPO *)block)->NoOfInputsProcessed = 0;

// last_value.im*=-1;
 ((DSP::u::CMPO *)block)->last_value.multiply_by(((DSP::u::CMPO *)block)->in_value);

//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], last_value.re, this);
  ((DSP::u::CMPO *)block)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::CMPO *)block)->OutputBlocks[0],
      ((DSP::u::CMPO *)block)->OutputBlocks_InputNo[0],
      ((DSP::u::CMPO *)block)->last_value.re, block);
//  OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], last_value.im, this);
  ((DSP::u::CMPO *)block)->OutputBlocks[1]->EXECUTE_PTR(
      ((DSP::u::CMPO *)block)->OutputBlocks[1],
      ((DSP::u::CMPO *)block)->OutputBlocks_InputNo[1],
      ((DSP::u::CMPO *)block)->last_value.im, block);
//  last_value.set(in_value);
  ((DSP::u::CMPO *)block)->last_value.re = ((DSP::u::CMPO *)block)->in_value.re;
  ((DSP::u::CMPO *)block)->last_value.im = -((DSP::u::CMPO *)block)->in_value.im; //already with conjuction
};

#define  THIS  ((DSP::u::CCPC *)block)
/**************************************************/
// CCPC - cartesian coordinated to polar coordinates converter
DSP::u::CCPC::CCPC()
  : DSP::Block()
{
  SetName("CCPC", false);

  SetNoOfInputs(2, false);
  DefineInput("in", 0, 1);
  DefineInput("in.re", 0);
  DefineInput("in.im", 1);
  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.abs", 0);
  DefineOutput("out.arg", 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  Execute_ptr = &InputExecute;
}

DSP::u::CCPC::~CCPC()
{
}

void DSP::u::CCPC::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->in_value.re = value;
  else
    THIS->in_value.im = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;


//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], in_value.abs(), this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->in_value.abs(), block);
//  OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], in_value.angle(), this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      THIS->in_value.angle(), block);
};
#undef THIS

/**************************************************/
// PCCC - polar coordinated to cartesian coordinates converter
DSP::u::PCCC::PCCC()
  : DSP::Block()
{
  in_value_abs = 0.0;
  in_value_phase = 0.0;

  SetName("PCCC", false);

  SetNoOfInputs(2, true);
  DefineInput("in", 0, 1);
  DefineInput("in.abs", 0);
  DefineInput("in.arg", 1);
  DefineInput("in.re", 0);
  DefineInput("in.im", 1);
  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  Execute_ptr = &InputExecute;
}

DSP::u::PCCC::~PCCC()
{
}

void DSP::u::PCCC::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    ((DSP::u::PCCC *)block)->in_value_abs = value;
  else
    ((DSP::u::PCCC *)block)->in_value_phase = value;
  ((DSP::u::PCCC *)block)->NoOfInputsProcessed++;

  if (((DSP::u::PCCC *)block)->NoOfInputsProcessed < ((DSP::u::PCCC *)block)->NoOfInputs)
    return;
  ((DSP::u::PCCC *)block)->NoOfInputsProcessed = ((DSP::u::PCCC *)block)->InitialNoOfInputsProcessed;

  if (((DSP::u::PCCC *)block)->IsUsingConstants)
  {
    if (((DSP::u::PCCC *)block)->IsConstantInput[0] == true)
      ((DSP::u::PCCC *)block)->in_value_abs = ((DSP::u::PCCC *)block)->ConstantInputValues[0];
    if (((DSP::u::PCCC *)block)->IsConstantInput[1] == true)
      ((DSP::u::PCCC *)block)->in_value_phase = ((DSP::u::PCCC *)block)->ConstantInputValues[1];
  }

//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], in_value_abs*cos(in_value_phase), this);
  ((DSP::u::PCCC *)block)->OutputBlocks[0]->EXECUTE_PTR(
      ((DSP::u::PCCC *)block)->OutputBlocks[0],
      ((DSP::u::PCCC *)block)->OutputBlocks_InputNo[0],
      ((DSP::u::PCCC *)block)->in_value_abs * COS(((DSP::u::PCCC *)block)->in_value_phase), block);
//  OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], in_value_abs*sin(in_value_phase), this);
  ((DSP::u::PCCC *)block)->OutputBlocks[1]->EXECUTE_PTR(
      ((DSP::u::PCCC *)block)->OutputBlocks[1],
      ((DSP::u::PCCC *)block)->OutputBlocks_InputNo[1],
      ((DSP::u::PCCC *)block)->in_value_abs * SIN(((DSP::u::PCCC *)block)->in_value_phase), block);
};

/**************************************************/
// User defined function block
/*
 *
 */

DSP::u::MyFunction::MyFunction(unsigned int NumberOfInputs, unsigned int NumberOfOutputs,
                               DSP::Callback_ptr func_ptr, int CallbackIdentifier)
  : DSP::Block()
{
  unsigned int ind;
  string temp;
  vector <unsigned int> indexes;

  SetName("MyFunction", false);

  if (NumberOfInputs<=0)
    NumberOfInputs=1;
  //if (NumberOfOutputs<0) NumberOfOutputs=0;
  InputData = new DSP::Float[NumberOfInputs];
  SetNoOfInputs(NumberOfInputs, false);

  indexes.resize(NoOfInputs);
  for (ind=0; ind<NoOfInputs; ind++)
  {
    temp = "in" + to_string(1+ind);
    DefineInput(temp, ind);
    indexes[ind] = ind;
  }
  DefineInput("in", indexes);

  OutputData = new DSP::Float[NumberOfOutputs];
  SetNoOfOutputs(NumberOfOutputs);
  indexes.resize(NoOfOutputs);
  for (ind=0; ind<NoOfOutputs; ind++)
  {
    temp = "out" + to_string(1+ind);
    DefineOutput(temp, ind);
    indexes[ind] = ind;
  }
  DefineOutput("out", indexes);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  UserCallbackID=CallbackIdentifier;
  UserData=NULL;
  UserFunction_ptr=func_ptr;

  // -# call from block constructor with NoOfInputs = -1;
  //  this is when the user can initiate UserData structure
  (*UserFunction_ptr)(DSP::c::Callback_Init, NULL, 0, NULL, &UserData, UserCallbackID, this);

  Execute_ptr = &InputExecute;
}

DSP::u::MyFunction::~MyFunction(void)
{
  //  -# call from block destructor with NoOfInputs = -2;
  //  this is when the user can free UserData structure
  (*UserFunction_ptr)(DSP::c::Callback_Delete, NULL, 0, NULL, &UserData, UserCallbackID, this);

  if (InputData != NULL)
    delete [] InputData;
  if (OutputData != NULL)
    delete [] OutputData;
}

void DSP::u::MyFunction::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  unsigned int ind;

  ((DSP::u::MyFunction *)block)->InputData[InputNo] = value;
  ((DSP::u::MyFunction *)block)->NoOfInputsProcessed++;

  if (((DSP::u::MyFunction *)block)->NoOfInputsProcessed < ((DSP::u::MyFunction *)block)->NoOfInputs)
    return;
  ((DSP::u::MyFunction *)block)->NoOfInputsProcessed=0;

  (*((DSP::u::MyFunction *)block)->UserFunction_ptr)(
                      ((DSP::u::MyFunction *)block)->NoOfInputs,
                      ((DSP::u::MyFunction *)block)->InputData,
                      ((DSP::u::MyFunction *)block)->NoOfOutputs,
                      ((DSP::u::MyFunction *)block)->OutputData,
                      &((DSP::u::MyFunction *)block)->UserData,
                      ((DSP::u::MyFunction *)block)->UserCallbackID,
                      block);


  for (ind=0; ind < ((DSP::u::MyFunction *)block)->NoOfOutputs; ind++)
  {
//    OutputBlocks[ind]->Execute(OutputBlocks_InputNo[ind], OutputData[ind], this);
    ((DSP::u::MyFunction *)block)->OutputBlocks[ind]->EXECUTE_PTR(
        ((DSP::u::MyFunction *)block)->OutputBlocks[ind],
        ((DSP::u::MyFunction *)block)->OutputBlocks_InputNo[ind],
        ((DSP::u::MyFunction *)block)->OutputData[ind], block);
  }
};

/**************************************************/
// Outputs some inputs samples on the basis of activation signal
/* If the activation signal is > 0 then the input values are
 * send to the output
 *
 * Output samples are processed when all input values (including activation
 * signal) are ready. When State[0] > 0 then process outputs and activate
 * output clocks, otherwise do nothing.
 *
 * State[0] - activation signal
 * State[1-...] - input signals
 *
 * This blocks creates signal activated clock but doesn't use it itself.
 *
 * Inputs and Outputs names:
 *   - Output:
 *    -# "out1", "out2", ...
 *    -# "out" == "out1" (real valued)
 *   - Input:
 *    -# "in1", "in2", ...
 *    -# "in" = "in1" (real valued)
 *    -# "act" activation signal
 */
DSP::u::SampleSelector::SampleSelector(DSP::Clock_ptr ParentClock,
                                         DSP::Clock_ptr OutputClock,
                                         bool ActivateOutputClock,
                                         int NumberOfInputs)
  : DSP::Block(), DSP::Source()
{
  Init(ParentClock, OutputClock, ActivateOutputClock, NumberOfInputs);
}

DSP::u::SampleSelector::SampleSelector(DSP::Clock_ptr ParentClock,
                                         int NumberOfInputs)
  : DSP::Block(), DSP::Source()
{
  Init(ParentClock, NULL, false, NumberOfInputs);
}

void DSP::u::SampleSelector::Init(DSP::Clock_ptr ParentClock,
                          DSP::Clock_ptr OutputClock,
                          bool ActivateOutputClock,
                          unsigned int NumberOfInputs)
{
  unsigned int ind;
  vector <unsigned int> indexes;
  string temp;

  SetName("SampleSelector", false);

  if (NumberOfInputs < 1)
    NumberOfInputs=1;

  SetNoOfOutputs(NumberOfInputs);
  if ((ActivateOutputClock == true) || (OutputClock == NULL))
  {
    SetNoOfInputs(NumberOfInputs+1, false);  //Add the activation signal
    DefineInput("act", NoOfOutputs); // the same as DefineInput("act", NoOfInputs-1);

    ClockGroups.AddInput2Group("activation", Input("act"));
  }
  else
  {
    SetNoOfInputs(NumberOfInputs, false);  //no activation signal is used
  }

  DefineInput("in.re", 0);
  DefineOutput("out.re", 0);
  if (NoOfOutputs >= 2)
  {
    DefineInput("in.im", 1);
    DefineOutput("out.im", 1);
  }
  indexes.resize(NoOfOutputs);
  for (ind=0; ind<NoOfOutputs; ind++)
  {
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind);

    indexes[ind] = ind;
  }
  DefineInput("in", indexes);
  DefineOutput("out", indexes);

  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));

  if (OutputClock != NULL)
  {
    //for (ind=0; ind<NoOfOutputs; ind++)
//    OutputClocks[0]=DSP::Clock::CreateClockGroup(ParentClock);
    OutputClocks[0]=OutputClock;

    ////ProtectOutputClock = true;
    //IsMultirate = true; // multirate block
    //L_factor = -1; M_factor = -1; // asynchronous block -> interpolation factor cannot be specified
    long L_factor, M_factor;
    IsMultirate = GetMultirateFactorsFromClocks(ParentClock, OutputClock, L_factor, M_factor, false);

    ClockGroups.AddClockRelation("input", "activation", 1, 1);
    ClockGroups.AddClockRelation("input", "output", L_factor, M_factor);
    ClockGroups.AddClockRelation("activation", "output", L_factor, M_factor);
  }
#ifdef __DEBUG__
  else
  {
    if (ActivateOutputClock == false)
      DSP::log << "DSP::u::SampleSelector" << DSP::LogMode::second << "Warning: undefined OutputClock" << endl;
    else
    {
      DSP::log << DSP::LogMode::Error << "DSP::u::SampleSelector" << DSP::LogMode::second << "undefined OutputClock" << endl;
      return;
    }
  }
#endif

  State=new DSP::Float[NoOfInputs];
  for (ind=0; ind<NoOfInputs; ind++)
    State[ind]=0.0;

  if (OutputClock == NULL)
    ActivateOutputClock = false;

  if (ActivateOutputClock == false)
  {
    if (OutputClock == NULL)
      Execute_ptr = &InputExecute_without_source_output;
    else
    {
      #ifdef __DEBUG__
      if (ParentClock == NULL)
      {
        DSP::log << DSP::LogMode::Error << "DSP::u::SampleSelector" << DSP::LogMode::second << "undefined ParentClock" << endl;
        return;
      }
      #endif
      Execute_ptr = &InputExecute;

      RegisterForNotification(ParentClock);
      RegisterOutputClock(OutputClock);
      OutputExecute_ptr = &OutputExecute;
    }
  }
  else
  {
    if (ParentClock == NULL)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSP::u::SampleSelector" << DSP::LogMode::second << "undefined ParentClock" << endl;
      #endif
      return;
    }
    #ifdef __DEBUG__
      if (ParentClock->GetMasterClockIndex() == OutputClock->GetMasterClockIndex())
        DSP::log << "DSP::u::SampleSelector" << DSP::LogMode::second << "ParentClock and OutputClock have the same MasterClock" << endl;
    #endif
    MasterClockIndex = ParentClock->GetMasterClockIndex();
    SignalActivatedClock = OutputClock;
    SignalActivatedClock_NoOfCycles = 1;
    Execute_ptr = &InputExecute_with_Activation;

    RegisterForNotification(ParentClock);
    RegisterOutputClock(SignalActivatedClock);
    OutputExecute_ptr = &OutputExecute;
  }

  SamplesReady = false;
}

DSP::u::SampleSelector::~SampleSelector(void)
{
  if (State != NULL)
  {
    delete [] State;
    State=NULL;
  }
}

#define THIS ((DSP::u::SampleSelector *)block)
void DSP::u::SampleSelector::InputExecute_without_source_output(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->State[InputNo] = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    // NoOfOutputs == NoOfInputs-1
    if (THIS->State[THIS->NoOfOutputs] > 0)
    {
      for (unsigned int ind=0; ind < THIS->NoOfOutputs; ind++)
//        OutputBlocks[ind]->Execute(OutputBlocks_InputNo[ind], State[1+ind], this);
        THIS->OutputBlocks[ind]->EXECUTE_PTR(
            THIS->OutputBlocks[ind],
            THIS->OutputBlocks_InputNo[ind],
            THIS->State[ind], block);
    }
    THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;
  }
}

void DSP::u::SampleSelector::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->State[InputNo] = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    THIS->SamplesReady = true;
    THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;
  }
}

void DSP::u::SampleSelector::InputExecute_with_Activation(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->State[InputNo] = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    THIS->SamplesReady = true;
    THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;

    // NoOfOutputs == NoOfInputs-1
    if (THIS->State[THIS->NoOfOutputs] > 0)
    {
      // we assume IsMultiClock==false
      DSP::Clock::AddSignalActivatedClock(
          THIS->MasterClockIndex,
          THIS->SignalActivatedClock, 1); // just one cycle
    }
  }
}
#undef THIS

#define THIS ((DSP::u::SampleSelector *)source)
bool DSP::u::SampleSelector::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  if (THIS->SamplesReady == false)
    return false;

  for (unsigned int ind=0; ind < THIS->NoOfOutputs; ind++)
    THIS->OutputBlocks[ind]->EXECUTE_PTR(
        THIS->OutputBlocks[ind],
        THIS->OutputBlocks_InputNo[ind],
        THIS->State[ind], source);
  return true;
};
#undef THIS

void DSP::u::SampleSelector::Notify(DSP::Clock_ptr clock)
{
  UNUSED_ARGUMENT(clock);

  // outputs sample is expected in this cycle
  SamplesReady = false;
}

/**************************************************/
DSP::u::Hold::Hold(DSP::Clock_ptr InputClock, DSP::Clock_ptr OutputClock,
                   bool UseZeros, unsigned int NumberOfInputs)
  : DSP::Block(), DSP::Source()
{
  vector <unsigned int> indexes;
  string temp;

  SetName("Hold", false);

  IsHold = !UseZeros;

  if (NumberOfInputs<1)
    NumberOfInputs=1;

  SetNoOfOutputs(NumberOfInputs);
  SetNoOfInputs(NumberOfInputs, false);
  IsMultiClock=false;

  indexes.resize(NumberOfInputs);
  for (unsigned int ind=0; ind<NumberOfInputs; ind++)
  {
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind);

    indexes[ind] = ind;
  }
  DefineInput("in", indexes);
  DefineOutput("out", indexes);

  if (NumberOfInputs == 1)
  {
    DefineInput("in.re", 0);
    DefineOutput("out.re", 0);
  }
  if (NumberOfInputs >= 2)
  {
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }


  if (InputClock == NULL)
  {
    DSP::log << DSP::LogMode::Error << "DSP::u::Hold" << DSP::LogMode::second << "Undefined InputClock (AutoUpdate not implemented yet)" << endl;
    return;
  }
  else
  {  //Register for notifications
    if (InputClock->GetMasterClockIndex() == OutputClock->GetMasterClockIndex())
    {
      #ifdef __DEBUG__
        DSP::log << "DSP::u::Hold::Hold" << DSP::LogMode::second << "WARNING: InputClock and OutputClock have the same MasterClock" << endl;
      #endif
      RegisterForNotification(InputClock);
    }
    else
    {
      // ??? is this realy necessary ???
      RegisterForNotification(InputClock);
    }
  }

  RegisterOutputClock(OutputClock);
  my_clock = OutputClock;

  long L_factor, M_factor;
  IsMultirate = GetMultirateFactorsFromClocks(InputClock, OutputClock, L_factor, M_factor, false);
  //IsMultirate = true; L_factor = -1; M_factor= -1;

  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));
  ClockGroups.AddClockRelation("input", "output", L_factor, M_factor);

  currentState=new DSP::Float[NoOfInputs];
  for (unsigned int ind=0; ind<NoOfInputs; ind++)
    currentState[ind]=0.0;
  newState=new DSP::Float[NoOfInputs];

  SamplesReady = true;

  Execute_ptr = &InputExecute;
  OutputExecute_ptr = &OutputExecute;
}

DSP::u::Hold::~Hold(void)
{
//  SetNoOfOutputs(0);

  if (currentState != NULL)
  {
    delete [] currentState;
    currentState = NULL;
  }
  if (newState != NULL)
  {
    delete [] newState;
    newState = NULL;
  }
}

//Execution as an processing block
void DSP::u::Hold::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  ((DSP::u::Hold *)block)->newState[InputNo] = value;
  ((DSP::u::Hold *)block)->NoOfInputsProcessed++;

  if (((DSP::u::Hold *)block)->NoOfInputsProcessed == ((DSP::u::Hold *)block)->NoOfInputs)
  {
    DSP::Float_ptr temp;

    temp = ((DSP::u::Hold *)block)->newState;
    ((DSP::u::Hold *)block)->newState = ((DSP::u::Hold *)block)->currentState;
    ((DSP::u::Hold *)block)->currentState = temp;

    ((DSP::u::Hold *)block)->SamplesReady = true;

    ((DSP::u::Hold *)block)->NoOfInputsProcessed = ((DSP::u::Hold *)block)->InitialNoOfInputsProcessed;
  }
}

//Execution as a source block
bool DSP::u::Hold::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  if (((DSP::u::Hold *)source)->my_clock != ((DSP::u::Hold *)source)->OutputClocks[0])
  { // This is InputClock calling ;-)
    #ifdef __DEBUG__
      if (clock == ((DSP::u::Hold *)source)->InputClocks[0])
      {
        DSP::log << "DSP::u::Hold::Execute(DSP::Clock_ptr clock)" << DSP::LogMode::second << "WARNING: InputClock expected !!!" << endl;
      }
    #endif
    return true;
  }

  //if (NoOfInputsProcessed == NoOfInputs)
  if (((DSP::u::Hold *)source)->SamplesReady == true)
  {
    for (unsigned int ind=0; ind < ((DSP::u::Hold *)source)->NoOfOutputs; ind++)
//      OutputBlocks[ind]->Execute(OutputBlocks_InputNo[ind], currentState[ind], this);
      ((DSP::u::Hold *)source)->OutputBlocks[ind]->EXECUTE_PTR(
          ((DSP::u::Hold *)source)->OutputBlocks[ind],
          ((DSP::u::Hold *)source)->OutputBlocks_InputNo[ind],
          ((DSP::u::Hold *)source)->currentState[ind], source);

    if (((DSP::u::Hold *)source)->IsHold == false)
      memset(((DSP::u::Hold *)source)->currentState, 0,
             sizeof(DSP::Float) * ((DSP::u::Hold *)source)->NoOfInputs);
    //DSP::log << "DSP::u::Hold::Execute(DSP::Clock_ptr clock)", "Hold outputs processed !!!");
    return true;
  }
  return false; //wait for input signals
}

/*! \note This is required only if Input and Output
 *  clocks have the same MasterClock
 */
void DSP::u::Hold::Notify(DSP::Clock_ptr clock)
{
  UNUSED_ARGUMENT(clock);

  // outputs sample is expected in this cycle
  SamplesReady = false;
}

/**************************************************/
// Demultiplexer block (y1[n]=x[L*n], y2[n]=x[L*n+1], yL[n]=x[L*n+L-1])
DSP::u::Demultiplexer::Demultiplexer(bool IsComplex, unsigned int OutputsNo)
  : DSP::Block() // , DSP::Source(NULL)
{
  /* It seems that decimation blocks do not need to
   * be registered to the output clock, they just should know
   * the output clock for error detection purpose
   */
  unsigned int ind;
  vector <unsigned int> indexes;
  string temp;

  SetName("Demultiplexer", false);
  if (OutputsNo < 2)
    OutputsNo = 2;
  if (IsComplex == true)
  {
    SetNoOfInputs(2,false);
    SetNoOfOutputs(2*OutputsNo);
  }
  else
  {
    SetNoOfInputs(1,false);
    SetNoOfOutputs(OutputsNo);
  }

  /* Input / output definitions */
  if (IsComplex == false)
  {
    DefineInput("in", 0);
    DefineInput("in.re", 0);
    for (ind=0; ind<OutputsNo; ind++)
    {
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, ind);
      temp = "out" + to_string(ind+1) + ".re";
      DefineOutput(temp, ind);
    }
  }
  else // if (IsComplex == true)
  {
    DefineInput("in", 0,1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
    for (ind=0; ind<OutputsNo; ind++)
    {
      temp = "out" + to_string(ind+1);
      DefineOutput(temp, 2*ind, 2*ind+1);
      temp = "out" + to_string(ind+1) + ".re";
      DefineOutput(temp, 2*ind);
      temp = "out" + to_string(ind+1) + ".im";
      DefineOutput(temp, 2*ind+1);
    }
  }

  indexes.resize(NoOfOutputs);
  for (ind=0; ind<NoOfOutputs; ind++)
  {
    indexes[ind] = ind;
  }
  DefineOutput("out", indexes);

//  IsMultirate=true;
//  L_factor=1; M_factor=OutputsNo; //set basic decimation factor

  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("outputs", Output("out"));
  ClockGroups.AddClockRelation("input", "outputs", 1, OutputsNo);
  IsMultirate = true;


//  if (ParentClock != NULL)
//  {
//    OutputClocks[0]=DSP::Clock::GetClock(ParentClock, 1, M_factor);
//  }
//  else
//  {
//    //OutputClocks[0]=DSP::Clock::GetClock(1, M);
//    ErrorMessage("DSP::u::Demultiplexer", "Undefined ParentClock");
//    return;
//  }
//  OutputClocks[0]->RegisterSource(this);

  CurrentOutputNo = 0;
  State[0]=0.0; State[1]=0.0;

  Execute_ptr = &InputExecute;
}

DSP::u::Demultiplexer::~Demultiplexer(void)
{
//  SetNoOfOutputs(0);
}

//Execution as an processing block
void DSP::u::Demultiplexer::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);
  
  /*! \TODO Zeroinserter czeka na drug� pr�bk� i nie pozwala zako�czy� cylku zegara
   *  z kolei drugi cykl generatora nie uruchomi si� dop�ki nie zako�czy si� obs�uga zeroinsertera
   */

  ((DSP::u::Demultiplexer *)block)->State[InputNo] = value;
  ((DSP::u::Demultiplexer *)block)->NoOfInputsProcessed++;

  if (((DSP::u::Demultiplexer *)block)->NoOfInputsProcessed == ((DSP::u::Demultiplexer *)block)->NoOfInputs)
  {
//    OutputBlocks[CurrentOutputNo]->Execute(
//      OutputBlocks_InputNo[CurrentOutputNo], State[0], this);
    ((DSP::u::Demultiplexer *)block)->OutputBlocks[((DSP::u::Demultiplexer *)block)->CurrentOutputNo]->EXECUTE_PTR(
        ((DSP::u::Demultiplexer *)block)->OutputBlocks[((DSP::u::Demultiplexer *)block)->CurrentOutputNo],
        ((DSP::u::Demultiplexer *)block)->OutputBlocks_InputNo[((DSP::u::Demultiplexer *)block)->CurrentOutputNo],
        ((DSP::u::Demultiplexer *)block)->State[0], block);
    ((DSP::u::Demultiplexer *)block)->CurrentOutputNo++;

    if (((DSP::u::Demultiplexer *)block)->NoOfInputs == 2)
    {
//      OutputBlocks[CurrentOutputNo]->Execute(
//        OutputBlocks_InputNo[CurrentOutputNo], State[1], this);
      ((DSP::u::Demultiplexer *)block)->OutputBlocks[((DSP::u::Demultiplexer *)block)->CurrentOutputNo]->EXECUTE_PTR(
          ((DSP::u::Demultiplexer *)block)->OutputBlocks[((DSP::u::Demultiplexer *)block)->CurrentOutputNo],
          ((DSP::u::Demultiplexer *)block)->OutputBlocks_InputNo[((DSP::u::Demultiplexer *)block)->CurrentOutputNo],
          ((DSP::u::Demultiplexer *)block)->State[1], block);
      ((DSP::u::Demultiplexer *)block)->CurrentOutputNo++;
    }

    ((DSP::u::Demultiplexer *)block)->CurrentOutputNo %= ((DSP::u::Demultiplexer *)block)->NoOfOutputs;
    ((DSP::u::Demultiplexer *)block)->NoOfInputsProcessed = ((DSP::u::Demultiplexer *)block)->InitialNoOfInputsProcessed;
  }
}

/**************************************************/
// Multiplexer block (y[L*n]=x1[n], y[L*n+1]=x2[n], y[L*n+L-1]=xL[n])
DSP::u::Multiplexer::Multiplexer(DSP::Clock_ptr ParentClock,
  bool IsComplex, unsigned int InputsNo)
  : DSP::Block(), DSP::Source()
{
  unsigned int ind;
  vector <unsigned int> indexes;
  string temp;

  SetName("Multiplexer", false);

  if (InputsNo < 2)
    InputsNo = 2;

  if (IsComplex == false)
  {
    SetNoOfOutputs(1);
    SetNoOfInputs(InputsNo,false);

    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
    for (ind=0; ind<InputsNo; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, ind);
      temp = "in" + to_string(ind+1) + ".re";
      DefineInput(temp, ind);
    }
  }
  else
  {
    SetNoOfOutputs(2);
    SetNoOfInputs(2*InputsNo,false);

    DefineOutput("out", 0,1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    for (ind=0; ind<InputsNo; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, 2*ind, 2*ind+1);
      temp = "in" + to_string(ind+1) + ".re";
      DefineInput(temp, 2*ind);
      temp = "in" + to_string(ind+1) + ".im";
      DefineInput(temp, 2*ind+1);
    }
  }

  indexes.resize(NoOfInputs);
  for (ind=0; ind<NoOfInputs; ind++)
  {
    indexes[ind] = ind;
  }
  DefineInput("in", indexes);

  ClockGroups.AddInput2Group("input", Input("in"));
  ClockGroups.AddOutput2Group("output", Output("out"));
  ClockGroups.AddClockRelation("input", "output", NoOfInputs, 1);
  IsMultirate=true;
//  L_factor=InputsNo; M_factor=1; //set basic interpolation factor

  if (ParentClock != NULL)
  {
//    RegisterOutputClock(DSP::Clock::GetClock(ParentClock, L_factor, 1));
    RegisterOutputClock(DSP::Clock::GetClock(ParentClock, InputsNo, 1));
  }
  else
  {
    // OutputClocks[0]=DSP::Clock::GetClock(L, 1);
    DSP::log << DSP::LogMode::Error << "DSP::u::Multiplexer" << DSP::LogMode::second << "Undefined ParentClock" << endl;
    return;
  }

  CurrentOutputSampleNo = 0;
  State = new DSP::Float[NoOfInputs];
  StateReady = new bool[NoOfInputs];
  for (ind = 0; ind < NoOfInputs; ind++)
  {
  	State[ind] = 0.0;
  	StateReady[ind] = false;
  }

  Execute_ptr = &InputExecute;
  //OutputExecute_ptr = &OutputExecute;
  if (NoOfOutputs == 1)
    OutputExecute_ptr = &OutputExecute_real;
  else
    OutputExecute_ptr = &OutputExecute_cplx;
};

DSP::u::Multiplexer::~Multiplexer(void)
{
//  SetNoOfOutputs(0);
  if (State != NULL)
  {
    delete [] State;
    State = NULL;
  }
  if (StateReady != NULL)
  {
    delete [] StateReady;
    StateReady = NULL;
  }
};

#define  THIS_ ((DSP::u::Multiplexer *)block)
void DSP::u::Multiplexer::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS_->State[InputNo]=value;
  THIS_->StateReady[InputNo]=true;
  THIS_->NoOfInputsProcessed++;
};
#undef THIS_

#define  THIS_ ((DSP::u::Multiplexer *)source)
/*
bool DSP::u::Multiplexer::OutputExecute(DSP::Source_ptr source, DSP::Clock_ptr clock)
{
  // all input samples from all inputs are expected at the first output cycle
  if (THIS_->NoOfInputsProcessed == THIS_->NoOfInputs)
  {
//    OutputBlocks[0]->Execute(
//      OutputBlocks_InputNo[0], State[CurrentOutputSampleNo], this);
    THIS_->OutputBlocks[0]->Execute_ptr(
        THIS_->OutputBlocks[0],
        THIS_->OutputBlocks_InputNo[0],
        THIS_->State[THIS_->CurrentOutputSampleNo], source);
    THIS_->CurrentOutputSampleNo++;

    if (THIS_->NoOfOutputs == 2)
    {
//      OutputBlocks[1]->Execute(
//        OutputBlocks_InputNo[1], State[CurrentOutputSampleNo], this);
      THIS_->OutputBlocks[1]->Execute_ptr(
          THIS_->OutputBlocks[1],
          THIS_->OutputBlocks_InputNo[1],
          THIS_->State[THIS_->CurrentOutputSampleNo], source);
      THIS_->CurrentOutputSampleNo++;
    }

    THIS_->CurrentOutputSampleNo %= THIS_->NoOfInputs;
    if (THIS_->CurrentOutputSampleNo == 0) //All outputs processed  wait for new input cycle
      THIS_->NoOfInputsProcessed = THIS_->InitialNoOfInputsProcessed;
    return true;
  }
  // wait for rest of samples
  return false;
}
*/
bool DSP::u::Multiplexer::OutputExecute_real(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  // check if sample THIS_->CurrentOutputSampleNo is ready
	if (THIS_->StateReady[THIS_->CurrentOutputSampleNo] == true)
	{
    THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0],
        THIS_->OutputBlocks_InputNo[0],
        THIS_->State[THIS_->CurrentOutputSampleNo], source);
    THIS_->StateReady[THIS_->CurrentOutputSampleNo] = false;
    THIS_->CurrentOutputSampleNo++;

    THIS_->CurrentOutputSampleNo %= THIS_->NoOfInputs;
    if (THIS_->CurrentOutputSampleNo == 0) //All outputs processed  wait for new input cycle
      THIS_->NoOfInputsProcessed = THIS_->InitialNoOfInputsProcessed;
		return true;
	}
  // wait for rest of samples
	return false;
}

bool DSP::u::Multiplexer::OutputExecute_cplx(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  // check if sample THIS_->CurrentOutputSampleNo is ready
	if ((THIS_->StateReady[THIS_->CurrentOutputSampleNo] == true) &&
			(THIS_->StateReady[THIS_->CurrentOutputSampleNo+1] == true))
	{
    THIS_->OutputBlocks[0]->EXECUTE_PTR(
        THIS_->OutputBlocks[0],
        THIS_->OutputBlocks_InputNo[0],
        THIS_->State[THIS_->CurrentOutputSampleNo], source);
    THIS_->StateReady[THIS_->CurrentOutputSampleNo] = false;
    THIS_->CurrentOutputSampleNo++;

    THIS_->OutputBlocks[1]->EXECUTE_PTR(
        THIS_->OutputBlocks[1],
        THIS_->OutputBlocks_InputNo[1],
        THIS_->State[THIS_->CurrentOutputSampleNo], source);
    THIS_->StateReady[THIS_->CurrentOutputSampleNo] = false;
    THIS_->CurrentOutputSampleNo++;

    THIS_->CurrentOutputSampleNo %= THIS_->NoOfInputs;
    if (THIS_->CurrentOutputSampleNo == 0) //All outputs processed  wait for new input cycle
      THIS_->NoOfInputsProcessed = THIS_->InitialNoOfInputsProcessed;
		return true;
	}
  // wait for rest of samples
  return false;
}
#undef THIS_


/**************************************************/
// DCO - digitaly controled oscilator
DSP::u::DCO::DCO(DSP::Float wo, DSP::Float d_wo, DSP::Float freq_alfa, DSP::Float phase_alfa,
                   bool output_current_frequency)
  : DSP::Block()
{
  SetName("DCO", false);

  SetNoOfInputs(2, false);
  DefineInput("in.freq_err", 0);
  DefineInput("in.phase_err", 1);
  if (output_current_frequency == true)
  {
    SetNoOfOutputs(3);
    DefineOutput("freq", 2);
  }
  else
    SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  // normalisation factor 1/(2*pi) - INSTEAD of 1/pi
  // that way accumulated phase main range will be in <0, 1) (or (-1, 0>)
  // modulo pi accumulation can be implemented as:  x-floor(x) or even better as
  // x-ceil(x)

  fo= wo/DSP_M_PIx2;
  freq_factor= freq_alfa/DSP_M_PIx2;
  phase_factor= phase_alfa/DSP_M_PIx2;

  //max_freq_deviation=d_wo/M_PIx2;
  if (d_wo >= 0)
  {
    freq_dev_max =  d_wo/DSP_M_PIx2;
    freq_dev_min = -d_wo/DSP_M_PIx2;
  }
  else
  {
    freq_dev_max = 1.0;
    freq_dev_min = -1.0;
  }

  freq_memo= 0.0;
  phase_memo=0.0;

  in_freq_err = 0.0; in_phase_err = 0.0;
  current_frequ = 0.0;

  if (output_current_frequency == true)
    Execute_ptr = &InputExecute_with_Freq;
  else
    Execute_ptr = &InputExecute;
}

DSP::u::DCO::~DCO()
{
}

#define  THIS  ((DSP::u::DCO *)block)
void DSP::u::DCO::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->in_freq_err = value;
  else
    THIS->in_phase_err = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;
  //frequency error accumulation
  THIS->freq_memo += THIS->freq_factor * THIS->in_freq_err;
  THIS->freq_memo += THIS->phase_factor * THIS->in_phase_err;

  // modulo "pi" operator
  //THIS->freq_memo -= ceil(THIS->freq_memo);
  THIS->freq_memo -= FLOOR(THIS->freq_memo + DSP::Float(0.5));

  // withhold within maximum deviation range
  if (THIS->freq_memo > THIS->freq_dev_max)
    THIS->freq_memo = THIS->freq_dev_max;
  if (THIS->freq_memo < THIS->freq_dev_min)
    THIS->freq_memo = THIS->freq_dev_min;

  //phase accumulation
  THIS->phase_memo += THIS->freq_memo;
  //phase error correction component
//  phase_memo+=phase_factor*in_phase_err;
  //central frequency component
  THIS->phase_memo += THIS->fo;

  // modulo "pi" operator
  THIS->phase_memo -= CEIL(THIS->phase_memo);


//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], cos(phase_memo*M_PIx2), this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      COS(THIS->phase_memo * DSP_M_PIx2), block);
//  OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], sin(phase_memo*M_PIx2), this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      SIN(THIS->phase_memo * DSP_M_PIx2), block);
};

void DSP::u::DCO::InputExecute_with_Freq(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo==0)
    THIS->in_freq_err = value;
  else
    THIS->in_phase_err = value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

  //frequency error accumulation
  THIS->freq_memo += THIS->freq_factor * THIS->in_freq_err;
  THIS->freq_memo += THIS->phase_factor * THIS->in_phase_err;

  // modulo "pi" operator
  //THIS->freq_memo -= ceil(THIS->freq_mem);
  THIS->freq_memo -= FLOOR(THIS->freq_memo+ DSP::Float(0.5));

  // withhold within maximum deviation range
  if (THIS->freq_memo > THIS->freq_dev_max)
    THIS->freq_memo = THIS->freq_dev_max;
  if (THIS->freq_memo < THIS->freq_dev_min)
    THIS->freq_memo = THIS->freq_dev_min;

  //phase accumulation
  THIS->phase_memo += THIS->freq_memo;
  //phase error correction component
//  phase_memo+=phase_factor*in_phase_err;
  //central frequency component
  THIS->phase_memo += THIS->fo;

  // modulo "pi" operator
  THIS->phase_memo -= CEIL(THIS->phase_memo);


//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], cos(phase_memo*M_PIx2), this);
  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      COS(THIS->phase_memo * DSP_M_PIx2), block);
//  OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], sin(phase_memo*M_PIx2), this);
  THIS->OutputBlocks[1]->EXECUTE_PTR(
      THIS->OutputBlocks[1],
      THIS->OutputBlocks_InputNo[1],
      SIN(THIS->phase_memo * DSP_M_PIx2), block);

  THIS->current_frequ = THIS->freq_memo + THIS->fo;
  if (THIS->current_frequ < -0.5)
    THIS->current_frequ++;

  THIS->OutputBlocks[2]->EXECUTE_PTR(
      THIS->OutputBlocks[2],
      THIS->OutputBlocks_InputNo[2],
      THIS->current_frequ, block);
};
#undef THIS

DSP::Float DSP::u::DCO::GetFrequency(DSP::Float Fp)
{
  DSP::Float temp;

  temp=freq_memo+fo;
  while (temp<-0.5)
    temp++;
  while (temp>=0.5)
    temp--;
  return temp*Fp;
}

/**************************************************/
// CrossSwitch - sends input signals stright or crossed to outputs
/*
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
DSP::u::CrossSwitch::CrossSwitch(bool IsComplex)
  : DSP::Block()
{
  SetName("CrossSwitch", false);

  if (IsComplex  == true)
  {
    default_state = 2;
    inputs = new DSP::Float[4];

    SetNoOfInputs(1+4, false);

    DefineInput("state", 0);
    DefineInput("in1", 1, 2);
    DefineInput("in1.re", 1);
    DefineInput("in1.im", 2);
    DefineInput("in2", 3, 4);
    DefineInput("in2.re", 3);
    DefineInput("in2.im", 4);

    SetNoOfOutputs(4);
    DefineOutput("out1", 0, 1);
    DefineOutput("out1.re", 0);
    DefineOutput("out1.im", 1);
    DefineOutput("out2", 2, 3);
    DefineOutput("out2.re", 2);
    DefineOutput("out2.im", 3);
  }
  else
  {
    default_state = 0;
    inputs = new DSP::Float[2];

    SetNoOfInputs(1+2, false);

    DefineInput("state", 0);
    DefineInput("in1", 1);
    DefineInput("in1.re", 1);
    DefineInput("in2", 2);
    DefineInput("in2.re", 2);

    SetNoOfOutputs(2);
    DefineOutput("out1", 0);
    DefineOutput("out1.re", 0);
    DefineOutput("out2", 1);
    DefineOutput("out2.re", 1);
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  state = false;
  Execute_ptr = &InputExecute;
}

DSP::u::CrossSwitch::~CrossSwitch()
{
  delete [] inputs;
  inputs = NULL;
}

void DSP::u::CrossSwitch::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (InputNo == 0)
  {
    ((DSP::u::CrossSwitch *)block)->state = ((DSP::u::CrossSwitch *)block)->default_state;
    if (value > 0.0)
      ((DSP::u::CrossSwitch *)block)->state++;
  }
  else
  {
    ((DSP::u::CrossSwitch *)block)->inputs[InputNo-1] = value;
  }
  ((DSP::u::CrossSwitch *)block)->NoOfInputsProcessed++;

  if (((DSP::u::CrossSwitch *)block)->NoOfInputsProcessed < ((DSP::u::CrossSwitch *)block)->NoOfInputs)
    return;
  ((DSP::u::CrossSwitch *)block)->NoOfInputsProcessed=0;

  switch (((DSP::u::CrossSwitch *)block)->state)
  {
    case 0: // real + stright
//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], inputs[0], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[0]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[0],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[0],
          ((DSP::u::CrossSwitch *)block)->inputs[0], block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], inputs[1], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[1]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[1],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[1],
          ((DSP::u::CrossSwitch *)block)->inputs[1], block);
      break;
    case 1: // real + crossed
//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], inputs[1], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[0]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[0],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[0],
          ((DSP::u::CrossSwitch *)block)->inputs[1], block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], inputs[0], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[1]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[1],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[1],
          ((DSP::u::CrossSwitch *)block)->inputs[0], block);
      break;

    case 2: // complex + stright
//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], inputs[0], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[0]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[0],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[0],
          ((DSP::u::CrossSwitch *)block)->inputs[0], block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], inputs[1], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[1]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[1],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[1],
          ((DSP::u::CrossSwitch *)block)->inputs[1], block);

//      OutputBlocks[2]->Execute(OutputBlocks_InputNo[2], inputs[2], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[2]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[2],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[2],
          ((DSP::u::CrossSwitch *)block)->inputs[2], block);
//      OutputBlocks[3]->Execute(OutputBlocks_InputNo[3], inputs[3], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[3]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[3],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[3],
          ((DSP::u::CrossSwitch *)block)->inputs[3], block);
      break;
    case 3: // complex + crossed
//      OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], inputs[2], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[0]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[0],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[0],
          ((DSP::u::CrossSwitch *)block)->inputs[2], block);
//      OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], inputs[3], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[1]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[1],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[1],
          ((DSP::u::CrossSwitch *)block)->inputs[3], block);

//      OutputBlocks[2]->Execute(OutputBlocks_InputNo[2], inputs[0], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[2]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[2],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[2],
          ((DSP::u::CrossSwitch *)block)->inputs[0], block);
//      OutputBlocks[3]->Execute(OutputBlocks_InputNo[3], inputs[1], this);
      ((DSP::u::CrossSwitch *)block)->OutputBlocks[3]->EXECUTE_PTR(
          ((DSP::u::CrossSwitch *)block)->OutputBlocks[3],
          ((DSP::u::CrossSwitch *)block)->OutputBlocks_InputNo[3],
          ((DSP::u::CrossSwitch *)block)->inputs[1], block);
      break;

   #ifdef __DEBUG__
    default:
      DSP::log << DSP::LogMode::Error << "DSP::u::CrossSwitch::Execute" << DSP::LogMode::second
        << "Unexpected state for block: >" << ((DSP::u::CrossSwitch *)block)->GetName() << "<" << endl;
      break;
   #endif
  }
};

/**************************************************/
// Differator - first order backward difference operator
/*
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
DSP::u::Differator::Differator(int NoOfInputs_in, bool IsInputComplex)
  : DSP::Block()
{
  int ind;
  string tekst;

  SetName("Differator", false);

  if (IsInputComplex == false)
  {
    SetNoOfInputs(NoOfInputs_in, false);
    DefineInput("in", 0);
    DefineInput("in.re", 0);

    SetNoOfOutputs(NoOfInputs_in);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
    for (ind=0; ind < NoOfInputs_in; ind++)
    {
      tekst = "in" + to_string(ind+1);
      DefineInput(tekst, ind);
      tekst = "in" + to_string(ind+1) + ".re";
      DefineInput(tekst, ind);

      tekst = "out" + to_string(ind+1);
      DefineOutput(tekst, ind);
      tekst = "out" + to_string(ind+1) + ".re";
      DefineOutput(tekst, ind);
    }
  }
  else
  {
    SetNoOfInputs(NoOfInputs_in*2, false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);

    SetNoOfOutputs(NoOfInputs_in*2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    for (ind=0; ind < NoOfInputs_in; ind++)
    {
      tekst = "in" + to_string(ind+1);
      DefineInput(tekst, ind*2, ind*2+1);
      tekst = "in" + to_string(ind+1) + ".re";
      DefineInput(tekst, ind*2);
      tekst = "in" + to_string(ind+1) + ".im";
      DefineInput(tekst, ind*2+1);

      tekst = "out" + to_string(ind+1);
      DefineOutput(tekst, ind*2, ind*2+1);
      tekst = "out" + to_string(ind+1) + ".re";
      DefineOutput(tekst, ind*2);
      tekst = "out" + to_string(ind+1) + ".im";
      DefineOutput(tekst, ind*2+1);
    }
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  State.resize(NoOfInputs, 0.0);

  Execute_ptr = &InputExecute;
}

void DSP::u::Differator::SetInitialState(DSP::Float State_init)
{
  DSP::Float_vector State_init_vector(1, State_init);
  SetInitialState(State_init_vector);
}
void DSP::u::Differator::SetInitialState(DSP::Float State_init_re, DSP::Float State_init_im)
{
  DSP::Float_vector temp(2);

  temp[0] = State_init_re;
  temp[1] = State_init_im;
  SetInitialState(temp);
}
void DSP::u::Differator::SetInitialState(DSP::Complex State_init)
{
  DSP::Float_vector temp(2);

  temp[0] = State_init.re;
  temp[1] = State_init.im;
  SetInitialState(temp);
}

// Setting up internal state
void DSP::u::Differator::SetInitialState(const DSP::Float_vector &State_init)
{
  if (State_init.size() > NoOfInputs)
  {
    #ifdef __DEBUG__
    DSP::log << DSP::LogMode::Error << "DSP::u::Differator::SetInitialState" << DSP::LogMode::second
      << "ABORDING: length(" << State_init.size() << ") > size of internal state(" << NoOfInputs << ")" << endl;
    #endif
    return;
  }
  if (State_init.size() < NoOfInputs)
  {
    #ifdef __DEBUG__
    DSP::log << "DSP::u::Differator::SetInitialState" << DSP::LogMode::second
      << "length(" << State_init.size() << ") < size of internal state(" << NoOfInputs << ")" << endl;
    #endif
  }

  State = State_init;
}

DSP::u::Differator::~Differator(void)
{
  State.clear();
}

#define  THIS_ ((DSP::u::Differator *)block)
void DSP::u::Differator::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

//  OutputBlocks[InputNo]->Execute(OutputBlocks_InputNo[InputNo],
//                                 value-State[InputNo], this);
  THIS_->OutputBlocks[InputNo]->EXECUTE_PTR(
      THIS_->OutputBlocks[InputNo],
      THIS_->OutputBlocks_InputNo[InputNo],
      value - THIS_->State[InputNo], block);
  THIS_->State[InputNo]=value;
  THIS_->NoOfInputsProcessed++;

  if (THIS_->NoOfInputsProcessed < THIS_->NoOfInputs)
    return;
  THIS_->NoOfInputsProcessed = 0;
};
#undef THIS_


/**************************************************/
// Accumulator - first order operator
/*
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
DSP::u::Accumulator::Accumulator(int NoOfInputs_in, bool IsInputComplex)
  : DSP::Block()
{
  Init(NoOfInputs_in, 0.0, IsInputComplex);

  Execute_ptr = &InputExecute_classic;
}

DSP::u::Accumulator::Accumulator(DSP::Float lambda_in, int NoOfInputs_in, bool IsInputComplex)
  : DSP::Block()
{
  Init(NoOfInputs_in, lambda_in, IsInputComplex);

  Execute_ptr = &InputExecute_leakage;
}

void DSP::u::Accumulator::Init(int NoOfInputs_in, DSP::Float lambda_in, bool IsInputComplex)
{
  int ind;
  string tekst;

  SetName("Accumulator", false);

  if (IsInputComplex == false)
  {
    SetNoOfInputs(NoOfInputs_in, false);
    DefineInput("in", 0);
    DefineInput("in.re", 0);

    SetNoOfOutputs(NoOfInputs_in);
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
    for (ind=0; ind < NoOfInputs_in; ind++)
    {
      tekst = "in" + to_string(ind+1);
      DefineInput(tekst, ind);
      tekst = "in" + to_string(ind+1) + ".re";
      DefineInput(tekst, ind);

      tekst = "out" + to_string(ind+1);
      DefineOutput(tekst, ind);
      tekst = "out" + to_string(ind+1) + ".re";
      DefineOutput(tekst, ind);
    }
  }
  else
  {
    SetNoOfInputs(NoOfInputs_in*2, false);
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);

    SetNoOfOutputs(NoOfInputs_in*2);
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
    for (ind=0; ind < NoOfInputs_in; ind++)
    {
      tekst = "in" + to_string(ind+1);
      DefineInput(tekst, ind*2, ind*2+1);
      tekst = "in" + to_string(ind+1) + ".re";
      DefineInput(tekst, ind*2);
      tekst = "in" + to_string(ind+1) + ".im";
      DefineInput(tekst, ind*2+1);

      tekst = "out" + to_string(ind+1);
      DefineOutput(tekst, ind*2, ind*2+1);
      tekst = "out" + to_string(ind+1) + ".re";
      DefineOutput(tekst, ind*2);
      tekst = "out" + to_string(ind+1) + ".im";
      DefineOutput(tekst, ind*2+1);
    }
  }

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  State.clear();
  State.resize(NoOfInputs, 0.0);

  lambda = lambda_in;
  one_minus_lambda = DSP::Float(1.0) - lambda;
}

void DSP::u::Accumulator::SetInitialState(DSP::Float State_init)
{
  SetInitialState(DSP::Float_vector(1, State_init));
}
void DSP::u::Accumulator::SetInitialState(DSP::Float State_init_re, DSP::Float State_init_im)
{
  DSP::Float_vector temp(2);

  temp[0] = State_init_re;
  temp[1] = State_init_im;
  SetInitialState(temp);
}
void DSP::u::Accumulator::SetInitialState(DSP::Complex State_init)
{
  DSP::Float_vector temp(2);

  temp[0] = State_init.re;
  temp[1] = State_init.im;
  SetInitialState(temp);
}

// Setting up internal state
void DSP::u::Accumulator::SetInitialState(const DSP::Float_vector &State_init)
{
  if (State_init.size() > NoOfInputs)
  {
    #ifdef __DEBUG__
    DSP::log << DSP::LogMode::Error << "DSP::u::Accumulator::SetInitialState"<< DSP::LogMode::second
      << "ABORTING: length(" << State_init.size() << ") > size of internal state(" << NoOfInputs << ")" << endl;
    #endif
    return;
  }
  if (State_init.size() < NoOfInputs)
  {
    #ifdef __DEBUG__
    DSP::log << "DSP::u::Accumulator::SetInitialState" << DSP::LogMode::second
      << "length(" << State_init.size() << ") < size of internal state(" << NoOfInputs << ")" << endl;
    #endif
  }

  State = State_init;
}

DSP::u::Accumulator::~Accumulator(void)
{
  State.clear();
}

#define  THIS_ ((DSP::u::Accumulator *)block)
void DSP::u::Accumulator::InputExecute_classic(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS_->State[InputNo] += value;
  THIS_->OutputBlocks[InputNo]->EXECUTE_PTR(
      THIS_->OutputBlocks[InputNo], THIS_->OutputBlocks_InputNo[InputNo],
      THIS_->State[InputNo], block);
  THIS_->NoOfInputsProcessed++;

  if (THIS_->NoOfInputsProcessed < THIS_->NoOfInputs)
    return;
  THIS_->NoOfInputsProcessed = 0;
};

void DSP::u::Accumulator::InputExecute_leakage(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS_->State[InputNo] = value + THIS_->lambda * THIS_->State[InputNo];
  THIS_->OutputBlocks[InputNo]->EXECUTE_PTR(
      THIS_->OutputBlocks[InputNo], THIS_->OutputBlocks_InputNo[InputNo],
      THIS_->one_minus_lambda * THIS_->State[InputNo], block);
  THIS_->NoOfInputsProcessed++;

  if (THIS_->NoOfInputsProcessed < THIS_->NoOfInputs)
    return;
  THIS_->NoOfInputsProcessed = 0;
};
#undef THIS_

/**************************************************/
// ClockTrigger - activates given clock based on activation signal
/* Inputs and Outputs names:
 *  - Output:
 *  - Input:
 *   -# "act" - activation signal
 *
 *  \warning InputClock must (?should?)  be different from OutputClock
 */
DSP::u::ClockTrigger::ClockTrigger(DSP::Clock_ptr ParentClock, DSP::Clock_ptr SignalActivatedClock_in, int NoOfCycles_in)
  : DSP::Block()
{
  SetName("ClockTrigger", false);

  SetNoOfInputs(1, false);
  DefineInput("act", 0);
  SetNoOfOutputs(0);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
//  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  #ifdef __DEBUG__
    if (ParentClock == NULL)
    {
      DSP::log << DSP::LogMode::Error << "DSP::u::ClockTrigger::ClockTrigger" << DSP::LogMode::second << "ParentClock is NULL" << endl;
      return;
    }
    if (SignalActivatedClock_in == NULL)
    {
      DSP::log << DSP::LogMode::Error << "DSP::u::ClockTrigger::ClockTrigger" << DSP::LogMode::second << "SignalActivatedClock is NULL" << endl;
      return;
    }
    if (NoOfCycles_in <= 0)
    {
      DSP::log << "DSP::u::ClockTrigger::ClockTrigger" << DSP::LogMode::second << "NoOfCycles <= 0 - \"act\" signal determines number of cycles of activated clock" << endl;
    }
  #endif
  MasterClockIndex = ParentClock->GetMasterClockIndex();
  SignalActivatedClock = SignalActivatedClock_in;
  SignalActivatedClock_NoOfCycles = NoOfCycles_in;

  if (SignalActivatedClock_NoOfCycles < 0)
    Execute_ptr = &InputExecute_multivalue;
  else
    Execute_ptr = &InputExecute;
}

DSP::u::ClockTrigger::~ClockTrigger()
{
}

#define  THIS  ((DSP::u::ClockTrigger *)block)
void DSP::u::ClockTrigger::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (value > 0)
  {
    // InputClocks[0]
    #ifdef __DEBUG__
      if (THIS->InputClocks[0]->GetMasterClockIndex() != THIS->MasterClockIndex)
      {
        DSP::log << DSP::LogMode::Error << "DSP::u::ClockTrigger::Execute" << DSP::LogMode::second << "Declared MasterClock and actual MasterClock are different" << endl;
      }
    #endif
    DSP::Clock::AddSignalActivatedClock(
        THIS->MasterClockIndex,
        THIS->SignalActivatedClock,
        THIS->SignalActivatedClock_NoOfCycles);
  }
};

void DSP::u::ClockTrigger::InputExecute_multivalue(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (value > 0)
  {
    // InputClocks[0]
    #ifdef __DEBUG__
      if (THIS->InputClocks[0]->GetMasterClockIndex() != THIS->MasterClockIndex)
      {
        DSP::log << DSP::LogMode::Error << "DSP::u::ClockTrigger::Execute" << DSP::LogMode::second << "Declared MasterClock and actual MasterClock are different" << endl;
      }
    #endif
    DSP::Clock::AddSignalActivatedClock(
        THIS->MasterClockIndex,
        THIS->SignalActivatedClock,
        (int)(value+0.5));
  }
};
#undef THIS

// finds index of the input connected to given output of this block
/*! \bug This does not seem to be dooing what should be expected on teh basis of the description
 */
unsigned int DSP::Block::FindOutputIndex_by_InputIndex(unsigned int InputIndex)
{
  long int ind;
  unsigned int ind2;
	DSP::Component_ptr temp;

  for (ind = 0; ind < NoOfComponentsInTable; ind++)
  {
    temp = ComponentsTable[ind];
    for (ind2 = 0; ind2 < temp->NoOfOutputs; ind2++)
    {
      if (this == temp->OutputBlocks[ind2])
      	if (InputIndex == temp->OutputBlocks_InputNo[ind2])
      	{
      		if (temp->NoOfOutputs == 1)
      			return DSP::c::FO_TheOnlyOutput;
      		return ind2;
      	}
    }
  }
  return DSP::c::FO_NoOutput;
}

/* ************************************************ */
/* ************************************************ */
/* ************************************************ */
// List all registered components
/*!
 * \warning This function works only in DEBUG mode
 */
void DSP::Component::ListComponents(void)
{
  #ifdef __DEBUG__
    for (int ind = 0; ind < NoOfComponentsInTable; ind++)
    {
      DSP::log << ComponentsTable[ind]->GetName() << endl;
    }
  #endif
}

// List all registered components
/*! \warning This function works only in DEBUG mode
 */
void DSP::Component::ListComponents(DSP::Clock_ptr InputClock)
{
  UNUSED_RELEASE_ARGUMENT(InputClock);

  #ifdef __DEBUG__
    DSP::Block_ptr temp;

    for (long int ind = 0; ind < NoOfComponentsInTable; ind++)
    {
      if (ComponentsTable[ind]->Convert2Source() == NULL)
      {
        temp = ComponentsTable[ind]->Convert2Block();
        if (temp != &(DSP::Block::DummyBlock))
        {
          if (ComponentsTable[ind]->Convert2Block()->IsUsingConstants)
          {
            //! \bug there might be problem if all inputs are constant (but his would mean that this component shloud not work properly)
            for (unsigned int ind2 = 0; ind2 < ComponentsTable[ind]->Convert2Block()->NoOfInputs; ind2++)
            {
              if (ComponentsTable[ind]->Convert2Block()->IsConstantInput[ind2] == false)
              {
                if (ComponentsTable[ind]->Convert2Block()->InputClocks[ind2] == InputClock)
                  DSP::log << "  >>block" << DSP::LogMode::second << ComponentsTable[ind]->GetName() << endl;
                break;
              }
            }
          }
          else
          {
            //! \bug problem if inputs work with different clocks
            if (ComponentsTable[ind]->Convert2Block()->InputClocks[0] == InputClock)
              DSP::log << "  >>block" << DSP::LogMode::second << ComponentsTable[ind]->GetName() << endl;
          }
        }
      }
    }
  #endif
}

void DSP::Component::ListOfAllComponents(bool list_outputs)
{
  UNUSED_RELEASE_ARGUMENT(list_outputs);

  #ifdef __DEBUG__
    DSP::Block_ptr temp;
    string tekst;

    DSP::log << "  Number of components" << DSP::LogMode::second << (NoOfComponentsInTable - 1) << endl;

    for (int ind = 0; ind < NoOfComponentsInTable; ind++)
    {
      if (ComponentsTable[ind]->Convert2Source() == NULL)
      {
        temp = ComponentsTable[ind]->Convert2Block();
        if (temp != &(DSP::Block::DummyBlock))
        {
          if (temp->IsAutoSplit == true)
          {
            DSP::log << setw(3) << ind << ":  >>auto(inputs=" << temp->NoOfInputs << ",outputs=" << temp->NoOfOutputs << ")"
              << DSP::LogMode::second << temp->GetName() << endl;
          }
          else
          {
            if (temp->Convert2Copy() != NULL)
            {
              DSP::log << setw(3) << ind << ":  >>copy(inputs=" << temp->NoOfInputs << ",outputs=" << temp->NoOfOutputs << ")"
                << DSP::LogMode::second << temp->GetName() << endl;
            }
            else
            {
              DSP::log << setw(3) << ind << ":  >>block(inputs=" << temp->NoOfInputs << ",outputs=" << temp->NoOfOutputs << ")"
                << DSP::LogMode::second << temp->GetName() << endl;
            }
          }
          {
            stringstream ss;
            if (temp->InputClocks[0] == NULL)
            {
              ss << "<" << static_cast<void*>(ComponentsTable[ind]) << "> = " << static_cast<void*>(temp->InputClocks[0])
                  << "; Master Clock index = ?";
            }
            else
            {
              ss << "<" << static_cast<void*>(ComponentsTable[ind]) << "> = " << static_cast<void*>(temp->InputClocks[0])
                  << "; Master Clock index = " << (int)(temp->InputClocks[0]->GetMasterClockIndex());
            }
            DSP::log << "    InputClocks[0]" << DSP::LogMode::second << ss.str() << endl;
          }
        }
      }
      else
        if (ComponentsTable[ind]->Convert2Block() == NULL)
        {
          {
            DSP::log << setw(3) << ind << ":  >>source(outputs=" << ComponentsTable[ind]->Convert2Source()->NoOfOutputs << ")"
              << DSP::LogMode::second << ComponentsTable[ind]->GetName() << endl;
          }
          {
            stringstream ss;
            if (ComponentsTable[ind]->Convert2Source()->OutputClocks[0] == NULL)
            {
              ss << " = " << static_cast<void*>(ComponentsTable[ind]->Convert2Source()->OutputClocks[0])
                 << "; Master Clock index = ?";
            }
            else
            {
              ss << " = " << static_cast<void*>(ComponentsTable[ind]->Convert2Source()->OutputClocks[0])
                 << "; Master Clock index = " << (int)(ComponentsTable[ind]->Convert2Source()->OutputClocks[0]->GetMasterClockIndex());
            }
            DSP::log << "    OutputClocks[0]" << DSP::LogMode::second << ss.str() << endl;
          }
        }
        else
        {
          {
            temp = ComponentsTable[ind]->Convert2Block();

            DSP::log << setw(3) << ind << ":  >>mixed(inputs=" << temp->NoOfInputs << ",outputs=" << temp->NoOfOutputs << ")"
              << DSP::LogMode::second << temp->GetName() << endl;
          }
          if (temp->NoOfInputs > 0)
          {
            if (temp->InputClocks[0] == NULL)
            {
              stringstream ss;
              ss << " = " << static_cast<void*>(temp->InputClocks[0])
                  << "; Master Clock index = ?";
              tekst = ss.str();
            }
            else
            {
              stringstream ss;
              ss << " = " << static_cast<void*>(temp->InputClocks[0])
                  << "; Master Clock index = " << (int)(temp->InputClocks[0]->GetMasterClockIndex());
              tekst = ss.str();
            }
            DSP::log << "    InputClocks[0]" << DSP::LogMode::second << tekst << endl;
          }
          if (temp->NoOfOutputs > 0) {
            stringstream ss;
            if (ComponentsTable[ind]->Convert2Source()->OutputClocks[0] == NULL)
            {
              ss << "<" << static_cast<void*>(ComponentsTable[ind]) << "> = " << static_cast<void*>(ComponentsTable[ind]->Convert2Source()->OutputClocks[0])
                  << "; Master Clock index = ?";
            }
            else
            {
              ss << "<" << static_cast<void*>(ComponentsTable[ind]) << "> = " << static_cast<void*>(ComponentsTable[ind]->Convert2Source()->OutputClocks[0])
                  << "; Master Clock index = " << (int)((int)(ComponentsTable[ind]->Convert2Source()->OutputClocks[0]->GetMasterClockIndex()));
            }
            DSP::log << "    OutputClocks[0]" << DSP::LogMode::second << ss.str() << endl;
          }
          else {
            DSP::log << "    OutputClocks[0]" << DSP::LogMode::second << "undefined (NoOfOutputs == 0)!" << endl;
          }
        }

      if (list_outputs == true)
      {
        for (unsigned int ind2 = 0; ind2 < ComponentsTable[ind]->NoOfOutputs; ind2++)
        {
          DSP::log  << "out" << setw(3) << ind2 << " ==>  <" << static_cast<void*>(ComponentsTable[ind]->OutputBlocks[ind2])
            << ">[" << setw(3) << ComponentsTable[ind]->OutputBlocks_InputNo[ind2] << "]"
            << DSP::LogMode::second
            << ComponentsTable[ind]->OutputBlocks[ind2]->GetName() << endl;
        }
      }
    }
    if (NoOfComponentsInTable > 1)
      DSP::log << endl;
  #endif
}

void DSP::Component::CheckInputsOfAllComponents(void)
{
  #ifdef __DEBUG__
    DSP::Block_ptr temp;
    DSP::Component_ptr tempBlock;
    unsigned int tempNo;
    unsigned int NoOfCopyBlocks;
    unsigned int NoOfAutoSplitBlocks;

    DSP::log << "DSP::Component::CheckInputsOfAllComponents" << DSP::LogMode::second << "Start" << endl;

    NoOfCopyBlocks = 0; NoOfAutoSplitBlocks = 0;
    for (int ind = 0; ind < NoOfComponentsInTable; ind++)
    {
      if (ComponentsTable[ind]->Convert2Copy() != NULL)
        NoOfCopyBlocks++;
      if (ComponentsTable[ind]->IsAutoSplit == true)
        NoOfAutoSplitBlocks++;

      temp = ComponentsTable[ind]->Convert2Block();
      if (temp != NULL)
      {
        if (temp->NoOfInputs > 0)
        {
          if ((temp->NoOfInputsConnected + temp->InitialNoOfInputsProcessed) != temp->NoOfInputs)
          {
            DSP::log << "  >>block (" << ComponentsTable[ind]->GetName() << "): NoOfInputsConnected + InitialNoOfInputsProcessed != NoOfInputs" << endl;
            DSP::log << "    NoOfInputs = " << temp->NoOfInputs << endl;
            DSP::log << "    NoOfInputsConnected = " << temp->NoOfInputsConnected << endl;
            DSP::log << DSP::LogMode::Error << "    InitialNoOfInputsProcessed = " << temp->InitialNoOfInputsProcessed << endl;
          }
        }

        for (unsigned int ind2=0; ind2<temp->NoOfInputs; ind2++)
        {
          if (IsOutputConnectedToThisInput(temp, ind2, tempBlock, tempNo) == false)
          {
            DSP::log << DSP::LogMode::Error << "   Block >>" << temp->GetName() << "<< input "
                  << setw(2) << setfill('0') << ind2 << " UNCONNECTED" << endl;
          }
          else
          {
            if (tempBlock != NULL)
            { // non constant input
              if ((temp->Convert2Copy() == NULL) && (tempBlock->Convert2Copy() != NULL))
              { // Copy ==> Copy chains are ignored
                DSP::log << DSP::LogMode::Error << "   Block >>" << temp->GetName()
                      << "<< input " << setw(2) << setfill('0') << ind2
                      << " connected only to DSP::u::Copy >>" << tempBlock->GetName()
                      << "<< input " << setw(2) << setfill('0') << tempNo << endl;
              }
            }
          }
        }

      }
    }


    DSP::log << "  Number of checked components" << DSP::LogMode::second
      << (NoOfComponentsInTable - 1) << " (" << NoOfCopyBlocks
      << " copy blocks & " << NoOfAutoSplitBlocks << " autosplitters)" << endl;

    if (NoOfComponentsInTable > 1)
      DSP::log << endl;

    DSP::log << "DSP::Component::CheckInputsOfAllComponents" << DSP::LogMode::second << "End" << endl;

  #endif
}


/**************************************************/
// Copies inputs to output
DSP::u::Copy::Copy(unsigned int NoOfInputs_in)
  : DSP::Block()
{
  vector <unsigned int> indexes;
  unsigned int ind;

  SetName("Copy", false);
  Type = DSP_CT_copy; // overide default type

  SetNoOfOutputs(NoOfInputs_in);
  SetNoOfInputs(NoOfInputs_in, false);

  InputBlocks = new DSP::Component_ptr[NoOfInputs];
  InputBlocks_OutputNo = new unsigned int[NoOfInputs];
  for (ind = 0; ind < NoOfInputs; ind++)
  {
    InputBlocks[ind] = NULL;
    InputBlocks_OutputNo[ind] = DSP::c::FO_NoInput;
  }

  indexes.resize(NoOfInputs);
  for (ind=0; ind<NoOfInputs; ind++)
    indexes[ind] = ind;
  DefineInput("in", indexes);
  DefineOutput("out", indexes);

  //Execute_ptr = &InputExecute;
};


DSP::u::Copy::~Copy(void)
{
  SetNoOfOutputs(0);
  SetNoOfInputs(0, false);
  if (InputBlocks != NULL)
  {
    delete [] InputBlocks;
    InputBlocks = NULL;
  }
  if (InputBlocks_OutputNo != NULL)
  {
    delete [] InputBlocks_OutputNo;
    InputBlocks_OutputNo = NULL;
  }
};

// DSP::u::Copy output info update
/* DSP::u::Copy component's output with index OutputNo must be
 *  connected to block's input with index block_InputNo.
 *
 *  DSP::u::Copy must store this info and use it to
 *  help DSP::Component::DSP::_connect_class::connect directly connect
 *  block which user connects through DSP::u::copy component.
 *
 *  Returns false if input block is still unknown.
 *
 *  \note this Copy block is inside copys' chain then set output of the last copy in the chain
 *  \note if DSP::u::Copy is being connected, put it into copys' chain
 */
bool DSP::u::Copy::SetCopyOutput(unsigned int OutputNo, DSP::Block_ptr block, unsigned int block_InputNo)
{
  DSP::u::Copy *current;
  unsigned int current_OutputNo;
  DSP::Component_ptr temp_current;
  unsigned int temp_current_OutputNo;

  if (OutputNo < NoOfOutputs)
  {
    temp_current = this;
    temp_current_OutputNo = OutputNo;

    // check if it is the copys' chain and find last copy in the chain
    do
    {
      current = temp_current->Convert2Copy();
      current_OutputNo = temp_current_OutputNo;

      current->GetOutput(current_OutputNo, temp_current, temp_current_OutputNo);
    }
    while (temp_current->Convert2Copy() != NULL);


    if (block->Convert2Copy() != NULL)
    {
      /*! \bug insert copy (with its copy chain) exactly where it was meant to be
       *   only its output block move to the end of the resulting copy chain
       */

      // if DSP::u::Copy is being connected, put it into copys' chain
      // check which copy chain has actual output block
      if (temp_current == &DSP::Component::DummyBlock)
      { // attach new copy chain to this chain
        current->OutputBlocks[current_OutputNo] = block;
        current->OutputBlocks_InputNo[current_OutputNo] = block_InputNo;
      }
      else
      { // attach this chain to new copy chain
        //current->OutputBlocks[current_OutputNo] = temp_current->Convert2Block(); ? not needed
        //current->OutputBlocks_InputNo[current_OutputNo] = temp_current_OutputNo; ? not needed

        block->Convert2Copy()->InputBlocks[block_InputNo] = current;
        block->Convert2Copy()->InputBlocks_OutputNo[block_InputNo] = current_OutputNo;
      }
    }
    else
    {
      // else just connect
      current->OutputBlocks[current_OutputNo]=block;
      current->OutputBlocks_InputNo[current_OutputNo]=block_InputNo;
    }

    //! \bug will fail in copy chain
  	if (InputBlocks[OutputNo] != NULL)
      return true;
  }

	return false;
}

//! Raw output block reading function
bool DSP::u::Copy::GetOutput(unsigned int OutputNo, DSP::Component_ptr &output_block, unsigned int &output_block_InputNo)
{
  output_block = NULL;
  output_block_InputNo = DSP::c::FO_NoInput;

  if (OutputNo < NoOfOutputs)
  {
    output_block = OutputBlocks[OutputNo];
    output_block_InputNo = OutputBlocks_InputNo[OutputNo];
    return true;
  }
  return false;
}

// Returns block and its input number to which is connected given DSP::u::Copy block output
/* If the requested data is no available function returns:
 *  - output_block = NULL
 *  - output_block_InputNo = FO_NoInput
 *  .
 *  \note For copy blocks chains returns output of the last copy in chain.
 *
 *  Returns true on success.
 */
bool DSP::u::Copy::GetCopyOutput(unsigned int OutputNo, DSP::Block_ptr &output_block, unsigned int &output_block_InputNo)
{
  output_block = NULL;
  output_block_InputNo = DSP::c::FO_NoInput;

  if (OutputNo < NoOfOutputs)
  {
    if (OutputBlocks[OutputNo] != &DummyBlock)
    {
      output_block = OutputBlocks[OutputNo];
      output_block_InputNo = OutputBlocks_InputNo[OutputNo];

      if (output_block->Convert2Copy() != NULL)
        return output_block->Convert2Copy()->GetCopyOutput(output_block_InputNo, output_block, output_block_InputNo);
      return true;
    }
  }

  return false;
}

// DSP::u::Copy input info update
/* DSP::u::Copy component's input with index InputNo must be
 *  connected to block's output with index block_OutputNo.
 *
 *  DSP::u::Copy must store this info and use it to
 *  help DSP::Component::DSP::_connect_class::connect directly connect
 *  block which user connects through DSP::u::copy component.
 *
 *  Returns false if output block is still unknown.
 *
 *  \note If block is DSP::u::Copy then, sets Copy input to the last Copy in the DSP::u::Copy chain instead
 *  \note If this Copy input points to another Copy then instead set input of the first Copy in the DSP::u::Copy chain
 */
bool DSP::u::Copy::SetCopyInput(unsigned int InputNo, DSP::Component_ptr block, unsigned int block_OutputNo)
{
  DSP::u::Copy *current;
  unsigned int current_InputNo;
  DSP::Component_ptr destination;
  unsigned int destination_OutputNo;
  DSP::Component_ptr temp_destination;
  unsigned int temp_destination_OutputNo;

  if (InputNo < NoOfInputs)
  {
    // Find first copy in the chain in which is this Copy
    current = this;
    current_InputNo = InputNo;
    while (current->InputBlocks[current_InputNo] != NULL)
    {
      if (current->InputBlocks[current_InputNo]->Convert2Copy() != NULL)
      {
        current_InputNo = current->InputBlocks_OutputNo[current_InputNo];
        current = current->InputBlocks[current_InputNo]->Convert2Copy();
      }
      else
        break;
    }

    temp_destination = block;
    temp_destination_OutputNo = block_OutputNo;
    // if block is Copy then find last copy from destination Copy chain if it exists
    if (temp_destination->Convert2Copy() == NULL)
    {
      destination = temp_destination;
      destination_OutputNo = temp_destination_OutputNo;
    }
    else
    {
      do
      {
        destination = temp_destination;
        destination_OutputNo = temp_destination_OutputNo;

        //destination_OutputNo = destination->OutputBlocks_InputNo[destination_OutputNo];
        //destination = destination->OutputBlocks[destination_OutputNo];
        destination->Convert2Copy()->GetOutput(destination_OutputNo, temp_destination, temp_destination_OutputNo);
      }
      while (temp_destination->Convert2Copy() != NULL);
    }

    // simulate connected inputs for DSP::u::copy (but only if it is information not update)
    //! \todo issue warning when updating information
    if (current->InputBlocks[current_InputNo] == NULL)
      current->NoOfInputsConnected++;

    current->InputBlocks[current_InputNo]=destination;
    current->InputBlocks_OutputNo[current_InputNo]=destination_OutputNo;

#ifdef __DEBUG__
    if (current->NoOfInputsConnected > current->NoOfInputs)
      DSP::log << DSP::LogMode::Error << "DSP::u::Copy::SetCopyInput" << DSP::LogMode::second << "NoOfInputsConnected > NoOfInputs" << endl;
#endif

    // return true if there is output block connected already
    DSP::Block_ptr temp;
    unsigned int tempNo;
    return GetCopyOutput(current_InputNo, temp, tempNo);
  }

  return false;
}

// Returns component and its output number connected to given DSP::u::Copy block input
/* If the requested data is no available function returns false and
 *  - input_block = NULL
 *  - input_block_OutputNo = FO_NoOutput
 *  .
 */
bool DSP::u::Copy::GetCopyInput(unsigned int InputNo, DSP::Component_ptr &input_block, unsigned int &input_block_OutputNo)
{
  input_block = NULL;
  input_block_OutputNo = DSP::c::FO_NoOutput;

  if (InputNo < NoOfInputs)
  {
    if (InputBlocks[InputNo] != NULL)
    {
      input_block = InputBlocks[InputNo];
      input_block_OutputNo = InputBlocks_OutputNo[InputNo];

      if (input_block->Convert2Copy() != NULL)
        return input_block->Convert2Copy()->GetCopyInput(input_block_OutputNo, input_block, input_block_OutputNo);
      return true;
    }
  }

  return false;
}

/*
#define  THIS  ((DSP::u::Copy *)block)
void DSP::u::Copy::InputExecute(INPUT_EXECUTE_ARGS)
{
  THIS->OutputBlocks[InputNo]->Execute_ptr(
        THIS->OutputBlocks[InputNo], THIS->OutputBlocks_InputNo[InputNo],
        value, block);
};
#undef THIS
*/


DSP::Macro_ptr *DSP::MacroStack::Stack = NULL;
unsigned int   DSP::MacroStack::Length = 0;
DSP::Macro_ptr *DSP::MacroStack::List = NULL;
unsigned int   DSP::MacroStack::ListLength = 0;

// adds this macro at the bottom of CurrentMacroStack table
void DSP::MacroStack::AddMacroToStack(DSP::Macro_ptr macro)
{
  DSP::Macro_ptr *temp_stack;

  temp_stack = Stack;
  Stack = new DSP::Macro_ptr[Length+1];
  if (temp_stack != NULL)
    memcpy(Stack, temp_stack, Length*sizeof(DSP::Macro_ptr));
  Stack[Length] = macro;
  Length++;

  if (temp_stack != NULL)
    delete [] temp_stack;
}
// removes this macro from CurrentMacroStack table
/* macro is expected at the bottom of the stack
 */
void DSP::MacroStack::RemoveMacroFromStack(DSP::Macro_ptr macro)
{
  DSP::Macro_ptr *temp_stack;

  // check if there is any macro on the stack
  if (Length == 0)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::MacroStack::RemoveMacroFromStack" << DSP::LogMode::second
        << "Stack empty. No macro:>>" << macro->GetName() << "<< on the list!" << endl;
    #endif
    return;
  }

  // check if the macro is ta the bottom of the stack
  if (Stack[Length-1] == macro)
  {
    Length--;
    if (Length == 0)
    {
      delete [] Stack;
      Stack = NULL;
    }
    else
    {
      temp_stack = new DSP::Macro_ptr[Length];
      memcpy(temp_stack, Stack, Length*sizeof(DSP::Macro_ptr));
      Stack = temp_stack;
    }
  }
  #ifdef __DEBUG__
  else
  {
    DSP::log << "DSP::MacroStack::RemoveMacroFromStack" << DSP::LogMode::second << "Current macro stack:" << endl;
    for (unsigned int ind = 0; ind < Length; ind++)
    {
      DSP::log << "  >>" << DSP::LogMode::second << Stack[ind]->GetName() << endl;
    }
    DSP::log << endl;

    DSP::log << DSP::LogMode::Error << "DSP::MacroStack::RemoveMacroFromStack" << DSP::LogMode::second
      << "Macro:>>" << macro->GetName() << "<< not found at the bottom of the stack!" << endl;
  }
  #endif
}

// Returns current macro stack
unsigned int DSP::MacroStack::GetCurrentMacroStack(DSP::Macro_ptr *&MacrosStack)
{
  MacrosStack = new DSP::Macro_ptr[Length];
  memcpy(MacrosStack, Stack, Length*sizeof(DSP::Macro_ptr));

  return Length;
}

unsigned int DSP::MacroStack::GetCurrentMacroList(vector<DSP::Macro_ptr> &MacrosList)
{
  MacrosList.resize(ListLength);
  // \TODO convert List into vector and copy vectors instead of using memcpy
  memcpy(MacrosList.data(), List, ListLength*sizeof(DSP::Macro_ptr));

  return ListLength;
}

// adds this macro at the bottom of List table
void DSP::MacroStack::AddMacroToList(DSP::Macro_ptr macro)
{
  DSP::Macro_ptr *temp_list;

  temp_list = List;
  List = new DSP::Macro_ptr[ListLength+1];
  if (temp_list != NULL)
    memcpy(List, temp_list, ListLength*sizeof(DSP::Macro_ptr));
  List[ListLength] = macro;
  ListLength++;

  if (temp_list != NULL)
    delete [] temp_list;
}

void DSP::MacroStack::RemoveMacroFromList(DSP::Macro_ptr macro)
{
  DSP::Macro_ptr *temp_list;
  unsigned int ind;

  // check if there is any macro on the list
  if (ListLength == 0)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::MacroStack::RemoveMacroFromList" << DSP::LogMode::second
        << "List empty. No macro:>>" << macro->GetName() << "<< on the list!" << endl;
    #endif
    return;
  }

  // check macro position on the list
  for (ind = 0; ind < ListLength; ind++)
  {
    if (List[ind] == macro)
    {
      ListLength--;
      if (ListLength == 0)
      {
        delete [] List;
        List = NULL;
      }
      else
      {
        temp_list = new DSP::Macro_ptr[ListLength];
        if (ind > 0)
          memcpy(temp_list, List, ind*sizeof(DSP::Macro_ptr));
        if (ListLength-ind > 0)
          memcpy(temp_list+ind, List+(ind+1), (ListLength-ind)*sizeof(DSP::Macro_ptr));
        delete [] List;
        List = temp_list;
      }
      return;
    }
  }

#ifdef __DEBUG__
  if (ind >= ListLength)
  {
    DSP::log << "DSP::MacroStack::RemoveMacroFromList" << DSP::LogMode::second << "Current macro list:" << endl;
    for (ind = 0; ind < ListLength; ind++)
    {
      DSP::log << "  >>" << DSP::LogMode::second << List[ind]->GetName() << endl;
    }
    DSP::log << endl;

    DSP::log << DSP::LogMode::Error << "DSP::MacroStack::RemoveMacroFromList" << DSP::LogMode::second
      << "Macro:>>" << macro->GetName() << "<< not found on the list!" << endl;
  }
#endif
}


bool DSP::Macro::DefineInput(const string &Name, const unsigned int &InputNo)
{
  if (MacroInput_block != NULL)
  {
    bool rs;

    rs = MacroInput_block->DefineInput(Name, InputNo);
    rs = (rs && MacroInput_block->DefineOutput(Name, InputNo));
    return rs;
  }
  return false;
}
bool DSP::Macro::DefineInput(const string &Name, const unsigned int &InputNo_re, const unsigned int &InputNo_im)
{
  if (MacroInput_block != NULL)
  {
    bool rs;

    rs = MacroInput_block->DefineInput(Name, InputNo_re, InputNo_im);
    rs = (rs && MacroInput_block->DefineOutput(Name, InputNo_re, InputNo_im));
    return rs;
  }
  return false;
}
bool DSP::Macro::DefineInput(const string &Name, const vector<unsigned int> &Inputs)
{
  if (MacroInput_block != NULL)
  {
    bool rs;

    rs = MacroInput_block->DefineInput(Name, Inputs);
    rs = (rs && MacroInput_block->DefineOutput(Name, Inputs));
    return rs;
  }
  return false;
}
bool DSP::Macro::UndefineInput(const string &Name)
{
  if (MacroInput_block != NULL)
  {
    bool rs;

    rs = MacroInput_block->UndefineInput(Name);
    rs = (rs && MacroInput_block->UndefineOutput(Name));
    return rs;
  }
  return false;
}


bool DSP::Macro::DefineOutput(const string &Name, const unsigned int &OutputNo)
{
  if (MacroOutput_block != NULL)
  {
    bool rs;

    rs = MacroOutput_block->DefineInput(Name, OutputNo);
    rs = (rs && MacroOutput_block->DefineOutput(Name, OutputNo));
    return rs;
  }
  return false;
}
bool DSP::Macro::DefineOutput(const string &Name, const unsigned int &OutputNo_re, const unsigned int &OutputNo_im)
{
  if (MacroOutput_block != NULL)
  {
    bool rs;

    rs = MacroOutput_block->DefineInput(Name, OutputNo_re, OutputNo_im);
    rs = (rs && MacroOutput_block->DefineOutput(Name, OutputNo_re, OutputNo_im));
    return rs;
  }
  return false;
}
bool DSP::Macro::DefineOutput(const string &Name, vector<unsigned int> &Outputs)
{
  if (MacroOutput_block != NULL)
  {
    bool rs;

    rs = MacroOutput_block->DefineInput(Name, Outputs);
    rs = (rs && MacroOutput_block->DefineOutput(Name, Outputs));
    return rs;
  }
  return false;
}
bool DSP::Macro::UndefineOutput(const string &Name)
{
  if (MacroOutput_block != NULL)
  {
    bool rs;

    rs = MacroOutput_block->UndefineInput(Name);
    rs = (rs && MacroOutput_block->UndefineOutput(Name));
    return rs;
  }
  return false;
}


// returns internal output of the macro input of the given name
DSP::output &DSP::Macro::MacroInput(const string &Name)
{
  return MacroInput_block->Output(Name);
}
// returns internal input of the macro output of the given name
DSP::input &DSP::Macro::MacroOutput(const string &Name)
{
  return MacroOutput_block->Input(Name);
}

// returns external macro output of the given name
DSP::output &DSP::Macro::Output(const string &Name)
{
  return MacroOutput_block->Output(Name); // external outputs
}
// returns external macro input of the given name
DSP::input &DSP::Macro::Input(const string &Name)
{
  return MacroInput_block->Input(Name); // external inputs
}

// Returns macro input line number connected to the given macro block input
/* Returns FO_NoInput if not connected to the macro input
 */
unsigned int DSP::Macro::GetMacroInputNo(DSP::Component_ptr output_block, unsigned int output_block_input_no)
{
  unsigned int ind, ind2;
  DSP::Block_ptr macro_input_block;  unsigned int macro_input_block_InputNo;
  DSP::Component_ptr tempBlock; unsigned int tempNo;

  // 0. Check if its input copy or DummyBlock
  if ( (output_block->Convert2Copy() == MacroInput_block) ||
       (output_block == &DSP::Component::DummyBlock) )
    return output_block_input_no;

  if (output_block->Convert2Copy() != NULL)
  {
    DSP::log << DSP::LogMode::Error << "DSP::Macro::GetMacroInputNo" << DSP::LogMode::second << "Encountered unrecognized copy block" << endl;
  }

  for (ind = 0; ind < NoOfInputs; ind++)
  {
    // 1. check if the macro input points at it directly
    MacroInput_block->GetCopyOutput(ind, macro_input_block, macro_input_block_InputNo);

    if (macro_input_block != NULL)
    { // macro output not pointing at DummyBlock
      if ( (macro_input_block == output_block) &&
           (macro_input_block_InputNo == output_block_input_no))
        return ind;

      if (macro_input_block->IsAutoSplit == true)
      {
        // 2. check if the macro input points at through autosplitter
        for (ind2 = 0; ind2 < macro_input_block->NoOfOutputs; ind2++)
        {
          if ( (macro_input_block->OutputBlocks[ind2] == output_block) &&
              (macro_input_block->OutputBlocks_InputNo[ind2] == output_block_input_no))
            return ind;
        }
      }

      // 3. check if it is not autosplitter block pointing at block inside the macro
      if (DSP::Component::IsOutputConnectedToThisInput2(macro_input_block, macro_input_block_InputNo, tempBlock, tempNo) == true)
        if ( (tempBlock->Convert2Block() == output_block) &&
            (tempNo == output_block_input_no))
          return ind;
    }
  }


  return DSP::c::FO_NoInput;
}

//! Returns clock assigned to macro external output number OutputNo
DSP::Clock_ptr DSP::Macro::GetOutputClock(unsigned int OutputNo)
{
  return MacroOutput_block->GetOutputClock(OutputNo);
}

void DSP::Macro::MacroInitStarted(void)
{
  vector <unsigned int> indexes;
  string temp;

  DSP::MacroStack::AddMacroToStack(this);

  MacroInput_block = new DSP::u::Copy(NoOfInputs);
  MacroInput_block->SetName(GetName(), false);
  MacroInput_block->SetName("Input");

  MacroOutput_block = new DSP::u::Copy(NoOfOutputs);
  MacroOutput_block->SetName(GetName(), false);
  MacroOutput_block->SetName("Output");


  // +++++++++++++++++++++++++++++++++ //
  UndefineInput();
  UndefineOutput();

  if (NoOfInputs >= 1)
  {
    DefineInput("in.re", 0);
    if (NoOfInputs >= 2)
      DefineInput("in.im", 1);

    indexes.resize(NoOfInputs);
    for (unsigned int ind=0; ind<NoOfInputs; ind++)
    {
      temp = "in" + to_string(ind+1);
      DefineInput(temp, ind);

      indexes[ind] = ind;
    }
    DefineInput("in", indexes);
  }

  if (NoOfOutputs >= 1)
  {
    DefineOutput("out.re", 0);
    if (NoOfOutputs >= 2)
    {
      DefineOutput("out.im", 1);
    }

    indexes.resize(NoOfOutputs);
    for (unsigned int ind=0; ind<NoOfOutputs; ind++)
    {
      temp = "ou" + to_string(ind+1);
      DefineOutput(temp, ind);

      indexes[ind] = ind;
    }
    DefineOutput("out", indexes);
  }
  // +++++++++++++++++++++++++++++++++ //

  MacroInitializationOn = true;
}
void DSP::Macro::MacroInitFinished(void)
{
  MacroInitializationOn = false;

  DSP::MacroStack::RemoveMacroFromStack(this);
}

// Creates macro component basic container
/* Creates MacroInput and MacroOutput objects
 *  with given input and output lines number.
 */
DSP::Macro::Macro(const string &macro_name,
          unsigned int NoOfInputs_in, unsigned int NoOfOutputs_in)
  : DSP::name(macro_name)
{
#ifdef __DEBUG__
  DOTmode = DSP_DOT_macro_wrap;
#endif
  SetName(macro_name, false);

  MacroInitializationOn = false;
  MacroInput_block = NULL;
  MacroOutput_block = NULL;

  DSP::MacroStack::AddMacroToList(this);

  NoOfInputs = NoOfInputs_in;
  NoOfOutputs = NoOfOutputs_in;
}

// Macro container clean up
/* Destroys MacroInput and MacroOutput objects.
 */
DSP::Macro::~Macro(void)
{
  if (MacroInput_block != NULL)
  {
    delete MacroInput_block;
    MacroInput_block = NULL;
  }
  if (MacroOutput_block != NULL)
  {
    delete MacroOutput_block;
    MacroOutput_block = NULL;
  }

  DSP::MacroStack::RemoveMacroFromList(this);
}

/**************************************************/
// Block calculating absolute value of real or complex sample
DSP::u::Quantizer::Quantizer(DSP::Float threshold, DSP::Float L_value, DSP::Float U_value)
  : DSP::Block()
{
  SetName("Quantizer", false);

  SetNoOfInputs(1, false);
  DefineInput("in", 0);
  SetNoOfOutputs(1);
  DefineOutput("out", 0);

  ClockGroups.AddInputs2Group("all", 0, NoOfInputs-1);
  ClockGroups.AddOutputs2Group("all", 0, NoOfOutputs-1);

  B = 1;
  thresholds.resize(B);
  thresholds[0] = threshold;
  q_levels.resize(1 << B);
  q_levels[0] = L_value; q_levels[1] = U_value;

  output_val = 0.0;

  Execute_ptr = &InputExecute_1bit;
}

DSP::u::Quantizer::~Quantizer(void)
{
  thresholds.clear();
  q_levels.clear();
}

#define THIS ((DSP::u::Quantizer *)block)
void DSP::u::Quantizer::InputExecute_1bit(INPUT_EXECUTE_ARGS)
{
  UNUSED_ARGUMENT(InputNo);
  UNUSED_DEBUG_ARGUMENT(Caller);

  THIS->output_val = (value <= THIS->thresholds[0]) ? THIS->q_levels[0] : THIS->q_levels[1];
  /*
  THIS->NoOfInputsProcessed++;
  if THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed=0;
  */

  THIS->OutputBlocks[0]->EXECUTE_PTR(
      THIS->OutputBlocks[0],
      THIS->OutputBlocks_InputNo[0],
      THIS->output_val, block);
}
#undef THIS




