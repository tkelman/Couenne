/*
 * Name:    aggressiveBT.cpp
 * Author:  Pietro Belotti
 * Purpose: aggressive bound tightening -- fake bounds in variables to
 *          exclude parts of the solution space through fathoming on
 *          bounds/infeasibility
 *
 * (C) Carnegie-Mellon University, 2007-09.
 * This file is licensed under the Common Public License (CPL)
 */

#include "BonTNLPSolver.hpp"
#include "BonNlpHeuristic.hpp"
#include "CoinHelperFunctions.hpp"
#include "BonCouenneInfo.hpp"

#include "CouenneCutGenerator.hpp"
#include "CouenneProblem.hpp"

#define MAX_ABT_ITER           1  // max # aggressive BT iterations
#define THRES_ABT_IMPROVED     0  // only continue ABT if at least these bounds have improved
#define THRES_ABT_ORIG      1000  // only do ABT on originals if they are more than this 

static double distanceToBound(int n, const double* xOrig,
			      const double* lower, const double* upper)
{
  double Xdist = 0.;
  for (int i=0; i<n; i++) {
    if (lower[i] > xOrig[i]) {
      Xdist += lower[i] - xOrig[i];
    }
    if (upper[i] < xOrig[i]) {
      Xdist += xOrig[i] - upper[i];
    }
  }
  return Xdist;
}

// Aggressive Bound Tightening: for each variable, fake new bounds
// [l,b] or [b,u] and apply bound tightening. If the interval is
// fathomed on bounds or on infeasibility, the complementary bound
// interval is a valid tightening.

