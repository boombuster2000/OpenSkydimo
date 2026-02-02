#pragma once
#include "spdlog/fmt/bundled/base.h"
#include "spdlog/fmt/bundled/format.h"

#include <algorithm>
#include <cstddef>
#include <string>

struct ColorRGB
{
    std::byte r;
    std::byte g;
    std::byte b;

    ColorRGB(const std::byte r, const std::byte g, const std::byte b) : r(r), g(g), b(b)
    {
    }

    ColorRGB(const int r, const int g, const int b)
        : r(static_cast<std::byte>(std::min(r, 255))), g(static_cast<std::byte>(std::min(g, 255))),
          b(static_cast<std::byte>(std::min(b, 255)))
    {
    }
};

template <>
struct fmt::formatter<ColorRGB> : formatter<std::string>
{
    static auto format(const ColorRGB& my, const fmt::format_context& ctx) -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "({},{},{})", static_cast<int>(my.r), static_cast<int>(my.g),
                         static_cast<int>(my.b));
    }
};