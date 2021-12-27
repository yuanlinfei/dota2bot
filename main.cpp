#include "db.h"
#include "report.h"
#include "curl/curl.h"
#include "http.h"
#include "sender.h"
#include "handle.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <signal.h>
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
    h.socket()->on("OnGroupMsgs", handle);
    report();
    h.sync_close();
    h.clear_con_listeners();
    return 0;
}