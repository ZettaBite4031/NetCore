#pragma once

#include "netcore/http_transport.hpp"
#include "netcore/err.hpp"
#include "netcore/url.hpp"
#include <memory>
#include <string>
#include <expected>


namespace NetCore {

    class RedirectingTransport : public IHttpTransport {
    public:
        RedirectingTransport(std::shared_ptr<IHttpTransport> inner, RedirectPolicy policy = {});

        std::expected<HttpResponse, std::error_code> send_request(const HttpRequest& req, const RequestOptions& opt) override;

    private:
        std::shared_ptr<IHttpTransport> m_Inner;
        RedirectPolicy m_Policy;

        static bool is_redirect(int status) { return status == 301 || status == 302 || status == 303 || status == 307 || status == 308; }

        static const HttpHeader* find_header(const std::vector<HttpHeader>& hdrs, std::string_view name);
    };

} // namespace NetCore