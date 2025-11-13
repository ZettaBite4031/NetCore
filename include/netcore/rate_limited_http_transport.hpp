#pragma once

#include "http_transport.hpp"
#include "rate_limit.hpp"

#include <memory>
#include <mutex>
#include <condition_variable>
#include <unordered_map>


namespace NetCore {

    class RateLimitedTransport : public IHttpTransport {
    public:
        RateLimitedTransport(std::shared_ptr<IHttpTransport> inner, std::shared_ptr<IRateLimitPolicy> policy)
            : m_Inner(std::move(inner)), m_Policy(std::move(policy)) {}

        std::expected<HttpResponse, std::error_code> send_request(const HttpRequest& request, const RequestOptions& opt) override;

    private:
        std::shared_ptr<IHttpTransport> m_Inner;
        std::shared_ptr<IRateLimitPolicy> m_Policy;

        std::mutex m_Mtx;
        std::condition_variable m_CV;
    };

} // namespace NetCore