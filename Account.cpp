#include "Account.h"
Account::Account(int64_t short_steam_id_, int64_t qq_id_, const std::string &nickname_, int64_t group_id_, int64_t last_match_id_)
    : short_steam_id(short_steam_id_), qq_id(qq_id_), nickname(nickname_), group_id(group_id_), last_match_id(last_match_id_) {}