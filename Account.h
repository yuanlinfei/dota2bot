#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_
#include <string>
class Account {
public:
    int64_t qq_id;
    std::string nickname;
    int64_t short_steam_id;
    int64_t last_match_id;
    Account(int64_t qq_id_, const std::string &nickname_, int64_t short_steam_id_, int64_t last_match_id_);
};
#endif