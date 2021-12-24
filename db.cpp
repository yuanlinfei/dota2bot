#include "db.h"
#include <sqlite3.h>
#include <fmt/core.h>
#include <iostream>

DataBase::DataBase()
{
    int rc;
    char *error_msg;
    rc = sqlite3_open("dota2.db", &db_);
    std::string sql = "CREATE TABLE IF NOT EXISTS playerInfo("
                      "qq_id INT PRIMARYKEY NOT NULL,"
                      "nickname CHAR(50) NOT NULL,"
                      "short_steam_id INT NOT NULL,"
                      "last_match_id INT)";
    rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_msg);
    if (rc != SQLITE_OK)
    {
        printf("sqlite error: %s\n", error_msg);
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

void DataBase::insert_account(int64_t qq_id, const std::string &nickname, int64_t short_steam_id)
{
    int rc;
    char *error_msg;
    std::string sql = fmt::format("insert into playerInfo(qq_id, nickname, short_steam_ID) values({}, '{}', {})",
                                  qq_id, nickname, short_steam_id);
    std::lock_guard<std::mutex> lock(mtx_);
    rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_msg);
    if (rc != SQLITE_OK)
    {
        printf("sqlite error: %s\n", error_msg);
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
    rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_msg);
    if (rc != SQLITE_OK)
    {
        printf("sqlite error: %s\n", error_msg);
        sqlite3_free(error_msg);
    }
}

std::vector<Account> DataBase::get_accounts()
{
    int rc;
    char *error_msg;
    std::vector<Account> accounts;
    std::lock_guard<std::mutex> lock(mtx_);
    std::string sql = "select * from playerInfo";
    rc = sqlite3_exec(db_, sql.c_str(), callback, (void *)&accounts, &error_msg);
    if (rc != SQLITE_OK)
    {
        printf("sqlite error: %s\n", error_msg);
        sqlite3_free(error_msg);
    }
    return accounts;
}

int DataBase::callback(void *arg, int argc, char **fields, char **colnames)
{
    std::vector<Account> *vector = static_cast<std::vector<Account> *>(arg);
    vector->emplace_back(atol(fields[0]), fields[0], atol(fields[2]), fields[3] ? atol(fields[3]) : 0);
    return 0;
}
