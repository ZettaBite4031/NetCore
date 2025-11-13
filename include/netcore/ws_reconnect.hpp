#pragma once

#include <chrono>
#include <system_error>
#include <cstdint>


namespace NetCore {

    struct ReconnectDecision {
        bool reconnect = false;
        std::chrono::milliseconds delay{0};
        bool reset_backoff = false;
    };

    class IReconnectPolicy {
    public:
        virtual ~IReconnectPolicy() = default;
        virtual ReconnectDecision on_close(uint16_t close_code) = 0;
        virtual ReconnectDecision on_error(std::error_code ec) = 0;
        virtual void reset() = 0;
    };

    class ExponentialReconnectPolicy : public IReconnectPolicy {
    public:
        explicit ExponentialReconnectPolicy(std::chrono::milliseconds base = std::chrono::milliseconds(500),
                                        std::chrono::milliseconds max  = std::chrono::milliseconds(30'000))
            : m_Base(base), m_Max(max) {}

        ReconnectDecision on_close(uint16_t code) override {
            // 1000 == normal close. Don't attempt reconnect
            if (code == 1000) { m_Attempt = 0; return { false, {}, true }; }
            return next(true);
        }

        ReconnectDecision on_error(std::error_code) override {
            return next(true);
        }

        void reset() override { m_Attempt = 0; }

    private:
        ReconnectDecision next(bool do_reconnect) {
            if (!do_reconnect) return { false, {}, false };
            using namespace std::chrono;
            auto mul = (m_Attempt < 10 ? (1u << m_Attempt) : (1u << 10));
            auto d = m_Base * mul;
            if (d > m_Max) d = m_Max;
            m_Attempt++;
            auto jitter = duration_cast<milliseconds>(d / 10);
            d += milliseconds(rand() % (2 * jitter.count() + 1) - jitter.count());
            return { true, d, false };
        }

        std::chrono::milliseconds m_Base, m_Max;
        unsigned m_Attempt = 0;
    };

} // NetCore