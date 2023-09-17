#ifndef GQY_COMMON_H
#define GQY_COMMON_H

#include <iostream>
#include <string>

enum class FontColor
{
    Red,
    Green,
    Yellow,
    Blue,
    Purple,
    Indigo,
    White
};

inline std::string setFontColor(const std::string& _str, const FontColor& _fontColor)
{
    std::string ans;
    switch (_fontColor)
    {
    case FontColor::Red:
        ans = "\033[31m" + _str + "\033[0m";
        break;
    case FontColor::Green:
        ans = "\033[32m" + _str + "\033[0m";
        break;
    case FontColor::Yellow:
        ans = "\033[33m" + _str + "\033[0m";
        break;
    case FontColor::Blue:
        ans = "\033[34m" + _str + "\033[0m";
        break;
    case FontColor::Purple:
        ans = "\033[35m" + _str + "\033[0m";
        break;
    case FontColor::Indigo:
        ans = "\033[36m" + _str + "\033[0m";
        break;
    case FontColor::White:
        ans = "\033[37m" + _str + "\033[0m";
        break;
    default:
        break;
    }
    return ans;
}

#endif