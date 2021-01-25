/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_ML_TOOLS_LINEAR_SVM_LINEAR_SVM_H_  // NOLINT
#define BAT_ADS_INTERNAL_ML_TOOLS_LINEAR_SVM_LINEAR_SVM_H_  // NOLINT

#include <map>
#include <string>

#include "bat/ads/internal/ml_tools/data_point/data_point.h"
#include "bat/ads/internal/ml_tools/ml_tools_util.h"

namespace ads {
namespace ml_tools {
namespace linear_svm {

class LinearSVM {
 public:
  LinearSVM();

  LinearSVM(
      const LinearSVM& other);

  explicit LinearSVM(
      const std::string& model);

  LinearSVM(
      const std::map<std::string, data_point::DataPoint>& weights,
      const std::map<std::string, double>& biases);

  ~LinearSVM();

  std::map<std::string, double> Predict(
      const data_point::DataPoint& x);

  std::map<std::string, double> TopPredictions(
      const data_point::DataPoint& x,
      int top_count = -1);

 private:
  std::map<std::string, data_point::DataPoint> weights_;
  std::map<std::string, double> biases_;
};

}  // namespace linear_svm
}  // namespace ml_tools
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_ML_TOOLS_LINEAR_SVM_LINEAR_SVM_H_  // NOLINT
