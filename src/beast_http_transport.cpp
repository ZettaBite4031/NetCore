#include "netcore/beast_http_transport.hpp"
#include "netcore/url.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

namespace NetCore {

    namespace http = boost::beast::http;
    namespace beast = boost::beast;
    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;

    static void configure_ssl_ctx(asio::ssl::context& ctx) {
        ctx.set_options(
            asio::ssl::context::default_workarounds
        | asio::ssl::context::no_sslv2
        | asio::ssl::context::no_sslv3
        | asio::ssl::context::single_dh_use);

        // try system CAs (needs ca-certificates on Linux)
        boost::system::error_code ec;
        ctx.set_default_verify_paths(ec);
        // if ec, you can later load a PEM bundle manually

        ctx.set_verify_mode(asio::ssl::verify_peer);
    }

    BeastHttpTransport::BeastHttpTransport(asio::any_io_executor exec)
        : m_Executor(std::move(exec)) {}

    std::expected<HttpResponse, std::error_code> BeastHttpTransport::send_request(const HttpRequest& request) {
        auto parsed_opt = parse_url(request.url);
        if (!parsed_opt) {
            return std::unexpected(std::make_error_code(std::errc::invalid_argument));
        }
        auto parsed = *parsed_opt;

        beast::error_code ec;

        if (parsed.scheme == "https") {
            asio::ssl::context ctx{ asio::ssl::context::tls_client };
            configure_ssl_ctx(ctx);

            tcp::resolver resolver{ m_Executor };
            auto results = resolver.resolve(parsed.host, parsed.port, ec);
            if (ec) return std::unexpected(ec);

            asio::ssl::stream<beast::tcp_stream> stream(m_Executor, ctx);
            if (!SSL_set_tlsext_host_name(stream.native_handle(), parsed.host.c_str())) {
                beast::error_code ec2{
                    static_cast<int>(::ERR_get_error()),
                    asio::error::get_ssl_category()
                };
                return std::unexpected(ec2);
            }


            beast::get_lowest_layer(stream).connect(results, ec);
            if (ec) return std::unexpected(ec);            

            stream.handshake(asio::ssl::stream_base::client, ec);
            if (ec) return std::unexpected(ec);

            http::request<http::string_body> req;
            req.method_string(request.method);
            req.target(parsed.target);
            req.version(11);
            req.set(http::field::host, parsed.host);
            for (auto& h : request.headers) {
                req.set(h.name, h.value);
            }
            req.body() = request.body;
            if (!request.body.empty()) {
                req.prepare_payload();
            }

            http::write(stream, req, ec);
            if (ec) return std::unexpected(ec);
            
            beast::flat_buffer buffer;
            http::response<http::string_body> hres;
            http::read(stream, buffer, hres, ec);
            if (ec) 
                return std::unexpected(ec);
            
            stream.shutdown(ec);
            if (ec == asio::error::eof || ec == asio::ssl::error::stream_truncated) ec = {};

            HttpResponse res; 
            res.status = static_cast<int>(hres.result_int());
            for (auto& f : hres) {
                res.headers.push_back({std::string(f.name_string()), std::string(f.value())});
            }
            res.body = std::move(hres.body());
            return res;
        } else {
            tcp::resolver resolver(m_Executor);
            auto results = resolver.resolve(parsed.host, parsed.port, ec);
            if (ec) return std::unexpected(ec);

            beast::tcp_stream stream(m_Executor);
            stream.connect(results, ec);
            if (ec) return std::unexpected(ec);

            http::request<http::string_body> req;
            req.method_string(request.method);
            req.target(parsed.target);
            req.version(11);
            req.set(http::field::host, parsed.host);
            for (auto& h : request.headers) {
                req.set(h.name, h.value);
            }
            req.body() = request.body;
            if (!request.body.empty()) {
                req.prepare_payload();
            }

            http::write(stream, req, ec);
            if (ec) return std::unexpected(ec);

            beast::flat_buffer buffer;
            http::response<http::string_body> hres;
            http::read(stream, buffer, hres, ec);
            if (ec) return std::unexpected(ec);

            stream.socket().shutdown(tcp::socket::shutdown_both, ec);

            HttpResponse res;
            res.status = static_cast<int>(hres.result_int());
            for (auto& f : hres) {
                res.headers.push_back({std::string(f.name_string()), std::string(f.value())});
            }
            res.body = std::move(hres.body());
            
            return res;
        }
    }

} // namespace NetCore