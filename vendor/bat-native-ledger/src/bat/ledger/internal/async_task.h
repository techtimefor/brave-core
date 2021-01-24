/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_LEDGER_ASYNC_TASK_H_
#define BAT_LEDGER_ASYNC_TASK_H_

#include <memory>
#include <utility>

#include "bat/ledger/internal/async_task.h"

namespace ledger {

template <typename T>
class AsyncTask {
 public:
  using Result = T::ResultType;
  using State = T::State;

  template <typename... Args>
  Task(Args&&... args) : state_(std::forward(args)...) {}

  State& operator*() const { return *state_; }
  State* operator->() const { return state_.get(); }

  void Complete(Result::CompleteType&& value) {
    resolver_.Complete(std::move(value));
  }

  void Error(Result::ErrorType&& error) { resolver_.Error(std::move(error)); }

 private:
  std::shared_ptr<State> state_;
  Result::Resolver resolver_;
};

}  // namespace ledger

#endif  // BAT_LEDGER_ASYNC_TASK_H_
