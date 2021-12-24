#ifndef _SENDER_H_
#define _SENDER_H_
#include <string>
void send_to_group(const std::string &message, int64_t group_id, int64_t at = 0);
#endif