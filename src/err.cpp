#include "netcore/err.hpp"

namespace NetCore {

    class netcore_error_category : public std::error_category {
    public:
        const char* name() const noexcept override {
            return "netcore";
        }

        std::string message(int ev) const override {
            switch (static_cast<errc>(ev)) {
            case errc::ok:                  return "no error";
            case errc::invalid_url:         return "invalid url";
            case errc::connect_failed:      return "connect failed";
            case errc::tls_failed:          return "TLS/SSL handshake failed";
            case errc::write_failed:        return "write failed";
            case errc::read_failed:         return "read failed";
            case errc::http_protocol_error: return "HTTP protocol error";
            case errc::unsupported_scheme:  return "unsupported URL scheme";
            case errc::not_connected:       return "not connected";
            case errc::timed_out:           return "connection timed out";
            }
            return "unknown netcore error";
        }
    };

    const std::error_category& error_category() {
        static netcore_error_category cat;
        return cat;
    }

} // namespace NetCore