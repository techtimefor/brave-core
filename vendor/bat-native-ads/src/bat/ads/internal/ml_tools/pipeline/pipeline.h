/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_ML_TOOLS_PIPELINE_PIPELINE_H_  // NOLINT
#define BAT_ADS_INTERNAL_ML_TOOLS_PIPELINE_PIPELINE_H_  // NOLINT

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "base/values.h"

#include "bat/ads/internal/ml_tools/data_point/data_point.h"
#include "bat/ads/internal/ml_tools/linear_svm/linear_svm.h"
#include "bat/ads/internal/ml_tools/transformation/transformation.h"

namespace ads {
namespace ml_tools {
namespace pipeline {

class Pipeline {
 public:
  Pipeline();

  Pipeline(
      const Pipeline& pipeline);

  Pipeline(
      const std::vector<transformation::Transformation>& transformations,
      const linear_svm::LinearSVM& classifier);

  ~Pipeline();

  bool FromJson(
      const std::string& json);

  std::map<std::string, double> Apply(
      const data_point::DataPoint& inp);

  std::map<std::string, double> GetTopPredictions(
      const std::string& content);

 private:
  void GetReverseCategories();

  bool ParseClassifier(
      base::Value* classifier);

  bool ParseTransformations(
      base::Value* transformations);

  bool GetVersionFromJSON(
      base::DictionaryValue* dictionary);

  bool GetTimestampFromJSON(
      base::DictionaryValue* dictionary);

  bool GetLocaleFromJSON(
      base::DictionaryValue* dictionary);

  bool GetTransformationsFromJSON(
      base::DictionaryValue* dictionary);

  bool GetClassifierFromJSON(
      base::DictionaryValue* dictionary);

  bool GetTransformationsFromList(
      base::ListValue List);

  uint16_t version_;
  std::string timestamp_;
  std::string locale_;
  std::vector<transformation::Transformation> transformations_;
  linear_svm::LinearSVM classifier_;
};

}  // namespace pipeline
}  // namespace ml_tools
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_ML_TOOLS_PIPELINE_PIPELINE_H_  // NOLINT
