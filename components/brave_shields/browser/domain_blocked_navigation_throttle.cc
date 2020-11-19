/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/domain_blocked_navigation_throttle.h"

#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/task/post_task.h"
#include "base/threading/thread_task_runner_handle.h"
#include "brave/components/brave_shields/browser/ad_block_service.h"
#include "brave/components/brave_shields/browser/domain_blocked_interstitial_controller_client.h"
#include "brave/components/brave_shields/browser/domain_blocked_page.h"
#include "components/prefs/pref_service.h"
#include "components/security_interstitials/content/security_interstitial_tab_helper.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_user_data.h"
#include "net/base/net_errors.h"

namespace {

bool ShouldBlockDomainOnTaskRunner(
    brave_shields::AdBlockService* ad_block_service,
    const GURL& url) {
  bool did_match_exception = false;
  std::string mock_data_url;
  return !ad_block_service->ShouldStartRequest(
      url, blink::mojom::ResourceType::kMainFrame, url.host(),
      &did_match_exception, &mock_data_url);
}

}  // namespace

namespace brave_shields {

// static
std::unique_ptr<DomainBlockedNavigationThrottle>
DomainBlockedNavigationThrottle::MaybeCreateThrottleFor(
    content::NavigationHandle* navigation_handle,
    AdBlockService* ad_block_service,
    const std::string& locale) {
  if (!ad_block_service)
    return nullptr;
  return std::make_unique<DomainBlockedNavigationThrottle>(
      navigation_handle, ad_block_service, locale);
}

DomainBlockedNavigationThrottle::DomainBlockedNavigationThrottle(
    content::NavigationHandle* navigation_handle,
    AdBlockService* ad_block_service,
    const std::string& locale)
    : content::NavigationThrottle(navigation_handle),
      ad_block_service_(ad_block_service),
      locale_(locale) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  content::BrowserContext* context =
      navigation_handle->GetWebContents()->GetBrowserContext();
  pref_service_ = user_prefs::UserPrefs::Get(context);
}

DomainBlockedNavigationThrottle::~DomainBlockedNavigationThrottle() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

content::NavigationThrottle::ThrottleCheckResult
DomainBlockedNavigationThrottle::WillStartRequest() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (0) {
    // TODO main frame checks and other things we can do on this thread
    return content::NavigationThrottle::PROCEED;
  }
  GURL url = navigation_handle()->GetURL();
  ad_block_service_->GetTaskRunner()->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(&ShouldBlockDomainOnTaskRunner, ad_block_service_, url),
      base::BindOnce(&DomainBlockedNavigationThrottle::OnShouldBlockDomain,
                     weak_ptr_factory_.GetWeakPtr()));
  return content::NavigationThrottle::DEFER;
}

void DomainBlockedNavigationThrottle::OnShouldBlockDomain(
    bool should_block_domain) {
  if (should_block_domain) {
    Resume();
  } else {
    ShowInterstitial();
    CancelDeferredNavigation(content::NavigationThrottle::CANCEL);
  }
}

void DomainBlockedNavigationThrottle::ShowInterstitial() {
  content::NavigationHandle* handle = navigation_handle();
  content::WebContents* web_contents = handle->GetWebContents();
  const GURL& request_url = handle->GetURL();

  auto controller_client =
      std::make_unique<DomainBlockedInterstitialControllerClient>(
          web_contents, request_url, pref_service_, locale_);
  auto blocked_page = std::make_unique<DomainBlockedPage>(
      web_contents, handle->GetURL(), std::move(controller_client));

  // Get the page content before giving up ownership of |blocked_page|.
  std::string blocked_page_content = blocked_page->GetHTMLContents();

  security_interstitials::SecurityInterstitialTabHelper::AssociateBlockingPage(
      web_contents, handle->GetNavigationId(), std::move(blocked_page));

  CancelDeferredNavigation(content::NavigationThrottle::ThrottleCheckResult(
      content::NavigationThrottle::CANCEL, net::ERR_BLOCKED_BY_CLIENT,
      blocked_page_content));
}

const char* DomainBlockedNavigationThrottle::GetNameForLogging() {
  return "DomainBlockedNavigationThrottle";
}

}  // namespace brave_shields
