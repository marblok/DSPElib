//---------------------------------------------------------------------------
/*! \file DSP::Fourier.cpp
 * Contains implementation of DSP::Fourier class
 *
 * \author Marek Blok
 */
//#include <cmath>
#include <stdio.h>
#include <string.h>
#include <functional>

#include <DSP_logstream.h>

#include "DSP_Fourier.h"

//---------------------------------------------------------------------------

//*******************************//
//* DFT functions, they use     *//
//* vectors' lengths differences *//
//* to minimalize computation   *//
//* time                        *//
//*******************************//
/*! \Fixed <b>2005.10.29</b> removed >>cSinFFT<< which seemed unused
 */
DSP::Fourier::Fourier(void)
{ //empty cSin and cSinConj
  K=0;   K2=0;

  fft.clear();
  cSin.clear(); 
//   cSinConj.clear();
// //  cSinFFT.clear();

  RevBitTable.clear(); FFTshift_RevBitTable.clear();

  IsFFT=false;
  isFFTshiftON=false;

  //FFT parameters
  Kbit = 0; //number of bits - 1
};

DSP::Fourier::Fourier(const unsigned long &K_new)
{
  K=0;   K2=0;

  fft.clear();
  cSin.clear(); 
//   cSinConj.clear();
// //  cSinFFT.clear();

  RevBitTable.clear(); FFTshift_RevBitTable.clear();

  IsFFT=false;
  isFFTshiftON=false;

  //FFT parameters
  Kbit = 0; //Liczba bit�w -1

  resize(K_new);
};

DSP::Fourier::~Fourier(void)
{
  K=0;   K2=0;

  fft.clear();
  cSin.clear();
//   cSinConj.clear();
// //  cSinFFT.clear();

  RevBitTable.clear();
  FFTshift_RevBitTable.clear();

  IsFFT=false;

  //FFT parameters
  Kbit = 0; //Liczba bit�w -1
};

void DSP::Fourier::resize(const unsigned long &K_in)
{
  unsigned long i, k;
  // DSP::Float skala;

  if (K_in != DSP::Fourier::K)
  {
    DSP::Fourier::K=K_in;
    K2=(K-K%2)/2+1;
  //  fft.resize(K>>1);
    IsFFT=CheckIsFFT(K);

    fft.clear();
    fft.resize(K);

    cSin.clear();
    cSin.resize(K);

//     cSinConj.clear();
//     cSinConj.resize(K);

// //    cSinFFT.clear();
// //    cSinFFT.resize(K>>1);

    RevBitTable.clear();
    RevBitTable.resize(K);

    FFTshift_RevBitTable.clear();
    FFTshift_RevBitTable.resize(K);

    // Processing input parameters for FFT
    Kbit = 0; // number of bits - 1 
    k = K>>1;
    while (k!=1)
    {
      Kbit++;
      k >>= 1;
    }

    for (i=0; i<K; i++)
    { //exp{j*2*pi/K*i}
      cSin[i].set(cos(2*DSP::M_PIx1*DSP::Float(i)/DSP::Float(K)),-sin(2*DSP::M_PIx1*DSP::Float(i)/DSP::Float(K)));
//      if ((i%2)==0)
//        cSinFFT[i>>1]=cSin[i];
      RevBitTable[i]=BitRev(i, Kbit+1);
      FFTshift_RevBitTable[(i+K/2)%K]=RevBitTable[i];
    }
    
// //     //memcpy(cSinConj, cSin, K*sizeof(DSP::Complex));
// //     cSinConj=cSin;
// // //    cSinConj.conj(); cSinConj/=K;
// //     temp=(DSP::Float *)cSinConj; temp++; skala=-1.0f/(DSP::Float)K;
// //     for (i=0; i<K; i++)
// //     {
// //       (*temp)*=skala;
// //       temp+=2;
// //     }

//     cSinConj=cSin;
//     skala=1.0f/(DSP::Float)K;
//     for (i=0; i<K; i++)
//     {
//       cSinConj[i].re *= skala;
//       cSinConj[i].im *= -skala;
//     }
  }
};

