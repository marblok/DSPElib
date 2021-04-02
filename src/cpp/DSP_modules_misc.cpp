/*! \file DSPmodules.cpp
 * This is DSP engine miscellaneous components
 * and sources definition module main file.
 *
 * \author Marek Blok
 */
//#include <math.h>

#include <DSP_modules.h>
#include <DSP_clocks.h>

#include "DSP_modules_misc.h"

#ifdef WIN32
  #include <windef.h>
  #include <wingdi.h>
#endif // WIN32
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int DSP::TMorseTable::TablesNo=0;
DSP::TMorseTable *DSP::TMorseTable::FirstTable=NULL;
DSP::TMorseTable *DSP::TMorseTable::Current=NULL;
const string &DSP::TMorseTable::BaseDirectory="./";

DSP::TMorseTable::TMorseTable(void)
{
  unsigned long ind;

  TablesNo++;

  FileName = "";
  TableDescription = "";

  NextTable=NULL;

  //Init Fonts   TMenuItem
  FontName[0] = "Courier New";
//! \TODO opracowanie wariantu dla linux'a
#ifdef WIN32
  FontCharset[0]=ANSI_CHARSET; //RUSSIAN_CHARSET;
#else
  FontCharset[0]=0; 
#endif // WIN32

  TestText[0] = "ABCD ĄĆĘ";
  for (ind=1; ind<FontsEditEntriesNo; ind++)
  {
    FontName[ind] = "";
    FontCharset[ind]=UINT16_MAX;
    TestText[ind] = "";
  }

  //Init Code table
  MorseCodeEntriesNo=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='A';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-..."); //Converter to number
  CharCode[MorseCodeEntriesNo]='B';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-.-."); //Converter to number
  CharCode[MorseCodeEntriesNo]='C';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-.."); //Converter to number
  CharCode[MorseCodeEntriesNo]='D';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("."); //Converter to number
  CharCode[MorseCodeEntriesNo]='E';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("..-."); //Converter to number
  CharCode[MorseCodeEntriesNo]='F';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("--."); //Converter to number
  CharCode[MorseCodeEntriesNo]='G';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("...."); //Converter to number
  CharCode[MorseCodeEntriesNo]='H';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".."); //Converter to number
  CharCode[MorseCodeEntriesNo]='I';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".---"); //Converter to number
  CharCode[MorseCodeEntriesNo]='J';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-.-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='K';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".-.."); //Converter to number
  CharCode[MorseCodeEntriesNo]='L';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("--"); //Converter to number
  CharCode[MorseCodeEntriesNo]='M';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-."); //Converter to number
  CharCode[MorseCodeEntriesNo]='N';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("---"); //Converter to number
  CharCode[MorseCodeEntriesNo]='O';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".--."); //Converter to number
  CharCode[MorseCodeEntriesNo]='P';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("--.-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='Q';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".-."); //Converter to number
  CharCode[MorseCodeEntriesNo]='R';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("..."); //Converter to number
  CharCode[MorseCodeEntriesNo]='S';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='T';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("..-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='U';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("...-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='V';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".--"); //Converter to number
  CharCode[MorseCodeEntriesNo]='W';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-..-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='X';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-.--"); //Converter to number
  CharCode[MorseCodeEntriesNo]='Y';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("--.."); //Converter to number
  CharCode[MorseCodeEntriesNo]='Z';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;

  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-----"); //Converter to number
  CharCode[MorseCodeEntriesNo]='0';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".----"); //Converter to number
  CharCode[MorseCodeEntriesNo]='1';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("..---"); //Converter to number
  CharCode[MorseCodeEntriesNo]='2';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("...--"); //Converter to number
  CharCode[MorseCodeEntriesNo]='3';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("....-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='4';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("....."); //Converter to number
  CharCode[MorseCodeEntriesNo]='5';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-...."); //Converter to number
  CharCode[MorseCodeEntriesNo]='6';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("--..."); //Converter to number
  CharCode[MorseCodeEntriesNo]='7';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("---.."); //Converter to number
  CharCode[MorseCodeEntriesNo]='8';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("----."); //Converter to number
  CharCode[MorseCodeEntriesNo]='9';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;

  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".-.-.-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='.';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("--..--"); //Converter to number
  CharCode[MorseCodeEntriesNo]=',';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("---..."); //Converter to number
  CharCode[MorseCodeEntriesNo]=':';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("..--.."); //Converter to number
  CharCode[MorseCodeEntriesNo]='/';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".----."); //Converter to number
  CharCode[MorseCodeEntriesNo]='\'';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-....-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='-';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-..-."); //Converter to number
  CharCode[MorseCodeEntriesNo]='/';
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number("-.--.-"); //Converter to number
  CharCode[MorseCodeEntriesNo]='('; //')'
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".-..-."); //Converter to number
  CharCode[MorseCodeEntriesNo]='\"'; //')'
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
  MorseCode[MorseCodeEntriesNo]=MorseCodeText2Number(".--.-."); //Converter to number
  CharCode[MorseCodeEntriesNo]='@'; //')'
  CharBCode[MorseCodeEntriesNo]=0;
  FontNo[MorseCodeEntriesNo++]=0;
}

