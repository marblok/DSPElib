/*! \file DSP_DOT.cpp
 * This DOT graph support module.
 *
 * \author Marek Blok
 */
#include <iomanip>

#include <DSP_DOT.h>

//#include <DSP_modules.h>
//#include <DSP_clocks.h>
#include <DSP_lib.h>

const vector<string> DOT_colors =
{
    "red", "royalblue2", "green3", "turquoise2", "yellow3",
    "chocolate4", "blueviolet", "deeppink1", "goldenrod1"
};


#ifdef __DEBUG__
  string DSPu_Splitter::GetComponentNodeParams_DOTfile()
  {
    return "[shape=point]";
  }

  // Returns true if ports should be used for edges
  bool DSPu_Splitter::UsePorts_DOTfile(void)
  { return false; }

  string DSPu_Splitter::GetComponentEdgeParams_DOTfile(const unsigned int &output_index)
  {
    unsigned int ind2;
    stringstream text_buffer;

    // start from output with the same input the skip by NoOfInputs
    //ind2 = output_index / (NoOfOutputs / NoOfInputs);
    if (NoOfInputs != 1)
      ind2 = output_index % NoOfInputs;
    else
      ind2 = output_index;

    // find index of the input connected to this output
    //  ==> find block and ind_ with
    //   block->OutputBlock[ind_] ==  this
    //   block->OutputBlocks_InputNo[ind_] == ind2

    //?? line color should be the same as of the line connected to
    //?? the input corresponding the current output (InputBlocks)
    //ind2 = FindOutputIndex_by_InputIndex(ind2);

    if (ind2 <= MaxOutputIndex)
      text_buffer << "[color=" << DOT_colors[ind2 % DOT_colors.size()] << "]";
    else
      text_buffer << "[color=black]";
    return text_buffer.str();
  }
#endif



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

#ifdef __DEBUG__
/*! Input parameters:
 *  - NULL - the whole scheme is drawn, but macros can be drawn instead of components
 *  - DrawnMacro != NULL - the given macro scheme is drawn. Only internal components must be drawn.
 *  .
 *
 * Cases
 *  -# Draw the whole scheme (unwrapped) (DrawnMacro == NULL)
 *  -# Draw the scheme with wrapped macros (DrawnMacro == NULL, based on macros' DOTmode)
 *  -# Draw the macro scheme (DrawnMacro != NULL)
 *  .
 * Special cases:
 *  - AutoSplitter: input inside the macro and some of the outputs inside the macro - should be always visible
 *  .
 *
 * Returns:
 *  - NULL - component is inside the DrawnMacro so it must be drawn (is visible)
 *  - DrawnMacro - component must not be drawn (is invisible - outside the DrawnMacro)
 *  - macro pointer - returned macro must be drawn instead of the component
 *  .
 *
 */
DSP::Macro_ptr DSP::Component::DOT_DrawAsMacro(DSP::Macro_ptr DrawnMacro)
{
  unsigned int ind;
  DSP::Macro_ptr *temp_stack;
  unsigned int temp_length;

  // consider all the components
  temp_stack = MacrosStack;
  temp_length = MacrosStackLength;

  if (DrawnMacro != NULL)
  {
    temp_stack = NULL;
    for (ind = 0; ind < MacrosStackLength; ind++)
    {
      if (MacrosStack[ind] == DrawnMacro)
      {
        // ignore DrawnMacro and all macros from the stack placed before it
        temp_length = MacrosStackLength - ind - 1;
        if (temp_length == 0)
        {
          //temp_stack = NULL;
          /*! \bug 2010.04.01 This can be autosplitter with only one output inside the DrawnMacro.
           *    The edge without node should be drawn instead the node.
           */
          return NULL; // DrawnMacro component so draw it
        }
        else
        {
          /*!When macro scheme is drawn, internal macros DOTmode must be checked
           */
          temp_stack = &(MacrosStack[ind+1]); // inside the DrawnMacro
        }
        break;
      }
    }
    if (temp_stack == NULL)
    { // no DrawnMacro on the Stack
      /*! \bug 2010.04.01 problem is that this can be autosplitter which is outside of the DrawnMacro
       *    but its input block and at least one of its output blocks can be inside the macro, which
       *    means it must be drawn (visible)
       */
      if (IsAutoSplit == true)
      {
        // if autosplitter then check if it should not be drawn
        // ==> input block is inside the macro OR at least one of its outputs is outside the macro
        //! \bug 2010.04.02 It should be rather at least two outputs, for one output the edge just should be corrected
        //! \bug 2010.04.02 the assumption that autosplitter has just one input is ok for now but it might change in the future
        DSP::Component_ptr tempBlock; unsigned int tempNo;
        DSP::Macro_ptr input_macro, output_macro;

        if (IsOutputConnectedToThisInput2(this, 0, tempBlock, tempNo))
        {
          input_macro = tempBlock->DOT_DrawAsMacro(DrawnMacro);
          if (input_macro == NULL)
          {
            return NULL;  // input is inside DrawnMacro component so draw it
          }
          else
          {
            for (unsigned int ind3 = 0; ind3 <NoOfOutputs; ind3++)
            {
              if (OutputBlocks[ind3] != NULL)
              {
                output_macro = OutputBlocks[ind3]->DOT_DrawAsMacro(DrawnMacro);
                if (output_macro == NULL)
                {
                  return NULL;  // is inside DrawnMacro component so draw it
                }
              }
            }
          }
        }
      }
      return DrawnMacro; // outside of the DrawnMacro
    }
  }


  /*! \bug 2010.04.01 There might be problems with autosplitters with nested macros
   */
  for (ind = 0; ind < temp_length; ind++)
  {
    if (temp_stack[ind]->DOT_DrawMacro() == true)
      return temp_stack[ind]; // don't draw component if any of its parent macros will be drawn instead
  }

  return NULL;
}

// Returns component name used in DOTfile
string DSP::Component::GetComponentName_DOTfile()
{
  long component_index;
  string type_name;
  string text_buffer;

  component_index = GetComponentIndexInTable(this);
  if (component_index < 0)
    return "";
  //text_tmp = text_buffer;

  // Bloczki(1).type = 's'; % 's' - source, 'b' - processing block, 'm' - mixed: processing & source block, % 'o' - output (generally == processing block with no outputs)
  switch (Type)
  {
    case DSP_CT_source:
      type_name = "source";
      break;
    case DSP_CT_mixed:
      type_name = "mixed";
      break;
    case DSP_CT_block:
      if (IsAutoSplit == false)
        type_name = "block";
      else
        type_name = "auto";
      break;
    case DSP_CT_copy:
      type_name = "copy";
      break;
    case DSP_CT_none:
    default:
      type_name = "unknown";
      break;
  }

  text_buffer = type_name;
  text_buffer += '_';
  text_buffer += to_string(component_index);

  return text_buffer;
}

