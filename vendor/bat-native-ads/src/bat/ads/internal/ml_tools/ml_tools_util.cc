/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <string>

#include "bat/ads/internal/ml_tools/ml_tools_util.h"

namespace ads {
namespace ml_tools {

std::map<std::string, double> softmax(
    const std::map<std::string, double>& y) {
  double maximum = -std::numeric_limits<double>::infinity();
  for (auto const& x : y) {
    maximum = std::max(maximum, x.second);
  }
  std::map<std::string, double> rtn;
  double sum_exp = 0.0;
  for (auto const& x : y) {
    double val = std::exp(x.second - maximum);
    rtn[x.first] = val;
    sum_exp += val;
  }
  for (auto const& x : rtn) {
    rtn[x.first] /= sum_exp;
  }
  return rtn;
}

}  // namespace ml_tools
}  // namespace ads
