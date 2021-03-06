/** 
 * @file ProductFunction.cxx
 * @brief ProductFunction class implementation
 * @author J. Chiang
 * 
 * $Header$
 */

#include <vector>
#include <string>
#include <cmath>
#include <cassert>
#include "optimizers/ProductFunction.h"

namespace optimizers {

ProductFunction::ProductFunction(Function & a, Function & b)
   : CompositeFunction(a, b) {
   assert( (a.funcType() == Addend && b.funcType() == Factor) || 
           (a.funcType() == Factor && b.funcType() == Addend) || 
           (a.funcType() == Factor && b.funcType() == Factor) );
   syncParams();
}

void ProductFunction::fetchDerivs(const Arg & x, std::vector<double> &derivs, 
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
      derivs.push_back(my_derivs[i]*m_b->operator()(x));
   }

   if (getFree) {
      m_b->getFreeDerivs(x, my_derivs);
   } else {
      m_b->getDerivs(x, my_derivs);
   }
   for (unsigned int i = 0; i < my_derivs.size(); i++) {
      derivs.push_back(my_derivs[i]*m_a->operator()(x));
   }
}

} // namespace optimizers
