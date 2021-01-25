/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cmath>
#include <vector>

#include "bat/ads/internal/ml_tools/data_point/data_point.h"
#include "bat/ads/internal/ml_tools/linear_svm/linear_svm.h"

#include "bat/ads/internal/json_helper.h"
#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {
namespace ml_tools {

class BatAdsLinearSVMTest : public UnitTestBase {
 protected:
  BatAdsLinearSVMTest() = default;

  ~BatAdsLinearSVMTest() override = default;
};

TEST_F(BatAdsLinearSVMTest, ThreeClassesPredictionTest) {
  std::map<std::string, data_point::DataPoint> weights = {
      {"class_1",
          data_point::DataPoint(std::vector<double>{1.0, 0.0, 0.0})},
      {"class_2",
          data_point::DataPoint(std::vector<double>{0.0, 1.0, 0.0})},
      {"class_3",
          data_point::DataPoint(std::vector<double>{0.0, 0.0, 1.0})}};

  std::map<std::string, double> biases = {
      {"class_1", 0.0}, {"class_2", 0.0}, {"class_3", 0.0}};

  linear_svm::LinearSVM linear_svm(weights, biases);

  auto class1_data_point =
      data_point::DataPoint(std::vector<double>{1.0, 0.0, 0.0});
  auto res1 = linear_svm.Predict(class1_data_point);
  ASSERT_TRUE(res1["class_1"] > res1["class_2"]);
  ASSERT_TRUE(res1["class_1"] > res1["class_3"]);

  auto class2_data_point =
      data_point::DataPoint(std::vector<double>{0.0, 1.0, 0.0});
  auto res2 = linear_svm.Predict(class2_data_point);
  ASSERT_TRUE(res2["class_2"] > res2["class_1"]);
  ASSERT_TRUE(res2["class_2"] > res2["class_3"]);

  auto class3_data_point =
      data_point::DataPoint(std::vector<double>{0.0, 1.0, 2.0});
  auto res3 = linear_svm.Predict(class3_data_point);
  EXPECT_TRUE(res3["class_3"] > res3["class_1"]
      && res3["class_3"] > res3["class_2"]);
}

TEST_F(BatAdsLinearSVMTest, BiasesPredictionTest) {
  std::map<std::string, data_point::DataPoint> weights = {
      {"class_1",
          data_point::DataPoint(std::vector<double>{1.0, 0.0, 0.0})},
      {"class_2",
          data_point::DataPoint(std::vector<double>{0.0, 1.0, 0.0})},
      {"class_3",
          data_point::DataPoint(std::vector<double>{0.0, 0.0, 1.0})}};

  std::map<std::string, double> biases = {
      {"class_1", 0.5}, {"class_2", 0.25}, {"class_3", 1.0}};

  linear_svm::LinearSVM biased_svm(weights, biases);

  auto avg_point = data_point::DataPoint(std::vector<double>{1.0, 1.0, 1.0});
  auto res = biased_svm.Predict(avg_point);
  EXPECT_TRUE(res["class_3"] > res["class_1"]
      && res["class_3"] > res["class_2"]
      && res["class_1"] > res["class_2"]);
}

TEST_F(BatAdsLinearSVMTest, BinaryClassifierPredictionTest) {
  std::map<std::string, data_point::DataPoint> weights = {
      {"the_only_class",
          data_point::DataPoint(std::vector<double>{0.3, 0.2, 0.25})},
  };

  std::map<std::string, double> biases = {
      {"the_only_class", -0.45},
  };

  linear_svm::LinearSVM linear_svm(weights, biases);

  auto data_point_0 =
      data_point::DataPoint(std::vector<double>{1.07, 1.52, 0.91});
  auto res_0 = linear_svm.Predict(data_point_0);
  ASSERT_EQ(res_0.size(), static_cast<size_t>(1));

  auto data_point_1 =
      data_point::DataPoint(std::vector<double>{1.11, 1.63, 1.21});
  auto res_1 = linear_svm.Predict(data_point_1);
  ASSERT_EQ(res_1.size(), static_cast<size_t>(1));

  EXPECT_TRUE(res_0["the_only_class"] < 0.5
      && res_1["the_only_class"] > 0.5);
}

TEST_F(BatAdsLinearSVMTest, TopPredictionsTest) {
  std::map<std::string, data_point::DataPoint> weights = {
      {"class_1",
          data_point::DataPoint(std::vector<double>{1.0, 0.5, 0.8})},
      {"class_2",
          data_point::DataPoint(std::vector<double>{0.3, 1.0, 0.7})},
      {"class_3",
          data_point::DataPoint(std::vector<double>{0.6, 0.9, 1.0})},
      {"class_4",
          data_point::DataPoint(std::vector<double>{0.7, 1.0, 0.8})},
      {"class_5",
          data_point::DataPoint(std::vector<double>{1.0, 0.2, 1.0})}};

  std::map<std::string, double> biases = {{"class_1", 0.21},
                                          {"class_2", 0.22},
                                          {"class_3", 0.23},
                                          {"class_4", 0.22},
                                          {"class_5", 0.21}};

  linear_svm::LinearSVM biased_svm(weights, biases);

  auto point_1 =
      data_point::DataPoint(std::vector<double>{1.0, 0.99, 0.98, 0.97, 0.96});
  auto res_1 = biased_svm.TopPredictions(point_1);
  ASSERT_EQ(res_1.size(), static_cast<size_t>(5));

  auto point_2 =
      data_point::DataPoint(std::vector<double>{0.83, 0.79, 0.91, 0.87, 0.82});
  auto res_2 = biased_svm.TopPredictions(point_2, 2);
  ASSERT_EQ(res_2.size(), static_cast<size_t>(2));

  auto point_3 =
      data_point::DataPoint(std::vector<double>{0.92, 0.95, 0.85, 0.91, 0.73});
  auto res_3 = biased_svm.TopPredictions(point_3, 1);
  EXPECT_EQ(res_3.size(), static_cast<size_t>(1));
}

TEST_F(BatAdsLinearSVMTest, SoftmaxTest) {
  const double kEps = 1e-8;

  std::map<std::string, double> group_1 = {
      {"c1", -1.0}, {"c2", 2.0}, {"c3", 3.0}};

  linear_svm::LinearSVM dummy_svm;
  auto sm = dummy_svm.Softmax(group_1);

  ASSERT_TRUE(sm["c3"] > sm["c1"]);
  ASSERT_TRUE(sm["c3"] > sm["c2"]);
  ASSERT_TRUE(sm["c2"] > sm["c1"]);
  ASSERT_TRUE(sm["c1"] > 0.0);
  ASSERT_TRUE(sm["c3"] < 1.0);

  double sum = 0.0;
  for (auto const& x : sm) {
    sum += x.second;
  }

  EXPECT_TRUE(sum - 1.0 < kEps);
}

TEST_F(BatAdsLinearSVMTest, ExtendedSoftmaxTest) {
  const double kEps = 1e-8;

  std::map<std::string, double> group_1 = {
      {"c1", 0.0}, {"c2", 1.0}, {"c3", 2.0}};

  std::map<std::string, double> group_2 = {
      {"c1", 3.0}, {"c2", 4.0}, {"c3", 5.0}};

  linear_svm::LinearSVM dummy_svm;

  auto sm_1 = dummy_svm.Softmax(group_1);
  auto sm_2 = dummy_svm.Softmax(group_2);
  ASSERT_TRUE(std::fabs(sm_1["c1"] - sm_2["c1"]) < kEps);
  ASSERT_TRUE(std::fabs(sm_1["c2"] - sm_2["c2"]) < kEps);
  ASSERT_TRUE(std::fabs(sm_1["c3"] - sm_2["c3"]) < kEps);

  EXPECT_TRUE(std::fabs(sm_1["c1"] - 0.09003057) < kEps
      && std::fabs(sm_1["c2"] - 0.24472847) < kEps
      && std::fabs(sm_1["c3"] - 0.66524095) < kEps);
}

}  // namespace ml_tools
}  // namespace ads
