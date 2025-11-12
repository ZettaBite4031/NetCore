#include "netcore/http_client.hpp"

namespace NetCore {

    HttpClient::HttpClient(std::shared_ptr<IHttpTransport> transport, ClientConfig cfg) 
            : m_Transport(std::move(transport)), m_Cfg(cfg) {}

    std::expected<HttpResponse, std::error_code> HttpClient::get(std::string url, std::string body, std::vector<HttpHeader> headers, const RequestOptions& opt) {
        HttpRequest req;
        req.method = "GET";
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req, effective(opt));
    }

    std::expected<HttpResponse, std::error_code> HttpClient::post(std::string url, std::string body, std::vector<HttpHeader> headers, const RequestOptions& opt) {
        HttpRequest req;
        req.method = "POST";
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req, effective(opt));
    }

    std::expected<HttpResponse, std::error_code> HttpClient::put(std::string url, std::string body, std::vector<HttpHeader> headers, const RequestOptions& opt) {
        HttpRequest req;
        req.method = "PUT";
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req, effective(opt));
    }

    std::expected<HttpResponse, std::error_code> HttpClient::patch(std::string url, std::string body, std::vector<HttpHeader> headers, const RequestOptions& opt) {
        HttpRequest req;
        req.method = "PATCH";
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req, effective(opt));
    }

    std::expected<HttpResponse, std::error_code> HttpClient::del(std::string url, std::string body, std::vector<HttpHeader> headers, const RequestOptions& opt) {
        HttpRequest req;
        req.method = "DELETE";
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req, effective(opt));
    }

    std::expected<HttpResponse, std::error_code> HttpClient::request(std::string method, std::string url, std::string body, std::vector<HttpHeader> headers, const RequestOptions& opt) {
        HttpRequest req;
        req.method = std::move(method);
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req, effective(opt));
    }

    RequestOptions HttpClient::effective(const RequestOptions& per_call) const {
        RequestOptions out = per_call;
        if (out.connect_timeout.count() == 0) out.connect_timeout = m_Cfg.default_connect_timeout;
        if (out.read_timeout.count() == 0) out.read_timeout = m_Cfg.default_read_timeout;
        return out;
    }

} // namespace NetCore