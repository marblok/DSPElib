/*! \file DSP_misc.cpp
 * This is Miscellaneous DSP functions module main file.
 *
 * \author Marek Blok
 *
 * \todo <b>2006.06.26</b> description of toolbox m-files
 */
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <iomanip>

#ifndef _GNU_SOURCE
  #define  _GNU_SOURCE
#endif
#include <math.h>
#include <algorithm>
//#include <cmath>
//using std::signbit;

#ifdef _MSC_VER
	#include<direct.h>
#endif

#include <DSP_misc.h>
#include <DSP_lib.h>

/*!
 *
 * \addtogroup ver_data
 * @{
 */

DSP_libver VersionData={DSP_VER_MAJOR, DSP_VER_MINOR, DSP_VER_BUILD};

DSP_libver DSP_lib_version(void)
{
  return VersionData;
}

string DSP_lib_version_string()
{
  stringstream tekst;
#ifdef __ANONYMOUS__
  return "";
#else
  tekst << "Signal processing in this application is built on\n"
           "DSP Engine ver. " << int(VersionData.major)
                       << "." << setw(2) << setfill('0') << int(VersionData.minor)
                       << "." << setw(3) << setfill('0') << VersionData.build
        << " 2005-" << DSP_VER_YEAR
        << " Marek Blok";
#endif
  return tekst.str();
}
/* @} ver_data */


//***************************************************//
//  Misc functions
//***************************************************//
unsigned long DSP::f::gcd(unsigned long a, unsigned long b)
{
  unsigned long r;

  while (b!=0)
  {
    r = a % b;
    a=b;
    b=r;
  }

  return a;
}

//void DSP::f::SolveMatrixEqu(int Size, //Size of matrix equation
//                    DSP::Float_ptr A_in, //matrix coeffitients (row after row)
//                    DSP::Float_ptr X,    //vector reserved for solution
//                    DSP::Float_ptr B_in) //right-hand side quantities vector
// Solves matrix equation using Gaussian elimination with backsubstitution
void DSP::f::SolveMatrixEqu(
                    const vector<DSP::Float_vector> &A_in, //!matrix coefficients (table of rows)
                    DSP::Float_vector &X,    //!vector reserved for solution
                    const DSP::Float_vector &B_in) //!right-hand side quantities vector
{
  int i;
  unsigned int j, row, pivot_row;
  DSP::Float max, tempB, factor;

  DSP::Float_vector B; //vector B
  unsigned int Size = (unsigned int)(B_in.size());
  if (Size != A_in.size()) {
    DSP::log << DSP::LogMode::Error << "DSP::f::SolveMatrixEqu" << DSP::LogMode::second << "Size mismatch of matrix A_in and vector B_in" << endl;
    return;
  }
  else {
    //! \TODO check also second dimension of matrix A_in
  }

  vector<DSP::Float_vector> A(Size,DSP::Float_vector(Size));
  for (i = 0; i < int(Size); i++) {
    A[i]=A_in[i]; // copy content of rows
  }
  B = B_in; // copy vector B

  row=0;
  while (row<(Size-1))
  {
    //do pivoting
    // 1) find row with maximum coefficient
    pivot_row=row; max=A[row][row];
    for (i=row; i<int(Size); i++)
    {
      if (max<A[i][row])
      {
        max=A[i][row];
        pivot_row=i;
      }
    }
    // 2) swap rows
    if (pivot_row != row)
    {
//      tempA=A[row]; A[row]=A[pivot_row]; A[pivot_row]=tempA;
      swap(A[row],A[pivot_row]);
      tempB=B[row]; B[row]=B[pivot_row]; B[pivot_row]=tempB;
    }

    //do elimination
    for (i=row+1; i<int(Size); i++)
    {
      factor=A[i][row]/max;
      A[i][row]=0.0;

      for (j=row+1; j<Size; j++)
        A[i][j]-=factor*A[row][j];
      B[i]-=factor*B[row];
    }
    row++;
  }

  //find solution using backsubstitution
  X.resize(Size);
  for (i=Size-1; i>=0; i--)
  {
    X[i]=B[i];
    for (j=i+1; j<Size; j++)
      X[i]-=X[j]*A[i][j];
    X[i]/=A[i][i];
  }
};

