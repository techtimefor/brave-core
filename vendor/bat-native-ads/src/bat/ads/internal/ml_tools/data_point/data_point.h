/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef BAT_ADS_INTERNAL_ML_TOOLS_DATA_POINT_DATA_POINT_H_  // NOLINT
#define BAT_ADS_INTERNAL_ML_TOOLS_DATA_POINT_DATA_POINT_H_  // NOLINT

#include <map>
#include <string>
#include <vector>

namespace ads {
namespace ml_tools {
namespace data_point {

enum class DataType {
  TextData = 0,
  VectorData = 1,
  SparseVector = 2 
};

class DataPoint {
 public:
  DataPoint(const DataPoint &other_point);
  DataPoint(const std::string &data);
  DataPoint(const std::vector<double>& data);
  DataPoint(const std::map<unsigned, double>& data,
      int ndims);
  ~DataPoint();
  friend double operator * (const DataPoint a,
      const DataPoint b);
  DataType GetType();
  int GetDims();

  DataType type;
  int n_dims;
  std::string data_text;
  std::vector<double> data_vector;
  std::map<unsigned, double> data_sparse;
};

}  // namespace data_point
}  // namespace ml_tools
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_ML_TOOLS_DATA_POINT_DATA_POINT_H_  // NOLINT
