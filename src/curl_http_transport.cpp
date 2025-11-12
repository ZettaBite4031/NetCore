#include "curl_http_transport.hpp"

#include <curl/curl.h>
#include <sstream>
#include <system_error>
#include <cstring>

namespace NetCore {

    std::mutex CurlHttpTransport::m_InitMutex;
    int CurlHttpTransport::m_InitCount = 0;

    static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* user_data) {
        auto* stream = static_cast<std::string*>(user_data);
        stream->append(ptr, size * nmemb);
        return size * nmemb;
    }

    static size_t header_callback(char* buffer, size_t size, size_t nitems, void* user_data) {
        auto* headers = static_cast<std::vector<HttpHeader>*>(user_data);
        std::string_view line(buffer, size * nitems);

        auto pos = line.find(':');
        if (pos != std::string_view::npos) {
            std::string name(line.substr(0, pos));
            std::string value(line.substr(pos + 1));

            auto trim = [](std::string& s) {
                while (!s.empty() && isspace(s.front())) s.erase(s.begin());
                while (!s.empty() && isspace(s.back())) s.pop_back();
            };
            trim(name);
            trim(value);
            headers->push_back({std::move(name), std::move(value)});
        }
        return size * nitems;
    }

    CurlHttpTransport::CurlHttpTransport() {
        std::scoped_lock lock(m_InitMutex);
        if (m_InitCount++ == 0) curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    CurlHttpTransport::~CurlHttpTransport() {
        std::scoped_lock lock(m_InitMutex);
        if (--m_InitCount == 0) curl_global_cleanup();
    }

    std::expected<HttpResponse, std::error_code> CurlHttpTransport::send_request(const HttpRequest& request, const RequestOptions& opt) {
        CURL* curl = curl_easy_init();
        if (!curl) return std::unexpected(std::make_error_code(std::errc::not_enough_memory));

        std::string response_body;
        std::vector<HttpHeader> response_headers;

        struct curl_slist* curl_headers = nullptr;
        for (auto& h : request.headers) {
            std::string header_line = h.name + ":" + h.value;
            curl_headers = curl_slist_append(curl_headers, header_line.c_str());
        }

        CURLcode code;
        long status = 0;

        curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_headers);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "NetCore-CurlHttpTransport/1.0");
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, static_cast<long>(opt.connect_timeout.count()));
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, static_cast<long>(opt.read_timeout.count()));

        if (request.method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.body.size());
        } else if (request.method != "GET") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.method.c_str());
            if (!request.body.empty()) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.body.size());
            }
        }

        code = curl_easy_perform(curl);

        HttpResponse res;
        if (code == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
            res.status = static_cast<int>(status);
            res.headers = std::move(response_headers);
            res.body = std::move(response_body);
        }

        curl_slist_free_all(curl_headers);
        curl_easy_cleanup(curl);

        if (code != CURLE_OK) return std::unexpected(std::make_error_code(std::errc::io_error));
        return res;
    }

} // namespace NetCore