#include <print>
#include <thread>
#include <boost/asio/io_context.hpp>
#include "netcore/ws_session.hpp"
#include "netcore/transport_factory.hpp"

int main() {
    auto transport = NetCore::make_ws_transport(NetCore::WsTransportKind::Beast);

    NetCore::WebSocketSession session{
        transport,
        [](const std::string& msg) {
            std::println("[WS] Received: {}", msg);
        }
    };

    std::string url = "wss://ws.postman-echo.com/raw";

    if (auto ec = session.connect(url); ec) {
        std::println("[WS] Connection error: {}", ec.message());
        return 1;
    }

    std::thread reader([&session]() {
        session.run();
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (auto ec = session.send("NetCore WS Test"); ec) {
        std::println("[WS] Transmission error: {}", ec.message());
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));


    session.close();

    if (reader.joinable()) reader.join();

    return 0;
}