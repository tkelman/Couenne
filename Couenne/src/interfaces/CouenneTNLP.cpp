/* $Id$
 *
 * Name:    CouenneTNLP.cpp
 * Authors: Pietro Belotti, Lehigh University
 * Purpose: Implementation of an NLP interface with gradient/Jacobian/etc
 * 
 * This file is licensed under the Eclipse Public License (EPL)
 */

#include "IpTNLP.hpp"
#include "IpIpoptApplication.hpp"

#include "CouenneProblem.hpp"
#include "CouenneProblemElem.hpp"
#include "CouenneExprVar.hpp"
#include "CouenneExprJac.hpp"
#include "CouenneExprHess.hpp"
#include "CouenneTNLP.hpp"

#include <stdio.h>

#include "CoinHelperFunctions.hpp"
#include "CoinFinite.hpp"

using namespace Couenne;

/// Empty constructor
CouenneTNLP::CouenneTNLP ():
  problem_ (NULL) {}


/// Constructor 
CouenneTNLP::CouenneTNLP (CouenneProblem *p):

  problem_ (p),
  sol0_    (NULL),
  sol_     (NULL),
  bestZ_   (COIN_DBL_MAX),
  Jac_     (p),
  HLa_     (p) {

  std::set <int> objDep;

  expression *obj = problem_ -> Obj (0) -> Body ();

  obj -> DepList (objDep, STOP_AT_AUX);

  for (std::set <int>::iterator i = objDep.begin (); i != objDep. end (); ++i)
    gradient_ . push_back (std::pair <int, expression *> (*i, obj -> differentiate (*i)));

  // create data structures for nonlinear variables (see
  // get_[number|list]_of_nonlinear_variables () below)

  // constraints

  printf ("constructing TNLP\n");

  for (int i = 0; i < problem_ -> nCons (); i++) {

    expression *e = problem_ -> Con (i) -> Body ();

    if (e -> Type      () == AUX ||
	e -> Type      () == VAR ||
	e -> Linearity () <= LINEAR)
      continue;

    // constraint is nonlinear, get all variables its left-hand side
    // depends on and make them nonlinear

    e -> DepList (nonLinVars_, STOP_AT_AUX);
  }

  // auxiliaries

  for (int i = 0; i < problem_ -> nVars (); i++) {

    exprVar *e = problem_ -> Var (i);

    if ((e -> Type         () != AUX)    ||
	(e -> Linearity    () <= LINEAR) ||
	(e -> Multiplicity () <= 0))
      continue;

    e -> Image () -> DepList (nonLinVars_, STOP_AT_AUX);
  }

  printf ("constructed TNLP\n");
}


// overload this method to return the number of variables and
// constraints, and the number of non-zeros in the jacobian and the
// hessian. The index_style parameter lets you specify C or Fortran
// style indexing for the sparse matrix iRow and jCol parameters.
// C_STYLE is 0-based, and FORTRAN_STYLE is 1-based.
bool CouenneTNLP::get_nlp_info (Index& n, 
				Index& m, 
				Index& nnz_jac_g,
			        Index& nnz_h_lag, 
				IndexStyleEnum& index_style) {
  n = problem_ -> nVars ();
  m = Jac_. nRows ();

  nnz_jac_g = Jac_.nnz ();
  nnz_h_lag = HLa_.nnz ();

  index_style = C_STYLE; // what else? ;-)

  return true;
}


/// set initial solution
void CouenneTNLP::setInitSol (double *sol)
{sol0_ = sol;}


// overload this method to return the information about the bound on
// the variables and constraints. The value that indicates that a
// bound does not exist is specified in the parameters
// nlp_lower_bound_inf and nlp_upper_bound_inf.  By default,
// nlp_lower_bound_inf is -1e19 and nlp_upper_bound_inf is 1e19. (see
// TNLPAdapter)
bool CouenneTNLP::get_bounds_info (Index n, Number* x_l, Number* x_u,
				   Index m, Number* g_l, Number* g_u) {
  // constraints

  for (int i = 0; i < problem_ -> nCons (); i++) {

    CouenneConstraint *c = problem_ -> Con (i);

    if (c -> Body () -> Type () == AUX ||
	c -> Body () -> Type () == VAR) 
      continue;

    *g_l++ = (*c -> Lb ()) ();
    *g_u++ = (*c -> Ub ()) ();
  }

  // auxiliaries

  for (int i = 0; i < problem_ -> nVars (); i++) {

    exprVar *e = problem_ -> Var (i);

    *x_l++ = e -> lb ();
    *x_u++ = e -> ub ();

    if ((e -> Type () != AUX) ||
	(e -> Multiplicity () <= 0))
      continue;

    *g_l++ = e -> lb ();
    *g_u++ = e -> ub ();
  }

  return true;
}


