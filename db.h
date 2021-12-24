#ifndef _DB_H_
#define _DB_H_
#include "Account.h"
#include <sqlite3.h>
#include <mutex>
#include <vector>

class DataBase
{
public:
    DataBase(const DataBase &) = delete;
    DataBase &operator=(const DataBase &) = delete;
    static DataBase &get_instance();
    ~DataBase();
    void insert_account(int64_t qq_id, const std::string &nickname, int64_t short_steam_id);
    void update_account(int64_t short_steam_id, int64_t last_match_id);
    std::vector<Account> get_accounts();

private:
    sqlite3 *db_;
    std::mutex mtx_;
    static int callback(void *arg, int argc, char **fields, char **colnames);
    DataBase();
};
#endif
