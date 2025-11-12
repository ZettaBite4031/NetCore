#include "netcore/ws_session.hpp"

namespace NetCore {

    WebSocketSession::WebSocketSession(std::shared_ptr<IWebSocketTransport> transport, MessageCallback cb)
            : m_Transport(std::move(transport)), m_Callback(std::move(cb)) {}

    std::error_code WebSocketSession::connect(std::string_view uri) { return m_Transport->connect(uri); }
    
    void WebSocketSession::run() {
        for(;;) {
            auto msg = m_Transport->receive_text();
            if (!msg) break;
            m_Callback(*msg);
        }
    }

    std::error_code WebSocketSession::send(std::string_view text) { 
        std::scoped_lock lock(m_Mutex);
        return m_Transport->send_text(text); 
    }

    void WebSocketSession::close() { 
        std::scoped_lock lock(m_Mutex);
        m_Transport->close(); 
    }
    
    void WebSocketSession::reset() { 
        std::scoped_lock lock(m_Mutex);
        m_Transport->reset(); 
    }

} // namespace NetCore