int DSP::TMorseTable::FontCharset2Ind(uint32_t charset)
{
  switch (charset)
  {
  /*
  CHINESEBIG5_CHARSET
  HANGEUL_CHARSET
  ? GB2313_CHARSET
  ? EE_CHARSET
  SYMBOL_CHARSET
  */

  //! \TODO opracowanie wariantu dla linux'a
  #ifdef WIN32
    case SHIFTJIS_CHARSET:
      return 9;
    case TURKISH_CHARSET:
      return 8;
    case RUSSIAN_CHARSET:
      return 7;
    case HEBREW_CHARSET:
      return 6;
    case GREEK_CHARSET:
      return 5;
    case EASTEUROPE_CHARSET:
      return 4;
    case BALTIC_CHARSET:
      return 3;
    case ARABIC_CHARSET:
      return 2;
    case ANSI_CHARSET:
      return 1;
    case DEFAULT_CHARSET:
      return 0;
  #endif // WIN32
    default:
      return -1;
  }
}

uint32_t DSP::TMorseTable::Ind2FontCharset(int ind)
{
  switch (ind)
  {
  //! \TODO opracowanie wariantu dla linux'a
  #ifdef WIN32
    case 9:
      return SHIFTJIS_CHARSET;
    case 8:
      return TURKISH_CHARSET;
    case 7:
      return RUSSIAN_CHARSET;
    case 6:
      return HEBREW_CHARSET;
    case 5:
      return GREEK_CHARSET;
    case 4:
      return EASTEUROPE_CHARSET;
    case 3:
      return BALTIC_CHARSET;
    case 2:
      return ARABIC_CHARSET;
    case 1:
      return ANSI_CHARSET;
    case 0:
      return DEFAULT_CHARSET;
  #endif // WIN32
    default:
      return UINT32_MAX;
  }
}


const string DSP::TMorseTable::Ind2AnsiString(int ind)
{
  switch (Ind2FontCharset(ind))
  {
  //! \TODO opracowanie wariantu dla linux'a
  #ifdef WIN32
    case SHIFTJIS_CHARSET:
      return "SHIFTJIS_CHARSET";
    case TURKISH_CHARSET:
      return "TURKISH_CHARSET";
    case RUSSIAN_CHARSET:
      return "RUSSIAN_CHARSET";
    case HEBREW_CHARSET:
      return "HEBREW_CHARSET";
    case GREEK_CHARSET:
      return "GREEK_CHARSET";
    case EASTEUROPE_CHARSET:
      return "EASTEUROPE_CHARSET";
    case BALTIC_CHARSET:
      return "BALTIC_CHARSET";
    case ARABIC_CHARSET:
      return "ARABIC_CHARSET";
    case ANSI_CHARSET:
      return "ANSI_CHARSET";
    case DEFAULT_CHARSET:
      return "DEFAULT_CHARSET";
  #endif //WIN32
    default:
      return "";
  }
}

uint32_t DSP::TMorseTable::MorseCodeText2Number(const string &dot_dash_text)
{
  uint32_t weigth;
  uint32_t Number;

  weigth=1;
  Number=0;
  for (unsigned int n=0; n< dot_dash_text.length(); n++)
  {
    switch (dot_dash_text[n])
    {
      case '.': //treat as one
        Number+=weigth;
        break;
      case '-': //treat as two
        Number+=(2*weigth);
        break;
      default:
        return UINT32_MAX;
    }
    weigth*=3;
  }
  return Number;
}

