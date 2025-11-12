#include <print>
#include <boost/asio/io_context.hpp>

#include "netcore/netcore.hpp"


int main() {
    boost::asio::io_context io;                     
    auto transport = NetCore::make_http_transport(NetCore::HttpTransportKind::Curl /* or Beast */);
    NetCore::HttpClient client{ transport };

    auto res = client.get("https://httpbin.org/get");
    if (!res) {
        std::println("ERROR: {}", res.error().message());
        return 1;
    }

    std::println("Status: {}", res->status);
    std::println("Body: {}", res->body);

    return 0;
}

