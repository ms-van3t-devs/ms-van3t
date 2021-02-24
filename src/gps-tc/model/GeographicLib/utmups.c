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

#include "utmups.h"
#include "utmups_math.h"
#include <math.h>
#include <stdio.h>
#include <complex.h>

// UTMUPS access these enums
enum mgrs_utmups_data {
	tile_ = 100000,            // Size MGRS blocks
	minutmcol_ = 1,
	maxutmcol_ = 9,
	minutmSrow_ = 10,
	maxutmSrow_ = 100,         // Also used for UTM S false northing
	minutmNrow_ = 0,           // Also used for UTM N false northing
	maxutmNrow_ = 95,
	minupsSind_ = 8,           // These 4 ind's apply to easting and northing
	maxupsSind_ = 32,
	minupsNind_ = 13,
	maxupsNind_ = 27,
	upseasting_ = 20,          // Also used for UPS false northing
	utmeasting_ = 5,           // UTM false easting
	// Difference between S hemisphere northing and N hemisphere northing
	utmNshift_ = (maxutmSrow_ - minutmNrow_) * tile_
};

double UTMUPS_WGS84_f(void) {
	// Evaluating this as 1000000000 / T(298257223563LL) reduces the
	// round-off error by about 10%.  However, expressing the flattening as
	// 1/298.257223563 is well ingrained.
	return 1.0 / ( ((double)298257223563LL) / 1000000000.0 );
}

transverse_mercator_t UTMUPS_init_TransverseMercator(double a, double f, double k0) {
	static const double b1coeff_ord6[] = {
		// b1*(n+1), polynomial in n2 of order 3
		1, 4, 64, 256, 256,
    };  // count = 5

	static const double alpcoeff_ord6[] = {
		// alp[1]/n^1, polynomial in n of order 5
		31564, -66675, 34440, 47250, -100800, 75600, 151200,
		// alp[2]/n^2, polynomial in n of order 4
		-1983433, 863232, 748608, -1161216, 524160, 1935360,
		// alp[3]/n^3, polynomial in n of order 3
		670412, 406647, -533952, 184464, 725760,
		// alp[4]/n^4, polynomial in n of order 2
		6601661, -7732800, 2230245, 7257600,
		// alp[5]/n^5, polynomial in n of order 1
		-13675556, 3438171, 7983360,
		// alp[6]/n^6, polynomial in n of order 0
		212378941, 319334400,
	};  // count = 27

	static const double betcoeff_ord6[] = {
		// bet[1]/n^1, polynomial in n of order 5
		384796, -382725, -6720, 932400, -1612800, 1209600, 2419200,
		// bet[2]/n^2, polynomial in n of order 4
		-1118711, 1695744, -1174656, 258048, 80640, 3870720,
		// bet[3]/n^3, polynomial in n of order 3
		22276, -16929, -15984, 12852, 362880,
		// bet[4]/n^4, polynomial in n of order 2
		-830251, -158400, 197865, 7257600,
		// bet[5]/n^5, polynomial in n of order 1
		-435388, 453717, 15966720,
		// bet[6]/n^6, polynomial in n of order 0
		20648693, 638668800,
	};  // count = 27

	transverse_mercator_t transmerc;

	transmerc.maxpow_=MAXPOW_;
	transmerc.numit_=5;

	transmerc._a=a;
	transmerc._f=f;
	transmerc._k0=k0;

	transmerc._e2=transmerc._f * (2 - transmerc._f);
	transmerc._es=(f < 0 ? -1 : 1) * sqrt(fabs(transmerc._e2));
	transmerc._e2m=1 - transmerc._e2;

	transmerc._c=sqrt(transmerc._e2m) * exp(UTMUPS_Math_eatanhe(1.0, transmerc._es));
	transmerc._n=transmerc._f / (2 - transmerc._f);

	int m = transmerc.maxpow_/2;
    transmerc._b1 = UTMUPS_Math_polyval(m, b1coeff_ord6, UTMUPS_Math_sq(transmerc._n)) / (b1coeff_ord6[m + 1] * (1+transmerc._n));
    // _a1 is the equivalent radius for computing the circumference of
    // ellipse.

    transmerc._a1 = transmerc._b1 * transmerc._a;
    int o = 0;
    double d = transmerc._n;
    for (int l = 1; l <= transmerc.maxpow_; ++l) {
		m = transmerc.maxpow_ - l;
		transmerc._alp[l] = d * UTMUPS_Math_polyval(m, alpcoeff_ord6 + o, transmerc._n) / alpcoeff_ord6[o + m + 1];
		transmerc._bet[l] = d * UTMUPS_Math_polyval(m, alpcoeff_ord6 + o, transmerc._n) / betcoeff_ord6[o + m + 1];
		o += m + 2;
		d *= transmerc._n;
    }

    transmerc.isinit = UINT8_MAX;

	return transmerc;
}

