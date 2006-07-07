/**
 * @file NewMinuit.cxx
 * @brief Implementation of the NewMinuit class
 * @author P. Nolan
 * $Header$
 */

#ifdef HAVE_NEW_MINUIT

#include "optimizers/NewMinuit.h"
#include "optimizers/Parameter.h"
#include "Minuit2/MnUserParameters.h"
#include "Minuit2/MnMigrad.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnHesse.h"
#include "optimizers/Exception.h"
#include "optimizers/OutOfBounds.h"

namespace optimizers {

  typedef std::vector<Parameter>::iterator pptr;

  // Constructor
  NewMinuit::NewMinuit(Statistic & stat) : Optimizer(stat), m_maxEval(0), 
                                           m_fitDone(false), m_FCN(stat) { }

  // Call Minuit's MIGRAD to find the minimum of the function
  void NewMinuit::find_min(int verbose, double tol, int TolType) {
     (void)(verbose);
    double tolerance = 1000. * tol;
    if (TolType == RELATIVE) tolerance *= fabs(m_stat->value());
    std::vector<Parameter> params;
    m_stat->getFreeParams(params);
    ROOT::Minuit2::MnUserParameters upar;
    for (pptr p = params.begin(); p != params.end(); p++) {
      upar.Add(p->getName().c_str(), p->getValue(), 1.0, 
	       p->getBounds().first, p->getBounds().second); 
      //  Q:  Is 1.0 the best choice for that parameter?
    }

    m_userState = ROOT::Minuit2::MnUserParameterState(upar);
    ROOT::Minuit2::MnMigrad migrad(m_FCN, m_userState, m_strategy);
    ROOT::Minuit2::FunctionMinimum min = migrad(m_maxEval, tolerance);
//     if (verbose > 0) std::cout << min;
    if (!min.IsValid()) {
      throw Exception("Minuit abnormal termination.  No convergence?");
    }
    m_fitDone = true;
    m_userState = migrad.State();
    m_distance = min.Edm();
    std::vector<double> ParamValues;
    unsigned int i = 0;
    for (pptr p = params.begin(); p != params.end(); p++, i++) {
      ParamValues.push_back(m_userState.Parameters().Value(i));
    }
    m_stat->setFreeParamValues(ParamValues);
  }

  // Call Minuit's HESSE to get a robust estimate of the covariance matrix
  void NewMinuit::hesse(int verbose) {
     (void)(verbose);
    if (!m_fitDone) 
      throw Exception("Minuit: find_min must be executed before hesse");
    ROOT::Minuit2::MnHesse hesse(m_strategy);
    m_userState = hesse(m_FCN, m_userState.Parameters(), m_maxEval);
//     if (verbose > 0) std::cout << m_userState;
    if (!m_userState.HasCovariance())
      throw Exception("Minuit HESSE results invalid");
  }

  // Constructor for the function to be minimized
  myFCN::myFCN(Statistic & stat): m_stat(&stat) {};

  // This is the function that Minuit minimizes
  double 
  myFCN::operator() (const std::vector<double> & params) const {
    try {m_stat->setFreeParamValues(params);}
    catch (OutOfBounds & e) {
      std::cerr << e.what() << std::endl;
      std::cerr << "Value " << e.value() << " is not between"
                << e.minValue() << " and " << e.maxValue() << std::endl;
      throw;
    }
    catch (Exception & e) {
      std::cerr << e.what() << std::endl;
      throw;
    }
    return -m_stat->value();
  }

  // Return values of the function gradient in the form
  // the Minuit wants
  std::vector<double> 
  myFCN::Gradient(const std::vector<double> & params) const {
    try {m_stat->setFreeParamValues(params);}
    catch (OutOfBounds & e) {
      std::cerr << e.what() << std::endl;
      std::cerr<< "Value " << e.value() << " is not between"
	       << e.minValue() << " and " << e.maxValue() << std::endl;
      throw;
    }
    catch (Exception & e) {
      std::cerr << e.what() << std::endl;
      throw;
    }
    std::vector<double> grad;
    m_stat->getFreeDerivs(grad);
    for (unsigned int i = 0; i < grad.size(); i++) {
      grad[i] = -grad[i];
    }
    return grad;
  }

  // Get the uncertainty values from covariance matrix
   const std::vector<double> & NewMinuit::getUncertainty(bool useBase) {
      if (useBase) {
         Optimizer::getUncertainty(useBase);
      } else {
         if (!m_userState.HasCovariance()) {
            hesse(0);
         }
         m_uncertainty.clear();
         for (size_t i = 0; i < m_userState.Params().size(); i++) {
            m_uncertainty.push_back(sqrt(m_userState.Covariance()(i, i)));
         }
      }
      return m_uncertainty;
   }

}
#endif