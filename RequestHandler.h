#ifndef  __REQUEST_HANDLER_HPP__
#define  __REQUEST_HANDLER_HPP__

#include "http_parser.h"

// 最大Referer长度
const unsigned int MAX_REFERER_LEN = 256;

// websocket握手请求中Upgrade字段最大长度
const unsigned int MAX_UPGRADE_LEN = 16;

// websocket握手请求中SecWebSocketKey字段最大长度
const unsigned int MAX_SECWEBSOCKETKEY_LEN = 32;

// 最大域名长度
const unsigned int MAX_DOMAINNAME_LEN = 64;
// 最大cookie长度
const unsigned int MAX_COOKIE_LEN = 10240;

const unsigned int MAX_USER_AGENT_LEN = 1024;

const unsigned int MAX_CACHE_CONTROL_LEN = 1024;
const unsigned int MAX_ACCEPT_ENCODING_LEN = 1024;
const unsigned int MAX_ACCEPT_LANGUAGE_LEN = 1024;
const unsigned int MAX_ACCEPT_LEN = 1024;
const unsigned int MAX_CONNECTION_LEN = 64;

extern const char * FIELD_NAME_LIST[];

struct HttpHeader
{
    int content_len;
    char domain_name[MAX_DOMAINNAME_LEN];
    char cookie[MAX_COOKIE_LEN];
    char connection[MAX_CONNECTION_LEN];
    char referer[MAX_REFERER_LEN];
    char upgrade[MAX_UPGRADE_LEN];
    char sec_websocket_key[MAX_SECWEBSOCKETKEY_LEN];
    char origin[MAX_DOMAINNAME_LEN];
    char user_agent[MAX_USER_AGENT_LEN];
    char cache_control[MAX_CACHE_CONTROL_LEN];
    char accept_encoding[MAX_ACCEPT_ENCODING_LEN];
    char accept_language[MAX_ACCEPT_LANGUAGE_LEN];
    char accept[MAX_ACCEPT_LEN];
    void Reset()
    {
        content_len = 0;
        domain_name[0] = 0;
        cookie[0] = 0;
        connection[0] = 0;
        referer[0] = 0;
        upgrade[0] = 0;
        sec_websocket_key[0] = 0;
        origin[0] = 0;
        user_agent[0] = 0;
        cache_control[0] = 0;
        accept_encoding[0] = 0;
        accept_language[0] = 0;
        accept[0] = 0;
    }
};

class RequestHandler
{
public:
    RequestHandler();
    HttpHeader GetHttpHeader(){ return http_header_; };
    int ParseRequest(char *buf, int len);
private:
    static int on_message_complete( http_parser *_ );
    static int on_header_field( http_parser *_, const char *at, size_t len );
    static int on_header_value( http_parser *_, const char *at, size_t len );
    static int on_url( http_parser *_, const char *at, size_t len );

    int HandleComplete();
    int HandleUrl(const char *at, size_t len );
    int HandleField(const char *at, size_t len );
    int HandlerValue(const char *at, size_t len );

    HttpHeader http_header_;
    int flag_;
    http_parser_settings settings_;
    http_parser parser_;
};

#endif
