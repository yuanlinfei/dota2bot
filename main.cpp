#include "db.h"
#include "report.h"
#include "curl/curl.h"
#include "http.h"
#include "sender.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <signal.h>
#include <regex>
#include <thread>
#include <sio_client.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>

std::mutex mutex;
std::condition_variable cond;
bool connect_finish = false;
sio::socket::ptr current_socket;
auto logger = spdlog::basic_logger_mt("dota2_bot_logger", "logs/log.txt");

void on_connected()
{
    std::unique_lock<std::mutex> lock(mutex);
    connect_finish = true;
    cond.notify_one();
}

void on_fail()
{
    logger->error("sio failed");
    exit(0);
}

void on_close(sio::client::close_reason const& reason)
{
    logger->info("sio closed");
    exit(0);
}

int main(int argc ,const char* args[])
{
    signal(SIGHUP,SIG_IGN);
    signal(SIGTERM,SIG_IGN);
    signal(SIGINT,SIG_IGN);
    daemon(1, 0);

    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);

    sio::client h;
    h.set_open_listener(&on_connected);
    h.set_close_listener(&on_close);
    h.set_fail_listener(&on_fail);
    h.connect("http://localhost:8888");
    std::unique_lock<std::mutex> lock(mutex);
    while (!connect_finish)
        cond.wait(lock);
    lock.unlock();
	current_socket = h.socket();
    current_socket->on("OnGroupMsgs", sio::socket::event_listener_aux([&](std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp)
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
                        message += fmt::format("[{}]  {}， {}， {}\n", ++num, account.short_steam_id, account.qq_id, account.nickname);
                }
                if (message.size() == 0) 
                {
                    send_to_group("没有人被监视", group_id);
                    logger->info("没有人被监视 to {}", group_id);
                }
                else
                {
                    send_to_group(message, group_id);
                    logger->info("发送 {} to {}", message, group_id);
                }
            }
            else if (content == "&帮助") 
            {
                send_to_group("支持以下命令\n&查看监视\n&添加监视 steam账号 QQ号 昵称\n&移除监视 steam账号", group_id);
                logger->info("发送 {} to {}", "支持以下命令\n&查看监视\n&添加监视 steam账号 QQ号 昵称\n&移除监视 steam账号", group_id);
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
                        logger->info("发送  {} to {}", "参数错误", group_id);

                    }
                    else
                    {
                        db.insert_account(Account(atol(args[1].c_str()), atol(args[2].c_str()), args[3], group_id, 0));
                        send_to_group("添加成功", group_id);
                        logger->info("发送 {} to {}", "添加成功", group_id);
                    }
                }
                else if (args.size() == 2 && args[0] == "&移除监视")
                {
                    std::regex digits("\\d{9,}");
                    if (!std::regex_match(args[1], digits))
                    {
                        send_to_group("参数错误", group_id);
                        logger->info("发送 {} to {}", "参数错误", group_id);
                    }
                    else 
                    {
                        db.remove_account(atol(args[1].c_str()));
                        send_to_group("移除成功", group_id);
                        logger->info("{} to {}", "移除成功", group_id);
                    }
                }
            }
        }
    }));
    report();
    h.sync_close();
    h.clear_con_listeners();
    return 0;
}