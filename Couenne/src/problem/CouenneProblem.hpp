/* $Id$
 *
 * Name:    CouenneProblem.hpp
 * Author:  Pietro Belotti, Lehigh University
 *          Andreas Waechter, IBM
 * Purpose: define the class CouenneProblem
 *
 * (C) Carnegie-Mellon University, 2006-10.
 * This file is licensed under the Common Public License (CPL)
 */

#ifndef COUENNE_PROBLEM_HPP
#define COUENNE_PROBLEM_HPP

#include <vector>
#include <map>

#include "CouenneTypes.hpp"
#include "CouenneExpression.hpp"
//#include "CouenneOrbitObj.hpp"

#ifdef COIN_HAS_NTY
#include "Nauty.h"
#endif

#include "CouenneJournalist.hpp"
#include "CouenneDomain.hpp"

/*
extern "C" {
#include <nauty.h>
}
*/
using namespace Ipopt;

class CglTreeInfo;

namespace Ipopt {
  class OptionsList;
  class Journalist;
}

class CbcModel;

namespace Bonmin {
  class RegisteredOptions;
  class BabInfo;
  class OsiTMINLPInterface;
  class BabSetupBase;
}

struct ASL;
struct expr;

class OsiObject;
class CoinWarmStart;
//class Nauty;

#ifdef COIN_HAS_NTY
  class Node{
    int index;
    double coeff;
    double lb;
    double ub;
    int color;
    int code;
  public:
    void node(int, double, double, double, int);
    void color_vertex(int);
    int get_index () {return index;
    };
    double get_coeff () {return coeff;
    };
    double get_lb () {return lb;
    };
    double get_ub () {return ub ;
    };
    int get_color () {return color;
    };
    int get_code () {return code;
    };
    void bounds( double a, double b){ lb = a; ub = b;
    };
  };

  struct myclass0 {
    bool operator() (Node a, Node b) {
      bool is_less = 0;
      
      if(a.get_code() < b.get_code() )
	is_less = 1;
      else {
	if(a.get_code() == b.get_code() )
	  if(a.get_coeff() < b.get_coeff() )
	    is_less = 1;
	  else{
	    if(a.get_coeff() ==  b.get_coeff() )
	      if(a.get_lb() < b.get_lb())
		is_less = 1;
	      else{
		if(a.get_lb() == b.get_lb())
		  if(a.get_ub() < b.get_ub())
		    is_less = 1;
		  else{
		    if(a.get_index() < b.get_index())
		      is_less = 1;
		  }
	      }
	  }
      }
    return is_less;
    }
  } ;
    
      
  struct myclass {
    bool operator() (Node a, Node b) {
      return (a.get_index() < b.get_index() );
    }
  };
#endif


namespace Couenne {

  class exprVar;
  class exprAux;
  class DepGraph;
  class CouenneObject;
  class CouenneCutGenerator;
  class quadElem;
  class LinMap;
  class QuadMap;
  class CouenneConstraint;
  class CouenneObjective;
  class GlobalCutOff;

  struct compExpr;


// default tolerance for checking feasibility (and integrality) of NLP solutions
const CouNumber feas_tolerance_default = 1e-5;

/** Class for MINLP problems with symbolic information
 *
 *  It is read from an AMPL .nl file and contains variables, AMPL's
 *  "defined variables" (aka common expressions), objective(s), and
 *  constraints in the form of expression's. Changes throughout the
 *  program occur in standardization.
 */

class CouenneProblem {

  /// structure to record fixed, non-fixed, and continuous variables
  enum fixType {UNFIXED, FIXED, CONTINUOUS};

 public:

  /// Type of multilinear separation
  enum multiSep {MulSepNone, MulSepSimple, MulSepTight};

 protected:

  /// problem name
  std::string problemName_;

  std::vector <exprVar           *> variables_;   ///< Variables (original, auxiliary, and defined)
  std::vector <CouenneObjective  *> objectives_;  ///< Objectives
  std::vector <CouenneConstraint *> constraints_; ///< Constraints

