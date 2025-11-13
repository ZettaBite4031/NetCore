#include "redirecting_transport.hpp"


namespace NetCore {
    
    const HttpHeader* RedirectingTransport::find_header(const std::vector<HttpHeader>& hdrs, std::string_view name) {
        for (auto& h : hdrs) {
            if (std::equal(h.name.begin(), h.name.end(), name.begin(), name.end(), [](char a, char b){ return std::tolower(a)==std::tolower(b); })) return &h;
        }
        return nullptr;
    }

    RedirectingTransport::RedirectingTransport(std::shared_ptr<IHttpTransport> inner, RedirectPolicy policy)
        : m_Inner(std::move(inner)), m_Policy(policy) {}

    std::expected<HttpResponse, std::error_code> RedirectingTransport::send_request(const HttpRequest& original, const RequestOptions& opt) {
        HttpRequest req = original;
        int redirects = m_Policy.max_redirects;

        while (true) {
            auto res = m_Inner->send_request(req, opt);
            if (!res) return res;

            if (!is_redirect(res->status)) return res;

            if (redirects-- <= 0) return std::unexpected(errc::http_protocol_error);

            auto loc = find_header(res->headers, "Location");
            if (!loc) return res; 

            std::string new_url;
            if (loc->value.rfind("http://", 0) == 0 || loc->value.rfind("https://", 0) == 0) new_url = loc->value;
            else {
                auto parsed = parse_url(req.url);
                if (!parsed) return std::unexpected(errc::invalid_url);
                new_url = parsed->scheme + "://" + parsed->host;
                if (parsed->port != "80" && parsed->port != "443") new_url += ":" + parsed->port;
                if (!loc->value.empty() && loc->value[0] == '/') new_url += loc->value;
                else new_url += "/" + loc->value;
            }

            if (res->status == 303 && m_Policy.allow_post_to_get_on_303 && req.method == "POST") {
                req.method = "GET";
                req.body.clear();
            }

            req.url = std::move(new_url);
        }
    }

}