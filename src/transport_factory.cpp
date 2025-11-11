#include "netcore/transport_factory.hpp"

#include "netcore/beast_http_transport.hpp"
#include "netcore/curl_http_transport.hpp"
#include "netcore/logging_http_transport.hpp"
#include <boost/asio/io_context.hpp>
#include <print>

namespace NetCore {

    std::shared_ptr<IHttpTransport> make_http_transport(TransportKind kind, TransportWrap wrap) {
        static boost::asio::io_context io;

        std::shared_ptr<IHttpTransport> base;

        switch (kind) {
        case TransportKind::Beast:
            base = std::make_shared<BeastHttpTransport>(io.get_executor());
            break;
        case TransportKind::Curl:
            base = std::make_shared<CurlHttpTransport>();
            break;
        case TransportKind::CppHttp:
            
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

} // namespace NetCore