  /// AMPL's common expressions (read from AMPL through structures cexps and cexps1)
  std::vector <expression *> commonexprs_; 

  mutable Domain domain_; ///< current point and bounds;

  /// Expression map for comparison in standardization and to count
  /// occurrences of an auxiliary
  std::set <exprAux *, compExpr> *auxSet_;

  /// Number of elements in the x_, lb_, ub_ arrays
  mutable int curnvars_;

  /// Number of discrete variables
  int nIntVars_;

  /// Best solution known to be loaded from file -- for testing purposes
  mutable CouNumber *optimum_;

  /// Best known objective function
  CouNumber bestObj_;

  /// Indices of variables appearing in products (used for SDP cuts)
  int *quadIndex_;

  /// Variables that have commuted to auxiliary
  bool *commuted_;

  /// numbering of variables. No variable xi with associated pi(i)
  /// greater than pi(j) should be evaluated before variable xj
  int *numbering_;

  /// Number of "defined variables" (aka "common expressions")
  int ndefined_;

  /// Dependence (acyclic) graph: shows dependence of all auxiliary
  /// variables on one another and on original variables. Used to
  /// create a numbering of all variables for evaluation and bound
  /// tightening (reverse order for implied bounds)
  DepGraph *graph_;

  /// Number of original variables
  int nOrigVars_;

  /// Number of original constraints (disregarding those that turned
  /// into auxiliary variable definition)
  int nOrigCons_;

  /// Number of original integer variables
  int nOrigIntVars_;

  /// Pointer to a global cutoff object
  mutable GlobalCutOff* pcutoff_;

  /// flag indicating if this class is creator of global cutoff object
  mutable bool created_pcutoff_;

  bool doFBBT_;  ///< do Feasibility-based bound tightening
  bool doRCBT_;  ///< do reduced cost      bound tightening
  bool doOBBT_;  ///< do Optimality-based  bound tightening
  bool doABT_;   ///< do Aggressive        bound tightening

  int logObbtLev_;   ///< frequency of Optimality-based bound tightening
  int logAbtLev_;    ///< frequency of Aggressive       bound tightening

  /// SmartPointer to the Journalist
  JnlstPtr jnlst_;

  /// window around known optimum (for testing purposes)
  CouNumber opt_window_;

  /// Use quadratic expressions?
  bool useQuadratic_;

  /// feasibility tolerance (to be used in checkNLP)
  CouNumber feas_tolerance_;

  /// inverse dependence structure: for each variable x give set of
  /// auxiliary variables (or better, their indices) whose expression
  /// depends on x
  std::vector <std::set <int> > dependence_;

  /// vector of pointer to CouenneObjects. Used by CouenneVarObjects
  /// when finding all objects related to (having as argument) a
  /// single variable
  std::vector <CouenneObject *> objects_;

  /// each element is true if variable is integer and, if auxiliary,
  /// depends on no integer
  mutable int *integerRank_;

  /// numberInRank_ [i] is the number of integer variables in rank i
  mutable std::vector <int> numberInRank_;

  /// maximum cpu time
  double maxCpuTime_;

  /// options
  Bonmin::BabSetupBase *bonBase_;

#ifdef COIN_HAS_ASL
  /// AMPL structure pointer (temporary --- looking forward to embedding into OS...)
  ASL *asl_;
#endif

  /// some originals may be unused due to their zero multiplicity
  /// (that happens when they are duplicates). This array keeps track
  /// of their indices and is sorted by evaluation order
  int *unusedOriginalsIndices_;

  /// number of unused originals
  int nUnusedOriginals_;

  /// Type of Multilinear separation
  enum multiSep multilinSep_;

  /// Use semiauxiliaries
  bool useSemiaux_;

  /// number of FBBT iterations
  int max_fbbt_iter_;

  /// true if FBBT exited for iteration limits as opposed to inability
  /// to further tighten bounds
  mutable bool fbbtReachedIterLimit_;

 public:

  CouenneProblem  (ASL * = NULL,
		   Bonmin::BabSetupBase *base = NULL,
		   JnlstPtr jnlst = NULL);  ///< Constructor
  CouenneProblem  (const CouenneProblem &); ///< Copy constructor
  ~CouenneProblem ();                       ///< Destructor

