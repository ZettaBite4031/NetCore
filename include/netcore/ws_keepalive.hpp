#pragma once

#include <chrono>


namespace NetCore {

    struct KeepaliveConfig {
        std::chrono::milliseconds ping_interval{ 30'000 };
        std::chrono::milliseconds pong_timeout{ 10'000 };
        bool enabled = true;
    };

} // namespace NetCore