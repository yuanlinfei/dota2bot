#include "http.h"
#include "Match.h"
#include "db.h"
#include "sender.h"
#include <fmt/chrono.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <nlohmann/json.hpp>
#include <exception>
#include <thread>
#include <string>
#include <vector>

#define to_long_steam_id(id) (id + 76561197960265728)
#define to_short_steam_id(id) (id - 76561197960265728)

extern std::shared_ptr<spdlog::logger> logger;

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

std::string generate_report(Account &account)
{
    Match match = get_match_detail(account.last_match_id);
    auto it = std::find_if(match.players.begin(), match.players.end(),
                           [=](const Player &player)
                           { return player.short_steam_id == account.short_steam_id; });
    assert(it != match.players.end());
    Player &player = *it;
    int total_hero_damage = 0;
    std::for_each(match.players.begin(), match.players.end(), [&](const Player &p)
                  {
                      if (p.radiant == player.radiant)
                          total_hero_damage += p.hero_damage;
                  });
    return fmt::format("{:%Y-%m-%d %H:%M:%S}，{}使用{}{}了比赛，KDA: {}[{}/{}/{}]，GPM/XPM: {}/{}，正补/反补: {}/{}, 输出{}({}%)。", *std::localtime(&match.start_time), account.nickname, player.hero, match.radiant_win == player.radiant ? "赢得" : "输掉", player.kda, player.kill,
                       player.death, player.assistance, player.gpm, player.xpm, player.last_hits, player.dennies, player.hero_damage, 100.0f * player.hero_damage / total_hero_damage);
}

void report()
{
    DataBase &db = DataBase::get_instance();
    for (;;)
    {
        try
        {
            for (Account &account : db.get_accounts())
            {
                if (int64_t last_match_id = get_last_match_id(account.short_steam_id);
                    last_match_id != account.last_match_id)
                {
                    account.last_match_id = last_match_id;
                    db.update_account(account.short_steam_id, last_match_id);
                    try
                    {
                        send_to_group(generate_report(account), account.group_id, account.qq_id);
                        logger->info("发送 战报 to {}", account.group_id);
                    }
                    catch (std::exception &e)
                    {
                        logger->error("发送战绩失败 {}", e.what());
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            std::this_thread::sleep_for(std::chrono::seconds(300));
        }
        catch (std::exception &e)
        {
            logger->error("运行时异常 {}", e.what());
        }
    }
}
