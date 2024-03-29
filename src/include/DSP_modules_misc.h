/*! \file DSPmodules_misc.h
 * This is DSP engine miscellaneous components and sources
 * definition module header file.
 *
 * \author Marek Blok
 */
#ifndef DSPmodules_miscH
#define DSPmodules_miscH

//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------
#include <DSP_modules.h>
#include <DSP_types.h>

namespace DSP {
  //Version of the morse code table file (*.mct)
  const unsigned long Morse_Table_VER = 1;

  const unsigned long FontsEditEntriesNo = 1;
  const unsigned long MorseCodeEditEntriesNo = 12;
  const unsigned long MaxMorseCodeEntriesNumber = 1024;

  class TMorseTable;
  namespace u {
    class MORSEkey;
  }
}


class DSP::TMorseTable
{
  private:
    static int TablesNo;
    static TMorseTable *FirstTable;

    TMorseTable *NextTable;

    std::string FileName;
    std::string TableDescription;

    void Save2File(const std::string &Name);

    static TMorseTable *Current;

  public:
    //! loads coding table from file *.mct, where Name if filename with full path
    bool LoadFromFile(const std::string &Name);

    std::string FontName[FontsEditEntriesNo];
    uint16_t  FontCharset[FontsEditEntriesNo];
    std::string TestText[FontsEditEntriesNo];

    uint16_t  MorseCodeEntriesNo;
    uint32_t MorseCode[MaxMorseCodeEntriesNumber]; //Converter to number
//    char  CharCode[MaxMorseCodeEntriesNumber];
    char CharCode[MaxMorseCodeEntriesNumber];
    char CharBCode[MaxMorseCodeEntriesNumber];
    unsigned char FontNo[MaxMorseCodeEntriesNumber];

    static int FontCharset2Ind(uint32_t charset);
    static uint32_t Ind2FontCharset(int ind);
    static const std::string Ind2AnsiString(int ind);

    static uint32_t MorseCodeText2Number(const std::string &dot_dash_text);
    /*!
     * @param Number        - numerical representation of MORSE code
     * @return output text
     */
    static std::string Number2MorseCodeText(uint32_t Number);
    //! Converts character into Morse code number
    /*! \warning  works only for single character codes
     */
    uint32_t Char2Number(char znak);

    int MorseCodeText2LetterInd(const std::string &dot_dash_text);

    TMorseTable(void);
    ~TMorseTable(void);

    static int Count(void); //number of tables
    const std::string Description(void);
    static void SelectCurrent(int ind);
    static TMorseTable *GetTable(int ind);
    static int  GetTableNo(TMorseTable *Table);
    static void LoadTables(const std::string &BaseDir);
    static void ReloadCurrentTable(void);
    static bool DeleteCurrentTable(void);
    static bool RenameCurrentTable(const std::string &NewName);
    static void NewTable(void);
    static void FreeTables(void);
    static void SaveCurrent(void);
    static TMorseTable *GetCurrent(void);
};


/**************************************************/
//! ON/OFF Morse code modulation key generator
/*! Inputs and Outputs names:
 *  - Output:
 *   -# "out", "out.re" - real valued keying signal
 *  - Input: none
 *
 *  \todo include DSP::Randomization class if random dot/dash length will be used
 */
class DSP::u::MORSEkey : public DSP::Source // , public DSP::Randomization
{
  private:
    void Init(DSP::Clock_ptr ParentClock);
    TMorseTable MorseTable;

    std::string AsciiText;
    //int current_char; // always first char is the current char

    //! current key state (1.0/0.0 == ON/OFF)
    DSP::Float value;
    int state, morse_state;

    std::string morse_text;
    int current_morse_segment;

    float WPM;
    long sampling_rate;
    float dash2dot_ratio, space2dot_ratio;
    int dot_len, dash_len, space_len;

    int ON_counter, OFF_counter;

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

  public:
    //! set Keying speed in words per minute
    /*! \todo user defined dash/dot ratio and space/dot ratio
     *
     * Speed is set based on the word PARIS
     *  P = di da da di = 2 dots, 2 dashes, 3 in-character spaces, 1 intercharacter space
     *  A = di da       = 1 dot,  1 dash,   1 in-character space,  1 intercharacter space
     *  R = di da di    = 2 dots, 1 dash,   2 in-character spaces, 1 intercharacter space
     *  I = di di       = 2 dots, 0 dashes, 1 in-character space,  1 intercharacter space
     *  S = di di di    = 3 dots, 0 dashes, 2 in-character spaces, 1 interword space
     *
     *  Total:
     *    dots:            10 (x1 = 10)  // dot_len
     *    dashes:           4 (x3 = 12)  // dash_len  = dash2dot_ratio * dot_len
     *    in-char spaces:   9 (x1 =  9)  // dot_len
     *    interchar spaces: 4 (x3 = 12)  // dash_len  = dash2dot_ratio * dot_len
     *    interword spaces: 1 (x7 =  7)  // space_len = space2dot_ratio * dot_len
     *   -------------------------------
     *                            = 50
     */
    void SetKeyingSpeed(float WPM_in, long sampling_rate_in,
        float dash2dot_ratio_in = 3.0, float space2dot_ratio_in = 7.0);
    static float GetDotLength(float WPM_in, long sampling_rate_in,
        float dash2dot_ratio = 3.0, float space2dot_ratio = 7.0);

    bool LoadCodeTable(const std::string &filename);
    //! \TODO add possibilitu to add user table without using file
    void SetTable(const TMorseTable &new_table);

    //! Changes manually current key state
    /*!
     * @param set_to_ON - true (key ON), false (key OFF)
     */
    void SetKeyState(bool set_to_ON);

    //! Append std::string to the characters to transmit
    void AddString(std::string AsciiText_in);
    //! Append char to the characters to transmit
    void AddChar(char znak);

    MORSEkey(DSP::Clock_ptr ParentClock, float WPM_in = 20.0, long sampling_rate_in = 8000);
    ~MORSEkey(void);
};

#endif
