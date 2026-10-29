#ifndef PANS_INCLUDE_PANS_LOGGER_APPENDER_H
#define PANS_INCLUDE_PANS_LOGGER_APPENDER_H


#include<memory>
#include<string>

#include<pans/export.h>
#include<pans/logger/log_level.h>

namespace pans {

namespace detail{

//用来代理我们的内部权限，严格控制谁能访问我们的Appender私有实现
//接口与实现隔离 ， 暴露在外面的 只有接口
class AppenderAccess;  // 这里只做前置声明 
}; // namespace pans::detail

class PANS_API Appender final{
public:
    class Impl;  //  impl 指向实现 //Impl  是 实现类 ， class Appender 只是暴露给外面的接口类
    ~Appender() = default;

    Appender(const Appender&) = delete;
    Appender& operator= (const Appender&) = delete;
    Appender(Appender&&) = delete;
    Appender& operator=(Appender&&) = delete;

    //不同Appender有不同的日志等级
    void setLevel(LogLevel::Level level) noexcept;
    LogLevel::Level getLevel() const noexcept;

    void flush();  //日志缓冲区(进程流的缓冲区) -> 操作系统的脏页  
    void sync(); // 操作系统脏页 -> 磁盘缓冲区


private:
    explicit Appender(std::unique_ptr<Impl> impl ) noexcept;
    std::unique_ptr<Impl> m_impl;
    friend class detail::AppenderAccess;
}; //end of Appender

using AppenderPtr = std::shared_ptr<Appender>;

// ? 这里不是全局函数吗
PANS_API AppenderPtr MakeStdoutAppender();
PANS_API AppenderPtr MakeFileAppender(std::string file_name);


} // namespace pans 
#endif 