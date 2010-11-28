/* $Id: impliedBounds-exprTrilinear.cpp 427 2010-10-19 05:48:12Z pbelotti $
 *
 * Name:    impliedBounds-exprTrilinear.cpp
 * Author:  Pietro Belotti
 * Purpose: implied bounds for trilinear terms
 *
 * This file is licensed under the Common Public License (CPL)
 */

#include "CouenneExprMul.hpp"
#include "CouenneExprTrilinear.hpp"
#include "CouennePrecisions.hpp"
#include "CoinFinite.hpp"

using namespace Couenne;


/// implied bound processing for expression w = x*y*z, upon change in
/// lower- and/or upper bound of w, whose index is wind

bool exprTrilinear::impliedBound (int wind, CouNumber *l, CouNumber *u, t_chg_bounds *chg, enum auxSign sign) {

  // in general, for i in {1,2,3},
  //
  // x [i] >= min {w / (x[(i+1) % 3] x[(i+2) % 3]): all variables in bounds}
  // x [i] <= max {w / (x[(i+1) % 3] x[(i+2) % 3]): all variables in bounds}
  //
  // There are cases:
  //
  // 

  return false;
}

