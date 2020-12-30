/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#include <limits>
#include <map>
#include <stdexcept>

#include "bat/ads/internal/ml_tools/data_point/data_point.h"

namespace ads {
namespace ml_tools {
namespace data_point {

DataPoint::DataPoint(
    const DataPoint &other_point) = default;

DataPoint::~DataPoint() = default;    

DataPoint::DataPoint(
    const std::string& data) {
  type = DataType::TextData;
  data_text = data;
  n_dims = 0;
}

DataPoint::DataPoint(
    const std::vector<double>& data) {
  type = DataType::VectorData;
  data_vector = data;
  n_dims = data.size();
}

DataPoint::DataPoint(
    const std::map<unsigned, double>& data,
    int dims) {
  type = DataType::SparseVector;
  n_dims = dims;
  data_sparse = data;
}

double operator * (
    const DataPoint a,
    const DataPoint b) {
  if (!a.n_dims || !b.n_dims) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  if (a.n_dims != b.n_dims) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  double rtn = 0.0;
  if (a.type == DataType::VectorData && b.type == DataType::VectorData) {
    for (unsigned i = 0; i < a.data_vector.size(); i++) {
      rtn += a.data_vector[i] * b.data_vector[i];
    }
  } else if (a.type == DataType::VectorData
      && b.type == DataType::SparseVector) {
    for (auto kv : b.data_sparse) {
      rtn += a.data_vector[kv.first] * kv.second;
    }
  } else if (a.type == DataType::SparseVector && b.type == DataType::VectorData) {
    for (auto kv : a.data_sparse) {
      rtn += b.data_vector[kv.first] * kv.second;
    }
  } else {
    for (auto kv : a.data_sparse) {
      if ( b.data_sparse.count(kv.first)>0 ){
        auto tmp = b.data_sparse.at(kv.first);
        rtn+=kv.second*tmp; 
      }
    }
  }
  return rtn;
}

}  // namespace data_point
}  // namespace ml_tools
}  // namespace ads
