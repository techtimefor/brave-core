/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVELEDGER_BITFLYER_BITFLYER_USER_H_
#define BRAVELEDGER_BITFLYER_BITFLYER_USER_H_

#include <memory>
#include <string>

#include "bat/ledger/ledger.h"

namespace ledger {
class LedgerImpl;

namespace endpoint {
class BitflyerServer;
}

namespace bitflyer {

enum UserStatus {
  EMPTY = 0,
  PENDING = 1,
  RESTRICTED = 2,
  BLOCKED = 3,
  OK = 4,
};

struct User {
  std::string name;
  std::string member_at;
  bool verified;
  UserStatus status;
  bool bat_not_allowed;

  User();
  ~User();
};

using GetUserCallback = std::function<void(const type::Result, const User&)>;

class BitflyerUser {
 public:
  explicit BitflyerUser(LedgerImpl* ledger);

  ~BitflyerUser();

  void Get(GetUserCallback callback);

 private:
  void OnGet(
      const type::Result result,
      const User& user,
      GetUserCallback callback);

  UserStatus GetStatus(const std::string& status);

  LedgerImpl* ledger_;  // NOT OWNED
  std::unique_ptr<endpoint::BitflyerServer> bitflyer_server_;
};

}  // namespace bitflyer
}  // namespace ledger
#endif  // BRAVELEDGER_BITFLYER_BITFLYER_USER_H_