// Returns component node parameters used in DOTfile
/*
 *    -# generate string segment (internal buffer - ?? size selection)
 *    -# check if it will fit into the output buffer
 *    -# copy string segment into output buffer.
 *    .
 */
string DSP::Component::GetComponentNodeParams_DOTfile()
{
  string tempName;
  unsigned int ind;
  //! pointer to an internal text buffer
  string internal_text;
  //! length of the output text (including trailing zero)
  stringstream text_buffer;

  tempName = GetName();
  if (tempName.length() == 0)
    tempName = "NONAME";

  /*
  mixed_2 [label="{{<in0> in0 | <in> in1 | DDScos | {<out0> out0 | <out1> out1}}"];
  mixed_2[shape=record,color=red];
  clock_003D6808 -> mixed_2:clock [style=dotted, constraint=false, color=red];
  */

  text_buffer << "[label=\"{";

  if (Convert2Block() != NULL)
  {
    if (Convert2Block()->NoOfInputs > 0)
    {
      text_buffer << "{";

      for (ind =0; ind < Convert2Block()->NoOfInputs; ind++)
      {
        if (ind == 0)
          text_buffer << "<in" << ind << "> ";
        else
          text_buffer << " | <in" << ind << "> ";
      }

      text_buffer << "} | ";
    }
  }

  // prepare label
  internal_text = "";
  for (ind = 0; ind < tempName.length(); ind++)
  {
    switch (tempName[ind])
    {
      case ' ':
      case '<':
      case '>':
      case '{':
      case '}':
      case '|':
      case '"':
        internal_text += '\\';
        internal_text += tempName[ind];
        break;
      default:
        internal_text += tempName[ind];
        break;
    }
  }
  text_buffer << internal_text;


  if (NoOfOutputs > 0)
  {
    text_buffer << " | {";

    for (ind =0; ind < NoOfOutputs; ind++)
    {
      if (ind == 0)
        text_buffer << "<out" << ind << "> ";
      else
        text_buffer << " | <out" << ind << "> ";
    }

    text_buffer << "}";
  }
  text_buffer << "}\",shape=record";

  if (Convert2ClockTrigger() != NULL)
  {
    //temp_text = text_buffer + strlen(text_buffer) - 1;
    text_buffer << ",penwidth=3.0,color=" <<
        DOT_colors[Convert2ClockTrigger()->SignalActivatedClock->GetMasterClockIndex() % DOT_colors.size()];
  }
  text_buffer << "]";

  return text_buffer.str();
}

string DSP::Component::GetComponentEdgeParams_DOTfile(const unsigned int &output_index)
{
  stringstream text_buffer;
  text_buffer << "[color=" << DOT_colors[output_index % DOT_colors.size()] << "]";
  return text_buffer.str();
}

// Returns true if ports should be used for edges
bool DSP::Component::UsePorts_DOTfile(void)
{ return true; }

// Writes component edges to file
void DSP::Component::ComponentEdgesToDOTfile(std::ofstream &dot_plik, const string &this_name,
    bool *UsedMacrosTable, DSP::Macro_ptr *MacrosList, long macros_number,
    DSP::Macro_ptr DrawnMacro, unsigned int space_sep)
{
  unsigned int ind, ind_sep;
  long ind2;
  string that_name, text_buffer;
  DSP::Block_ptr temp_block;
  DSP::Macro_ptr current_macro;

  //Bloczki(1).output_blocks = [2.0]; % indexes of output blocks (b.i <= b - block index (1, 2, ...), i - output block input index  (0, 1, ...)
  //    DSP::Block_ptr *OutputBlocks; //!one block pointer for one output
  //    int *OutputBlocks_InputNo; //!Input number of the output block
  for (ind=0; ind<NoOfOutputs; ind++)
  {
    if (OutputBlocks[ind] != NULL)
    {
      current_macro = OutputBlocks[ind]->DOT_DrawAsMacro(DrawnMacro);
      if (current_macro != NULL)
      {
        if (current_macro != DrawnMacro)
        { // otherwise should not be drawn (input and output edges of the macro DOT file are drawn somewhere else)
          DSP::Component_ptr output_block;
          unsigned int output_block_input_no, macro_input_no;
          output_block = OutputBlocks[ind];
          output_block_input_no = OutputBlocks_InputNo[ind];

          macro_input_no = current_macro->GetMacroInputNo(output_block, output_block_input_no);

          if (macro_input_no != FO_NoInput)
          {
            for (ind2 = 0; ind2 < macros_number; ind2++)
            {
              if (MacrosList[ind2] == current_macro)
              {
                UsedMacrosTable[ind2] = true;
                break;
              }
            }

            // Write edge to the macro
            for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
              dot_plik << ' ';
            if (UsePorts_DOTfile() == true)
              dot_plik << this_name << ":out" << ind << " -> ";
            else
              dot_plik << this_name << " -> ";

            that_name = current_macro->GetMacroName_DOTfile();

            if (current_macro->UsePorts_DOTfile() == true)
            {
              dot_plik <<  that_name << ":in" << macro_input_no;
            }
            else
              dot_plik <<  that_name;


            if (NoOfOutputs > 1)
            {
              dot_plik << " "
                       << GetComponentEdgeParams_DOTfile(ind);
            }
            dot_plik << ";"<< std::endl;
          }
          else
          {
            // this is internal macro edge
          }
        }
      }
      else
      {
        //! \todo unconnected outputs ==> DummyBlock
        for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
          dot_plik << ' ';
        if (UsePorts_DOTfile() == true)
          dot_plik << this_name << ":out" << ind << " -> ";
        else
          dot_plik << this_name << " -> ";


        that_name = OutputBlocks[ind]->GetComponentName_DOTfile();

        if (OutputBlocks[ind]->UsePorts_DOTfile() == true)
          dot_plik <<  that_name << ":in" << OutputBlocks_InputNo[ind];
        else
          dot_plik <<  that_name;

        if (NoOfOutputs > 1)
        {
          dot_plik << " "
                   << GetComponentEdgeParams_DOTfile(ind);
        }
        dot_plik << ";"<< std::endl;
      }
    }
    else
    { //! \todo unconnected outputs

    }
  }
  dot_plik << std::endl;

  // constant inputs if any
  temp_block = Convert2Block();
  if (temp_block != NULL)
  {
    if (temp_block->IsUsingConstants == true)
    {
      for (ind=0; ind<temp_block->NoOfInputs; ind++)
        if (temp_block->IsConstantInput[ind] == true)
        {
          for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
            dot_plik << ' ';
          stringstream ss;
          ss << this_name << "_const_in" << ind
             << " [shape=none,label="
             << fixed << setprecision(3) << temp_block->ConstantInputValues[ind] << "];";
          dot_plik << ss.str() << std::endl;

          for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
            dot_plik << ' ';
          ss.clear(); ss.str("");
          if (UsePorts_DOTfile() == true) {
            ss << this_name << "_const_in" << ind
               << " -> " << this_name << ":in" << ind << " ";
          }
          else {
            ss << this_name << "_const_in" << ind << " -> " << this_name << " ";
          }
          ss << DSP::Component::GetComponentEdgeParams_DOTfile(ind);
          dot_plik << ss.str() << ";" << std::endl;
        }
    }
  }
  dot_plik << std::endl;

  //! \todo unconnected inputs
}


