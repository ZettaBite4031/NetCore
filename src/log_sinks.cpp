#include "netcore/log_sinks.hpp"
#include <iomanip>
#include <filesystem>


namespace NetCore {

    static const char* level_name(LogLevel l) {
        switch (l) {
        case LogLevel::trace: return "TRACE";
        case LogLevel::debug: return "DEBUG";
        case LogLevel::info:  return "INFO";
        case LogLevel::warn:  return "WARN";
        case LogLevel::error: return "ERROR";
        case LogLevel::critical: return "CRIT";
        case LogLevel::off: return "OFF";
        }
        return "?";
    }

    std::string Iso8601Formatter::format(const LogRecord& rec) {
        using namespace std::chrono;
        auto t = system_clock::to_time_t(rec.ts);
        auto ms = duration_cast<milliseconds>(rec.ts.time_since_epoch()).count() % 1000;

        std::tm tm{};

#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << ms;
        oss << " [" << level_name(rec.level) << "] ";
        oss << rec.logger << " tid=" << rec.tid;
        oss << " - " << rec.msg;

        for (auto& f : rec.fields) {
            oss << ' ' << f.key << "=";

            bool need_quote = f.value.find_first_of(" \t\r\n") != std::string::npos;
            if (need_quote) oss << '"';
            oss << f.value;
            if (need_quote) oss << '"';
        }
        return oss.str();
    }

    void StderrSink::write(const LogRecord& rec) {
        auto s = m_Fmt->format(rec);
        std::fwrite(s.data(), 1, s.size(), stderr);
        std::fputc('\n', stderr);
        std::fflush(stderr);
    }

    RotatingFileSink::RotatingFileSink(std::string path, size_t max_bytes, int max_files, std::shared_ptr<ILogFormatter>fmt)
        : m_Base(std::move(path)), m_MaxBytes(max_bytes), m_MaxFiles(max_files), m_Fmt(std::move(fmt)) {
        out.open(m_Base, std::ios::app | std::ios::binary);
        m_Size = std::filesystem::exists(m_Base) ? std::filesystem::file_size(m_Base) : 0;
    }     
    
    void RotatingFileSink::rotate_if_needed(size_t append_len) {
        if (m_Size + append_len <= m_MaxBytes) return;

        out.close();
        for (int i = m_MaxFiles-1; i >= 1; i++) {
            std::filesystem::path from = m_Base + "." + std::to_string(i);
            std::filesystem::path to   = m_Base + "." + std::to_string(i+1);
            if (std::filesystem::exists(from)) {
                std::error_code ec;
                std::filesystem::rename(from, to, ec);
            }
        }

        if (std::filesystem::exists(m_Base)) {
            std::error_code ec;
            std::filesystem::rename(m_Base, m_Base + ".1", ec);
        }

        out.open(m_Base, std::ios::trunc | std::ios::binary);
        m_Size = 0;
    }

    void RotatingFileSink::write(const LogRecord& rec) {
        auto s = m_Fmt->format(rec);
        rotate_if_needed(s.size()+1);
        out << s << '\n';
        out.flush();
        m_Size += s.size()+1;
    }

    AsyncSink::AsyncSink(std::shared_ptr<ILogSink> inner, size_t qmax) 
        : m_Inner(std::move(inner)), m_Qmax(qmax) {
        m_Worker = std::thread([this]{
            std::unique_lock lock(m_Mtx);
            for (;;) {
                m_CV.wait(lock, [&]{ return m_Stop || !m_Deque.empty(); });
                if (m_Stop && m_Deque.empty()) break;
                auto rec = std::move(m_Deque.front());
                m_Deque.pop_front();
                lock.unlock();
                m_Inner->write(rec);
                lock.lock();
            }
        });
    }

    AsyncSink::~AsyncSink() {
        {
            std::lock_guard lock(m_Mtx);
            m_Stop = true;
        }
        m_CV.notify_all();
        if (m_Worker.joinable()) m_Worker.join();
    }

    void AsyncSink::write(const LogRecord& rec) {
        std::lock_guard lock(m_Mtx);
        if (m_Deque.size() >= m_Qmax) return;
        m_Deque.push_back(rec);
        m_CV.notify_one();
    }

} // namespace NetCore