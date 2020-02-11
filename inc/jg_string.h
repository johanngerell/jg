#pragma once

#include <string>

namespace jg
{

inline std::string& trim_left(std::string& string, const std::string& chars = "\t\n\v\f\r ")
{
    return string.erase(0, string.find_first_not_of(chars));
}
 
inline std::string& trim_right(std::string& string, const std::string& chars = "\t\n\v\f\r ")
{
    return string.erase(string.find_last_not_of(chars) + 1);
}
 
inline std::string& trim(std::string& string, const std::string& chars = "\t\n\v\f\r ")
{
    return trim_left(trim_right(string, chars), chars);
}

}