/*
void DSP::Fourier::resizeR(const unsigned long &K2)
{
  resize((K2%2)?(2*(K2-1)):(2*K2-1));
};

void DSP::Fourier::DFTR(complex *dft, float *sygn)
{  //DFT of real sequence
  if (IsFFT)
  {
    FFTR(dft, sygn);
  }
  else
  {
    unsigned long k, n;

    //Warto wykorzysta� symetri� DFT sygna�u rzeczywistego
    dft.resize(K2);
    for (k=0; k<K2; k++)
    {
      dft[k]=0;
      for (n=0; n<sygn.N; n++)
        dft[k]+=sygn[n]*cSin[(k*n)%cSin.N];
    }
  }
};

//cvector& DSP::Fourier::DFT(cvector)
//{  //DFT of complex sequence
//  return cSin;
//};

void DSP::Fourier::IDFTR(rvector& sygn, cvector& dft)
{  //real part of IDFT
  if (IsFFT)
  {
    IFFTR(sygn, dft);
  }
  else
  {
    unsigned long k, n;

    //Warto wykorzysta� symetri� DFT sygna�u rzeczywistego
    dft.resize(K2);
    for (n=0; n<sygn.N; n++)
    {
      sygn[n]=real(dft[0]*cSinConj[0]);
      sygn[n]+=real(dft[K2-1]*cSinConj[(K2*n)%cSinConj.N]);
      for (k=1; k<K2-1; k++)
        sygn[n]+=2*(real(dft[k]*cSinConj[(k*n)%cSinConj.N]));
    }
  }
};
*/

/*
unsigned long DSP::Fourier::DFTlength(void)
{ return K; };

unsigned long DSP::Fourier::DFTdatalen(void)
{ return K2; };
*/

void DSP::Fourier::FFT(const unsigned long &N, DSP::Complex_vector &probki)
{  //FFT of real sequence
  unsigned long Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP::Complex COStmp, temp;
  // https://www.nextptr.com/tutorial/ta1441164581/stdref-and-stdreference_wrapper-common-use-cases
  auto  tmp_rev_bit_table = std::ref(RevBitTable);

  if (K != N)
    resize(N);

  if (isFFTshiftON == false)
    //! \todo test FFTshift in FFT
    tmp_rev_bit_table = std::ref(FFTshift_RevBitTable);

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  Kbit_tmp=Kbit+1;
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
    do
    {
        COStmp=cSin[RevBitTable[ind2>>Kbit_tmp]>>1];
        for (ind3=0; ind3<M; ind3++)
        {
          temp=COStmp*probki[ind2b];
          probki[ind2b]=probki[ind2]-temp;
          probki[ind2]+=temp;

          ind2++; ind2b++;
        }
        ind2+= M; ind2b+= M;
    }
    while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
    Kbit_tmp--;
  }
  for (ind1=0; ind1<K-1; ind1++)
  {
    ind2 = tmp_rev_bit_table.get()[ind1];
    //i = BitRev(k,Kbit0);
    if (ind2>ind1)
    {
      COStmp = probki[ind1];
      probki[ind1]=probki[ind2];
      probki[ind2]=COStmp;
    }
  }
};

void DSP::Fourier::absFFT(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Complex_vector &probki)
{
  unsigned long Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP::Complex COStmp, temp;

  if (K != N)
    resize(N);

  #ifdef __DEBUG__
    if (fft.size() != probki.size()) {
      DSP::log << DSP::LogMode::Error << "DSP::Fourier::absFFT" << DSP::LogMode::second << "fft.size() != probki.size()" << endl;
      return;
    }
  #endif
  fft = probki;
  //memcpy(fft, probki, N*sizeof(DSP::Complex));

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  Kbit_tmp=Kbit+1;
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
	do
	{
      COStmp=cSin[RevBitTable[ind2>>Kbit_tmp]>>1];
      for (ind3=0; ind3<M; ind3++)
      {
//        temp=COStmp*probki[ind2b];
//        probki[ind2b]=probki[ind2]-temp;
//        probki[ind2]+=temp;
        temp=COStmp*fft[ind2b];
        fft[ind2b]=fft[ind2]-temp;
        fft[ind2]+=temp;

        ind2++; ind2b++;
      }
      ind2+= M; ind2b+= M;
    }
    while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
    Kbit_tmp--;
  }
