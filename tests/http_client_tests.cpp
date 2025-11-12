#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "netcore/netcore.hpp"
#include "netcore/mock_http_transport.hpp"

using namespace NetCore;

TEST_CASE("HttpClient GET happy path") {
    auto mock = std::make_shared<MockHttpTransport>();
    HttpResponse r; r.status = 200; r.body = R"({"ok":true})";
    mock->expect({.method="GET", .url="https://foo.bar/test", .response=r});

    HttpClient client{mock};
    auto res = client.get("https://foo.bar/test");
    REQUIRE(res);
    CHECK(res->status == 200);
    CHECK(res->body.find("ok") != std::string::npos);
}

TEST_CASE("HttpClient propagates transport error") {
    auto mock = std::make_shared<MockHttpTransport>();
    mock->expect({.method="GET", .url="https://x", .ec=make_error_code(errc::connect_failed)});

    HttpClient client{mock};
    auto res = client.get("https://x");
    CHECK(!res);
    CHECK(res.error() == make_error_code(errc::connect_failed));
}