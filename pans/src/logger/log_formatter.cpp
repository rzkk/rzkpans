#include"logger/log_formatter.h"

#include<unordered_map>

namespace pans::detail{

constexpr std::string_view DEFAULT_DATE_FORMAT = "%Y-%m-%d %H:%M:%S" ;

Formatter::Formatter(std::string_view pattern):m_pattern(pattern) {
    if(parse() != 0){// 日志解析失败， 无日志可用
        throw std::invalid_argument("invalid  logger formate pattern"); 
    }
}

void Formatter::format(const LogRecordView& record , FormattedRecordBuffer&output) const {
// 遍历得到的 m_items，将每一个都追加到 output buffer
    for(const auto & item :m_items){
        item->format(record , output);
    }

}

//std::to_string : 可能在堆上开内存 -> 有可能抛异常
template<class T>
void AppendInteger(FormattedRecordBuffer& output , T value) {
    std::array<char , 24> buffer{}; //栈上的 buffer 

    // 基本上不会失败，转失败了也没关系
    const auto result = std::to_chars(buffer.data() , buffer.data()+ buffer.size() , value);

    ASSERT_RETNONE2(result.ec == std::errc() , "trans" << value <<" to chars  failed" );
    output.append(buffer.data() , static_cast<std::size_t>(result.ptr - buffer.data()) );
}

class LiteralFormattItem final : public Formatter:: FormatItem{
public:
    explicit LiteralFormattItem(std::string_view value):m_value(value){  // ???????????? 目前不知道 调用方怎么写
    }  
    virtual void format(const LogRecordView&  , FormattedRecordBuffer & output)const  override{
        output.append(m_value);
    }
private:
    std::string m_value;
};

class MessageFormattItem final : public Formatter:: FormatItem{
public:
    virtual void format(const LogRecordView& record , FormattedRecordBuffer & output)const  override{
        output.append(record.m_message);
    }
};

class LevelFormattItem final : public Formatter:: FormatItem{
public:
    virtual void format(const LogRecordView& record , FormattedRecordBuffer & output)const  override{
        output.append(LogLevel::ToString(record.m_level));
    }
};

class ElapsedFormattItem final : public Formatter:: FormatItem{
public:
    virtual void format(const LogRecordView& record , FormattedRecordBuffer & output)const  override{
        //将整数贴进来
        AppendInteger(output , std::chrono::duration_cast<std::chrono::milliseconds>(record.m_elapsed).count());
    }
};

class LoggerNameFormattItem final : public Formatter:: FormatItem{
public:
    virtual void format(const LogRecordView& record , FormattedRecordBuffer & output)const  override{
        output.append(record.m_loggerName);
    }
};

class ThreadIdFormattItem final : public Formatter:: FormatItem{
public:
    virtual void format(const LogRecordView& record , FormattedRecordBuffer & output)const  override{
        AppendInteger(output , record.m_threadId);
    }
};

class NewLineFormattItem final : public Formatter:: FormatItem{  // 换行
public:
    virtual void format(const LogRecordView&  , FormattedRecordBuffer & output)const  override{
        output.append('\n');
    }
};

//有可能是有很多日志 在同一秒内打出来 ： 时间戳精确到秒的转换需要做很多次 -> 同一秒的日志的时间直接复用
class DataTimeFormattItem final : public Formatter:: FormatItem{
public:

    explicit DataTimeFormattItem(std::string_view format):m_format(format.empty() ? DEFAULT_DATE_FORMAT : format){  
    }  

    virtual void format(const LogRecordView& record , FormattedRecordBuffer & output)const  override{
        static thread_local time_t last_second = 0; // 保证每个线程都有一份
        static thread_local char cached_date_time[20] = {'\0'};

        const auto duration = record.m_timestamp.time_since_epoch();

        const time_t current_second = static_cast<time_t>(
                     std::chrono::duration_cast<std::chrono::seconds>(duration).count());
        
        if(current_second != last_second){
            std::tm buffer{};
#if defined(_WIN32)
            const errno_t result = localtime_s(buffer , & current_second);
            ASSERT_RETNONE2(result == 0 , "failed to convert log time to local time ");
#else   
            // localtime_r是线程安全的，但是消耗也大：
            //_r :表示线程安全的接口 ， 会有一个全局锁 ， 全局锁就有可能要排队
            // 需要进行复杂的时区计算 （ 特别是有夏令时变更历史的时区 ）
            // 如果系统没有加载时区信息或者要求响应时区变更, 每次调用这个接口还要去发起磁盘IO，读系统文件/etc/localtime
            const std::tm * result = localtime_r(&current_second , & buffer);
            ASSERT_RETNONE2(result != nullptr , "failed to convert log time to local time ");
#endif
            const std::size_t size = std::strftime(cached_date_time , sizeof(cached_date_time) ,
                                                    m_format.c_str() , & buffer);
            ASSERT_RETNONE2(size != 0 , "failed to format log time ");
            last_second = current_second;
        }

        output.append(cached_date_time);

    }
private:
    std::string m_format; // 格式的
};


class MicrosecondsFormattItem final : public Formatter:: FormatItem{
public:

    virtual void format(const LogRecordView& record , FormattedRecordBuffer & output)const  override{
        const auto total_microseconds = 
            std::chrono::duration_cast<std::chrono::microseconds>(record.m_timestamp.time_since_epoch()).count();
            std::int64_t microseconds = total_microseconds % 1000000;
            if(microseconds < 0 ) 
                microseconds += 1000000;
            // 秒内微秒必须补齐成固定六位，否则 12:00:00.1234 这样的时间戳无法解析，也无法按字典序排序。

            std::array<char , MICROSECONDS_WIDTH> digits{} ;
            
            auto remaining = static_cast <std::uint32_t>(microseconds);

            for(std::size_t index = digits.size() ; index -- >0 ;) {
                digits[index] = static_cast<char>('0' + remaining % 10);
                remaining /= 10;
            }

            output.append(digits.data() , digits.size());
    }
private:
    static constexpr std::size_t MICROSECONDS_WIDTH = 6; // 常量
};


class FileNameFormatItem final : public Formatter::FormatItem
{
public:
    virtual void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        output.append(record.m_fileName);
    }
};