string DSP::TMorseTable::Number2MorseCodeText(uint32_t Number)
{
  string dash_dot_text = "";

  while (Number!=0)
  {
    switch (Number%3)
    {
      case 1: //dot
        dash_dot_text +='.';
        break;
      case 2: //dash
        dash_dot_text  +='-';
        break;
    }
    Number/=3;
  }

  return dash_dot_text;
}

int DSP::TMorseTable::MorseCodeText2LetterInd(const string &dot_dash_text)
{
  uint32_t number;
  int ind;

  number=MorseCodeText2Number(dot_dash_text);
  ind=0;
  while (ind<MorseCodeEntriesNo)
  {
    if (number==MorseCode[ind])
    {
      return ind;
    }
    ind++;
  }
  return -1;
}

// works only for codes of single charakters
uint32_t DSP::TMorseTable::Char2Number(char znak)
{
  int ind;

  ind=0;
  while (ind<MorseCodeEntriesNo)
  {
    if ((toupper(znak)==CharCode[ind]) && (CharBCode[ind] == 0x00))
    {
      return MorseCode[ind];
    }
    ind++;
  }
  return 0;
}

void DSP::TMorseTable::Save2File(const string &Name)
{
  #if _DEMO_ == 0
    string Dir_FileName;
    unsigned char pomB;
    uint16_t pom;
    unsigned long ind;
    FILE *plik;

    FileName = Name;

    Dir_FileName = BaseDirectory;
    Dir_FileName += "config/";
    Dir_FileName += FileName;

    plik = fopen(Dir_FileName.c_str(), "wb");

    pomB=FontsEditEntriesNo;
    fwrite(&pomB, sizeof(unsigned char), 1, plik);
    pomB=Morse_Table_VER;
    fwrite(&pomB, sizeof(unsigned char), 1, plik);
    for (ind=0; ind<FontsEditEntriesNo; ind++)
    {
      pom=(uint16_t) FontName[ind].length();
      fwrite(&pom, sizeof(uint16_t), 1, plik);
      fwrite(FontName[ind].c_str(), 1, pom, plik);

      fwrite(FontCharset+ind, sizeof(uint16_t), 1, plik);

      pom=(uint16_t) TestText[ind].length();
      fwrite(&pom, sizeof(uint16_t), 1, plik);
      fwrite(TestText[ind].c_str(), 1, pom, plik);
    }

    pom=MorseCodeEntriesNo;
    fwrite(&pom, sizeof(uint16_t), 1, plik);
    for (ind=0; ind<MorseCodeEntriesNo; ind++)
    {
      fwrite(MorseCode+ind, sizeof(uint16_t), 1, plik);

      fwrite(CharCode+ind, sizeof(char), 1, plik);
      fwrite(CharBCode+ind, sizeof(char), 1, plik);

      fwrite(FontNo+ind, sizeof(char), 1, plik);
    }

    fclose(plik);

    Dir_FileName = "";
  #endif // _DEMO_ == 0
}

//---------------------------------------------------------------------------

