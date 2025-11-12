#include <print>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include <boost/asio/io_context.hpp>

#include "netcore/netcore.hpp"

struct TestResult {
    std::string name;
    bool ok;
    std::string info;
};

static TestResult run_ws_test(const std::string& url) {
    using namespace NetCore;

    std::string received;
    bool got_msg = false;

    WebSocketSession session{
        make_ws_transport(WsTransportKind::Beast),
        [&](const std::string& msg) {
            received = msg;
            got_msg = true;
        }
    };

    if (auto ec = session.connect(url); ec) {
        return { url, false, "Connection error: " + ec.message() };
    }

    std::thread recv_thread([&session]() {
        session.run();
    });

    const std::string to_send = "NetCore WS-Smoke tests";
    if (auto ec = session.send(to_send); ec) {
        session.close();
        if (recv_thread.joinable()) recv_thread.join();
        return { url, false, "Transmission error: " + ec.message() };
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    session.close();

    if (recv_thread.joinable()) recv_thread.join();

    session.reset();

    if (!got_msg) return { url, false, "No message received" };
    if (received != to_send) {
        return { url, false, "Echo mismatch! Got: '" + received + "'" };
    }

    return { url, true, "" };
}

int main() {
    std::vector<std::string> urls = {
        "wss://ws.postman-echo.com/raw",
        "wss://ws.ifelse.io/",
        "ws://echo-websocket.fly.dev/"
    };

    bool all_ok = true;

    for (auto& url : urls) {
        auto res = run_ws_test(url);
        if (res.ok) std::println("[OK]    {}", res.name);
        else {
            std::print("[FAIL]  {}", res.name);
            if (!res.info.empty()) std::print(" -- {}", res.info);
            std::println();
        }
    }

    return all_ok ? 0 : 1;
}