  /// initializes parameters like doOBBT
  void initOptions (SmartPtr <Ipopt::OptionsList> options);

  /// Clone method (for use within CouenneCutGenerator::clone)
  CouenneProblem *clone () const
  {return new CouenneProblem (*this);}

  int nObjs     () const {return (int) objectives_.   size ();} ///< Get number of objectives
  int nCons     () const {return (int) constraints_.  size ();} ///< Get number of constraints
  int nOrigCons () const {return nOrigCons_;}                   ///< Get number of original constraints

  inline int nOrigVars    () const {return nOrigVars_;}                ///< Number of orig. variables
  inline int nDefVars     () const {return ndefined_;}                 ///< Number of def'd variables
  inline int nOrigIntVars () const {return nOrigIntVars_;}             ///< Number of original integers
  inline int nIntVars     () const {return nIntVars_;}                 ///< Number of integer variables
  inline int nVars        () const {return (int) variables_. size ();} ///< Total number of variables
  
  void setNDefVars(int ndefined__) { ndefined_ = ndefined__; }

  


  // Symmetry Info

#ifdef COIN_HAS_NTY
  std::vector<int>  Find_Orbit(int);
  mutable std::vector<Node> node_info;
  mutable Nauty *nauty_info;

  myclass0  node_sort; 
  myclass index_sort;

  void sym_setup();
  void Compute_Symmetry() const;
  void Print_Orbits();
  void ChangeBounds (const double * , const double *, int ) const;
  bool compare (  Node a, Node b) const;
  // bool node_sort (  Node  a, Node  b);
  // bool index_sort (  Node  a, Node  b);
#endif
  
  /// get evaluation order index 
  inline int evalOrder (int i) const
  {return numbering_ [i];}

  /// get evaluation order vector (numbering_)
  inline int *evalVector ()
  {return numbering_;}

  // get elements from vectors
  inline CouenneConstraint *Con (int i) const {return constraints_ [i];} ///< i-th constraint
  inline CouenneObjective  *Obj (int i) const {return objectives_  [i];} ///< i-th objective

  /// Return pointer to i-th variable
  inline exprVar *Var   (int i) const 
  {return variables_ [i];}

  /// Return vector of variables (symbolic representation)
  inline std::vector <exprVar *> &Variables () 
  {return variables_;}

  /// Return pointer to set for comparisons
  inline std::set <exprAux *, compExpr> *& AuxSet () 
  {return auxSet_;}

  /// Return pointer to dependence graph
  inline DepGraph *getDepGraph () 
  {return graph_;}

  /// return current point & bounds
  inline Domain *domain () const
  {return &domain_;}
  
  inline std::vector <expression *>& commonExprs() { return commonexprs_; }

  // Get and set current variable and bounds
  inline CouNumber   &X     (int i) const {return domain_.x   (i);} ///< \f$x_i\f$
  inline CouNumber   &Lb    (int i) const {return domain_.lb  (i);} ///< lower bound on \f$x_i\f$
  inline CouNumber   &Ub    (int i) const {return domain_.ub  (i);} ///< upper bound on \f$x_i\f$

  // get and set current variable and bounds
  inline CouNumber  *X     () const {return domain_.x  ();} ///< Return vector of variables
  inline CouNumber  *Lb    () const {return domain_.lb ();} ///< Return vector of lower bounds
  inline CouNumber  *Ub    () const {return domain_.ub ();} ///< Return vector of upper bounds

  // get optimal solution and objective value
  CouNumber  *&bestSol () const {return optimum_;} ///< Best known solution (read from file)
  CouNumber    bestObj () const {return bestObj_;} ///< Objective of best known solution

  /// Get vector of commuted variables
  bool *&Commuted () 
  {return commuted_;}

  /// Add (non linear) objective function
  void addObjective     (expression *, const std::string & = "min");

