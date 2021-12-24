#include "Match.h"

void from_json(const nlohmann::json &json, Match &match)
{
    json.at("match_id").get_to(match.match_id);
    json.at("radiant_win").get_to(match.radiant_win);
    json.at("radiant_score").get_to(match.socres[0]);
    json.at("dire_score").get_to(match.socres[1]);
    json.at("players").get_to(match.players);
    json.at("start_time").get_to(match.start_time);
    json.at("duration").get_to(match.duration);
}