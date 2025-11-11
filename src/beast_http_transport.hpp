#pragma once

#include "netcore/http_transport.hpp"
#include <boost/asio/any_io_executor.hpp>

namespace NetCore {

    class BeastHttpTransport : public IHttpTransport {
    public:
        explicit BeastHttpTransport(boost::asio::any_io_executor exec);

        std::expected<HttpResponse, std::error_code> send_request(const HttpRequest& request) override;

    private:
        boost::asio::any_io_executor m_Executor;
    };

} // namespace NetCore