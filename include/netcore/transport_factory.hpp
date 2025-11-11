#pragma once

#include <memory>
#include "http_transport.hpp"

namespace NetCore {

    enum class TransportKind {
        Beast,
        Curl,
        CppHttp,
    };

    enum class TransportWrap {
        None,
        Logging,
    };

    std::shared_ptr<IHttpTransport> make_http_transport(TransportKind kind, TransportWrap wrap = TransportWrap::None);

} // namespace NetCore