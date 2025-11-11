#pragma once

#include <string>
#include <vector>
#include <utility>

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

} // namespace NetCore