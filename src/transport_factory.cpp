#include "netcore/transport_factory.hpp"

#include "netcore/http_transport.hpp"

#include "beast_http_transport.hpp"
#include "curl_http_transport.hpp"
#include "redirecting_transport.hpp"
#include "retrying_transport.hpp"

#include "beast_ws_transport.hpp"

#include "netcore/rate_limited_http_transport.hpp"

#include <boost/asio/io_context.hpp>
#include <print>


namespace NetCore {

    std::shared_ptr<IHttpTransport> make_http_transport(HttpTransportKind kind, TransportWrap wrap, RedirectPolicy redirect_policy, RetryPolicy retry_policy) {
        if (kind == HttpTransportKind::Curl && (wrap == TransportWrap::Redirect || wrap == TransportWrap::RedirectRatelimit || wrap == TransportWrap::RedirectRetry || wrap == TransportWrap::All))
            std::println("[TransportFactory] Warning: Curl automatically follows redirects. No need to wrap in redirecting transport");
        
        static boost::asio::io_context io;

        std::shared_ptr<IHttpTransport> base;

        switch (kind) {
        case HttpTransportKind::Beast:
            base = std::make_shared<BeastHttpTransport>(io.get_executor());
            break;
        case HttpTransportKind::Curl:
            base = std::make_shared<CurlHttpTransport>();
            break;
        }
        auto ratelimit_policy = std::make_shared<SimpleHeaderRateLimitPolicy>();

        switch (wrap) {
        case TransportWrap::None: break;
        case TransportWrap::RateLimit: {
            base = std::make_shared<RateLimitedTransport>(base, ratelimit_policy);
        } break;
        case TransportWrap::Redirect: {
            base = std::make_shared<RedirectingTransport>(base, redirect_policy);
        } break;
        case TransportWrap::Retry: {
            base = std::make_shared<RetryingTransport>(base, retry_policy);
        } break;
        case TransportWrap::RedirectRatelimit: {
            base = std::make_shared<RedirectingTransport>(base, redirect_policy);
            base = std::make_shared<RateLimitedTransport>(base, ratelimit_policy);
        } break;
        case TransportWrap::RetryRatelimit: {
            base = std::make_shared<RetryingTransport>(base, retry_policy);
            base = std::make_shared<RateLimitedTransport>(base, ratelimit_policy);
        } break;
        case TransportWrap::RedirectRetry: {
            base = std::make_shared<RedirectingTransport>(base, redirect_policy);
            base = std::make_shared<RetryingTransport>(base, retry_policy);
        } break;
        case TransportWrap::All: {
            base = std::make_shared<RedirectingTransport>(base, redirect_policy);
            base = std::make_shared<RetryingTransport>(base, retry_policy);
            base = std::make_shared<RateLimitedTransport>(base, ratelimit_policy);
        } break;
        }

        if (!base) {
            std::println("[TransportFactory] ERROR: Failed to create transport");
            return {};
        }

        return base;
    }

    std::shared_ptr<IWebSocketTransport> make_ws_transport(WsTransportKind kind) {
        static boost::asio::io_context io;
        
        switch (kind) {
        case WsTransportKind::Beast:
            return std::make_shared<NetCore::BeastWebSocketTransport>(io.get_executor());  
        }
        return {};
    }

} // namespace NetCore