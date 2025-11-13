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

        std::string route_key{};
        std::string trace_id{};
    };

    struct ClientConfig {
        std::chrono::milliseconds default_connect_timeout{5000};
        std::chrono::milliseconds default_read_timeout{10000};
    };

    struct RedirectPolicy {
        int max_redirects = 5;
        bool allow_post_to_get_on_303 = true;
    };

    struct RetryPolicy {
        int max_attempts = 5;
        std::chrono::milliseconds base_delay{ 200 };
        bool retry_5xx = true;
        bool retry_429 = true;
        bool retry_network_errors = true;
    };

} // namespace NetCore
