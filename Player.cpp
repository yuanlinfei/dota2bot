#include <nlohmann/json.hpp>
#include "Player.h"
#include "dota.h"

void from_json(const nlohmann::json &json, Player &player)
{
    json.at("persona").get_to(player.persona);
    json.at("account_id").get_to(player.short_steam_id);
    player.radiant = json["player_slot"].get<int>() < 5;
    player.hero = heros[json["hero_id"].get<int>()];
    json.at("kills").get_to(player.kill);
    json.at("deaths").get_to(player.death);
    json.at("assists").get_to(player.assistance);
    player.kda = player.death ? (player.kill + player.assistance) / (float)player.death : player.kill + player.assistance;
    json.at("gold_per_min").get_to(player.gpm);
    json.at("xp_per_min").get_to(player.xpm);
    json.at("last_hits").get_to(player.last_hits);
    json.at("denies").get_to(player.dennies);
    json.at("hero_damage").get_to(player.hero_damage);
    json.at("tower_damage").get_to(player.tower_damage);
    json.at("hero_healing").get_to(player.hero_healing);
}