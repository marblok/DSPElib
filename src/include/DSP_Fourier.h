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
class DSP_Fourier
{
  private:
    DWORD K, K2;
    DSP::Complex *fft;
    DSP::Complex *cSin, *cSinConj;
//    DSP::Complex *cSinFFT;

    DWORD *RevBitTable, *FFTshift_RevBitTable;


    bool IsFFT, isFFTshiftON;
    DWORD Kbit;

    DWORD BitRev(DWORD x, DWORD s);
  public:
    void resize(DWORD K);
    //void resizeR(DWORD K2);

    DSP_Fourier(void);
    DSP_Fourier(DWORD K);
    ~DSP_Fourier(void);

    bool CheckIsFFT(DWORD K);
    void FFTshiftON(bool val);

    //! \TODO use private temporary vector so, the content pointed by the probki variable would not be destroyed

    //! \warning probki table is used to store internal values so it will be overwritten
    void FFT(DWORD N, DSP::Complex *probki);
    //! \warning probki table is used to store internal values so it will be overwritten
    void IFFT(DWORD N, DSP::Complex *probki);

    void absFFT(DWORD N, DSP::Float *abs_fft, DSP::Complex *probki);
    void absFFT(DWORD N, DSP::Float *abs_fft, DSP::Float *probki, DSP::Float *probki_imag);
    void absFFTR(DWORD N, DSP::Float *abs_fft, DSP::Float *probki, bool JustHalf = false);
    void abs2FFT(DWORD N, DSP::Float *abs_fft, DSP::Complex *probki);

    void absDFT(DWORD N, DSP::Float *abs_fft, DSP::Complex *probki);
    void absDFT(DWORD N, DSP::Float *abs_fft, DSP::Float *probki, DSP::Float *probki_imag);
    void absDFTR(DWORD N, DSP::Float *abs_fft, DSP::Float *probki, bool JustHalf = false);

    //! DFT (if possible FFT radix-2 is used)
    /*! \note probki_in and probki_out can point to the same buffer
     * @param N
     * @param probki_in
     * @param probki_out
     */
    void DFT(DWORD N, DSP::Complex *probki_in, DSP::Complex *probki_out);
};

#endif // DSP_FourierH
