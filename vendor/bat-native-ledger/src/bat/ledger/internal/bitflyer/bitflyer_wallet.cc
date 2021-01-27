/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <utility>

#include "base/json/json_reader.h"
#include "bat/ledger/global_constants.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/logging/event_log_keys.h"
#include "bat/ledger/internal/bitflyer/bitflyer_util.h"
#include "bat/ledger/internal/bitflyer/bitflyer_wallet.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace ledger {
namespace bitflyer {

BitflyerWallet::BitflyerWallet(LedgerImpl* ledger) :
    ledger_(ledger) {
}

BitflyerWallet::~BitflyerWallet() = default;

void BitflyerWallet::Generate(ledger::ResultCallback callback) {
  auto wallet = GetWallet(ledger_);
  if (!wallet) {
    wallet = type::BitflyerWallet::New();
    wallet->status = type::WalletStatus::NOT_CONNECTED;
  }

  if (wallet->one_time_string.empty()) {
    wallet->one_time_string = GenerateRandomString(ledger::is_testing);
  }

  if (wallet->token.empty() &&
      (wallet->status == type::WalletStatus::PENDING ||
       wallet->status == type::WalletStatus::CONNECTED)) {
    wallet->status = type::WalletStatus::NOT_CONNECTED;
  }

  wallet = GenerateLinks(std::move(wallet));
  ledger_->bitflyer()->SetWallet(wallet->Clone());

  if (wallet->status == type::WalletStatus::CONNECTED ||
      wallet->status == type::WalletStatus::VERIFIED ||
      wallet->status == type::WalletStatus::PENDING) {
    const auto user_callback = std::bind(&BitflyerWallet::OnGenerate,
        this,
        _1,
        _2,
        callback);
    ledger_->bitflyer()->GetUser(user_callback);
    return;
  }

  callback(type::Result::LEDGER_OK);
}

void BitflyerWallet::OnGenerate(
    const type::Result result,
    const User& user,
    ledger::ResultCallback callback) {
  auto wallet_ptr = GetWallet(ledger_);
  if (result == type::Result::EXPIRED_TOKEN) {
    ledger_->bitflyer()->DisconnectWallet();
    callback(result);
    return;
  }

  if (user.bat_not_allowed) {
    BLOG(0, "BAT not allowed");
    callback(type::Result::BAT_NOT_ALLOWED);
    return;
  }

  if (!wallet_ptr || result != type::Result::LEDGER_OK) {
    BLOG(0, "Wallet not generated");
    callback(result);
    return;
  }

  wallet_ptr->user_name = user.name;

  if (user.status != UserStatus::OK) {
    wallet_ptr->status = type::WalletStatus::PENDING;
  } else {
    wallet_ptr->status = GetNewStatus(wallet_ptr->status, user);
  }

  ledger_->bitflyer()->SetWallet(wallet_ptr->Clone());

  if (wallet_ptr->status != type::WalletStatus::PENDING &&
      wallet_ptr->address.empty()) {
    auto card_callback = std::bind(&BitflyerWallet::OnCreateCard,
        this,
        _1,
        _2,
        callback);
    ledger_->bitflyer()->CreateCard(card_callback);
    return;
  }

  if (user.verified) {
    ledger_->wallet()->ClaimFunds(callback);
    return;
  }

  callback(type::Result::LEDGER_OK);
}

void BitflyerWallet::OnCreateCard(
    const type::Result result,
    const std::string& address,
    ledger::ResultCallback callback) {
  auto wallet_ptr = GetWallet(ledger_);
  if (result != type::Result::LEDGER_OK || !wallet_ptr) {
    BLOG(0, "Card not created");
    callback(result);
    return;
  }

  wallet_ptr->address = address;
  wallet_ptr = GenerateLinks(std::move(wallet_ptr));
  ledger_->bitflyer()->SetWallet(wallet_ptr->Clone());

  if (wallet_ptr->status == type::WalletStatus::VERIFIED) {
    ledger_->wallet()->ClaimFunds(callback);
    return;
  }

  callback(type::Result::LEDGER_OK);
}

type::WalletStatus BitflyerWallet::GetNewStatus(
    const type::WalletStatus old_status,
    const User& user) {
  type::WalletStatus new_status = old_status;
  switch (old_status) {
    case type::WalletStatus::CONNECTED: {
      if (!user.verified) {
        break;
      }
      new_status = type::WalletStatus::VERIFIED;
      ledger_->ledger_client()->ShowNotification(
          "wallet_new_verified",
          {"Bitflyer"},
          [](type::Result _){});
      ledger_->database()->SaveEventLog(
          log::kWalletVerified,
          constant::kWalletBitflyer);
      break;
    }
    case type::WalletStatus::VERIFIED: {
      if (user.verified) {
        break;
      }

      new_status = type::WalletStatus::CONNECTED;
      break;
    }
    case type::WalletStatus::PENDING: {
      if (user.status != UserStatus::OK) {
        break;
      }

      if (user.verified) {
        new_status = type::WalletStatus::VERIFIED;
        ledger_->ledger_client()->ShowNotification(
            "wallet_new_verified",
            {"Bitflyer"},
            [](type::Result _){});
      } else {
        new_status = type::WalletStatus::CONNECTED;
      }
      break;
    }
    case type::WalletStatus::NOT_CONNECTED:
    case type::WalletStatus::DISCONNECTED_VERIFIED:
    case type::WalletStatus::DISCONNECTED_NOT_VERIFIED: {
      break;
    }
  }

  return new_status;
}

}  // namespace bitflyer
}  // namespace ledger
