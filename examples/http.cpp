#include <print>

#include "netcore/netcore.hpp"


int main() {
    auto transport = NetCore::make_http_transport(NetCore::HttpTransportKind::Curl /* or Beast */);
    NetCore::HttpClient client{ transport };

    auto res = client.get("https://httpbingo.org/get");
    if (!res) {
        std::println("ERROR: {}", res.error().message());
        return 1;
    }

    std::println("Status: {}", res->status);
    std::println("Body: {}", res->body);

    return 0;
}

