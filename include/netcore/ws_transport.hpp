#pragma once

#include <expected>
#include <string>
#include <system_error>

namespace NetCore {

    class IWebSocketTransport {
    public:
        virtual ~IWebSocketTransport() = default;

        virtual std::error_code connect(std::string_view uri) = 0;
        virtual std::error_code send_text(std::string_view message) = 0;
        virtual std::expected<std::string, std::error_code> receive_text() = 0;
        virtual void close() = 0;
        virtual void reset() = 0;

        virtual std::error_code ping() { return std::make_error_code(std::errc::operation_not_supported); }
    };

}; // namespace NetCore