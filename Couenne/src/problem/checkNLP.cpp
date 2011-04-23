/* $Id$
 *
 * Name:    checkNLP.cpp
 * Author:  Pietro Belotti
 * Purpose: check NLP feasibility of incumbent integer solution
 *
 * (C) Carnegie-Mellon University, 2006-10.
 * This file is licensed under the Eclipse Public License (EPL)
 */

#include "CouenneProblem.hpp"
#include "CouenneProblemElem.hpp"
#include "CouenneExprVar.hpp"
#include "CoinHelperFunctions.hpp"
#include "CoinFinite.hpp"

using namespace Couenne;

// check if solution is MINLP feasible
bool CouenneProblem::checkNLP (const double *solution, double &obj, bool recompute) const {

  if (Jnlst () -> ProduceOutput (Ipopt::J_ALL, J_PROBLEM)) {

    printf ("Checking solution: %.12e (", obj);

    for (int i=0; i<nOrigVars_; i++)
      printf ("%g ", solution [i]);
    printf (")\n");
  }

  // pre-check on original variables --- this is done after every LP,
  // and should be efficient
  for (register int i=0; i < nOrigVars_; i++) {

    if (variables_ [i] -> Multiplicity () <= 0) 
      continue;

    CouNumber val = solution [i];

    // check (original and auxiliary) variables' integrality

    exprVar *v = variables_ [i];

    if ((v -> Type ()      == VAR) &&
	(v -> isInteger ())        &&
	(v -> Multiplicity () > 0) &&
	(fabs (val - COUENNE_round (val)) > feas_tolerance_)) {

      Jnlst()->Printf(Ipopt::J_MOREVECTOR, J_PROBLEM,
		      "checkNLP: integrality %d violated: %.6f [%g,%g]\n", 
		      i, val, domain_.lb (i), domain_.ub (i));

      Jnlst () -> Printf (Ipopt::J_MOREVECTOR, J_PROBLEM, "Done: (0)\n");

      return false;
    }
  }

  const int infeasible = 1;
  const int wrong_obj  = 2;

  CouNumber *sol = new CouNumber [nVars ()];

  // copy solution, evaluate the corresponding aux, and then replace
  // the original variables again for checking
  CoinCopyN (solution, nOrigVars_, sol);
  getAuxs (sol);
  CoinCopyN (solution, nOrigVars_, sol);

  // install NL solution candidate in evaluation structure
  domain_.push (nVars (), sol, domain_.lb (), domain_.ub (), false);

  if (Jnlst () -> ProduceOutput (Ipopt::J_ALL, J_PROBLEM)) {
    printf ("  checkNLP: %d vars -------------------\n    ", domain_.current () -> Dimension ());
    for (int i=0; i<domain_.current () -> Dimension (); i++) {
      if (i && !(i%5)) printf ("\n    ");
      printf ("%4d %16g [%16e %16e]  ", i, domain_.x (i), domain_.lb (i), domain_.ub (i));
    }
  }

  expression *objBody = Obj (0) -> Body ();

  // BUG: if Ipopt solution violates bounds of original variables and
  // objective depends on originals, we may have a "computed object"
  // out of bounds

  //CouNumber realobj = (*(objBody -> Image () ? objBody -> Image () : objBody)) ();
  CouNumber realobj = obj;

  if (objBody) 
    realobj = 
      (objBody -> Index () >= 0) ?
      sol [objBody -> Index ()] : 
      (*(objBody -> Image () ? objBody -> Image () : objBody)) ();

  if (Jnlst () -> ProduceOutput (Ipopt::J_ALL, J_PROBLEM)) {
    printf ("  Objective: %.12e %.12e %.12e\n", 
	    realobj, sol [objBody -> Index ()], 
	    (*(objBody -> Image () ? objBody -> Image () : objBody)) ());
  }

  bool retval = true;

  try {

    // check if objective corresponds
    
    if (fabs (realobj - obj) / (1. + fabs (realobj)) > feas_tolerance_) {

      Jnlst()->Printf(Ipopt::J_MOREVECTOR, J_PROBLEM,
		      "  checkNLP, false objective: computed %g != %g xQ (diff. %g)\n", 
		      realobj, obj, realobj - obj);

      if (!recompute)
	throw wrong_obj;
    }

    if (recompute)
      obj = realobj;

    if (Jnlst () -> ProduceOutput (Ipopt::J_ALL, J_PROBLEM))
      printf ("  recomputed: %.12e\n", obj);

    for (int i=0; i < nOrigVars_; i++) {

      if (variables_ [i] -> Multiplicity () <= 0) 
	continue;

      CouNumber val = domain_.x (i);

      // check bounds

      if ((val > domain_.ub (i) + feas_tolerance_) ||
	  (val < domain_.lb (i) - feas_tolerance_)) {

	Jnlst()->Printf(Ipopt::J_MOREVECTOR, J_PROBLEM,
			"  checkNLP: variable %d out of bounds: %.6f [%g,%g] (diff %g)\n", 
			i, val, domain_.lb (i), domain_.ub (i),
			CoinMax (fabs (val - domain_.lb (i)), 
				 fabs (val - domain_.ub (i))));
	throw infeasible;
      }

      // check (original and auxiliary) variables' integrality

      if (variables_ [i] -> isInteger () &&
	  (fabs (val - COUENNE_round (val)) > feas_tolerance_)) {

	Jnlst()->Printf(Ipopt::J_MOREVECTOR, J_PROBLEM,
			"  checkNLP: integrality %d violated: %.6f [%g,%g]\n", 
			i, val, domain_.lb (i), domain_.ub (i));

	throw infeasible;
      }
    }

    // check ALL auxs

    for (int i=0; i < nVars (); i++) {

      exprVar *v = variables_ [i];

      if ((v -> Type         () != AUX) ||
	  (v -> Multiplicity () <= 0))
	continue;

      if (Jnlst () -> ProduceOutput (Ipopt::J_ALL, J_PROBLEM)) {
	printf ("    "); v -> print (); 
	CouNumber
	  val = (*(v)) (), 
	  img = (*(v -> Image ())) (), 
	  diff = fabs (val - img);
	printf (": val = %15g; img = %-15g ", val, img);
	if (diff > 1e-9)
	  printf ("[diff %12e] ", diff);
	//for (int j=0; j<nVars (); j++) printf ("%.12e ", (*(variables_ [j])) ());
	v -> Image () -> print (); 
	printf ("\n");
      }
      
      // check if auxiliary has zero infeasibility

      // same as in CouenneObject::checkInfeasibility -- main difference is use of gradientNorm()

      double 
	vval = (*v) (),
	fval = (*(v -> Image ())) (),
	denom  = CoinMax (1., v -> Image () -> gradientNorm (X ()));

      // check if fval is a number (happens with e.g. w13 = w12/w5 and w5=0, see test/harker.nl)
      if (CoinIsnan (fval)) {
	fval = vval + 1.;
	denom = 1.;
      }

      if (fabs (fval) > COUENNE_INFINITY)
	fval = COUENNE_INFINITY;

      double
	delta = 
	((v -> sign () == expression::AUX_GEQ) && (vval >= fval)) ? 0. : 
	((v -> sign () == expression::AUX_LEQ) && (vval <= fval)) ? 0. : fabs (vval - fval),

	ratio = (CoinMax (1., fabs (vval)) / 
		 CoinMax (1., fabs (fval)));

      //printf ("checkinf --> v=%e f=%e den=%e ret=%e ratio=%e\n", vval, fval, denom, retval, ratio);

      if (delta > 0. && ((ratio > 2.)  ||  // check delta > 0 to take into account semi-auxs
			 (ratio <  .5)) &&
	  ((delta /= denom) > CoinMin (COUENNE_EPS, feas_tolerance_))) {

	Jnlst () -> Printf (Ipopt::J_MOREVECTOR, J_PROBLEM,
			    "  checkNLP: auxiliary %d violates tolerance %g by %g/%g = %g\n", 
			    i, CoinMin (COUENNE_EPS, feas_tolerance_), delta*denom, denom, delta);

	throw infeasible;
      }
    }

    // check constraints

    for (int i=0; i < nCons (); i++) {

      CouenneConstraint *c = Con (i);

      CouNumber
	body = (*(c -> Body ())) (),
	lhs  = (*(c -> Lb   ())) (),
	rhs  = (*(c -> Ub   ())) ();

      if (((rhs <  COUENNE_INFINITY) && (body > rhs + feas_tolerance_ * (1. + CoinMax (fabs (body), fabs (rhs))))) || 
	  ((lhs > -COUENNE_INFINITY) && (body < lhs - feas_tolerance_ * (1. + CoinMax (fabs (body), fabs (lhs)))))) {

	if (Jnlst () -> ProduceOutput (Ipopt::J_MOREVECTOR, J_PROBLEM)) {

	  printf ("  checkNLP: constraint %d violated (lhs=%+e body=%+e rhs=%+e, violation %g): ",
		  i, lhs, body, rhs, CoinMax (lhs-body, body-rhs));

	  c -> print ();
	}

	throw infeasible;
      }
    }
  }

  catch (int exception) {

    switch (exception) {

    case wrong_obj:
      retval = false;
      break;

    case infeasible:
    default:
      retval = false;
      break;
    }
  }

  delete [] sol;
  domain_.pop ();

  Jnlst () -> Printf (Ipopt::J_MOREVECTOR, J_PROBLEM, "Done: %d\n", retval);

  return retval;
}
