/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "bat/ads/internal/ml_tools/linear_svm/linear_svm.h"

namespace ads {
namespace ml_tools {
namespace linear_svm {

LinearSVM::LinearSVM() {}

LinearSVM::LinearSVM(
    const std::map<std::string, data_point::DataPoint>& weights,
    const std::map<std::string, double>& biases) {
  weights_ = weights;
  biases_ = biases;
}

LinearSVM::LinearSVM(
    const LinearSVM &other) {
  weights_ = other.weights_;
  biases_ = other.biases_;
}

LinearSVM::~LinearSVM() = default;

std::map<std::string, double> LinearSVM::Predict(
    const data_point::DataPoint& x) {
  std::map<std::string, double> rtn;
  for (auto kv : weights_) {
    rtn[kv.first] = kv.second * x + biases_[kv.first];
  }
  return rtn;
}

std::map<std::string, double> LinearSVM::TopPredictions(
    const data_point::DataPoint& x,
    int top_count) {
  std::map<std::string, double> pred_map = Predict(x);
  std::map<std::string, double> pred_map_softmax = softmax(pred_map);
  std::vector<std::pair<double, std::string>> pred_order;
  pred_order.reserve(pred_map_softmax.size());
  for (auto pred_it = pred_map_softmax.begin();
      pred_it != pred_map_softmax.end();
      pred_it++) {
    pred_order.push_back(std::make_pair(pred_it->second, pred_it->first));
  }
  std::sort(pred_order.rbegin(), pred_order.rend());
  std::map<std::string, double> top_pred;
  if (top_count > 0) {
    pred_order.resize(top_count);
  }
  for (size_t i = 0; i < pred_order.size(); ++i) {
    top_pred[pred_order[i].second] = pred_order[i].first;
  }
  return top_pred;
}

}  // namespace linear_svm
}  // namespace ml_tools
}  // namespace ads
