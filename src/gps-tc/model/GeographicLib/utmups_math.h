/*
	The MIT License (MIT)

	Copyright (c) 2008-2019, Charles Karney (original code and GeographicLib)
	Copyright (c) 2020, Francesco Raviglione, Marco Malinverno (C adapation of the UTMUPS module)

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use, copy,
	modify, merge, publish, distribute, sublicense, and/or sell copies
	of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/

#ifndef UTMUPS_MATH_H_INCLUDED
#define UTMUPS_MATH_H_INCLUDED

#include "float.h"
#include "inttypes.h"

// Acting as an invalid value (like Math::NaN in GeographicLib)
#define COORD_NAN DBL_MAX

// Equivalent of Math::sq in C++ GeographicLib, defined as macro
#define UTMUPS_Math_sq(n) (n*n)

// Addition: macro to convert from DEG to RAD
#define DEG_2_RAD(val) ((val)*M_PI/180.0)

// All these C versions take into account double types
// as in the original C++ GeographicLib when GEOGRAPHICLIB_PRECISION == 2 (default)
// C version of Math::polyval
double UTMUPS_Math_polyval(int n, const double *p, double x);
// C version of Math::sum
double UTMUPS_Math_sum(double u, double v, double *t);

// C version of Math::eatanhe
double UTMUPS_Math_eatanhe(double x, double es);
// C version of Math::taupf
double UTMUPS_Math_taupf(double tau, double es);
// C version of Math::atan2d
double UTMUPS_Math_atan2d(double y, double x);
// C version of Math::sincosd
void UTMUPS_Math_sincosd(double x, double *sinx, double *cosx);
// C version of Math::tand
double UTMUPS_Math_tand(double x);

// C version of Math::AngNormalize
double UTMUPS_Math_AngNormalize(double x);
// C version of Math::AngDiff(T x, T y, T& e) (with the type fixed as double)
double UTMUPS_Math_AngDiff_e(double x, double y, double *e);
// C version of Math::AngDiff(T x, T y) (with the type fixed as double)
double UTMUPS_Math_AngDiff(double x, double y);
// C version of Math::LatFix
double UTMUPS_Math_LatFix(double x);

// Additional function to compute the (approximated) distance between two points on Earth, using the Haversine formula
double UTMUPS_Math_haversineDist(double lat_a, double lon_a, double lat_b, double lon_b);

#endif // UTMUPS_MATH_H_INCLUDED
