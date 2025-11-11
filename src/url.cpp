#include "netcore/url.hpp"

namespace NetCore {

    std::optional<ParsedUrl> parse_url(std::string_view url) {
        auto pos_scheme = url.find("://");
        if (pos_scheme == std::string_view::npos) return std::nullopt;

        ParsedUrl parsed;
        parsed.scheme = std::string(url.substr(0, pos_scheme));
        auto rest = url.substr(pos_scheme + 3);

        auto pos_path = rest.find('/');
        std::string_view hostport = rest;
        std::string_view path = "/";
        if (pos_path != std::string_view::npos) {
            hostport = rest.substr(0, pos_path);
            path = rest.substr(pos_path);
        }
        parsed.target = std::string(path);

        auto pos_colon = hostport.find(':');
        if (pos_colon != std::string_view::npos) {
            parsed.host = std::string(hostport.substr(0, pos_colon));
            parsed.port = std::string(hostport.substr(pos_colon + 1));
        } else {
            parsed.host = std::string(hostport);
            parsed.port = (parsed.scheme == "https") ? "443" : "80";
        }

        return parsed;
    }

} // namespace NetCore