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

#ifndef drf_RELABELINGSTRATEGY_H
#define drf_RELABELINGSTRATEGY_H


#include "commons/DefaultData.h"
#include "commons/Data.h"
#include <vector>

namespace drf {

/**
 * Produces a relabelled set of outcomes for a set of training samples. These outcomes
 * will then be used in calculating a standard regression (or classification) split.
 */
class RelabelingStrategy {
public:

  virtual ~RelabelingStrategy() = default;

 /**
   * samples: the subset of samples to relabel.
   * data: the training data matrix.
   * responses_by_sample: the output of the method, containing a map from sample ID to relabelled response.
   *
   * Note that for performance reasons (avoiding clearing out the vector after each split) this vector may
   * contain garbage values for indices outside of the given set of sample IDs.
   *
   * returns: a boolean that will be 'true' if splitting should stop early.
   */
  virtual bool relabel(const std::vector<size_t>& samples,
                       const Data& data,
                       std::vector<std::vector<double>>& responses_by_sample) const = 0; // std::vector<double> -> std::vector<std::vector<double>> 
};

} // namespace drf

#endif //drf_RELABELINGSTRATEGY_H
