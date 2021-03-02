//---------------------------------------------------------------------------
/*! \file DSP_Fourier.cpp
 * Contains implementation of DSP_Fourier class
 *
 * \author Marek Blok
 */
#include <math.h>
#include <stdio.h>
#include <string.h>

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
DSP_Fourier::DSP_Fourier(void)
{ //cSin i cSinConj puste
  K=0;   K2=0;

  fft=NULL;
  cSin=NULL; cSinConj=NULL;
//  cSinFFT=NULL;

  RevBitTable=NULL; FFTshift_RevBitTable=NULL;

  IsFFT=false;
  isFFTshiftON=false;

  //FFT parameters
  Kbit = 0; //Liczba bit�w -1
};

DSP_Fourier::DSP_Fourier(DWORD K_new)
{
  K=0;   K2=0;

  fft=NULL;
  cSin=NULL; cSinConj=NULL;
//  cSinFFT=NULL;

  RevBitTable=NULL; FFTshift_RevBitTable=NULL;

  IsFFT=false;
  isFFTshiftON=false;

  //FFT parameters
  Kbit = 0; //Liczba bit�w -1

  resize(K_new);
};

DSP_Fourier::~DSP_Fourier(void)
{
  K=0;   K2=0;

  if (fft!=NULL)
    delete [] fft;
  fft=NULL;
  if (cSin!=NULL)
    delete [] cSin;
  cSin=NULL;
  if (cSinConj!=NULL)
    delete [] cSinConj;
  cSinConj=NULL;
//  if (cSinFFT!=NULL)
//    delete [] cSinFFT;
//  cSinFFT=NULL;

  if (RevBitTable!=NULL)
    delete [] RevBitTable;
  RevBitTable=NULL;
  if (FFTshift_RevBitTable!=NULL)
    delete [] FFTshift_RevBitTable;
  FFTshift_RevBitTable=NULL;

  IsFFT=false;

  //FFT parameters
  Kbit = 0; //Liczba bit�w -1
};

void DSP_Fourier::resize(DWORD K_in)
{
  DWORD i, k;
  DSP_float *temp, skala;

  if (K_in != DSP_Fourier::K)
  {
    DSP_Fourier::K=K_in;
    K2=(K-K%2)/2+1;
  //  fft.resize(K>>1);
    IsFFT=CheckIsFFT(K);

//    fft.resize(K);
    if (fft!=NULL)
      delete [] fft;
    fft= new DSP_complex[K];

//    cSin.resize(K);
    if (cSin!=NULL)
      delete [] cSin;
    cSin= new DSP_complex[K];

//    cSinConj.resize(K);
    if (cSinConj!=NULL)
      delete [] cSinConj;
    cSinConj= new DSP_complex[K];

////    cSinFFT.resize(K>>1);
//    if (cSinFFT!=NULL)
//      delete [] cSinFFT;
//    cSinFFT= new DSP_complex[K>>1];

//    RevBitTable.resize(K);
    if (RevBitTable!=NULL)
      delete [] RevBitTable;
    RevBitTable= new DWORD[K];

//    FFTshift_RevBitTable.resize(K);
    if (FFTshift_RevBitTable!=NULL)
      delete [] FFTshift_RevBitTable;
    FFTshift_RevBitTable= new DWORD[K];

    //Przetworzenie parametr�w wej�ciowych for FFT
    Kbit = 0; //Liczba bit�w -1 (czyli powy�sze p)
    k = K>>1;
    while (k!=1)
    {
      Kbit++;
      k >>= 1;
    }

    for (i=0; i<K; i++)
    { //exp{j*2*pi/K*i}
      cSin[i].set(cos(2*M_PIx1f*DSP_float(i)/DSP_float(K)),-sin(2*M_PIx1f*DSP_float(i)/DSP_float(K)));
//      if ((i%2)==0)
//        cSinFFT[i>>1]=cSin[i];
      RevBitTable[i]=BitRev(i, Kbit+1);
      FFTshift_RevBitTable[(i+K/2)%K]=RevBitTable[i];
    }
    //cSinConj=cSin;
    memcpy(cSinConj, cSin, K*sizeof(DSP_complex));

//    cSinConj.conj(); cSinConj/=K;
    temp=(DSP_float *)cSinConj; temp++; skala=-1.0f/(DSP_float)K;
    for (i=0; i<K; i++)
    {
      (*temp)*=skala;
      temp+=2;
    }
  }
};

/*
void DSP_Fourier::resizeR(DWORD K2)
{
  resize((K2%2)?(2*(K2-1)):(2*K2-1));
};

void DSP_Fourier::DFTR(complex *dft, float *sygn)
{  //DFT of real sequence
  if (IsFFT)
  {
    FFTR(dft, sygn);
  }
  else
  {
    DWORD k, n;

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

//cvector& DSP_Fourier::DFT(cvector)
//{  //DFT of complex sequence
//  return cSin;
//};

void DSP_Fourier::IDFTR(rvector& sygn, cvector& dft)
{  //real part of IDFT
  if (IsFFT)
  {
    IFFTR(sygn, dft);
  }
  else
  {
    DWORD k, n;

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
DWORD DSP_Fourier::DFTlength(void)
{ return K; };

DWORD DSP_Fourier::DFTdatalen(void)
{ return K2; };
*/