  // Add (non linear) "=", ">=", "<=", and range constraints
  void addEQConstraint  (expression *, expression * = NULL); ///< Add equality constraint \f$ h(x) = b\f$
  void addGEConstraint  (expression *, expression * = NULL); ///< Add \f$\ge\f$ constraint, \f$h(x)\ge b\f$
  void addLEConstraint  (expression *, expression * = NULL); ///< Add \f$\le\f$ constraint, \f$h(x)\le b\f$
  void addRNGConstraint (expression *, expression * = NULL, 
			               expression * = NULL); ///< Add range constraint, \f$a\le h(x)\le b\f$

  /// Add (non linear) objective function
  void setObjective (int indObj = 0, expression * = NULL, const std::string & = "min");

  /// Add original variable.
  ///
  /// @param isint if true, this variable is integer, otherwise it is
  /// continuous
  expression *addVariable (bool isint = false, Domain *d = NULL);

  /// Add auxiliary variable and associate it with expression given as
  /// argument (used in standardization)
  exprAux *addAuxiliary (expression *);

  /// preprocess problem in order to extract linear relaxations etc.
  void reformulate (CouenneCutGenerator * = NULL);

  /// Break problem's nonlinear constraints in simple expressions to
  /// be convexified later. Return true if problem looks feasible,
  /// false if proven infeasible.
  bool standardize ();

  /// Display current representation of problem: objective, linear and
  /// nonlinear constraints, and auxiliary variables.
  void print (std::ostream & = std::cout);

#ifdef COIN_HAS_ASL
  /// Read problem from .nl file using the Ampl Solver Library (ASL)
  int readnl (const struct ASL *);

  /// Generate a Couenne expression from an ASL expression
  expression *nl2e (struct expr *, const ASL *asl);
#endif

  // bound tightening parameters
  bool doFBBT () const {return doFBBT_;} ///< shall we do Feasibility Based Bound Tightening?
  bool doRCBT () const {return doRCBT_;} ///< shall we do reduced cost      Bound Tightening?
  bool doOBBT () const {return doOBBT_;} ///< shall we do Optimality  Based Bound Tightening?
  bool doABT  () const {return doABT_;}  ///< shall we do Aggressive        Bound Tightening?

  int  logObbtLev () const {return logObbtLev_;} ///< How often shall we do OBBT?
  int  logAbtLev  () const {return logAbtLev_;}  ///< How often shall we do ABT?

  /// Write nonlinear problem to a .mod file (with lots of defined
  /// variables)
  /// 
  /// @param fname Name of the .mod file to be written
  ///
  /// @param aux controls the use of auxiliaries. If true, a problem
  /// is written with auxiliary variables written with their
  /// associated expression, i.e. \f$w_i = h_i(x,y,w)\f$ and bounds
  /// \f$l_i \le w_i \le u_i\f$, while if false these constraints are
  /// written in the form \f$l_i \le h_i (x,y) \le u_i\f$.
  ///
  /// Note: if used before standardization, writes original AMPL formulation
  void writeAMPL (const std::string &fname, bool aux);

  /// Write nonlinear problem to a .gms file
  /// 
  /// @param fname Name of the .gams file to be written.
  void writeGAMS (const std::string &fname);

  /// Initialize auxiliary variables and their bounds from original
  /// variables
  //void initAuxs (const CouNumber *, const CouNumber *, const CouNumber *);
  void initAuxs () const;

  /// Get auxiliary variables from original variables
  void getAuxs (CouNumber *) const;

  /// tighten bounds using propagation, implied bounds and reduced costs
  bool boundTightening (t_chg_bounds *, 
			Bonmin::BabInfo * = NULL) const;

  /// core of the bound tightening procedure
  bool btCore (t_chg_bounds *chg_bds) const;

  /// Optimality Based Bound Tightening
  int obbt (const CouenneCutGenerator *cg,
	    const OsiSolverInterface &csi,
	    OsiCuts &cs,
	    const CglTreeInfo &info,
	    Bonmin::BabInfo * babInfo,
	    t_chg_bounds *chg_bds);

