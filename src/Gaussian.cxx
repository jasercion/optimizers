/** 
 * @file Gaussian.cxx
 * @brief Implementation for the (1D) Gaussian class
 * @author J. Chiang
 *
 * $Header$
 */

#include <cmath>

#include <iostream>
#include <string>
#include <vector>

#include "optimizers/dArg.h"
#include "optimizers/ParameterNotFound.h"
#include "optimizers/Gaussian.h"

namespace optimizers {

Gaussian::Gaussian(double Prefactor, double Mean, double Sigma)
   : Function("Gaussian", 3, "Prefactor", "dArg", Addend) {
   addParam("Prefactor", Prefactor, true);
   addParam("Mean", Mean, true);
   addParam("Sigma", Sigma, true);
}

double Gaussian::integral(const Arg & xargmin, const Arg & xargmax) const {
   double xmin = dynamic_cast<const dArg &>(xargmin).getValue();
   double xmax = dynamic_cast<const dArg &>(xargmax).getValue();

   std::vector<Parameter> my_params;
   getParams(my_params);
   enum ParamTypes {Prefactor, Mean, Sigma};

   double f0 = my_params[Prefactor].getTrueValue();
   double x0 = my_params[Mean].getTrueValue();
   double sigma = my_params[Sigma].getTrueValue();

   double zmin = (xmin - x0)/sqrt(2.)/sigma;
   double zmax = (xmax - x0)/sqrt(2.)/sigma;

   return f0*(erfcc(zmin) - erfcc(zmax))/2.;
}

double Gaussian::erfcc(double x) const {
/* (C) Copr. 1986-92 Numerical Recipes Software 0@.1Y.. */
   double t, z, ans;

   z=fabs(x);
   t=1.0/(1.0+0.5*z);
   ans = t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+
         t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+
         t*(-0.82215223+t*0.17087277)))))))));
   return x >= 0.0 ? ans : 2.0-ans;
}

double Gaussian::value(const Arg & xarg) const {
   double x = dynamic_cast<const dArg &>(xarg).getValue();

   enum ParamTypes {Prefactor, Mean, Sigma};

   std::vector<Parameter> my_params;
   getParams(my_params);

   return my_params[Prefactor].getTrueValue()/sqrt(2.*M_PI)
      /my_params[Sigma].getTrueValue()
      *exp(-pow( (x - my_params[Mean].getTrueValue())
                 /my_params[Sigma].getTrueValue(), 2 )/2.);
}

double Gaussian::derivByParamImp(const Arg & xarg, 
                                 const std::string & paramName) const {
   double x = dynamic_cast<const dArg &>(xarg).getValue();

   enum ParamTypes {Prefactor, Mean, Sigma};

   std::vector<Parameter> my_params;
   getParams(my_params);

   int iparam = -1;
   for (unsigned int i = 0; i < my_params.size(); i++) {
      if (paramName == my_params[i].getName()) {
         iparam = i;
      }
   }

   if (iparam == -1) {
      throw ParameterNotFound(paramName, getName(), "Gaussian::derivByParam");
   }
   
   switch (iparam) {
   case Prefactor:
      return my_params[Prefactor].getScale()/sqrt(2.*M_PI)
         /my_params[Sigma].getTrueValue()
         *exp(-pow( (x - my_params[Mean].getTrueValue())
                    /my_params[Sigma].getTrueValue(), 2 )/2.);
      break;
   case Mean:
      return value(xarg)*(x - my_params[Mean].getTrueValue())
         /pow(my_params[Sigma].getTrueValue(), 2)
         *my_params[Mean].getScale();
      break;
   case Sigma:
      return value(xarg)/my_params[Sigma].getTrueValue()
         *( pow((x - my_params[Mean].getTrueValue())
                /my_params[Sigma].getTrueValue(), 2) - 1. )
         *my_params[Sigma].getScale();
      break;
   default:
      break;
   }
   return 0;
}

const std::vector<double> & Gaussian::xvalues(size_t nx) const {
   const double nsig(5);

   double mean(m_parameter[1].getValue());
   double sigma(m_parameter[2].getValue());
   
   double xmin = mean - nsig*sigma;

   double dx = 10*sigma/(nx - 1.);
   m_xvalues.clear();
   for (size_t i(0); i < nx; i++) {
      m_xvalues.push_back(i*dx + xmin);
   }
   return m_xvalues;
}

} // namespace optimizers
