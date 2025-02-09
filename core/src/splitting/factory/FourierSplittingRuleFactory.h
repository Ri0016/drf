/*-------------------------------------------------------------------------------
 This file is part of distributional random forest (drf).
 
 drf is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 drf is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with drf. If not, see <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------*/

#ifndef drf_FOURIERSPLITTINGRULEFACTORY_H
#define drf_FOURIERSPLITTINGRULEFACTORY_H


#include "splitting/factory/SplittingRuleFactory.h"

namespace drf {

/**
 * A factory that produces standard regression splitting rules.
 *
 * In addition to performing standard regression splits, this rule applies
 * a penalty to avoid splits too close to the edge of the node's data.
 */
class FourierSplittingRuleFactory final: public SplittingRuleFactory {
public:
  FourierSplittingRuleFactory() = default;
  std::unique_ptr<SplittingRule> create(const Data& data,
                                        const TreeOptions& options) const;
private:
  DISALLOW_COPY_AND_ASSIGN(FourierSplittingRuleFactory);
};

} // namespace drf

#endif //drf_FOURIERSPLITTINGRULEFACTORY_H
