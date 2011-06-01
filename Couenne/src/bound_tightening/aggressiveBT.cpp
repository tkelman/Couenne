/* $Id$
 *
 * Name:    aggressiveBT.cpp
 * Author:  Pietro Belotti
 * Purpose: probing -- fake bounds in variables to exclude parts of
 *          the solution space through fathoming on
 *          bounds/infeasibility
 *
 * (C) Carnegie-Mellon University, 2007-11.
 * This file is licensed under the Eclipse Public License (EPL)
 */

#include "CouenneCutGenerator.hpp"

#include "BonTNLPSolver.hpp"
#include "BonNlpHeuristic.hpp"
#include "CoinHelperFunctions.hpp"
#include "BonCouenneInfo.hpp"

#include "CouenneProblem.hpp"
#include "CouenneExprVar.hpp"
#include "CouenneProblemElem.hpp"

using namespace Couenne;

namespace Bonmin {
  class OsiTMINLPInterface;
  class BabInfo;
  class TNLPSolver;
}

#define MAX_ABT_ITER           4  // max # aggressive BT iterations
#define THRES_ABT_IMPROVED     0  // only continue ABT if at least these bounds have improved
#define THRES_ABT_ORIG       100  // only do ABT on auxiliaries if they are less originals than this 

static double distanceToBound (int n, const double* xOrig,
			       const double* lower, const double* upper) {

  double Xdist = 0.;

  for (int i=0, j=n; j--; i++) {

    CouNumber 
      diff, 
      xO = xOrig [i];

    if      ((diff = lower [i] - xO) > 0) Xdist += diff;
    else if ((diff = xO - upper [i]) > 0) Xdist += diff;
  }

  return Xdist;
}


// Probing: for each variable, fake new bounds [l,b] or [b,u], with
// given b, and apply bound tightening. If the interval is fathomed on
// bounds or on infeasibility, the complementary bound interval is a
// valid tightening.

