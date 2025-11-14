#pragma once

#include <string>
#include <string_view>
#include <expected>
#include <system_error>


namespace NetCore {

    class IDecompressor {
    public:
        virtual ~IDecompressor() = default;
        virtual std::expected<std::string, std::error_code> feed(std::string_view compressed, bool message_end) = 0;
        virtual void reset() = 0;
    };

    class ZlibStreamDecompressor : public IDecompressor {
    public: 
        ZlibStreamDecompressor();
        ~ZlibStreamDecompressor();

        std::expected<std::string, std::error_code> feed(std::string_view compressed, bool message_end) override;
        void reset() override;

    private:
        struct Impl;
        Impl* m_Impl;
    };

} // namespace NetCore