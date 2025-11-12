#pragma once

#include "http_transport.hpp"
#include <vector>
#include <memory>

namespace NetCore {

    class HttpClient {
    public:
        explicit HttpClient(std::shared_ptr<IHttpTransport> transport);

        std::expected<HttpResponse, std::error_code> get(std::string url);
        std::expected<HttpResponse, std::error_code> post(std::string url, std::string body, std::vector<HttpHeader> headers = {});
        std::expected<HttpResponse, std::error_code> put(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {});
        std::expected<HttpResponse, std::error_code> patch(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {});
        std::expected<HttpResponse, std::error_code> del(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {});
        std::expected<HttpResponse, std::error_code> request(std::string method, std::string url, std::string body = {}, std::vector<HttpHeader> headers = {});

    private:
        std::shared_ptr<IHttpTransport> m_Transport;
    };

} // namespace NetCore