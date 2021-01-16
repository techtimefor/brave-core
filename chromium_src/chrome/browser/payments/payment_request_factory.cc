/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/payments/payment_request_factory.h"

#include "brave/browser/payments/brave_payment_request_delegate.h"

#define ChromePaymentRequestDelegate BravePaymentRequestDelegate
#include "../../../../../chrome/browser/payments/payment_request_factory.cc"
#undef ChromePaymentRequestDelegate