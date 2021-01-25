/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/ipfs/translate_ipfs_uri.h"

#include <string>

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "brave/components/ipfs/ipfs_constants.h"
#include "brave/components/ipfs/ipfs_gateway.h"

namespace ipfs {

bool TranslateIPFSURI(const GURL& url,
                      GURL* new_url,
                      const GURL& gateway_url,
                      bool use_subdomain) {
  if (!url.SchemeIs(kIPFSScheme) && !url.SchemeIs(kIPNSScheme)) {
    return false;
  }

  std::string path = url.path();
  // In the case of a URL like ipfs://[cid]/wiki/Vincent_van_Gogh.html
  // host is empty and path is //wiki/Vincent_van_Gogh.html
  if (url.host().empty() && path.length() > 2 && path.substr(0, 2) == "//") {
    std::string cid(path.substr(2));
    // If we have a path after the CID, get at the real resource path
    size_t pos = cid.find("/");
    std::string path;
    if (pos != std::string::npos && pos != 0) {
      // path would be /wiki/Vincent_van_Gogh.html
      path = cid.substr(pos, cid.length() - pos);
      // cid would be [cid]
      cid = cid.substr(0, pos);
    }
    bool ipfs_scheme = url.scheme() == kIPFSScheme;
    bool ipns_scheme = url.scheme() == kIPNSScheme;
    if ((ipfs_scheme && std::all_of(cid.begin(), cid.end(),
                                    [loc = std::locale{}](char c) {
                                      return std::isalnum(c, loc);
                                    })) ||
        ipns_scheme) {
      // new_url would be:
      // https://dweb.link/ipfs/[cid]//wiki/Vincent_van_Gogh.html
      if (new_url) {
        if (use_subdomain) {
          GURL uri(gateway_url);
          GURL::Replacements replacements;
          std::string host = base::StringPrintf("%s.%s.%s", cid.c_str(),
                                                ipfs_scheme ? "ipfs" : "ipns",
                                                gateway_url.host().c_str());
          replacements.SetHostStr(host);
          replacements.SetPathStr(path);
          *new_url = gateway_url.ReplaceComponents(replacements);
        } else {
          std::string gateway_empty_path = gateway_url.spec();
          if (gateway_empty_path.size() > 0 &&
              gateway_empty_path[gateway_empty_path.size() - 1] == '/') {
            gateway_empty_path.pop_back();
          }
          *new_url = GURL(gateway_empty_path +
                          (ipfs_scheme ? "/ipfs/" : "/ipns/") + cid + path);
        }
        VLOG(1) << "[IPFS] " << __func__ << " new URL: " << *new_url;
      }

      return true;
    }
  }
  return false;
}

}  // namespace ipfs
