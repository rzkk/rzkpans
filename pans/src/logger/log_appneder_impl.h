#ifndef PANS_SRC_LOGGER_APPNEDER_IMPL_H
#define PANS_SRC_LOGGER_APPNEDER_IMPL_H

#include <atomic>
#include<cstdio>

#include<mutex>
#include<string_view>
#include<string>


#include<pans/logger/log_appender.h>

namespace pans
{
// 是StdoutAppender FileAppender 的父类
class Appender::Impl{ // 在class Appender 中声明 ，在外面定义实现
public:
    virtual ~Impl() = default;

    //写日志
    void append(LogLevel::Level level , std::string_view formatted_record) noexcept;

    void setLevel(LogLevel::Level level)noexcept;

    LogLevel::Level getLevel() const noexcept;
    



private:
    std::atomic<LogLevel::Level>m_level{LogLevel::Level::LOG_LV_DEBUG};
}









} // namespace pans











#endif