// Saves this component and calls it's output blocks (except of source & mixed blocks) to DOT-file
/* See DSP::Clock::SchemeToDOTfile and DSP::Clock::ClockComponetsToDOTfile
 *
 * Can be called for sources and mixed blocks but cannot call itself for sources & mixed blocks
 */
void DSP::Component::ComponentToDOTfile(std::ofstream &dot_plik,
          bool *ComponentDoneTable, long max_components_number,
          bool *UsedMacrosTable, DSP::Macro_ptr *MacrosList, long macros_number,
          bool *UsedClocksTable, DSP::Clock_ptr *ClocksList, long clocks_number,
          DSP::Macro_ptr DrawnMacro, DSP::Clock_ptr clock_ptr)
{
  unsigned int ind;
  long ind2;
  DSP::Block_ptr temp_OUT;
  long component_index;
  DSP::Macro_ptr current_macro;
  string this_name;


  component_index = GetComponentIndexInTable(this);
  if (component_index >= max_components_number)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Component::ComponentToDOTfile" << DSP::LogMode::second
        <<  "max_components_number (" << (int)max_components_number
        << ") <= component_index (" << (int)component_index << ")" << endl;
    #endif
    return;
  }
  if (ComponentDoneTable[component_index] == true)
  {
    return; // component was processed before
  }

  ///////////////////////////////
  // Define component
  ///////////////////////////////

  current_macro = DOT_DrawAsMacro(DrawnMacro);
  if (current_macro != NULL)
  {
    for (ind2 = 0; ind2 < macros_number; ind2++)
    {
      if (MacrosList[ind2] == current_macro)
      {
        UsedMacrosTable[ind2] = true;
        break;
      }
    }
  }
  else
  {
//    int name_len, temp_name_len;
    string temp_name;
//    bool done;

    ComponentDoneTable[component_index] = true;

    // Save info for current component
    // Bloczki(1).unique_index = 12; % unique number identifying object == integer part of Bloczki(ind).output_blocks
    this_name = GetComponentName_DOTfile();
    temp_name = GetComponentNodeParams_DOTfile();
    dot_plik << "    " << this_name << " " << temp_name << ";" << std::endl;

    if (clock_ptr != NULL)
    {
      dot_plik << "    " << this_name << "[color="
               << DOT_colors[clock_ptr->GetMasterClockIndex() % DOT_colors.size()]
                             << "];" << std::endl;

      /*! \bug 2010.04.19 For multiclock blocks clocks edge should point at appropriate block's input/output instead of the block itself
       */
      stringstream ss;
      ss << "clock_" << clock_ptr;
      dot_plik << "    " << ss.str() << " -> ";
      dot_plik << this_name << " [style=dotted, constraint=false, color="
               << DOT_colors[clock_ptr->GetMasterClockIndex() % DOT_colors.size()]
                             << "];" << std::endl << std::endl << std::endl;
    }

    /*!  \bug multirate blocks
     *    which are not source or registered for notification
     *    should have the parent clock indicated
     *
     *  !!! chyba nie wszystkie powinny by� tak traktowane ??
     *    a mo�e
     */
    if ((Type & DSP_CT_source) == 0)
    {
      if (Convert2Block()->IsMultirate == true)
      {
        if (OutputClocks[0] != NULL)
        {
          // update UsedClocksTable
          for (ind2 = 0; ind2 < clocks_number; ind2++)
          {
            if(ClocksList[ind2] == OutputClocks[0])
              UsedClocksTable[ind2] = true;
          }

          dot_plik << "    " << this_name << "[color="
                  << DOT_colors[OutputClocks[0]->GetMasterClockIndex() % DOT_colors.size()]
                                << "];" << std::endl;

          stringstream ss;
          ss << "clock_" << OutputClocks[0];
          dot_plik << "    " << ss.str() << " -> ";
          dot_plik << this_name << " [style=dotted, constraint=false, color="
                   << DOT_colors[OutputClocks[0]->GetMasterClockIndex() % DOT_colors.size()]
                                 << "];" << std::endl << std::endl << std::endl;
        }
      }
    }

    ///////////////////////////////
    // Define outgoing connections
    ///////////////////////////////
    ComponentEdgesToDOTfile(dot_plik, this_name,
        UsedMacrosTable, MacrosList, macros_number,
        DrawnMacro);
  }

  for (ind=0; ind<NoOfOutputs; ind++)
  {
    temp_OUT = OutputBlocks[ind];

    bool call_next = false;
    if (temp_OUT->Type == DSP_CT_block) {
      call_next = true;
    }
    else {
      if ((temp_OUT->Type & DSP_CT_source) == DSP_CT_source) {
        DSP::Source_ptr tmp_source = temp_OUT->Convert2Source();
        if (tmp_source->OutputExecute_ptr == &DSP::Source::DummyExecute) {
          call_next = true; // block nie jest pod��czony pod �r�d�o
        }
      }
    }
    // call this function for all output blocks except for sources & mixed blocks
    if (call_next) {
      temp_OUT->ComponentToDOTfile(dot_plik, ComponentDoneTable, max_components_number,
                                   UsedMacrosTable, MacrosList, macros_number,
                                   UsedClocksTable, ClocksList, clocks_number,
                                   DrawnMacro);
    }
  }
}

#endif





// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

