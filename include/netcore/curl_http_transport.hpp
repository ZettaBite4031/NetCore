#pragma once

#include "http_transport.hpp"
#include <memory>
#include <mutex>

namespace NetCore {

    class CurlHttpTransport : public IHttpTransport {
    public:
        CurlHttpTransport();
        ~CurlHttpTransport() override;

        std::expected<HttpResponse, std::error_code> send_request(const HttpRequest& request) override;

    private:
        static std::mutex m_InitMutex;
        static int m_InitCount;
    };

} // namespace NetCore