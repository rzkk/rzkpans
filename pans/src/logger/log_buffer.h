#ifndef PANS_SRC_LOGGER_BUFFER_H
#define PANS_SRC_LOGGER_BUFFER_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <streambuf>
#include <string_view>
#include <vector>

namespace pans::detail{

// 自定义一个日志缓冲区，针对高频、小数据量的日志场景，在栈上分配日志缓冲区，遇超长日志，再在堆上分配内存
template<std::size_t INLINE_CAPACITY>  // INLINE_CAPACITY 日志的 栈buffer 
class InlineBuffer{
public:
    void append(const char * data , std::size_t size) {  // 将传进来的数据 放入 buffer中 
        if(size == 0 ) return;
        if(m_overflow.empty() && m_size + size <= INLINE_CAPACITY){
            std::memcpy(m_inline.data() + m_size , data , size);
            m_size += size;
            return;
        }

        if(m_overflow.empty()){  // 短日志不够放了 ， 要放到长的里面
            const std::size_t required_capacity = m_size + size ; // 至少需要这么多
            m_overflow.reserve(std::max( INLINE_CAPACITY *2  , required_capacity )) ;
            
            // 把栈上的拷贝过来
            m_overflow.insert(m_overflow.end(), m_inline.data() , m_inline.data() + m_size); 
        }
        m_overflow.insert(m_overflow.end(), data , data + size);  // 利用vector的扩容
        m_size = m_overflow.size();
    }

    void append(std::string_view value){
        append(value.data() , value.size());
    }

    void append(char value){
        append(&value , 1);
    }

    [[nodiscard]] const char * data() const noexcept{
        return m_overflow.empty() ? m_inline.data() : m_overflow.data();
    }

    [[nodiscard]] std::size_t size() const noexcept{
        return m_size;
    }

    [[nodiscard]] std::string_view view() const noexcept  // string_view : 就是一个二元组
    {
        return {data() , size()}
    }
private:
    std::array<char , INLINE_CAPACITY> m_inline{}; //短日志 int m_inline[256]; 
    std::vector<char>m_overflow;             // 长日志 存在堆上 
    std::size_t m_size = 0 ; // 日志长度
};

// 继承std::streambuf是为了能够能够继续使用流式输入接口，兼容ostream生态
// std::hex , setw setprecision
template<std::size_t INLINE_CAPACITY>  
class SmallStreamBuffer final : public std::streambuf{
public:
    explicit SmallStreamBuffer(InlineBuffer<INLINE_CAPACITY> & buffer) noexcept
        :m_buffer(buffer){}

//子类重写
protected:
    std::streamsize xsputn(const char * data , std::streamsize size) override{
        if(size <= 0) return 0; // 非法
        m_buffer.append(data , static_cast<std::size_t>(size));
        return size;
    }
    // 为了单字符输出 // 如果不重写这个接口 有可能造成流错误
    // bug 出在 sputc
    // overflow，除了可以接受一般字符，还可能会接收流终止符EOF
    // 如果我们遇到了EOF, 把它转成非EOF值向上报告写入成功，实际上不向Buffer里写入任何值。
    int_type overflow(int_type character) override{
        if(traits_type::eq_int_type(character , traits_type::eof())){
            return traits_type::not_eof(character);
        }

        m_buffer.append(traits_type::to_char_type(character));
        return character;
    }

private:
    InlineBuffer<INLINE_CAPACITY>& m_buffer;
};

    
} // namespace pans::detail










#endif