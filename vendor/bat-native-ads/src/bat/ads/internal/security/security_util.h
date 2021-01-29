/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_SECURITY_SECURITY_UTIL_H_
#define BAT_ADS_INTERNAL_SECURITY_SECURITY_UTIL_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

namespace ads {

struct ConfirmationInfo;

namespace security {

struct ConversionIdInfo;

// TODO(Moritz Haller): Rename to crypto_util.cc

std::string Sign(
    const std::map<std::string, std::string>& headers,
    const std::string& key_id,
    const std::string& secret_key);

std::vector<uint8_t> Sha256Hash(
    const std::string& value);

// The tweetnacl |crypto_box| API requires first 32 bytes of |padded_message|
// to be 0
std::vector<uint8_t> Encrypt(
    const std::vector<uint8_t>& message,
    const std::vector<uint8_t>& nonce,
    const std::vector<uint8_t>& public_key,
    const std::vector<uint8_t>& ephemeral_secret_key);

std::string Decrypt(
    const std::vector<uint8_t>& ciphertext,
    const std::vector<uint8_t>& nonce,
    const std::vector<uint8_t>& ephemeral_public_key,
    const std::vector<uint8_t>& secret_key);

// TODO(Moritz Haller): privacy/wallet/wallet_util.cc
std::vector<uint8_t> GenerateSecretKeyFromSeed(
    const std::string& seed_base64);

// TODO(Moritz Haller): privacy/confirmations/confirmations_util.cc
bool Verify(
    const ConfirmationInfo& confirmation);

// TODO(Moritz Haller): conversions/conversions_util.cc
ConversionIdInfo EncryptAndEncodeConversionId(
    const std::string& message,
    const std::string& advertiser_pk_base64);

std::string DecodeAndDecryptConversionId(
    const ConversionIdInfo conversion_id_info,
    const std::string& advertiser_sk_base64);

}  // namespace security
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_SECURITY_SECURITY_UTIL_H_
