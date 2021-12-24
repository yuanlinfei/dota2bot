#include "db.h"
#include "report.h"
#include <iostream>
#include <thread>

int main(int argc, char *argv[])
{
    DataBase &db = DataBase::get_instance();
    for (;;)
    {
        for (Account &account : db.get_accounts())
        {
            if (int64_t last_match_id = get_last_match_id(account.short_steam_id);
                last_match_id != account.last_match_id)
            {
                db.update_account(account.short_steam_id, last_match_id);
                std::cout << generate_report(last_match_id, account.short_steam_id) << std::endl;
            }
        }
    }
    return 0;
}
