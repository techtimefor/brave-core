/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_ML_TOOLS_ML_TOOLS_UTIL_H_
#define BAT_ADS_INTERNAL_ML_TOOLS_ML_TOOLS_UTIL_H_

#include <cmath>
#include <map>
#include <string>

namespace ads {
namespace ml_tools {

std::map<std::string, double> softmax(
    const std::map<std::string, double>& y);

}  // namespace ml_tools
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_ML_TOOLS_ML_TOOLS_UTIL_H_
