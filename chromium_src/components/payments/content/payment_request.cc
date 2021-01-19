/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "components/payments/content/payment_request.h"

namespace payments {

void PaymentRequest::OnPaymentResponseAvailable(mojom::PaymentResponsePtr response) {
  if (response->method_name == "bat") {
    client_->OnPaymentResponse(std::move(response));
    return;
  }
  PaymentRequest::OnPaymentResponseAvailable_ChromiumImpl(std::move(response));
}

}  // namespace payments

#define OnPaymentResponseAvailable OnPaymentResponseAvailable_ChromiumImpl
#include "../../../../../components/payments/content/payment_request.cc"
#undef OnPaymentResponseAvailable