transverse_mercator_t UTMUPS_init_UTM_TransverseMercator(void) {
	return UTMUPS_init_TransverseMercator(WGS84_a,UTMUPS_WGS84_f(),UTM_k0);
}

polar_stereographic_t UTMUPS_init_PolarStereographic(double a, double f, double k0) {
	polar_stereographic_t polster;

	polster._a=a;
	polster._f=f;
	polster._e2=polster._f * (2 - polster._f);
	polster._es=(polster._f < 0 ? -1 : 1) * sqrt(fabs(polster._e2));
	polster._e2m=1 - polster._e2;
	polster._c=(1 - polster._f) * exp(UTMUPS_Math_eatanhe((double) 1.0, polster._es));
	polster._k0=k0;

	return polster;
}

polar_stereographic_t UTMUPS_init_UPS_PolarStereographic(void) {
	return UTMUPS_init_PolarStereographic(WGS84_a,UTMUPS_WGS84_f(),UTM_k0);
}

// Return latitude band number [-10, 10) for the given latitude (degrees).
// The bands are reckoned in include their southern edges.
// This function was originally inside the MGRS module in GeographicLib
// All the "real" numbers have been set as "double", as also most of the
// GeographicLib testing happened with "real" set as "double".
// std::max and std::min have been replaced with C's fmax and fmin (see https://en.cppreference.com/w/c/numeric/math/fmax)
static int MGRS_LatitudeBand(double lat) {
  int ilat = (int) floor(lat);
  return fmax(-10, fmin(9, (ilat + 80)/8 - 10));
}

static inline double UTMUPS_CentralMeridian(int zone) { 
	return (double) (6.0 * zone - 183.0); 
}

int UTMUPS_StandardZone(double lat, double lon, zonespec_t setzone) {
	if (!(setzone >= MINPSEUDOZONE && setzone <= MAXZONE)) {
		fprintf(stderr,"[UTMUPS ERROR] Illegal zone requested. Code: %d\n",setzone);
		return ERR_ILLEGALZONE;
	}

	if (setzone >= MINZONE || setzone == INVALID) {
	  return setzone;
	}

	if (setzone == UTM || (lat >= -80 && lat < 84)) {
		int ilon = (int) floor(UTMUPS_Math_AngNormalize(lon));

		if (ilon == 180) {
			ilon = -180; // ilon now in [-180,180)
		}

		int zone = (ilon + 186)/6;
		int band = MGRS_LatitudeBand(lat);

		if (band == 7 && zone == 31 && ilon >= 3) { // The Norway exception
			zone = 32;
		} else if (band == 9 && ilon >= 0 && ilon < 42) { // The Svalbard exception
			zone = 2 * ((ilon + 183)/12) + 1;
		}
		return zone;
	} else {
		return UPS;
	}
}

