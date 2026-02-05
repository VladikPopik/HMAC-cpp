#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace service::logging
{

    enum class LogLevel
    {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3,
        NONE = 4
    };

    class Logger
    {
    public:
        Logger(const std::string &filename = "", LogLevel level = LogLevel::INFO);

        ~Logger() = default;

        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;

        Logger(Logger &&) = delete;
        Logger &operator=(Logger &&) = delete;

        void debug(const std::string &message);
        void info(const std::string &message);
        void warn(const std::string &message);
        void error(const std::string &message);

        void setLogLevel(LogLevel level);
        void setOutputToConsole(bool enable);
        void setOutputToFile(bool enable);

        class Timer
        {
        public:
            Timer(Logger &logger, const std::string &operation);
            ~Timer();

            Timer(const Timer &) = delete;
            Timer &operator=(const Timer &) = delete;

        private:
            Logger &logger_;
            std::string operation_;
            std::chrono::time_point<std::chrono::high_resolution_clock> start_;
        };

    private:
        void log(LogLevel level, const std::string &message);
        std::string getCurrentTimestamp() const;
        std::string levelToString(LogLevel level) const;

        std::unique_ptr<std::ofstream> file_stream_;
        std::mutex log_mutex_;
        LogLevel current_level_;
        bool output_to_console_;
        bool output_to_file_;
        std::string filename_;
    };

#define LOG_DEBUG(logger, msg) logger.debug(msg)
#define LOG_INFO(logger, msg) logger.info(msg)
#define LOG_WARN(logger, msg) logger.warn(msg)
#define LOG_ERROR(logger, msg) logger.error(msg)
#define LOG_TIMER(logger, operation) Logger::Timer timer##__LINE__(logger, operation)

}