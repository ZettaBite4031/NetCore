#include "logging_http_transport.hpp"

#include <type_traits>
#include <print>

namespace NetCore {

    LoggingHttpTransport::LoggingHttpTransport(std::shared_ptr<IHttpTransport> inner) 
        : m_Inner(std::move(inner)) {
        static_assert(!std::is_same<LoggingHttpTransport, std::decay_t<decltype(*inner)>>::value,
                      "LoggingHttpTransports cannot be wrapped inside another!");
    }

    std::expected<HttpResponse, std::error_code> LoggingHttpTransport::send_request(const HttpRequest& request) {
        std::println("[HTTP] {} {}", request.method, request.url);
        auto res = m_Inner->send_request(request);
        if (res) std::println("[HTTP] -> {}", res->status);
        else std::println("[HTTP] ERROR: {}", res.error().message());
        return res;
    }

} // namespace Netcore