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


//! Main DSPElib namespace
namespace DSP {
       //! DOT colors table
       extern const std::vector<std::string> DOT_colors;
       
       //! DSPElib sub-namespace for enums definitions
       namespace e {
         enum struct DOTmode {
              DOT_macro_unwrap = 0, //! draw separate macro component (ignore macro structure)
              DOT_macro_wrap = 1, //! as component in parent DOT graph, allow for separate macro graph
              DOT_macro_as_component = 2, //! never unwrap
              DOT_macro_subgraph = 3, //! unwrap as subgraph in parent DOT graph
              DOT_macro_inactive = -1, //! draw inactive
         };
       }
}
#endif
