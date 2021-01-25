/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <algorithm>
#include <map>

#include "bat/ads/internal/user_model/user_model.h"

namespace ads {
namespace user_model {

UserModel* UserModel::CreateInstance() {
  return new UserModel();
}

UserModel::UserModel() : is_initialized_(false) {}

bool UserModel::InitializePageClassifier(
    const std::string& model) {
  if (is_initialized_) {
    return false;
  }

  page_classifier_pipeline_ = ml_tools::pipeline::Pipeline();
  is_initialized_ = page_classifier_pipeline_.FromJson(model);
  return is_initialized_;
}

bool UserModel::IsInitialized() const {
  return is_initialized_;
}

const std::map<std::string, double> UserModel::ClassifyPage(
    const std::string& content) {
  if (!is_initialized_ || content.empty()) {
    return {};
  }

  const std::map<std::string, double> top_predictions =
      page_classifier_pipeline_.GetTopPredictions(content);

  return top_predictions;
}

}  // namespace user_model
}  // namespace ads
