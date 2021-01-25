/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_AD_TARGETING_RESOURCES_CONTEXTUAL_TEXT_CLASSIFICATION_TEXT_CLASSIFICATION_RESOURCE_H_  // NOLINT
#define BAT_ADS_INTERNAL_AD_TARGETING_RESOURCES_CONTEXTUAL_TEXT_CLASSIFICATION_TEXT_CLASSIFICATION_RESOURCE_H_  // NOLINT

#include <memory>
#include <string>

#include "bat/ads/internal/ad_targeting/resources/resource.h"
#include "bat/ads/internal/user_model/user_model.h"

namespace ads {

using user_model::UserModel;

namespace ad_targeting {
namespace resource {

class TextClassification : public Resource<UserModel*> {
 public:
  TextClassification();

  ~TextClassification() override;

  bool IsInitialized() const override;

  void LoadForLocale(
      const std::string& locale);

  void LoadForId(
      const std::string& locale);

  UserModel* get() const override;

 private:
  std::unique_ptr<UserModel> user_model_;
};

}  // namespace resource
}  // namespace ad_targeting
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_AD_TARGETING_RESOURCES_CONTEXTUAL_TEXT_CLASSIFICATION_TEXT_CLASSIFICATION_RESOURCE_H_  // NOLINT
