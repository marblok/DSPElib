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
class DSPu_CCPC : public DSP::Block
{
  private:
    DSP::Complex in_value;
    
    static void InputExecute(DSP::Block *block, int InputNo, DSP::Float value, DSP::Component *Caller);
  public:
    DSPu_CCPC(void); 
    ~DSPu_CCPC(void);
};



#define  THIS  ((DSPu_CCPC *)block)
/**************************************************/
// CCPC - cartesian coordinated to polar coordinates converter
DSPu_CCPC::DSPu_CCPC()
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

DSPu_CCPC::~DSPu_CCPC()
{
}

void DSPu_CCPC::InputExecute(DSP::Block *block, int InputNo, DSP::Float value, DSP::Component *Caller)
{
  if (InputNo==0)
    THIS->in_value.re = value;
  else   
    THIS->in_value.im = value;
  THIS->NoOfInputsProcessed++;
    
  if (THIS->NoOfInputsProcessed < THIS->NoOfInputs)
    return;
  THIS->NoOfInputsProcessed = 0;

      
//  OutputBlocks[0]->Execute(OutputBlocks_InputNo[0], in_value.abs(), this);
  THIS->OutputBlocks[0]->Execute_ptr(
      THIS->OutputBlocks[0], 
      THIS->OutputBlocks_InputNo[0], 
      THIS->in_value.abs(), block);
//  OutputBlocks[1]->Execute(OutputBlocks_InputNo[1], in_value.angle(), this);
  THIS->OutputBlocks[1]->Execute_ptr(
      THIS->OutputBlocks[1], 
      THIS->OutputBlocks_InputNo[1], 
      THIS->in_value.angle(), block);
};
#undef THIS
