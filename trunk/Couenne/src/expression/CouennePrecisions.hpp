/* $Id$
 *
 * Name:    CouennePrecisions.hpp
 * Author:  Pietro Belotti
 * Purpose: constants for evaluation procedures
 *
 * (C) Carnegie-Mellon University, 2006-10.
 * This file is licensed under the Eclipse Public License (EPL)
 */

#ifndef COUENNE_PRECISIONS_HPP
#define COUENNE_PRECISIONS_HPP

#include <math.h>

namespace Couenne {

// must be >= 1e-7
#define COUENNE_EPS           1.e-07

// to be used in bounds tightening to avoid node pruning due to strict COUENNE_EPS tolerance
#define COUENNE_BOUND_PREC    1.e-5

// for integrality check
#define COUENNE_EPS_INT       1.e-9

// for simplification
#define COUENNE_EPS_SIMPL     1.e-20

// for bounds
#ifndef COUENNE_INFINITY
#define COUENNE_INFINITY      1.e+50
#endif

// for cuts, ensures stability and scaling in Clp
#define COU_MAX_COEFF     1.e+9

// for cuts, ditto
#define COU_MIN_COEFF     1.e-9

// rounds to nearest integer
#define COUENNE_round(x) ((int) (floor ((x) + 0.5)))

// sign of a value
#define COUENNE_sign(x) ((x) > 0.0 ? 1.0 : -1.0)

#define MAX_BOUND 1.e45

/// used to declare LP unbounded
const double Couenne_large_bound =  9.999e12;

}

#endif
