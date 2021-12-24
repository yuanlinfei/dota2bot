#ifndef _REPORT_H_
#define _REPORT_H_
#include "Match.h"

#define to_long_steam_id(id) (id + 76561197960265728)
#define to_short_steam_id(id) (id - 76561197960265728)

int64_t get_last_match_id(int64_t short_steam_id);
Match get_match_detail(int64_t match_id);
std::string generate_report(int64_t match_id, int64_t short_steam_id);
#endif