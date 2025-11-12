#pragma once

#include <system_error>

namespace NetCore {

    enum class errc {
        ok = 0,
        invalid_url,
        connect_failed,
        tls_failed,
        write_failed,
        read_failed,
        http_protocol_error,
        unsupported_scheme,
        not_connected,
        timed_out,
    };

    const std::error_category& error_category();

    inline std::error_code make_error_code(errc e) noexcept {
        return {static_cast<int>(e), error_category()};
    }

} // namespace NetCore

namespace std {
    template<>
    struct is_error_code_enum<NetCore::errc> : true_type {};
} // namespace std