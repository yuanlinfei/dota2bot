#include "http.h"
#include <nlohmann/json.hpp>
#include <string>
void send_to_group(const std::string &message, int64_t group_id, int64_t at = 0)
{
    nlohmann::json json = {
        {"ToUserUid", group_id},
        {"SendToType", 2},
        {"SendMsgType", "TextMsg"},
        {"Content", message}};
    std::string response;
    http_post("http://localhost:8888/v1/LuaApiCaller?qq=3192047076&funcname=SendMsgV2", json.dump(), response, 10);
}