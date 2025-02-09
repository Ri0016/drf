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

#ifndef DRF_SPARSEDATA_H
#define DRF_SPARSEDATA_H


#include "Data.h"
#include "Eigen/Sparse"

namespace drf {

class SparseData final: public Data {
public:
  SparseData();

  SparseData(Eigen::SparseMatrix<double>& data,
             size_t num_rows,
             size_t num_cols);

  ~SparseData() = default;

  double get(size_t row, size_t col) const;

  void reserve_memory();
  void set(size_t col, size_t row, double value, bool& error);

private:
  Eigen::SparseMatrix<double> data;

  DISALLOW_COPY_AND_ASSIGN(SparseData);
};

} // namespace drf

#endif //DRF_SPARSEDATA_H