int TransverseMercator_Forward(transverse_mercator_t *transmercp, double lon0, double lat, double lon,
	double *x, double *y,
	double *gamma,double *k) {

	if(transmercp==NULL || transmercp->isinit!=UINT8_MAX) {
		return ERR_TMERC_UNINITIALIZED;
	}

    lat = UTMUPS_Math_LatFix(lat);
    lon = UTMUPS_Math_AngDiff(lon0, lon);
    // Explicitly enforce the parity

    int latsign = (lat < 0) ? -1 : 1;
    int lonsign = (lon < 0) ? -1 : 1;

    lon *= lonsign;
    lat *= latsign;

    uint8_t backside = lon > 90;

    if (backside) {
      if (lat == 0)
        latsign = -1;
      lon = 180 - lon;
    }
    double sphi, cphi, slam, clam;

    UTMUPS_Math_sincosd(lat, &sphi, &cphi);
    UTMUPS_Math_sincosd(lon, &slam, &clam);
    // phi = latitude
    // phi' = conformal latitude
    // psi = isometric latitude
    // tau = tan(phi)
    // tau' = tan(phi')
    // [xi', eta'] = Gauss-Schreiber TM coordinates
    // [xi, eta] = Gauss-Krueger TM coordinates
    //
    // We use
    //   tan(phi') = sinh(psi)
    //   sin(phi') = tanh(psi)
    //   cos(phi') = sech(psi)
    //   denom^2    = 1-cos(phi')^2*sin(lam)^2 = 1-sech(psi)^2*sin(lam)^2
    //   sin(xip)   = sin(phi')/denom          = tanh(psi)/denom
    //   cos(xip)   = cos(phi')*cos(lam)/denom = sech(psi)*cos(lam)/denom
    //   cosh(etap) = 1/denom                  = 1/denom
    //   sinh(etap) = cos(phi')*sin(lam)/denom = sech(psi)*sin(lam)/denom
    double etap, xip;
    if (lat != 90) {
		double tau = sphi / cphi;
		double taup = UTMUPS_Math_taupf(tau, transmercp->_es);
		
		xip = atan2(taup, clam);

		// Used to be
		//   etap = Math::atanh(sin(lam) / cosh(psi));
		etap = asinh(slam / hypot(taup, clam));
		// convergence and scale for Gauss-Schreiber TM (xip, etap) -- gamma0 =
		// atan(tan(xip) * tanh(etap)) = atan(tan(lam) * sin(phi'));
		// sin(phi') = tau'/sqrt(1 + tau'^2)
		// Krueger p 22 (44)
		*gamma = UTMUPS_Math_atan2d(slam * taup, clam * hypot(1.0, taup));
		// k0 = sqrt(1 - _e2 * sin(phi)^2) * (cos(phi') / cos(phi)) * cosh(etap)
		// Note 1/cos(phi) = cosh(psip);
		// and cos(phi') * cosh(etap) = 1/hypot(sinh(psi), cos(lam))
		//
		// This form has cancelling errors.  This property is lost if cosh(psip)
		// is replaced by 1/cos(phi), even though it's using "primary" data (phi
		// instead of psip).
		*k = sqrt(transmercp->_e2m + transmercp->_e2 * UTMUPS_Math_sq(cphi)) * hypot(1.0, tau) / hypot(taup, clam);
	} else {
		xip = M_PI_2;
		etap = 0;
		*gamma = lon;
		*k = transmercp->_c;
	}

	// {xi',eta'} is {northing,easting} for Gauss-Schreiber transverse Mercator
	// (for eta' = 0, xi' = bet). {xi,eta} is {northing,easting} for transverse
	// Mercator with constant scale on the central meridian (for eta = 0, xip =
	// rectifying latitude).  Define
	//
	//   zeta = xi + i*eta
	//   zeta' = xi' + i*eta'
	//
	// The conversion from conformal to rectifying latitude can be expressed as
	// a series in _n:
	//
	//   zeta = zeta' + sum(h[j-1]' * sin(2 * j * zeta'), j = 1..maxpow_)
	//
	// where h[j]' = O(_n^j).  The reversion of this series gives
	//
	//   zeta' = zeta - sum(h[j-1] * sin(2 * j * zeta), j = 1..maxpow_)
	//
	// which is used in Reverse.
	//
	// Evaluate sums via Clenshaw method.  See
	//    https://en.wikipedia.org/wiki/Clenshaw_algorithm
	//
	// Let
	//
	//    S = sum(a[k] * phi[k](x), k = 0..n)
	//    phi[k+1](x) = alpha[k](x) * phi[k](x) + beta[k](x) * phi[k-1](x)
	//
	// Evaluate S with
	//
	//    b[n+2] = b[n+1] = 0
	//    b[k] = alpha[k](x) * b[k+1] + beta[k+1](x) * b[k+2] + a[k]
	//    S = (a[0] + beta[1](x) * b[2]) * phi[0](x) + b[1] * phi[1](x)
	//
	// Here we have
	//
	//    x = 2 * zeta'
	//    phi[k](x) = sin(k * x)
	//    alpha[k](x) = 2 * cos(x)
	//    beta[k](x) = -1
	//    [ sin(A+B) - 2*cos(B)*sin(A) + sin(A-B) = 0, A = k*x, B = x ]
	//    n = maxpow_
	//    a[k] = _alp[k]
	//    S = b[1] * sin(x)
	//
	// For the derivative we have
	//
	//    x = 2 * zeta'
	//    phi[k](x) = cos(k * x)
	//    alpha[k](x) = 2 * cos(x)
	//    beta[k](x) = -1
	//    [ cos(A+B) - 2*cos(B)*cos(A) + cos(A-B) = 0, A = k*x, B = x ]
	//    a[0] = 1; a[k] = 2*k*_alp[k]
	//    S = (a[0] - b[2]) + b[1] * cos(x)
	//
	// Matrix formulation (not used here):
	//    phi[k](x) = [sin(k * x); k * cos(k * x)]
	//    alpha[k](x) = 2 * [cos(x), 0; -sin(x), cos(x)]
	//    beta[k](x) = -1 * [1, 0; 0, 1]
	//    a[k] = _alp[k] * [1, 0; 0, 1]
	//    b[n+2] = b[n+1] = [0, 0; 0, 0]
	//    b[k] = alpha[k](x) * b[k+1] + beta[k+1](x) * b[k+2] + a[k]
	//    N.B., for all k: b[k](1,2) = 0; b[k](1,1) = b[k](2,2)
	//    S = (a[0] + beta[1](x) * b[2]) * phi[0](x) + b[1] * phi[1](x)
	//    phi[0](x) = [0; 0]
	//    phi[1](x) = [sin(x); cos(x)]

	double c0 = cos(2 * xip), ch0 = cosh(2 * etap);
	double s0 = sin(2 * xip), sh0 = sinh(2 * etap);

	double complex a = (2 * c0 * ch0) + (-2 * s0 * sh0)*I; // 2 * cos(2*zeta')

	int n = transmercp->maxpow_;

	double complex y0 = n & 1 ?       transmercp->_alp[n] : 0;
	double complex y1 = 0;
	double complex z0 = n & 1 ? 2*n * transmercp->_alp[n] : 0;
	double complex z1 = 0;

	if (n & 1) --n;

	while (n) {
		y1 = a * y0 - y1 +       transmercp->_alp[n];
		z1 = a * z0 - z1 + 2*n * transmercp->_alp[n];
		--n;
		y0 = a * y1 - y0 +       transmercp->_alp[n];
		z0 = a * z1 - z0 + 2*n * transmercp->_alp[n];
		--n;
	}

	a /= 2.0;               // cos(2*zeta')
	z1 = 1.0 - z1 + a * z0;

	a = (s0 * ch0) + (c0 * sh0)*I; // sin(2*zeta')
	y1 = xip + etap*I + a * y0;
	// Fold in change in convergence and scale for Gauss-Schreiber TM to
	// Gauss-Krueger TM.
	*gamma -= UTMUPS_Math_atan2d(cimag(z1), creal(z1));
	*k *= transmercp->_b1 * cabs(z1);
	double xi = creal(y1);
	double eta = cimag(y1);
	*y = transmercp->_a1 * transmercp->_k0 * (backside ? M_PI - xi : xi) * latsign;
	*x = transmercp->_a1 * transmercp->_k0 * eta * lonsign;

	if (backside) {
		*gamma = 180 - *gamma;
	}

	*gamma *= latsign * lonsign;
	*gamma = UTMUPS_Math_AngNormalize(*gamma);
	*k *= transmercp->_k0;

	return UTMUPS_OK;
}