void DSP::f::SolveMatrixEqu_prec(
    const vector<DSP::Float_vector> &A_in, //!matrix coefficients (table of rows)
    DSP::Float_vector &X_in,    //!vector reserved for solution
    const DSP::Float_vector &B_in) //right-hand side quantities vector
{
  int i;
  unsigned int j, row, pivot_row;
  DSP::Prec_Float max, tempB, factor;

  DSP::Prec_Float_vector B; //vector B
  unsigned int Size = (unsigned int)(B_in.size());
  if (Size != A_in.size()) {
    DSP::log << DSP::LogMode::Error << "DSP::f::SolveMatrixEqu_prec" << DSP::LogMode::second << "Size mismatch of matrix A_in and vector B_in" << endl;
    return;
  }
  else {
    //! \TODO check also second dimension of matrix A_in
  }

  vector<DSP::Prec_Float_vector> A(Size,DSP::Prec_Float_vector(Size));
  B.resize(Size);
  for (i = 0; i < int(Size); i++) {
    for (j = 0; j < Size; j++) {
      A[i][j]=A_in[i][j]; // copy content of rows
    }
    B[i] = B_in[i]; // copy vector B
  }

  DSP::Prec_Float_vector X; //vector X - for solutions
  X.resize(Size);

  row=0;
  while (row<(Size-1))
  {
    //do pivoting
    // 1) find row with maximum coefficient
    pivot_row=row; max=A[row][row];
    for (i=row; i<int(Size); i++)
    {
      if (max<A[i][row])
      {
        max=A[i][row];
        pivot_row=i;
      }
    }
    // 2) swap rows
    if (pivot_row != row)
    {
//      tempA=A[row]; A[row]=A[pivot_row]; A[pivot_row]=tempA;
      swap(A[row],A[pivot_row]);
      tempB=B[row]; B[row]=B[pivot_row]; B[pivot_row]=tempB;
    }

    //do elimination
    for (i=row+1; i<int(Size); i++)
    {
      factor=A[i][row]/max;
      A[i][row]=0.0;

      for (j=row+1; j<Size; j++)
        A[i][j]-=factor*A[row][j];
      B[i]-=factor*B[row];
    }
    row++;
  }

  //find solution using backsubstitution
  for (i=Size-1; i>=0; i--)
  {
    X[i]=B[i];
    for (j=i+1; j<Size; j++)
      X[i]-=X[j]*A[i][j];
    X[i]/=A[i][i];
  }

  //do solution type conversion
  X_in.resize(Size);
  for (i = 0; i < int(Size); i++)
    X_in[i]= DSP::Float(X[i]);
};

