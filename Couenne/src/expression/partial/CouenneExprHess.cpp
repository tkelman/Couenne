/* $Id$
 *
 * Name:    CouenneExprHess.cpp
 * Authors: Pietro Belotti, Lehigh University
 * Purpose: Hessian of the Lagrangian, implementation
 * 
 * This file is licensed under the Eclipse Public License (EPL)
 */

#include "CoinHelperFunctions.hpp"

#include "CouenneExprHess.hpp"
#include "CouenneProblem.hpp"
#include "CouenneProblemElem.hpp"
#include "CouenneExprAux.hpp"

using namespace Couenne;

//#define DEBUG

ExprHess::ExprHess ():

  nnz_  (0),
  iRow_ (NULL),
  jCol_ (NULL),
  numL_ (NULL),
  lamI_ (NULL),
  expr_ (NULL) {}


ExprHess::~ExprHess () {

  if (nnz_) {

    free (iRow_);
    free (jCol_);

    for (int i=0; i<nnz_; i++) {

      for (int j=0; j<numL_ [i]; j++)
	delete expr_ [i] [j];

      free (expr_ [i]);
      free (lamI_ [i]);
    }

    free (numL_);
    free (lamI_);
    free (expr_);
  }
}


/// code for refilling jacobian
void HessElemFill (int i, int level,
		   std::set <int> &list, 
		   expression *expr,
		   int *row, int **lam, expression ***eee,
		   CouenneProblem *);


/// code for refilling arrays through realloc
static void reAlloc (int nCur, int &nMax, int *&r, int *&c, int *&n, int **&l, expression ***&e);


/// code for refilling jacobian
ExprHess::ExprHess (CouenneProblem *p):

  nnz_  (0),
  iRow_ (NULL),
  jCol_ (NULL),
  numL_ (NULL),
  lamI_ (NULL),
  expr_ (NULL) {

#ifdef DEBUG
  printf ("creating Hessian\n");
  // p -> print ();
#endif

  /// for each j in (obj,con)
  ///   create j->deplist()

  std::set <int> *deplist = new std::set <int> [1 + p -> nVars () + p -> nCons ()];

  int nLevels = 0; // depth of this 3D structure (<= #con + #var + 1)

  // fill in dependence list ///////////////////////////////////////////

  // 1) for the objective (nLevels = 0). Note: STOP_AT_AUX is
  //    sufficient to get to the leaves of this sum of squares

  p -> Obj (0) -> Body () -> DepList (deplist [nLevels++], STOP_AT_AUX);

  // 2) for the constraints

  for (int i = 0; i < p -> nCons (); i++) {

    expression *c = p -> Con (i) -> Body ();

    enum nodeType ntype = c -> Type ();

    if (ntype == AUX ||
	ntype == VAR ||
	ntype == CONST) 
      continue;

    c -> DepList (deplist [nLevels++], STOP_AT_AUX);
  }

  // 3) and the auxiliaries

  for (int i = 0; i < p -> nVars (); i++) {

    exprVar *e = p -> Var (i);

    if ((e -> Type         () != AUX) ||
	(e -> Multiplicity () <= 0))
      continue;

    e -> Image () -> DepList (deplist [nLevels++], STOP_AT_AUX);
  }

#ifdef DEBUG
  for (int i=0; i<nLevels; i++) {
    printf ("level %d: ", i);
    for (std::set <int>::iterator j = deplist [i]. begin () ; j != deplist [i].end (); ++j)
      printf ("%d ", *j);
    printf ("\n");
  }
#endif

  /// for each variable i
  ///   create dense row
  ///   for each j in (obj,con)
  ///     for k in j->deplist(): k<=i
  ///       if d^2(j)/(di dj) nonzero
  ///         add term to list [i,j]
  ///   sparsify row

  int 
    nVars   = p -> nVars (),
    curSize = 0; // used to expand array through realloc

  /// for each variable, fill a row of the hessian
  for (int i=0; i < nVars; i++) {

    // create dense row. These will become numL later
    int          *rnz = (int          *) malloc (nVars * sizeof (int));   // row's number of nonzero
    int         **lam = (int         **) malloc (nVars * sizeof (int *)); // row's vectors of indices of nonzeros
    expression ***eee = (expression ***) malloc (nVars * sizeof (expression **));

    CoinFillN (rnz, nVars, 0);

    // No CoinFillN for int** and expression***
    for (int j=nVars; j--;) {
      *lam++ = NULL;
      *eee++ = NULL;
    }

    lam -= nVars;
    eee -= nVars;

    // scan all levels
    int level = 0;

    /// fill term for objective
    HessElemFill (i, 0, deplist [0], p -> Obj (0) -> Body (), rnz, lam, eee, p);

    ++level;

    for (int j = 0; j < p -> nCons (); j++) {

      CouenneConstraint *c = p -> Con (j);
      if (c -> Body () -> Type () == AUX || 
	  c -> Body () -> Type () == VAR) 
	continue;

      HessElemFill (i, level, deplist [level], c -> Body (), rnz, lam, eee, p);

      ++level;
    }

    // and auxiliaries

    for (int j = 0; j < p -> nVars (); j++) {

      exprVar *e = p -> Var (j);

      if ((e -> Type         () != AUX) || 
	  (e -> Multiplicity () <= 0)) 
	continue;

      HessElemFill (i, level, deplist [level], e -> Image (), rnz, lam, eee, p);

      ++level;
    }

    // sparsify rnz, eee, lam

    for (int j=0; j <= i; j++) 
      if (rnz [j]) {

	reAlloc (nnz_+1, curSize, iRow_, jCol_, numL_, lamI_, expr_);

	iRow_ [nnz_] = i;
	jCol_ [nnz_] = j;
	numL_ [nnz_] = rnz [j];
	lamI_ [nnz_] = (int         *) realloc (lam [j], rnz [j] * sizeof (int));
	expr_ [nnz_] = (expression **) realloc (eee [j], rnz [j] * sizeof (expression *));

	++nnz_;
      }

    free (rnz);
    free (lam);
    free (eee);
  }

#ifdef DEBUG
  printf ("hessian: %d nonzeros\n", nnz_);

  for (int i=0; i<nnz_; i++) {

    printf ("[%d,%d]: %d lambdas: ", 
	    iRow_ [i], jCol_ [i], numL_ [i]); 

    fflush (stdout);

    for (int j=0; j<numL_ [i]; j++) {
      printf ("(%d,", lamI_ [i][j]); 
      fflush (stdout);
      expr_ [i][j] -> print (); 
      fflush (stdout);
      printf (") ");
    }
    printf ("\n");
  }
#endif
}


