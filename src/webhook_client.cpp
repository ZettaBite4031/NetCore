#include "netcore/webhook_client.hpp"

namespace NetCore {

    WebhookClient::WebhookClient(std::shared_ptr<IHttpTransport> http) 
            : m_Transport(std::move(http)) {}

    std::expected<void, std::error_code> WebhookClient::post_json(std::string url, std::string json) {
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

} // namespace NetCore