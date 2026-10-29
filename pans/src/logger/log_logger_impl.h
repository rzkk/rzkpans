#ifndef PANS_SRC_LOGGER_LOGGER_IMPL_H
#define PANS_SRC_LOGGER_LOGGER_IMPL_H

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include <pans/logger/log_logger.h>

#include "logger/log_formatter.h"
#include "logger/log_record.h"

namespace pans {

class Logger::Impl
{
public:
    explicit Impl(std::string name);
    [[nodiscard]] bool shouldLog(LogLevel::Level level) const noexcept;// 大于logger的level , 并且logger 需要有 appender 
    void submit(const detail::LogRecordView& record) noexcept; // 构造好的日志 提交到 APPENDER 
    void setLevel(LogLevel::Level level) noexcept;
    [[nodiscard]] LogLevel::Level getLevel() const noexcept;
    [[nodiscard]] std::string_view getName() const noexcept;
    void setFormatter(std::shared_ptr<const detail::Formatter> formatter);
    [[nodiscard]] std::shared_ptr<const detail::Formatter> getFormatter() const noexcept;
    void addAppender(AppenderPtr appender);
    void removeAppender(const AppenderPtr& appender);
    void clearAppenders();
    
    void flush();
    void sync();

    void setRoot(const LoggerPtr& root) noexcept;
private:
    std::string m_name;
    std::atomic<LogLevel::Level> m_level{LogLevel::Level::LOG_LV_DEBUG};

    mutable std::shared_mutex m_mutex;
    std::shared_ptr<const detail::Formatter> m_formatter;
    std::vector<AppenderPtr> m_appenders;

    LoggerPtr m_root;
};

namespace detail {
/**
 * LoggerAccess本质上是一个内部权限访问的桥梁，
 * 用于在不扩大Logger公共API的情况下，访问其私有实现
 */
class LoggerAccess final
{
public:
    static void Submit(Logger& logger, const LogRecordView& record) noexcept;
    static void SetRoot(Logger& logger, const LoggerPtr& root) noexcept;
};

} // namespace detail
} // namespace pans

#endif // PANS_SRC_LOGGER_LOGGER_IMPL_H
