/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef BAT_ADS_INTERNAL_ML_TOOLS_TRANSFORMATION_HASHING_EXTRACTOR_H_  // NOLINT
#define BAT_ADS_INTERNAL_ML_TOOLS_TRANSFORMATION_HASHING_EXTRACTOR_H_  // NOLINT

#include <string>
#include <map>
#include <vector>

#include "bat/ads/internal/ml_tools/data_point/data_point.h"

namespace ads {
namespace ml_tools {

using data_point::DataPoint;

namespace transformation {

class HashVectorizer {
 public:
  HashVectorizer();
  HashVectorizer(const HashVectorizer& other);
  HashVectorizer(int n_buckets, std::vector<int> subgrams);
  ~HashVectorizer();

  std::map<unsigned, double> GetFrequencies(const std::string& html);
  int GetBucketCount();

 private:
  int GetHash(std::string& substring);
  std::vector<unsigned int> substring_sizes_; // not ints for type comparison issues
  int num_buckets_; // number of buckets to use for hashing
};

}  // namespace transformation
}  // namespace ml_tools
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_ML_TOOLS_TRANSFORMATION_HASHING_EXTRACTOR_H_  // NOLINT