class LineFormatItem final : public Formatter::FormatItem
{
public:
    virtual void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        AppendInteger(output , record.m_line);
    }
};


class TableFormatItem final : public Formatter::FormatItem
{
public:
    virtual void format(const LogRecordView& , FormattedRecordBuffer& output) const override
    {
        output.append('\t');
    }
};


class FiberIdFormatItem final : public Formatter::FormatItem
{
public:
    virtual void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        AppendInteger(output , record.m_fiberId);
    }
};


class ThreadNameFormatItem final : public Formatter::FormatItem
{
public:
    virtual void format(const LogRecordView&record , FormattedRecordBuffer& output) const override
    {
        output.append(record.m_thredName);
    }
};
//函数指针 
using FormatItemFactory = std::unique_ptr<Formatter::FormatItem> (*)(std::string_view);

//工厂函数 ; 产生 FormatItemFactory ，生产Item 
template<class Item>
std::unique_ptr<Formatter::FormatItem> CreateSimpleFormatItem(std::string_view){
    return std::make_unique<Item>();
}

template<class Item>
std::unique_ptr<Formatter::FormatItem> CreateConfiguredFormatItem(std::string_view format){
    return std::make_unique<Item>(format);  // format 给这个Item 初始化用的
}

const std::unordered_map<char , FormatItemFactory> & GetFormatItemFactories(){ // 函数

    static const std::unordered_map<char , FormatItemFactory> FORMAT_ITEM_FACTORIES{
        {'m' , &CreateSimpleFormatItem<MessageFormattItem>},    // 日志内容
        {'p' , &CreateSimpleFormatItem<LevelFormattItem>},      // LogLevel
        {'r' , &CreateSimpleFormatItem<ElapsedFormattItem>},    // 起服已过时间
        {'c' , &CreateSimpleFormatItem<LoggerNameFormattItem>}, // 日志器名称
        {'t' , &CreateSimpleFormatItem<ThreadIdFormattItem>},   // 线程id
        {'n' , &CreateSimpleFormatItem<NewLineFormattItem>},    // 换行
        {'d' , &CreateConfiguredFormatItem<DataTimeFormattItem>},   // datetime
        {'u' , &CreateSimpleFormatItem<MicrosecondsFormattItem>},// 毫秒数
        {'f' , &CreateSimpleFormatItem<FileNameFormatItem>},    // 文件名
        {'l' , &CreateSimpleFormatItem<LineFormatItem>},        // 行号

        {'T' , &CreateSimpleFormatItem<TableFormatItem>},       // 制表符

        {'F' , &CreateSimpleFormatItem<FiberIdFormatItem>},     // 协程ID
        {'N' , &CreateSimpleFormatItem<ThreadNameFormatItem>}   // 大N线程名
    };
    return FORMAT_ITEM_FACTORIES;
}

std::unique_ptr<Formatter::FormatItem> CreatFormatItem(char directive    , std::string_view format){
    const auto& factories = GetFormatItemFactories(); 
    const auto it = factories.find(directive);
    ASSERT_RETVAL2(it != factories.end() , nullptr ,  "unknown logger format directive: " << directive);
    return it->second(format);
}

//插入纯文本内容
// %d{%Y-%m-%d %H:%M:%S}.%u%Tthread=%t%Tfiber=%F%T[%p]%T%f:%l%T%m%n
void Formatter::addLiteral(std::string & literal){
    if(literal .empty()) return ;
    m_items.push_back(std::make_unique<LiteralFormattItem> (std::move(literal)));
    literal.clear(); // 资源被移动之后 不应该再被引用
}

int Formatter::parse(){
    std::string literal;

    for(std::size_t index = 0 ; index < m_pattern.size(); ++index ){
        if(m_pattern[index] != '%'){  
            literal.push_back(m_pattern[index]);
            continue;
        }
        //以 % 结尾了
        ASSERT_RETVAL2(index +1 < m_pattern.size() , -1 , "logger format pattern ends with an incomplete directive" );

        //现在 m_pattern[index] 是 %  
        // 
        const char directive = m_pattern[++index];  // % 后面一个 , index 变成了 下一个

        if(directive == '%') { // %%
            literal.push_back('%');
            continue;
        }

        addLiteral(literal);  // 放在上面 if 的下面 ： 防止 %%%% 这中多个 % 的情况

        std::string_view item_format;

        // {  ...   }
        //  %     d     {      %Y-%m-%d %H:%M:%S}.%u%Tthread=%t%Tfiber=%F%T[%p]%T%f:%l%T%m%n
        //       index  index + 1 
        if(index +1  < m_pattern.size() && m_pattern[index +1 ] == '{' ){

            const std::size_t closing_brace = m_pattern.find('}' , index + 2 );
            // 防止  ： %d{} // 
            ASSERT_RETVAL2(closing_brace != std::string::npos && closing_brace != (index +2 ) , -2 , "missing a closing brace or empty");

            item_format = std::string_view(m_pattern).substr(index+2 , closing_brace - index -2); 
            index = closing_brace;
        }
        m_items.push_back(CreatFormatItem(directive , item_format));  // 
    }
    addLiteral(literal);

    for(const auto & item : m_items){
        ASSERT_RETVAL2(item != nullptr , -3 , "logger formatter contains a null format item");
    }

    return 0;
}














} // namespace pans detail


