#include <sio_client.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <regex>
#include "sender.h"
#include "db.h"

extern std::shared_ptr<spdlog::logger> logger;

void handle(std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp)
{
    int64_t group_id = data->get_map()["CurrentPacket"]->get_map()["Data"]->get_map()["FromGroupId"]->get_int();
    int64_t qq_id = data->get_map()["CurrentPacket"]->get_map()["Data"]->get_map()["FromUserId"]->get_int();
    std::string content = data->get_map()["CurrentPacket"]->get_map()["Data"]->get_map()["Content"]->get_string();
    if (qq_id != 3192047076 && content[0] == '&') 
    {
        logger->info("收到 {}", content);
        DataBase &db = DataBase::get_instance();
        if (content == "&查看监视") {
            std::string message;
            int num = 0;
            for (Account &account : db.get_accounts())
            {
                if (account.group_id == group_id) 
                    message += fmt::format("[{}] {}，{}，{}\n", ++num, account.short_steam_id, account.qq_id, account.nickname);
            }
            if (message.size() == 0) 
            {
                send_to_group("没有人被监视", group_id);
                logger->info("发送 没有人被监视 to {}", group_id);
            }
            else
            {
                send_to_group(message, group_id);
                std::replace(message.begin(), message.end(), '\n', ' ');
                logger->info("发送 {}to {}", message, group_id);
            }
        }
        else if (content == "&帮助") 
        {
            send_to_group("支持以下命令\n&查看监视\n&添加监视 steam账号 QQ号 昵称\n&移除监视 steam账号", group_id);
            logger->info("发送 帮助 to {}", group_id);
        }
        else 
        {
            std::regex re("\\s+");
            std::vector<std::string> args(std::sregex_token_iterator(content.begin(), content.end(), re, -1), std::sregex_token_iterator());
            if (args.size() == 4 && args[0] == "&添加监视") 
            {
                std::regex digits("\\d{9,}");
                if (!std::regex_match(args[1], digits) || !std::regex_match(args[2], digits))
                {
                    send_to_group("参数错误", group_id);
                    logger->info("发送 参数错误 to {}", group_id);
                }
                else
                {
                    db.insert_account(Account(atol(args[1].c_str()), atol(args[2].c_str()), args[3], group_id, 0));
                    send_to_group("添加成功", group_id);
                    logger->info("发送 添加成功 to {}", group_id);
                }
            }
            else if (args.size() == 2 && args[0] == "&移除监视")
            {
                std::regex digits("\\d{9,}");
                if (!std::regex_match(args[1], digits))
                {
                    send_to_group("参数错误", group_id);
                    logger->info("发送 参数错误 to {}", group_id);
                }
                else 
                {
                    db.remove_account(atol(args[1].c_str()));
                    send_to_group("", group_id);
                    logger->info("发送 移除成功 to {}", group_id);
                }
            }
        }
    }
}