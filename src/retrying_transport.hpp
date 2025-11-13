#pragma once

#include "netcore/http_transport.hpp"
#include "netcore/err.hpp"

#include <chrono>
#include <memory>
#include <expected>


namespace NetCore {

    class RetryingTransport : public IHttpTransport {
    public:
        RetryingTransport(std::shared_ptr<IHttpTransport> inner, RetryPolicy policy = {});

        std::expected<HttpResponse, std::error_code> send_request(const HttpRequest& req, const RequestOptions& opt) override;
        
    private:
        std::shared_ptr<IHttpTransport> m_Inner;
        RetryPolicy m_Policy;

        static bool is_network_error(const std::error_code& ec);
        static bool is_retriable_status(int status, const RetryPolicy& p);
    };

} // namespace NetCore