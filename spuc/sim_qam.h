#ifndef SPUC_SIM_QAM
#define SPUC_SIM_QAM

/*
    Copyright (C) 2014 Tony Kirke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// from directory: spuc_real_templates
#include <spuc/spuc_types.h>
#include <cmath>
#include <spuc/complex.h>
#include <spuc/max_pn.h>   	
#include <spuc/noise.h>     
#include <spuc/a_d.h>     
#include <spuc/vco.h>
#include <spuc/fading_channel.h>
#include <spuc/lagrange.h>
#include <spuc/qam_tx.h>
#include <spuc/root_raised_cosine.h>
#include <spuc/base_demod.h>
#include <spuc/builtin.h>
#include <spuc/fundtype.h>
namespace SPUC {
//! \file
//! \brief  A Class for simulating a QAM system 
//
//! \brief  A Class for simulating a QAM system 
//
//! that includes transmitters, receivers, A/D, frequency offsets,
//! gaussian noise, and a BER tester
//! \author Tony Kirke
//!  \ingroup real_templates sim
template <class Numeric> class sim_qam {

public:
  typedef typename fundtype<Numeric>::ftype CNumeric;
  typedef complex<CNumeric> complex_type;

  const long over; // Oversampling rate
  float_type alpha;
  lagrange<complex<float_type> > interp;
  fir<complex<float_type>,float_type > rx_filter;
  qam_tx<float_type> TX;
  noise* n;
  base_demod<Numeric>* DUT;
  vco<float_type>* freq_offset;
  a_d* ADC;
  float_type var;
  float_type snr;
  float_type channel_pwr;
  complex<float_type> tx_data; 
  complex<long> data; 
  long output_delay; // Equalizer output delay (for paths to merge)
  long rate;
  
  bool enable_freq_offset;
  bool enable_time_offset;
  float_type carrier_offset_rate;
  float_type time_inc;
  float_type time_offset;
  
  complex_type adc_out;
  complex<float_type> base; 
  complex<float_type> main; 
  complex<float_type> main1, base1;
  
  complex<float_type> b_noise;  // Noise
  long rcv_symbols;       // Number of symbols decoded
  long tx_symbols;       // Counter for transmitted symbols
  long count;  			// index of sample number at input rate
  float_type phase_inc;
  float_type phase_acc;
  
  ~sim_qam() {
	if (n) delete n;
  }
  //----------------------------------------------------------------------------
  // Constructor!
  //---------------------------------------------------------------------------
  sim_qam(float_type rc_alpha=0.25)
	:  over(4),
	   alpha(rc_alpha),
	   interp(4),
	   rx_filter(12*4+1),
	   TX(12, 4, 0, 0, rc_alpha)
  {
	snr = 10.0;
	base = complex<float_type>(0,0);
	count = 0;
	output_delay = 0;
	n = new noise;
	
	enable_freq_offset = 0;
	enable_time_offset = 0;
	rate = 0; // default
	rcv_symbols=0; // Number of symbols decoded
	//! alpha = 0.35 root raised cosine fir
	fir_coeff<float_type> fir_c(rx_filter.num_taps);
	root_raised_cosine(fir_c,alpha,over);
	rx_filter.settaps(fir_c);

	float_type scale  = (1.0/float_type(over));
	for (int j=0;j<rx_filter.num_taps;j++) { rx_filter.coeff[j] *= scale; }
	channel_pwr = 1.0;
  }
  //---------------------------------------------------------------------------
  // Initialize pointers
  //---------------------------------------------------------------------------
  void loop_init(long rate, long conv_rate,
						  float_type carrier_off=0,  float_type time_off=0)
  {
	output_delay = 0;
	var = sqrt(0.5*(float_type)over)*pow(10.0,-0.05*snr);   // Unfiltered noise std dev	
	
	TX.loop_init(rate, conv_rate);
	
	carrier_offset_rate = carrier_off;
	time_inc = 1.0 + (float_type)(time_off/1000000.0);
	if (carrier_off) enable_freq_offset=1;
	else enable_freq_offset = 0;
	if (time_off) enable_time_offset=1;
	else enable_time_offset = 0;
	
	freq_offset = new vco<float_type>;
	freq_offset->reset_frequency(carrier_offset_rate);
	
	time_offset = 0;
	rcv_symbols=0;  		  // Number of symbols decoded
	count=0;    			 // index of sample number at input rate
	
	rx_filter.reset();
	ADC = new a_d(8);
  }
  //---------------------------------------------------------------------------
  // STEP
  //---------------------------------------------------------------------------
  complex<float_type> tx_step() 
  {
	
	// Get new sample from transmitter
	if (enable_time_offset) {
	  time_offset += time_inc;
	  while (time_offset >= 1.0) {
		time_offset -= 1.0; 
		interp.input(TX.clock());
	  }
	  base = TX.data_level*interp.rephase(time_offset);
	} else {
	  base = TX.data_level*TX.clock();
	}
	// Apply Frequency offset
	if (enable_freq_offset) {
	  complex<float_type> rot = freq_offset->clock();
	  base *= rot;
	}
	// Noise term
	b_noise = var*n->Cgauss(); 
	base1 = base + b_noise;
	main1 = rx_filter.update(base1);
	return(main1);
  }	
  //---------------------------------------------------------------------------
  // STEP
  //---------------------------------------------------------------------------
  void step(void) {
	adc_out = ADC->clock(tx_step());
	rx_step(adc_out);
  }
  complex<long> rx_step(complex_type x) 
  {
	data = DUT->step(x);
	if (DUT->sym_pulse()) rcv_symbols++;
	return(data);
  }	
  //---------------------------------------------------------------------------
  // Delete pointers
  //----------------------------------------------------------------------------
  void loop_end(void)
  {
	//  delete multipaths;
	delete freq_offset;
	interp.reset();
	rx_filter.reset();
	delete ADC;
  }     
};
} // namespace SPUC
#endif
