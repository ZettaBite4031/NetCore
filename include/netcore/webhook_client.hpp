#pragma once

#include "http_transport.hpp"
#include "http_types.hpp"
#include <memory>
#include <expected>
#include <system_error>
#include <string>

namespace NetCore {

    class WebhookClient {
    public:
        explicit WebhookClient(std::shared_ptr<IHttpTransport> http);

        std::expected<void, std::error_code> post_json(std::string url, std::string json);

    private:
        std::shared_ptr<IHttpTransport> m_Transport;
    };

} // namespace NetCore