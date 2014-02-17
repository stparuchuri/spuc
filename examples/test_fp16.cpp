#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;
#include <spuc/fp16.h>
#include <spuc/iir_2nd.h>
#include <spuc/iir.h>
#include <spuc/butterworth_iir.h>
#include <spuc/delay.h>
using namespace SPUC;
//! \brief testing IIR with fp16 data type
//
//! \ingroup examples
int main(int argv, char* argc[]) {
  int i;
  long iimp;
  const long N=1;
  const long Order=2;
  const long Int_Bits =4;
  const long Frac_Bits=4;
  double a0,a1;

  ofstream ff("imp.dat");
  ofstream tmpf("tmp.dat");

  iir_coeff BPF(Order);
  butterworth_iir(BPF,0.1,true,3.0);

  iir<double> LPF(BPF);

  LPF.print();

  complex<double> ab = BPF.get_root(0);
  a0 = ab.re;
  a1 = ab.im;

  iir_2nd<long, double>         testa(a0,a1);
 
  iir_2nd<long, fp16>  test3;

  test3.set_coeffs(
				   fp16(1), 
				   fp16(2), 
				   fp16(1), 
				   fp16(a0), 
				   fp16(a1) 
				   );

  test3.print();

  iimp = 100;
  fp16 imp_fp;

  for (i=0;i<N;i++) {
	tmpf << testa.clock(iimp) << " ";
	imp_fp = iimp;
	ff << test3.clock(imp_fp) << "\n";
	iimp = 0;
  }

  ff.close();
  return(1);
}
