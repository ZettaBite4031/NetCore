#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace NetCore {

    struct ParsedUrl {
        std::string scheme;
        std::string host;
        std::string port;
        std::string target;
    };

    std::optional<ParsedUrl> parse_url(std::string_view url);

} // namespace NetCore