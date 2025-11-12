#pragma once

#include <string>
#include <vector>
#include <utility>
#include <chrono>

namespace NetCore {
    
    struct HttpHeader {
        std::string name;
        std::string value;
    };

    struct HttpRequest {
        std::string method;
        std::string url;
        std::vector<HttpHeader> headers;
        std::string body;
    };

    struct HttpResponse {
        int status = 0;
        std::vector<HttpHeader> headers;
        std::string body;
    };

    struct RequestOptions {
        std::chrono::milliseconds connect_timeout   { 0 };
        std::chrono::milliseconds read_timeout      { 0 };
        std::vector<HttpHeader> extra_headers       {};
    };

    struct ClientConfig {
        std::chrono::milliseconds default_connect_timeout{5000};
        std::chrono::milliseconds default_read_timeout{10000};
    };

} // namespace NetCore