/*
  for (ind1=0; ind1<K-1; ind1++)
  {
		ind2 = RevBitTable.data[ind1];
		//i = BitRev(k,Kbit0);
		if (ind2>ind1)
		{
		  abs_fft[ind1]=abs(probki[ind2]);
		  abs_fft[ind2]=abs(probki[ind1]);
		}
	}
*/
  if (isFFTshiftON)
  {
    //FFTshift
    for (ind1=0; ind1<K; ind1++)
    {
      ind2 = FFTshift_RevBitTable[ind1];
//      abs_fft[ind1]=probki[ind2].abs();
      abs_fft[ind1]=fft[ind2].abs();
    }
  }
  else
  {
    for (ind1=0; ind1<K; ind1++)
    {
      ind2 = RevBitTable[ind1];
//      abs_fft[ind1]=probki[ind2].abs();
      abs_fft[ind1]=fft[ind2].abs();
    }
  }
};

void DSP::Fourier::absFFT(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Float_vector &probki, const DSP::Float_vector &probki_imag)
{
  unsigned long Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP::Complex COStmp, temp;

  if (K != N)
    resize(N);

  for (ind1=0; ind1<N; ind1++)
  {
    fft[ind1].set(probki[ind1],probki_imag[ind1]);
  }

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  Kbit_tmp=Kbit+1;
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
	do
	{
      COStmp=cSin[RevBitTable[ind2>>Kbit_tmp]>>1];
      for (ind3=0; ind3<M; ind3++)
      {
        temp=COStmp*fft[ind2b];
        fft[ind2b]=fft[ind2]-temp;
        fft[ind2]+=temp;

        ind2++; ind2b++;
      }
      ind2+= M; ind2b+= M;
	}
	while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
    Kbit_tmp--;
  }
/*
  for (ind1=0; ind1<K-1; ind1++)
  {
		ind2 = RevBitTable.data[ind1];
		//i = BitRev(k,Kbit0);
		if (ind2>ind1)
		{
		  abs_fft[ind1]=abs(probki[ind2]);
		  abs_fft[ind2]=abs(probki[ind1]);
		}
	}
*/

  //future: mozna kopiowac tylko polowe
  if (isFFTshiftON)
  {
    //FFTshift
    for (ind1=0; ind1<K; ind1++)
    {
      ind2 = FFTshift_RevBitTable[ind1];
      abs_fft[ind1]=fft[ind2].abs();
    }
  }
  else
  {
    for (ind1=0; ind1<K; ind1++)
    {
      ind2 = RevBitTable[ind1];
      abs_fft[ind1]=fft[ind2].abs();
    }
  }
};


void DSP::Fourier::abs2FFT(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Complex_vector &probki)
{  //FFT of real sequence
  unsigned long Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP::Complex COStmp, temp;

  if (K != N)
    resize(N);

  #ifdef __DEBUG__
    if (fft.size() != probki.size()) {
      DSP::log << DSP::LogMode::Error << "DSP::Fourier::abs2FFT" << DSP::LogMode::second << "fft.size() != probki.size()" << endl;
      return;
    }
  #endif
  fft = probki;

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  Kbit_tmp=Kbit+1;
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
	do
	{
      COStmp=cSin[RevBitTable[ind2>>Kbit_tmp]>>1];
      for (ind3=0; ind3<M; ind3++)
      {
        temp=COStmp*fft[ind2b];
        fft[ind2b]=fft[ind2]-temp;
        fft[ind2]+=temp;

        ind2++; ind2b++;
      }
      ind2+= M; ind2b+= M;
	}
	while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
    Kbit_tmp--;
  }

  if (isFFTshiftON)
  {
    //FFTshift
    for (ind1=0; ind1<K; ind1++)
    {
      ind2 = FFTshift_RevBitTable[ind1];
//      abs_fft[ind1]=abs2(probki[ind2]);
      abs_fft[ind1]=abs2(fft[ind2]);
    }
  }
  else
  {
    for (ind1=0; ind1<K; ind1++)
    {
      ind2 = RevBitTable[ind1];
//      abs_fft[ind1]=abs2(probki[ind2]);
      abs_fft[ind1]=abs2(fft[ind2]);
    }
  }
};

