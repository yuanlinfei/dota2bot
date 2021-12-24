#ifndef _HTTP_H_
#define _HTTP_H_
#include <curl/curl.h>
#include <string>

size_t http_response(void *contents, size_t size, size_t nmemb, void *stream);
CURLcode http_get(const std::string &url, std::string &response, int timeout);
CURLcode http_post(const std::string &url, const std::string &json, std::string &response, int timeout);
#endif