#define reallocStep 100

// fills a row (?) of the Hessian using expression

void HessElemFill (int i, int level,
		   std::set <int> &list, 
		   expression *expr,
		   int *nnz, 
		   int **lam, 
		   expression ***eee,
		   CouenneProblem *p) {

  if (list. find (i) == list.end () ||   // expression does not depend on x_i
      (expr -> Linearity () <= LINEAR))  // no second derivative here
    return;

  // only fills lower triangular part (for symmetry)
  for (int k=0; k <= i; k++) 
    if ((k==i) || (list. find (k) != list.end ())) {

      // objective depends on k and i. Is its second derivative, w.r.t. k and i, nonzero?

      expression 
	*d1  = expr -> differentiate (k), // derivative w.r.t. k
	*sd1 = d1  -> simplify (),        // simplified
	*rd1 = (sd1 ? sd1 : d1),          // actually used
	*d2  = rd1 -> differentiate (i),  // its derivative w.r.t. i
	*sd2 = d2  -> simplify (),        // simplified
	*rd2 = (sd2 ? sd2 : d2);          // actually used

#ifdef DEBUG
      printf (" rd2 [x_%d, x_%d]: d/d x_%d = ", k, i, k); fflush (stdout); 
      rd1 -> print (); printf (" -> d/(d x_%d,d x_%d) = ", k, i); 
      rd2 -> print (); printf ("\n");
#endif

      delete d1;
      if (sd1) delete sd1;
      if (sd2) delete d2;

      if ((rd2 -> Type  () != CONST) ||
	  (rd2 -> Value () != 0.)) {

	// we have a nonzero

	if (!(nnz [k] % reallocStep)) {

	  lam [k] = (int         *) realloc (lam [k], (nnz [k] + reallocStep) * sizeof (int));
	  eee [k] = (expression **) realloc (eee [k], (nnz [k] + reallocStep) * sizeof (expression *));
	}

	rd2 -> realign (p); // fixes variables' domain with the problem.

	lam [k] [nnz [k]]   = level;
	eee [k] [nnz [k]++] = rd2;

      } else delete rd2;
    }
}

static void reAlloc (int nCur, int &nMax, int *&r, int *&c, int *&n, int **&l, expression ***&e) {

  if (nCur >= nMax) {

    nMax += reallocStep;

    r = (int          *) realloc (r, nMax * sizeof (int));
    c = (int          *) realloc (c, nMax * sizeof (int));
    n = (int          *) realloc (n, nMax * sizeof (int));
    l = (int         **) realloc (l, nMax * sizeof (int *));
    e = (expression ***) realloc (e, nMax * sizeof (expression **));
  }
}
