/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_PAYMENTS_CHROME_PAYMENT_REQUEST_DELEGATE_H_
#define BRAVE_BROWSER_PAYMENTS_CHROME_PAYMENT_REQUEST_DELEGATE_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/payments/chrome_payment_request_delegate.h"
#include "components/payments/content/content_payment_request_delegate.h"
#include "components/payments/content/secure_payment_confirmation_controller.h"
#include "content/public/browser/global_routing_id.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace payments {

class PaymentRequestDialog;
class PaymentUIObserver;

class BravePaymentRequestDelegate : public ChromePaymentRequestDelegate {
 public:
  explicit BravePaymentRequestDelegate(
      content::RenderFrameHost* render_frame_host);
  ~BravePaymentRequestDelegate() override;

  // PaymentRequestDelegate:
  void ShowDialog(base::WeakPtr<PaymentRequest> request) override;

  DISALLOW_COPY_AND_ASSIGN(BravePaymentRequestDelegate);
};

}  // namespace payments

#endif  // BRAVE_BROWSER_PAYMENTS_CHROME_PAYMENT_REQUEST_DELEGATE_H_
