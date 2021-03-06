/** 
 * @file SumFunction.cxx
 * @brief SumFunction class implementation
 * @author J. Chiang
 *
 * $Header$
 */

#include <vector>
#include <string>
#include <cmath>
#include <cassert>
#include "optimizers/SumFunction.h"

namespace optimizers {

SumFunction::SumFunction(Function & a, Function & b) 
   : CompositeFunction(a, b) {
   assert(a.funcType() == Addend && b.funcType() == Addend);
   syncParams(); 
}

void SumFunction::fetchDerivs(const Arg & x, std::vector<double> & derivs, 
                              bool getFree) const {
   if (!derivs.empty()) {
      derivs.clear();
   }

   std::vector<double> my_derivs;
   if (getFree) {
      m_a->getFreeDerivs(x, my_derivs);
   } else {
      m_a->getDerivs(x, my_derivs);
   }
   for (unsigned int i = 0; i < my_derivs.size(); i++) {
      derivs.push_back(my_derivs[i]);
   }

   if (getFree) {
      m_b->getFreeDerivs(x, my_derivs);
   } else {
      m_b->getDerivs(x, my_derivs);
   }
   for (unsigned int i = 0; i < my_derivs.size(); i++) {
      derivs.push_back(my_derivs[i]);
   }
}

} // namespace optimizers
