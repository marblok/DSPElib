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
class DSPu_Const : public DSP_source
{
  private:
    DSP_float const_val;
    DSP_float_ptr const_state;
    
    static bool OutputExecute_one(DSP_source_ptr source, DSP::clock_ptr clock=NULL);
    static bool OutputExecute_many(DSP_source_ptr source, DSP::clock_ptr clock=NULL);

  public:
    DSPu_Const(DSP::clock_ptr ParentClock,
               DSP_float value);
    DSPu_Const(DSP::clock_ptr ParentClock,
               DSP_float value_re, DSP_float value_im);
    DSPu_Const(DSP::clock_ptr ParentClock,
               DSP_complex value);
    DSPu_Const(DSP::clock_ptr ParentClock,
               int NoOfInputs_in, DSP_float_ptr values);
    DSPu_Const(DSP::clock_ptr ParentClock,
               int NoOfInputs_in, DSP_complex_ptr values);
    ~DSPu_Const(void);
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
DSPu_Const::DSPu_Const(DSP::clock_ptr ParentClock,
                       DSP_float value)
  : DSP_source(ParentClock)
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

DSPu_Const::DSPu_Const(DSP::clock_ptr ParentClock,
                       DSP_float value_re, DSP_float value_im)
  : DSP_source(ParentClock)
{
  SetName("Const", false);
 
  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  RegisterOutputClock(ParentClock);
  
  const_val = 0.0;

  const_state = new DSP_float[2];
  const_state[0] = value_re;
  const_state[1] = value_im;
  
  OutputExecute_ptr = &OutputExecute_many;
}

DSPu_Const::DSPu_Const(DSP::clock_ptr ParentClock,
                       DSP_complex value)
  : DSP_source(ParentClock)
{
  SetName("Const", false);
 
  SetNoOfOutputs(2);
  DefineOutput("out", 0, 1);
  DefineOutput("out.re", 0);
  DefineOutput("out.im", 1);

  RegisterOutputClock(ParentClock);
  
  const_val = 0.0;

  const_state = new DSP_float[2];
  const_state[0] = value.re;
  const_state[1] = value.im;
  
  OutputExecute_ptr = &OutputExecute_many;
}

DSPu_Const::DSPu_Const(DSP::clock_ptr ParentClock,
                       int NoOfOutputs_in, DSP_float_ptr values)
  : DSP_source(ParentClock)
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
  const_state = new DSP_float[NoOfOutputs_in];
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    const_state[ind] = values[ind];
  }
  
  OutputExecute_ptr = &OutputExecute_many;
}

DSPu_Const::DSPu_Const(DSP::clock_ptr ParentClock,
                       int NoOfOutputs_in, DSP_complex_ptr values)
  : DSP_source(ParentClock)
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
  const_state = new DSP_float[2*NoOfOutputs_in];
  for (ind = 0; ind < NoOfOutputs_in; ind++)
  {
    const_state[2*ind] = values[ind].re;
    const_state[2*ind+1] = values[ind].im;
  }
  
  OutputExecute_ptr = &OutputExecute_many;
}

DSPu_Const::~DSPu_Const(void)
{
  if (const_state != NULL)
  {
    delete [] const_state;
    const_state = NULL;
  }
}

#define THIS_ ((DSPu_Const *)source)
bool DSPu_Const::OutputExecute_one(DSP_source_ptr source, DSP::clock_ptr clock)
{
  THIS_->OutputBlocks[0]->Execute_ptr(
      THIS_->OutputBlocks[0], THIS_->OutputBlocks_InputNo[0], 
      THIS_->const_val, source);
      
  return true;
};

bool DSPu_Const::OutputExecute_many(DSP_source_ptr source, DSP::clock_ptr clock)
{
  int ind;
  
  for (ind = 0; ind < THIS_->NoOfOutputs; ind++)
    THIS_->OutputBlocks[ind]->Execute_ptr(
          THIS_->OutputBlocks[ind], THIS_->OutputBlocks_InputNo[ind], 
          THIS_->const_state[ind], source);
      
  return true;
};
#undef THIS_
