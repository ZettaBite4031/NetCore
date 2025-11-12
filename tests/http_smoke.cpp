#include <print>
#include <vector>
#include <memory>

#include <boost/asio/io_context.hpp>

#include "netcore/netcore.hpp"

struct TestResult {
    std::string name;
    bool ok;
    std::string info;
};

static TestResult test_get_postman(NetCore::HttpClient& client) {
    auto res = client.get("https://postman-echo.com/get");
    if (!res) 
        return { "GET postman-echo", false, res.error().message() };
    if (res->status != 200) 
        return { "GET postman-echo", false, "Expected Status: " + std::to_string(res->status) };
    return { "GET postman-echo", true, "" };
}

static TestResult test_post_postman(NetCore::HttpClient& client) {
    std::string body = R"({"hello":"world"})";
    std::vector<NetCore::HttpHeader> headers = {
        {"Content-Type", "application/json"}
    };
    auto res = client.post("https://postman-echo.com/post", body, headers);
    if (!res) {
        return {"POST postman-echo", false, res.error().message()};
    }
    if (res->status != 200) {
        return {"POST postman-echo", false, "unexpected status: " + std::to_string(res->status)};
    }
    // optional: check if body contains `"hello":"world"`
    if (res->body.find("\"hello\":\"world\"") == std::string::npos) {
        return {"POST postman-echo", false, "echo body missing field"};
    }
    return {"POST postman-echo", true, ""};
}

static TestResult test_get_reqres(NetCore::HttpClient& client) {
    auto res = client.get("https://reqres.in/api/users/2");
    if (!res) {
        return {"GET reqres user 2", false, res.error().message()};
    }
    if (res->status != 200) {
        return {"GET reqres user 2", false, "unexpected status: " + std::to_string(res->status)};
    }
    if (res->body.find("\"id\": 2") == std::string::npos &&
        res->body.find("\"id\":2") == std::string::npos) {
        return {"GET reqres user 2", false, "body did not look like user 2"};
    }
    return {"GET reqres user 2", true, ""};
}

static TestResult test_get_jsonplaceholder(NetCore::HttpClient& client) {
    auto res = client.get("https://jsonplaceholder.typicode.com/posts/1");
    if (!res) {
        return {"GET jsonplaceholder post 1", false, res.error().message()};
    }
    if (res->status != 200) {
        return {"GET jsonplaceholder post 1", false, "unexpected status: " + std::to_string(res->status)};
    }
    if (res->body.find("\"id\": 1") == std::string::npos &&
        res->body.find("\"id\":1") == std::string::npos) {
        return {"GET jsonplaceholder post 1", false, "body did not look like post 1"};
    }
    return {"GET jsonplaceholder post 1", true, ""};
}

static TestResult test_delay_postman(NetCore::HttpClient& client) {
    auto res = client.get("https://postman-echo.com/delay/3");
    if (!res) {
        return {"GET postman delay 3", false, res.error().message()};
    }
    if (res->status != 200) {
        return {"GET postman delay 3", false, "unexpected status: " + std::to_string(res->status)};
    }
    return {"GET postman delay 3", true, ""};
}

static TestResult test_put_postman(NetCore::HttpClient& client) {
    auto res = client.put("https://postman-echo.com/put", R"({"name":"NetCore"})", {{"Content-Type", "application/json"}});
    if (!res) {
        return {"GET postman put", false, res.error().message()};
    }
    if (res->status != 200) {
        return {"GET postman put", false, "unexpected status: " + std::to_string(res->status)};
    }
    return {"GET postman put", true, ""};
}

static TestResult test_del_postman(NetCore::HttpClient& client) {
    auto res = client.del("https://postman-echo.com/delete");
    if (!res) {
        return {"GET postman delete", false, res.error().message()};
    }
    if (res->status != 200) {
        return {"GET postman delete", false, "unexpected status: " + std::to_string(res->status)};
    }
    return {"GET postman delete", true, ""};
}

static TestResult test_patch_postman(NetCore::HttpClient& client) {
    auto res = client.patch("https://postman-echo.com/patch", "name=NetCore");
    if (!res) {
        return {"GET postman patch", false, res.error().message()};
    }
    if (res->status != 200) {
        return {"GET postman patch", false, "unexpected status: " + std::to_string(res->status)};
    }
    return {"GET postman patch", true, ""};
}

int main() {
    struct TransportConfig {
        std::string name;
        std::shared_ptr<NetCore::IHttpTransport> transport;
    };

    std::vector<TransportConfig> transports;

    transports.push_back({
        "Beast",
        NetCore::make_http_transport(NetCore::HttpTransportKind::Beast, NetCore::TransportWrap::Logging)
    });

    transports.push_back({
        "cURL",
        NetCore::make_http_transport(NetCore::HttpTransportKind::Curl, NetCore::TransportWrap::Logging)
    });

    bool global_ok = true;

    for (auto& t : transports) {
        std::println("\n=== Transport: {} ===", t.name);
        NetCore::HttpClient client{t.transport};

        std::vector<TestResult> results;
        results.push_back(test_get_postman(client));
        results.push_back(test_post_postman(client));
        results.push_back(test_get_reqres(client));
        results.push_back(test_get_jsonplaceholder(client));
        results.push_back(test_put_postman(client));
        results.push_back(test_del_postman(client));
        results.push_back(test_patch_postman(client));
        results.push_back(test_delay_postman(client));

        for (auto& r : results) {
            if (r.ok) {
                std::println("[OK]   {}", r.name);
            } else {                
                std::print("[FAIL] {}", r.name);
                if (!r.info.empty())
                    std::print(" -- {}", r.info);
                std::println();
                global_ok = false;
            }
        }
    }

    return global_ok ? 0 : 1;
}