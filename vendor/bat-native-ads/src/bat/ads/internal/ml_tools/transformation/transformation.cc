/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ml_tools/transformation/transformation.h"

#include <algorithm>
#include <cmath>
#include <codecvt>
#include <map>
#include <string>

namespace ads {
namespace ml_tools {
namespace transformation {

Transformation::Transformation() {}

Transformation::Transformation(
    const TransformationType& t) {
  type = t;
}

Transformation::~Transformation() = default;

TransformationType Transformation::GetType() {
  return type;
}

data_point::DataPoint Transformation::Get(
    const data_point::DataPoint& inp) {
  switch (type) {
    case TransformationType::TO_LOWER:
      return GetLower(inp);
    case TransformationType::HASHED_NGRAMS:
      return GetNGrams(inp);
    case TransformationType::NORMALIZE:
      return GetNormalized(inp);
    case TransformationType::NONE:
      return data_point::DataPoint("");
  }
}

data_point::DataPoint Transformation::GetLower(
    const data_point::DataPoint& datapoint) {
  std::string rtn_str;
  rtn_str.assign(datapoint.data_text);
  std::transform(rtn_str.begin(), rtn_str.end(), rtn_str.begin(), ::tolower);
  return data_point::DataPoint(rtn_str);
}

data_point::DataPoint Transformation::GetNGrams(
    const data_point::DataPoint& datapoint) {
  auto hashed_vector = hash_vectorizer.GetFrequencies(datapoint.data_text);
  return data_point::DataPoint(hashed_vector, hash_vectorizer.GetBucketCount());
}

data_point::DataPoint Transformation::GetNormalized(
    const data_point::DataPoint& datapoint) {
  auto s = 0.0;
  std::map<unsigned, double> normalized_vector;

  for (auto const& x : datapoint.data_sparse) {
    s += x.second * x.second;
  }

  auto norm = sqrt(s);
  for (auto const& x : datapoint.data_sparse) {
    normalized_vector[x.first] = x.second / norm;
  }

  return data_point::DataPoint(normalized_vector,
      hash_vectorizer.GetBucketCount());
}

ToLower::ToLower() {
  type = TransformationType::TO_LOWER;
}

Normalize::Normalize() {
  type = TransformationType::NORMALIZE;
}

HashedNGrams::HashedNGrams() {
  type = TransformationType::HASHED_NGRAMS;
  hash_vectorizer = HashVectorizer();
}

HashedNGrams::HashedNGrams(
    int n_b,
    const std::vector<int>& subgrams) {
  type = TransformationType::HASHED_NGRAMS;
  hash_vectorizer = HashVectorizer(n_b, subgrams);
}

HashedNGrams::HashedNGrams(
    const std::string& config) {}

}  // namespace transformation
}  // namespace ml_tools
}  // namespace ads
