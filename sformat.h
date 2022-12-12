#pragma once

#if  __has_include(<format>) && !defined __clang__
#include <format>
#else 
#include<fmt/format.h>
namespace std{
    using fmt::format;
}
#endif
