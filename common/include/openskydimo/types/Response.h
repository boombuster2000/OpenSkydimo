#pragma once
#include <format>
#include <string>

#include <nlohmann/json.hpp>

namespace openskydimo::types
{
struct Response
{
    int code;
    std::string message;
};

inline Response MakeOk()
{
    return {0, ""};
}

inline Response MakeError(const int code, std::string_view msg)
{
    return {code, std::format("error: {}", msg)};
}

inline Response MakeError(const int code, std::string_view msg, std::string_view tryCmd)
{
    return {code, std::format("error: {}; try: {}", msg, tryCmd)};
}

inline Response MakeWarning(const int code, std::string_view msg)
{
    return {code, std::format("warning: {}", msg)};
}

inline Response MakeWarning(const int code, std::string_view msg, std::string_view tryCmd)
{
    return {code, std::format("warning: {}; try: {}", msg, tryCmd)};
}

inline void to_json(nlohmann::json& j, const Response& r)
{
    j = nlohmann::json{{"code", r.code}, {"message", r.message}};
}

inline void from_json(const nlohmann::json& j, Response& r)
{
    j.at("code").get_to(r.code);
    j.at("message").get_to(r.message);
}

} // namespace openskydimo::types