#ifdef __DEBUG__
  bool DSP::Macro::DOT_DrawMacro(void)
  {
    bool result;

    result = false;
    switch (DOTmode)
    {
      case DSP_DOT_macro_wrap:
      case DSP_DOT_macro_as_component:
        result = true;
        break;
      case DSP_DOT_macro_subgraph:
      case DSP_DOT_macro_unwrap:
      default:
        result = false;
        break;
    }

    return result;
  }

  string DSP::Macro::GetMacroName_DOTfile()
  {
    stringstream ss;
    ss << "macro_" << this;
    return ss.str();
  }


  // Returns macro node parames used in DOTfile
  string DSP::Macro::GetMacroNodeParams_DOTfile()
  {
    stringstream params_ss;
    string tempName;
    string label;
    unsigned int ind;

    tempName = GetName();
    if (tempName.length() == 0)
      tempName = "NONAME";

    /*
    mixed_2 [label="{{<in0> in0 | <in> in1 | DDScos | {<out0> out0 | <out1> out1}}"];
    mixed_2[shape=record,color=red];
    */
    params_ss << "[label=\"{";

    if (NoOfInputs > 0)
    {
      params_ss << "{";
      for (ind =0; ind < NoOfInputs; ind++)
      {
        if (ind == 0)
          params_ss << "<in" << ind << "> ";
        else
          params_ss << " | <in" << ind << "> ";
       }
      params_ss << "} | ";
    }
    // prepare label
    label = "";
    for (ind = 0; ind < tempName.length(); ind++)
    {
      switch (tempName[ind])
      {
        case ' ':
        case '<':
        case '>':
        case '{':
        case '}':
        case '|':
          label += '\\';
          label += tempName[ind];
          break;
        default:
          label += tempName[ind];
          break;
      }
    }
    params_ss << label;

    if (NoOfOutputs > 0)
    {
      params_ss << " | {";
      for (ind =0; ind < NoOfOutputs; ind++)
      {
        if (ind == 0)
          params_ss << "<out" << ind << "> ";
        else
          params_ss << " | <out" << ind << "> ";
      }
      params_ss << "}";
    }
    params_ss << "}\",shape=record,penwidth=4.0]";

    return params_ss.str();
  }

  string DSP::Macro::GetMacroInputNodeParams_DOTfile()
  {
    stringstream params_ss;
    string tempName;
    string label;
    unsigned int ind;

    tempName = MacroInput_block->GetName();
    if (tempName.length() == 0)
      tempName = "NONAME";

    /*
    mixed_2 [label="{{<in0> in0 | <in> in1 | DDScos | {<out0> out0 | <out1> out1}}"];
    mixed_2[shape=record,color=red];
    */
    params_ss << "[label=\"{";

    // prepare label
    label = "";
    for (ind = 0; ind < tempName.length(); ind++)
    {
      switch (tempName[ind])
      {
        case ' ':
        case '<':
        case '>':
        case '{':
        case '}':
        case '|':
          label += '\\';
          label += tempName[ind];
          break;
        default:
          label += tempName[ind];
          break;
      }
    }
    params_ss << label;

    //MacroInput->NoOfOutputs == NoOfInputs
    if (MacroInput_block->NoOfOutputs > 0)
    {
      params_ss << " | {";
      for (ind =0; ind < MacroInput_block->NoOfOutputs; ind++)
      {
        if (ind == 0)
          params_ss <<  "<out" << ind << "> ";
        else
          params_ss << " | <out" << ind << "> ";
      }
      params_ss << "}";
    }
    params_ss << "}\",shape=record,penwidth=4.0]";

    return params_ss.str();
  }

  string DSP::Macro::GetMacroOutputNodeParams_DOTfile()
  {
    stringstream text_buffer;
    string tempName;
    string temp_text;
    string label;
    unsigned int ind;

    tempName = MacroOutput_block->GetName();
    if (tempName.length() == 0)
      tempName = "NONAME";

    /*
    mixed_2 [label="{{<in0> in0 | <in> in1 | DDScos | {<out0> out0 | <out1> out1}}"];
    mixed_2[shape=record,color=red];
    */
    text_buffer << "[label=\"{";

    // NoOfOutputs == MacroOutput->NoOfInputs
    if (MacroOutput_block->NoOfInputs > 0)
    {
      text_buffer << "{";
      for (ind =0; ind < MacroOutput_block->NoOfInputs; ind++)
      {
        if (ind == 0)
          text_buffer << "<in" << ind << "> ";
        else
          text_buffer << " | <in" << ind << "> ";
       }
       text_buffer << "} | ";
    }
    // prepare label
    label = "";
    for (ind = 0; ind < tempName.length(); ind++)
    {
      switch (tempName[ind])
      {
        case ' ':
        case '<':
        case '>':
        case '{':
        case '}':
        case '|':
          label += '\\';
          label += tempName[ind];
          break;
        default:
          label += tempName[ind];
          break;
      }
    }
    text_buffer << label << "}\",shape=record,penwidth=4.0]";

    return text_buffer.str();
  }

  string DSP::Macro::GetMacroEdgeParams_DOTfile(const unsigned int &output_index)
  {
    stringstream text_buffer;
    text_buffer << "[color=" << DOT_colors[output_index % DOT_colors.size()] << "]";
    return text_buffer.str();
  }

  // Returns true if ports should be used for edges
  bool DSP::Macro::UsePorts_DOTfile(void)
  { return true; }

  // Writes macro outgoing and constant inputs edges to file
  void DSP::Macro::MacroEdgesToDOTfile(std::ofstream &dot_plik, const string &macro_name,
      DSP::Macro_ptr DrawnMacro, unsigned int space_sep)
  {
    unsigned int ind, ind_sep;
    string that_name;
    stringstream text_buffer;
    DSP::Macro_ptr current_macro;
    bool input_done;
    //DSP::Component_ptr temp_component;
    //unsigned int temp_component_output_no;

    DSP::Block_ptr macro_output_block;  unsigned int macro_output_block_InputNo;

    for (ind=0; ind<NoOfOutputs; ind++)
    {
      MacroOutput_block->GetCopyOutput(ind, macro_output_block, macro_output_block_InputNo);

      if (macro_output_block != NULL)
      {
        //! \todo unconnected outputs ==> DummyBlock

        for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
          dot_plik << ' ';
        if (UsePorts_DOTfile() == true)
          dot_plik << macro_name << ":out" << ind << " -> ";
        else
          dot_plik << macro_name << " -> ";


        DSP::Component_ptr output_block; unsigned int output_block_input_no;
        DSP::Component_ptr tempBlock; unsigned int tempNo;

        output_block = macro_output_block;
        output_block_input_no = macro_output_block_InputNo;


        // 1. Copy ==> OUT.outside_macro  //  IN.in_macro ==> OUT.outside_macro
        // 2. Copy ==> OUT.outside_macro  //  IN.in_macro ==> AUTO.in_macro ==> OUT.outside_macro
        if (DSP::Component::IsOutputConnectedToThisInput2(output_block, output_block_input_no, tempBlock, tempNo))
        {
          if (tempBlock->IsAutoSplit == true)
          { // use the autosplitter instead
            output_block = tempBlock;
            output_block_input_no = tempNo;
          }
          else
          { // probably nothing to do at all
          }
        }
        else
        {
         // this should not happen
          DSP::log << "DSP::Macro::MacroEdgesToDOTfile" << DSP::LogMode::second << "Nothing connected to the output block's input" << endl;
        }


        current_macro = output_block->DOT_DrawAsMacro(DrawnMacro);
        if (current_macro != NULL)
        {

          // edge to other macro
          // Must identify current_macro input port
          that_name = current_macro->GetMacroName_DOTfile();

          if (current_macro->UsePorts_DOTfile() == true)
          {
            output_block_input_no = current_macro->GetMacroInputNo(output_block, output_block_input_no);
            dot_plik <<  that_name << ":in" << output_block_input_no;
          }
          else
            dot_plik <<  that_name;
        }
        else
        {
          // edge to the component
          that_name = output_block->GetComponentName_DOTfile();

          if (output_block->UsePorts_DOTfile() == true)
            dot_plik <<  that_name << ":in" << output_block_input_no;
          else
            dot_plik <<  that_name;
        }

        if (NoOfOutputs > 1)
        {
          dot_plik << " "
                   << GetMacroEdgeParams_DOTfile(ind);
        }
        dot_plik << ";"<< std::endl;
      }
      else
      { //! \todo unconnected outputs

      }
    }
    dot_plik << std::endl;

    //! \todo 2010.04.02 prepare example with autospliters at input and output of the macro (example for output is already available)
    // constant inputs if any
    for (ind=0; ind<NoOfInputs; ind++)
    {
      input_done = false;
      if (MacroInput_block->IsUsingConstants == true)
      {
        if (MacroInput_block->IsConstantInput[ind] == true)
        {
          for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
            dot_plik << ' ';
          text_buffer.clear(); text_buffer.str("");
          text_buffer << macro_name << "_const_in" << ind << " [shape=none,label="
                      << fixed << setprecision(3) << MacroInput_block->ConstantInputValues[ind] << "];";
          dot_plik << text_buffer.str() << std::endl;

          for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
            dot_plik << ' ';
          text_buffer.clear(); text_buffer.str("");
          if (UsePorts_DOTfile() == true)
            text_buffer << macro_name << "_const_in" << ind
                        << " -> "<< macro_name << ":in" << ind << " ";
          else
            text_buffer << macro_name << "_const_in" << ind << " -> " << macro_name << " ";
          text_buffer << GetMacroEdgeParams_DOTfile(ind);
          dot_plik << text_buffer.str() << ";" << std::endl;

          input_done = true;
        }
      }
      if (input_done == false)
      { //! \note Macro input edges should be created in DSP::Component::ComponentEdgesToDOTfile
        /*
        // <bug> does not support Copy chains !!!
        temp_component = MacroInput_block->InputBlocks[ind];
        temp_component_output_no = MacroInput_block->InputBlocks_OutputNo[ind];
        if (temp_component->DOT_DrawAsMacro(DrawnMacro) == NULL)
        { // other inputs (from other blocks)
          temp_component->GetComponentName_DOTfile(that_name, 1024);

          for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
            dot_plik << ' ';
          if (temp_component->UsePorts_DOTfile() == true)
            dot_plik << that_name << ":out" << temp_component_output_no << " -> ";
          else
            dot_plik << that_name << " -> ";

          if (UsePorts_DOTfile() == true)
            dot_plik <<  macro_name << ":in" << ind; // macro input index
            //dot_plik <<  macro_name << ":in" << temp_component->OutputBlocks_InputNo[ind];
          else
            dot_plik <<  macro_name;

          if (temp_component->NoOfOutputs > 1)
          {
            dot_plik << " "
                     << temp_component->GetComponentEdgeParams_DOTfile(text_buffer, 1024, ind);
          }
          dot_plik << ";"<< std::endl;

        }
        */
        // other inputs (from other macros) <-- this must be done in the source macro
      }
    }
    dot_plik << std::endl;


    //! \todo unconnected inputs
  }

  //! Writes macro input edges to file
  void DSP::Macro::MacroInputEdgesToDOTfile(std::ofstream &dot_plik, const string &macro_input_name,
      DSP::Macro_ptr DrawnMacro, unsigned int space_sep)
  {
    unsigned int ind, ind_sep;
    string that_name;
    stringstream text_buffer;
    DSP::Macro_ptr current_macro;
    DSP::Block_ptr current_output;

    for (ind=0; ind<MacroInput_block->NoOfOutputs; ind++)
    {
      current_output = MacroInput_block->OutputBlocks[ind];

      if (current_output != NULL)
      {
        /* Identify output
         * 2. Copy
         * 3. Block
         * 4. Auto
         */
        if (current_output == &DSP::Component::DummyBlock)
        { // 1. DummyBlock
          // unconnected output : do nothing ???
          continue;
        }
        if (current_output->Convert2Copy() != NULL)
        { // 2. Copy
          DSP::log << "DSP::Macro::MacroInputEdgesToDOTfile" << DSP::LogMode::second << "Edge to Copy !!!" << endl;
        }

        // ++++++++++++++++++++++++++++++++++++++++++++++++++ //
        current_macro = current_output->DOT_DrawAsMacro(DrawnMacro);
        if (current_macro != NULL)
        {
          //! \todo edge to other macro
          DSP::log << "DSP::Macro::MacroInputEdgesToDOTfile" << DSP::LogMode::second << "edge to other macro not implemented yet" << endl;
        }
        else
        {
          //! \todo unconnected outputs ==> DummyBlock

          for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
            dot_plik << ' ';
          if (UsePorts_DOTfile() == true)
            dot_plik << macro_input_name << ":out" << ind << " -> ";
          else
            dot_plik << macro_input_name << " -> ";

          // 1. Copy  ==>  OUT.in_macro  //  OUTinput.outside_macro ==> OUT.in_macro
          // 2. Copy  ==>  OUT.in_macro  //  OUTinput.outside_macro ==> AUTO.in_macro ==> OUT.in_macro
          DSP::Component_ptr output_block; unsigned int output_block_input_no;
          DSP::Component_ptr tempBlock; unsigned int tempNo;

          output_block = MacroInput_block->OutputBlocks[ind];
          output_block_input_no = MacroInput_block->OutputBlocks_InputNo[ind];

          // If autosplitter is connected to the output block draw edge to the autospliter instead
          if (DSP::Component::IsOutputConnectedToThisInput2(output_block, output_block_input_no, tempBlock, tempNo))
          {
            if (tempBlock->IsAutoSplit == true)
            { // check if the autosplitter input is in the macro
              output_block = tempBlock;
              output_block_input_no = tempNo;
            }
            else
            { // probably nothing to do at all
            }
          }
          else
          {
            // this should rather not happen
          }

          if (output_block->DOT_DrawAsMacro(DrawnMacro) == NULL)
          {
            that_name = output_block->GetComponentName_DOTfile();

            if (output_block->UsePorts_DOTfile() == true)
              dot_plik <<  that_name << ":in" << output_block_input_no;
            else
              dot_plik <<  that_name;
          }
          else
          {
            that_name = output_block->DOT_DrawAsMacro(DrawnMacro)->GetMacroName_DOTfile();

            if (output_block->UsePorts_DOTfile() == true)
              dot_plik <<  that_name << ":in" << ind; // just to macro input
            else
              dot_plik <<  that_name;
          }


          if (MacroInput_block->NoOfOutputs > 1)
          {
            dot_plik << " "
                     << GetMacroEdgeParams_DOTfile(ind);
          }
          dot_plik << ";"<< std::endl;
        }
      }
      else
      { //! \todo unconnected outputs

      }
    }
    dot_plik << std::endl;
  }

  // Writes component edges to file
  void DSP::Macro::MacroOutputEdgesToDOTfile(std::ofstream &dot_plik, const string &macro_output_name,
      DSP::Macro_ptr DrawnMacro, unsigned int space_sep)
  {
    unsigned int ind, ind_sep;
    string that_name;
    stringstream text_buffer;
    bool input_done;


    // constant inputs if any
    for (ind=0; ind<MacroOutput_block->NoOfInputs; ind++)
    {
      input_done = false;
      if (MacroOutput_block->IsUsingConstants == true)
      { // draw constant inputs
        if (MacroOutput_block->IsConstantInput[ind] == true)
        {
          for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
            dot_plik << ' ';
          text_buffer.clear(); text_buffer.str("");
          text_buffer << macro_output_name << "_const_in" << ind << " [shape=none,label="
                      << fixed << setprecision(3) << MacroOutput_block->ConstantInputValues[ind] << "];",
          dot_plik << text_buffer.str() << std::endl;

          for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
            dot_plik << ' ';
          text_buffer.clear(); text_buffer.str("");
          if (UsePorts_DOTfile() == true)
            text_buffer << macro_output_name << "_const_in" << ind
                        << " -> " << macro_output_name << ":in" << ind << " ";
          else
            text_buffer << macro_output_name << "_const_in" << ind << " -> " << macro_output_name << " ";
          text_buffer << GetMacroEdgeParams_DOTfile(ind);
          dot_plik << text_buffer.str() << ";" << std::endl;

          input_done = true;
        }
      }

      if (input_done == false)
      { // draw non constant inputs
        // 1. IN.outside_macro <== MacroOutput // IN.outside_macro ==> OUT.in_macro
        // 2. IN.outside_macro <== MacroOutput // IN.outside_macro ==> AUTO.in_macro ==> OUT.in_macro
        DSP::Component_ptr input_block; unsigned int input_block_output_no;
        DSP::Component_ptr tempBlock; unsigned int tempNo;

        input_block = MacroOutput_block->InputBlocks[ind];
        input_block_output_no = MacroOutput_block->InputBlocks_OutputNo[ind];
        tempBlock = input_block->OutputBlocks[input_block_output_no];
        tempNo = input_block->OutputBlocks_InputNo[input_block_output_no];
        if (tempBlock->IsAutoSplit == true)
        { // check if the autosplitter output is in the macro
          input_block = tempBlock;
          input_block_output_no = tempNo;
        }
        else
        { // probably nothing to do at all
          if (input_block->Convert2Copy() != NULL)
          {
            // [Copy] <== MacroOutput // [Copy] ==> OUT  // AUTO.in_macro ==> OUT
            if (DSP::Component::IsOutputConnectedToThisInput2(tempBlock, tempNo, tempBlock, tempNo))
            {
              if (tempBlock->IsAutoSplit == true)
              { // check if the autosplitter input is in the macro
                input_block = tempBlock;
                input_block_output_no = tempNo;
              }
              else
              { // probably nothing to do at all
              }
            }
            else
            {
              // AUTO.in_macro ==> [Copy] <== MacroOutput
              if (DSP::Component::IsOutputConnectedToThisInput2(input_block, input_block_output_no, tempBlock, tempNo))
              {
                if (tempBlock->IsAutoSplit == true)
                { // check if the autosplitter input is in the macro
                  input_block = tempBlock;
                  input_block_output_no = tempNo;
                }
                else
                { // probably nothing to do at all
                }
              }
              else
              {
                // this should rather not happen
              }
            }
          }
        }

        //temp_component = MacroOutput->InputBlocks[ind];
        if (input_block->DOT_DrawAsMacro(DrawnMacro) == NULL)
        { // other inputs (from other blocks)
          that_name = input_block->GetComponentName_DOTfile();

          for (ind_sep = 0; ind_sep < space_sep; ind_sep++)
            dot_plik << ' ';
          if (input_block->UsePorts_DOTfile() == true)
            dot_plik << that_name << ":out" << input_block_output_no << " -> ";
          else
            dot_plik << that_name << " -> ";

          if (UsePorts_DOTfile() == true)
            dot_plik <<  macro_output_name << ":in" << ind;
          else
            dot_plik <<  macro_output_name;

          if (input_block->NoOfOutputs > 1)
          {
            dot_plik << " "
                     << input_block->GetComponentEdgeParams_DOTfile(input_block_output_no);
          }
          dot_plik << ";"<< std::endl;

        }
        // other inputs (from other macros) <-- this must be done in the source macro
      }
    }
    dot_plik << std::endl;


    //! \todo unconnected inputs
  }

  void DSP::Macro::MacroToDOTfile(std::ofstream &dot_plik, DSP::Macro_ptr DrawnMacro)
  {
    if (DrawnMacro != this)
    {
      string macro_name;

      ///////////////////////////////
      // Zdefiniuj macro node
      ///////////////////////////////
      macro_name = GetMacroName_DOTfile();

      dot_plik << "    " << macro_name << " " << GetMacroNodeParams_DOTfile() << ";" << std::endl;

      ///////////////////////////////
      // Zdefiniuj po��czenia wychodz�ce
      ///////////////////////////////
      MacroEdgesToDOTfile(dot_plik, macro_name, DrawnMacro);
    }
    else
    {
      string component_name;

      // draw macro input
      component_name = MacroInput_block->GetComponentName_DOTfile();

      dot_plik << "    " << component_name << " " << GetMacroInputNodeParams_DOTfile() << ";" << std::endl;

      MacroInputEdgesToDOTfile(dot_plik, component_name, DrawnMacro);

      // draw macro output
      component_name = MacroOutput_block->GetComponentName_DOTfile();

      dot_plik << "    " << component_name << " " << GetMacroOutputNodeParams_DOTfile() << ";" << std::endl;

      MacroOutputEdgesToDOTfile(dot_plik, component_name, DrawnMacro);
    }
  }

