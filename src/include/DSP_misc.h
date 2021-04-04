//---------------------------------------------------------------------------
/*! \file DSP_misc.h
 * This is DSP engine auxiliary functions module header file.
 *
 * \author Marek Blok
 */
/*!
 * \defgroup misc_func Miscellaneous functions
 *
 * \defgroup ver_data Version and major changes information
 *
 *
 * \addtogroup misc_func
 * @{
 */
#ifndef DSP_misc_H
#define DSP_misc_H

#ifdef __GNUC__
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <dirent.h>
#elif defined(__BORLANDC__)
  #include <direct.h>
#elif defined(_MSC_VER)
	#pragma message ("test _MSC_VER")
	//#define _WINSOCK_DEPRECATED_NO_WARNINGS
	//#define _CRT_SECURE_NO_WARNINGS
	#pragma warning (disable : 4250 ) // do zweryfikowania: dziedziczenie jednej metody z dw�ch klas bazowych

	#if defined(_AMD64_)
		#pragma message ("_AMD64_")
	#elif defined(_X86_)
		#pragma message ("_X86_")
	#endif
	#if defined(_WIN32)
		#pragma message ("_WIN32")
	#endif

#else
  #error Compiler not supported
#endif

#include <fstream>
#include <string>

using namespace std;

//---------------------------------------------------------------------------
#include <DSP_setup.h>
#include <DSP_logstream.h>
//---------------------------------------------------------------------------
#include <DSP_types.h>
// All function names should begin with DSP::f::

namespace DSP {
  namespace e {
    enum struct LoadCoef_Type : unsigned int; // DSPe_LoadCoef_Type
    LoadCoef_Type operator|(LoadCoef_Type __a, LoadCoef_Type __b);
  }
  class LoadCoef;

  namespace f {
    //! Greatest Common Dividor
    unsigned long gcd(unsigned long a, unsigned long b);

    //! Solves matrix equation using Gaussian elimination with backsubstitution
    void SolveMatrixEqu(const vector<DSP::Float_vector> &A_in, //! square matrix of coefficients (table of rows)
                        DSP::Float_vector &X,    //!vector reserved for solution
                        const DSP::Float_vector &B_in); //!right-hand side quantities vector

    //! Solves matrix equation using Gaussian elimination with backsubstitution
    /*! All calculations are internally done at high precision
    */
    void SolveMatrixEqu_prec(
                        const vector<DSP::Float_vector> &A_in, //!matrix coefficients (table of rows)
                        DSP::Float_vector &X_in,    //!vector reserved for solution
                        const DSP::Float_vector &B_in); //!right-hand side quantities vector
    //! Solves matrix equation using Gaussian elimination with backward substitution
    /*! All calculations are internally done at high precision based on high precision input coefficients
    * \note suggested use_pivoting = 2
    * \note use_pivoting = 0 should not be used
    */
    void SolveMatrixEqu_prec(
                        const vector<DSP::Prec_Float_vector> &A_in, //!matrix coefficients (table of rows)
                        DSP::Prec_Float_vector &X_in,    //!vector reserved for solution
                        const DSP::Prec_Float_vector &B_in, //!right-hand side quantities vector
                        int use_pivoting); //! pivoting mode: 0-none; 1-rows; 2-rows&cols

    //! Minimax filter design based on real Remez exchange algorithm
    /*! - N - filter impulse response length (order N-1)
    *  - h_buffer - buffer for impulse response
    * also needed: filter prototype in frequency domain,
    * weighting function, approximation bandwidths, ...
    */
    void RealRemez(int N, DSP::Float_vector &h_buffer);

    //! Least-squares lowpass filter design
    /*!
    *  - N - filter impulse responce length (order N-1)
    *  - fp - passband upper frequency
    *  - fs - stopband lower frequency
    *  - Ws - stopband error weighting coefficient (Wp = 1)
    *  - h_buffer - user buffer for impulse response
    *  .
    */
    void LPF_LS (int N, DSP::Float fp, DSP::Float fs, DSP::Float_vector &h_buffer, DSP::Float Ws = 1.0);

