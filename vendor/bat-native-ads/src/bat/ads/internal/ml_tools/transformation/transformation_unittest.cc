/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cmath>
#include <string>
#include <vector>

#include "base/values.h"

#include "bat/ads/internal/ml_tools/data_point/data_point.h"
#include "bat/ads/internal/ml_tools/transformation/transformation.h"
#include "bat/ads/internal/ml_tools/transformation/hashing_extractor.h"

#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {
namespace ml_tools {

class BatAdsTransformationTest : public UnitTestBase {
 protected:
  BatAdsTransformationTest() = default;

  ~BatAdsTransformationTest() override = default;
};

TEST_F(BatAdsTransformationTest, ToLowerTest) {
  std::string upper_case = "LOWER CASE";
  std::string lower_case = "lower case";
  auto upper_datapoint = data_point::DataPoint(upper_case);
  transformation::ToLower to_lower;
  auto lower_datapoint = to_lower.Get(upper_datapoint);
  EXPECT_TRUE(data_point::DataType::TextData == lower_datapoint.type);
  EXPECT_EQ(0, lower_case.compare(lower_datapoint.data_text));
}

TEST_F(BatAdsTransformationTest, HashingTest) {
  std::string test_string = "tiny";
  auto text_datapoint = data_point::DataPoint(test_string);
  transformation::HashedNGrams hashed_ngrams;
  auto vector_data = hashed_ngrams.Get(text_datapoint);
  EXPECT_EQ(data_point::DataType::SparseVector, vector_data.type);
  // 10000 is the default size
  EXPECT_EQ(10000, vector_data.n_dims);
  // Hashes for [t, i, n, y, ti, in, ny, tin, iny, tiny] -- 10 in total
  size_t expected_elements = 10;
  EXPECT_EQ(expected_elements, vector_data.data_sparse.size());
}

TEST_F(BatAdsTransformationTest, CustomHashingTest) {
  std::string test_string = "tiny";
  auto text_datapoint = data_point::DataPoint(test_string);
  transformation::HashedNGrams hashed_ngrams(3, std::vector<int>{1, 2, 3});
  auto vector_data = hashed_ngrams.Get(text_datapoint);
  EXPECT_EQ(data_point::DataType::SparseVector, vector_data.type);
  EXPECT_EQ(3, vector_data.n_dims);
  size_t expected_elements = 3;
  EXPECT_EQ(expected_elements, vector_data.data_sparse.size());
}

TEST_F(BatAdsTransformationTest, NormalizationTest) {
  const double kEps = 1e-7;

  std::string test_string = "quite a small test string";
  auto text_datapoint = data_point::DataPoint(test_string);
  transformation::HashedNGrams hashed_ngrams(10, std::vector<int>{3, 4});
  auto vector_data = hashed_ngrams.Get(text_datapoint);
  transformation::Normalize normalize;
  auto normalized_datapoint = normalize.Get(vector_data);
  auto s = 0.0; 
  for (auto const& x : normalized_datapoint.data_sparse) {
    EXPECT_TRUE(x.second >= 0.0);
    EXPECT_TRUE(x.second <= 1.0);
    s += x.second * x.second;
  }

  EXPECT_TRUE(std::fabs(s - 1.0) < kEps);
}

TEST_F(BatAdsTransformationTest, ChainingTest) {
  std::vector<transformation::Transformation> chain;
  transformation::ToLower to_lower;
  chain.push_back(to_lower);
  transformation::HashedNGrams hashed_ngrams;
  chain.push_back(hashed_ngrams);

  std::string test_string = "TINY";
  auto last_point = DataPoint(test_string);

  for (size_t i = 0; i < chain.size(); ++i) {
    auto transform = chain[i];
    last_point = transform.Get(last_point);
  }

  EXPECT_EQ(data_point::DataType::SparseVector, last_point.type);
  EXPECT_EQ(10000, last_point.n_dims);
  // Hashes for [t, i, n, y, ti, in, ny, tin, iny, tiny] -- 10 in total
  size_t expected_elements = 10;
  EXPECT_EQ(expected_elements, last_point.data_sparse.size());
}

}  // namespace ml_tools
}  // namespace ads
