/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cmath>
#include <map>
#include <string>

#include "bat/ads/internal/ml_tools/ml_tools_util.h"

#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {
namespace ml_tools {

class BatAdsMLToolsUtilTest : public UnitTestBase {
 protected:
  BatAdsMLToolsUtilTest() = default;

  ~BatAdsMLToolsUtilTest() override = default;
};


TEST_F(BatAdsMLToolsUtilTest, SoftmaxTest) {
  const double kEps = 1e-8;

  std::map<std::string, double> group_1 = {
      {"c1", -1.0}, {"c2", 2.0}, {"c3", 3.0}};

  auto sm = softmax(group_1);

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

TEST_F(BatAdsMLToolsUtilTest, ExtendedSoftmaxTest) {
  const double kEps = 1e-8;

  std::map<std::string, double> group_1 = {
      {"c1", 0.0}, {"c2", 1.0}, {"c3", 2.0}};

  std::map<std::string, double> group_2 = {
      {"c1", 3.0}, {"c2", 4.0}, {"c3", 5.0}};

  auto sm_1 = softmax(group_1);
  auto sm_2 = softmax(group_2);
  ASSERT_TRUE(std::fabs(sm_1["c1"] - sm_2["c1"]) < kEps);
  ASSERT_TRUE(std::fabs(sm_1["c2"] - sm_2["c2"]) < kEps);
  ASSERT_TRUE(std::fabs(sm_1["c3"] - sm_2["c3"]) < kEps);

  EXPECT_TRUE(std::fabs(sm_1["c1"] - 0.09003057) < kEps
      && std::fabs(sm_1["c2"] - 0.24472847) < kEps
      && std::fabs(sm_1["c3"] - 0.66524095) < kEps);
}

}  // namespace ml_tools
}  // namespace ads