bool DSP::TMorseTable::LoadFromFile(const string &Name)
{
  string Dir_FileName;
  uint8_t IleFONT;
  uint8_t ver;
  FILE *plik;
  uint16_t Ile, pom;
  std::vector<char> Fake;
  unsigned long ind;

  FileName = Name;

  if (Name.length() > 3)
    TableDescription = Name.substr(0,Name.length()-4); // discard extension
  else
    TableDescription = Name;

  Dir_FileName = BaseDirectory;
  Dir_FileName += "config/";
  Dir_FileName += FileName;

  plik = fopen(Dir_FileName.c_str(), "rb");
  if (plik == NULL)
    return false;

  //Read number of fonts //in general should be one
  if (fread(&IleFONT, sizeof(uint8_t), 1, plik) < 1)
    return false; //File's empty

  //Read file version number
  fread(&ver, sizeof(uint8_t), 1, plik);
  if (ver>Morse_Table_VER)
    return false; //Unknown file version

  //Read font data
  for (ind=0; ind<IleFONT; ind++)
  {
    if (ind<FontsEditEntriesNo)
    {
      FontName[ind] = "";
      fread(&pom, sizeof(uint16_t), 1, plik);
      if (pom!=0)
      {
        vector<char> buffer(pom+1);
        fread(buffer.data(), 1, pom, plik);
        buffer[pom] = 0x00;
        FontName[ind] = buffer.data();
      }

      fread(FontCharset+ind, sizeof(uint16_t), 1, plik);

      TestText[ind] = "";
      fread(&pom, sizeof(uint16_t), 1, plik);
      if (pom!=0)
      {
        vector<char> buffer(pom+1);
        fread(buffer.data(), 1, pom, plik);
        buffer[pom] = 0x00;
        TestText[ind] = buffer.data();
      }
    }
    else
    {
      fread(&pom, sizeof(uint16_t), 1, plik);
      if (pom!=0)
      {
        Fake=std::vector<char>(pom);
        fread(Fake.data(), pom, 1, plik);
      }

      fread(&pom, sizeof(uint16_t), 1, plik);

      fread(&pom, sizeof(uint16_t), 1, plik);
      if (pom!=0)
      {
        Fake=std::vector<char>(pom);
        fread(Fake.data(), pom, 1, plik);
      }
    }
  }

  //Read morse codes
  fread(&Ile, sizeof(uint16_t), 1, plik);
  MorseCodeEntriesNo=0;
  for (ind=0; ind<Ile; ind++)
  {
    if (ind<MaxMorseCodeEntriesNumber)
    {
      fread(MorseCode+ind, sizeof(uint16_t), 1, plik);

      fread(CharCode+ind, sizeof(char), 1, plik);
      switch(ver)
      {
        case 1:
          fread(CharBCode+ind, sizeof(char), 1, plik);
          break;
        default: //0
          CharBCode[ind]=0;
          break;
      }

      fread(FontNo+ind, sizeof(char), 1, plik);

      MorseCodeEntriesNo++;
    }
  }

  fclose(plik);

  Dir_FileName = "";
  return true;
}

void DSP::TMorseTable::FreeTables(void)
{
  TMorseTable *Pom;

  while (FirstTable!=NULL)
  {
    Pom=FirstTable->NextTable;
    delete FirstTable;
    FirstTable=Pom;
  }
  TablesNo=0;
  Current=NULL;
}

//! \warning Not yet implemented
void DSP::TMorseTable::LoadTables(const string &BaseDir)
{
  UNUSED_ARGUMENT(BaseDir);
  //! \todo convert to gcc
  /*
  TSearchRec F;
  bool LoadedSuccessfully;
  AnsiString FileMask;

  BaseDirectory=BaseDir;
  //If there're no files Set one default
  //If there is at least one table replace default table with the first one
  //Subsequent call -> load next file

  if (FirstTable!=NULL)
  {
    FreeTables();
  }


  //First Table should be current
  FirstTable=new TMorseTable;  TablesNo=1;
  Current=FirstTable;

  LoadedSuccessfully=false;
  FileMask=BaseDirectory+"config\\*.mct";
  if (FindFirst(FileMask, faAnyFile, F)==0)
  {
    do
    {
      //Try to load that file
      if (LoadedSuccessfully==false)
      {
        LoadedSuccessfully=Current->LoadFromFile(F.Name);
      }
      else
      {
        Current->NextTable=new TMorseTable;

        if (Current->NextTable->LoadFromFile(F.Name))
        {
          Current=Current->NextTable;
          TablesNo++;
        }
        else
        {
          delete Current->NextTable;
          Current->NextTable=NULL;
        }
      }
    }
    while (FindNext(F)==0);
  }
  FindClose(F);

  if (LoadedSuccessfully==false)
  {
    FreeTables();
    NewTable();
  }

  Current=FirstTable;
  */
}

