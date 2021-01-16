/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/payments/brave_payment_request_delegate.h"

#include <vector>

#include "base/check_op.h"
#include "base/memory/ref_counted.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/net/system_network_context_manager.h"
#include "chrome/browser/payments/payment_request_display_manager_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/identity_manager_factory.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/views/payments/payment_request_dialog_view.h"
#include "chrome/browser/ui/web_applications/app_browser_controller.h"
#include "chrome/browser/web_data_service_factory.h"
#include "components/autofill/content/browser/webauthn/internal_authenticator_impl.h"
#include "components/autofill/core/browser/address_normalizer_impl.h"
#include "components/autofill/core/browser/geo/region_data_loader_impl.h"
#include "components/autofill/core/browser/personal_data_manager.h"
#include "components/autofill/core/browser/ui/region_combobox_model.h"
#include "components/keyed_service/core/service_access_type.h"
#include "components/payments/content/payment_manifest_web_data_service.h"
#include "components/payments/content/payment_request.h"
#include "components/payments/content/payment_request_dialog.h"
#include "components/payments/content/ssl_validity_checker.h"
#include "components/payments/core/payment_prefs.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "services/metrics/public/cpp/ukm_recorder.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "third_party/libaddressinput/chromium/chrome_metadata_source.h"
#include "third_party/libaddressinput/chromium/chrome_storage_impl.h"

namespace payments {

BravePaymentRequestDelegate::BravePaymentRequestDelegate(
    content::RenderFrameHost* render_frame_host)
    : shown_dialog_(nullptr),
      frame_routing_id_(content::GlobalFrameRoutingId(
          render_frame_host->GetProcess()->GetID(),
          render_frame_host->GetRoutingID())) {}

BravePaymentRequestDelegate::~BravePaymentRequestDelegate() = default;

void BravePaymentRequestDelegate::ShowDialog(
    base::WeakPtr<PaymentRequest> request) {
  DCHECK_EQ(nullptr, shown_dialog_.get());
  DCHECK_EQ(nullptr, spc_dialog_.get());

  if (request->spec() &&                                                       
      request->spec()->stringified_method_data().count("bat") > 0) {           
    shown_dialog_ = BatPaymentRequestDialogView::Create(request, nullptr);
    shown_dialog_->ShowDialog();                 
    return;                                                                  
  }

  switch (dialog_type_) {
    case DialogType::PAYMENT_REQUEST:
      shown_dialog_ = PaymentRequestDialogView::Create(request, nullptr);
      break;
    case DialogType::SECURE_PAYMENT_CONFIRMATION:
      spc_dialog_ =
          std::make_unique<SecurePaymentConfirmationController>(request);
      shown_dialog_ = spc_dialog_->GetWeakPtr();
      break;
  }
  shown_dialog_->ShowDialog();
}

}  // namespace payments