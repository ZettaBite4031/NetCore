#pragma once

#include <memory>
#include "http_transport.hpp"
#include "ws_transport.hpp"

namespace NetCore {

    enum class HttpTransportKind {
        Beast,
        Curl,
    };

    enum class WsTransportKind {
        Beast,
    };

    enum class TransportWrap {
        None,
        Logging,
    };

    std::shared_ptr<IHttpTransport> make_http_transport(HttpTransportKind kind, TransportWrap wrap = TransportWrap::None);
    std::shared_ptr<IWebSocketTransport> make_ws_transport(WsTransportKind kind);

} // namespace NetCore