//! \todo full conversion to gcc
void DSP::TMorseTable::NewTable(void)
{
  //TSearchRec F;
  TMorseTable *pom;
  string Dir_FileName;
  int ind_;

  pom=FirstTable;
  if (pom!=NULL)
  {
    while (pom->NextTable!=NULL)
      pom=pom->NextTable;

    pom->NextTable=new TMorseTable;
    Current=pom->NextTable;
  }
  else
  {
    pom=new TMorseTable;
    FirstTable=pom;
    Current=pom;
  }

  ind_=0;
  Current->TableDescription = "new_00";

  Current->FileName = Current->TableDescription;
  Current->FileName += ".mct";

  Dir_FileName = Current->BaseDirectory;
  Dir_FileName += "config/";
  Dir_FileName += Current->FileName;

  /*
  while (FindFirst(Dir_FileName, faAnyFile, F)==0)
  {
    ind_++;

    if (ind_==100)
      break;
    Current->TableDescription[5]=char(48+ind_/10);
    Current->TableDescription[6]=char(48+ind_%10);

    Current->FileName=Current->TableDescription+".mct";
    Dir_FileName=BaseDirectory+"config\\";
    Dir_FileName=Dir_FileName+Current->FileName;

    FindClose(F);
  }
  */

  if (ind_==100)
  {
    if (pom->NextTable==NULL)
    {
      delete pom;
      pom=NULL;
      FirstTable=NULL;
    }
    else
    {
      delete pom->NextTable;
      pom->NextTable=NULL;
    }
    Current=pom;
  }
  else
    SaveCurrent();

  Dir_FileName = "";
}

void DSP::TMorseTable::SaveCurrent(void)
{
  if (Current!=NULL)
  {
    Current->Save2File(Current->FileName);
  }
}

//! \todo convert to gcc
bool DSP::TMorseTable::RenameCurrentTable(const string &NewName)
{
  UNUSED_ARGUMENT(NewName);
/*
  char *Dir_FileName;
  char *Dir_FileNameOld;
  char *Dir_FileNameNew;

  if (Current!=NULL)
  {
//    Current->Save2File(NewName);
    Dir_FileName=BaseDirectory+"config\\";
    Dir_FileNameOld=Dir_FileName+Current->FileName;
    Dir_FileNameNew=Dir_FileName+NewName+".mct";

    if (MoveFile(Dir_FileNameOld.c_str(), // address of name of the existing file
            Dir_FileNameNew.c_str()))   // address of new name for the file
    {
      Current->FileName=NewName+".mct";
      Current->TableDescription=Current->FileName.SubString(1,Current->FileName.Length()-4);
    }
    else
    {
      Application->MessageBox(
        "A problem occured while renaming table!\n"
        " a) new name may contain invalid characters (such as ?,-,*)\n"
        " b) file with such name may already exist\n"
        " c) table file might be read only!",
        "Table rename failed !!!",
        MB_OK | MB_ICONWARNING);
      return false;
    }
  }
  */
  return true;
}

const string DSP::TMorseTable::Description(void)
{
  return TableDescription;
}

DSP::TMorseTable *DSP::TMorseTable::GetCurrent(void)
{
  return Current;
}

DSP::TMorseTable *DSP::TMorseTable::GetTable(int ind)
{
  int ind_temp=0;
  TMorseTable *pom=FirstTable;

  while (pom!=NULL)
  {
    if (ind_temp==ind)
      break;
    pom=pom->NextTable;
    ind_temp++;
  }
  return pom;
}

int  DSP::TMorseTable::GetTableNo(DSP::TMorseTable *Table)
{
  int ind_temp=0;
  TMorseTable *pom=FirstTable;

  while (pom!=NULL)
  {
    if (pom==Table)
      break;
    pom=pom->NextTable;
    ind_temp++;
  }
  if (pom==NULL)
    return -1;
  else
    return ind_temp;
}

void DSP::TMorseTable::SelectCurrent(int ind)
{
  int ind_temp=0;
  TMorseTable *pom=FirstTable;

  while (pom!=NULL)
  {
    if (ind_temp==ind)
      break;
    pom=pom->NextTable;
    ind_temp++;
  }
  if (pom!=NULL)
    Current=pom;
  else
    Current=FirstTable;
}

//returns number of tables
int DSP::TMorseTable::Count(void)
{
  return TablesNo;
}

DSP::TMorseTable::~TMorseTable(void)
{
  unsigned long ind;

  for (ind=0; ind<FontsEditEntriesNo; ind++)
  {
    FontName[ind] = "";
    FontCharset[ind]=UINT16_MAX;
    TestText[ind] = "";
  }

  TableDescription = "";

  TablesNo--;
}

