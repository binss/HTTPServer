#include "RequestHandler.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

// 注意！由于匹配是从前向后执行最短匹配，所以"accept"要放到"accept-encoding"后，不然他们都会被"accept"匹配掉
const char * FIELD_NAME_LIST[] = {"host", "connection", "content-length", "cache-control", "referer", "cookie", "user-agent",
                                    "accept-encoding", "accept-language", "accept", "upgrade", "sec-websocket-key", "origin"};
const int FIELD_NAME_LIST_LEN = 13;

RequestHandler::RequestHandler()
{
    settings_.on_header_field = on_header_field;
    settings_.on_header_value = on_header_value;
    settings_.on_message_complete = on_message_complete;
    settings_.on_url = on_url;

    parser_.data = (void*)this;
    // parser_ = malloc(sizeof(http_parser));
    http_parser_init(&parser_, HTTP_REQUEST);
    flag_ = -1;
    http_header_.Reset();
}



int RequestHandler::ParseRequest(char buf[], int len)
{
    if( len <= 0 )
    {
        return -1;
    }
    int nparsed = http_parser_execute(&parser_, &settings_, buf, len);
    if (parser_.upgrade)
    {
        /* handle new protocol */
    }
    else if(nparsed != len)
    {
        /* Handle error. Usually just close the connection. */
    }
    // cout<<"method:"<<parser_.method<<endl;
    return 0;
}

int RequestHandler::on_url( http_parser *parser, const char *at, size_t len )
{
    RequestHandler * handler = (RequestHandler*)parser->data;
    handler->HandleUrl(at, len);
    return 0;
}


int RequestHandler::HandleUrl( const char *at, size_t len )
{
    return 0;
}

int RequestHandler::on_message_complete( http_parser *parser )
{
    RequestHandler * handler = (RequestHandler*)parser->data;
    handler->HandleComplete();
    return 0;

}

int RequestHandler::HandleComplete()
{
    cout<<"--------Header---------"<<endl;
    cout<<http_header_.domain_name<<endl;
    cout<<http_header_.connection<<endl;
    cout<<http_header_.cache_control<<endl;
    cout<<http_header_.user_agent<<endl;
    cout<<http_header_.accept<<endl;
    cout<<http_header_.accept_encoding<<endl;
    cout<<http_header_.accept_language<<endl;
    cout<<http_header_.cookie<<endl;
    cout<<"-----------------------"<<endl;
    return 0;
}
int RequestHandler::on_header_field( http_parser *parser, const char *at, size_t len )
{
    RequestHandler * handler = (RequestHandler*)parser->data;
    handler->HandleField(at, len);
    return 0;
}

int RequestHandler::HandleField(const char *at, size_t len )
{
    if(!at)
    {
        return -1;
    }

    // 如果键匹配，下一个读到的value就是对应的值
    char temp[32] = {0};
    for(int i = 0; i < (int)len && i < (int)sizeof(temp); i++)
    {
        temp[i] = tolower(at[i]);
    }

    flag_ = -1;

    for(int i=0; i < FIELD_NAME_LIST_LEN; i++)
    {
        // cout<<temp<<endl;
        if(!memcmp(temp, FIELD_NAME_LIST[i], strlen(FIELD_NAME_LIST[i])))
        {
            flag_ = i;
            break;
        }
    }
    return 0;
}

int RequestHandler::on_header_value( http_parser *parser, const char *at, size_t len )
{
    RequestHandler * handler = (RequestHandler*)parser->data;
    handler->HandlerValue(at, len);
    return 0;
}

int RequestHandler::HandlerValue(const char *at, size_t len )
{
    if(!at)
    {
        return -1;
    }
    // cout<<"flag:"<<flag_<<endl<<"len:"<<len<<endl<<"at:"<<at;

    switch(flag_)
    {
        // host
        case 0:
        {
            if(len > MAX_DOMAINNAME_LEN)
            {
                len = MAX_DOMAINNAME_LEN - 1;
            }

            memcpy(http_header_.domain_name, at, len);
            http_header_.domain_name[len] = 0;
            break;
        }
        // connection
        case 1:
        {
            if (len >= MAX_CONNECTION_LEN)
            {
                len = MAX_CONNECTION_LEN - 1;
            }

            memcpy(http_header_.connection, at, len);
            http_header_.connection[len] = 0;
            break;
        }
        // content-length
        case 2:
        {
            char char_len[6] = {0};
            memcpy(char_len, at, len);
            http_header_.content_len = atoi(char_len);
            break;

        }
        // cache_control
        case 3:
        {
            if (len >= MAX_CACHE_CONTROL_LEN)
            {
                len = MAX_CACHE_CONTROL_LEN - 1;
            }
            memcpy(http_header_.cache_control, at, len);
            http_header_.cache_control[len] = 0;
            break;
        }
        //referer
        case 4:
        {
            if (len >= MAX_REFERER_LEN)
            {
                len = MAX_REFERER_LEN - 1;
            }

            memcpy(http_header_.referer, at, len);
            http_header_.referer[len] = 0;
            break;
        }
        // cookie
        case 5:
        {
            if(len > MAX_COOKIE_LEN)
            {
                len = MAX_COOKIE_LEN-1;
            }

            memcpy(http_header_.cookie, at, len);
            http_header_.cookie[len] = 0;
            break;

        }
        // user_agent
        case 6:
        {
            if(len > MAX_USER_AGENT_LEN)
            {
                len = MAX_USER_AGENT_LEN-1;
            }

            memcpy(http_header_.user_agent, at, len);
            http_header_.user_agent[len] = 0;
            break;

        }
        // accept_encoding
        case 7:
        {
            if(len > MAX_ACCEPT_ENCODING_LEN)
            {
                len = MAX_ACCEPT_ENCODING_LEN-1;
            }
            memcpy(http_header_.accept_encoding, at, len);
            http_header_.accept_encoding[len] = 0;
            break;

        }
        // accept_language
        case 8:
        {
            if(len > MAX_ACCEPT_LANGUAGE_LEN)
            {
                len = MAX_ACCEPT_LANGUAGE_LEN-1;
            }

            memcpy(http_header_.accept_language, at, len);
            http_header_.accept_language[len] = 0;
            break;
        }
        // accept
        case 9:
        {
            if(len > MAX_ACCEPT_LEN)
            {
                len = MAX_ACCEPT_LEN-1;
            }

            memcpy(http_header_.accept, at, len);
            http_header_.accept[len] = 0;
            break;
        }
        // upgrade
        case 10:
        {
            if (len >= MAX_UPGRADE_LEN)
            {
                len = MAX_UPGRADE_LEN - 1;
            }

            memcpy(http_header_.upgrade, at, len);
            http_header_.upgrade[len] = 0;
            break;
        }
        // sec-websocket-key
        case 11:
        {
            if (len >= MAX_SECWEBSOCKETKEY_LEN)
            {
                len = MAX_SECWEBSOCKETKEY_LEN - 1;
            }

            memcpy(http_header_.sec_websocket_key, at, len);
            http_header_.sec_websocket_key[len] = 0;
            break;
        }
        // origin
        case 12:
        {
            if (len >= MAX_DOMAINNAME_LEN)
            {
                len = MAX_DOMAINNAME_LEN - 1;
            }
            memcpy(http_header_.origin, at, len);
            http_header_.origin[len] = 0;
            break;
        }
        default:
        {
            // cout<<"The Field is undefined. Value Ignored."<<endl;
            break;
        }
    }
    return 0;
}