bool CouenneProblem::aggressiveBT (Bonmin::OsiTMINLPInterface *nlp,
				   t_chg_bounds *chg_bds, 
				   Bonmin::BabInfo * babInfo) const {

  Jnlst()->Printf (J_ITERSUMMARY, J_BOUNDTIGHTENING, "Aggressive FBBT\n");

  Bonmin::CouenneInfo* couInfo =
    dynamic_cast<Bonmin::CouenneInfo*>(babInfo);

  int  ncols  = nVars ();
  bool retval = true;

  CouNumber
    *olb = new CouNumber [ncols],
    *oub = new CouNumber [ncols];

  // save current bounds
  CoinCopyN (Lb (), ncols, olb);
  CoinCopyN (Ub (), ncols, oub);

  // Find the solution that is closest to the current bounds
  // TODO: Also check obj value
  SmartPtr<const Bonmin::CouenneInfo::NlpSolution> closestSol;
  double dist = 1e50;

  const std::list<SmartPtr<const Bonmin::CouenneInfo::NlpSolution> >& solList =
    couInfo->NlpSolutions();

  for (std::list<SmartPtr<const Bonmin::CouenneInfo::NlpSolution> >::const_iterator 
	 i = solList.begin();
       i != solList.end(); i++) {
    assert(nOrigVars_ == (*i)->nVars());
    const double thisDist = distanceToBound(nOrigVars_, (*i)->solution(), olb, oub);
    if (thisDist < dist) {
      closestSol = *i;
      dist = thisDist;
    }
  }

  jnlst_ -> Printf(J_VECTOR, J_BOUNDTIGHTENING, "best dist = %e\n", dist);

  // If this solution is not sufficiently inside the bounds, we solve the NLP now
  if (dist > 0.1) { // TODO: Find tolerance

    // find integer initial point /////////////////////////

    int nvars = nVars ();

    double *lower = new double [nvars];
    double *upper = new double [nvars];

    CoinFillN (lower, nvars, -COUENNE_INFINITY);
    CoinFillN (upper, nvars,  COUENNE_INFINITY);

    CoinCopyN (nlp -> getColLower (), nOrigVars_, lower);
    CoinCopyN (nlp -> getColUpper (), nOrigVars_, upper);

    double *Y = new double [nvars];
    CoinFillN (Y, nvars, 0.);
    CoinCopyN (X (), nOrigVars_, Y);

    if (getIntegerCandidate (nlp -> getColSolution (), Y, lower, upper) < 0) {
      jnlst_ -> Printf(J_ITERSUMMARY, J_BOUNDTIGHTENING, "TODO: find NLP point in ABT failed\n");
      delete [] Y;
      delete [] lower;
      delete [] upper;
      return true;
    }

    //////////////////////////////////////////////////////

    nlp -> setColLower    (lower);
    nlp -> setColUpper    (upper);
    nlp -> setColSolution (Y);

    try {
      nlp -> initialSolve ();
    }
    catch (Bonmin::TNLPSolver::UnsolvedError *E) {
    }
    
    delete [] Y;
    delete [] lower;
    delete [] upper;

    if (nlp->isProvenOptimal()) {
      closestSol = new Bonmin::CouenneInfo::NlpSolution 
	(nOrigVars_, nlp->getColSolution(), nlp->getObjValue());
      couInfo->addSolution(closestSol);
      dist = 0.;
    }
    else {
      jnlst_ -> Printf(J_ITERSUMMARY, J_BOUNDTIGHTENING, "TODO: NLP solve in ABT failed\n");
      return true;
    }
  }

  if (dist>1e10) {
    jnlst_ -> Printf(J_ITERSUMMARY, J_BOUNDTIGHTENING, "TODO: Don't have point for ABT\n");
    return true;
  }

  // X is now the NLP solution, but in a low-dimensional space. We
  // have to get the corresponding point in higher dimensional space
  // through getAuxs()

  double *X = new double [ncols];
  CoinCopyN (closestSol->solution(), nOrigVars_, X);
  getAuxs (X);

  // create a new, fictitious, bound bookkeeping structure
  t_chg_bounds *f_chg = new t_chg_bounds [ncols];

  if (Jnlst()->ProduceOutput(J_ITERSUMMARY, J_BOUNDTIGHTENING)) {
    //    CouNumber cutoff = getCutOff ();
    int       objind = Obj (0) -> Body  () -> Index ();
    for (int i=0; i<nOrigVars_; i++)
      Jnlst()->Printf(J_MOREVECTOR, J_BOUNDTIGHTENING,
		      "   %2d %+20g [%+20g %+20g]\n",
		      i, X [i], Lb (i), Ub (i));
    Jnlst()->Printf(J_ITERSUMMARY, J_BOUNDTIGHTENING,
		    "-------------\nAggressive BT. Current bound = %g, cutoff = %g, %d vars\n", 
		    Lb (objind), getCutOff (), ncols);
  }

  int improved, second, iter = 0;

  // Repeatedly fake tightening bounds on both sides of every variable
  // to concentrate around current NLP point.
  //
  // MAX_ABT_ITER is the maximum # of outer cycles. Each call to
  // fake_tighten in turn has an iterative algorithm for a
  // derivative-free, uni-dimensional optimization problem on a
  // monotone function.

  do {

    improved = 0;

    // scan all variables
    for (int i=0; i<ncols; i++) {

      if (CoinCpuTime () > maxCpuTime_)
	break;

      int index = evalOrder (i);

      if (Var (index) -> Multiplicity () <= 0) 
	continue;

      // AW: We only want to do the loop that temporarily changes
      // bounds around the NLP solution only for those points from the
      // NLP solution (no auxiliary vars)?

      // PBe: if we do want that, index should be initialized as i, as
      // evalOrder gives a variable index out of an array index.

      // PBe: That makes a lot of sense when problems are really
      // big. Instances arki000[24].nl spend a lot of time here

      if ((nOrigVars_ < THRES_ABT_ORIG) || (index < nOrigVars_)) {

	// if (index == objind) continue; // don't do it on objective function

	Jnlst()->Printf(J_DETAILED, J_BOUNDTIGHTENING,
			"------------- tighten left x%d\n", index);

	// tighten on left
	if ((X [index] >= Lb (index) + COUENNE_EPS)
	    && ((improved = fake_tighten (0, index, X, olb, oub, chg_bds, f_chg)) < 0)) {
	  retval = false;
	  break;
	}

	second = 0;

	Jnlst()->Printf(J_DETAILED, J_BOUNDTIGHTENING,
			"------------- tighten right x%d\n", index);

	// tighten on right
	if ((X [index] <= Ub (index) - COUENNE_EPS)
	    && ((second = fake_tighten (1, index, X, olb, oub, chg_bds, f_chg) < 0))) {
	  retval = false;
	  break;
	}

	improved += second;
      }
    }
  } while (retval && (improved > THRES_ABT_IMPROVED) && (iter++ < MAX_ABT_ITER));

  // store new valid bounds, or restore old ones if none changed
  CoinCopyN (olb, ncols, Lb ());
  CoinCopyN (oub, ncols, Ub ());

  if (Jnlst()->ProduceOutput(J_ITERSUMMARY, J_BOUNDTIGHTENING)) {
    Jnlst()->Printf(J_ITERSUMMARY, J_BOUNDTIGHTENING,"------------------\n");

    if (!retval) Jnlst()->Printf(J_ITERSUMMARY, J_BOUNDTIGHTENING,
				 "Couenne infeasible node from aggressive BT\n");

    int objind = Obj (0) -> Body  () -> Index ();

    Jnlst()->Printf(J_ITERSUMMARY, J_BOUNDTIGHTENING,
		    "-------------\ndone Aggressive BT. Current bound = %g, cutoff = %g, %d vars\n", 
		    Lb (objind), getCutOff (), ncols);

    if (Jnlst()->ProduceOutput(J_DETAILED, J_BOUNDTIGHTENING))
      for (int i=0; i<nOrigVars_; i++)
	printf("   x%02d [%+20g %+20g]  | %+20g\n",
	       i, Lb (i), Ub (i), X [i]);

    if (Jnlst()->ProduceOutput(J_MOREDETAILED, J_BOUNDTIGHTENING))
      for (int i=nOrigVars_; i<ncols; i++)
	printf ("   w%02d [%+20g %+20g]  | %+20g\n", i, Lb (i), Ub (i), X [i]);
  }

  delete [] X;
  delete [] f_chg;
  delete [] olb;
  delete [] oub;

  return retval;// && btCore (psi, cs, chg_bds, babInfo, true); // !!!
  //return retval && btCore (psi, cs, chg_bds, babInfo, true);
}