  /// aggressive bound tightening. Fake bounds in order to cut
  /// portions of the solution space by fathoming on
  /// bounds/infeasibility
  bool aggressiveBT (Bonmin::OsiTMINLPInterface *nlp,
		     t_chg_bounds *, 
		     Bonmin::BabInfo * = NULL) const;

  /// procedure to strengthen variable bounds. Return false if problem
  /// turns out to be infeasible with given bounds, true otherwise.
  int redCostBT (const OsiSolverInterface *psi,
		 t_chg_bounds *chg_bds) const;

  /// "Forward" bound tightening, that is, propagate bound of variable
  /// \f$x\f$ in an expression \f$w = f(x)\f$ to the bounds of \f$w\f$.
  int tightenBounds (t_chg_bounds *) const;

  /// "Backward" bound tightening, aka implied bounds. 
  int impliedBounds (t_chg_bounds *) const;

  /// Look for quadratic terms to be used with SDP cuts
  void fillQuadIndices ();

  /// Fill vector with coefficients of objective function
  void fillObjCoeff (double *&);

  /// Replace all occurrences of original variable with new aux given
  /// as argument
  void auxiliarize (exprVar *, exprVar * = NULL);

  /// Set cutoff
  void setCutOff (CouNumber cutoff, const CouNumber *sol = NULL) const;

  /// Get cutoff
  CouNumber getCutOff () const;

  /// Get cutoff solution
  CouNumber *getCutOffSol () const;

  /// Make cutoff known to the problem
  void installCutOff () const;

  /// Provide Journalist
  ConstJnlstPtr Jnlst() const 
  {return ConstPtr (jnlst_);}

  /// Check if solution is MINLP feasible
  bool checkNLP (const double *solution, double &obj, bool recompute = false) const;

  /// generate integer NLP point Y starting from fractional solution
  /// using bound tightening
  int getIntegerCandidate (const double *xFrac, double *xInt, double *lb, double *ub) const;

  /// Read best known solution from file given in argument
  bool readOptimum (std::string *fname = NULL);

  /// Add list of options to be read from file
  static void registerOptions (SmartPtr <Bonmin::RegisteredOptions> roptions);

  /// standardization of linear exprOp's
  exprAux *linStandardize (bool addAux, 
			   CouNumber c0, 
			   LinMap  &lmap,
			   QuadMap &qmap);

  /// split a constraint w - f(x) = c into w's index (it is returned)
  /// and rest = f(x) + c
  int splitAux (CouNumber, expression *, expression *&, bool *, enum expression::auxSign &);

  /// translates pair (indices, coefficients) into vector with pointers to variables
  void indcoe2vector (int *indexL,
		      CouNumber *coeff,
		      std::vector <std::pair <exprVar *, CouNumber> > &lcoeff);

  /// translates triplet (indicesI, indicesJ, coefficients) into vector with pointers to variables
  void indcoe2vector (int *indexI,
		      int *indexJ,
		      CouNumber *coeff,
		      std::vector <quadElem> &qcoeff);

  /// given (expression *) element of sum, returns (coe,ind0,ind1)
  /// depending on element:
  ///
  /// 1) a * x_i ^ 2   ---> (a,i,?)   return COU_EXPRPOW
  /// 2) a * x_i       ---> (a,i,?)   return COU_EXPRVAR
  /// 3) a * x_i * x_j ---> (a,i,j)   return COU_EXPRMUL
  /// 4) a             ---> (a,?,?)   return COU_EXPRCONST
  ///
  /// x_i and/or x_j may come from standardizing other (linear or
  /// quadratic operator) sub-expressions
  void decomposeTerm (expression *term,
		      CouNumber initCoe,
		      CouNumber &c0,
		      LinMap  &lmap,
		      QuadMap &qmap);

  /// return problem name
  const std::string &problemName () const
  {return problemName_;}
  
  void setProblemName(std::string& problemName__)
  { problemName_ = problemName__; }

  /// return inverse dependence structure
  const std::vector <std::set <int> > &Dependence () const
  {return dependence_;}

  /// return object vector
  const std::vector <CouenneObject *> &Objects () const
  {return objects_;}

