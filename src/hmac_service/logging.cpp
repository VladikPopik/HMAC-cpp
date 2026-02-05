#include "logging.hpp"

namespace service::logging
{

    Logger::Logger(const std::string &filename, LogLevel level)
        : current_level_(level), output_to_console_(true), output_to_file_(!filename.empty()), filename_(filename)
    {
        if (output_to_file_ && !filename_.empty())
        {
            file_stream_ = std::make_unique<std::ofstream>(filename_, std::ios::app);
            if (!file_stream_->is_open())
            {
                std::cerr << "Failed to open log file: " << filename_ << std::endl;
                output_to_file_ = false;
            }
        }
    }

    void Logger::log(LogLevel level, const std::string &message)
    {
        if (level < current_level_)
            return;

        std::lock_guard<std::mutex> lock(log_mutex_);

        std::stringstream log_entry;
        log_entry << getCurrentTimestamp() << " ["
                  << levelToString(level) << "] "
                  << message;

        if (output_to_console_)
        {
            switch (level)
            {
            case LogLevel::ERROR:
                std::cerr << "\033[1;31m" << log_entry.str() << "\033[0m" << std::endl;
                break;
            case LogLevel::WARN:
                std::cerr << "\033[1;33m" << log_entry.str() << "\033[0m" << std::endl;
                break;
            case LogLevel::INFO:
                std::cout << "\033[1;32m" << log_entry.str() << "\033[0m" << std::endl;
                break;
            default:
                std::cout << log_entry.str() << std::endl;
            }
        }

        if (output_to_file_ && file_stream_ && file_stream_->is_open())
        {
            (*file_stream_) << log_entry.str() << std::endl;
            file_stream_->flush();
        }
    }

    std::string Logger::getCurrentTimestamp() const
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::string Logger::levelToString(LogLevel level) const
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    void Logger::debug(const std::string &message)
    {
        log(LogLevel::DEBUG, message);
    }

    void Logger::info(const std::string &message)
    {
        log(LogLevel::INFO, message);
    }

    void Logger::warn(const std::string &message)
    {
        log(LogLevel::WARN, message);
    }

    void Logger::error(const std::string &message)
    {
        log(LogLevel::ERROR, message);
    }

    void Logger::setLogLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        current_level_ = level;
    }

    void Logger::setOutputToConsole(bool enable)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        output_to_console_ = enable;
    }

    void Logger::setOutputToFile(bool enable)
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        output_to_file_ = enable;
    }

    Logger::Timer::Timer(Logger &logger, const std::string &operation)
        : logger_(logger), operation_(operation), start_(std::chrono::high_resolution_clock::now())
    {
        logger_.debug("Starting: " + operation_);
    }

    Logger::Timer::~Timer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
        logger_.debug("Completed: " + operation_ + " (" + std::to_string(duration.count()) + " ms)");
    }

}