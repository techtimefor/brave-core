/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_DOMAIN_BLOCKED_NAVIGATION_THROTTLE_H_
#define BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_DOMAIN_BLOCKED_NAVIGATION_THROTTLE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "content/public/browser/navigation_throttle.h"
#include "url/gurl.h"

namespace content {
class NavigationHandle;
class WebContents;
}  // namespace content

class PrefService;

namespace brave_shields {

class AdBlockService;

class DomainBlockedNavigationThrottle : public content::NavigationThrottle {
 public:
  explicit DomainBlockedNavigationThrottle(
      content::NavigationHandle* navigation_handle,
      AdBlockService* ad_block_service,
      const std::string& locale);
  ~DomainBlockedNavigationThrottle() override;

  DomainBlockedNavigationThrottle(const DomainBlockedNavigationThrottle&) =
      delete;
  DomainBlockedNavigationThrottle& operator=(
      const DomainBlockedNavigationThrottle&) = delete;

  static std::unique_ptr<DomainBlockedNavigationThrottle>
  MaybeCreateThrottleFor(content::NavigationHandle* navigation_handle,
                         AdBlockService* ad_block_service,
                         const std::string& locale);

  // content::NavigationThrottle implementation:
  content::NavigationThrottle::ThrottleCheckResult WillStartRequest() override;
  const char* GetNameForLogging() override;

 private:
  void OnShouldBlockDomain(bool should_block_domain);
  void ShowInterstitial();

  AdBlockService* ad_block_service_ = nullptr;
  PrefService* pref_service_ = nullptr;
  std::string locale_;
  base::WeakPtrFactory<DomainBlockedNavigationThrottle> weak_ptr_factory_{this};
};

}  // namespace brave_shields

#endif  // BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_DOMAIN_BLOCKED_NAVIGATION_THROTTLE_H_