void DSP_Fourier::FFT(DWORD N, DSP_complex *probki)
{  //FFT of real sequence
  DWORD Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP_complex COStmp, temp;
  DWORD *tmp_rev_bit_table;

  if (K != N)
    resize(N);

  if (isFFTshiftON == true)
    tmp_rev_bit_table = RevBitTable;
  else
    //! \todo test FFTshift in FFT
    tmp_rev_bit_table = FFTshift_RevBitTable;

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
    ind2 = tmp_rev_bit_table[ind1];
    //i = BitRev(k,Kbit0);
    if (ind2>ind1)
    {
      COStmp = probki[ind1];
      probki[ind1]=probki[ind2];
      probki[ind2]=COStmp;
    }
  }
};

void DSP_Fourier::absFFT(DWORD N, DSP_float *abs_fft, DSP_complex *probki)
{
  DWORD Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP_complex COStmp, temp;

  if (K != N)
    resize(N);

  memcpy(fft, probki, N*sizeof(DSP_complex));

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

void DSP_Fourier::absFFT(DWORD N, DSP_float *abs_fft, DSP_float *probki, DSP_float *probki_imag)
{
  DWORD Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP_complex COStmp, temp;

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


void DSP_Fourier::abs2FFT(DWORD N, DSP_float *abs_fft, DSP_complex *probki)
{  //FFT of real sequence
  DWORD Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP_complex COStmp, temp;

  if (K != N)
    resize(N);

  memcpy(fft, probki, N*sizeof(DSP_complex));

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
void DSP_Fourier::FFT(cvector& probki)
{  //FFT of real sequence
  //K - powinno by� pot�g� dw�jki
  //DWORD K, N2,Kbit0,Kbit1, i,l,k;
  //DWORD Kpom, Pom;
  DWORD M, seg_no, ind1, ind2, ind2b, ind3;
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

void DSP_Fourier::IFFT(DWORD N, DSP_complex *probki)
{  //FFT of real sequence
  DSP_float *temp;
  DWORD i;

  FFT(N, probki);

//  probki.conj();
  temp=(DSP_float *)probki;
  for (i=0; i<N; i++)
  {
    *temp=(*temp)/(DSP_float)K;
    temp++;
    *temp=(-*temp)/(DSP_float)K;
    temp++;
  }
};

/*
void  DSP_Fourier::FFTR(cvector& dft, rvector& sygn)
{
  DWORD i, k, new_FFT_size;
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

void  DSP_Fourier::IFFTR(rvector& sygn, cvector& dft)
{
  DWORD i, k;
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
DWORD DSP_Fourier::BitRev(DWORD x, DWORD s)
{
  DWORD Pom;
  DWORD i;

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

bool DSP_Fourier::CheckIsFFT(DWORD K_in)
{
  DWORD bits, k;

  bits = 0; //Liczba bit�w -1 (czyli powy�sze p)
  k = K_in;
  while (k!=1)
  {
    bits++;
	  k >>= 1;
  }
//  IsFFT= (K == (DWORD(1) << bits));
  return (K_in == (DWORD(1) << bits));
}

//! Real input signal (FFT): Full output spectrum
/*! JustHalf == true: outputs only N/2+1 samples
 *  JustHalf == false: outputs all N samples
 */
void DSP_Fourier::absFFTR(DWORD N, DSP_float *abs_fft, DSP_float *probki, bool JustHalf)
{
  DWORD Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  DSP_complex COStmp, temp;

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
void DSP_Fourier::absDFTR(DWORD N, DSP_float *abs_dft, DSP_float *probki, bool JustHalf)
{  //DFT of real sequence
  DSP_float temp_abs;

  if (K != N)
    resize(N);
  IsFFT=CheckIsFFT(N);

  if (IsFFT)
  {
    absFFTR(N, abs_dft, probki, JustHalf);
  }
  else
  {
    DWORD k, n;

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

void DSP_Fourier::absDFT(DWORD N, DSP_float *abs_dft, DSP_complex *probki)
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
    DWORD k, n;

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

void DSP_Fourier::absDFT(DWORD N, DSP_float *abs_dft, DSP_float *probki, DSP_float *probki_imag)
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
    DWORD k, n;

    for (k=0; k<K; k++)
    {
      fft[k]=0;
      for (n=0; n<K; n++)
      {
        fft[k]+=DSP_complex(probki[n],probki_imag[n])*cSin[(k*n)%K];
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

void DSP_Fourier::DFT(DWORD N, DSP_complex *probki_in, DSP_complex *probki_out)
{  //DFT of real sequence
  if (K != N)
    resize(N);
  IsFFT=CheckIsFFT(N);

  if (IsFFT)
  {
    if (probki_out != probki_in)
      memcpy(probki_out, probki_in, N*sizeof(DSP_complex));
    FFT(N, probki_out);
  }
  else
  {
    DWORD k, n;

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
      memcpy(fft, probki_out, N*sizeof(DSP_complex));
    }
  }
};


void DSP_Fourier::FFTshiftON(bool val)
{
  isFFTshiftON=val;
};

