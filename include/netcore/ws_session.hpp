#pragma once

#include "ws_transport.hpp"

#include <functional>
#include <memory>
#include <mutex>

namespace NetCore {

    class WebSocketSession {
    public:
        using MessageCallback = std::function<void(const std::string&)>;

        WebSocketSession(std::shared_ptr<IWebSocketTransport> transport, MessageCallback cb);

        std::error_code connect(std::string_view uri);
        void run();
        std::error_code send(std::string_view text);
        void close();
        void reset();

    private:
        std::mutex m_Mutex;
        std::shared_ptr<IWebSocketTransport> m_Transport;
        MessageCallback m_Callback;
    };

} // namespace NetCore