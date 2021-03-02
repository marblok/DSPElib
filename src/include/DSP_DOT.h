/*! \file DSP_DOT.h
 * DOT graph support
 *
 * \author Marek Blok
 */
//---------------------------------------------------------------------------
#ifndef DSP_DOT_H
#define DSP_DOT_H

#include <string>
#include <vector>
using namespace std;

//extern const int DOT_colors_len;
//! DOT colors table
extern const vector<string> DOT_colors;

enum DSPe_DOTmode {
       DSP_DOT_macro_unwrap = 0, //! draw separate macro component (ignore macro structure)
       DSP_DOT_macro_wrap = 1, //! as component in parent DOT graph, allow for separate macro graph
       DSP_DOT_macro_as_component = 2, //! never unwrap
       DSP_DOT_macro_subgraph = 3, //! unwrap as subgraph in parent DOT graph
       DSP_DOT_macro_inactive = -1, //! draw inactive
};

#endif
