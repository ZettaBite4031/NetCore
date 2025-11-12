#pragma once 
#include <expected>
#include <system_error>
#include "http_types.hpp"

namespace NetCore {
    
    class IHttpTransport {
    public:
        virtual ~IHttpTransport() = default;

        virtual std::expected<HttpResponse, std::error_code> send_request(const HttpRequest& request, const RequestOptions& opt) = 0;
    };

} // namespace NetCore