#include "netcore/rate_limit.hpp"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <cstring>

namespace NetCore {

    static inline std::string to_lower_copy(std::string s) {
        for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    bool SimpleHeaderRateLimitPolicy::iequals(std::string_view a, std::string_view b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); i++) 
            if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i])) return false;
        return true;
    }

    std::chrono::steady_clock::time_point SimpleHeaderRateLimitPolicy::parse_retry_after(const std::vector<HttpHeader>& headers, std::string_view retry_after_name) {
        /*
            Retry-After can be:
                - integer secodns
                - HTTP-date (RFC 7231)
        */
        for (auto& h : headers) {
            if (iequals(h.name, retry_after_name)) {
                try {
                    int secs = std::stoi(h.value);
                    return std::chrono::steady_clock::now() + std::chrono::seconds(secs);
                } catch (...) {
                    std::tm tm{};
                    char wk[4], mon[4], tz[4];
                    int day, year, hour, min, sec;

                    if (std::sscanf(h.value.c_str(), "%3s, %d %3s %d %d:%d:%d %3s",
                                    wk, &day, mon, &year, &hour, &min, &sec, tz) == 8) {
                        static const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
                        const char* p = std::strstr(months, mon);
                        if (p) {
                            tm.tm_mday = day;
                            tm.tm_year = year - 1900;
                            tm.tm_hour = hour;
                            tm.tm_min = min;
                            tm.tm_sec = sec;
                            tm.tm_mon = int((p - months) / 3);
                            tm.tm_isdst = 0;

                            time_t t = timegm(&tm);
                            auto now = std::chrono::system_clock::now();
                            auto tp = std::chrono::system_clock::from_time_t(t);
                            auto dur = tp - now;
                            return std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(dur);
                        }
                    }
                }
            }
        }
        return std::chrono::steady_clock::time_point{};
    }

    bool SimpleHeaderRateLimitPolicy::header_truthy(const std::vector<HttpHeader>& headers, std::string_view name) {
        for (auto& h : headers) {
            if (iequals(h.name, name)) {
                auto v = to_lower_copy(h.value);
                return (v == "1" || v == "true" || v == "yes");
            }
        }
        return false;
    }

    RateLimitDecision SimpleHeaderRateLimitPolicy::on_before_request(std::string_view route_key_sv) {
        std::lock_guard lock(m_Mtx);
        auto now = std::chrono::steady_clock::now();

        if (m_Gate.next_allowed != std::chrono::steady_clock::time_point{} && now < m_Gate.next_allowed) return { false, m_Gate.next_allowed, true };

        std::string route_key(route_key_sv);
        auto& g = m_Buckets[route_key];
        if (g.next_allowed != std::chrono::steady_clock::time_point{} && now < g.next_allowed) return { false, g.next_allowed, false };

        return { true, {}, true };
    }

    void SimpleHeaderRateLimitPolicy::on_response(const RateLimitContext& ctx) {
        if (!ctx.headers) return;
        auto until = parse_retry_after(*ctx.headers, m_RetryAfter);
        bool is_global = header_truthy(*ctx.headers, m_GlobalFlag);

        std::lock_guard lock(m_Mtx);
        if (until != std::chrono::steady_clock::time_point{}) {
            if (is_global && until > m_Gate.next_allowed) m_Gate.next_allowed = until;
            else {
                auto& g = m_Buckets[ctx.route_key];
                if (until > g.next_allowed) g.next_allowed = until;
            }
        }
    }

    void SimpleHeaderRateLimitPolicy::reset() {
        std::lock_guard lock(m_Mtx);
        m_Buckets.clear();
        m_Gate = {};
    }
} // namespace NetCore