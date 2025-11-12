#include "netcore/transport_factory.hpp"

#include "netcore/http_transport.hpp"

#include "beast_http_transport.hpp"
#include "curl_http_transport.hpp"
#include "logging_http_transport.hpp"

#include "beast_ws_transport.hpp"

#include <boost/asio/io_context.hpp>
#include <print>

namespace NetCore {

    std::shared_ptr<IHttpTransport> make_http_transport(HttpTransportKind kind, TransportWrap wrap) {
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

        if (!base) {
            std::println("[TransportFactory] ERROR: Failed to create transport");
            return {};
        }

        if (wrap == TransportWrap::Logging) {
            base = std::make_shared<LoggingHttpTransport>(base);
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