#endif




// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


//Saves scheme information of the algorithm to dot-file
/* ReferenceClock - one of the clocks associated with
 *  the algorithm which we want to store in
 *  dot-file format.
 *
 */
void DSP::Clock::SchemeToDOTfile(DSP::Clock_ptr ReferenceClock, const string &dot_filename,
                                DSP::Macro_ptr DrawnMacro)
{
  UNUSED_RELEASE_ARGUMENT(ReferenceClock);
  UNUSED_RELEASE_ARGUMENT(dot_filename);
  UNUSED_RELEASE_ARGUMENT(DrawnMacro);

  #if __DEBUG__ == 1
    // ********************************** //
    if (ReferenceClock==NULL)
    {
      DSP::log << DSP::LogMode::Error << "DSP::Clock::SchemeToDOTfile" << DSP::LogMode::second << "NULL ReferenceClock" << endl;
      return;
    }

    // ********************************** //
    string tekst;
    std::ofstream dot_plik(dot_filename);
    long int clocks_number, temp;
    DSP::Clock_ptr *ClocksList, temp_clock;
    bool *UsedClocksTable;
    long int max_components_number;
    bool *ComponentDoneTable;
    unsigned int macros_number;
    bool *UsedMacrosTable;
    DSP::Macro_ptr *MacrosList = NULL;
    DSP::Clock_trigger_ptr Clock_trigger;

    // ********************************** //
    dot_plik << "/* This is output of DSP::Clock::SchemeToDOTfile */" << std::endl;
    tekst = DSP_lib_version_string();
    for (unsigned int ind = 0; ind < tekst.length(); ind++)
      if (tekst[ind] == '\n')
        tekst[ind] = ' ';
    dot_plik << "/* " << tekst << " */" << std::endl;
    dot_plik << std::endl;

    dot_plik << "digraph TEST {" << std::endl;

    // ********************************** //
    // reserve space for info whether given component
    // has been already saved
    max_components_number = DSP::Component::GetNoOfComponentsInTable();
    ComponentDoneTable = new bool [max_components_number];
    for (long ind = 0; ind < max_components_number; ind ++)
      ComponentDoneTable[ind] = false;

    //! \todo info which macros must be drawn as the separate component

    // ********************************** //
    // Get list of clocks
    clocks_number = DSP::Clock::GetNoOfClocks();
    ClocksList = new DSP::Clock_ptr[clocks_number];
    temp = DSP::Clock::GetAlgorithmClocks(ReferenceClock, ClocksList, clocks_number, true);
    clocks_number = temp;

    // ********************************** //
    // reserve space for info whether given clock
    // has been used and should be saved
    UsedClocksTable = new bool [clocks_number];
    for (long ind = 0; ind < clocks_number; ind ++)
      UsedClocksTable[ind] = false;

    // ********************************** //
    // UsedMacrosTable generatiojn
    macros_number = DSP::MacroStack::GetCurrentMacroList(MacrosList);
    UsedMacrosTable = new bool[macros_number];
    for (unsigned int ind = 0; ind < macros_number; ind ++)
      UsedMacrosTable[ind] = false;

    // ********************************** //
    // clocks sorting : smaller cycle_length first // prevent MasterClocks grouping
    for (long ind = 1; ind < clocks_number; ind++)
    {
      if (ClocksList[ind]->MasterClockIndex == ClocksList[ind-1]->MasterClockIndex)
      {
        if (ClocksList[ind]->cycle_length < ClocksList[ind-1]->cycle_length)
        {
          // push clock up
          // 1. find where to place this clock
          for (long ind2 = ind-1; ind2 >= 0; ind2--)
          { // bubble up the clock // slow but simple
            if (ClocksList[ind2+1]->cycle_length < ClocksList[ind2]->cycle_length)
            { // ind2+1 <== here this clock should be
              temp_clock = ClocksList[ind2+1];
              ClocksList[ind2+1] = ClocksList[ind2];
              ClocksList[ind2] = temp_clock;
            }
            else
            {
              break;
            }
          }
        }
      }
    }


    // ***************************************** //
    //dot_plik << "  subgraph cluster_BLOCKS {" << std::endl;
    dot_plik << "  subgraph BLOCKS {" << std::endl;
    // Process all clocks in the list
    for (long ind = 0; ind < clocks_number; ind ++)
    {
      UsedClocksTable[ind] |= ClocksList[ind]->ClockComponentsToDOTfile(dot_plik,
                                      ComponentDoneTable, max_components_number,
                                      UsedMacrosTable, MacrosList, macros_number,
                                      UsedClocksTable, ClocksList, clocks_number,
                                      DrawnMacro);

    }
    dot_plik << "    style=invis;" << std::endl;
    dot_plik << "  }" << std::endl << std::endl;

    // ****************************************** //
    // check for clock triggers
    // using ComponentDoneTable to sift out unused components
    for (long ind = 0; ind < max_components_number; ind ++)
    {
      if (ComponentDoneTable[ind] == true)
      {
        Clock_trigger = DSP::Component::GetComponent(ind)->Convert2ClockTrigger();

        if (Clock_trigger != NULL)
        {
          stringstream ss;

          ss << "clock_" << Clock_trigger->SignalActivatedClock;
          dot_plik << "  " << DSP::Component::GetComponent(ind)->GetComponentName_DOTfile() << " -> ";
          dot_plik << ss.str() << " [style=dashed,color="
                << DOT_colors[Clock_trigger->SignalActivatedClock->MasterClockIndex % DOT_colors.size()]
                << "];" << std::endl;

          // mark clock as used
          for (long ind2 = 0; ind2 < clocks_number; ind2++)
          {
            if (ClocksList[ind2] == Clock_trigger->SignalActivatedClock)
              UsedClocksTable[ind2] |= true;
          }
        }
      }
    }

    // ************************************************* //
    //! draw notifications
    if (clocks_number > 0)
    {
      for (long ind = 0; ind < clocks_number; ind ++)
      { // check for all clock: some ne used clock may come up
        UsedClocksTable[ind] |= ClocksList[ind]->ClockNotificationsToDOTfile(dot_plik,
                                        ComponentDoneTable, max_components_number);
                                        //UsedClocksTable, ClocksList, clocks_number);
      }
    }



    // ********************************** //
    // clocks sorting : move used clock higher and update clocks_number
    long ind_clock = 0;
    while (ind_clock < clocks_number)
    {
      if (UsedClocksTable[ind_clock] == true)
      {
        ind_clock++;
      }
      else
      { // push rest of the clocks down and check the same slot
        for (long ind2 = ind_clock+1; ind2 < clocks_number; ind2++)
        {
          ClocksList[ind2-1] = ClocksList[ind2];
          UsedClocksTable[ind2-1] = UsedClocksTable[ind2];
        }
        clocks_number--;
      }
    }


    // ************************************************* //
    // draw used macro blocks
    for (unsigned int ind = 0; ind < macros_number; ind++)
    {
      if (UsedMacrosTable[ind] == true)
      {
        //if (MacrosList[ind] == DrawnMacro) ==> Draw macro input and output
        MacrosList[ind]->MacroToDOTfile(dot_plik, DrawnMacro);
      }
    }

    // ************************************************* //
    // clocks subgraph
    if (clocks_number > 0)
    {
      stringstream ss;
      //dot_plik << "  subgraph cluster_CLOCKS {" << std::endl;
      ss << "    subgraph cluster_clock_group_" << ClocksList[0]->MasterClockIndex << " {";
      dot_plik << ss.str() << std::endl;

      ss.clear(); ss.str("");
      if (ClocksList[0] == MasterClocks[ClocksList[0]->MasterClockIndex]) {
        ss  << "      clock_" << ClocksList[0]
            << " [shape=box,peripheries=2,label = \"cycle length=" << ClocksList[0]->cycle_length
            << "\",color=" << DOT_colors[0] << "];";
      }
      else {
        ss  << "      clock_" << ClocksList[0]
            <<" [shape=box,label = \"cycle length=" << ClocksList[0]->cycle_length
            << "\",color=" << DOT_colors[0] << "];";
      }
      dot_plik << ss.str() << std::endl;

      for (long ind = 1; ind < clocks_number; ind ++)
      {
        if (ClocksList[ind-1]->MasterClockIndex == ClocksList[ind]->MasterClockIndex)
        {
          ss.clear(); ss.str("");
          if (ClocksList[ind] == MasterClocks[ClocksList[ind]->MasterClockIndex]) {
            ss << "      clock_" << ClocksList[ind]
               << " [shape=box,peripheries=2,label = \"cycle length=" << ClocksList[ind]->cycle_length
               << "\",color=" << DOT_colors[ClocksList[ind]->MasterClockIndex % DOT_colors.size()] << "];";
          }
          else {
            ss << "      clock_" << ClocksList[ind]
               << " [shape=box,label = \"cycle length=" << ClocksList[ind]->cycle_length
               << "\",color=" << DOT_colors[ClocksList[ind]->MasterClockIndex % DOT_colors.size()] << "];";
          }
          dot_plik << ss.str() << std::endl;

          ss.clear(); ss.str("");
          ss << "      clock_" << ClocksList[ind-1]
             << " -> clock_" << ClocksList[ind] << ";";
          dot_plik << ss.str() << std::endl;
        }
        else
        {
          ss.clear(); ss.str("");
          ss << "      label = \"Clocks group #" << ClocksList[ind-1]->MasterClockIndex << "\";";
          dot_plik << ss.str() << std::endl;
          dot_plik << "    }" << std::endl;

          ss.clear(); ss.str("");
          ss << "    subgraph cluster_clock_group_" << ClocksList[ind]->MasterClockIndex << " {";
          dot_plik << ss.str() << std::endl;

          ss.clear(); ss.str("");
          if (ClocksList[ind] == MasterClocks[ClocksList[ind]->MasterClockIndex]) {
            ss << "      clock_" << ClocksList[ind]
               << " [shape=box,peripheries=2,label = \"cycle length=" << ClocksList[ind]->cycle_length
               << "\",color=" << DOT_colors[ClocksList[ind]->MasterClockIndex % DOT_colors.size()] << "];";
          }
          else {
            ss << "      clock_" << ClocksList[ind]
               << " [shape=box,label = \"cycle length=" << ClocksList[ind]->cycle_length
               << "\",color=" << DOT_colors[ClocksList[ind]->MasterClockIndex % DOT_colors.size()] << "];";
          }
          dot_plik << ss.str() << std::endl;
        }
        //ClocksList[ind]->L;
        //ClocksList[ind]->M;
      }
      ss.clear(); ss.str("");
      ss << "      label = \"Clocks group #" << ClocksList[clocks_number-1]->MasterClockIndex << "\";";
      dot_plik << ss.str() << std::endl;
      dot_plik << "    }" << std::endl;
      //dot_plik << "    label=\"Algorithm clocks\";" << std::endl;
      //dot_plik << "  }" << std::endl;
    }

    // *********************************** //
    if (ComponentDoneTable != NULL)
      delete [] ComponentDoneTable;
    if (ClocksList != NULL)
      delete [] ClocksList;
    if (UsedClocksTable != NULL)
      delete [] UsedClocksTable;
    if (UsedMacrosTable != NULL)
      delete [] UsedMacrosTable;
    if (MacrosList != NULL)
      delete [] MacrosList;

    dot_plik << "}" << std::endl;
    // ********************************** //
    dot_plik.close();
  #endif
}

