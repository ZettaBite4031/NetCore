#pragma once

#include "http_transport.hpp"
#include <memory>
#include <vector>

namespace NetCore {

    class HttpClient {
    public:
        explicit HttpClient(std::shared_ptr<IHttpTransport> transport) 
            : m_Transport(std::move(transport)) {}

        std::expected<HttpResponse, std::error_code> get(std::string url) {
            HttpRequest req;
            req.method = "GET";
            req.url = std::move(url);
            return m_Transport->send_request(req);
        }

        std::expected<HttpResponse, std::error_code> post(std::string url, std::string body, std::vector<HttpHeader> headers = {}) {
            HttpRequest req;
            req.method = "POST";
            req.url = std::move(url);
            req.body = std::move(body);
            req.headers = std::move(headers);
            return m_Transport->send_request(req);
        }

        std::expected<HttpResponse, std::error_code> put(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {}) {
            HttpRequest req;
            req.method = "PUT";
            req.url = std::move(url);
            req.body = std::move(body);
            req.headers = std::move(headers);
            return m_Transport->send_request(req);
        }

        std::expected<HttpResponse, std::error_code> patch(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {}) {
            HttpRequest req;
            req.method = "PATCH";
            req.url = std::move(url);
            req.body = std::move(body);
            req.headers = std::move(headers);
            return m_Transport->send_request(req);
        }

        std::expected<HttpResponse, std::error_code> del(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {}) {
            HttpRequest req;
            req.method = "DELETE";
            req.url = std::move(url);
            req.body = std::move(body);
            req.headers = std::move(headers);
            return m_Transport->send_request(req);
        }

        std::expected<HttpResponse, std::error_code> request(std::string method, std::string url, std::string body = {}, std::vector<HttpHeader> headers = {}) {
            HttpRequest req;
            req.method = std::move(method);
            req.url = std::move(url);
            req.body = std::move(body);
            req.headers = std::move(headers);
            return m_Transport->send_request(req);
        }

    private:
        std::shared_ptr<IHttpTransport> m_Transport;
    };

} // namespace NetCore