int PolarStereographic_Forward(polar_stereographic_t *polsterp, uint8_t northp, double lat, double lon,
	double *x, double *y,
	double *gamma, double *k) {

	if(polsterp==NULL) {
		return ERR_PSTER_UNINITIALIZED;
	}

	lat = UTMUPS_Math_LatFix(lat);	
	lat *= northp ? 1 : -1;

	double tau = UTMUPS_Math_tand(lat);
	double secphi = hypot((double) 1.0, tau);
	double taup = UTMUPS_Math_taupf(tau, polsterp->_es);
	double rho = hypot((double) 1.0, taup) + fabs(taup);

	rho = taup >= 0 ? (lat != 90 ? 1/rho : 0) : rho;

	rho *= 2 * polsterp-> _k0 * polsterp->_a / polsterp->_c;
	*k = lat != 90 ? (rho / polsterp->_a) * secphi * sqrt(polsterp->_e2m + polsterp->_e2 / UTMUPS_Math_sq(secphi)) : polsterp->_k0;
	UTMUPS_Math_sincosd(lon, x, y);
	*x *= rho;
	*y *= (northp ? -rho : rho);
	*gamma = UTMUPS_Math_AngNormalize(northp ? lon : -lon);

	return UTMUPS_OK;
}

uint8_t UTMUPS_CheckCoords(uint8_t utmp, uint8_t northp, double x, double y, double mgrslimits, uint8_t printerr) {
	// Limits are all multiples of 100km and are all closed on the both ends.
	// Failure tests are such that NaNs succeed.
	double slop = mgrslimits ? 0 : tile_;
	int ind = (utmp ? 2 : 0) + (northp ? 1 : 0);

	const int mineasting_[] = { minupsSind_ * tile_, minupsNind_ * tile_, minutmcol_ * tile_, minutmcol_ * tile_ };
	const int maxeasting_[] = { maxupsSind_ * tile_, maxupsNind_ * tile_, maxutmcol_ * tile_, maxutmcol_ * tile_ };
	const int minnorthing_[] = { minupsSind_ * tile_, minupsNind_ * tile_, minutmSrow_ * tile_, (minutmNrow_ + minutmSrow_ - maxutmSrow_) * tile_ };
	const int maxnorthing_[] = { maxupsSind_ * tile_, maxupsNind_ * tile_, (maxutmSrow_ + maxutmNrow_ - minutmNrow_) * tile_, maxutmNrow_ * tile_ };

	if (x < mineasting_[ind] - slop || x > maxeasting_[ind] + slop) {
		if (!printerr) {
			return 0;
		}

		fprintf(stderr,"[UTMUPS ERROR] Easting %lf km not in %s%s range for %s hemisphere [ %lfkm, %lfkm]",
			x/1000.0,
			(mgrslimits ? "MGRS/" : ""),
			(utmp ? "UTM" : "UPS"),
			(northp ? "N" : "S" ),
			(mineasting_[ind] - slop)/1000.0,
			(maxeasting_[ind] + slop)/1000.0);
	}
	if (y < minnorthing_[ind] - slop || y > maxnorthing_[ind] + slop) {
		if (!printerr) {
			return 0;
		}

		fprintf(stderr,"[UTMUPS ERROR] Northing %lf km not in %s%s range for %s hemisphere [ %lfkm, %lfkm]",
			y/1000.0,
			(mgrslimits ? "MGRS/" : ""),
			(utmp ? "UTM" : "UPS"),
			(northp ? "N" : "S" ),
			(minnorthing_[ind] - slop)/1000.0,
			(maxnorthing_[ind] + slop)/1000.0);

		return 0;
	}
	return 1;
}

