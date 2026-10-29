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

    void flush();
    void sync();

    std::mutex m_mutex;  // C++20 对std::mutex 有优化

protected:
    //在上层加锁， 写不加锁
    virtual void writeUnlocked(std::string_view formatted_record) noexcept = 0;
    virtual void flushUnlocked() noexcept = 0;
    virtual void syncUnlocked() noexcept = 0;


private:
    std::atomic<LogLevel::Level>m_level{LogLevel::Level::LOG_LV_DEBUG};
};

namespace detail{
// 控制appender 的访问权限
// 创建appender 
// 奇异递归模式 
class AppenderAccess final{
public:
    [[nodiscard]] static AppenderPtr MakeStdoutAppender();
    [[nodiscard]] static AppenderPtr MakeFileAppender(std::string file_name);
    // 写日志接口 
    static void Append(const AppenderPtr & appender , LogLevel::Level level , std::string_view formatted_record)noexcept;
};

    
} // namespace detail








} // namespace pans











#endif