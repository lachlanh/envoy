#include "extensions/filters/http/tap/tap_config_impl.h"

#include "common/common/assert.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace TapFilter {

HttpTapConfigImpl::HttpTapConfigImpl(envoy::service::tap::v2alpha::TapConfig&& proto_config,
                                     TapSink& admin_streamer)
    : TapConfigBaseImpl(std::move(proto_config)), admin_streamer_(admin_streamer) {
  // The streaming admin output sink is the only currently supported sink.
  ASSERT(proto_config_.output_config().sinks()[0].has_streaming_admin());
  for (const auto& header_match :
       proto_config_.match_config().http_match_config().request_match_config().headers()) {
    request_headers_to_match_.emplace_back(header_match);
  }

  for (const auto& header_match :
       proto_config_.match_config().http_match_config().response_match_config().headers()) {
    response_headers_to_match_.emplace_back(header_match);
  }
}

bool HttpTapConfigImpl::matchesRequestHeaders(const Http::HeaderMap& headers) {
  return !request_headers_to_match_.empty() &&
         Http::HeaderUtility::matchHeaders(headers, request_headers_to_match_);
}

bool HttpTapConfigImpl::matchesResponseHeaders(const Http::HeaderMap& headers) {
  return !response_headers_to_match_.empty() &&
         Http::HeaderUtility::matchHeaders(headers, response_headers_to_match_);
}

HttpPerRequestTapperPtr HttpTapConfigImpl::newPerRequestTapper() {
  return std::make_unique<HttpPerRequestTapperImpl>(shared_from_this());
}

void HttpPerRequestTapperImpl::onRequestHeaders(const Http::HeaderMap& headers) {
  request_headers_ = &headers;
  if (config_->matchesRequestHeaders(headers)) {
    tapping_ = true;
  }
}

void HttpPerRequestTapperImpl::onResponseHeaders(const Http::HeaderMap& headers) {
  if (config_->matchesResponseHeaders(headers)) {
    ASSERT(false); // fixfix
  }

  // fixfix comment
  if (tapping_) {
    buildBufferedTrace(*request_headers_, headers);
  }
}

void HttpPerRequestTapperImpl::buildBufferedTrace(const Http::HeaderMap& request_headers,
                                                  const Http::HeaderMap& response_headers) {}

} // namespace TapFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
