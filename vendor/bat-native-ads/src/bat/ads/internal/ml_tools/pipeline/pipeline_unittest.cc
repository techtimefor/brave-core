/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include "bat/ads/internal/ml_tools/data_point/data_point.h"
#include "bat/ads/internal/ml_tools/linear_svm/linear_svm.h"
#include "bat/ads/internal/ml_tools/pipeline/pipeline.h"
#include "bat/ads/internal/ml_tools/transformation/transformation.h"

#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {
namespace ml_tools {

namespace {

const char kPipelineSpam[] = "pipeline_spam.json";

}  // namespace

class BatAdsPipelineTest : public UnitTestBase {
 protected:
  BatAdsPipelineTest() = default;

  ~BatAdsPipelineTest() override = default;
};

TEST_F(BatAdsPipelineTest, BuildSimplePipeline) {
  const double kEps = 1e-6;

  std::vector<transformation::Transformation> transformations;
  transformation::ToLower to_lower;
  transformations.push_back(to_lower);
  auto hashed_ngrams =
      transformation::HashedNGrams(3, std::vector<int>{1, 2, 3});
  transformations.push_back(hashed_ngrams);

  std::map<std::string, data_point::DataPoint> weights = {
      {"class_1",
          data_point::DataPoint(std::vector<double>{1.0, 2.0, 3.0})},
      {"class_2",
          data_point::DataPoint(std::vector<double>{3.0, 2.0, 1.0})},
      {"class_3",
          data_point::DataPoint(std::vector<double>{2.0, 2.0, 2.0})}};

  std::map<std::string, double> biases = {
      {"class_1", 0.0}, {"class_2", 0.0}, {"class_3", 0.0}};

  unsigned expected_len = 3;
  linear_svm::LinearSVM linear_svm(weights, biases);
  pipeline::Pipeline pipeline = pipeline::Pipeline(transformations, linear_svm);

  auto data_point_3 = data_point::DataPoint(
      std::vector<double>{1.0, 0.0, 0.0});
  auto data_point_3_res = pipeline.Apply(data_point_3);
  ASSERT_EQ(expected_len, data_point_3_res.size());

  std::string test_string = "Test String";
  auto res = pipeline.GetTopPredictions(test_string);
  ASSERT_TRUE(res.size() && res.size() <= expected_len);
  for (auto const& pred : res) {
    EXPECT_TRUE(pred.second > -kEps && pred.second < 1.0 + kEps);
  }
}

TEST_F(BatAdsPipelineTest, TestLoadFromJson) {
  const base::Optional<std::string> opt_value =
      ReadFileFromTestPathToString(kPipelineSpam);
  ASSERT_TRUE(opt_value.has_value());

  const std::string json = opt_value.value();
  pipeline::Pipeline pipeline;
  auto load_success = pipeline.FromJson(json);
  ASSERT_TRUE(load_success);

  std::vector<std::string> train_texts = {
      "This is a spam email.", "Another spam trying to sell you viagra",
      "Message from mom with no real subject",
      "Another messase from mom with no real subject", "Yadayada"};
  std::vector<std::string> train_labels = {"spam", "spam", "ham",
      "ham", "junk"};

  for (size_t i = 0; i < train_texts.size(); i++) {
    auto preds = pipeline.Apply(data_point::DataPoint(train_texts[i]));
    for (auto const& pred : preds) {
      auto other_prediction = pred.second;
      EXPECT_TRUE(preds[train_labels[i]] >= other_prediction);
    }
  }
}

}  // namespace ml_tools
}  // namespace ads
