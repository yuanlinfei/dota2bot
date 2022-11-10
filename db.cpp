#include "db.h"
#include <sqlite3.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>

extern std::shared_ptr<spdlog::logger> logger;

DataBase::DataBase()
{
    int rc;
    char *error_msg;
    rc = sqlite3_open("dota2.db", &db_);
    std::string sql = "CREATE TABLE IF NOT EXISTS playerInfo("
                      "short_steam_id INT PRIMARYKEY UNIQUE NOT NULL,"
                      "qq_id INT NOT NULL,"
                      "nickname CHAR(50) NOT NULL,"
                      "group_id INT NOT NULL,"
                      "last_match_id INT)";
    rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_msg);
    if (rc != SQLITE_OK)
    {
        logger->error("sqlite error: {}", error_msg);
        sqlite3_free(error_msg);
    }
}

DataBase::~DataBase()
{
    sqlite3_close(db_);
}

DataBase &DataBase::get_instance()
{
    static DataBase instance;
    return instance;
}

void DataBase::insert_account(const Account &account)
{
    int rc;
    char *error_msg;
    std::string sql = fmt::format("insert into playerInfo(short_steam_ID, qq_id, nickname, group_id) values({}, {}, '{}', {});",
                                  account.short_steam_id, account.qq_id, account.nickname, account.group_id);
    std::lock_guard<std::mutex> lock(mtx_);
    logger->info("执行sql: {}", sql);
    rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_msg);
    if (rc != SQLITE_OK)
    {
        logger->error("sqlite error: {}", error_msg);
        sqlite3_free(error_msg);
    }
}

void DataBase::update_account(int64_t short_steam_id, int64_t last_match_id)
{
    int rc;
    char *error_msg;
    std::string sql = fmt::format("update playerInfo set last_match_id = {} where short_steam_id = {}",
                                  last_match_id, short_steam_id);
    std::lock_guard<std::mutex> lock(mtx_);
    logger->info("执行sql: {}", sql);
    rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_msg);
    if (rc != SQLITE_OK)
    {
        logger->error("sqlite error: {}", error_msg);
        sqlite3_free(error_msg);
    }
}

void DataBase::remove_account(int64_t short_steam_id)
{
    int rc;
    char *error_msg;
    std::string sql = fmt::format("delete from  playerInfo where short_steam_id = {}", short_steam_id);
    std::lock_guard<std::mutex> lock(mtx_);
    logger->info("执行sql: {}", sql);
    rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_msg);
    if (rc != SQLITE_OK)
    {
        logger->error("sqlite error: {}", error_msg);
        sqlite3_free(error_msg);
    }
}

std::vector<Account> DataBase::get_accounts()
{
    int rc;
    char *error_msg;
    std::vector<Account> accounts;
    std::string sql = "select * from playerInfo";
    std::lock_guard<std::mutex> lock(mtx_);
    // logger->info("执行sql: {}", sql);
    rc = sqlite3_exec(db_, sql.c_str(), callback, (void *)&accounts, &error_msg);
    if (rc != SQLITE_OK)
    {
        logger->error("sqlite error: {}", error_msg);
        sqlite3_free(error_msg);
    }
    return accounts;
}

int DataBase::callback(void *arg, int argc, char **fields, char **colnames)
{
    std::vector<Account> *vector = static_cast<std::vector<Account> *>(arg);
    vector->emplace_back(atol(fields[0]), atol(fields[1]), fields[2], atol(fields[3]), fields[4] ? atol(fields[4]) : 0);
    return 0;
}