    //! Creates subdirectory in parent_dir
    /*! \warning if parent_dir != NULL then it must exist.
    *  \warning Will create single subdirectory (no nested subdir creation)
    */
    bool MakeDir(const string &dir_name, const string &parent_dir = "");
    //! Splits directory name into bits and tries to create it subdirectory by subdirectory
    void MakeDir_Ex(const string &dir_name);


    //! Symbol error rate estimation for QPSK
    DSP::Float SER4QPSK(DSP::Float SNR_lin);
    //! Bit error rate estimation for QPSK
    DSP::Float BER4BPSK(DSP::Float SNR_lin);

    //! implemented in DSP_IO.cpp
    void Sleep(uint32_t time);
    //! implemented in DSP_IO.cpp
    void SetSleepFunction(DSP::ExternalSleep_ptr new_function);

    //! Divides each window sample by sum of all samples.
    /*! Gives window with frequency response equal 1 at 0 frequency.
    */
    void normalise_window(int size, DSP::Float_ptr buffer);

    //! Blackman window
    /*! \f$w[n] = 0.42 - 0.5\cos\left(\frac{2{\pi}n}{N-1}\right)
            + 0.08\cos\left(\frac{4{\pi}n}{N-1}\right)\f$
    */
    void Blackman(int size, DSP::Float_ptr buffer, bool normalize = false);

    //! Hamming window
    /*! \f$w[n] = 0.53836 - 0.46164\cos\left(\frac{2{\pi}n}{N-1}\right)\f$
    */
    void Hamming(int size, DSP::Float_ptr buffer, bool normalize = false);

    //! von Hann window
    /*! \f$w[n] = 0.5 - 0.5\cos\left(\frac{2{\pi}n}{N-1}\right)\f$
    */
    void Hann(int size, DSP::Float_ptr buffer, bool normalize = false);

    //! Bartlett window (zero valued end-points)
    /*! \f$w[n] = \frac{2}{N-1} \left(
    *            \frac{N-1}{2} - \left| n - \frac{N-1}{2} \right|
    *            \right) \f$
    */
    void Bartlett(int size, DSP::Float_ptr buffer, bool normalize = false);

    //! Triangular window (non-zero end-points)
    /*! \f$w[n] = \frac{2}{N} \left(
    *            \frac{N}{2} - \left| n - \frac{N-1}{2} \right|
    *            \right) \f$
    */
    void Triangular(int size, DSP::Float_ptr buffer, bool normalize = false);

    //! Bartlett-Hann window
    /*! \f$w[n] = 0.62
    *          - 0.48 \left| \frac{n}{N-1} - \frac{1}{2} \right|
    *          - 0.38 \cos\left(\frac{2{\pi}n}{N-1}\right)\f$
    */
    void Bartlett_Hann(int size, DSP::Float_ptr buffer, bool normalize = false);

    //! Gauss window
    /*! \f$w[n] = \exp ^ {
    *          - \frac{1}{2}
    *            \left(
    *              \frac
    *                   {n - (N-1)/2}
    *                   {\sigma(N-1)/2}
    *            \right) ^ 2
    *          }\f$
    * where \f$\sigma <= 0.5\f$
    */
    void Gauss(int size, DSP::Float_ptr buffer, DSP::Float sigma, bool normalize = false);

    //! Rectangular window
    /*! \f$w[n] = 1\f$
    */
    void Rectangular(int size, DSP::Float_ptr buffer, bool normalize = false);

    //! Nuttall window, continuous first derivative
    /*! \f$w[n] = 0.355768
    *          - 0.487396 \cos\left(\frac{2{\pi}n}{N-1}\right)
    *          + 0.144232 \cos\left(\frac{4{\pi}n}{N-1}\right)
    *          - 0.012604 \cos\left(\frac{6{\pi}n}{N-1}\right)
    * \f$
    */
    void Nuttall(int size, DSP::Float_ptr buffer, bool normalize = false);

    //! Blackman-Harris window, continuous first derivative
    /*! \f$w[n] = 0.35875
    *          - 0.48829 \cos\left(\frac{2{\pi}n}{N-1}\right)
    *          + 0.14128 \cos\left(\frac{4{\pi}n}{N-1}\right)
    *          - 0.01168 \cos\left(\frac{6{\pi}n}{N-1}\right)
    * \f$
    */
    void Blackman_Harris(int size, DSP::Float_ptr buffer, bool normalize = false);

