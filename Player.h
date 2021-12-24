#ifndef _PLAYER_H_
#define _PLAYER_H_
#include <nlohmann/json.hpp>
#include <string>
struct Player {
    std::string persona;
    int64_t short_steam_id;
    bool radiant;
    std::string hero;
    int kill;
    int death;
    int assistance;
    float kda;
    int gpm;
    int xpm;
    int last_hits;
    int dennies;
    int hero_damage;
    int tower_damage;
    int hero_healing;
};

void from_json(const nlohmann::json& json, Player& player);
#endif