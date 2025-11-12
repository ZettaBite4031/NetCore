#pragma once

#include "http_transport.hpp"
#include "http_types.hpp"
#include <memory>
#include <expected>
#include <system_error>
#include <string>

namespace NetCore {

    class WebhookClient {
    public:
        explicit WebhookClient(std::shared_ptr<IHttpTransport> http, ClientConfig cfg);

        std::expected<void, std::error_code> post_json(std::string url, std::string json, const RequestOptions& opt);

    private:
        RequestOptions effective(const RequestOptions& per_call) const;
        
        std::shared_ptr<IHttpTransport> m_Transport;
        ClientConfig m_Cfg;
    };

} // namespace NetCore