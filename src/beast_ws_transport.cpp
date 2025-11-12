#include "beast_ws_transport.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/err.h>

namespace NetCore {

    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;

    struct BeastWebSocketTransport::SslCtxHolder {
        asio::ssl::context ctx;
        explicit SslCtxHolder() : ctx(asio::ssl::context::tls_client) {}
    };

    struct WsPlain {
        websocket::stream<beast::tcp_stream> ws;
        explicit WsPlain(asio::any_io_executor exec) : ws(exec) {}
    };

    struct WsSsl {
        websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws;
        explicit WsSsl(asio::any_io_executor exec, asio::ssl::context& ctx) : ws(exec, ctx) {}
    };

    BeastWebSocketTransport::BeastWebSocketTransport(asio::any_io_executor exec) 
        : m_Executor(std::move(exec)) {}

    BeastWebSocketTransport::~BeastWebSocketTransport() = default;

    std::error_code BeastWebSocketTransport::connect(std::string_view uri) {
        auto parsed_opt = parse_url(uri);
        if (!parsed_opt) return std::make_error_code(std::errc::invalid_argument);
        auto parsed = *parsed_opt;
        beast::error_code ec;

        if (parsed.scheme == "ws") {
            tcp::resolver resolver(m_Executor);
            auto results = resolver.resolve(parsed.host, parsed.port, ec);
            if (ec) return ec;

            m_Plain = std::make_unique<WsPlain>(m_Executor);
            beast::get_lowest_layer(m_Plain->ws).connect(results, ec);
            if(ec) return ec;

            m_Plain->ws.handshake(parsed.host, parsed.target, ec);
            return ec;
        } else if (parsed.scheme == "wss") {
            m_SSLCTX = std::make_unique<SslCtxHolder>();
            m_SSLCTX->ctx.set_default_verify_paths(ec);
            m_SSLCTX->ctx.set_verify_mode(asio::ssl::verify_peer);

            tcp::resolver resolver(m_Executor);
            auto results = resolver.resolve(parsed.host, parsed.port, ec);
            if (ec) return ec;

            m_SSL = std::make_unique<WsSsl>(m_Executor, m_SSLCTX->ctx);

            if (!SSL_set_tlsext_host_name(
                    m_SSL->ws.next_layer().native_handle(),
                    parsed.host.c_str())) {
                beast::error_code ec2{
                    static_cast<int>(::ERR_get_error()),
                    asio::error::get_ssl_category()
                };
                return ec2;
            }

            beast::get_lowest_layer(m_SSL->ws).connect(results, ec);
            if (ec) return ec;

            m_SSL->ws.next_layer().handshake(asio::ssl::stream_base::client, ec);
            if (ec) return ec;

            m_SSL->ws.handshake(parsed.host, parsed.target, ec);
            return ec;
        } else {
            return std::make_error_code(std::errc::protocol_not_supported);
        }
    }

    std::error_code BeastWebSocketTransport::send_text(std::string_view text) {
        beast::error_code ec;
        if (m_Plain) {
            m_Plain->ws.text(true);
            m_Plain->ws.write(asio::buffer(text), ec);
            return ec;
        } 
        if (m_SSL) {
            m_SSL->ws.text(true);
            m_SSL->ws.write(asio::buffer(text), ec);
            return ec;
        }
        return std::make_error_code(std::errc::not_connected);
    }

    std::expected<std::string, std::error_code> BeastWebSocketTransport::receive_text() {
        beast::flat_buffer buffer;
        beast::error_code ec;
        if (m_Plain) m_Plain->ws.read(buffer, ec);
        else if (m_SSL) m_SSL->ws.read(buffer, ec);
        else return std::unexpected(std::make_error_code(std::errc::not_connected));
        if (ec) return std::unexpected(ec);
        return std::string(beast::buffers_to_string(buffer.data()));
    }

    void BeastWebSocketTransport::close() {
        beast::error_code ec;
        if (m_Plain) {
            m_Plain->ws.close(websocket::close_code::normal, ec);
            m_Plain.reset();
        }
        if (m_SSL) {
            m_SSL->ws.close(websocket::close_code::normal, ec);
            m_SSL.reset();
        }
        m_SSLCTX.reset();
    }

} // namespace NetCore