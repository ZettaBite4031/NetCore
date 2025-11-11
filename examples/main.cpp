#include <print>
#include <boost/asio/io_context.hpp>

#include "netcore/http_client.hpp"
#include "netcore/beast_http_transport.hpp"


int main(int argc, char** argv) {
    
    boost::asio::io_context io;
    auto transport = std::make_shared<NetCore::BeastHttpTransport>(io.get_executor());
    NetCore::HttpClient client{ transport };

    auto res = client.get("https://example.com");
    if (!res) {
        std::println("ERROR: {}", res.error().message());
        return 1;
    }

    std::println("Status: {}", res->status);
    std::println("Body: {}", res->body);

    return 0;
}

