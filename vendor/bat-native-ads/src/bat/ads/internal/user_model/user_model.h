/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_USER_MODEL_USER_MODEL_H_  // NOLINT
#define BAT_ADS_INTERNAL_USER_MODEL_USER_MODEL_H_  // NOLINT

#include <map>
#include <string>
#include <vector>

#include "bat/ads/internal/ml_tools/pipeline/pipeline.h"
#include "bat/ads/internal/user_model/user_model.h"

namespace ads {
namespace user_model {

class UserModel {
 public:
  static UserModel* CreateInstance();

  ~UserModel() = default;

  UserModel();

  UserModel(
      const UserModel&) = delete;

  UserModel& operator=(
      const UserModel&) = delete;

  bool InitializePageClassifier(
      const std::string& model);

  bool IsInitialized() const;

  const std::map<std::string, double> ClassifyPage(
      const std::string& html);

 private:
  bool is_initialized_;

  ml_tools::pipeline::Pipeline page_classifier_pipeline_;
};

}  // namespace user_model
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_USER_MODEL_USER_MODEL_H_  // NOLINT
