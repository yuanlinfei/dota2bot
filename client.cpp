#include <sio_client.h>
#include <string.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include "http.h"


#define HIGHLIGHT(STR) std::cout<<"\e[1;31m"<< STR <<"\e[0m"<<std::endl

std::mutex mutex;

std::condition_variable cond;
bool connect_finish = false;
sio::socket::ptr current_socket;

void on_connected()
{
    std::unique_lock<std::mutex> lock(mutex);
    connect_finish = true;
    cond.notify_one();
}

void on_fail()
{
    std::cout<<"sio failed "<<std::endl;
    exit(0);
}

void on_close(sio::client::close_reason const& reason)
{
    std::cout<<"sio closed "<<std::endl;
    exit(0);
}


int main(int argc ,const char* args[])
{
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
        nlohmann::json json = {
            {"ToUserUid", qq_id},
            {"SendToType", 1},
            {"SendMsgType", "TextMsg"},
            {"Content", content}
        };

        std::string response;
        CURLcode ret = http_post("http://localhost:8888/v1/LuaApiCaller?qq=3192047076&funcname=SendMsgV2", json.dump(), response, 10);
        std::cout << ret << std::endl;
        std::cout << response << std::endl;
    }));
    current_socket->on("OnFriendMsgs", sio::socket::event_listener_aux([&](std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp)
    {
        HIGHLIGHT("1");
        int64_t qq_id = data->get_map()["CurrentPacket"]->get_map()["Data"]->get_map()["FromUin"]->get_int();
        std::string content = data->get_map()["CurrentPacket"]->get_map()["Data"]->get_map()["Content"]->get_string();
        nlohmann::json json = {
            {"ToUserUid", qq_id},
            {"SendToType", 1},
            {"SendMsgType", "TextMsg"},
            {"Content", content}
        };
        std::string response;
        CURLcode ret = http_post("http://localhost:8888/v1/LuaApiCaller?qq=3192047076&funcname=SendMsgV2", json.dump(), response, 10);
        std::cout << ret << std::endl;
        std::cout << response << std::endl;
    }));
    for (;;);
    HIGHLIGHT("Closing...");
    h.sync_close();
    h.clear_con_listeners();
	return 0;
}
