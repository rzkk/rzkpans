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

    [[nodiscard]] std::string_view view() const noexcept  // string_view : 就是一个二元组 //std::string_view 只是借用日志数据。
    {
        return {data() , size()};
    }
private:
    std::array<char , INLINE_CAPACITY> m_inline{}; //短日志  m_inline{} 中的 {} 会初始化全部字符为零
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
    // << 会调用 xsputn 
    std::streamsize xsputn(const char * data , std::streamsize size) override{
        // size < 0 非法 
        // size ==0 , 没有字符串输入
        if(size <= 0) return 0; 
        m_buffer.append(data , static_cast<std::size_t>(size));
        return size;//返回 本次 xsputn() 成功写入了多少字符。
    }
    // 为了单字符输出 // 如果不重写这个接口 有可能造成流错误
    //通常 using char_type = char;using int_type = int;  char只有8位 ，表示字符 ，但还需额外表示 “没有字符”或“写入失败”
    //    traits_type::eof() ： -1  ， but 不同编译器 eof 值不同
    // eof()	获取 EOF 特殊值   ，  eq_int_type(a, b)	比较两个 int_type 值
    //not_eof(c)	返回一个不是 EOF 的值，  to_char_type(c)	把 int_type 转为字符类型

    int_type overflow(int_type character) override{
        ////如果这次收到的是 EOF 特殊值，就不要把它当作实际字符追加到缓冲区，而是返回非 EOF 的结果，表示本次操作成功。
        if(traits_type::eq_int_type(character , traits_type::eof())){
            return traits_type::not_eof(character); // 它保证返回一个不等于 EOF 的值。
        }
        //通过 traits_type::to_char_type() 转换，比直接写一个不加解释的整数转字符更符合标准流的类型设计。
        m_buffer.append(traits_type::to_char_type(character));
        return character;
    }

private:
    InlineBuffer<INLINE_CAPACITY>& m_buffer;
};


void func(){
    InlineBuffer<128> buffer;
    SmallStreamBuffer<128> streambuf(buffer);

    std::ostream os(&streambuf);
    std::cout<<"hello";
    os << "age=" << 20;
}

    
} // namespace pans::detail


/* 
*
*
*
*
*/








#endif