#include "http.h"
#include "Match.h"
#include "db.h"
#include <nlohmann/json.hpp>
#include <fmt/core.h>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#define to_long_steam_id(id) (id + 76561197960265728)
#define to_short_steam_id(id) (id - 76561197960265728)

int64_t get_last_match_id(int64_t short_steam_id)
{
    std::string response;
    std::string url = fmt::format("https://api.steampowered.com/IDOTA2Match_570/GetMatchHistory/"
                                  "v1?key=0A93BB155FCA1A281587AE2567CE9EFE&account_id={}&matches_requested=1",
                                  short_steam_id);
    http_get(url, response, 10);
    nlohmann::json json = nlohmann::json::parse(response);
    return json["result"]["matches"][0]["match_id"].get<int64_t>();
}

Match get_match_detail(int64_t match_id)
{
    std::string response;
    std::string url = fmt::format("https://api.steampowered.com/IDOTA2Match_570/GetMatchDetails/"
                                  "v1?key=0A93BB155FCA1A281587AE2567CE9EFE&match_id={}&include_persona_names=1",
                                  match_id);
    http_get(url, response, 10);
    nlohmann::json json = nlohmann::json::parse(response);
    return json["result"].get<Match>();
}

std::string generate_report(int64_t match_id, int64_t short_steam_id)
{
    Match &&match = get_match_detail(match_id);
    auto it = std::find_if(match.players.begin(), match.players.end(),
                           [=](const Player &player)
                           { return player.short_steam_id == short_steam_id; });
    assert(it != match.players.end());
    Player &player = *it;
    return fmt::format("KDA: {}[{}/{}/{}]，GPM/XPM: {}/{}，正补/反补: {}/{}", player.kda, player.kill,
                       player.death, player.assistance, player.gpm, player.xpm, player.last_hits, player.dennies);
}