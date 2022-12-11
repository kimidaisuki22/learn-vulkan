#pragma once

#if  __has_include(<format>)
#include <format>
#else 
#include<fmt/format.h>
namespace std{
    using fmt::format;
}
#endif