void DSP::TMorseTable::ReloadCurrentTable(void)
{
  if (Current!=NULL)
    Current->LoadFromFile(Current->FileName);
};






// ***************************************************** //
// generates MORSE code keying signal
DSP::u::MORSEkey::MORSEkey(DSP::Clock_ptr ParentClock,
    float WPM_in, long sampling_rate_in)
  : DSP::Source()
{
  SetName("MORSEkey", false);

  SetNoOfOutputs(1);
  DefineOutput("out", 0);
  DefineOutput("out.re", 0);
  IsMultiClock=false;

  //! \todo user defined dash/dot ratio and space/dot ratio
  //dash2dot_ratio = 3.0;  space2dot_ratio = 7.0;
  // calculate dot_len based on WPM and sampling rate
  SetKeyingSpeed(WPM_in, sampling_rate_in, 3.0, 7.0);
  //dot_len = 1000;
  //dash_len  = dash2dot_ratio  * dot_len;
  //space_len = space2dot_ratio * dot_len;

  Init(ParentClock);

  AsciiText = "";
  //current_char = 0;

  value = 0.0;
  state = 0;  morse_state = 0;
  morse_text = "";
  current_morse_segment = 0;

  ON_counter = 0; OFF_counter = 0;

  OutputExecute_ptr = &OutputExecute;
}

void DSP::u::MORSEkey::Init(DSP::Clock_ptr ParentClock)
{
  RegisterOutputClock(ParentClock);
};

bool DSP::u::MORSEkey::LoadCodeTable(const string &filename)
{
  return MorseTable.LoadFromFile(filename);
}

DSP::u::MORSEkey::~MORSEkey(void)
{
}

float DSP::u::MORSEkey::GetDotLength(float WPM_in, long sampling_rate_in,
    float dash2dot_ratio, float space2dot_ratio)
{
  float dots_per_minute;
  float dots_per_word;
  float dot_len_float;

  // get dots per word
  //    dots:            10 (x1 = 10)  // dot_len
  dots_per_word = 10;
  //    dashes:           4 (x3 = 12)  // dash_len  = dash2dot_ratio * dot_len
  dots_per_word += (4 * dash2dot_ratio);
  //    in-char spaces:   9 (x1 =  9)  // dot_len
  dots_per_word += 9;
  //    interchar spaces: 4 (x3 = 12)  // dash_len  = dash2dot_ratio * dot_len
  dots_per_word += (4 * dash2dot_ratio);
  //    interword spaces: 1 (x7 =  7)  // space_len = space2dot_ratio * dot_len
  dots_per_word += (1 * space2dot_ratio);

  // get dots per minute
  dots_per_minute = WPM_in * dots_per_word;

  dot_len_float = float(double(sampling_rate_in*60) / dots_per_minute);

  return dot_len_float;
}

void DSP::u::MORSEkey::SetKeyingSpeed(float WPM_in, long sampling_rate_in,
    float dash2dot_ratio_in, float space2dot_ratio_in)
{
  float dot_len_float;

  WPM = WPM_in;
  sampling_rate = sampling_rate_in;

  dash2dot_ratio = dash2dot_ratio_in;
  space2dot_ratio = space2dot_ratio_in;

  dot_len_float = GetDotLength(WPM, sampling_rate, dash2dot_ratio, space2dot_ratio);

  dot_len = (int)(dot_len_float+0.5);
  dash_len = (int)(dash2dot_ratio * dot_len_float+0.5);
  space_len = (int)(space2dot_ratio * dot_len_float+0.5);
}

void DSP::u::MORSEkey::AddString(string AsciiText_in)
{
  //! TODO mutex
  AsciiText += AsciiText_in;
}

void DSP::u::MORSEkey::AddChar(char znak)
{
  //! TODO mutex
  AsciiText += znak;
}

void DSP::u::MORSEkey::SetKeyState(bool set_to_ON)
{
  if (set_to_ON == true)
    value = 1.0;
  else
    value = 0.0;
}

