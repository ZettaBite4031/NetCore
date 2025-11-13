#include "retrying_transport.hpp"

#include <thread>
#include <system_error>


namespace NetCore {

    RetryingTransport::RetryingTransport(std::shared_ptr<IHttpTransport> inner, RetryPolicy policy)
        : m_Inner(std::move(inner)), m_Policy(policy) {}

    bool RetryingTransport::is_network_error(const std::error_code& ec) {
        if (!ec) return false;
        switch (static_cast<std::errc>(ec.value())) {
        case std::errc::connection_reset:
        case std::errc::connection_refused:
        case std::errc::network_unreachable:
        case std::errc::timed_out:
            return true;
        default: return false;
        }
    }

    bool RetryingTransport::is_retriable_status(int status, const RetryPolicy& p) {
        if (status == 429) return p.retry_429;
        if (status >= 500 && status <= 599) return p.retry_5xx;
        return false;
    }

    std::expected<HttpResponse, std::error_code> RetryingTransport::send_request(const HttpRequest& req, const RequestOptions& opt) {
        int attempt = 0;
        std::expected<HttpResponse, std::error_code> last_res = std::unexpected(std::make_error_code(std::errc::io_error));

        while (attempt < m_Policy.max_attempts) {
            auto res = m_Inner->send_request(req, opt);
            attempt++;
            if (res) {
                if (!is_retriable_status(res->status, m_Policy)) return res;
                last_res = res;
            } else {
                if (!m_Policy.retry_network_errors || !is_network_error(res.error())) return res;
                last_res = res;
            }

            // backoff
            using namespace std::chrono;
            auto delay = m_Policy.base_delay * (1 << (attempt - 1));
            if (delay > 30s) delay = 30s;
            // jitter ±25%
            auto jitter = delay / 4;
            auto rand_ms = (rand() % (int(2 * jitter.count()) + 1)) - jitter.count();
            delay += milliseconds(rand_ms);
            std::this_thread::sleep_for(delay);
        }

        return last_res;
    }

} // namespace NetCore