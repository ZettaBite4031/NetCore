
#include <atomic>
#include <chrono>
#include <csignal>
#include <print>
#include <string>
#include <thread>

#include "netcore/netcore.hpp"
#include "netcore/log.hpp"
#include "netcore/log_sinks.hpp"
#include "netcore/ws_supervisor.hpp"
#include "netcore/ws_reconnect.hpp"
#include "netcore/ws_keepalive.hpp"
#include "netcore/ws_decompress.hpp"


using namespace std::chrono_literals;

static std::atomic<bool> g_Stop{ false };
static void on_sigint(int) { g_Stop.store(true); }

int main() {
    {
        auto &wslog = NetCore::get_logger("ws");
        wslog.set_level(NetCore::LogLevel::info);
        wslog.add_sink(std::make_shared<NetCore::StderrSink>());
    }

    std::signal(SIGINT, on_sigint);
#if defined (SIGTERM)
    std::signal(SIGTERM, on_sigint);
#endif

    auto transport = NetCore::make_ws_transport(NetCore::WsTransportKind::Beast);
    auto reconnect = std::make_unique<NetCore::ExponentialReconnectPolicy>(500ms, 10s);

    NetCore::KeepaliveConfig keepalive;
    keepalive.enabled = true;
    keepalive.ping_interval = 20s;
    keepalive.pong_timeout = 8s;

    NetCore::WebSocketSupervisor sup{ transport, std::move(reconnect), keepalive };
    auto decom = std::make_unique<NetCore::ZlibStreamDecompressor>();
    sup.set_decompressor(std::move(decom));

    const std::string url = "wss://ws.postman-echo.com/raw";

    auto ec = sup.start(url, [](const std::string& msg) { std::println("[WS] Received: {}", msg); });
    if (ec) {
        std::println("[WS] Error: {}", ec.message());
        return 1;
    }

    int counter = 0;
    while (!g_Stop.load()) {
        std::string payload = "NetCore WS Supervisor example #" + std::to_string(++counter);
        if (auto send_ec = sup.send(payload); send_ec) {
            std::println("[WS] Error: {}", send_ec.message());
        }
        std::this_thread::sleep_for(1s);
    }

    std::println("[WS] Stopping");
    sup.stop();


    return 0;
}