/*
void DSP::Fourier::FFT(cvector& probki)
{  //FFT of real sequence
  //K - powinno by� pot�g� dw�jki
  //unsigned long K, N2,Kbit0,Kbit1, i,l,k;
  //unsigned long Kpom, Pom;
  unsigned long M, seg_no, ind1, ind2, ind2b, ind3;
  complex COStmp;


  if (K != probki.N)
    resize(probki.N);

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
	  do
	  {
      for (ind3=0; ind3<M; ind3++)
      {
        COStmp=probki.data[ind2];
//        probki.data[ind2]=probki.data[ind2b]+COStmp;
        probki.data[ind2]+=probki.data[ind2b];
        probki.data[ind2b]=COStmp-probki.data[ind2b];
        probki.data[ind2b]*=cSin[ind3*seg_no];
//        probki.data[ind2b]=probki.data[ind2b]*cSin[ind3*seg_no];

        ind2++; ind2b++;
      }
      ind2+= M; ind2b+= M;
		}
		while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
  }
  for (ind1=0; ind1<K-1; ind1++)
  {
		ind2 = RevBitTable.data[ind1];
		//i = BitRev(k,Kbit0);
		if (ind2>ind1)
		{
		  COStmp = probki[ind1];
		  probki[ind1]=probki[ind2];
		  probki[ind2]=COStmp;
		}
	}
};
*/

void DSP::Fourier::IFFT(const unsigned long &N, DSP::Complex_vector &probki)
{  //FFT of real sequence
  unsigned long i;

  FFT(N, probki);

// //  probki.conj();
//   temp=(DSP::Float *)probki;
  for (i=0; i<N; i++)
  {
    probki[i].re =  probki[i].re/(DSP::Float)K;
    probki[i].im = -probki[i].im/(DSP::Float)K;
  }
};

/*
void  DSP::Fourier::FFTR(cvector& dft, rvector& sygn)
{
  unsigned long i, k, new_FFT_size;
  complex pom1, pom2, pom;
  complex SINtmp;

//  new_FFT_size=(sygn.N+(sygn.N % 2))/2;
  new_FFT_size=(sygn.N+1)>>1;

  //Prepare input data
  resize(new_FFT_size);
  fft.resize(new_FFT_size);
  dft.resize(new_FFT_size+1);

  ZeroMemory(fft.data, fft.N*sizeof(complex));

//  for (i=0; i<sygn.N; i++)
//    if (i%2)
//     fft.data[i>>1].im=sygn.data[i];
//    else
//     fft.data[i>>1].re=sygn.data[i];
  CopyMemory(fft.data, sygn.data, sygn.N*sizeof(Tfloat));

  //compute complex FFT
  FFT(fft);

  //extract real signal DFT
  //i=0
  dft.data[0].re=fft.data[0].re+fft.data[0].im; //pom1+pom2
  k=fft.N-1;
  for (i=1; i<fft.N; i++,k--)
  {
    //pom1=(pom+conj(reverse(pom)))/2
    pom1.re=(fft.data[i].re+fft.data[k].re)/2;
    pom1.im=(fft.data[i].im-fft.data[k].im)/2;
    //pom2=(pom-conj(reverse(pom)))/2
    pom2.re=(fft.data[i].re-fft.data[k].re)/2;
    pom2.im=(fft.data[i].im+fft.data[k].im)/2;

    //pom2=pom2.*exp(-sqrt(-1)*(0:length(pom)-1)*pi/length(pom))*(-sqrt(-1));
    pom=pom2*cSin[i];

    dft.data[i].re=pom1.re+pom.im;
    dft.data[i].im=pom1.im-pom.re;
  }
  //i=K2-1 (fft.N)
  dft.data[fft.N].re=fft.data[0].re-fft.data[0].im; //pom1+pom2
};

void  DSP::Fourier::IFFTR(rvector& sygn, cvector& dft)
{
  unsigned long i, k;
  complex pom1, pom2, pom;
  //ZeroMemory(fft.data, fft.N*sizeof(complex));
  //test=(dft2(1)+dft2(end))/2+(dft2(1)-dft2(end))/2*sqrt(-1)
  fft.data[0].re=(dft.data[0].re+dft.data[fft.N].re)/2;
  fft.data[0].im=(dft.data[0].re-dft.data[fft.N].re)/2;

  k=fft.N-1;
  for (i=1; i<fft.N; i++,k--)
  {
    //pom1=(dft(2:(end-1))+conj(dft((end-1):-1:2)))/2
    pom1.re=(dft.data[i].re+dft.data[k].re)/2;
    pom1.im=(dft.data[i].im-dft.data[k].im)/2;
    //pom2=(dft(2:(end-1))-conj(dft((end-1):-1:2)))/2
    pom2.re=(dft.data[i].re-dft.data[k].re)/2;
    pom2.im=(dft.data[i].im+dft.data[k].im)/2;

    //pom2=pom2.*(exp(sqrt(-1)*(1:length(pom)-1)*pi/length(pom)))*sqrt(-1)

    //pom=pom2.*(-exp(sqrt(-1)*(1:length(pom)-1)*pi/length(pom)))
    pom=pom2*cSin[k];

    //pom1-pom*sqrt(-1);
    fft.data[i].re=pom1.re+pom.im;
    fft.data[i].im=pom1.im-pom.re;
  }

  //ifft(test)
  FFT(fft);
  fft.data[0]/=fft.N;
  fft.data[fft.N>>1]/=fft.N;
  k=fft.N-1;
  for (i=1; i<(fft.N>>1); i++,k--)
  {
    pom=fft.data[k]/fft.N;
    fft.data[k]=fft.data[i]/fft.N;
    fft.data[i]=pom;
  }

  CopyMemory(sygn.data, fft.data, sygn.N*sizeof(Tfloat));
}
*/

