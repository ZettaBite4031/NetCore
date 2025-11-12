#include "netcore/url.hpp"

#include <regex>

namespace NetCore {

    std::optional<ParsedUrl> parse_url(std::string_view url) {
        static const std::regex re(
            R"(^(\w+)://([^/:]+)(?::(\d+))?(.*)$)",
            std::regex::ECMAScript
        );

        std::smatch match;
        std::string url_s{ url };
        if (!std::regex_match(url_s, match, re)) return std::nullopt;

        ParsedUrl result;
        result.scheme = match[1].str();
        result.host   = match[2].str();
        result.port   = match[3].matched ? match[3].str() : "";
        result.target = match[4].matched && !match[4].str().empty() ? match[4].str() : "/";
        
        if (result.port.empty()) {
            if (result.scheme == "http" || result.scheme == "ws") result.port = "80";
            if (result.scheme == "https" || result.scheme == "wss") result.port = "443";
        }

        return result;
    }

} // namespace NetCore