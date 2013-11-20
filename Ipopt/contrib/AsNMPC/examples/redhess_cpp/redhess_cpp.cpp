// Copyright 2009 Hans Pirnay
// All Rights Reserved.
// This code is published under the Common Public License.
//
// Date   : 2010-10-04

#include "MyNmpcTNLP.hpp"

#include "IpIpoptApplication.hpp"
#include "AsNMPCApplication.hpp"
#include "IpPDSearchDirCalc.hpp"
#include "IpIpoptAlg.hpp"
#include "AsAsNMPCRegOp.hpp"

int main(int argv, char**argc)
{

  using namespace Ipopt;

  SmartPtr<IpoptApplication> app_ipopt = new IpoptApplication();

  SmartPtr<NmpcApplication> app_nmpc = new NmpcApplication(app_ipopt->Jnlst(),
							   app_ipopt->Options(),
							   app_ipopt->RegOptions());

  // Register AsNMPC options
  RegisterOptions_AsNMPC(app_ipopt->RegOptions());
  app_ipopt->Options()->SetRegisteredOptions(app_ipopt->RegOptions());

  // Call Initialize the first time to create a journalist, but ignore
  // any options file
  ApplicationReturnStatus retval;
  retval = app_ipopt->Initialize("");
  if (retval != Solve_Succeeded) {
    //printf("ampl_ipopt.cpp: Error in first Initialize!!!!\n");
    exit(-100);
  }

    
  app_ipopt->Initialize();

  // create AmplSensTNLP from argc. This is an nlp because we are using our own TNLP Adapter
  SmartPtr<TNLP> nmpc_tnlp = new MyNmpcTNLP();

  app_ipopt->Options()->SetStringValueIfUnset("compute_red_hessian", "yes");

  app_nmpc->Initialize();

  retval = app_ipopt->OptimizeTNLP(nmpc_tnlp);

  /* give pointers to Ipopt algorithm objects to NMPC Application */
  app_nmpc->SetIpoptAlgorithmObjects(app_ipopt, retval);

  app_nmpc->Run();

  return 0;

}
