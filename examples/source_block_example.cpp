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
    DSP::Float_vector const_state;
    
    static bool OutputExecute_one(DSP::Source_ptr source, DSP::Clock_ptr clock=NULL);
    static bool OutputExecute_many(DSP::Source_ptr source, DSP::Clock_ptr clock=NULL);

  public:
    onst(DSP::Clock_ptr ParentClock, DSP::Float value);
    Const(DSP::Clock_ptr ParentClock,
          DSP::Float value_re, DSP::Float value_im);
    Const(DSP::Clock_ptr ParentClock, DSP::Complex value);
    Const(DSP::Clock_ptr ParentClock,
          int NoOfInputs_in, DSP::Float_ptr values);
    Const(DSP::Clock_ptr ParentClock,
          int NoOfInputs_in, DSP::Complex_ptr values);
    ~Const(void);
};


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
  : DSP::Source(ParentClock)
{
  SetName("Const", false);

  SetNoOfOutputs(1);
  //SetNoOfInputs(0,false);
  DefineOutput("out", 0);
  DefineOutput("out.re", 0);
  
  RegisterOutputClock(ParentClock);

  const_val = value;
  const_state.clear();
  
  OutputExecute_ptr = &OutputExecute_one;
}

DSP::u::Const::Const(DSP::Clock_ptr ParentClock,
                       DSP::Float value_re, DSP::Float value_im)
  : DSP::Source(ParentClock)
{
  SetName("Const", false);
 
  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  RegisterOutputClock(ParentClock);
  
  const_val = 0.0;

  const_state.resize(2);
  const_state[0] = value_re;
  const_state[1] = value_im;
  
  OutputExecute_ptr = &OutputExecute_many;
}

DSP::u::Const::Const(DSP::Clock_ptr ParentClock,
                       DSP::Complex value)
  : DSP::Source(ParentClock)
{
  SetName("Const", false);
 
  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  RegisterOutputClock(ParentClock);
  
  const_val = 0.0;

  const_state.resize(2);
  const_state[0] = value.re;
  const_state[1] = value.im;
  
  OutputExecute_ptr = &OutputExecute_many;
}

DSP::u::Const::Const(DSP::Clock_ptr ParentClock,
                       int NoOfOutputs_in, DSP::Float_ptr values)
  : DSP::Source(ParentClock)
{
  string tekst;
  int ind;
  vector<int> temp_int;
  
  SetName("Const", false);
 
  SetNoOfOutputs(NoOfOutputs_in);
  temp_int = new int[NoOfOutputs_in];
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    tekst = "out" + to_string(ind+1);
    DefineOutput(tekst, ind);
    tekst = "out" + to_string(ind+1) + ".re";
    DefineOutput(tekst, ind);
    
    temp_int.push_back(ind);
  }
  DefineOutput("out", NoOfOutputs_in, temp_int.data());

  RegisterOutputClock(ParentClock);
  
  const_val = 0.0;
  const_state.resize(NoOfOutputs_in);
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    const_state[ind] = values[ind];
  }
  
  OutputExecute_ptr = &OutputExecute_many;
}

DSP::u::Const::Const(DSP::Clock_ptr ParentClock,
                       int NoOfOutputs_in, DSP::Complex_ptr values)
  : DSP::Source(ParentClock)
{
  string tekst;
  int ind;
  vector<int> temp_int;
  
  SetName("Const", false);
 
  SetNoOfOutputs(2*NoOfOutputs_in);
  temp_int = new int[2*NoOfOutputs_in];
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    tekst = "out" + to_string(ind+1);
    DefineOutput(tekst, 2*ind, 2*ind+1);
    tekst = "out" + to_string(ind+1) + ".re";
    DefineOutput(tekst, 2*ind);
    tekst = "out" + to_string(ind+1) + ".im";
    DefineOutput(tekst, 2*ind+1);
    
    temp_int.push_back(ind);
  }
  DefineOutput("out", NoOfOutputs_in, temp_int.data());

  RegisterOutputClock(ParentClock);
  
  const_val = 0.0;
  const_state.resize(2*NoOfOutputs_in);
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    const_state[2*ind] = values[ind].re;
    const_state[2*ind+1] = values[ind].im;
  }
  
  OutputExecute_ptr = &OutputExecute_many;
}

DSP::u::Const::~Const(void)
{
  const_state.clear();
}

#define THIS_ ((DSP::u::Const *)source)
bool DSP::u::Const::OutputExecute_one(DSP::Source_ptr source, DSP::Clock_ptr clock)
{
  THIS_->OutputBlocks[0]->Execute_ptr(
      THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0], 
      THIS_->const_val, source);
      
  return true;
};

bool DSP::u::Const::OutputExecute_many(DSP::Source_ptr source, DSP::Clock_ptr clock)
{
  int ind;
  
  for (ind = 0; ind < THIS_->NoOfOutputs; ind++)
    THIS_->OutputBlocks[ind]->Execute_ptr(
          THIS_->OutputBlocks[ind], THIS_->OutputBlocks_InputNo[ind], 
          THIS_->const_state[ind], source);
      
  return true;
};
#undef THIS_
