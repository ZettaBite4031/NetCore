#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

#include "http_types.hpp"


namespace NetCore {

    struct RateLimitDecision {
        bool allow_now = true;
        std::chrono::steady_clock::time_point next_allowed{};
        bool is_global_block = false;
    };

    struct RateLimitContext {
        std::string route_key;
        int status = 0;
        const std::vector<HttpHeader>* headers = {};
    };

    class IRateLimitPolicy {
    public:
        virtual ~IRateLimitPolicy() = default;

        virtual RateLimitDecision on_before_request(std::string_view route_key) = 0;
        virtual void on_response(const RateLimitContext& ctx) = 0;
        virtual void reset() {}
    };

    class SimpleHeaderRateLimitPolicy : public IRateLimitPolicy {
    public:
        SimpleHeaderRateLimitPolicy(std::string retry_after = "Retry-After",
                                    std::string global_flag = "X-RateLimit-Global")
            : m_RetryAfter(std::move(retry_after)), m_GlobalFlag(std::move(global_flag)) {}

        RateLimitDecision on_before_request(std::string_view route_key) override;
        void on_response(const RateLimitContext& ctx) override;
        void reset() override;

    private:
        struct Gate {
            std::chrono::steady_clock::time_point next_allowed{};
        };

        std::string m_RetryAfter;
        std::string m_GlobalFlag;
        std::mutex m_Mtx;
        Gate m_Gate;
        std::unordered_map<std::string, Gate> m_Buckets;

        static bool iequals(std::string_view a, std::string_view b);
        static std::chrono::steady_clock::time_point parse_retry_after(const std::vector<HttpHeader>& headers, std::string_view retry_after_name);
        static bool header_truthy(const std::vector<HttpHeader>& headers, std::string_view name);
    };

} // namespace NetCore