#include "log.hpp"

#include <cstdio>
#include <fstream>
#include <deque>
#include <thread>
#include <condition_variable>

namespace NetCore {

    class Iso8601Formatter : public ILogFormatter {
    public:
        std::string format(const LogRecord& rec) override;
    };

    class StderrSink : public ILogSink {
    public:
        explicit StderrSink(std::shared_ptr<ILogFormatter> fmt = std::make_shared<Iso8601Formatter>())
            : m_Fmt(std::move(fmt)) {}
        void write(const LogRecord& rec) override;
    private:
        std::shared_ptr<ILogFormatter> m_Fmt;
    };

    class RotatingFileSink : public ILogSink {
    public: 
        RotatingFileSink(std::string path, size_t max_bytes, int max_files,
                         std::shared_ptr<ILogFormatter> fmt = std::make_shared<Iso8601Formatter>());
        void write(const LogRecord& rec) override;
    private:
        void rotate_if_needed(size_t append_len);
        std::string m_Base;
        size_t m_MaxBytes;
        int m_MaxFiles;
        std::ofstream out;
        std::shared_ptr<ILogFormatter> m_Fmt;
        size_t m_Size{ 0 };
    };

    class AsyncSink : public ILogSink {
    public:
        explicit AsyncSink(std::shared_ptr<ILogSink> inner, size_t queue_max = 4096);
        ~AsyncSink();
        void write(const LogRecord& rec) override;
    private:
        std::shared_ptr<ILogSink> m_Inner;
        std::mutex m_Mtx;
        std::condition_variable m_CV;
        std::deque<LogRecord> m_Deque;
        const size_t m_Qmax;
        bool m_Stop = false;
        std::thread m_Worker;
    };

} // namespace NetCore