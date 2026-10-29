#ifndef PANS_INCLUDE_PANS_LOGGER_LOGGER_H
#define PANS_INCLUDE_PANS_LOGGER_LOGGER_H

#include <memory>
#include <string>
#include <string_view>

#include <pans/export.h>
#include <pans/logger/log_appender.h>
#include <pans/logger/log_level.h>

namespace pans {

namespace detail {
class LoggerAccess;
}

class PANS_API Logger final
{
public:
    class Impl;
    explicit Logger(std::string name = "root"); //日志器 名字
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    [[nodiscard]] bool shouldLog(LogLevel::Level level) const noexcept;// 要不要构造这条日志

    void setLevel(LogLevel::Level level) noexcept;
    [[nodiscard]] LogLevel::Level getLevel() const noexcept;
    [[nodiscard]] std::string_view getName() const noexcept;

    void setFormatter(std::string_view pattern); // 不同的logger 可以不同的formatter , 比如写入file 的就要尽可能详细 ， 网络发出去的就不需要知道协程号之类的信息
    [[nodiscard]] std::string getFormatterPattern() const;
    void addAppender(AppenderPtr appender);
    void removeAppender(const AppenderPtr& appender);
    void clearAppenders();

    void flush();
    void sync();
private:
    std::unique_ptr<Impl> m_impl;
    friend class detail::LoggerAccess;
};

using LoggerPtr = std::shared_ptr<Logger>;
[[nodiscard]] PANS_API LoggerPtr GetRootLogger();
[[nodiscard]] PANS_API LoggerPtr GetLogger(std::string_view name); // 根据名字获取

} // namespace pans

#endif // PANS_INCLUDE_PANS_LOGGER_LOGGER_H