void DSP::f::SolveMatrixEqu_prec(
                        const vector<DSP::Prec_Float_vector> &A_in, //!matrix coefficients (table of rows)
                        DSP::Prec_Float_vector &X_in,    //!vector reserved for solution
                        const DSP::Prec_Float_vector &B_in, //right-hand side quantities vector
                        int use_pivoting) // pivoting mode
{
  int i;
  unsigned int j, row, pivot_row, pivot_col;
  DSP::Prec_Float max, tempB, factor;
  DSP::Prec_Float tempA_val;
  int tempX_ind;

  DSP::Prec_Float_vector B; //vector B
  unsigned int Size = (unsigned int)(B_in.size());
  if (Size != A_in.size()) {
    DSP::log << DSP::LogMode::Error << "DSP::f::SolveMatrixEqu_prec" << DSP::LogMode::second << "Size mismatch of matrix A_in and vector B_in" << endl;
    return;
  }
  else {
    //! \TODO check also second dimension of matrix A_in
  }

  vector<DSP::Prec_Float_vector> A(Size,DSP::Prec_Float_vector(Size));
  B.resize(Size);
  vector<int> X_ind; // indexing of solution samples <== important if column pivoting is used
  X_ind.resize(Size);

  DSP::Prec_Float_vector X; //vector X - for solutions
  X.resize(Size);
  for (i = 0; i < int(Size); i++) {
    A[i]=A_in[i]; // copy content of rows
    for (j = 0; j < Size; j++) {
    }
    X_ind[i] = i;
  }
  B = B_in; // copy vector B

  row=0;
  while (row<Size)
  {
    max=A[row][row];
    if (use_pivoting == 1)
    {
      //do pivoting
      // 1) find row with maximum coefficient
      pivot_row=row;
      for (i=row; i<int(Size); i++)
      {
        if (fabsl(max)<fabsl(A[i][row]))
        {
          max=A[i][row];
          pivot_row=i;
        }
      }
      // 2) swap rows
      if (pivot_row != row)
      {
//        tempA=A[row]; A[row]=A[pivot_row]; A[pivot_row]=tempA;
        swap(A[row],A[pivot_row]);
        tempB=B[row]; B[row]=B[pivot_row]; B[pivot_row]=tempB;
      }

      //do elimination
      for (i=row+1; i<int(Size); i++)
      {
        factor=A[i][row]/max;
        A[i][row]=0.0;

        for (j=row+1; j<Size; j++)
          A[i][j]-=factor*A[row][j];
        B[i]-=factor*B[row];
      }
    }

    if (use_pivoting == 2)
    {
      //do pivoting
      // 1) find row and col with maximum coefficient
      pivot_row=row;
      pivot_col=row;
      for (i=row; i<int(Size); i++)
      {
        for (j=row; j<Size; j++)
        {
          if (fabsl(max)<fabsl(A[i][j]))
          {
            max=A[i][j];
            pivot_row=i;
            pivot_col=j;
          }
        }
      }

      // 2) swap rows
      if (pivot_row != row)
      {
//        tempA=A[row]; A[row]=A[pivot_row]; A[pivot_row]=tempA;
        swap(A[row],A[pivot_row]);
        tempB=B[row]; B[row]=B[pivot_row]; B[pivot_row]=tempB;
      }

      // 3) swap cols and solution index
      for (i=0; i<int(Size); i++)
      { // \note col == row
        tempA_val=A[i][row]; A[i][row]=A[i][pivot_col]; A[i][pivot_col]=tempA_val;
      }
      tempX_ind=X_ind[row]; X_ind[row]=X_ind[pivot_col]; X_ind[pivot_col]=tempX_ind;

      //do elimination to diagonal matrix
      for (i=0; i<int(Size); i++)
      {
        if (i != int(row))
        {
          factor=A[i][row]/max;
          A[i][row]=0.0;

          for (j=row+1; j<Size; j++)
            A[i][j]-=factor*A[row][j];
          B[i]-=factor*B[row];
        }
      }
    }

    row++;
  }

  if ((use_pivoting == 0) || (use_pivoting == 1))
  { // we have upper triangular matrix
    //find solution using backward substitution
    for (i=Size-1; i>=0; i--)
    {
      X[i]=B[i];
      for (j=i+1; j<Size; j++)
        X[i]-=X[j]*A[i][j];
      X[i]/=A[i][i];
    }
  }
  if (use_pivoting == 2)
  { // we have diagonal matrix
    for (i=Size-1; i>=0; i--)
    {
      X[i] = B[i] / A[i][i];
    }
  }

  //do solution type conversion
  X_in.resize(Size);
  for (i = 0; i < int(Size); i++)
  {
    // possible column pivot
    X_in[X_ind[i]]= DSP::Float(X[i]);
  }
};

#ifndef _MSC_VER
//! \bug problem with DSP::f::MakeDir(".", "outputs")
bool DSP::f::MakeDir(const string &dir_name, const string &parent_dir)
{
  string full_name;
  DIR *temp_dir;

  //strncpy(full_name, ".", 1023);
  if (parent_dir.length() == 0)
  {
    full_name = dir_name;
  }
  else
  {
    full_name = parent_dir;

    temp_dir=opendir(full_name.c_str());
    closedir(temp_dir);
    if (temp_dir == NULL)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSP::f::MakeDir" << DSP::LogMode::second << "Parent directory does not exist" << endl;
      #endif
      return false;
    }

    full_name += "/" + dir_name;
  }

  //#ifdef __MINGW32_VERSION
  #ifdef WIN32
    mkdir(full_name.c_str());
  #else
    //#ifdef __GNUC__
    #ifdef S_IRWXG
//    mkdir(full_name,0);
      mkdir(full_name.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);
//    mkdir(full_name,S_IRWXU);
    #else
      mkdir(full_name.c_str());
    #endif
  #endif

  temp_dir=opendir(full_name.c_str());
  closedir(temp_dir);
  if (temp_dir == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::f::MakeDir" << DSP::LogMode::second << "Could not create directory" << endl;
    #endif
    return false;
  }
  return true;
}
#else

