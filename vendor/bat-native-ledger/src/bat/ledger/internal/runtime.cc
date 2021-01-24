/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/runtime.h"

#include <array>
#include <string>
#include <utility>

#include "base/strings/string_util.h"
#include "bat/ledger/ledger_client.h"

namespace ledger {

using LogStream = Runtime::LogStream;

namespace {

constexpr size_t kMaxResponseBodySizeForLogging = 128;

constexpr std::array<const char*, 4> kRequestHeadersForLogging = {
    "digest", "signature", "accept", "content-type"};

void LogURLRequest(LogStream&& stream, const mojom::UrlRequest& request) {
  stream << "\n[ REQUEST ]";
  stream << "\n> URL: " << request.url;
  stream << "\n> Method: " << request.method;

  if (!request.content.empty())
    stream << "\n> Content: " << request.content;

  if (!request.content_type.empty())
    stream << "\n> Content-Type: " << request.content_type;

  for (const std::string& header : request.headers) {
    for (const char* name : kRequestHeadersForLogging) {
      if (base::StartsWith(header, name, base::CompareCase::INSENSITIVE_ASCII))
        stream << "\n> Header " << header;
    }
  }
}

void LogURLResponse(LogStream&& stream, const mojom::UrlResponse& response) {
  std::string result;
  if (!response.error.empty())
    result = "Error (" + response.error + ")";
  else if (response.status_code >= 200 && response.status_code < 300)
    result = "Success";
  else
    result = "Failure";

  stream << "\n[ RESPONSE ]";
  stream << "\n> URL: " << response.url;
  stream << "\n> Result: " << result;
  stream << "\n> HTTP Status: " << response.status_code;

  if (response.body.size() <= kMaxResponseBodySizeForLogging)
    stream << "\n> Body: " << response.body;
}

void LogURLResponseDetails(LogStream&& stream,
                           const mojom::UrlResponse& response) {
  stream << "\n[ RESPONSE DETAILS ]"
         << "\n> URL: " << response.url;

  if (response.body.size() > kMaxResponseBodySizeForLogging)
    stream << "\n> Body:\n" << response.body;

  for (auto& header : response.headers)
    stream << "\n> Header: " << header.first << " = " << header.second;
}

}  // namespace

Runtime::Runtime(LedgerClient* ledger_client) : ledger_client_(ledger_client) {}

Runtime::~Runtime() = default;

LogStream::LogStream(Runtime* runtime,
                     base::Location location,
                     LogLevel log_level)
    : runtime_(runtime), location_(location), log_level_(log_level) {}

LogStream::LogStream(LogStream&& other)
    : runtime_(other.runtime_),
      location_(other.location_),
      log_level_(other.log_level_),
      stream_(std::move(other.stream_)) {
  other.moved_ = true;
}

LogStream& LogStream::operator=(LogStream&& other) {
  runtime_ = other.runtime_;
  location_ = other.location_;
  log_level_ = other.log_level_;
  stream_ = std::move(other.stream_);

  other.moved_ = true;
  return *this;
}

LogStream::~LogStream() {
  if (!moved_) {
    runtime_->ledger_client_->Log(location_.file_name(),
                                  location_.line_number(),
                                  static_cast<int>(log_level_), stream_.str());
  }
}

LogStream Runtime::Log(base::Location location, LogLevel log_level) {
  return LogStream(this, location, log_level);
}

LogStream Runtime::LogError(base::Location location) {
  return LogStream(this, location, LogLevel::kError);
}

LogStream Runtime::LogInfo(base::Location location) {
  return LogStream(this, location, LogLevel::kInfo);
}

LogStream Runtime::LogVerbose(base::Location location) {
  return LogStream(this, location, LogLevel::kVerbose);
}

LogStream Runtime::LogFull(base::Location location) {
  return LogStream(this, location, LogLevel::kFull);
}

AsyncResult<mojom::UrlResponse> Runtime::LoadURL(mojom::UrlRequestPtr request) {
  DCHECK(request);
  // TODO(zenparsing): Examine usage of skip_log and document. If there is a
  // security issue, then choose a different name and reverse the default.
  if (!request->skip_log)
    LogURLRequest(this->LogVerbose(FROM_HERE), *request);

  AsyncResult<mojom::UrlResponse>::Resolver resolver;
  ledger_client_->LoadURL(
      std::move(request),
      [ref = GetRef(), resolver](const mojom::UrlResponse& response) mutable {
        LogURLResponse(ref->LogVerbose(FROM_HERE), response);
        LogURLResponseDetails(ref->LogFull(FROM_HERE), response);
        resolver.Complete(mojom::UrlResponse(response));
      });
  return resolver.result();
}

}  // namespace ledger
