/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_ML_TOOLS_TRANSFORMATION_TRANSFORMATION_H_  // NOLINT
#define BAT_ADS_INTERNAL_ML_TOOLS_TRANSFORMATION_TRANSFORMATION_H_  // NOLINT

#include <iostream>
#include <string>
#include <vector>

#include "bat/ads/internal/ml_tools/data_point/data_point.h"
#include "bat/ads/internal/ml_tools/transformation/hashing_extractor.h"

namespace ads {
namespace ml_tools {
namespace transformation {

enum class TransformationType {
  NONE = 0,
  TO_LOWER = 1,
  HASHED_NGRAMS = 2,
  NORMALIZE = 3
};

class Transformation {
 public:
  Transformation();

  explicit Transformation(
      const TransformationType& t);

  explicit Transformation(
      const std::string& parameters);

  ~Transformation();

  data_point::DataPoint Get(
      const data_point::DataPoint& inp);

  TransformationType GetType();

 protected:
  HashVectorizer hash_vectorizer;
  TransformationType type;

 private:
  data_point::DataPoint GetLower(
      const data_point::DataPoint& inp);

  data_point::DataPoint GetNGrams(
      const data_point::DataPoint& inp);

  data_point::DataPoint GetNormalized(
      const data_point::DataPoint& inp);
};

class ToLower : public Transformation {
 public:
  ToLower();
};

class HashedNGrams : public Transformation {
 public:
  HashedNGrams();

  HashedNGrams(
      int n_b,
      const std::vector<int>& subgrams);

  explicit HashedNGrams(
      const std::string& parameters);
};

class Normalize : public Transformation {
 public:
  Normalize();
};

}  // namespace transformation
}  // namespace ml_tools
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_ML_TOOLS_TRANSFORMATION_TRANSFORMATION_H_  // NOLINT
