// Copyright 2009 Hans Pirnay
// All Rights Reserved.
// This code is published under the Common Public License.
//
// Date    : 2009-04-06
// 
// Purpose : This is the same as IpSensitivityCalculator.hpp
//           It implements the AsBacksolver Interface.

#ifndef __ASSIMPLEBACKSOLVER_HPP__
#define __ASSIMPLEBACKSOLVER_HPP__


#include "IpPDSystemSolver.hpp"
#include "AsAsBacksolver.hpp"

namespace Ipopt {
  class SimpleBacksolver : public AsBacksolver
  {
  public:

    SimpleBacksolver(SmartPtr<PDSystemSolver> pd_solver);

    ~SimpleBacksolver()
    {}

    bool InitializeImpl(const OptionsList& options,
			const std::string& prefix);

    bool Solve(SmartPtr<IteratesVector> delta_lhs, SmartPtr<const IteratesVector> delta_rhs);


  private:

    SimpleBacksolver();

    SmartPtr<PDSystemSolver> pd_solver_;
  };
}

#endif