#define THIS ((DSP::u::MORSEkey *)source)
/* state == 0
 *  - start transmit new char
 *    - get char
 *    - get Morse code
 *    - reset morse_state to 0
 *  - transmit current Morse code based on morse_state
 * state == 1
 *  - output 0.0 == transmission ended
 *
 * morse_state == 0
 *  - get first code char
 *  - set state ON_counter and OFF_counter
 *  - if transmission end detected set state = 1 and morse_state = 3
 * morse_state == 1
 *  - 1.0 (ON)
 *  - decrease ON_counter
 *  - if ON_counter == 0,
 *    - morse_state == 2,
 * morse_state == 2
 *  - 0.0 (OFF)
 *  - decrease OFF_counter
 *  - if OFF_counter == 0,
 *    - morse_state == 0,
 * morse_state == 3
 *  - 0.0 (OFF) - transmission end
 */
bool DSP::u::MORSEkey::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);
  uint32_t code_number;
  char znak;
  //int ind;

  switch (THIS->state)
  {
    case 0:
      // start transmit new char
      //! TODO mutex
      if (THIS->AsciiText.length() > 0) {
        znak = THIS->AsciiText[0];
        //THIS->AsciiText = THIS->AsciiText.substr(1); // remove it
        THIS->AsciiText.erase(THIS->AsciiText.begin());
      }
      else {
        znak = 0x00;
      }


      if (znak == 0x00)
      {
        //THIS->state = 2;
        THIS->morse_state = 4;
      }
      else
      {
        code_number = THIS->MorseTable.Char2Number(znak);

        if (code_number != 0)
          THIS->morse_text = THIS->MorseTable.Number2MorseCodeText(code_number);
        else
        {
          THIS->morse_text = " ";
        }
        THIS->state = 1;
        THIS->morse_state = 0;
        THIS->current_morse_segment = 0;
      }
      break;

    case 1:
    case 2:
    default:
      break;
  }

  switch (THIS->morse_state)
  {
    case 0:
      if (THIS->current_morse_segment >= (int)(THIS->morse_text.length()))
      {
        // end of char
        THIS->ON_counter = 0;
        // one dot_len space already used after last dot/dash
        THIS->OFF_counter = THIS->dash_len - THIS->dot_len; //2000;
        THIS->morse_state = 3;
        //value = 0.0;
      }
      else
      {
        if (THIS->morse_text[THIS->current_morse_segment] == '.')
        {
          THIS->ON_counter = THIS->dot_len; //1000;
          THIS->OFF_counter = THIS->dot_len; //1000;

          THIS->morse_state = 1;
          THIS->value = 1.0;
        }
        else
          if (THIS->morse_text[THIS->current_morse_segment] == '-')
          {
            THIS->ON_counter = THIS->dash_len; //3000;
            THIS->OFF_counter = THIS->dot_len; //1000;

            THIS->morse_state = 1;
            THIS->value = 1.0;
          }
          else
          { // word end
            THIS->ON_counter = 0;
            THIS->OFF_counter = THIS->space_len - THIS->dot_len;
            THIS->morse_state = 2;
            //value = 0.0;
          }
        THIS->current_morse_segment++;
      }
      break;

    case 1:
      // key ON
      //value = 1.0;
      THIS->ON_counter--;
      if (THIS->ON_counter <= 0)
      {
        THIS->morse_state = 2;
        THIS->value = 0.0;
      }
      break;

    case 2:
      // key OFF
      //value = 0.0;
      THIS->OFF_counter--;
      if (THIS->OFF_counter <= 0)
        THIS->morse_state = 0;
      break;

    case 3:
      // key OFF - end of char
      //value = 0.0;
      THIS->OFF_counter--;
      if (THIS->OFF_counter <= 0)
      {
        THIS->morse_state = 0;
        THIS->state = 0; // start transmit new char
      }
      break;

    case 4:
    default:
      // transmission end
      //value = 0.0;
      break;
  }

  #ifdef __DEBUG__
    THIS->OutputBlocks[0]->Execute_ptr(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0], THIS->value, source);
  #else
    THIS->OutputBlocks[0]->Execute_ptr(
        THIS->OutputBlocks[0], THIS->OutputBlocks_InputNo[0], THIS->value);
  #endif

  return true;
};
#undef THIS
