#pragma once

#include <algorithm>
#include <cstddef>

#include "spdlog/fmt/fmt.h"
#include <nlohmann/json.hpp>

namespace openskydimo::types
{
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

inline void to_json(nlohmann::json& j, const ColorRGB& color)
{
    j = nlohmann::json{{"r", std::to_integer<int>(color.r)},
                       {"g", std::to_integer<int>(color.g)},
                       {"b", std::to_integer<int>(color.b)}};
}

inline void from_json(const nlohmann::json& j, ColorRGB& color)
{
    color = ColorRGB(j.at("r").get<int>(), j.at("g").get<int>(), j.at("b").get<int>());
}

} // namespace openskydimo::types

template <>
struct fmt::formatter<openskydimo::types::ColorRGB> : formatter<std::string>
{
    auto format(const openskydimo::types::ColorRGB& my, format_context& ctx) const -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "({},{},{})", std::to_integer<int>(my.r), std::to_integer<int>(my.g),
                         std::to_integer<int>(my.b));
    }
};
