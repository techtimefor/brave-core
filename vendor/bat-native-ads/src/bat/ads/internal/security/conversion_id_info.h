/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_SECURITY_CONVERSION_ID_INFO_H_
#define BAT_ADS_INTERNAL_SECURITY_CONVERSION_ID_INFO_H_

#include <string>

namespace ads {
namespace security {

struct ConversionIdInfo {
  ConversionIdInfo();
  ConversionIdInfo(
      const ConversionIdInfo& info);
  ~ConversionIdInfo();

  std::string algorithm;
  std::string ciphertext;
  std::string ephemeral_pk;
  std::string nonce;
};

}  // namespace security
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_SECURITY_CONVERSION_ID_INFO_H_
