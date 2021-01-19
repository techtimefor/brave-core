/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_COMPONENTS_PAYMENTS_CONTENT_PAYMENT_REQUEST_H_
#define BRAVE_CHROMIUM_SRC_COMPONENTS_PAYMENTS_CONTENT_PAYMENT_REQUEST_H_

#include "components/payments/content/payment_request_state.h"
#include "third_party/blink/public/mojom/payments/payment_request.mojom.h"

#define OnPaymentResponseAvailable                                              \
  OnPaymentResponseAvailable_ChromiumImpl(mojom::PaymentResponsePtr response);  \
  virtual void OnPaymentResponseAvailable
#include "../../../../../components/payments/content/payment_request.h"
#undef OnPaymentResponseAvailable

#endif  // BRAVE_CHROMIUM_SRC_COMPONENTS_PAYMENTS_CONTENT_PAYMENT_REQUEST_H_
