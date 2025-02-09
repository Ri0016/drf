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

#ifndef DRF_DEFAULTDATA_H_
#define DRF_DEFAULTDATA_H_

#include <vector>
#include <iostream>

#include "Data.h"
#include "globals.h"

namespace drf {

class DefaultData final: public Data {
public:
  DefaultData();

  DefaultData(const std::vector<double>& data,
      size_t num_rows,
      size_t num_cols);

  double get(size_t row, size_t col) const;

  void reserve_memory();
  void set(size_t col, size_t row, double value, bool& error);

private:
  std::vector<double> data;

  DISALLOW_COPY_AND_ASSIGN(DefaultData);
};

} // namespace drf

#endif /* DRF_DEFAULTDATA_H_ */
