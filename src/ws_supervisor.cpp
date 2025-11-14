#include "netcore/ws_supervisor.hpp"
#include "netcore/log.hpp"

#include <chrono>
#include <thread>

namespace NetCore {

    using namespace std::chrono;

    WebSocketSupervisor::WebSocketSupervisor(std::shared_ptr<IWebSocketTransport> transport, std::unique_ptr<IReconnectPolicy> reconnect, KeepaliveConfig keepalive)
        : m_Transport(std::move(transport)), m_Reconnect(std::move(reconnect)), m_Keepalive(keepalive) {}

    std::error_code WebSocketSupervisor::start(std::string uri, MessageCallback cb) {
        m_URI = std::move(uri);
        m_Callback = std::move(cb);

        if (auto ec = m_Transport->connect(m_URI); ec) {
            if (m_Reconnect) m_Reconnect->reset();
            return ec;
        }

        m_Running.store(true, std::memory_order_relaxed);
        m_Reader = std::thread([this]{ reader_loop(); });

        if (m_Keepalive.enabled) m_Pinger = std::thread([this]{ pinger_loop(); });

        return {};
    }

    void WebSocketSupervisor::reader_loop() {
        auto& log = get_logger("WS");

        while (m_Running.load(std::memory_order_relaxed)) {
            auto msg = m_Transport->receive_text();
            if (!msg) {
                // connection died; attempt reconnect per policy
                NC_LOG_WARN(log, "WS Read Error", {{"ec", msg.error().message()}});
                if (!m_Running.load()) break;

                m_Reconnecting.store(true);

                auto dec = m_Reconnect ? m_Reconnect->on_error(msg.error()) : ReconnectDecision{ true, 1000ms };
                 if (!dec.reconnect) break;

                 std::this_thread::sleep_for(dec.delay);
                 m_Transport->close();

                 for (;;) {
                    if (!m_Running.load()) return;
                    auto ec = m_Transport->connect(m_URI);
                    if (!ec) {
                        NC_LOG_INFO(log, "WS Reconnected", {{"uri", m_URI}});
                        m_Reconnect->reset();
                        m_Reconnecting.store(false);
                        break;
                    }
                    auto dec2 = m_Reconnect->on_error(ec);
                    if (!dec2.reconnect) { m_Running.store(false); return; }
                    std::this_thread::sleep_for(dec2.delay);
                }
                continue;
            }

            if (m_Callback)  { 
                if (m_Decompressor) {
                    auto dec = m_Decompressor->feed(*msg, true);
                    if (!dec) break;
                    m_Callback(*msg); 
                } else m_Callback(*msg); 
            }
        }
    }

    void WebSocketSupervisor::pinger_loop() {
        auto& log = get_logger("WS");

        while (m_Running.load(std::memory_order_relaxed)) {
            auto start = steady_clock::now();

            if (!m_Reconnecting.load()) {
                if (!m_AwaitingPong.load()) {
                    if (auto ec = m_Transport->ping(); !ec) {
                        m_PingSent.store(start);
                        m_AwaitingPong.store(true);
                    } else {
                        // ping not supported or failed. currently ignore
                    }
                }
            }

            auto until = start + m_Keepalive.ping_interval;
            while (m_Running.load() && steady_clock::now() < until) {
                std::this_thread::sleep_for(200ms);
                if (m_AwaitingPong.load()) {
                    auto sent = m_PingSent.load();
                    if (sent.time_since_epoch().count() != 0 && steady_clock::now() - sent > m_Keepalive.pong_timeout) {
                        NC_LOG_WARN(log, "WS Pong Timeout", {});
                        m_Transport->close();
                        m_AwaitingPong.store(false);
                        break;
                    }
                }
            }

            // RTT accounting:
            // Beast calls control_callback on pong; recorded it in the transport (m_LastPong).
            // Since it isn't exposed  directly, it can approximate by clearing on next text receive,
            // or a tiny callback from transport when pong arrives to update m_LastRTT could be added.
            if (m_AwaitingPong.load() == true) {
                // Might extend transport to expose "on_pong" callback.
                // For now, keep it best effort:  clear when next text arrives in reader loop.
            } else {
                // if a pong was observed via a transport callback, set m_LastRTT there
            }
        }
    }

    std::error_code WebSocketSupervisor::send(std::string_view text) {
        std::scoped_lock lock(m_SendMtx);
        return m_Transport->send_text(text);
    }

    void WebSocketSupervisor::stop() {
        m_Running.store(false, std::memory_order_relaxed);
        {
            std::scoped_lock lock(m_SendMtx);
            m_Transport->close();
        }

        if (m_Pinger.joinable()) m_Pinger.join();
        if (m_Reader.joinable()) m_Reader.join();
    }

    std::chrono::milliseconds WebSocketSupervisor::last_rtt() const {
        return m_LastRTT.load();
    }

} // namespace NetCore