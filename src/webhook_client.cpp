#include "netcore/webhook_client.hpp"



namespace NetCore {

    WebhookClient::WebhookClient(std::shared_ptr<IHttpTransport> http, ClientConfig cfg) 
            : m_Transport(std::move(http)), m_Cfg(cfg) {}

    std::expected<void, std::error_code> WebhookClient::post_json(std::string url, std::string json, const RequestOptions& opt) {
        HttpRequest req;
        req.method = "POST";
        req.url = std::move(url);
        req.body = std::move(json);
        req.headers.push_back({"Content-Type", "application/json"});

        auto res = m_Transport->send_request(req, effective(opt));
        if (!res) return std::unexpected(res.error());
        if (res->status / 100 != 2) 
            return std::unexpected(std::make_error_code(std::errc::io_error));
        return {};
    }

    RequestOptions WebhookClient::effective(const RequestOptions& per_call) const {
        RequestOptions out = per_call;
        if (out.connect_timeout.count() == 0) out.connect_timeout = m_Cfg.default_connect_timeout;
        if (out.read_timeout.count() == 0) out.read_timeout = m_Cfg.default_read_timeout;
        return out;
    }

} // namespace NetCore