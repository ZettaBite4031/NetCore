#include "netcore/rate_limited_http_transport.hpp"
#include "netcore/log.hpp"
#include <thread>


namespace NetCore {

    std::expected<HttpResponse, std::error_code> RateLimitedTransport::send_request(const HttpRequest& req, const RequestOptions& opt) {
        const std::string route_key = opt.route_key;

        for (;;) {
            RateLimitDecision d = m_Policy->on_before_request(route_key);
            if (d.allow_now) break;

            auto now = std::chrono::steady_clock::now();
            auto wake = d.next_allowed;
            if (wake <= now) break;

            std::unique_lock lock(m_Mtx);
            m_CV.wait_until(lock, wake);
        }

        auto res = m_Inner->send_request(req, opt);

        RateLimitContext ctx;
        ctx.route_key = route_key;
        if (res) {
            ctx.status = res->status;
            ctx.headers = &res->headers;
        } else {
            ctx.status = 0;
            ctx.headers = nullptr;
        }
        m_Policy->on_response(ctx);
        m_CV.notify_all();
        return res;
    }

} // namespace NetCore