// FFT
unsigned long DSP::Fourier::BitRev(unsigned long x, const unsigned long &s)
{
  unsigned long Pom;
  unsigned long i;

  Pom = 0;
  for (i=0; i<s; i++)
  {
    Pom <<= 1;
    if (x & 1)
      Pom++;
    x >>= 1;
  }
  return Pom;
}

bool DSP::Fourier::CheckIsFFT(const unsigned long &K_in)
{
  unsigned long bits, k;

  bits = 0; //number of bits - 1
  k = K_in;
  while (k!=1)
  {
    bits++;
	  k >>= 1;
  }
//  IsFFT= (K == (unsigned long(1) << bits));
  return (K_in == ((unsigned long)(1) << bits));
}

//! Real input signal (FFT): Full output spectrum
/*! JustHalf == true: outputs only N/2+1 samples
 *  JustHalf == false: outputs all N samples
 */
void DSP::Fourier::absFFTR(const unsigned long &N, DSP::Float_vector &abs_fft, const DSP::Float_vector &probki, const bool &JustHalf)
{
  unsigned long Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP::Complex COStmp, temp;

  if (K != N)
    resize(N);

  for (ind1=0; ind1<N; ind1++)
  {
    fft[ind1]=probki[ind1];
  }

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  Kbit_tmp=Kbit+1;
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
	do
	{
      COStmp=cSin[RevBitTable[ind2>>Kbit_tmp]>>1];
      for (ind3=0; ind3<M; ind3++)
      {
        temp=COStmp*fft[ind2b];
        fft[ind2b]=fft[ind2]-temp;
        fft[ind2]+=temp;

        ind2++; ind2b++;
      }
      ind2+= M; ind2b+= M;
	}
	while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
    Kbit_tmp--;
  }
/*
  for (ind1=0; ind1<K-1; ind1++)
  {
		ind2 = RevBitTable.data[ind1];
		//i = BitRev(k,Kbit0);
		if (ind2>ind1)
		{
		  abs_fft[ind1]=abs(probki[ind2]);
		  abs_fft[ind2]=abs(probki[ind1]);
		}
	}
*/

  //future: mozna kopiowac tylko polowe
  if (JustHalf == true)
  {
    // just K/2+1 first samples
    for (ind1=0; ind1<=K/2; ind1++)
    {
      ind2 = RevBitTable[ind1];
      abs_fft[ind1]=abs(fft[ind2]);
    }
  }
  else
  {
    // all samples
    if (isFFTshiftON)
    {
      //FFTshift
      for (ind1=0; ind1<K; ind1++)
      {
        ind2 = FFTshift_RevBitTable[ind1];
        abs_fft[ind1]=abs(fft[ind2]);
      }
    }
    else
    {
      for (ind1=0; ind1<K; ind1++)
      {
        ind2 = RevBitTable[ind1];
        abs_fft[ind1]=abs(fft[ind2]);
      }
    }
  }
};


