/* $Id$
 *
 * Name:    CouenneCutGenerator.cpp
 * Author:  Pietro Belotti
 * Purpose: define a class of convexification procedures 
 *
 * (C) Carnegie-Mellon University, 2006-09.
 * This file is licensed under the Common Public License (CPL)
 */

#include "CglCutGenerator.hpp"

#include "CouenneProblem.hpp"
#include "CouenneCutGenerator.hpp"
#include "CouenneChooseStrong.hpp"
#include "CouenneChooseVariable.hpp"

using namespace Couenne;

/// constructor
CouenneCutGenerator::CouenneCutGenerator (Bonmin::OsiTMINLPInterface *nlp,
					  Bonmin::BabSetupBase *base,
					  CouenneProblem *problem,
					  struct ASL *asl):

  CglCutGenerator (),
  
  firstcall_      (true),
  problem_        (problem),
  nrootcuts_      (0),
  ntotalcuts_     (0),
  septime_        (0),
  objValue_       (- DBL_MAX),
  nlp_            (nlp),
  BabPtr_         (NULL),
  infeasNode_     (false),
  jnlst_          (base ? base -> journalist () : NULL),
  rootTime_       (-1.) {

  if (base) {

    base -> options () -> GetIntegerValue ("convexification_points", nSamples_, "couenne.");

    std::string s;

    base -> options () -> GetStringValue ("convexification_type", s, "couenne.");
    if      (s == "current-point-only") convtype_ = CURRENT_ONLY;
    else if (s == "uniform-grid")       convtype_ = UNIFORM_GRID;
    else                                convtype_ = AROUND_CURPOINT;

    base -> options () -> GetStringValue ("violated_cuts_only", s, "couenne."); 
    addviolated_ = (s == "yes");

    base -> options () -> GetStringValue ("check_lp", s, "couenne."); 
    check_lp_ = (s == "yes");

    base -> options () -> GetStringValue ("enable_lp_implied_bounds", s, "couenne.");
    enable_lp_implied_bounds_ = (s == "yes");

  } else {

    nSamples_                 = 4;
    convtype_                 = CURRENT_ONLY;
    addviolated_              = true;
    check_lp_                 = false;
    enable_lp_implied_bounds_ = false;
  }

  //if (asl) // deal with problems not originating from AMPL
  //problem_ = new CouenneProblem (asl, base, jnlst_);
}


/// destructor
CouenneCutGenerator::~CouenneCutGenerator ()
{}//if (problem_) delete problem_;}


/// copy constructor
CouenneCutGenerator::CouenneCutGenerator (const CouenneCutGenerator &src):

  CglCutGenerator (src),

  firstcall_   (src. firstcall_),
  addviolated_ (src. addviolated_), 
  convtype_    (src. convtype_), 
  nSamples_    (src. nSamples_),
  problem_     (src. problem_),// -> clone ()),
  nrootcuts_   (src. nrootcuts_),
  ntotalcuts_  (src. ntotalcuts_),
  septime_     (src. septime_),
  objValue_    (src. objValue_),
  nlp_         (src. nlp_),
  BabPtr_      (src. BabPtr_),
  infeasNode_  (src. infeasNode_),
  jnlst_       (src. jnlst_),
  rootTime_    (src. rootTime_),
  check_lp_    (src. check_lp_),
  enable_lp_implied_bounds_ (src.enable_lp_implied_bounds_)
{}


#define MAX_SLOPE 1e3

/// add half-space through two points (x1,y1) and (x2,y2)
int CouenneCutGenerator::addSegment (OsiCuts &cs, int wi, int xi, 
				     CouNumber x1, CouNumber y1, 
				     CouNumber x2, CouNumber y2, int sign) const { 

  if (fabs (x2-x1) < COUENNE_EPS) {
    if (fabs (y2-y1) > MAX_SLOPE * COUENNE_EPS)
      jnlst_->Printf(J_WARNING, J_CONVEXIFYING,
		     "warning, discontinuity of %e over an interval of %e\n", y2-y1, x2-x1);
    else return createCut (cs, y2, (int) 0, wi, 1.);
  }

  CouNumber dx = x2-x1, dy = y2-y1;

  //  return createCut (cs, y1 + oppslope * x1, sign, wi, 1., xi, oppslope);
  return createCut (cs, y1*dx - dy*x1, (dx>0) ? sign : -sign, wi, dx, xi, -dy);
}


