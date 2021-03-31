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

//Version of the morse code table file (*.mct)
#define morse_table_VER 1

#define FontsEditEntriesNo 1
#define MorseCodeEditEntriesNo 12
#define MaxMorseCodeEntriesNumber 1024

class TMorseTable
{
  private:
    static int TablesNo;
    static TMorseTable *FirstTable;
    static const string &BaseDirectory;

    TMorseTable *NextTable;

    string FileName;
    string TableDescription;

    void Save2File(const string &Name);

    static TMorseTable *Current;

  public:
    bool LoadFromFile(const string &Name);

    string FontName[FontsEditEntriesNo];
    WORD  FontCharset[FontsEditEntriesNo];
    string TestText[FontsEditEntriesNo];

    WORD  MorseCodeEntriesNo;
    DWORD MorseCode[MaxMorseCodeEntriesNumber]; //Converter to number
//    char  CharCode[MaxMorseCodeEntriesNumber];
    char CharCode[MaxMorseCodeEntriesNumber];
    char CharBCode[MaxMorseCodeEntriesNumber];
    unsigned char FontNo[MaxMorseCodeEntriesNumber];

    static int FontCharset2Ind(DWORD charset);
    static DWORD Ind2FontCharset(int ind);
    static const string Ind2AnsiString(int ind);

    static DWORD MorseCodeText2Number(const string &dot_dash_text);
    /*!
     * @param Number        - numerical representation of MORSE code
     * @return output text
     */
    static string Number2MorseCodeText(DWORD Number);
    //! Converts character into Morse code number
    /*! \warning  works only for single character codes
     */
    DWORD Char2Number(char znak);

    int MorseCodeText2LetterInd(const string &dot_dash_text);

    TMorseTable(void);
    ~TMorseTable(void);

    static int Count(void); //number of tables
    const string Description(void);
    static void SelectCurrent(int ind);
    static TMorseTable *GetTable(int ind);
    static int  GetTableNo(TMorseTable *Table);
    static void LoadTables(const string &BaseDir);
    static void ReloadCurrentTable(void);
    static bool DeleteCurrentTable(void);
    static bool RenameCurrentTable(const string &NewName);
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
 *  \todo include DSP::Rand class if random dot/dash length will be used
 */
class DSPu_MORSEkey : public DSP::Source // , public DSP::Rand
{
  private:
    void Init(DSP::Clock_ptr ParentClock);
    TMorseTable MorseTable;

    string AsciiText;
    //int current_char; // always first char is the current char

    //! current key state (1.0/0.0 == ON/OFF)
    DSP::Float value;
    int state, morse_state;

    string morse_text;
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

    bool LoadCodeTable(const string &filename);

    //! Changes manually current key state
    /*!
     * @param set_to_ON - true (key ON), false (key OFF)
     */
    void SetKeyState(bool set_to_ON);

    //! Append string to the characters to transmit
    void AddString(string AsciiText_in);
    //! Append char to the characters to transmit
    void AddChar(char znak);

    DSPu_MORSEkey(DSP::Clock_ptr ParentClock, float WPM_in = 20.0, long sampling_rate_in = 8000);
    ~DSPu_MORSEkey(void);
};

#endif
