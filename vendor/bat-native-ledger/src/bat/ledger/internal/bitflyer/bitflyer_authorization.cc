/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/bitflyer/bitflyer_authorization.h"

#include <utility>

#include "base/json/json_reader.h"
#include "base/strings/stringprintf.h"
#include "bat/ledger/global_constants.h"
#include "bat/ledger/internal/bitflyer/bitflyer_util.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/logging/event_log_keys.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace ledger {
namespace bitflyer {

BitflyerAuthorization::BitflyerAuthorization(LedgerImpl* ledger) :
    ledger_(ledger),
    bitflyer_server_(std::make_unique<endpoint::BitflyerServer>(ledger)) {
}

BitflyerAuthorization::~BitflyerAuthorization() = default;

void BitflyerAuthorization::Authorize(
    const base::flat_map<std::string, std::string>& args,
    ledger::ExternalWalletAuthorizationCallback callback) {
  auto wallet = GetWallet(ledger_);

  if (!wallet) {
    BLOG(0, "Wallet is null");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  const auto current_one_time = wallet->one_time_string;

  // we need to generate new string as soon as authorization is triggered
  wallet->one_time_string = GenerateRandomString(ledger::is_testing);
  const bool success = ledger_->bitflyer()->SetWallet(wallet->Clone());

  if (!success) {
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  auto it = args.find("error_description");
  if (it != args.end()) {
    const std::string message = args.at("error_description");
    BLOG(1, message);
    if (message == "User does not meet minimum requirements") {
      callback(type::Result::NOT_FOUND, {});
      return;
    }

    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  if (args.empty()) {
    BLOG(0, "Arguments are empty");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  std::string code;
  it = args.find("code");
  if (it != args.end()) {
    code = args.at("code");
  }

  if (code.empty()) {
    BLOG(0, "Code is empty");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  std::string one_time_string;
  it = args.find("state");
  if (it != args.end()) {
    one_time_string = args.at("state");
  }

  if (one_time_string.empty()) {
    BLOG(0, "One time string is empty");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  if (current_one_time != one_time_string) {
    BLOG(0, "One time string mismatch");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  auto url_callback = std::bind(&BitflyerAuthorization::OnAuthorize,
      this,
      _1,
      _2,
      callback);

  bitflyer_server_->post_oauth()->Request(code, url_callback);
}

void BitflyerAuthorization::OnAuthorize(
    const type::Result result,
    const std::string& token,
    ledger::ExternalWalletAuthorizationCallback callback) {
  if (result == type::Result::EXPIRED_TOKEN) {
    BLOG(0, "Expired token");
    callback(type::Result::EXPIRED_TOKEN, {});
    ledger_->bitflyer()->DisconnectWallet();
    return;
  }

  if (result != type::Result::LEDGER_OK) {
    BLOG(0, "Couldn't get token");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  if (token.empty()) {
    BLOG(0, "Token is empty");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  auto wallet_ptr = GetWallet(ledger_);

  wallet_ptr->token = token;

  switch (wallet_ptr->status) {
    case type::WalletStatus::NOT_CONNECTED: {
      wallet_ptr->status = type::WalletStatus::CONNECTED;
      break;
    }
    case type::WalletStatus::DISCONNECTED_NOT_VERIFIED: {
      wallet_ptr->status = type::WalletStatus::CONNECTED;
      break;
    }
    case type::WalletStatus::DISCONNECTED_VERIFIED: {
      wallet_ptr->status = type::WalletStatus::VERIFIED;
      break;
    }
    default:
      break;
  }

  ledger_->bitflyer()->SetWallet(wallet_ptr->Clone());

  auto user_callback = std::bind(&BitflyerAuthorization::OnGetUser,
      this,
      _1,
      _2,
      callback);
  ledger_->bitflyer()->GetUser(user_callback);
}

void BitflyerAuthorization::OnGetUser(
    const type::Result result,
    const User& user,
    ledger::ExternalWalletAuthorizationCallback callback) {
  auto wallet_ptr = GetWallet(ledger_);
  base::flat_map<std::string, std::string> args;

  if (user.bat_not_allowed || !wallet_ptr) {
    BLOG(0, "BAT not allowed");
    callback(type::Result::BAT_NOT_ALLOWED, args);
    return;
  }

  if (user.status == UserStatus::OK) {
    wallet_ptr->status = user.verified
        ? type::WalletStatus::VERIFIED
        : type::WalletStatus::CONNECTED;
    ledger_->bitflyer()->SetWallet(wallet_ptr->Clone());

    if (wallet_ptr->address.empty()) {
      auto new_callback = std::bind(&BitflyerAuthorization::OnCardCreate,
          this,
          _1,
          _2,
          callback);
      ledger_->bitflyer()->CreateCard(new_callback);
      return;
    }

    if (!user.verified) {
      args["redirect_url"] = GetSecondStepVerify();
    }
  } else {
    wallet_ptr->status = type::WalletStatus::PENDING;
    ledger_->bitflyer()->SetWallet(wallet_ptr->Clone());
    args["redirect_url"] = GetSecondStepVerify();
  }

  callback(type::Result::LEDGER_OK, args);
}

void BitflyerAuthorization::OnCardCreate(
    const type::Result result,
    const std::string& address,
    ledger::ExternalWalletAuthorizationCallback callback) {
  if (result == type::Result::LEDGER_ERROR) {
    BLOG(0, "Card creation");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  auto wallet_ptr = GetWallet(ledger_);
  wallet_ptr->address = address;
  ledger_->bitflyer()->SetWallet(wallet_ptr->Clone());

  if (!address.empty()) {
    ledger_->database()->SaveEventLog(
        log::kWalletConnected,
        static_cast<std::string>(constant::kWalletBitflyer) + "/" +
            address.substr(0, 5));
  }

  base::flat_map<std::string, std::string> args;
  if (wallet_ptr->status != type::WalletStatus::VERIFIED) {
    args["redirect_url"] = GetSecondStepVerify();
  }

  callback(type::Result::LEDGER_OK, args);
}

}  // namespace bitflyer
}  // namespace ledger
