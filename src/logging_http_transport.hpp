#pragma once

#include "netcore/http_transport.hpp"
#include <boost/asio/any_io_executor.hpp>

namespace NetCore {

    class LoggingHttpTransport : public IHttpTransport {
    public: 
        explicit LoggingHttpTransport(std::shared_ptr<IHttpTransport> inner);

        std::expected<HttpResponse, std::error_code> send_request(const HttpRequest& request) override;

    private:
        std::shared_ptr<IHttpTransport> m_Inner;
    };

} // namespace NetCore