bool checkIfDirectoryExists(const string& dirname) {
  WIN32_FIND_DATA data;
  HANDLE hFile = FindFirstFile(dirname.c_str(), &data);
  bool _dir_exists = false;

  _dir_exists = false;
  if (hFile != INVALID_HANDLE_VALUE) // directory doesn't exist
  {
    // is it folder or file?
    FindClose(hFile);
    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      _dir_exists = true;
  }
  return _dir_exists;
}

//! \bug problem with DSP::f::MakeDir(".", "outputs")
bool DSP::f::MakeDir(const string& dir_name, const string& parent_dir)
{
	string full_name;

	//strncpy(full_name, ".", 1023);
	if (parent_dir.length() == 0)
	{
		full_name = dir_name;
	}
	else
	{
		full_name = parent_dir;

		bool _dir_exists = checkIfDirectoryExists(full_name);

		if (_dir_exists == false)
		{
#ifdef __DEBUG__
			DSP::log << DSP::LogMode::Error << "DSP::f::MakeDir", "Parent directory does not exist");
#endif
			return false;
		}

		full_name += "/" + dir_name;
	}

	if (_mkdir(full_name.c_str()) != 0)
	{
#ifdef __DEBUG__
		DSP::log << DSP::LogMode::Error << "DSP::f::MakeDir", "Could not create directory");
#endif
		return false;
	}
	return true;
}
#endif


// Splits directory name into bits and tries to create it
void DSP::f::MakeDir_Ex(const string &dir_name)
{
  string sub_dir;
  string parent_subdir;
  string parent_dir;
  string temp_parent_dir;

#ifndef _MSC_VER
  DIR *temp_dir;
//  int dir_name_len, subdir_len;

//  sub_dir = dir_name;
  // extract parent directory
  parent_dir = ""; sub_dir = "";
  temp_parent_dir = "";
  for (unsigned int ind = 0; ind < dir_name.length(); ind++)
  {
    if ((dir_name[ind] == '/') || (dir_name[ind] == '\\'))
    {
//      temp_parent_dir[ind] = 0x00;
      //check if such parent dir exists
      temp_dir=opendir(temp_parent_dir.c_str());
      closedir(temp_dir);
      if (temp_dir != NULL)
      {
        parent_dir = temp_parent_dir;
//        strcpy(sub_dir, dir_name+ind+1);
        sub_dir = dir_name.substr(ind+1);
      }
      else
        break;
    }
    temp_parent_dir += dir_name[ind];
  }
  temp_dir=opendir(temp_parent_dir.c_str());
  closedir(temp_dir);
  if (temp_dir != NULL)
  {
    parent_dir = temp_parent_dir;
    // directory already exists !!!
    sub_dir = "";
  }
#else
  // extract parent directory
  parent_dir = ""; sub_dir = "";
  temp_parent_dir = "";
  for (unsigned int ind = 0; ind < dir_name.length(); ind++)
  {
	  if ((dir_name[ind] == '/') || (dir_name[ind] == '\\'))
	  {
		  //      temp_parent_dir[ind] = 0x00;
				//check if such parent dir exists
      bool _dir_exists = checkIfDirectoryExists(temp_parent_dir);
      if (_dir_exists == true) {
        parent_dir = temp_parent_dir;
        //        strcpy(sub_dir, dir_name+ind+1);
        sub_dir = dir_name.substr(ind + 1);
      }
      else
        break;
	  }
	  temp_parent_dir += dir_name[ind];
  }

  bool _dir_exists = checkIfDirectoryExists(temp_parent_dir);
  if (_dir_exists == true)
  {
	  parent_dir = temp_parent_dir;
	  // directory already exists !!!
	  sub_dir = "";
  }
#endif

  // ++++++++++++++++++++++++++++++++++++ //
  // create subdirectories
  while (sub_dir.length() != 0)
  {
    // get parent subdir
//    subdir_len = (int)strlen(sub_dir);
    parent_subdir = "";
    for (unsigned int ind = 0; ind < sub_dir.length(); ind++)
    {
      if ((sub_dir[ind] == '/') || (sub_dir[ind] == '\\'))
      {
//        parent_subdir[ind] = 0x00;

//        strcpy(temp_parent_dir, sub_dir+ind+1);
        temp_parent_dir = sub_dir.substr(ind+1);
        sub_dir = temp_parent_dir;
        break;
      }
      parent_subdir += sub_dir[ind];
    }
    if (parent_subdir.compare(sub_dir) == 0)
      sub_dir = "";

    // create parent subdir
    if (DSP::f::MakeDir(parent_subdir, parent_dir) == true)
    {
      parent_dir += "/";
      parent_dir += parent_subdir;
    }
    else
      // problem !!!
      return;
  }
}

