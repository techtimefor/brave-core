/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_INTERSTITIAL_CONTROLLER_CLIENT_H_
#define BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_INTERSTITIAL_CONTROLLER_CLIENT_H_

#include <memory>
#include <string>

#include "components/security_interstitials/content/security_interstitial_controller_client.h"
#include "url/gurl.h"

namespace content {
class WebContents;
}  // namespace content

namespace security_interstitials {
class MetricsHelper;
}  // namespace security_interstitials

namespace brave_shields {

class DomainBlockedInterstitialControllerClient
    : public security_interstitials::SecurityInterstitialControllerClient {
 public:
  static std::unique_ptr<security_interstitials::MetricsHelper>
  GetMetricsHelper(const GURL& url);

  DomainBlockedInterstitialControllerClient(content::WebContents* web_contents,
                                            const GURL& request_url,
                                            PrefService* prefs,
                                            const std::string& locale);
  ~DomainBlockedInterstitialControllerClient() override = default;

  DomainBlockedInterstitialControllerClient(
      const DomainBlockedInterstitialControllerClient&) = delete;
  DomainBlockedInterstitialControllerClient& operator=(
      const DomainBlockedInterstitialControllerClient&) = delete;

  // security_interstitials::SecurityInterstitialControllerClient:
  void Proceed() override;

 private:
  const GURL request_url_;
};

}  // namespace brave_shields

#endif  // BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_INTERSTITIAL_CONTROLLER_CLIENT_H_
