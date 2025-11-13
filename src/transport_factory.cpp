#include "netcore/transport_factory.hpp"

#include "netcore/http_transport.hpp"

#include "beast_http_transport.hpp"
#include "curl_http_transport.hpp"

#include "beast_ws_transport.hpp"

#include "netcore/rate_limited_http_transport.hpp"

#include <boost/asio/io_context.hpp>
#include <print>

namespace NetCore {

    std::shared_ptr<IHttpTransport> make_http_transport(HttpTransportKind kind, TransportWrap wrap) {
        (void)(wrap); // unused
        
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

        switch (wrap) {
        case TransportWrap::None: break;
        case TransportWrap::RateLimit:
            auto policy = std::make_shared<SimpleHeaderRateLimitPolicy>();
            base = std::make_shared<RateLimitedTransport>(base, policy);
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