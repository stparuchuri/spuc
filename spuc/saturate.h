#ifndef SPUC_SATURATE
#define SPUC_SATURATE

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
// from directory: spuc_template_functions
#include <spuc/spuc_types.h>
#include <cmath>
#include <spuc/saturater.h>
namespace SPUC {
//! \file
//! \brief Templated saturate function
//
//! \brief Templated saturate function
//! template function that uses a class to allow template specialization
//! \author Tony Kirke
//! \ingroup template_functions misc
template <class T> T saturate(T in, long bits) {
  return( saturater<T>::saturated(in,bits) );
}
} // namespace SPUC
#endif