// overload this method to return the variables linearity
// (TNLP::LINEAR or TNLP::NON_LINEAR). The var_types array should be
// allocated with length at least n. (default implementation just
// return false and does not fill the array).
bool CouenneTNLP::get_variables_linearity (Index n, Ipopt::TNLP::LinearityType* var_types) {

  CoinFillN (var_types, n, Ipopt::TNLP::LINEAR);

  for (std::set <int>:: iterator i = nonLinVars_. begin (); i != nonLinVars_. end (); ++i)
    var_types [*i] = Ipopt::TNLP::NON_LINEAR;

  return true;
}

// overload this method to return the constraint linearity.  array
// should be alocated with length at least n. (default implementation
// just return false and does not fill the array).
bool CouenneTNLP::get_constraints_linearity (Index m, Ipopt::TNLP::LinearityType* const_types) {

  // constraints

  for (int i = 0; i < problem_ -> nCons (); i++) {

    expression *b = problem_ -> Con (i) -> Body ();

    if (b -> Type () == AUX ||
	b -> Type () == VAR) 
      continue;

    *const_types++ = 
      (b -> Linearity () > LINEAR) ? 
      Ipopt::TNLP::NON_LINEAR : 
      Ipopt::TNLP::LINEAR;
 }

  // auxiliaries

  for (int i = 0; i < problem_ -> nVars (); i++) {

    exprVar *e = problem_ -> Var (i);

    if ((e -> Type () != AUX) ||
	(e -> Multiplicity () <= 0))
      continue;

    *const_types++ = 
      (e -> Image () -> Linearity () > LINEAR) ? 
      Ipopt::TNLP::NON_LINEAR : 
      Ipopt::TNLP::LINEAR;
  }

  return true;
}


// overload this method to return the starting point. The bool
// variables indicate whether the algorithm wants you to initialize x,
// z_L/z_u, and lambda, respectively.  If, for some reason, the
// algorithm wants you to initialize these and you cannot, return
// false, which will cause Ipopt to stop.  You will have to run Ipopt
// with different options then.
bool CouenneTNLP::get_starting_point (Index n, 
				      bool init_x, Number* x,
				      bool init_z, Number* z_L, Number* z_U,
				      Index m, 
				      bool init_lambda, Number* lambda) {
  if (init_x)
    CoinCopyN (sol0_, n, x);

  assert (!init_z);      // can't initialize bound multipliers
  assert (!init_lambda); // can't initialize Lagrangian multipliers

  return true;
}


// overload this method to return the value of the objective function
bool CouenneTNLP::eval_f (Index n, const Number* x, bool new_x,
			  Number& obj_value) {
  if (new_x)
    CoinCopyN (x, n, problem_ -> X ()); // can't push domain as we
					// don't know when to pop

  obj_value = (*(problem_ -> Obj (0) -> Body ())) ();
  return true;
}


// overload this method to return the vector of the gradient of
// the objective w.r.t. x
bool CouenneTNLP::eval_grad_f (Index n, const Number* x, bool new_x,
			       Number* grad_f) {

  if (new_x)
    CoinCopyN (x, n, problem_ -> X ()); // can't push domain as we
					// don't know when to pop
  
  CoinFillN (grad_f, n, 0.);

  for (std::vector <std::pair <int, expression *> >::iterator i = gradient_. begin (); i != gradient_. end (); ++i)
    grad_f [i -> first] = (*(i -> second)) ();

  return true;
}


// overload this method to return the vector of constraint values
bool CouenneTNLP::eval_g (Index n, const Number* x, bool new_x,
			  Index m, Number* g) {

  if (new_x)
    CoinCopyN (x, n, problem_ -> X ()); // can't push domain as we
					// don't know when to pop

  for (int i = 0; i < problem_ -> nCons (); i++) {

    expression *b = problem_ -> Con (i) -> Body ();

    if (b -> Type () == AUX ||
	b -> Type () == VAR) 
      continue;

    *g++ = (*b) (); // this element of g is the evaluation of the constraint
  }

  // auxiliaries

  for (int i = 0; i < problem_ -> nVars (); i++) {

    exprVar *e = problem_ -> Var (i);

    if ((e -> Type () != AUX) ||
	(e -> Multiplicity () <= 0))
      continue;

    *g++ = (*e) ();
  }

  return true;
}


