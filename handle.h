#ifndef _HANDLE_H_
#define _HANDLE_H_
#include <string>
#include <sio_client.h>
void handle(std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp);
#endif
