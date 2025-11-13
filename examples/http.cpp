#include <print>

#include "netcore/netcore.hpp"


int main() {
    auto transport = NetCore::make_http_transport(NetCore::HttpTransportKind::Beast /* or Beast */, NetCore::TransportWrap::RedirectRateLimit);
    NetCore::HttpClient client{ transport };

    auto res = client.get("https://httpbin.org/absolute-redirect/5");
    if (!res) {
        std::println("ERROR: {}", res.error().message());
        return 1;
    }

    std::println("Status: {}", res->status);
    std::println("Body: {}", res->body);

    return 0;
}

