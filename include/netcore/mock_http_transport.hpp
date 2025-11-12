#pragma once

#include "http_transport.hpp"
#include "netcore/err.hpp"

#include <queue>
#include <mutex>

namespace NetCore {

    class MockHttpTransport : public IHttpTransport {
    public:
        struct Expectation {
            std::string method, url;
            std::string request_body;
            HttpResponse response;
            std::error_code ec{};
        };

        void expect(Expectation e) {
            std::lock_guard lock(m_Mtx);
            m_Q.push(std::move(e));
        }

        std::expected<HttpResponse, std::error_code> send_request(const HttpRequest& req, const RequestOptions&) override {
            std::lock_guard lock(m_Mtx);
            if (m_Q.empty()) return std::unexpected(make_error_code(errc::http_protocol_error));
            auto e = std::move(m_Q.front()); m_Q.pop();
            if ((!e.method.empty() && e.method != req.method) ||
                (!e.url.empty()    && e.url != req.url) ||
                (!e.request_body.empty() && e.request_body != req.body)) {
                return std::unexpected(make_error_code(errc::http_protocol_error));
            }
            if (e.ec) return std::unexpected(e.ec);
            return e.response;
        }

    private:
        std::mutex m_Mtx;
        std::queue<Expectation> m_Q;
    };

} // NetCore