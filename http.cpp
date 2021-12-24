#include <curl/curl.h>
#include <string.h>
#include <string>

size_t http_response(void *contents, size_t size, size_t nmemb, void *stream)
{
    std::string *str = (std::string *)stream;
    (*str).append((char *)contents, size * nmemb);
    return size * nmemb;
}

CURLcode http_get(const std::string &url, std::string &response, int timeout)
{
    CURLcode ret;
    CURL *pCURL = curl_easy_init();
    if (pCURL == NULL)
        return CURLE_FAILED_INIT;
    curl_easy_setopt(pCURL, CURLOPT_URL, url.c_str());
    curl_easy_setopt(pCURL, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(pCURL, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(pCURL, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(pCURL, CURLOPT_WRITEFUNCTION, &http_response);
    curl_easy_setopt(pCURL, CURLOPT_WRITEDATA, (void *)&response);
    ret = curl_easy_perform(pCURL);
    curl_easy_cleanup(pCURL);
    return ret;
}

CURLcode http_post(const std::string &url, const std::string &json, std::string &response, int timeout)
{
    CURLcode ret;
    char json_buf[1024];
    memset(json_buf, 0, sizeof(json_buf));
    strcpy(json_buf, json.c_str());
    CURL *pCURL = curl_easy_init();
    struct curl_slist *headers = NULL;
    if (pCURL == NULL)
        return CURLE_FAILED_INIT;
    curl_easy_setopt(pCURL, CURLOPT_URL, url.c_str());
    curl_easy_setopt(pCURL, CURLOPT_POST, 1L);
    headers = curl_slist_append(headers, "content-type:application/json");
    curl_easy_setopt(pCURL, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(pCURL, CURLOPT_POSTFIELDS, json_buf);
    curl_easy_setopt(pCURL, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(pCURL, CURLOPT_WRITEFUNCTION, &http_response);
    curl_easy_setopt(pCURL, CURLOPT_WRITEDATA, (void *)&response);
    ret = curl_easy_perform(pCURL);
    curl_slist_free_all(headers);
    curl_easy_cleanup(pCURL);
    return ret;
}