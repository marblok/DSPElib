/*! \file DSP_Fourier.h
 * DSP_Fourier class header file
 *
 * \author Marek Blok
 */
//---------------------------------------------------------------------------
#ifndef DSP_FourierH
#define DSP_FourierH

//#include "complex.h";
//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------
#include <DSP_types.h>
//---------------------------------------------------------------------------
namespace DSP {
  class Fourier
  {
    private:
      unsigned long K, K2;
      DSP::Complex_vector fft;
      DSP::Complex_vector cSin;
  //    DSP::Complex_vector cSinConj;
  ////    DSP::Complex_vector cSinFFT;

      std::vector <unsigned long> RevBitTable, FFTshift_RevBitTable;


      bool IsFFT, isFFTshiftON;
      unsigned long Kbit;

      unsigned long BitRev(unsigned long x, const unsigned long &s);
    public:
      void resize(const unsigned long &K);
      //void resizeR(unsigned long K2);

      Fourier(void);
      Fourier(const unsigned long &K);
      ~Fourier(void);

      bool CheckIsFFT(const unsigned long &K);
      void FFTshiftON(const bool &val);

      //! \warning probki table is used to store internal values so it will be overwritten
      void FFT(const unsigned long &N, DSP::Complex_vector &probki);
      //! \warning probki table is used to store internal values so it will be overwritten
      void IFFT(const unsigned long &N, DSP::Complex_vector &probki);

      void absFFT(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Complex_vector &probki);
      void absFFT(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Float_vector &probki, const DSP::Float_vector &probki_imag);
      void absFFTR(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Float_vector &probki, const bool &JustHalf = false);
      void abs2FFT(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Complex_vector &probki);

      void absDFT(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Complex_vector &probki);
      void absDFT(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Float_vector &probki, const DSP::Float_vector &probki_imag);
      void absDFTR(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Float_vector &probki, const bool &JustHalf = false);

      //! DFT (if possible FFT radix-2 is used)
      /*! \note probki_in and probki_out can point to the same buffer
      * @param N
      * @param probki_in
      * @param probki_out
      */
      void DFT(const unsigned long &N, const DSP::Complex_vector &probki_in, DSP::Complex_vector &probki_out);
  };
}

#endif // DSP_FourierH