/// add tangent at (x,w) with given slope
int CouenneCutGenerator::addTangent (OsiCuts &cs, int wi, int xi, 
				     CouNumber x, CouNumber w, 
				     CouNumber slope, int sign) const
  {return createCut (cs, w - slope * x, sign, wi, 1., xi, - slope);}


/// total number of variables (original + auxiliary) of the problem
int CouenneCutGenerator::getnvars () const
  {return problem_ -> nVars ();} 


/// Add list of options to be read from file
void CouenneCutGenerator::registerOptions (Ipopt::SmartPtr <Bonmin::RegisteredOptions> roptions) {

  roptions -> SetRegisteringCategory ("Couenne options", Bonmin::RegisteredOptions::CouenneCategory);

  roptions -> AddLowerBoundedIntegerOption
    ("convexification_cuts",
     "Specify the frequency (in terms of nodes) at which couenne ecp cuts are generated.",
     -99,1,
     "A frequency of 0 amounts to never solve the NLP relaxation.");

  roptions -> AddStringOption2
    ("check_lp",
     "Check all LPs through an independent call to OsiClpSolverInterface::initialSolve()",
     "no",
     "no","",
     "yes","");

  roptions -> AddStringOption2
    ("local_optimization_heuristic",
     "Search for local solutions of MINLP's",
     "yes",
     "no","",
     "yes","",
     "If enabled, a heuristic based on Ipopt is used to find feasible solutions for the problem. "
     "It is highly recommended that this option is left enabled, as it would be difficult to find feasible solutions otherwise.");

  roptions -> AddStringOption2
    ("local_branching_heuristic",
     "Apply local branching heuristic",
     "no",
     "no","",
     "yes","",
     "A local-branching heuristic based is used to find feasible solutions.");

  roptions -> AddStringOption2
    ("feas_pump_heuristic",
     "Apply the nonconvex Feasibility Pump",
     "no",
     "no","",
     "yes","",
     "An implementation of the Feasibility Pump for nonconvex MINLPs");

  roptions -> AddLowerBoundedIntegerOption
    ("log_num_local_optimization_per_level",
     "Specify the logarithm of the number of local optimizations to perform" 
     " on average for each level of given depth of the tree.",
     -1,
     2, "Solve as many nlp's at the nodes for each level of the tree. "
     "Nodes are randomly selected. If for a "
     "given level there are less nodes than this number nlp are solved for every nodes. "
     "For example if parameter is 8, nlp's are solved for all node until level 8, " 
     "then for half the node at level 9, 1/4 at level 10.... "
     "Value -1 specify to perform at all nodes.");

  roptions -> AddStringOption3
    ("convexification_type",
     "Determines in which point the linear over/under-estimator are generated",
     "current-point-only",
     "current-point-only","Only at current optimum of relaxation",
     "uniform-grid","Points chosen in a uniform grid between the bounds of the problem",
     "around-current-point","At points around current optimum of relaxation",
     "For the lower envelopes of convex functions, this is the number of points where a supporting hyperplane is generated. "
     "This only holds for the initial linearization, as all other linearizations only add at most one cut per expression."
    );
    
  roptions -> AddLowerBoundedIntegerOption
    ("convexification_points",
     "Specify the number of points at which to convexify when convexification type "
     "is uniform-grid or around-current-point.",
     0,4,
     "");

  roptions -> AddStringOption2 
    ("violated_cuts_only",
     "Yes if only violated convexification cuts should be added",
     "yes",
     "no","",
     "yes","");

  roptions -> AddStringOption2 
    ("enable_lp_implied_bounds",
     "Enable OsiSolverInterface::tightenBounds () -- warning: it has caused "
     "some trouble to Couenne",
     "no",
     "no","",
     "yes","");

  CouenneProblem        :: registerOptions (roptions);
  CouenneChooseStrong   :: registerOptions (roptions);
  CouenneChooseVariable :: registerOptions (roptions);
}
