/*
 * Name:    domain.cpp
 * Author:  Pietro Belotti
 * Purpose: implementation of methods of classes for point and bounding box
 *
 * (C) Carnegie-Mellon University, 2008-09.
 * This file is licensed under the Common Public License (CPL)
 */

#include "CoinHelperFunctions.hpp"
#include "domain.hpp"
#include "CouennePrecisions.hpp"

#define EXTRA_STORAGE 1024

/// constructor
DomainPoint::DomainPoint (int dim, 
			  CouNumber *x, 
			  CouNumber *lb, 
			  CouNumber *ub,
			  bool copy):
  dimension_ (dim),
  x_         (x),
  lb_        (lb),
  ub_        (ub),
  copied_    (copy) {

  if ((dimension_ > 0) && copied_) {

    x_  = (CouNumber *) malloc (dim * sizeof (CouNumber)); 
    lb_ = (CouNumber *) malloc (dim * sizeof (CouNumber)); 
    ub_ = (CouNumber *) malloc (dim * sizeof (CouNumber)); 

    if (x)  CoinCopyN (x,  dim, x_);  else CoinFillN (x_,  dim, 0.);
    if (lb) CoinCopyN (lb, dim, lb_); else CoinFillN (lb_, dim, -COUENNE_INFINITY);
    if (ub) CoinCopyN (ub, dim, ub_); else CoinFillN (ub_, dim,  COUENNE_INFINITY);
  }
}


/// constructor
DomainPoint::DomainPoint (int dim, 
			  const CouNumber *x, 
			  const CouNumber *lb, 
			  const CouNumber *ub):
  dimension_ (dim),
  x_         (NULL),
  lb_        (NULL),
  ub_        (NULL),
  copied_    (true) {

  if (dimension_ > 0) {

    x_  = (CouNumber *) malloc (dim * sizeof (CouNumber)); 
    lb_ = (CouNumber *) malloc (dim * sizeof (CouNumber)); 
    ub_ = (CouNumber *) malloc (dim * sizeof (CouNumber)); 

    if (x)  CoinCopyN (x,  dim, x_);  else CoinFillN (x_,  dim, 0.);
    if (lb) CoinCopyN (lb, dim, lb_); else CoinFillN (lb_, dim, -COUENNE_INFINITY);
    if (ub) CoinCopyN (ub, dim, ub_); else CoinFillN (ub_, dim,  COUENNE_INFINITY);
  }
}


/// copy constructor
DomainPoint::DomainPoint (const DomainPoint &src):
  dimension_ (src.dimension_),
  x_         (src.x_),
  lb_        (src.lb_),
  ub_        (src.ub_),
  copied_    (src.copied_) {

  if ((dimension_ > 0) && copied_) {

    x_  = (CouNumber *) malloc (dimension_ * sizeof (CouNumber));
    lb_ = (CouNumber *) malloc (dimension_ * sizeof (CouNumber));
    ub_ = (CouNumber *) malloc (dimension_ * sizeof (CouNumber));

    CoinCopyN (src.x_,  dimension_, x_);
    CoinCopyN (src.lb_, dimension_, lb_);
    CoinCopyN (src.ub_, dimension_, ub_);
  }
}


/// resize domain point (for extending into higher space)
void DomainPoint::resize (int newdim) {

  if (newdim == dimension_) return;

  assert (copied_); // cannot resize domain point as it was not copied

  if (newdim==0) { // clear

    free (x_);  x_  = NULL;
    free (lb_); lb_ = NULL;
    free (ub_); ub_ = NULL;

    dimension_ = newdim;

  } else if (newdim < dimension_) { // resize exactly

    x_  = (CouNumber *) realloc (x_,  newdim * sizeof (CouNumber)); 
    lb_ = (CouNumber *) realloc (lb_, newdim * sizeof (CouNumber)); 
    ub_ = (CouNumber *) realloc (ub_, newdim * sizeof (CouNumber)); 

    dimension_ = newdim;

  } else if (newdim > dimension_) { // expand and leave some extra space

    newdim += EXTRA_STORAGE;

    x_  = (CouNumber *) realloc (x_,  newdim * sizeof (CouNumber)); 
    lb_ = (CouNumber *) realloc (lb_, newdim * sizeof (CouNumber)); 
    ub_ = (CouNumber *) realloc (ub_, newdim * sizeof (CouNumber)); 

    dimension_ = newdim;
  }
}