// The last argument has been added with respect to the C++ full version of GeographicLic
// It allows the user to supply an already initialized transverse_mercator_t, thus saving
// the initialization cost in case this structure is always the same (as in the LonLat -> UTM case)
int UTMUPS_Forward(double lat, double lon,
	int *zone, uint8_t *northp, double *x, double *y,
	double *gamma, double *k,
	int setzone, uint8_t mgrslimits,
	transverse_mercator_t *preinit_transverse_mercator) {

	const int falseeasting_[] = { upseasting_ * tile_, upseasting_ * tile_, utmeasting_ * tile_, utmeasting_ * tile_ };
	const int falsenorthing_[] = { upseasting_ * tile_, upseasting_ * tile_, maxutmSrow_ * tile_, minutmNrow_ * tile_ };

	if (fabs(lat) > 90) {
		fprintf(stderr,"[UTMUPS ERROR] Latitude %lfdeg not in [-90deg, 90deg]\n",lat);
		return ERR_LAT_OUTOFRANGE;
   	}

	uint8_t northp1 = lat >= 0;
	int zone1 = UTMUPS_StandardZone(lat, lon, setzone);

	if (zone1 == INVALID) {
	  *zone = zone1;
	  *northp = northp1;
	  *x = COORD_NAN;
	  *y = COORD_NAN;
	  *gamma = COORD_NAN;
	  *k = COORD_NAN;

	  return 0;
	}

	double x1, y1, gamma1, k1;
	uint8_t utmp = zone1 != UPS;

	if (utmp) {
		double lon0 = UTMUPS_CentralMeridian(zone1);
		double dlon = lon - lon0;

		dlon = fabs(dlon - 360.0 * floor((dlon + 180.0)/360.0));

		if (!(dlon <= 60)) {
			fprintf(stderr,"[UTMUPS ERROR] Longitude %lfdeg more than 60d from center of UTM zone %d\n",lon,zone1);
			return ERR_LON_FARFROMCENTER;
		}
		    
		if(preinit_transverse_mercator==NULL) {
			transverse_mercator_t utm_transmerc=UTMUPS_init_UTM_TransverseMercator();
			TransverseMercator_Forward(&utm_transmerc, lon0, lat, lon, &x1, &y1, &gamma1, &k1);
		} else {
			TransverseMercator_Forward(preinit_transverse_mercator, lon0, lat, lon, &x1, &y1, &gamma1, &k1);
		}
	} else {
		if (fabs(lat) < 70) {
			fprintf(stderr,"[UTMUPS ERROR] Latitude %lfdeg more than 20d from %s pole\n",lat,(northp1 ? "N" : "S"));
			return ERR_LAT_FARFROMPOLE;
		}
		polar_stereographic_t ups_polster=UTMUPS_init_UPS_PolarStereographic();
		PolarStereographic_Forward(&ups_polster, northp1, lat, lon, &x1, &y1, &gamma1, &k1);
	}

	int ind = (utmp ? 2 : 0) + (northp1 ? 1 : 0);
	x1 += falseeasting_[ind];
	y1 += falsenorthing_[ind];
	if (! UTMUPS_CheckCoords(zone1 != UPS, northp1, x1, y1, mgrslimits, 0) ) {
		fprintf(stderr,"Latitude %lf, longitude %lf out of legal range for ",
			lat, lon);

		if(utmp) {
			fprintf(stderr,"UTM Zone %d\n",zone1);
		} else {
			fprintf(stderr,"UTS\n");
		}

		return ERR_OUTOFLEGALRANGE;
	}

	*zone = zone1;
	*northp = northp1;
	*x = x1;
	*y = y1;
	*gamma = gamma1;
	*k = k1;

	return UTMUPS_OK;
}