/*
static const double rel_error= 1E-12; //calculate 12 significant figures
//you can adjust rel_error to trade off between accuracy and speed
//but don't ask for > 15 figures (assuming usual 52 bit mantissa in a double)


//erf(x) = 2/sqrt(pi)*integral(exp(-t^2),t,0,x)
// = 2/sqrt(pi)*[x - x^3/3 + x^5/5*2! - x^7/7*3! + ...]
// = 1-erfc(x)
// http://www.digitalmars.com/archives/cplusplus/3634.html
double DSP::f::erf(double x)
{
  static const double two_sqrtpi= 1.128379167095512574; // 2/sqrt(pi)
  if (fabs(x) > 2.2)
  {
    return 1.0 - erfc(x); //use continued fraction when fabs(x) > 2.2
  }
  double sum= x, term= x, xsqr= x*x;
  int j= 1;
  do
  {
    term*= xsqr/j;
    sum-= term/(2*j+1);
    ++j;
    term*= xsqr/j;
    sum+= term/(2*j+1);
    ++j;
  }
  while (fabs(term/sum) > rel_error); // CORRECTED LINE

  return two_sqrtpi*sum;
}


//erfc(x) = 2/sqrt(pi)*integral(exp(-t^2),t,x,inf)
// = exp(-x^2)/sqrt(pi) * [1/x+ (1/2)/x+ (2/2)/x+ (3/2)/x+ (4/2)/x+ ...]
// = 1-erf(x)
//expression inside [] is a continued fraction so '+' means add to denominator only
// http://www.digitalmars.com/archives/cplusplus/3634.html
double DSP::f::erfc(double x)
{
  static const double one_sqrtpi= 0.564189583547756287; // 1/sqrt(pi)
  if (fabs(x) < 2.2)
  {
    return 1.0 - erf(x); //use series when fabs(x) < 2.2
  }
  if (signbit(x))
  { //continued fraction only valid for x>0
    return 2.0 - erfc(-x);
  }

  double a=1, b=x; //last two convergent numerators
  double c=x, d=x*x+0.5; //last two convergent denominators
  double q1, q2= b/d; //last two convergents (a/c and b/d)
  double n= 1.0, t;
  do
  {
    t= a*n+b*x;
    a= b;
    b= t;
    t= c*n+d*x;
    c= d;
    d= t;
    n+= 0.5;
    q1= q2;
    q2= b/d;
  }
  while (fabs(q1-q2)/q2 > rel_error);

  return one_sqrtpi*exp(-x*x)*q2;
}
*/

// Symbol error rate estimation for QPSK
DSP::Float DSP::f::SER4QPSK(DSP::Float SNR_lin)
{
  //QPSK: SER_est=1/2*erfc(sqrt(SNR_lin_all/2)); SER_est=SER_est*2
  return (DSP::Float)erfc(sqrt(SNR_lin/2));
}

// Bit error rate estimation for QPSK
DSP::Float DSP::f::BER4BPSK(DSP::Float SNR_lin)
{
  //BPSK: BER_est=1/2*erfc(sqrt(SNR_lin_all));
  return (DSP::Float)erfc(sqrt(SNR_lin))/2;
}

void DSP::f::normalise_window(int size, DSP::Float_ptr buffer)
{
  int n;
  DSP::Float sum = 0.0;
  DSP::Float scale;

  for (n=0; n<size; n++)
  {
    sum += buffer[n];
  }
  scale = 1.0f / sum;
  for (n=0; n<size; n++)
  {
    buffer[n] *= scale;
  }
}

