#pragma once

#include "http_transport.hpp"
#include <vector>
#include <memory>

namespace NetCore {

    class HttpClient {
    public:
        explicit HttpClient(std::shared_ptr<IHttpTransport> transport, ClientConfig cfg = {});

        std::expected<HttpResponse, std::error_code> get(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {}, const RequestOptions& opt);
        std::expected<HttpResponse, std::error_code> post(std::string url, std::string body, std::vector<HttpHeader> headers = {}, const RequestOptions& opt);
        std::expected<HttpResponse, std::error_code> put(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {}, const RequestOptions& opt);
        std::expected<HttpResponse, std::error_code> patch(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {}, const RequestOptions& opt);
        std::expected<HttpResponse, std::error_code> del(std::string url, std::string body = {}, std::vector<HttpHeader> headers = {}, const RequestOptions& opt);
        std::expected<HttpResponse, std::error_code> request(std::string method, std::string url, std::string body = {}, std::vector<HttpHeader> headers = {}, const RequestOptions& opt);

    private:
        RequestOptions effective(const RequestOptions& per_call) const;

        std::shared_ptr<IHttpTransport> m_Transport;
        ClientConfig m_Cfg;
    };

} // namespace NetCore