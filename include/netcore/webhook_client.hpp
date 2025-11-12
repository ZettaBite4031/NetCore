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
        explicit WebhookClient(std::shared_ptr<IHttpTransport> http) 
            : m_Transport(std::move(http)) {}

        std::expected<void, std::error_code> post_json(std::string url, std::string json) {
            HttpRequest req;
            req.method = "POST";
            req.url = std::move(url);
            req.body = std::move(json);
            req.headers.push_back({"Content-Type", "application/json"});

            auto res = m_Transport->send_request(req);
            if (!res) return std::unexpected(res.error());
            if (res->status / 100 != 2) 
                return std::unexpected(std::make_error_code(std::errc::io_error));
            return {};
        }

    private:
        std::shared_ptr<IHttpTransport> m_Transport;
    };

} // namespace NetCore