// Blackman window
void DSP::f::Blackman(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = DSP::Float(0.42)
        - DSP::Float(0.5)*COS(DSP::Float(n)*DSP_M_PIx2/DSP::Float(size-1))
        + DSP::Float(0.08)*COS(DSP::Float(2*n)*DSP_M_PIx2/DSP::Float(size-1));
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Hamming window
void DSP::f::Hamming(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = DSP::Float(0.53836)
        - DSP::Float(0.46164)*COS(DSP::Float(n)*DSP_M_PIx2/DSP::Float(size-1));
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// von Hann window
void DSP::f::Hann(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = DSP::Float(0.5)
        - DSP::Float(0.5)*COS(DSP::Float(n)*DSP_M_PIx2/DSP::Float(size-1));
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Rectangular window
void DSP::f::Rectangular(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = 1.0;
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Bartlett window (zero valued end-points)
void DSP::f::Bartlett(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = (2/DSP::Float(size-1))
         * ( DSP::Float(size-1)/2 - FABS( DSP::Float(n) - DSP::Float(size-1)/2 ) );
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Triangular window (zero valued end-points)
void DSP::f::Triangular(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = (2/DSP::Float(size))
         * ( DSP::Float(size)/2 - FABS( DSP::Float(n) - DSP::Float(size-1)/2 ) );
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Bartlett-Hann window
void DSP::f::Bartlett_Hann(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = DSP::Float(0.62) +
         - DSP::Float(0.48) * FABS( DSP::Float(n)/ DSP::Float(size-1) - DSP::Float(0.5) )
         - DSP::Float(0.38) * COS(DSP::Float(n)*DSP_M_PIx2/DSP::Float(size-1));
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Gauss window
void DSP::f::Gauss(int size, DSP::Float_ptr buffer, DSP::Float sigma, bool normalize)
{
  int n;

  if (sigma > 0.5)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::f::Gauss" << DSP::LogMode::second << "Sigma must be less or equal 0.5" << endl;
    #endif
    return;
  }
  for (n=0; n<size; n++)
  {
    buffer[n] =  (DSP::Float(n)-DSP::Float(size-1)/2) / (sigma * DSP::Float(size-1) / 2);
    buffer[n] = EXP( - DSP::Float(0.5) * buffer[n] * buffer[n] );
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Nuttall window, continuous first derivative
void DSP::f::Nuttall(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = DSP::Float(0.355768)
        - DSP::Float(0.487396)*COS(DSP::Float(n)*DSP_M_PIx2/DSP::Float(size-1))
        + DSP::Float(0.144232)*COS(DSP::Float(2*n)*DSP_M_PIx2/DSP::Float(size-1))
        - DSP::Float(0.012604)*COS(DSP::Float(3*n)*DSP_M_PIx2/DSP::Float(size-1));
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Blackman-Harris window, continuous first derivative
void DSP::f::Blackman_Harris(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = DSP::Float(0.35875)
        - DSP::Float(0.48829)*COS(DSP::Float(n)*DSP_M_PIx2/DSP::Float(size-1))
        + DSP::Float(0.14128)*COS(DSP::Float(2*n)*DSP_M_PIx2/DSP::Float(size-1))
        - DSP::Float(0.01168)*COS(DSP::Float(3*n)*DSP_M_PIx2/DSP::Float(size-1));
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Blackman-Nuttall window
void DSP::f::Blackman_Nuttall(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = DSP::Float(0.3635819)
        - DSP::Float(0.4891775)*COS(DSP::Float(n)*DSP_M_PIx2/DSP::Float(size-1))
        + DSP::Float(0.1365995)*COS(DSP::Float(2*n)*DSP_M_PIx2/DSP::Float(size-1))
        - DSP::Float(0.0106411)*COS(DSP::Float(3*n)*DSP_M_PIx2/DSP::Float(size-1));
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

//Flat top window
void DSP::f::Flat_top(int size, DSP::Float_ptr buffer, bool normalize)
{
  int n;

  for (n=0; n<size; n++)
  {
    buffer[n] = DSP::Float(1.0)
        - DSP::Float(1.93)*COS(DSP::Float(n)*DSP_M_PIx2/DSP::Float(size-1))
        + DSP::Float(1.29)*COS(DSP::Float(2*n)*DSP_M_PIx2/DSP::Float(size-1))
        - DSP::Float(0.388)*COS(DSP::Float(3*n)*DSP_M_PIx2/DSP::Float(size-1))
        + DSP::Float(0.032)*COS(DSP::Float(4*n)*DSP_M_PIx2/DSP::Float(size-1));
  }

  if (normalize == true)
    DSP::f::normalise_window(size, buffer);
}

// Normalized sinc function
/*  - Input: buffer contains function arguments
 *   - Output: results are stored in buffer (input values will be overwritten)
 *
 * \f$x(t) = \frac{\sin({\pi}x)}{{\pi}x)\f$
 */
void DSP::f::sinc(int size, DSP::Float_ptr buffer)
{
  int n;
  DSP::Float tmp, x;

  for (n=0; n<size; n++)
  {
    if (FABS(buffer[n]) > 0.000001)
    {
      buffer[n] *= DSP_M_PIx1;
      buffer[n] = SIN(buffer[n])/buffer[n];
    }
    else
    { // Prudnikov et al. 1986, p. 757
      buffer[n] = DSP_M_PIx1 * buffer[n] / 3;

      tmp = SIN(buffer[n]);
      x = 1 - DSP::Float(0.75) * tmp * tmp;

      // four cycles
      for (int k = 2; k <= 4; k++)
      {
        buffer[n] /= 3;
        tmp = SIN(buffer[n]);
        x *= (1 - DSP::Float(0.75) * tmp * tmp);
      }
      buffer[n] = x;
    }
  }
}

template <typename T>
void DSP::f::sinc(const DSP::Float_vector& arguments, vector<T>& output_buffer) {
  unsigned int n;
  DSP::Float x;
  output_buffer.resize(arguments.size());

  for (n = 0; n < arguments.size(); n++)
  {
    x = arguments[n];
    if (FABS(x) > 0.000001)
    {
      x *= DSP_M_PIx1;
      output_buffer[n] = SIN(x) / x;
    }
    else
    { // Prudnikov et al. 1986, p. 757
      DSP::Float tmp, y;

      x = DSP_M_PIx1 * x / 3;

      tmp = SIN(x);
      y = 1 - DSP::Float(0.75) * tmp * tmp;

      // four cycles
      for (int k = 2; k <= 4; k++)
      {
        x /= 3;
        tmp = SIN(x);
        y *= (1 - DSP::Float(0.75) * tmp * tmp);
      }
      output_buffer[n] = y;
    }
  }
}
template void DSP::f::sinc<DSP::Float>(const DSP::Float_vector& arguments, DSP::Float_vector& output_buffer);
template void DSP::f::sinc<DSP::Complex>(const DSP::Float_vector& arguments, DSP::Complex_vector& output_buffer);



// Normalized sinc function
/* \f$x(t) = \frac{\sin({\pi}x)}{{\pi}x)\f$
 */
DSP::Float DSP::f::sinc(DSP::Float x)
{
  DSP::Float tmp, y;

  if (FABS(x) > 0.000001)
  {
    x *= DSP_M_PIx1;
    return SIN(x)/x;
  }
  else
  { // Prudnikov et al. 1986, p. 757
    x = DSP_M_PIx1 * x / 3;

    tmp = SIN(x);
    y = 1 - DSP::Float(0.75) * tmp * tmp;

    // four cycles
    for (int k = 2; k <= 4; k++)
    {
      x /= 3;
      tmp = SIN(x);
      y *= (1 - DSP::Float(0.75) * tmp * tmp);
    }
    return y;
  }
}

// Normalized sinc function
/* \f$x(t) = \frac{\sin({\pi}x)}{{\pi}x)\f$
 */
DSP::Prec_Float DSP::f::sinc_prec(DSP::Prec_Float x)
{
  DSP::Prec_Float tmp, y;

  if (fabsl(x) > 0.000001)
  {
    x *= M_PIx1;
    return sinl(x)/x;
  }
  else
  { // Prudnikov et al. 1986, p. 757
    x = M_PIx1 * x / 3;

    tmp = sinl(x);
    y = 1 - 0.75 * tmp * tmp;

    // four cycles
    for (int k = 2; k <= 4; k++)
    {
      x /= 3;
      tmp = sinl(x);
      y *= (1 - 0.75 * tmp * tmp);
    }
    return y;
  }
}


// returns sample size in bytes for given sample type
int DSP::f::SampleType2SampleSize(DSP::e::SampleType type)
{
  switch (type)
  {
    case DSP::e::SampleType::ST_long_double:
      return sizeof(long double);
    case DSP::e::SampleType::ST_double:
      return sizeof(double);
    case DSP::e::SampleType::ST_float:
      return sizeof(float);
    case DSP::e::SampleType::ST_short:
      return sizeof(short);
    case DSP::e::SampleType::ST_uchar:
      return sizeof(unsigned char);
    case DSP::e::SampleType::ST_bit:
    case DSP::e::SampleType::ST_bit_reversed:
      return 0;
    case DSP::e::SampleType::ST_bit_text:
      return 1;
    case DSP::e::SampleType::ST_none:
    default:
      return -1;
  }
}

//! Least-squares lowpass filter design
/*!
 *  - N - filter impulse responce length (order N-1)
 *  - fp - passband upper frequency
 *  - fs - stopband lower frequency
 *  - h_buffer - user buffer for impulse response
 *  - Ws - stopband error weighting coefficient (Wp = 1)
 *  .
 *
 * \todo Add to the documentation
 */
void DSP::f::LPF_LS (int N, DSP::Float fp, DSP::Float fs, DSP::Float_ptr h_buffer, DSP::Float Ws)
{
  DSP::Prec_Float L;
  int Nodd, temp;
  DSP::Prec_Float b0;
//  DSP::Prec_Float_ptr b;
  DSP::Prec_Float_vector b;
  int I1, I2;
  int ind_1, ind_2;

//  DSP::Prec_Float_ptr *G;
//  DSP::Prec_Float_ptr a;
  vector<DSP::Prec_Float_vector> G;
  DSP::Prec_Float_vector a;

  L=(DSP::Prec_Float(N)-1)/2;
  Nodd = N % 2;

  temp=(N+Nodd)/2;

  if (Nodd == 1)
    // first entry must be handled separately (where k(1)=0)
    // b0 = b0 + 1*fp*(W(0)^2) + 0*fs*(W(1)^2)
    b0 = fp;
  else
    b0 = 0; // this will be overwritten anyway

  b.resize(temp, 0.0);
  b[0] = b0;
  for (ind_1 = Nodd; ind_1 < temp; ind_1++)
    b[ind_1] += fp*DSP::f::sinc_prec((2*(ind_1+0.5)-Nodd)*fp);


  G.resize(temp);
  for (ind_1 = 0; ind_1 < temp; ind_1++)
  {
    G[ind_1].resize(temp, 0.0);
  }
  for (ind_1 = 0; ind_1 < temp; ind_1++)
    for (ind_2 = 0; ind_2 < temp; ind_2++)
    {
      I1 = ind_2 + ind_1 + 1 - Nodd;
      // warto�� na diagonalnej odejmnij od kolumny
      // kolumna ind_2: diag = 2*ind_2 + + 1 - Nodd
      I2 = -ind_2 + ind_1;
      G[ind_1][ind_2] += (.5* fp*(DSP::f::sinc_prec(2*DSP::Float(I1)*fp)+DSP::f::sinc_prec(2*DSP::Float(I2)*fp)));
      G[ind_1][ind_2] += (.5*0.5*(DSP::f::sinc_prec(2*I1*0.5)+DSP::f::sinc_prec(2*I2*0.5))
                         -.5* fs*(DSP::f::sinc_prec(DSP::Float(2*I1)*fs)+DSP::f::sinc_prec(DSP::Float(2*I2)*fs)) )
                        * (Ws*Ws);
    }

  // solve equation
//  a.resize(temp);
  DSP::f::SolveMatrixEqu_prec(G,    //matrix coefficients (row after row)
                           a,    //vector reserved for solution
                           b,    //right-hand side quantities vector
                           2);  // do pivot rows&cols or we get badly scaled matrix
  // fill h_buffer
  ind_2 = temp-1;
  for (ind_1=0; ind_1 < N; ind_1++)
  {
    if (ind_1 < L)
    {
      h_buffer[ind_1] = (DSP::Float)(0.5*a[ind_2]);
      ind_2--;
    }
    else
    {
      if (ind_1 == L)
        h_buffer[ind_1] = (DSP::Float)a[0];
      else
      {
        ind_2++;
        h_buffer[ind_1] = (DSP::Float)(0.5*a[ind_2]);
      }
    }
  }
}

