/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <algorithm>
#include <codecvt>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "bat/ads/internal/ml_tools/ml_tools_static_values.h"
#include "bat/ads/internal/ml_tools/transformation/crc.h"
#include "bat/ads/internal/ml_tools/transformation/hashing_extractor.h"

namespace ads {
namespace ml_tools {
namespace transformation {

HashVectorizer::~HashVectorizer() = default;

HashVectorizer::HashVectorizer() {
  substring_sizes_.resize(kMaxSubLen);
  for (int i = 0; i < kMaxSubLen; ++i) {
    substring_sizes_[i] = i + 1;
  }
  num_buckets_ = kNumBuckets;
}

HashVectorizer::HashVectorizer(
    int n_buckets,
    const std::vector<int>& subgrams) {
  for (size_t i = 0; i < subgrams.size(); i++) {
    substring_sizes_.push_back(subgrams[i]);
  }
  num_buckets_ = n_buckets;
}

HashVectorizer::HashVectorizer(
    const HashVectorizer& other) {
  substring_sizes_ = other.substring_sizes_;
  num_buckets_ = other.num_buckets_;
}

int HashVectorizer::GetBucketCount() {
  return num_buckets_;
}

int HashVectorizer::GetHash(
    const std::string& substring) {
  auto* u8str = substring.c_str();
  auto rtn = CRC::Calculate(u8str, strlen(u8str), CRC::CRC_32()) % num_buckets_;
  return rtn;
}

std::map<unsigned, double> HashVectorizer::GetFrequencies(
    const std::string& html) {
  std::string data = html;
  std::map<unsigned, double> frequencies;
  if (data.length() > kMaximumHtmlLengthToClassify) {
    data = data.substr(0, kMaximumHtmlLengthToClassify - 1);
  }
  // get hashes of substrings for each of the substring lengths defined:
  for (auto const& substring_size : substring_sizes_) {
    if (substring_size <= data.length()) {
      for (size_t i = 0; i <= (data.length() - substring_size); i++) {
        auto ss = data.substr(i, substring_size);
        auto idx = GetHash(ss);
        ++frequencies[idx];
      }
    }
  }
  return frequencies;
}

}  // namespace transformation
}  // namespace ml_tools
}  // namespace ads