bool CouenneProblem::aggressiveBT (Bonmin::OsiTMINLPInterface *nlp,
				   t_chg_bounds *chg_bds, 
				   const CglTreeInfo &info,
				   Bonmin::BabInfo * babInfo) const {

  Jnlst () -> Printf (J_ITERSUMMARY, J_BOUNDTIGHTENING, "Probing\n");

  if (info.level <= 0 && !(info.inTree))  {
    jnlst_ -> Printf (J_ERROR, J_COUENNE, "Probing: ");
    fflush (stdout);
  }

  CouenneInfo* couInfo =
    dynamic_cast <CouenneInfo *> (babInfo);

  int  ncols  = nVars ();
  bool retval = false;

  CouNumber
    *olb = new CouNumber [ncols],
    *oub = new CouNumber [ncols];

  // save current bounds
  CoinCopyN (Lb (), ncols, olb);
  CoinCopyN (Ub (), ncols, oub);

  // Find the solution that is closest to the current bounds
  // TODO: Also check obj value
  SmartPtr<const CouenneInfo::NlpSolution> closestSol;
  double dist = 1e50;

  if (couInfo) {

    const std::list<SmartPtr<const CouenneInfo::NlpSolution> >& solList =
      couInfo->NlpSolutions();

    for (std::list<SmartPtr<const CouenneInfo::NlpSolution> >::const_iterator 
	   i = solList.begin();
	 i != solList.end(); i++) {
      assert(nOrigVars_ == (*i)->nVars());
      const double thisDist = distanceToBound(nOrigVars_, (*i)->solution(), olb, oub);
      if (thisDist < dist) {
	closestSol = *i;
	dist = thisDist;
      }
    }
  }

  jnlst_ -> Printf (J_VECTOR, J_BOUNDTIGHTENING, "best dist = %e\n", dist);

  bool haveNLPsol = false;

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

    CoinFillN (Y,    nvars,      0.);
    CoinCopyN (X (), nOrigVars_, Y);

    if (getIntegerCandidate (nlp -> getColSolution (), Y, lower, upper) < 0) {

      jnlst_ -> Printf (J_ITERSUMMARY, J_BOUNDTIGHTENING, "TODO: find NLP point in ABT failed\n");
      retval = true;

    } else {

      //////////////////////////////////////////////////////

      nlp -> setColLower    (lower);
      nlp -> setColUpper    (upper);
      nlp -> setColSolution (Y);

      try {
	nlp -> options () -> SetNumericValue ("max_cpu_time", CoinMax (0., getMaxCpuTime () - CoinCpuTime ()));
	nlp -> initialSolve ();
      }
      catch (Bonmin::TNLPSolver::UnsolvedError *E) {}
    
      delete [] Y;
      delete [] lower;
      delete [] upper;

      if (nlp->isProvenOptimal()) {

	if (couInfo) {
	  closestSol = new CouenneInfo::NlpSolution 
	    (nOrigVars_, nlp->getColSolution(), nlp->getObjValue());
	  couInfo->addSolution(closestSol);
	  dist = 0.;
	  haveNLPsol = true;      
	}
      }
      else {
	jnlst_ -> Printf(J_ITERSUMMARY, J_BOUNDTIGHTENING, "TODO: NLP solve in ABT failed\n");
	retval = true;
      }
    }
  }

  int nTotImproved = 0;

  // Probing can also run on an LP point.

  //if (true || (retval && (dist < 1e10))) {

  {
    retval = true;

    // X is now the NLP solution, but in a low-dimensional space. We
    // have to get the corresponding point in higher dimensional space
    // through getAuxs()

    double *X = NULL;

    if (haveNLPsol) {

      X = new double [ncols];
      CoinCopyN (closestSol -> solution(), nOrigVars_, X);
      getAuxs (X);
    } else X = domain () -> x ();

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

      bool maxTimeReached = false;

      improved = 0;

      // scan all variables
      for (int i=0; i<ncols; i++) {

	if (CoinCpuTime () > maxCpuTime_) {
	  maxTimeReached = true; // avoids getrusage...
	  break;
	}

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

	  improved = 0;

	  if ((variables_ [index] -> sign () != expression::AUX_GEQ) &&
	      (X [index] >= Lb (index) + COUENNE_EPS)) {

	    Jnlst()->Printf(J_DETAILED, J_BOUNDTIGHTENING,
			    "------------- tighten left x%d\n", index);

	    // tighten on left
	    if ((improved = fake_tighten (0, index, X, olb, oub, chg_bds, f_chg)) < 0) {

	      retval = false;
	      break;
	    }
	  }

	  second = 0;

	  if (retval && (variables_ [index] -> sign () != expression::AUX_LEQ) &&
	      (X [index] <= Ub (index) - COUENNE_EPS)) {
	    Jnlst()->Printf(J_DETAILED, J_BOUNDTIGHTENING,
			    "------------- tighten right x%d\n", index);

	    // tighten on right
	    if ((second = fake_tighten (1, index, X, olb, oub, chg_bds, f_chg) < 0)) {
	      retval = false;
	      break;
	    }
	  }

	  improved += second;
	  nTotImproved += improved;
	}
      }

      if (maxTimeReached)
	break;

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

    if (haveNLPsol)
      delete [] X;
    delete [] f_chg;    
  }
  // else
  // if ((dist > 1e10) && !retval)
  //   jnlst_ -> Printf(J_ITERSUMMARY, J_BOUNDTIGHTENING, "TODO: Don't have point for ABT\n");

  delete [] olb;
  delete [] oub;

  if (info.level <= 0 && !(info.inTree))  {
    if (!retval) jnlst_ -> Printf (J_ERROR, J_COUENNE, "done (infeasible)\n");
    else         jnlst_ -> Printf (J_ERROR, J_COUENNE, "done (%d improved bounds)\n", nTotImproved);
  }

  return retval; // && btCore (psi, cs, chg_bds, babInfo, true); // !!!
  //return retval && btCore (psi, cs, chg_bds, babInfo, true);
}
