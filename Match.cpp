#include "Match.h"
#include "dota.h"
void from_json(const nlohmann::json &json, Match &match)
{
    json.at("match_id").get_to(match.match_id);
    json.at("start_time").get_to(match.start_time);
    int duration = json["duration"].get<int>();
    match.minutes = duration / 60;
    match.seconds = duration % 60;
    json.at("radiant_score").get_to(match.socres[0]);
    json.at("dire_score").get_to(match.socres[1]);
    json.at("radiant_win").get_to(match.radiant_win);
    match.game_mode = game_modes[json["game_mode"].get<int>()];
    json.at("players").get_to(match.players);
}