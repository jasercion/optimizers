/**
 * @file NewMinuit.h
 * @brief Interface to the new C++ version of Minuit
 * @author P. Nolan
 * $Header: 
 */

#ifndef optimizers_NEWMINUIT_H
#define optimizers_NEWMINUIT_H

#ifdef HAVE_NEW_MINUIT
#include <vector>
#include "optimizers/Optimizer.h"
#include "optimizers/Statistic.h"
#include "Minuit2/FCNGradientBase.h"
#include "Minuit2/MnStrategy.h"
#include "Minuit2/MnUserParameterState.h"

namespace optimizers {

  /**
   * @class myFCN
   * @brief The function minimized by Minuit
   * @author P. Nolan
   Minuit provides a base class for the function to be minimized.
   This is the implementation using the Optimizer infrastructure.

   Q:  Would it be better to do this as a hidden class within NewMinuit?
  */

  class myFCN : public ROOT::Minuit2::FCNGradientBase {
  public:
    myFCN(Statistic &);
    virtual ~myFCN() {};
    virtual double Up() const {return 0.5;}
    virtual double operator() (const std::vector<double> &) const;
    virtual std::vector<double> Gradient(const std::vector<double> &) const;
  private:
    Statistic * m_stat;
  };

  /**
   * @class NewMinuit
   * @brief Wrapper class for the Minuit optimizer from CERN
   * @author P. Nolan
   This class implements an Optimizer by using Minuit, a well-known
   package from CERN.  It uses only a few of Minuit's features.
   It uses only the Migrad and Hesse algorithms.  All variables are
   treates as bounded.  No user interaction is allowed.  The new
   C++ implementation of Minuit is used, which has no limits on the
   number of free parameters.  The older Fortran version of Minuit is
   well known in the HEP community.  It was developed at CERN over a
   span of about 30 years.
  */

  class NewMinuit : public Optimizer {
  public:
    NewMinuit(Statistic &);
    virtual ~NewMinuit() {};
    void find_min(int verbose=0, double tole = 1e-5, int tolType = ABSOLUTE);
    void setStrategy(unsigned int strat = 2) {
       m_strategy=ROOT::Minuit2::MnStrategy(strat);
    }
    void setMaxEval(unsigned int n) {m_maxEval=n;}
    double getDistance(void) const {return m_distance;};
    void hesse(int verbose = 0);
    virtual const std::vector<double> & getUncertainty(bool useBase = false);
  private:
    unsigned int m_maxEval;
    bool m_fitDone;
    myFCN m_FCN;
    double m_distance;
    ROOT::Minuit2::MnStrategy m_strategy;
    ROOT::Minuit2::MnUserParameterState m_userState;
  };

}
#endif // HAVE_NEW_MINUIT
#endif