    //!Blackman-Nuttall window
    /*! \f$w[n] = 0.3635819
    *          - 0.4891775 \cos\left(\frac{2{\pi}n}{N-1}\right)
    *          + 0.1365995 \cos\left(\frac{4{\pi}n}{N-1}\right)
    *          - 0.0106411 \cos\left(\frac{6{\pi}n}{N-1}\right)
    * \f$
    */
    void Blackman_Nuttall(int size, DSP::Float_ptr buffer, bool normalize = false);

    //!Flat top window
    /*! \f$w[n] = 1
    *          - 1.93 \cos\left(\frac{2{\pi}n}{N-1}\right)
    *          + 1.29 \cos\left(\frac{4{\pi}n}{N-1}\right)
    *          - 0.388 \cos\left(\frac{6{\pi}n}{N-1}\right)
    *          + 0.032 \cos\left(\frac{8{\pi}n}{N-1}\right)
    * \f$
    */
    void Flat_top(int size, DSP::Float_ptr buffer, bool normalize = false);

    //! Normalized sinc function
    /*!  - Input: buffer contains funtion argumets
    *   - Output: results are stored in buffer (input values will be overwritten)
    *
    * \f$x(x) = \frac{\sin({\pi}x)}{{\pi}x)}\f$
    */
    void sinc(int size, DSP::Float_ptr buffer);
    //! Normalized sinc function
    /*!  - Input: arguments contains funtion argumets
    *   - Output: returns results
    *
    * \f$x(x) = \frac{\sin({\pi}x)}{{\pi}x)}\f$
    */
    template <typename T>
    void sinc(const DSP::Float_vector& arguments, vector<T> &output_buffer);

    //! Normalized sinc function
    /*!
    * \f$x(x) = \frac{\sin({\pi}x)}{{\pi}x)}\f$
    */
    DSP::Float sinc(DSP::Float x);
    DSP::Prec_Float sinc_prec(DSP::Prec_Float x);

    //! Saves to *.flt file samples from given real valued vector with information out its sampling rate
    bool SaveVector(const std::string &filename, const DSP::Float_vector &vector, const unsigned int &Fp=0);
    //! Saves to *.flt file samples from given complex valued vector with information out its sampling rate
    bool SaveVector(const std::string &filename, const DSP::Complex_vector &vector, const unsigned int &Fp=0);

  
    //! returns sample size in bytes for given sample type
    int SampleType2SampleSize(DSP::e::SampleType type);
  }
}

// ***************************************************** //
// ***************************************************** //
// ***************************************************** //


/*! \addtogroup load_func Config/data files processing functions
 * @{
 */
//! LoadCoeffient file type enumerations
/*! Several options may be used together
 */
enum struct DSP::e::LoadCoef_Type : unsigned int // DSPe_LoadCoef_Type
{
  error = 0, // DSP_LC_error

  real  = 1, // DSP_LC_real
  complex  = real << 1, // DSP_LC_complex

  FIR = complex << 1, // DSP_LC_FIR
  IIR = FIR << 1, // DSP_LC_IIR

  FIR_real    = (FIR | real), // DSP_LC_FIR_real
  FIR_complex = (FIR | complex), // DSP_LC_FIR_complex
  IIR_real    = (IIR | real), // DSP_LC_IIR_real
  IIR_complex = (IIR | complex) // DSP_LC_IIR_complex

};
// DSP::e::LoadCoef_Type operator|(DSP::e::LoadCoef_Type __a, DSP::e::LoadCoef_Type __b)
// { 
//   return DSP::e::LoadCoef_Type(static_cast<int>(__a) | static_cast<int>(__b));
// }
inline DSP::e::LoadCoef_Type DSP::e::operator|(DSP::e::LoadCoef_Type __a, DSP::e::LoadCoef_Type __b)
{ 
  return static_cast<DSP::e::LoadCoef_Type>(static_cast<std::underlying_type<DSP::e::LoadCoef_Type>::type>(__a) 
                                          | static_cast<std::underlying_type<DSP::e::LoadCoef_Type>::type>(__b));
}


