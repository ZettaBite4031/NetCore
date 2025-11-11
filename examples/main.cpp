#include <print>
#include <boost/asio/io_context.hpp>

#include "netcore/http_client.hpp"
#include "netcore/transport_factory.hpp"


int main(int argc, char** argv) {
    
    boost::asio::io_context io;
    auto transport = NetCore::make_http_transport(NetCore::TransportKind::Beast, NetCore::TransportWrap::Logging);
    NetCore::HttpClient client{ transport };

    auto res = client.get("https://httpbin.org/delay/10");
    if (!res) {
        std::println("ERROR: {}", res.error().message());
        return 1;
    }

    std::println("Status: {}", res->status);
    std::println("Body: {}", res->body);

    return 0;
}

