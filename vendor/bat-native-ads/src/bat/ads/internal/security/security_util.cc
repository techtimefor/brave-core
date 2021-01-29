/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/security/security_util.h"

#include <openssl/digest.h>
#include <openssl/hkdf.h>
#include <openssl/sha.h>

#include <iostream>

#include <algorithm>

#include "base/base64.h"
#include "base/json/json_reader.h"
#include "base/rand_util.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "tweetnacl.h"  // NOLINT
#include "wrapper.hpp"
#include "bat/ads/internal/account/confirmations/confirmation_info.h"
#include "bat/ads/internal/logging.h"
#include "bat/ads/internal/privacy/challenge_bypass_ristretto_util.h"
#include "bat/ads/internal/security/conversion_id_info.h"
#include "bat/ads/internal/string_util.h"
#include "bat/ads/internal/tokens/redeem_unblinded_token/create_confirmation_util.h"

namespace ads {
namespace security {

using challenge_bypass_ristretto::UnblindedToken;
using challenge_bypass_ristretto::VerificationKey;
using challenge_bypass_ristretto::VerificationSignature;

namespace {

const int kHKDFSeedLength = 32;
const char kAlgorithm[] = "x25519-xsalsa20-poly1305";
const size_t kVacCipherTextLength = 32;
const size_t kVacMessageMaxLength = 30;
const size_t kVacMessageMinLength = 15;
const size_t kVacCryptoBoxPaddedBytes =
    crypto_box_ZEROBYTES + kVacCipherTextLength;

const uint8_t kHKDFSalt[] = {
  126, 244,  99, 158,  51,  68, 253,  80,
  133, 183,  51, 180,  77,  62,  74, 252,
   62, 106,  96, 125, 241, 110, 134,  87,
  190, 208, 158,  84, 125,  69, 246, 207,
  162, 247, 107, 172,  37,  34,  53, 246,
  105,  20, 215,   5, 248, 154, 179, 191,
   46,  17,   6,  72, 210,  91,  10, 169,
  145, 248,  22, 147, 117,  24, 105,  12
};

std::vector<uint8_t> GetHKDF(
    const std::string& secret) {
  if (secret.empty()) {
    return {};
  }

  std::vector<uint8_t> raw_secret;
  raw_secret.assign(secret.begin(), secret.end());

  std::vector<uint8_t> derived_key(kHKDFSeedLength);

  const uint8_t info[] = { 0 };

  const int result = HKDF(&derived_key.front(), kHKDFSeedLength, EVP_sha512(),
      &raw_secret.front(), raw_secret.size(), kHKDFSalt, base::size(kHKDFSalt),
          info, sizeof(info) / sizeof(info[0]));

  if (result == 0) {
    return {};
  }

  return derived_key;
}

bool GenerateKeyPair(
    const std::vector<uint8_t>& seed,
    std::vector<uint8_t>* public_key,
    std::vector<uint8_t>* secret_key) {
  if (seed.empty() || !public_key || !secret_key) {
    return false;
  }

  public_key->resize(crypto_sign_PUBLICKEYBYTES);

  *secret_key = seed;
  secret_key->resize(crypto_sign_SECRETKEYBYTES);

  crypto_sign_keypair(&public_key->front(), &secret_key->front(), 1);

  if (public_key->empty() || secret_key->empty()) {
    return false;
  }

  return true;
}

std::vector<uint8_t> Base64ToUint8List(
    const std::string& value_base64) {
  std::string value_string;
  base::Base64Decode(value_base64, &value_string);
  std::vector<uint8_t> value(value_string.begin(), value_string.end());

  return value;
}

}  // namespace

std::vector<uint8_t> GenerateSecretKeyFromSeed(
    const std::string& seed_base64) {
  std::string seed;
  if (!base::Base64Decode(seed_base64, &seed)) {
    return {};
  }

  const std::vector<uint8_t> derived_key = GetHKDF(seed);

  std::vector<uint8_t> public_key;
  std::vector<uint8_t> secret_key;
  if (!GenerateKeyPair(derived_key, &public_key, &secret_key)) {
    return {};
  }

  return secret_key;
}

std::string Sign(
    const std::map<std::string, std::string>& headers,
    const std::string& key_id,
    const std::string& secret_key) {
  if (headers.empty() || key_id.empty() || secret_key.empty()) {
    return "";
  }

  std::string concatenated_header = "";
  std::string concatenated_message = "";

  unsigned int index = 0;
  for (const auto& header : headers) {
    if (index != 0) {
      concatenated_header += " ";
      concatenated_message += "\n";
    }

    concatenated_header += header.first;
    concatenated_message += header.first + ": " + header.second;

    index++;
  }

  std::vector<uint8_t> signed_message(crypto_sign_BYTES +
      concatenated_message.length());

  std::vector<uint8_t> raw_secret_key;
  if (!base::HexStringToBytes(secret_key, &raw_secret_key)) {
    return "";
  }

  // Resolving the following linter error breaks the build on Windows
  unsigned long long signed_message_length = 0;  // NOLINT
  crypto_sign(&signed_message.front(), &signed_message_length,
      reinterpret_cast<const unsigned char*>(concatenated_message.c_str()),
          concatenated_message.length(), &raw_secret_key.front());

  std::vector<uint8_t> signature(crypto_sign_BYTES);
  std::copy(signed_message.begin(), signed_message.begin() +
      crypto_sign_BYTES, signature.begin());

  return "keyId=\"" + key_id + "\",algorithm=\"" + crypto_sign_PRIMITIVE +
      "\",headers=\"" + concatenated_header + "\",signature=\"" +
          base::Base64Encode(signature) + "\"";
}

std::vector<uint8_t> Sha256Hash(
    const std::string& value) {
  if (value.empty()) {
    return {};
  }

  std::vector<uint8_t> sha256(SHA256_DIGEST_LENGTH);
  SHA256((uint8_t*)value.c_str(), value.length(), &sha256.front());
  return sha256;
}

// TODO(Moritz Haller): move to security/confirmation_security_util.cc
bool Verify(
    const ConfirmationInfo& confirmation) {
  std::string credential;
  base::Base64Decode(confirmation.credential, &credential);

  base::Optional<base::Value> value = base::JSONReader::Read(credential);
  if (!value || !value->is_dict()) {
    return false;
  }

  base::DictionaryValue* dictionary = nullptr;
  if (!value->GetAsDictionary(&dictionary)) {
    return false;
  }

  const std::string* signature = dictionary->FindStringKey("signature");
  if (!signature) {
    return false;
  }

  VerificationSignature verification_signature =
      VerificationSignature::decode_base64(*signature);
  if (privacy::ExceptionOccurred()) {
    NOTREACHED();
    return false;
  }

  const std::string payload = CreateConfirmationRequestDTO(confirmation);

  UnblindedToken unblinded_token = confirmation.unblinded_token.value;
  VerificationKey verification_key = unblinded_token.derive_verification_key();
  if (privacy::ExceptionOccurred()) {
    NOTREACHED();
    return false;
  }

  return verification_key.verify(verification_signature, payload);
}

std::vector<uint8_t> Encrypt(
    const std::vector<uint8_t>& message,
    const std::vector<uint8_t>& nonce,
    const std::vector<uint8_t>& public_key,
    const std::vector<uint8_t>& ephemeral_sk) {
  // API requires 32 leading zero-padding bytes
  std::vector<uint8_t> padded_message = message;
  padded_message.insert(padded_message.begin(), crypto_box_ZEROBYTES, 0);
  DCHECK_EQ(kVacCryptoBoxPaddedBytes, padded_message.size());

  std::vector<uint8_t> ciphertext(kVacCryptoBoxPaddedBytes);
  crypto_box(&ciphertext.front(), &padded_message.front(),
      kVacCryptoBoxPaddedBytes, &nonce.front(), &public_key.front(),
          &ephemeral_sk.front());

  return ciphertext;
}

std::string Decrypt(
    const std::vector<uint8_t>& ciphertext,
    const std::vector<uint8_t>& nonce,
    const std::vector<uint8_t>& ephemeral_pk,
    const std::vector<uint8_t>& secret_key) {
  std::vector<uint8_t> plaintext(crypto_box_ZEROBYTES + kVacCipherTextLength);
  crypto_box_open(&plaintext.front(), &ciphertext.front(), ciphertext.size(),
      &nonce.front(), &ephemeral_pk.front(), &secret_key.front());

  std::vector<uint8_t> plaintext_slice(plaintext.begin() +
      crypto_box_ZEROBYTES, plaintext.end());

  return (const char*)&plaintext_slice.front();
}

ConversionIdInfo EncryptAndEncodeConversionId(
    const std::string& message,
    const std::string& advertiser_pk_base64) {
  ConversionIdInfo envelope;

  if (message.length() < kVacMessageMinLength ||
      message.length() > kVacMessageMaxLength) {
    return envelope;
  }

  if (!IsLatinAlphaNumeric(message)) {
    return envelope;
  }

  // Protocol requires at least 2 trailing zero-padding bytes
  std::vector<uint8_t> padded_message(message.begin(), message.end());
  padded_message.insert(padded_message.end(),
      kVacCipherTextLength - padded_message.size(), 0);
  DCHECK_EQ(kVacCipherTextLength, padded_message.size());

  std::vector<uint8_t> advertiser_pk = Base64ToUint8List(advertiser_pk_base64);
  if (advertiser_pk.size() != crypto_box_PUBLICKEYBYTES) {
    return envelope;
  }

  std::vector<uint8_t> ephemeral_sk(crypto_box_SECRETKEYBYTES);
  std::vector<uint8_t> ephemeral_pk(crypto_box_PUBLICKEYBYTES);
  crypto_box_keypair(&ephemeral_pk.front(), &ephemeral_sk.front());

  std::vector<uint8_t> nonce(crypto_box_NONCEBYTES);
  base::RandBytes(&nonce.front(), nonce.size());

  std::vector<uint8_t> ciphertext = Encrypt(padded_message, nonce,
      advertiser_pk, ephemeral_sk);

  std::cout << "*** ciphertext.size: " << ciphertext.size();

  // The receiving TweetNaCl.js client does not require padding
  std::vector<uint8_t> ciphertext_slice(ciphertext.begin() +
      crypto_box_BOXZEROBYTES, ciphertext.end());

  std::cout << "*** ciphertext_slice.size: " << ciphertext_slice.size();

  envelope.algorithm = kAlgorithm;
  envelope.ciphertext = base::Base64Encode(ciphertext_slice);
  envelope.ephemeral_pk = base::Base64Encode(ephemeral_pk);
  envelope.nonce = base::Base64Encode(nonce);

  return envelope;
}

std::string DecodeAndDecryptConversionId(
    const ConversionIdInfo envelope,
    const std::string& advertiser_sk_base64) {
  std::string message;
  if (envelope.ciphertext.empty() || envelope.nonce.empty() ||
      envelope.ephemeral_pk.empty()) {
    return message;
  }

  std::vector<uint8_t> advertiser_sk = Base64ToUint8List(advertiser_sk_base64);
  std::vector<uint8_t> nonce = Base64ToUint8List(envelope.nonce);
  std::vector<uint8_t> ciphertext = Base64ToUint8List(envelope.ciphertext);
  std::vector<uint8_t> ephemeral_pk = Base64ToUint8List(envelope.ephemeral_pk);

  // API requires 16 leading zero-padding bytes
  ciphertext.insert(ciphertext.begin(), crypto_box_BOXZEROBYTES, 0);
  std::cout << "*** ciphertext.size: " << ciphertext.size() << "\n";
  std::cout << "*** ciphertext: " << envelope.ciphertext << "\n";
  std::cout << "*** nonce: " << envelope.nonce << "\n";
  std::cout << "*** ephemeral_pk: " << envelope.ephemeral_pk << "\n";

  return Decrypt(ciphertext, nonce, ephemeral_pk, advertiser_sk);
}

}  // namespace security
}  // namespace ads
