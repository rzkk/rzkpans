#ifndef _PANS_SRC_LOGGER_FORMATTER_H
#define _PANS_SRC_LOGGER_FORMATTER_H

#include <memory>
#include<string>
#include<string_view>
#include<vector>

#include"logger/log_buffer.h"
#include"logger/log_buffer_config.h"
#include"logger/log_record.h"


namespace pans::detail
{
//当日志比较长的时候， InlineBuffer 使用了 vector  , 会有堆上分配内存 ， 有可能抛异常  // 一点内存都分不出来的时候
// 所以没法 保证下面的类 不抛异常
using FormatterRecordBuffer = InlineBuffer<FORMATTED_RECORD_INLINE_CAPACITY>;

//final : 表示类不能被继承
class Formatter final{
public:
    // 面向
    class FormatItem   // 日志的每个组成部分 都由一个 FormatItem 的子类表示 ， FormatItem 定义统一接口
    {
    public:
        //具体干活的 ， 负责将record 内容传递给 buffer 中
        virtual void format(const LogRecordView& record , FormatterRecordBuffer & output)const  = 0;
        virtual ~FormatItem() = default; // 定义虚析构
    //private:
        /* data */
    };

public:
    explicit Formatter(std::string_view pattern);

    void format(const LogRecordView& recode ,  FormatterRecordBuffer& output)const ;

    [[nodiscard]] const std::string & getPanttern() const noexcept{  //todo  返回值 也是 const 
        return m_pattern;
    };
private:
    int parse() ;  // 解析pattern 
    void addLiteral(std::string & literal); // 在pattern 中插入的字面数据 ，当成literal插入

private:
    std::string m_pattern = ""; // 通过配置传进来的 ， 或者 默认值 
    std::vector<std::unique_ptr<FormatItem>> m_items;
};





} // namespace pans::detail




#endif 