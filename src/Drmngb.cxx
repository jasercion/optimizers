/** 
 * @file Drmngb.cxx
 * @brief Drmngb class implementation
 * @author P. Nolan
 *
 */

#include "optimizers/Drmngb.h"
#include "optimizers/Parameter.h"
#include "optimizers/Exception.h"
#include "optimizers/dArg.h"
#include <vector>
#include <algorithm>
#include <iostream>

namespace optimizers {

  typedef std::vector<double>::iterator dptr;
  typedef std::vector<Parameter>::iterator pptr;

//   std::vector<double> & Drmngb::getUncertainty(void) {
//     return m_uncertainty;
//   }

  int Drmngb::getRetCode(void) const {
    return m_retCode;
  }

  void Drmngb::setNeedCovariance(bool b) {
    m_NeedCovariance = b;
  }
  
  void Drmngb::find_min(int verbose, double tol, int tolType) {

    /// Unpack model parameters into the arrays needed by Drmngb
    
    std::vector<Parameter> params;
    m_stat->getFreeParams(params);
    const int nparams = params.size();
    
    std::vector<double> paramVals;
    std::vector<double> paramBounds;
    for (pptr p = params.begin(); p != params.end(); p++) {
      paramVals.push_back(p->getValue());
      paramBounds.push_back(p->getBounds().first);
      paramBounds.push_back(p->getBounds().second);
    }
    
    /// Create the variables and arrays used by DRMNGB.
    /// These serve as storage between calls to drmngb_
    /// so they must be declared outside the loop.
    /// Most of them don't need to be initialized.

    double funcVal;
    const int liv = 59 + nparams;
    const int lv = 71 + nparams*(nparams+19)/2;
    std::vector<double> gradient(nparams), scale(nparams, 1.);
    std::vector<int> iv(liv);
    std::vector<double> v(lv);

    // Set default values for internal settings
    const int kind = 2;
    divset_(&kind, &iv[0], &liv, &lv, &v[0]);
    if (verbose == 0) {iv[20] = 0;}
    if (tolType == RELATIVE) {
      v[31] = tol;
    } else if (tolType == ABSOLUTE) {
      v[31] = 3.e-16;
    }

    /// Call the optimizing function in an infinite loop.
    double oldVal = 1.e+30;
    for (;;) {
      drmngb_(&paramBounds[0], &scale[0], &funcVal, &gradient[0], &iv[0], 
	      &liv,&lv, &nparams, &v[0], &paramVals[0]);
      int rcode = iv[0];
      if (rcode == 1) { /// request for a function value
	try {m_stat->setFreeParamValues(paramVals);}
	catch (OutOfBounds e) {
	  std::cerr << "Drmngb::find_min error" << std::endl;
	  std::cerr << e.what() << std::endl;
	  std::cerr << "Value " << e.value() << " is not between "
		    << e.minValue() << " and " << e.maxValue() << std::endl;
	}
	catch (Exception e) {
	  std::cerr << e.what() << std::endl;
	}
	funcVal = -m_stat->value();
      }
      else if (rcode == 2) { /// request for the gradient
	m_stat->setFreeParamValues(paramVals);
	m_stat->getFreeDerivs(gradient);
	for (dptr p = gradient.begin(); p != gradient.end(); p++) {
	  *p = -*p;
	}
	if (tolType == ABSOLUTE && iv[28] == 3 && fabs(funcVal-oldVal) < tol) {
	  // check after a successful line search
	  m_retCode = 6;
	  if (verbose != 0)
	    std::cout << "***** ABSOLUTE FUNCTION CONVERGENCE *****" 
		      << std::endl;
	  break;
	}
	oldVal = funcVal;
      }
      else {  /// Finished.  Exit loop.
	m_retCode = rcode;
	if (rcode > 6) {throw Exception("DRMNGB error", rcode);}
	break;
      }
    }

    /// Get parameter values and put them back into the Function
    m_stat->setFreeParamValues(paramVals);

    if (m_NeedCovariance) {
      /// Get the Cholesky factor of the Hessian.  It's a lower triangular
      /// matrix stored in compact fashion, so we treat it as 1-dimensional.
      const dptr vp = v.begin() + iv[41] - 1;
      std::vector<double> hess(vp, vp + nparams*(nparams+1)/2);
      
      /// Invert the Hessian to produce the covariance matrix.  The result 
      /// is also stored as a compact triangular matrix.
      /// Dpptri and drmngb have opposite definitions of "lower" and
      /// "upper" triangular matrices.  Drmngb says it produces a lower
      /// matrix.  If we tell dpptri that it's upper, all is OK.

      int info;
      const char uplo = 'U';
      dpptri_(&uplo, &nparams, &hess[0], &info, 1);
      if (info < 0) 
	throw Exception("DPPTRI: illegal argument value", -info);
      else if (info > 0) 
	throw Exception("DPPTRI: Zero diagonal element in Cholesky factor",
			info);
      
      /// Extract the diagonal elements of the covariance matrix.
      /// Their square roots are the parameter uncertainties.
      m_uncertainty.clear();
      for (int i = 0; i < nparams; i++) {
	m_uncertainty.push_back(sqrt(hess[i*(i+3)/2]));
      }
    }

  } // End of find_min
} // namespace optimizers