// overload this method to return the jacobian of the constraints. The
// vectors iRow and jCol only need to be set once. The first call is
// used to set the structure only (iRow and jCol will be non-NULL, and
// values will be NULL) For subsequent calls, iRow and jCol will be
// NULL.
bool CouenneTNLP::eval_jac_g (Index n, const Number* x, bool new_x,
			      Index m, Index nele_jac, Index* iRow,
			      Index *jCol, Number* values) {
  if (new_x)
    CoinCopyN (x, n, problem_ -> X ()); // can't push domain as we
					// don't know when to pop

  //problem_ -> domain () -> push (n, x, NULL, NULL);

  printf ("Domain: %x\n", problem_ -> domain ());

  if (values == NULL && 
      iRow   != NULL && 
      jCol   != NULL) {

    // initialization of the Jacobian's structure. This has been
    // already prepared by the constructor, so simply copy it

    CoinCopyN (Jac_.iRow (), nele_jac, iRow);
    CoinCopyN (Jac_.jCol (), nele_jac, jCol);

  } else {

    // fill in Jacobian's values. Evaluate each member using the
    // domain modified above by the new value of x

    printf ("filling in jacobian values:\n");

    register expression **e = Jac_. expr ();

    for (register int i=0; i<nele_jac; i++) {
      printf ("%d: ", i); fflush (stdout);
      if (*e)
	(*e) -> print ();
      fflush (stdout);
      const exprVar *var = dynamic_cast <const exprVar *> ((*e) -> Original ());
      if (var)
	printf (" [addr %x]", var);
      printf ("..\n");
      *values++ = (**(e++)) ();
    }
  }

  //problem_ -> domain () -> pop ();

  return true;
}


// overload this method to return the hessian of the lagrangian. The
// vectors iRow and jCol only need to be set once (during the first
// call). The first call is used to set the structure only (iRow and
// jCol will be non-NULL, and values will be NULL) For subsequent
// calls, iRow and jCol will be NULL.
//
// This matrix is symmetric - specify the lower diagonal only.
//
// A default implementation is provided, in case the user wants to use
// quasi-Newton approximations to estimate the second derivatives and
// doesn't need to implement this method.
bool CouenneTNLP::eval_h (Index n, const Number* x,      bool new_x,      Number obj_factor, 
			  Index m, const Number* lambda, bool new_lambda, 
			  Index nele_hess,
			  Index* iRow, Index* jCol, Number* values) {

  if (new_x)
    CoinCopyN (x, n, problem_ -> X ()); // can't push domain as we
					// don't know when to pop
  if (values == NULL && 
      iRow   != NULL && 
      jCol   != NULL) {

    /// first call, must determine structure iRow/jCol

    CoinCopyN (HLa_.iRow (), nele_hess, iRow);
    CoinCopyN (HLa_.jCol (), nele_hess, jCol);

  } else {

    /// generic call, iRow/jCol are known and we should fill in the
    /// values

    for (int i=0; i<m; i++, values++) {

      *values = 0.;

      int 
	 numL = HLa_. numL () [i],
	*lamI = HLa_. lamI () [i];

      expression **expr = HLa_. expr () [i];

      for (int j=0; j<numL; j++)
	*values += lambda [lamI [j]] * (*(expr [j])) ();
    }
  }

  return true;
}


// This method is called when the algorithm is complete so the TNLP
// can store/write the solution
void CouenneTNLP::finalize_solution (SolverReturn status,
				     Index n, const Number* x, const Number* z_L, const Number* z_U,
				     Index m, const Number* g, const Number* lambda,
				     Number obj_value,
				     const IpoptData* ip_data,
				     IpoptCalculatedQuantities* ip_cq) {

  printf ("Ipopt[FP] solution: %12e\n", obj_value);
  bestZ_ = obj_value;
  CoinCopyN (x, n, sol_);
}


// Intermediate Callback method for the user.  Providing dummy default
// implementation.  For details see IntermediateCallBack in IpNLP.hpp.
bool CouenneTNLP::intermediate_callback (AlgorithmMode mode,
					 Index iter, Number obj_value,
					 Number inf_pr, Number inf_du,
					 Number mu, Number d_norm,
					 Number regularization_size,
					 Number alpha_du, Number alpha_pr,
					 Index ls_trials,
					 const IpoptData* ip_data,
					 IpoptCalculatedQuantities* ip_cq) {

  printf ("Ipopt[FP] %4d %12e %12e %12e\n", iter, obj_value, inf_pr, inf_du);
  return true;
}


// Methods for quasi-Newton approximation.  If the second derivatives
// are approximated by Ipopt, it is better to do this only in the
// space of nonlinear variables.  The following methods are called by
// Ipopt if the quasi-Newton approximation is selected.  If -1 is
// returned as number of nonlinear variables, Ipopt assumes that all
// variables are nonlinear.
Index CouenneTNLP::get_number_of_nonlinear_variables () 
{return nonLinVars_. size ();}


// Otherwise, it calls get_list_of_nonlinear_variables with an array
// into which the indices of the nonlinear variables should be written
// - the array has the lengths num_nonlin_vars, which is identical
// with the return value of get_number_of_nonlinear_variables().  It
// is assumed that the indices are counted starting with 1 in the
// FORTRAN_STYLE, and 0 for the C_STYLE.
bool CouenneTNLP::get_list_of_nonlinear_variables (Index  num_nonlin_vars,
						   Index* pos_nonlin_vars) {

  for (std::set <int>:: iterator i = nonLinVars_. begin (); i != nonLinVars_. end (); ++i)
    *pos_nonlin_vars++ = *i;

  return true;
}
