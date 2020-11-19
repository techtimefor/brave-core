/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/domain_blocked_page.h"

#include <utility>

#include "base/notreached.h"
#include "base/strings/string_number_conversions.h"
#include "components/grit/brave_components_resources.h"
#include "components/grit/brave_components_strings.h"
#include "components/security_interstitials/content/security_interstitial_controller_client.h"
#include "ui/base/l10n/l10n_util.h"

namespace brave_shields {

// static
const security_interstitials::SecurityInterstitialPage::TypeID
    DomainBlockedPage::kTypeForTesting = &DomainBlockedPage::kTypeForTesting;

DomainBlockedPage::DomainBlockedPage(
    content::WebContents* web_contents,
    const GURL& request_url,
    std::unique_ptr<
        security_interstitials::SecurityInterstitialControllerClient>
        controller)
    : security_interstitials::SecurityInterstitialPage(web_contents,
                                                       request_url,
                                                       std::move(controller)) {}

DomainBlockedPage::~DomainBlockedPage() = default;

void DomainBlockedPage::CommandReceived(const std::string& command) {
  if (command == "\"pageLoadComplete\"") {
    // content::WaitForRenderFrameReady sends this message when the page
    // load completes. Ignore it.
    return;
  }

  int cmd = 0;
  bool retval = base::StringToInt(command, &cmd);
  DCHECK(retval);

  switch (cmd) {
    case security_interstitials::CMD_PROCEED:
      controller()->Proceed();
      break;
    default:
      NOTREACHED() << "Unsupported command: " << command;
  }
}

void DomainBlockedPage::PopulateInterstitialStrings(
    base::DictionaryValue* load_time_data) {
  load_time_data->SetString(
      "tabTitle", l10n_util::GetStringUTF16(IDS_DOMAIN_BLOCKED_TITLE));
  load_time_data->SetString(
      "heading", l10n_util::GetStringUTF16(IDS_DOMAIN_BLOCKED_HEADING));

  load_time_data->SetString(
      "primaryParagraph",
      l10n_util::GetStringUTF16(IDS_DOMAIN_BLOCKED_PRIMARY_PARAGRAPH));

#if 0 // TODO
  load_time_data->SetString(
      "primaryButtonText",
      l10n_util::GetStringUTF16(IDS_DOMAIN_BLOCKED_PRIMARY_BUTTON));
  load_time_data->SetString(
      "openDetails",
      l10n_util::GetStringUTF16(IDS_DOMAIN_BLOCKED_OPEN_DETAILS_BUTTON));
  load_time_data->SetString(
      "closeDetails",
      l10n_util::GetStringUTF16(IDS_DOMAIN_BLOCKED_CLOSE_DETAILS_BUTTON));
#endif
  load_time_data->SetString(
      "explanationParagraph",
      l10n_util::GetStringUTF16(IDS_DOMAIN_BLOCKED_EXPLANATION));
  load_time_data->SetString("finalParagraph", base::string16());
}

int DomainBlockedPage::GetHTMLTemplateId() {
  return IDR_DOMAIN_BLOCKED_INTERSTITIAL_HTML;
}

security_interstitials::SecurityInterstitialPage::TypeID
DomainBlockedPage::GetTypeForTesting() {
  return DomainBlockedPage::kTypeForTesting;
}

}  // namespace brave_shields