#if __DEBUG__ == 1
  //!Saves components information to m-file
  /*! For all components linked with this clock info is stored
   *  in m-file format. Called from DSP::Clock::SchemeToMfile
   */
  void DSP::Clock::ClockComponentsToMfile(std::ofstream &m_plik, bool *ComponentDoneTable, long max_components_number)
  {
    unsigned long ind;

    for (ind=0; ind<NoOfSources; ind++)
    {
      //    ((DSP::Component_ptr)SourcesTable[ind])->ComponentToMfile(m_plik);  // saves component and it's output blocks (except of source & mixed blocks) to m-file
      SourcesTable[ind]->ComponentToMfile(m_plik, ComponentDoneTable, max_components_number);  // saves component and it's output blocks (except of source & mixed blocks) to m-file
    }
  }

  //!Saves components information to dot-file
  /*! For all components linked with this clock info is stored
   *  in dot-file format. Called from DSP::Clock::SchemeToDOTfile
   *
   * Returns true if any of the sources has been drawn.
   */
  bool DSP::Clock::ClockComponentsToDOTfile(std::ofstream &dot_plik,
                             bool *ComponentDoneTable, long max_components_number,
                             bool *UsedMacrosTable, DSP::Macro_ptr *MacrosList, long macros_number,
                             bool *UsedClocksTable, DSP::Clock_ptr *ClocksList, long clocks_number,
                             DSP::Macro_ptr DrawnMacro)
  {
    unsigned long ind;
    bool clock_used;

    clock_used = false;
    for (ind=0; ind<NoOfSources; ind++)
    {
      if (SourcesTable[ind]->DOT_DrawAsMacro(DrawnMacro) == NULL)
      {
        clock_used |= true;
      }

      // saves component and it's output blocks (except of source & mixed blocks) to DOT-file
      SourcesTable[ind]->ComponentToDOTfile(dot_plik,
          ComponentDoneTable, max_components_number,
          UsedMacrosTable, MacrosList, macros_number,
          UsedClocksTable, ClocksList, clocks_number,
          DrawnMacro, this);
    }

    return clock_used;
  }

  bool DSP::Clock::ClockNotificationsToDOTfile(std::ofstream &dot_plik,
                             bool *ComponentDoneTable, long max_components_number)
                             //, bool *UsedClocksTable, DSP::Clock_ptr *ClocksList) long clocks_number)
  {
    unsigned long ind;
    long component_index;
    string block_name;
    string clock_name;
    bool clock_used;

    clock_used = false;

    for (ind=0; ind<NoOfComponents; ind++)
    {
      component_index = DSP::Component::GetComponentIndexInTable(ComponentsNotifications_Table[ind]);
      if (component_index >= max_components_number)
      {
        #ifdef __DEBUG__
          DSP::log << DSP::LogMode::Error << "DSP::Clock::ClockNotificationsToDOTfile"  << DSP::LogMode::second
            << "max_components_number (" << max_components_number
            << ") <= component_index (" << component_index << ")" << endl;
        #endif
      }
      else
      {
        if (ComponentDoneTable[component_index] == true)
        { // component was drawn so we can add notifications now
          //if (ComponentsNotifications_Table[ind]->DOT_DrawAsMacro() == NULL)
          block_name = ComponentsNotifications_Table[ind]->GetComponentName_DOTfile();

          stringstream ss;
          ss << "clock_" << this;
          clock_name = ss.str();

          dot_plik << "    " << clock_name << " -> ";
          dot_plik << block_name << " [style=dashed, constraint=false, color="
                   << DOT_colors[GetMasterClockIndex() % DOT_colors.size()]
                                 << "];" << std::endl << std::endl << std::endl;

          clock_used |= true;
        }
      }
    }

    return clock_used;
  }
#endif

