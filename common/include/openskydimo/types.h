#pragma once

#include <algorithm>
#include <cstddef>
#include <string>

#include "spdlog/fmt/bundled/base.h"
#include "spdlog/fmt/bundled/format.h"

struct ColorRGB
{
    std::byte r = static_cast<std::byte>(0);
    std::byte g = static_cast<std::byte>(0);
    std::byte b = static_cast<std::byte>(0);

    ColorRGB() = default;

    ColorRGB(const std::byte r, const std::byte g, const std::byte b) : r(r), g(g), b(b)
    {
    }

    ColorRGB(const int r, const int g, const int b)
        : r(static_cast<std::byte>(std::clamp(r, 0, 255))), g(static_cast<std::byte>(std::clamp(g, 0, 255))),
          b(static_cast<std::byte>(std::clamp(b, 0, 255)))
    {
    }
};

template <>
struct fmt::formatter<ColorRGB> : formatter<std::string>
{
    static auto format(const ColorRGB& my, const format_context& ctx) -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "({},{},{})", static_cast<int>(my.r), static_cast<int>(my.g),
                         static_cast<int>(my.b));
    }
};