  /// find SOS constraints in problem
  int findSOS (CbcModel *CbcModelPtr,
	       OsiSolverInterface *solver, 
	       OsiObject ** objects);

  /// set maximum CPU time
  inline void setMaxCpuTime (double time)
  {maxCpuTime_ = time;}

  /// return maximum CPU time
  inline double getMaxCpuTime () const
  {return maxCpuTime_;}

  /// save CouenneBase
  void setBase (Bonmin::BabSetupBase *base);

  /// Some originals may be unused due to their zero multiplicity
  /// (that happens when they are duplicates). This procedure creates
  /// a structure for quickly checking and restoring their value after
  /// solving.
  void createUnusedOriginals ();

  /// Some originals may be unused due to their zero multiplicity (that
  /// happens when they are duplicates). This procedure restores their
  /// value after solving
  void restoreUnusedOriginals (CouNumber * = NULL) const;

  /// return indices of neglected redundant variables
  int *unusedOriginalsIndices () 
  {return unusedOriginalsIndices_;}

  /// number of unused originals
  int nUnusedOriginals ()
  {return nUnusedOriginals_;}

  /// return type of separator for multilinear terms
  enum multiSep MultilinSep () const
  {return multilinSep_;}

  /// return usage of semiauxiliaries
  bool useSemiaux () const
  {return useSemiaux_;}

  /// true if latest call to FBBT terminated due to iteration limit reached
  bool fbbtReachedIterLimit () const
  {return fbbtReachedIterLimit_;}

protected:

  /// single fake tightening. Return
  ///
  /// -1   if infeasible
  ///  0   if no improvement
  /// +1   if improved
  int fake_tighten (char direction,  ///< 0: left, 1: right
		    int index,       ///< index of the variable tested
		    const double *X, ///< point round which tightening is done
		    CouNumber *olb,  ///< cur. lower bound
		    CouNumber *oub,  ///< cur. upper bound
		    t_chg_bounds *chg_bds,
		    t_chg_bounds *f_chg) const;

  /// Optimality Based Bound Tightening -- inner loop
  int obbtInner (OsiSolverInterface *, 
		 OsiCuts &,
		 t_chg_bounds *, 
		 Bonmin::BabInfo *) const;

  int obbt_iter (OsiSolverInterface *csi, 
		 t_chg_bounds *chg_bds, 
		 const CoinWarmStart *warmstart, 
		 Bonmin::BabInfo *babInfo,
		 double *objcoe,
		 int sense, 
		 int index) const;

  int call_iter (OsiSolverInterface *csi, 
		 t_chg_bounds *chg_bds, 
		 const CoinWarmStart *warmstart, 
		 Bonmin::BabInfo *babInfo,
		 double *objcoe,
		 enum nodeType type,
		 int sense) const;

  /// analyze sparsity of potential exprQuad/exprGroup and change
  /// linear/quadratic maps accordingly, if necessary by adding new
  /// auxiliary variables and including them in the linear map
  void analyzeSparsity (CouNumber, 
			LinMap &,
			QuadMap &);

  /// re-organizes multiplication and stores indices (and exponents) of
  /// its variables
  void flattenMul (expression *mul, 
		   CouNumber &coe, 
		   std::map <int, CouNumber> &indices);

  /// clear all spurious variables pointers not referring to the variables_ vector
  void realign ();

  /// fill dependence_ structure
  void fillDependence (Bonmin::BabSetupBase *base, CouenneCutGenerator * = NULL);

  /// fill freeIntegers_ array
  void fillIntegerRank () const;

  /// Test fixing of an integer variable (used in getIntegerCandidate())
  int testIntFix (int index, 
		  CouNumber xFrac, 
		  enum fixType *fixed,
		  CouNumber *xInt,
		  CouNumber *dualL, CouNumber *dualR,
		  CouNumber *olb,   CouNumber *oub,
		  bool patient) const;
};


/// Called from simulateBranch when object is not CouenneObject and
/// therefore needs explicit FBBT
bool BranchingFBBT (CouenneProblem *problem,
		    OsiObject *Object,
		    OsiSolverInterface *solver);

}

#endif