//! Real input signal (DFT): Full output spectrum
/*! JustHalf == true: outputs only
 *   - N/2+1 samples for N even
 *   - (N+1)/2 samples for N odd
 *   .
 *  JustHalf == false: outputs all N samples
 */
void DSP::Fourier::absDFTR(const unsigned long &N, DSP::Float_vector &abs_dft, const DSP::Float_vector &probki, const bool &JustHalf)
{  //DFT of real sequence
  DSP::Float temp_abs;

  if (K != N)
    resize(N);
  IsFFT=CheckIsFFT(N);

  if (IsFFT)
  {
    absFFTR(N, abs_dft, probki, JustHalf);
  }
  else
  {
    unsigned long k, n;

    //future: Warto wykorzysta� symetri� DFT sygna�u rzeczywistego
    if ((K % 2) == 0)
    {
      for (k=0; k<=K/2; k++)
      {
        fft[k]=0;
        for (n=0; n<K; n++)
          fft[k]+=probki[n]*cSin[(k*n)%K];
      }
    }
    else
    {
      for (k=0; k<=(K-1)/2; k++)
      {
        fft[k]=0;
        for (n=0; n<K; n++)
          fft[k]+=probki[n]*cSin[(k*n)%K];
      }
    }
    /*
    for (k=0; k<K; k++)
    {
      fft[k]=0;
      for (n=0; n<K; n++)
        fft[k]+=probki[n]*cSin[(k*n)%K];
    }
    */

    if (JustHalf == true)
    { // just "K/2+1" first samples
      if ((K % 2) == 0)
        for (k=0; k<=K/2; k++)
          abs_dft[k]=fft[k].abs();
      else
        for (k=0; k<=(K-1)/2; k++)
          abs_dft[k]=fft[k].abs();
    }
    else
    { // all samples
      if (isFFTshiftON)
      { //We need to implement FFTshift
        if ((K%2)==0)
        {
          abs_dft[0]=fft[K/2].abs();
          abs_dft[K/2]=fft[0].abs();
          for (k=1; k<K/2; k++)
          {
            temp_abs = fft[k].abs();
            abs_dft[K/2+k] = temp_abs;
            abs_dft[K/2-k] = temp_abs;
          }
        }
        else
        {
          abs_dft[(K-1)/2]=fft[0].abs();
          for (k=1; k<=(K-1)/2; k++)
          {
            temp_abs = fft[k].abs();
            abs_dft[(K-1)/2+k] = temp_abs;
            abs_dft[(K-1)/2-k] = temp_abs;
          }
        }
      }
      else
      {
        for (k=0; k<K; k++)
          abs_dft[k]=fft[k].abs();
      }
    }

    /*
    if (isFFTshiftON)
    { //We need to implement FFTshift
      if ((K%2)==0)
      {
        for (k=0; k<K/2; k++)
        {
          abs_dft[k]=fft[K/2+k].abs();
          abs_dft[K/2+k]=fft[k].abs();
        }
      }
      else
      {
        for (k=0; k<K/2; k++)
        {
          abs_dft[k]=fft[K/2+1+k].abs();
          abs_dft[K/2+k]=fft[k].abs();
        }
        abs_dft[K-1]=fft[K/2].abs();
      }
    }
    else
    {
      for (k=0; k<K; k++)
        abs_dft[k]=fft[k].abs();
    }
    */
  }
};

