/*
	The MIT License (MIT)

	Copyright (c) 2008-2019, Charles Karney (original code and GeographicLib)
	Copyright (c) 2020-2021, Francesco Raviglione, Marco Malinverno (C adapation of the UTMUPS and TransverseMercator modules)

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

#include "utmups_math.h"
#include <math.h>

static inline void UTMUPS_Math_swap_(double *a, double *b) {
	double tmp;

	tmp=*a;
	*a=*b;
	*b=tmp;
}

// This is a C porting of the Math::polyval method in GeographicLib
double UTMUPS_Math_polyval(int n, const double *p, double x) { 
	double y = n < 0 ? 0 : *p++; 
	while (--n >= 0) {
		y = y * x + *p++; 
	}

	return y;
}

double UTMUPS_Math_sum(double u, double v, double *t) {
	volatile double s = u + v;
	volatile double up = s - v;
	volatile double vpp = s - up;
	up -= u;
	vpp -= v;
	*t = -(up + vpp);

	// u + v =       s      + t
	//       = round(u + v) + t
	return s;
}

double UTMUPS_Math_eatanhe(double x, double es) {
	return es > (double) 0.0 ? es * atanh(es * x) : -es * atan(es * x);
}

double UTMUPS_Math_taupf(double tau, double es) {
	if (isfinite(tau)) {
		double tau1 = hypot(1.0, tau);

		double sig = sinh(UTMUPS_Math_eatanhe(tau / tau1, es ));

		return hypot(1.0, sig) * tau - sig * tau1;
	} else {
		return tau;
	}
}

double UTMUPS_Math_atan2d(double y, double x) {
	// In order to minimize round-off errors, this function rearranges the
	// arguments so that result of atan2 is in the range [-pi/4, pi/4] before
	// converting it to degrees and mapping the result to the correct
	// quadrant.
	int q = 0;
	if (fabs(y) > fabs(x)) { 
		UTMUPS_Math_swap_(&x, &y); q = 2;
	}

	if (x < 0) { x = -x; ++q; }
	// here x >= 0 and x >= abs(y), so angle is in [-pi/4, pi/4]

	// (atan2((double) 0.0, (double)-1.0) / 180) corresponds to calling, in C++ GeographicLib, "degree<T>()"
	double ang = atan2(y, x) / (atan2((double) 0.0, (double)-1.0) / 180);
	switch (q) {
		// Note that atan2d(-0.0, 1.0) will return -0.  However, we expect that
		// atan2d will not be called with y = -0.  If need be, include
		//
		//   case 0: ang = 0 + ang; break;
		//
		// and handle mpfr as in AngRound.
		case 1: 
			ang = (y >= 0 ? 180 : -180) - ang; 
			break;

		case 2: 
			ang =  90 - ang; 
			break;

		case 3: 
			ang = -90 + ang; 
			break;
	}

	return ang;
}

double UTMUPS_Math_atand(double x) {
	return UTMUPS_Math_atan2d(x, 1.0);
}

void UTMUPS_Math_sincosd(double x, double *sinx, double *cosx) {
	// In order to minimize round-off errors, this function exactly reduces
	// the argument to the range [-45, 45] before converting it to radians.
	double r;
	int q;

	// N.B. the implementation of remquo in glibc pre 2.22 were buggy.  See
	// https://sourceware.org/bugzilla/show_bug.cgi?id=17569
	// This was fixed in version 2.22 on 2015-08-05
	r = remquo(x, (double) 90.0, &q);   // now abs(r) <= 45

	// (atan2((double) 0.0, (double)-1.0) / 180) corresponds to calling, in C++ GeographicLib, "degree<T>()"
	r *= (atan2((double) 0.0, (double)-1.0) / 180);
	// g++ -O turns these two function calls into a call to sincos
	double s = sin(r), c = cos(r);

	#if defined(_MSC_VER) && _MSC_VER < 1900
		// Before version 14 (2015), Visual Studio had problems dealing
		// with -0.0.  Specifically
		//   VC 10,11,12 and 32-bit compile: fmod(-0.0, 360.0) -> +0.0
		//   VC 12       and 64-bit compile:  sin(-0.0)        -> +0.0
		// AngNormalize has a similar fix.
		// python 2.7 on Windows 32-bit machines has the same problem.
		if (x == 0) s = x;
	#endif

		switch ((unsigned) (q) & 3U) {
		case 0U: 
			*sinx =  s; *cosx =  c; 
			break;

		case 1U: 
			*sinx =  c; *cosx = -s; 
			break;

		case 2U: 
			*sinx = -s; *cosx = -c; 
			break;

		default: 
			*sinx = -c; *cosx =  s; 
			break; // case 3U
	}
	// Set sign of 0 results.  -0 only produced for sin(-0)
	if (x != 0) { 
		*sinx += (double) 0.0; 
		*cosx += (double) 0.0; 
	}
}

double UTMUPS_Math_tand(double x) {
	// DBL_EPSIPLON replaces std::numeric_limits<T>::epsilon() which is obviously not available in C
	double overflow = 1.0 / UTMUPS_Math_sq(DBL_EPSILON);
	double s, c;
	UTMUPS_Math_sincosd(x, &s, &c);
	return c != 0 ? s / c : (s < 0 ? -overflow : overflow);
}

double UTMUPS_Math_AngNormalize(double x) {
	x = remainder(x, (double) 360.0); return x != -180 ? x : 180;
}

double UTMUPS_Math_AngDiff_e(double x, double y, double *e) {
	double t;
	double d = UTMUPS_Math_AngNormalize(UTMUPS_Math_sum(remainder(-x, (double) 360.0),
						  remainder( y, (double) 360.0), &t));
	// Here y - x = d + t (mod 360), exactly, where d is in (-180,180] and
	// abs(t) <= eps (eps = 2^-45 for doubles).  The only case where the
	// addition of t takes the result outside the range (-180,180] is d = 180
	// and t > 0.  The case, d = -180 + eps, t = -eps, can't happen, since
	// sum would have returned the exact result in such a case (i.e., given t
	// = 0).
	return UTMUPS_Math_sum(d == 180 && t > 0 ? -180 : d, t, e);
}

double UTMUPS_Math_AngDiff(double x, double y) {
	double e; 
	return UTMUPS_Math_AngDiff_e(x, y, &e); 
}

double UTMUPS_Math_LatFix(double x) {
	return fabs(x) > 90 ? COORD_NAN : x;
}

double UTMUPS_Math_haversineDist(double lat_a, double lon_a, double lat_b, double lon_b) {
	// 12742000 is the mean Earth radius (6371 km) * 2 * 1000 (to convert from km to m)
	return 12742000.0*asin(sqrt(sin(DEG_2_RAD(lat_b-lat_a)/2)*sin(DEG_2_RAD(lat_b-lat_a)/2)+cos(DEG_2_RAD(lat_a))*cos(DEG_2_RAD(lat_b))*sin(DEG_2_RAD(lon_b-lon_a)/2)*sin(DEG_2_RAD(lon_b-lon_a)/2)));
}

double UTMUPS_Math_tauf(double taup, double es) {
	const int numit = 5;
	// min iterations = 1, max iterations = 2; mean = 1.95
	const double tol = sqrt(DBL_EPSILON) / 10;

	const double taumax = 2 / sqrt(DBL_EPSILON);

	double e2m = 1.0 - UTMUPS_Math_sq(es),
	// To lowest order in e^2, taup = (1 - e^2) * tau = _e2m * tau; so use
	// tau = taup/e2m as a starting guess. Only 1 iteration is needed for
	// |lat| < 3.35 deg, otherwise 2 iterations are needed.  If, instead, tau
	// = taup is used the mean number of iterations increases to 1.999 (2
	// iterations are needed except near tau = 0).
	//
	// For large tau, taup = exp(-es*atanh(es)) * tau.  Use this as for the
	// initial guess for |taup| > 70 (approx |phi| > 89deg).  Then for
	// sufficiently large tau (such that sqrt(1+tau^2) = |tau|), we can exit
	// with the intial guess and avoid overflow problems.  This also reduces
	// the mean number of iterations slightly from 1.963 to 1.954.
	tau = fabs(taup) > 70 ? taup * exp(UTMUPS_Math_eatanhe(1.0, es)) : taup/e2m,
	stol = tol * fmax(1.0, fabs(taup));

	if(!(fabs(tau) < taumax)) {
		return tau; // handles +/-inf and nan
	}

	for (int i = 0; i < numit || GEOGRAPHICLIB_PANIC; ++i) {
		double taupa = UTMUPS_Math_taupf(tau, es);
		double dtau = (taup - taupa) * (1 + e2m * UTMUPS_Math_sq(tau)) /
			( e2m * hypot(1.0, tau) * hypot(1.0, taupa) );

		tau += dtau;

		if (!(fabs(dtau) >= stol)) {
			break;
		}

	}
	
	return tau;
}