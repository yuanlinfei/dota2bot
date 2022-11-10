#include <fmt/chrono.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <nlohmann/json.hpp>
#include <exception>
#include <thread>
#include <string>
#include <vector>
#include "http.h"
#include "Match.h"
#include "db.h"
#include "sender.h"

#define to_long_steam_id(id) (id + 76561197960265728)
#define to_short_steam_id(id) (id - 76561197960265728)

extern std::shared_ptr<spdlog::logger> logger;

bool open_dota_match_history(int64_t short_steam_id) 
{
    std::string url = fmt::format("https://api.steampowered.com/IDOTA2Match_570/GetMatchHistory/"
                                  "v1?key=F5DC8EE8056D66D5F7CDAC073AA0E10B&account_id={}&matches_requested=1",
                                  short_steam_id);
    std::string response;
    http_get(url, response, 10);
    nlohmann::json json = nlohmann::json::parse(response);
    return json["result"]["status"].get<int64_t>() == 1;
}

int64_t get_last_match_id(int64_t short_steam_id)
{
    int64_t match_id = 0;
    std::string response;
    nlohmann::json json;
    std::string url = fmt::format("https://api.steampowered.com/IDOTA2Match_570/GetMatchHistory/"
                                  "v1?key=C6198ACA29D2CFB952C8F2CB3463C42C&account_id={}&matches_requested=1",
                                  short_steam_id);

    http_get(url, response, 10);
    try
    {
        json = nlohmann::json::parse(response);
        match_id = json["result"]["matches"][0]["match_id"].get<int64_t>();
    }
    catch (std::exception &e)
    {
        logger->error("获取{}的战绩失败: {}, {}, {}", short_steam_id, e.what(), url, json);
    }
    return match_id;
}

Match get_match_detail(int64_t match_id)
{
    std::string response;
    std::string url = fmt::format("https://api.steampowered.com/IDOTA2Match_570/GetMatchDetails/"
                                  "v1?key=C6198ACA29D2CFB952C8F2CB3463C42C&match_id={}&include_persona_names=1",
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
    return fmt::format("开始时间: {:%Y-%m-%d %H:%M:%S}\n"
                       "持续时间: {}分{}秒\n游戏模式: {}\n"
                       "[{}]使用[{}]{}\n"
                       "KDA: {:.2f}[{}/{}/{}]\n"
                       "参战率: {:.2f}\n"
                       "GPM/XPM: {}/{}\n"
                       "正补/反补: {}/{}\n"
                       "输出: {}({:.2f}%)",
                       *std::localtime(&match.start_time), 
                       match.minutes, match.seconds, match.game_mode, 
                       account.nickname, player.hero, match.radiant_win == player.radiant ? "WIN" : "LOSS",
                       player.kda, player.kill, player.death, player.assistance, 
                       (player.kill + player.death) / ((player.radiant ? match.socres[0] : match.socres[1]) ? (player.radiant ? match.socres[0] : match.socres[1]) : 1),
                       player.gpm, player.xpm, 
                       player.last_hits, player.dennies, 
                       player.hero_damage, total_hero_damage ? 100.0f * player.hero_damage / total_hero_damage: 0.0f);
}

void report()
{
    DataBase &db = DataBase::get_instance();
    for (;;)
    {
        for (Account &account : db.get_accounts())
        {
            if (int64_t last_match_id = get_last_match_id(account.short_steam_id);
                last_match_id != 0 && last_match_id != account.last_match_id)
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
        }
        std::this_thread::sleep_for(std::chrono::seconds(180));
    }
}