void DSP::Fourier::absDFT(const unsigned long &N, DSP::Float_vector &abs_dft, const DSP::Complex_vector &probki)
{  //DFT of real sequence
  if (K != N)
    resize(N);
  IsFFT=CheckIsFFT(N);

  if (IsFFT)
  {
    absFFT(N, abs_dft, probki);
  }
  else
  {
    unsigned long k, n;

    for (k=0; k<K; k++)
    {
      fft[k]=0;
      for (n=0; n<K; n++)
      {
        fft[k]+=probki[n]*cSin[(k*n)%K];
      }
    }
    if (isFFTshiftON)
    { //We need to implement FFTshift
      if ((K%2)==0)
      {
        for (k=0; k<K/2; k++)
        {
          abs_dft[k]=abs(fft[K/2+k]);
          abs_dft[K/2+k]=abs(fft[k]);
        }
      }
      else
      {
        for (k=0; k<K/2; k++)
        {
          abs_dft[k]=abs(fft[K/2+1+k]);
          abs_dft[K/2+k]=abs(fft[k]);
        }
        abs_dft[K-1]=abs(fft[K/2]);
      }
    }
    else
    {
      for (k=0; k<K; k++)
        abs_dft[k]=abs(fft[k]);
    }
  }
};

void DSP::Fourier::absDFT(const unsigned long &N, DSP::Float_vector &abs_dft, const DSP::Float_vector &probki, const DSP::Float_vector &probki_imag)
{  //DFT of real sequence
  if (K != N)
    resize(N);
  IsFFT=CheckIsFFT(N);

  if (IsFFT)
  {
    absFFT(N, abs_dft, probki, probki_imag);
  }
  else
  {
    unsigned long k, n;

    for (k=0; k<K; k++)
    {
      fft[k]=0;
      for (n=0; n<K; n++)
      {
        fft[k]+=DSP::Complex(probki[n],probki_imag[n])*cSin[(k*n)%K];
      }
    }
    if (isFFTshiftON)
    { //We need to implement FFTshift
      if ((K%2)==0)
      {
        for (k=0; k<K/2; k++)
        {
          abs_dft[k]=abs(fft[K/2+k]);
          abs_dft[K/2+k]=abs(fft[k]);
        }
      }
      else
      {
        for (k=0; k<K/2; k++)
        {
          abs_dft[k]=abs(fft[K/2+1+k]);
          abs_dft[K/2+k]=abs(fft[k]);
        }
        abs_dft[K-1]=abs(fft[K/2]);
      }
    }
    else
    {
      for (k=0; k<K; k++)
        abs_dft[k]=abs(fft[k]);
    }
  }
};

void DSP::Fourier::DFT(const unsigned long &N, const DSP::Complex_vector &probki_in, DSP::Complex_vector &probki_out)
{  //DFT of real sequence
  if (K != N)
    resize(N);
  IsFFT=CheckIsFFT(N);

  #ifdef __DEBUG__
    if (probki_in.size() != probki_out.size()) {
      DSP::log << DSP::LogMode::Error << "DSP::Fourier::DFT" << DSP::LogMode::second << "probki_in.size() != probki_out.size()" << endl;
      return;
    }
  #endif

  if (IsFFT)
  {
    if (&probki_out != &probki_in) {
      //memcpy(probki_out, probki_in, N*sizeof(DSP::Complex));
      probki_out = probki_in;
    }
    FFT(N, probki_out);
  }
  else
  {
    unsigned long k, n;

    for (k=0; k<K; k++)
    {
      fft[k]=0;
      for (n=0; n<K; n++)
      {
        fft[k]+=probki_in[n]*cSin[(k*n)%K];
      }
    }

    if (isFFTshiftON)
    { //We need to implement FFTshift
      if ((K%2)==0)
      {
        for (k=0; k<K/2; k++)
        {
          probki_out[k] = fft[K/2+k];
          probki_out[K/2+k] = fft[k];
        }
      }
      else
      {
        for (k=0; k<K/2; k++)
        {
          probki_out[k] = fft[K/2+1+k];
          probki_out[K/2+k] = fft[k];
        }
        probki_out[K-1] = fft[K/2];
      }
    }
    else
    {
      //for (k=0; k<K; k++)
      //  probki_out[k]=fft[k];
      probki_out = fft;
    }
  }
};


void DSP::Fourier::FFTshiftON(const bool &val)
{
  isFFTshiftON=val;
};

