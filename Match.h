#ifndef _MATCH_H_
#define _MATCH_H_
#include "Player.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <vector>

struct Match
{
    int64_t match_id;
    int64_t start_time;
    int64_t duration;
    int socres[2];
    bool radiant_win;
    std::vector<Player> players;
};
void from_json(const nlohmann::json& json, Match& match);
#endif