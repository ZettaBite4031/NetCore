#pragma once

#include "netcore/ws_transport.hpp"
#include "netcore/url.hpp"

#include <boost/asio/any_io_executor.hpp>
#include <memory>

#include <atomic>
#include <chrono>

namespace NetCore {

    class BeastWebSocketTransport : public IWebSocketTransport {
    public:
        explicit BeastWebSocketTransport(boost::asio::any_io_executor exec);
        ~BeastWebSocketTransport();

        std::error_code connect(std::string_view uri) override;
        std::error_code send_text(std::string_view message) override;
        std::expected<std::string, std::error_code> receive_text() override;
        void close() override;
        void reset() override;
        std::error_code ping() override;

    private:
        boost::asio::any_io_executor m_Executor;

        struct SslCtxHolder;
        std::unique_ptr<SslCtxHolder> m_SSLCTX;

        std::unique_ptr<class WsPlain> m_Plain;
        std::unique_ptr<class WsSsl> m_SSL;

        std::atomic<std::chrono::steady_clock::time_point> m_LastPong;
    };

} // namespace NetCore