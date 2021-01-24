/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_LEDGER_RUNTIME_H_
#define BAT_LEDGER_RUNTIME_H_

#include <sstream>

#include "base/location.h"
#include "base/memory/weak_ptr.h"
#include "bat/ledger/internal/async_result.h"
#include "brave/vendor/bat-native-ledger/include/bat/ledger/public/interfaces/ledger.mojom.h"

namespace ledger {

class LedgerClient;

// TODO(zenparsing): Some of the "levels" don't really make a whole lot
// of sense. We should review. Can we just have Error, Info, Verbose, and
// Full? We should remove the other log levels and reset these number.
enum class LogLevel { kError = 0, kInfo = 1, kVerbose = 6, kFull = 9 };

class Runtime {
 public:
  explicit Runtime(LedgerClient* ledger_client);

  Runtime(const Runtime&) = delete;
  Runtime& operator=(const Runtime&) = delete;

  ~Runtime();

  class LogStream {
   public:
    LogStream(Runtime* runtime, base::Location location, LogLevel log_level);

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    LogStream(LogStream&& other);
    LogStream& operator=(LogStream&& other);

    ~LogStream();

    template <typename T>
    LogStream& operator<<(const T& value) {
      stream_ << value;
      return *this;
    }

   private:
    Runtime* runtime_;
    base::Location location_;
    LogLevel log_level_;
    std::ostringstream stream_;
    bool moved_ = false;
  };

  base::WeakPtr<Runtime> GetRef() { return weak_factory_.GetWeakPtr(); }

  LogStream Log(base::Location location, LogLevel log_level);
  LogStream LogError(base::Location location);
  LogStream LogInfo(base::Location location);
  LogStream LogVerbose(base::Location location);
  LogStream LogFull(base::Location location);

  AsyncResult<mojom::UrlResponse> LoadURL(mojom::UrlRequestPtr request);

 private:
  LedgerClient* ledger_client_;
  base::WeakPtrFactory<Runtime> weak_factory_{this};
};

using RuntimeRef = base::WeakPtr<Runtime>;

}  // namespace ledger

#endif  // BAT_LEDGER_RUNTIME_H_