/// assignment operator
DomainPoint &DomainPoint::operator= (const DomainPoint &src) {

  copied_ = src.copied_;

  if (src.dimension_ != dimension_) {
    if (x_)  free (x_);  x_  = (CouNumber *) malloc (src.dimension_ * sizeof (CouNumber));
    if (lb_) free (lb_); lb_ = (CouNumber *) malloc (src.dimension_ * sizeof (CouNumber));
    if (ub_) free (ub_); ub_ = (CouNumber *) malloc (src.dimension_ * sizeof (CouNumber));
    dimension_ = src.dimension_;
  }

  CoinCopyN (src.x_,  dimension_, x_);
  CoinCopyN (src.lb_, dimension_, lb_);
  CoinCopyN (src.ub_, dimension_, ub_);

  return *this;
}


/// destructor
Domain::~Domain () {

  if (point_) 
    delete point_;

  while (!(domStack_.empty ())) {
    delete domStack_.top ();
    domStack_.pop ();
  }
}


/// save current point and start using another
void Domain::push (int dim, CouNumber *x, CouNumber *lb, CouNumber *ub, bool copy) {

  if (!x)  x  = point_ -> x  ();
  if (!lb) lb = point_ -> lb ();
  if (!ub) ub = point_ -> ub ();

  if (point_)
    domStack_.push (point_);

  point_ = new DomainPoint (dim, x, lb, ub, copy);
}


/// save current point and start using another
void Domain::push (int dim, 
		   const CouNumber *x, 
		   const CouNumber *lb, 
		   const CouNumber *ub) {

  if (point_) 
    domStack_.push (point_);

  point_ = new DomainPoint (dim, x, lb, ub);
}


/// save current point and start using another
void Domain::push (const DomainPoint &dp, bool copy) {

  if (point_)
    domStack_.push (point_);
  point_ = new DomainPoint (dp);
}


/// restore previous point
void Domain::pop () {

  delete point_;
  if (!(domStack_.empty ())) {
    point_ = domStack_.top ();
    domStack_.pop ();
  }
  else point_ = NULL;
}

/*int main (int argc, char **argv) {

CouNumber 
x1 [] = {1,2,3},
l1 [] = {0,-1,-2},
u1 [] = {2,5,8},

x3 [] = {14,15,16,17,18,19},
l3 [] = {-100,-100,-100,-100,-100,-100},
u3 [] = {10,10,10,10,10,10},

x2 [] = {5001,5002,5003,5006},
l2 [] = {4000,3000,2000,1000},
u2 [] = {5000,6000,8000,6000};

Domain dom;

for (int i=80; i--;) {
dom.push (3,x1,l1,u1);printf("1: %g %g %g\n", dom.x(1),dom.x(2),dom.x(0));
dom.push(5,x3,l3,u3);printf("2: %g %g %g %g %g\n",dom.x(1),dom.x(2),dom.x(3),dom.x(4),dom.x(0));
dom.push (4,x2,l2,u2); printf ("3: %g %g %g %g\n", dom.x(1),dom.x(2),dom.x(3),dom.x(0));
dom.push (3,x1,l1,u1); printf ("4: %g %g %g\n", dom.x(1),dom.x(2),dom.x(0));
dom.push (4,x2,l2,u2); printf ("5: %g %g %g %g\n", dom.x(1),dom.x(2),dom.x(3),dom.x(0));
dom.push(5,x3,l3,u3);printf("6: %g %g %g %g %g\n",dom.x(1),dom.x(2),dom.x(3),dom.x(4),dom.x(0));
dom.pop ();
dom.pop ();
dom.pop ();
dom.pop ();
dom.pop ();
dom.pop ();
}
}*/
