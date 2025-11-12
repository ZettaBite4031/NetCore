#include "netcore/log.hpp"

#include <chrono>
#include <thread>
#include <sstream>

#if defined(_WIN32)
#include <windows.h>
static uint64_t tid_hash() { return static_cast<uint64_t>(::GetCurrentThreadId()); }
#else
#include <pthread.h>
static uint64_t tid_hash() { return static_cast<uint64_t>(::pthread_self()); }
#endif

namespace NetCore {

    static std::mutex registry_mutex;
    static std::vector<std::unique_ptr<Logger>> registry;

    Logger::Logger(std::string name) : m_Name (std::move(name)) {}
    Logger::~Logger() = default;

    void Logger::add_sink(std::shared_ptr<ILogSink> sink) {
        std::lock_guard lock(m_SinksMtx);
        m_Sinks.push_back(std::move(sink));
    }

    void Logger::clear_sinks() {
        std::lock_guard lock(m_SinksMtx);
        m_Sinks.clear();
    }

    void Logger::set_redacted_keys(std::vector<std::string> keys) {
        std::lock_guard lock(m_SinksMtx);
        m_RedactedKeys = std::move(keys);
    }

    void Logger::log(LogLevel lvl, std::string_view msg, LogFields fields) {
#if NETCORE_LOGGING_ENABLED
        LogRecord rec;
        rec.ts = std::chrono::system_clock::now();
        rec.level = lvl;
        rec.logger = m_Name;
        rec.msg.assign(msg.begin(), msg.end());
        rec.tid = tid_hash();

        rec.fields.reserve(fields.size());
        for (auto& f : fields) {
            bool redact = false;
            for (auto& key : m_RedactedKeys) {
                if (f.key == key) { redact = true; break; }
            }
            rec.fields.push_back({f.key, redact ? "***" : f.value});
        }

        std::vector<std::shared_ptr<ILogSink>> sinks_copy;
        {
            std::lock_guard lock(m_SinksMtx);
            sinks_copy = m_Sinks;
        }
        for (auto& s : sinks_copy) s->write(rec);
#else
        (void)lvl; (void)msg; (void)fields;
#endif
    }

    Logger& get_logger(std::string_view name) {
        std::lock_guard lock(registry_mutex);
        for (auto& p : registry) if (p->name() == name) return *p;
        registry.emplace_back(std::make_unique<Logger>(std::string(name)));
        return *registry.back();
    }

} // namespace NetCore