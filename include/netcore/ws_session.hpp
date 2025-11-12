#pragma once

#include "ws_transport.hpp"

#include <functional>
#include <memory>


namespace NetCore {

    class WebSocketSession {
    public:
        using MessageCallback = std::function<void(const std::string&)>;

        WebSocketSession(std::shared_ptr<IWebSocketTransport> transport, MessageCallback cb)
            : m_Transport(std::move(transport)), m_Callback(std::move(cb)) {}

        std::error_code connect(std::string_view uri) { return m_Transport->connect(uri); }

        void run() {
            for(;;) {
                auto msg = m_Transport->receive_text();
                if (!msg) break;
                m_Callback(*msg);
            }
        }

        std::error_code send(std::string_view text) { return m_Transport->send_text(text); }

        void close() { m_Transport->close(); }

    private:
        std::shared_ptr<IWebSocketTransport> m_Transport;
        MessageCallback m_Callback;
    };

} // namespace NetCore