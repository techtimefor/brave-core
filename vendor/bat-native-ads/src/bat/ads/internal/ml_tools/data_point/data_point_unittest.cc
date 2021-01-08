/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <cmath>
#include <string>

#include "bat/ads/internal/ml_tools/data_point/data_point.h"

#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"

namespace ads {
namespace ml_tools {

class DataPointTest : public UnitTestBase {
 protected:
  DataPointTest() = default;

  ~DataPointTest() override = default;
};

TEST_F(DataPointTest, DataPointTypeAssertions) {
  // String
  std::string a = "trololol";
  auto string_data_point = data_point::DataPoint(a);

  EXPECT_EQ(data_point::DataType::TextData, string_data_point.type);
  EXPECT_EQ(0, string_data_point.n_dims);

  // Dense vectors
  std::vector<double> v_5{1.0, 2.0, 3.0, 4.0, 5.0};
  auto dense_data_point_5 = data_point::DataPoint(v_5);

  EXPECT_EQ(data_point::DataType::VectorData, dense_data_point_5.type);
  EXPECT_EQ(5, dense_data_point_5.n_dims);

  std::vector<double> v_3{1.0, 2.0, 3.0};
  auto dense_data_point_3 = data_point::DataPoint(v_3);

  EXPECT_EQ(data_point::DataType::VectorData, dense_data_point_3.type);
  EXPECT_EQ(3, dense_data_point_3.n_dims);

  // Sparse vectors
  // Dense equivalent is [1, 0, 2]
  std::map<unsigned, double> s_3 = {
    {0UL, 1.0},
    {2UL, 2.0}
  };
  auto sparse_data_point_3 = data_point::DataPoint(s_3, 3);

  EXPECT_EQ(data_point::DataType::SparseVector, sparse_data_point_3.type);
  EXPECT_EQ(3, sparse_data_point_3.n_dims);

  // Dense equivalent is [1, 0, 3, 2, 0]
  std::map<unsigned, double> s_5 = {
    {0UL, 1.0},
    {2UL, 3.0},
    {3UL, -2.0}
  };
  auto sparse_data_point_5 = data_point::DataPoint(s_5, 5);

  EXPECT_EQ(data_point::DataType::SparseVector, sparse_data_point_5.type);
  EXPECT_EQ(5, sparse_data_point_5.n_dims);
}

TEST_F(DataPointTest, DenseDenseProduct) {
  const double kEps = 1e-6;

  std::vector<double> v_5{1.0, 2.0, 3.0, 4.0, 5.0};
  auto dense_data_point_5 = data_point::DataPoint(v_5);

  std::vector<double> v_3{1.0, 2.0, 3.0};
  auto dense_data_point_3 = data_point::DataPoint(v_3);

  std::vector<double> v_3_1{1.0, 1.0, 1.0};
  auto dense_data_point_3_1 = data_point::DataPoint(v_3_1);

  auto res_3x3 = dense_data_point_3 * dense_data_point_3;
  EXPECT_TRUE(std::fabs(14.0 - res_3x3) < kEps);

  auto res_5x5 = dense_data_point_5 * dense_data_point_5;
  EXPECT_TRUE(std::fabs(55.0 - res_5x5) < kEps);

  auto res_3x1 = dense_data_point_3 * dense_data_point_3_1;
  EXPECT_TRUE(std::fabs(6.0 - res_3x1) < kEps);
}

TEST_F(DataPointTest, SparseSparseProduct) {
  const double kEps = 1e-6;

  // Dense equivalent is [1, 0, 2]
  std::map<unsigned, double> s_3 = {
    {0UL, 1.0},
    {2UL, 2.0}
  };
  auto sparse_data_point_3 = data_point::DataPoint(s_3, 3);

  // Dense equivalent is [1, 0, 3, 2, 0]
  std::map<unsigned, double> s_5 = {
    {0UL, 1.0},
    {2UL, 3.0},
    {3UL, -2.0}
  };
  auto sparse_data_point_5 = data_point::DataPoint(s_5, 5); 

  auto res_3x3 = sparse_data_point_3 * sparse_data_point_3; // = 5
  auto res_5x5 = sparse_data_point_5 * sparse_data_point_5; // = 14
  EXPECT_TRUE(std::fabs(5.0 - res_3x3) < kEps);
  EXPECT_TRUE(std::fabs(14.0 - res_5x5) < kEps);
}

TEST_F(DataPointTest, SparseDenseProduct) {
  const double kEps = 1e-6;

  std::vector<double> v_5{1.0, 2.0, 3.0, 4.0, 5.0};
  auto dense_data_point_5 = data_point::DataPoint(v_5);

  std::vector<double> v_3{1.0, 2.0, 3.0};
  auto dense_data_point_3 = data_point::DataPoint(v_3);

  // Dense equivalent is [1, 0, 2]
  std::map<unsigned, double> s_3 = {
    {0UL, 1.0},
    {2UL, 2.0}
  };
  auto sparse_data_point_3 = data_point::DataPoint(s_3, 3);

  // Dense equivalent is [1, 0, 3, 2, 0]
  std::map<unsigned, double> s_5 = {
    {0UL, 1.0},
    {2UL, 3.0},
    {3UL, -2.0}
  };
  auto sparse_data_point_5 = data_point::DataPoint(s_5, 5);

  auto mixed_res_3x3_1 = dense_data_point_3 * sparse_data_point_3; // = 7
  auto mixed_res_5x5_1 = dense_data_point_5 * sparse_data_point_5; // = 2
  auto mixed_res_3x3_2 = sparse_data_point_3 * dense_data_point_3; // = 7
  auto mixed_res_5x5_2 = sparse_data_point_5 * dense_data_point_5; // = 2

  EXPECT_TRUE(std::fabs(mixed_res_3x3_1 - mixed_res_3x3_2) < kEps);
  EXPECT_TRUE(std::fabs(mixed_res_5x5_1 - mixed_res_5x5_2) < kEps);
  EXPECT_TRUE(std::fabs(7.0 - mixed_res_3x3_1) < kEps);
  EXPECT_TRUE(std::fabs(2.0 - mixed_res_5x5_2) < kEps);
}

TEST_F(DataPointTest, NonsenseProduct) {
  std::string a = "trololol";
  auto string_data_point = data_point::DataPoint(a);

  std::vector<double> v_5{1.0, 2.0, 3.0, 4.0, 5.0};
  auto dense_data_point_5 = data_point::DataPoint(v_5);

  std::vector<double> v_3{1.0, 2.0, 3.0};
  auto dense_data_point_3 = data_point::DataPoint(v_3);

  // Dense equivalent is [1, 0, 2]
  std::map<unsigned, double> s_3 = {
    {0UL, 1.0},
    {2UL, 2.0}
  };
  auto sparse_data_point_3 = data_point::DataPoint(s_3, 3);

  // Dense equivalent is [1, 0, 3, 2, 0]
  std::map<unsigned, double> s_5 = {
    {0UL, 1.0},
    {2UL, 3.0},
    {3UL, -2.0}
  };
  auto sparse_data_point_5 = data_point::DataPoint(s_5, 5);

  auto wrong_dd = dense_data_point_5 * dense_data_point_3;
  EXPECT_TRUE(std::isnan(wrong_dd));

  auto wrong_ss = sparse_data_point_3 * sparse_data_point_5;
  EXPECT_TRUE(std::isnan(wrong_ss));

  auto wrong_sd = sparse_data_point_3 * dense_data_point_5;
  EXPECT_TRUE(std::isnan(wrong_sd)); 

  auto wrong_ds = dense_data_point_5 * sparse_data_point_3;
  EXPECT_TRUE(std::isnan(wrong_ds));

  auto wrong_strd = string_data_point * dense_data_point_5;
  EXPECT_TRUE(std::isnan(wrong_strd));

  auto wrong_strs = string_data_point * sparse_data_point_3;
  EXPECT_TRUE(std::isnan(wrong_strs));
}

}  // namespace ml_tools
}  // namespace ads
