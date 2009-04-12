// (C) Copyright International Business Machines Corporation and Carnegie Mellon University 2006, 2007
// All Rights Reserved.
// This code is published under the Common Public License.
//
// Authors :
// Pietro Belotti, Carnegie Mellon University,
// Pierre Bonami, International Business Machines Corporation
//
// Date : 12/19/2006


#if defined(_MSC_VER)
// Turn off compiler warning about long names
#  pragma warning(disable:4786)
#endif
#include <iomanip>
#include <fstream>

#include <stdlib.h>

#include "CoinTime.hpp"
#include "CoinError.hpp"
#include "BonCouenneInterface.hpp"

#include "BonCouenneSetup.hpp"

#include "BonCbc.hpp"
#ifdef COIN_HAS_FILTERSQP
#include "BonFilterSolver.hpp"
#endif

#include "CbcCutGenerator.hpp"
#include "CouenneProblem.hpp"
#include "CouenneCutGenerator.hpp"

using namespace Bonmin;

// the maximum difference between a printed optimum and a CouNumber
#define PRINTED_PRECISION 1e-5

#include "exprVar.hpp"
#include "exprConst.hpp"
#include "exprSum.hpp"
#include "exprClone.hpp"
#include "CouenneProblemElem.hpp"
#include "CouenneProblem.hpp"

int main (int argc, char *argv[]) {

  //WindowsErrorPopupBlocker();
  using namespace Ipopt;

  char * pbName = NULL;
  double time_start = CoinCpuTime();

  const int infeasible = 1;

  try {

    Bab bb;
    bb.setUsingCouenne (true);

    CouenneProblem *p = NULL;
    CouenneInterface *ci = NULL;

#if 0
    ci = new CouenneInterface;
    p = new CouenneProblem;

    p -> addVariable (false, p -> domain ());
    p -> addVariable (false, p -> domain ());
    p -> addVariable (false, p -> domain ());
    p -> addVariable (false, p -> domain ());

    p -> addObjective    (new exprSum (new exprClone (p->Var (1)), new exprClone (p->Var (2))), "min");
    p -> addLEConstraint (new exprSum (new exprClone (p->Var (0)), new exprClone (p->Var (2))), 
			  new exprConst (1));
    p -> addEQConstraint (new exprSum (new exprClone (p->Var (1)), new exprClone (p->Var (2))), 
			  new exprConst (1));
    p -> addEQConstraint (new exprSum (new exprClone (p->Var (1)), new exprClone (p->Var (3))), 
			  new exprConst (1));
    p -> addEQConstraint (new exprSum (new exprClone (p->Var (2)), new exprClone (p->Var (3))), 
			  new exprConst (1));
#endif

    CouenneSetup couenne;
    if (!couenne.InitializeCouenne (argv, p, ci))
      throw infeasible;

#if 0
    CouenneFeasibility feasibility;
    bb.model().setProblemFeasibility (feasibility);
#endif

    /// update time limit (read/preprocessing might have taken some)
    double timeLimit = 0;
    couenne.options () -> GetNumericValue ("time_limit", timeLimit, "couenne.");
    couenne.setDoubleParameter (BabSetupBase::MaxTime, 
				timeLimit - (time_start = (CoinCpuTime () - time_start)));

    //////////////////////////////////

    bb (couenne); // do branch and bound

    //////////////////////////////////

    std::cout.precision (10);

    //////////////////////////////
    CouenneCutGenerator *cg = NULL;

    if (bb.model (). cutGenerators ())
      cg = dynamic_cast <CouenneCutGenerator *> 
	(bb.model (). cutGenerators () [0] -> generator ());

    ////////////////////////////////
    int nr=-1, nt=-1;
    double st=-1;

    if (cg) cg -> getStats (nr, nt, st);
    else printf ("Warning, could not get pointer to CouenneCutGenerator\n");

    CouenneProblem *cp = cg ? cg -> Problem () : NULL;

    // retrieve test value to check
    double global_opt;
    couenne.options () -> GetNumericValue ("couenne_check", global_opt, "couenne.");

    if (global_opt < COUENNE_INFINITY) { // some value found in couenne.opt

      double opt = bb.model (). getBestPossibleObjValue ();

      printf ("Global Optimum Test on %-40s %s\n", 
	      cp ? cp -> problemName ().c_str () : "unknown", 
	      (fabs (opt - global_opt) / 
	       (1. + CoinMax (fabs (opt), fabs (global_opt))) < PRINTED_PRECISION) ? 
	      (const char *) "OK" : (const char *) "FAILED");
	      //opt, global_opt,
	      //fabs (opt - global_opt));

    } else // good old statistics

    if (couenne.displayStats ()) { // print statistics

      // CAUTION: assuming first cut generator is our CouenneCutGenerator

      if (cg && !cp) printf ("Warning, could not get pointer to problem\n");
      else
	printf ("Stats: %-15s %4d [var] %4d [int] %4d [con] %4d [aux] "
		"%6d [root] %8d [tot] %6g [sep] %8g [time] %8g [bb] "
		"%20e [lower] %20e [upper] %7d [nodes]\n",// %s %s\n",
		cp ? cp -> problemName ().c_str () : "unknown",
		(cp) ? cp -> nOrigVars     () : -1, 
		(cp) ? cp -> nOrigIntVars  () : -1, 
		(cp) ? cp -> nOrigCons     () : -1,
		(cp) ? (cp -> nVars     () - 
			cp -> nOrigVars ()): -1,
		nr, nt, st, 
		CoinCpuTime () - time_start,
		cg ? (CoinCpuTime () - cg -> rootTime ()) : CoinCpuTime (),
		bb.model (). getBestPossibleObjValue (),
		bb.model (). getObjValue (),
		//bb.bestBound (),
		//bb.bestObj (),
		bb.numNodes ());
		//bb.iterationCount ());
		//status.c_str (), message.c_str ());
    }

//    nlp_and_solver -> writeAmplSolFile (message, bb.bestSolution (), NULL);
  }
  catch(TNLPSolver::UnsolvedError *E) {
     E->writeDiffFiles();
     E->printError(std::cerr);
    //There has been a failure to solve a problem with Ipopt.
    //And we will output file with information on what has been changed in the problem to make it fail.
    //Now depending on what algorithm has been called (B-BB or other) the failed problem may be at different place.
    //    const OsiSolverInterface &si1 = (algo > 0) ? nlpSolver : *model.solver();
  }
  catch(OsiTMINLPInterface::SimpleError &E) {
    std::cerr<<E.className()<<"::"<<E.methodName()
	     <<std::endl
	     <<E.message()<<std::endl;
  }
  catch(CoinError &E) {
    std::cerr<<E.className()<<"::"<<E.methodName()
	     <<std::endl
	     <<E.message()<<std::endl;
  }
  catch (Ipopt::OPTION_INVALID &E)
  {
   std::cerr<<"Ipopt exception : "<<E.Message()<<std::endl;
  }
  catch (int generic_error) {
    if (generic_error == infeasible)
      printf ("problem infeasible\n");
  }

//  catch(...) {
//    std::cerr<<pbName<<" unrecognized excpetion"<<std::endl;
//    std::cerr<<pbName<<"\t Finished \t exception"<<std::endl;
//    throw;
//  }

  delete [] pbName;
  return 0;
}