//! LoadCoeffient file info structure
/*! Stores information retrieved from coefficients file
 */
class DSP::LoadCoef
{
  public:
    string filename; // file name with path

    unsigned char file_version;

    DSP::e::LoadCoef_Type type; // DSPe_LoadCoef_Type

    DSP::e::SampleType sample_type;
    int sample_size; //! sample component size in bytes
    unsigned char sample_dim; //! sample dimension: 1 - real, 2 - complex, ...

    int NoOfVectors; //! -1 if unsupported format
    unsigned int Fp; //! sampling frequency (valid if file_version > 0)

    int header_size; //! number of bytes in header

    //! constructor
    LoadCoef(void)
    {
      file_version = 0xff;
      filename[0] = 0x00;

      header_size = 0;
      sample_dim = 0;

      type = DSP::e::LoadCoef_Type::error;
      sample_type = DSP::e::SampleType::ST_none;
      sample_size = -1;
      NoOfVectors = -1;
      Fp = 0;
    }

    //! Opens Coefficients file & fills structure
    /*! File format (*.coef) - this is open format, for general use
     *  (not only for storing coefficients)
     *
     *  - (uchar) 1B - file version number
     *  -  data - coefficients data (depends on file version)
     *  .
     *  Data segment format:
     *  -# (version: >= 0x01)
     *   -  (uint) 4B - sampling frequency
     *  -# (version: >= 0x00)
     *   -  (uchar) 1B - number of sample dimensions
     *         1 - real, 2 - complex, ...
     *   -  (uchar) 1B - sample component type
     *    - DSP::e::FileType::FT_float (=1) : C++ float (32bit floating point)
     *    - DSP::e::FileType::FT_short (=2) : C++ short (16bit signed integer)
     *    - DSP::e::FileType::FT_uchar (=3) : C++ unsigned char (8bit unsigned integer with bias (0x80))
     *    - DSP::e::FileType::FT_double (=7) : C++ double (64bit floating point)
     *    - DSP::e::FileType::FT_long_double (=8) : C++ long double (80bit floating point)
     *   -  (uchar) 1B - number of vectors
     *    -   1 - FIR filter coefficients (one vector)
     *    -   2 - IIR filter coefficients (two vectors)
     *   -  (x number of vectors)
     *    -   (ushort) 2B - number of samples in vector
     *    -   (x number of samples)
     *      -   (x number of sample dimensions)
     *       -    (sample component type) xB - sample component
     *               e.g. real, imag part
     *
     *  @return Returns false on error.
     */
    bool Open(const string &Filename, const string &Dir);

    //! Checks Coefficients size for given vector
    /*! \param vector_no = 0, 1, ...
     *   - 0 - numerator coefficients (FIR or IIR filters)
     *   - 1 - denominator coefficients (only IIR filters)
     *   .
     * Larger values can be used for files with
     * multiple responses sets.
     *
     * @return Returns size on success and -1 on error.
     */
    int GetSize(int vector_no = 0);
    //! Returns number of vectors in file
    int GetNoOfVectors(void);

    //! Loads complex coefficients vector
    /*!
     *  \param FIR_coef - pointer to vector where coefficients will be stored
     *  \param vector_index - index of coefficients vector to read.
     *    Typically 0, but can be used to read from file
     *    containing set of FIR filter responses. All the responses
     *    must be complex.
     *
     * Returns FALSE if file type or size mismatch is detected.
     */
    bool Load(DSP::Complex_vector &FIR_coef, int vector_index = 0);
    //! Loads real coefficients vector
    /*!
     *  \param FIR_coef - pointer to vector where coefficients will be stored
     *  \param vector_index - index of coefficients vector to read.
     *    Typically 0, but can be used to read from file
     *    containing set of FIR filter responses. All the responses
     *    must be complex.
     *
     * Returns FALSE if file type mismatch is detected.
     */
    bool Load(DSP::Float_vector &FIR_coef, int vector_index = 0);
};

/*@} load_func*/

/*@} misc_func*/

#endif

