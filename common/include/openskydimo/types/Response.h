#pragma once
#include <string>

#include <nlohmann/json.hpp>

struct Response
{
    int code;
    std::string message;
};

inline void to_json(nlohmann::json& j, const Response& r)
{
    j = nlohmann::json{{"code", r.code}, {"message", r.message}};
}

inline void from_json(const nlohmann::json& j, Response& r)
{
    j.at("code").get_to(r.code);
    j.at("message").get_to(r.message);
}
