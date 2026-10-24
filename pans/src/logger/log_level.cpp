#include <pans/logger/log_level.h>

#include<algorithm>

#include<array>
#include<string>
#include<cctype>

// ## : 标记拼接操作符
// #：宏参数变为字符串字面量 ， 字符串化操作符  , 得到字符串字面量
namespace pans
{
    std::string_view LogLevel::ToString(Level level) noexcept{
        switch (level)
        {
#define XX(name) case Level::LOG_LV_##name: return #name;
            XX(DEBUG)
            XX(INFO)
            XX(WARN)
            XX(ERROR)
            XX(FATAL)
            XX(OFF)
#undef XX  
//取消宏定义 ， 宏不受 C++ 花括号作用域限制。
        }
        return "UNKONWN";
    }


    LogLevel::Level LogLevel::FromString(std::string_view value) noexcept { 
        //1 先统一转成大写
        std::array<char , 6>normalized{};
        std::transform( value.begin() ,value.end() ,normalized.begin() ,[](unsigned char character){
            return static_cast<char>(std::toupper(character));
        }  );

        const std::string_view upper_value(normalized.data() , normalized.size());
#define XX(name) if(upper_value == #name) return Level::LOG_LV_##name;
        XX(DEBUG)
        XX(INFO)
        XX(WARN)
        XX(ERROR)
        XX(FATAL)
        XX(OFF)
#undef XX  
        return Level::LOG_LV_OFF;
    }

} // namespace pans
