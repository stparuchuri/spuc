#ifndef SPUC_QUAD_DATA
#define SPUC_QUAD_DATA

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
#include <spuc/complex.h>
#include <spuc/max_pn.h>
#include <spuc/noise.h>
#include <spuc/fir.h>
#include <spuc/fir_coeff.h>
#include <spuc/fir_interp.h>
#include <spuc/root_raised_cosine.h>
#include <spuc/lagrange.h>
#include <spuc/builtin.h>
#include <spuc/fundtype.h>
namespace SPUC {
//! \file
//! \brief  Class for QPSK data using a root raised cosine transmit filter.
//
//! \brief  Class for QPSK data using a root raised cosine transmit filter.
//
//! Samples are generated by a combination of polyphase FIR and lagrange
//! interpolation (to allow for a wide range of sampling rates)
//!  \author Tony Kirke,  Copyright(c) 2001 
//! \author Tony Kirke
//!  \ingroup real_templates comm
template <class Numeric> class quad_data
{
 public:
  typedef typename fundtype<Numeric>::ftype CNumeric;
  fir_interp <complex<CNumeric>, Numeric > rcfir;
  max_pn pn_i;
  max_pn pn_q;
  int symbols;
  int over;
  complex<CNumeric> data;
  lagrange <complex<CNumeric> > interp;
  float_type prev_timing_offset;
  void set_initial_offset(float_type timing_init) { 
	prev_timing_offset=timing_init;
  }	

  quad_data(int total_over) : rcfir(12*total_over+1), pn_i(0x006d, 63, -1),
							  pn_q(0x074d, 1023, -1), data(1,1), interp(4) {

	over = total_over;
	rcfir.set_rate(over);
	rcfir.set_automatic();
	fir_coeff<Numeric> rc_coeff(rcfir.number_of_taps());
	root_raised_cosine(rc_coeff, 0.35, total_over);
	rcfir.settaps(rc_coeff);
	prev_timing_offset = 0.0;
  }
  complex<CNumeric> get_fir_output(void)	{
	if (rcfir.phase==0) {
#ifndef NO_QUAD_TX_DATA
	  data = complex<CNumeric>(pn_i.out(),pn_i.out());
#endif
	  rcfir.input(data);
	}
	return(rcfir.clock());
  }
  complex<CNumeric> get_sample(float_type timing_inc) 
  {
	// timing inc is in units of total_over oversampling rate
	// i.e 1 corresponds to 1/total_over of a symbol
	if (timing_inc < 0) timing_inc = 0; // Timing_inc should
	// not be negative in the first place!
	float_type next_timing_offset = prev_timing_offset + timing_inc;
	while (next_timing_offset >= 1.0) {
	  next_timing_offset -=  1.0;
	  interp.input(get_fir_output());
	};
	
	prev_timing_offset = next_timing_offset;
	return(interp.rephase(next_timing_offset));
  }
};
} // namespace SPUC
#endif
