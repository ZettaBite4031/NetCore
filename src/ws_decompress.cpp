#include "netcore/ws_decompress.hpp"

#include <zlib.h>
#include <vector>


namespace NetCore {

    static bool looks_like_zlib(std::string_view s) {
        if (s.size() < 2) return false;

        unsigned char cmf = static_cast<unsigned char>(s[0]);
        unsigned char flg = static_cast<unsigned char>(s[1]);

        if ((cmf & 0x0F) != 0) return false;
        if (((cmf << 8) + flg) % 31 != 0) return false;
        if (flg & 0x20) return false;
        return true; 
    }

    struct ZlibStreamDecompressor::Impl {
        z_stream zs{};
        bool init = false;
    };

    ZlibStreamDecompressor::ZlibStreamDecompressor() 
        : m_Impl(new Impl) {};
    
    ZlibStreamDecompressor::~ZlibStreamDecompressor() {
        if (m_Impl->init) inflateEnd(&m_Impl->zs);
        delete m_Impl;
    }

    void ZlibStreamDecompressor::reset() {
        if (m_Impl->init) {
            inflateEnd(&m_Impl->zs);
            m_Impl->init = false;
        }
    }

    std::expected<std::string, std::error_code> ZlibStreamDecompressor::feed(std::string_view compressed, bool message_end) {
        if (!looks_like_zlib(compressed)) return std::string(compressed);
        if (!m_Impl->init) {
            m_Impl->zs = {};
            int ret = inflateInit(&m_Impl->zs);
            if (ret != Z_OK) return std::unexpected(std::make_error_code(std::errc::io_error));
            m_Impl->init = true;
        }

        std::string out;
        out.reserve(compressed.size() * 2);

        m_Impl->zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed.data()));
        m_Impl->zs.avail_in = static_cast<uInt>(compressed.size());

        char buf[4096];
        int flush = Z_NO_FLUSH;

        do {
            m_Impl->zs.next_out = reinterpret_cast<Bytef*>(buf);
            m_Impl->zs.avail_out = sizeof(buf);
            int ret = inflate(&m_Impl->zs, flush);
            if (ret == Z_STREAM_END) {
                out.append(buf, sizeof(buf) - m_Impl->zs.avail_out);
                if (message_end) reset();
                break;
            }
            if (ret != Z_OK) return std::unexpected(std::make_error_code(std::errc::io_error));
            out.append(buf, sizeof(buf) - m_Impl->zs.avail_out);
            if (m_Impl->zs.avail_in == 0 && m_Impl->zs.avail_out == sizeof(buf)) break;
        } while (m_Impl->zs.avail_out == 0);

        return out;
    }

} // namespace NetCore