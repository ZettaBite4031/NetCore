#include "netcore/http_client.hpp"

namespace NetCore {

    HttpClient::HttpClient(std::shared_ptr<IHttpTransport> transport) 
            : m_Transport(std::move(transport)) {}

    std::expected<HttpResponse, std::error_code> HttpClient::get(std::string url) {
        HttpRequest req;
        req.method = "GET";
        req.url = std::move(url);
        return m_Transport->send_request(req);
    }

    std::expected<HttpResponse, std::error_code> HttpClient::post(std::string url, std::string body, std::vector<HttpHeader> headers) {
        HttpRequest req;
        req.method = "POST";
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req);
    }

    std::expected<HttpResponse, std::error_code> HttpClient::put(std::string url, std::string body, std::vector<HttpHeader> headers) {
        HttpRequest req;
        req.method = "PUT";
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req);
    }

    std::expected<HttpResponse, std::error_code> HttpClient::patch(std::string url, std::string body, std::vector<HttpHeader> headers) {
        HttpRequest req;
        req.method = "PATCH";
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req);
    }

    std::expected<HttpResponse, std::error_code> HttpClient::del(std::string url, std::string body, std::vector<HttpHeader> headers) {
        HttpRequest req;
        req.method = "DELETE";
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req);
    }

    std::expected<HttpResponse, std::error_code> HttpClient::request(std::string method, std::string url, std::string body, std::vector<HttpHeader> headers) {
        HttpRequest req;
        req.method = std::move(method);
        req.url = std::move(url);
        req.body = std::move(body);
        req.headers = std::move(headers);
        return m_Transport->send_request(req);
    }

} // namespace NetCore