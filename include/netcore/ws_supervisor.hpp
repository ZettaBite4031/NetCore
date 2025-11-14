#pragma once

#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <string>
#include <mutex>

#include "ws_transport.hpp"
#include "ws_keepalive.hpp"
#include "ws_reconnect.hpp"
#include "ws_decompress.hpp"

namespace NetCore {

    class WebSocketSupervisor {
    public:
        using MessageCallback = std::function<void(const std::string&)>;

        WebSocketSupervisor(std::shared_ptr<IWebSocketTransport> transport, std::unique_ptr<IReconnectPolicy> reconnect, KeepaliveConfig keepalive);

        std::error_code start(std::string uri, MessageCallback cb);
        std::error_code send(std::string_view text);
        void stop();
        std::chrono::milliseconds last_rtt() const;
        void set_decompressor(std::unique_ptr<IDecompressor> decompressor) { m_Decompressor = std::move(decompressor); }

    private:
        void reader_loop();
        void pinger_loop();

        std::string m_URI;
        MessageCallback m_Callback;
        
        std::shared_ptr<IWebSocketTransport> m_Transport;
        std::shared_ptr<IReconnectPolicy> m_Reconnect;
        std::unique_ptr<IDecompressor> m_Decompressor;
        KeepaliveConfig m_Keepalive;
        
        std::thread m_Reader, m_Pinger;
        std::atomic<bool> m_Running{ false };
        std::atomic<bool> m_Reconnecting{ false };

        std::atomic<bool> m_AwaitingPong{ false };
        std::atomic<std::chrono::steady_clock::time_point> m_PingSent{};
        std::atomic<std::chrono::milliseconds> m_LastRTT{ std::chrono::milliseconds{ -1 } };

        std::mutex m_SendMtx;
    };

} // namespace NetCore