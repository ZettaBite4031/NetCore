#pragma once

/*
    - Might pull this out into its own library at some point
*/

#include <cstdint>
#include <string>
#include <string_view>
#include <initializer_list>
#include <chrono>
#include <memory>
#include <atomic>
#include <vector>
#include <mutex>

#ifndef NETCORE_LOGGING_ENABLED
#define NETCORE_LOGGING_ENABLED 1
#endif

namespace NetCore {

    enum class LogLevel : uint8_t {
        trace=0,
        debug=1,
        info=2,
        warn=3,
        error=4,
        critical=5,
        off=6,
    };

    struct LogField {
        std::string key;
        std::string value;
    };

    using LogFields = std::initializer_list<LogField>;

    struct LogRecord {
        std::chrono::system_clock::time_point ts;
        LogLevel level;
        std::string logger;
        std::string msg;
        std::vector<LogField> fields;
        uint64_t tid;
    };

    class ILogSink {
    public:
        virtual ~ILogSink() = default;
        virtual void write(const LogRecord& rec) = 0;
    };

    class ILogFormatter {
    public:
        virtual ~ILogFormatter() = default;
        virtual std::string format(const LogRecord& rec) = 0;
    };

    class Logger {
    public:
        explicit Logger(std::string name);
        ~Logger();

        const std::string& name() const noexcept { return m_Name; }

        void set_level(LogLevel lvl) { m_Level.store(lvl, std::memory_order_relaxed); }
        LogLevel level() const { return m_Level.load(std::memory_order_relaxed); }
        bool should_log(LogLevel lvl) const { return static_cast<int>(lvl) >= static_cast<int>(level()); }

        void log(LogLevel lvl, std::string_view msg, LogFields fields = {});

        void add_sink(std::shared_ptr<ILogSink> sink);
        void clear_sinks();

        void set_redacted_keys(std::vector<std::string> keys);

    private:
        std::string m_Name;
        std::atomic<LogLevel> m_Level{ LogLevel::info };
        std::vector<std::shared_ptr<ILogSink>> m_Sinks;
        std::vector<std::string> m_RedactedKeys;
        std::mutex m_SinksMtx;
    };

    Logger& get_logger(std::string_view name);


#if NETCORE_LOGGING_ENABLED
#define NC_LOG(logger, lvl, msg, ...) do {\
    if ((logger).should_log(lvl)) (logger).log(lvl, (msg), __VA_ARGS__); \
} while(0)
#else
#define NC_LOG(logger, lvl, msg, ...) do {} while(0)
#endif

#define NC_LOG_TRACE(lg, msg, ...)   NC_LOG(lg, ::NetCore::LogLevel::trace,   msg, __VA_ARGS__)
#define NC_LOG_DEBUG(lg, msg, ...)   NC_LOG(lg, ::NetCore::LogLevel::debug,   msg, __VA_ARGS__)
#define NC_LOG_INFO(lg,  msg, ...)   NC_LOG(lg, ::NetCore::LogLevel::info,    msg, __VA_ARGS__)
#define NC_LOG_WARN(lg,  msg, ...)   NC_LOG(lg, ::NetCore::LogLevel::warn,    msg, __VA_ARGS__)
#define NC_LOG_ERROR(lg, msg, ...)   NC_LOG(lg, ::NetCore::LogLevel::error,   msg, __VA_ARGS__)
#define NC_LOG_CRIT(lg,  msg, ...)   NC_LOG(lg, ::NetCore::LogLevel::critical,msg, __VA_ARGS__)


} // namespace NetCore