/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ml_tools/transformation/transformation.h"

#include <cmath>
#include <codecvt>

namespace ads {
namespace ml_tools {
namespace transformation {

Transformation::Transformation() {}

Transformation::Transformation(TransformationType t) {
  type = t;
}

Transformation::~Transformation() = default;

TransformationType Transformation::GetType() {
  return type;
}

DataPoint Transformation::Get(DataPoint inp){
  switch(type){
    case TransformationType::TO_LOWER:
      return GetLower(inp);
    case TransformationType::HASHED_NGRAMS:
      return GetNGrams(inp);
    case TransformationType::NORMALIZE:
      return GetNormalized(inp);
    case TransformationType::NONE:
      return DataPoint("");
  }
}

DataPoint Transformation::GetLower(DataPoint datapoint){
  std::string rtn_str;
  rtn_str.assign(datapoint.data_text);
  std::transform(rtn_str.begin(), rtn_str.end(), rtn_str.begin(), ::tolower);
  return DataPoint(rtn_str);
}

DataPoint Transformation::GetNGrams(DataPoint datapoint){
  auto hashed_vector = hash_vectorizer.GetFrequencies(datapoint.data_text);
  return DataPoint(hashed_vector, hash_vectorizer.GetBucketCount());
}

DataPoint Transformation::GetNormalized(DataPoint datapoint){
  auto s = 0.0;
  std::map<unsigned, double> normalized_vector;

  for (auto const& x : datapoint.data_sparse){
    s += x.second * x.second ;
  }

  auto norm = sqrt(s);
  for (auto const& x : datapoint.data_sparse){
    normalized_vector[x.first] = x.second / norm;
  }

  return DataPoint(normalized_vector, hash_vectorizer.GetBucketCount());
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

HashedNGrams::HashedNGrams(int n_b, std::vector<int> subgrams) {
  type = TransformationType::HASHED_NGRAMS;
  hash_vectorizer = HashVectorizer(n_b, subgrams);
}

HashedNGrams::HashedNGrams(std::string config) {}

}  // namespace transformation
}  // namespace ml_tools
}  // namespace ads
