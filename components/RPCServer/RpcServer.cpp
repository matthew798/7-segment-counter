#include <RpcServer.h>
#include <string>
#include <ctype.h>
#include "esp_log.h"

#define SHORT_STR_LEN 6
#define USHORT_STR_LEN 5
#define INT_STR_LEN 11
#define UINT_STR_LEN 10

/**
 * @brief Checks if a string represents a number
 * 
 * @param string The string to check
 * @return true If the string contains only digits and an optional '-' at the beginning
 * @return false If the string contains characters that are not part of a number
 */
bool isNumber(const char* string) {
    const size_t string_len = strlen(string);

    if(string_len < 1)
        return false;

    //First char should be a '-' or a digit
    if(string[0] != '-' && !isdigit(string[0])){ 
        return false;
    }
    else if(string_len == 1){
        return true;
    }

    //Check if secord char is a digit or the letter 'x' or 'X'
    if(!isdigit(string[1]) && string[1] != 'x' && string[1] != 'X'){
        return false;
    }
    else if(string_len == 2){
        return true;
    }

    //Everything else should be a digit or a '.'
    for(int i = 2; i < string_len; ++i) { 
        if(!isalnum(string[i]))
            return false;
    }

    return true;
}

/**
 * @brief Tries to parse a short from an input string
 * 
 * @param input The input string to attemp to parse
 * @param out Where the parsed value will be stored
 * @return true If parsing succeeded
 * @return false If parsing failed
 */
bool TryParseShort(const char* input, short& out){
    if(isNumber(input)){
        long val = strtol(input, nullptr, 0);
        if(val <= SHRT_MAX && val >= SHRT_MIN){
            out = (short)val;
            return true;
        }
    }

    return false;
}

/**
 * @brief Tries to parse an unsigned short from an input string
 * 
 * @param input The input string to attemp to parse
 * @param out Where the parsed value will be stored
 * @return true If parsing succeeded
 * @return false If parsing failed
 */
bool TryParseUshort(const char* input, ushort& out){
    if(isNumber(input)){
        long val = strtol(input, nullptr, 0);
        if(val <= USHRT_MAX && val >= 0){
            out = (ushort)val;
            return true;
        }
    }

    return false;
}

/**
 * @brief Tries to parse an int from an input string
 * 
 * @param input The input string to attemp to parse
 * @param out Where the parsed value will be stored
 * @return true If parsing succeeded
 * @return false If parsing failed
 */
bool TryParseInt(const char* input, int& out){
    if(isNumber(input)){
        long val = strtol(input, nullptr, 0);
        if(val <= INT_MAX && val >= INT_MIN){
            out = (int)val;
            return true;
        }
    }

    return false;
}

/**
 * @brief Tries to parse an unsigned int from an input string
 * 
 * @param input The input string to attemp to parse
 * @param out Where the parsed value will be stored
 * @return true If parsing succeeded
 * @return false If parsing failed
 */
bool TryParseUint(const char* input, uint& out){
    if(isNumber(input)){
        long val = strtol(input, nullptr, 0);
        if(val <= UINT_MAX && val >= 0){
            out = (uint)val;
            return true;
        }
    }

    return false;
}

/**
 * @brief Tries to parse a long from an input string
 * 
 * @param input The input string to attemp to parse
 * @param out Where the parsed value will be stored
 * @return true If parsing succeeded
 * @return false If parsing failed
 */
bool TryParseLong(const char* input, long& out){
    if(isNumber(input)){
        out = strtol(input, nullptr, 0);
        return true;
    }
    return false;
}

/**
 * @brief Tries to parse an unsigned long from an input string
 * 
 * @param input The input string to attemp to parse
 * @param out Where the parsed value will be stored
 * @return true If parsing succeeded
 * @return false If parsing failed
 */
bool TryParseUlong(const char* input, ulong& out){
    if(isNumber(input)){
        out = strtol(input, nullptr, 0);
        return true;
    }

    return false;
}

/**
 * @brief Construct a new rpc request::rpc request object
 * 
 * @param httpdReq The underlying httpd_req_t from which to pull data
 */
rpc_request::rpc_request(httpd_req_t *httpdReq) : httpdReq(httpdReq)
{
    size_t len = httpd_req_get_url_query_len(httpdReq) + 1;

    if (len > 1)
    {
        queryStrBuf = (char *)malloc(len);
        httpd_req_get_url_query_str(httpdReq, queryStrBuf, len);
    }
}

rpc_request::~rpc_request()
{
    //TODO This causes a corrupt heap, need to find out why
    /*if (queryStrBuf != NULL)
    {
        free(queryStrBuf);
    }*/
}

/**
 * @brief Attemps to parse a short from query string parameter key
 * 
 * @tparam
 * @param key The key of the parameter
 * @return short 
 */
template <>
bool rpc_request::TryGetParam(const char *key, short& out){
    char buf[9];
    auto res = httpd_query_key_value(queryStrBuf, key, buf, 8);

    if(res != ESP_OK){
        return false;
    }

    return TryParseShort(buf, out);
}

/**
 * @brief Attemps to parse an unsigned short from query string parameter key
 * 
 * @tparam
 * @param key The key of the parameter
 * @return ushort 
 */
template <>
bool rpc_request::TryGetParam(const char *key, ushort& out){
    char buf[9];
    auto res = httpd_query_key_value(queryStrBuf, key, buf, 8);

    if(res != ESP_OK){
        return false;
    }

    return TryParseUshort(buf, out);
}

/**
 * @brief Attemps to parse an int from query string parameter key
 * 
 * @tparam
 * @param key The key of the parameter
 * @return int 
 */
template <>
bool rpc_request::TryGetParam(const char *key, int& out){
    char buf[9];
    auto res = httpd_query_key_value(queryStrBuf, key, buf, 8);

    if(res != ESP_OK){
        return false;
    }

    return TryParseInt(buf, out);
}

/**
 * @brief Attemps to parse an unsigned int from query string parameter key
 * 
 * @tparam
 * @param key The key of the parameter
 * @return uint 
 */
template <>
bool rpc_request::TryGetParam(const char *key, uint& out){
    char buf[9];
    auto res = httpd_query_key_value(queryStrBuf, key, buf, 8);

    if(res != ESP_OK){
        return false;
    }

    return TryParseUint(buf, out);
}

/**
 * @brief Attemps to get the raw data from query string parameter key
 * 
 * @tparam
 * @param key The key of the parameter
 * @return uint 
 */
bool rpc_request::TryGetParam(const char *key, void* out, size_t len){
    return !httpd_query_key_value(queryStrBuf, key, (char *) out, len);
}

/**
 * @brief Construct a new Rpc Server:: Rpc Server object
 * 
 * @param uri The base endpoint
 * @details The base endpoint and individual handler endpoints make up the full URIs for handlers.
 * I.E. /base_ep/handler_ep?query_string
 */
RpcServer::RpcServer(const char *uri, httpd_handle_t server){
    this->uri = uri;
    this->server = server;
}

/**
 * @brief Starts the underlying HTTP server and begins listening for requests
 * If a server handle was provided on instantiation, nothing happens.
 * 
 * @return esp_err_t 
 *  - ESP_OK If the http server was started
 * - See errors for httpd_start for other error possibilities
 */
esp_err_t RpcServer::Start(ushort maxRoutes = 0){
    if(server == NULL){
        ESP_LOGI("RPC", "No server provided, starting a new one");
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
        if(maxRoutes > 0){
            config.max_uri_handlers = maxRoutes;
        }
        return httpd_start(&server, &config);
    }
    ESP_LOGI("RPC", "Server provided");
    return ESP_OK;
}

/**
 * @brief Stops the underlying HTTP server
 * 
 */
void RpcServer::Stop(){
    if (server) {
        httpd_stop(server);
    }
}

/**
 * @brief Registers a custom handler
 * 
 * @param uri Endpoint of this handler
 * @param method HTTP method for this handler. I.E. GET, POST, etc
 * @param handler Handler function
 * @return esp_err_t 
 *  - ESP_OK If the handler is registered succesfully
 *  - See errors for httpd_register_uri_handler for other error possibilities
 * 
 * @details A custom handler (rpc_handler_t) receives an rpc_request object, a buffer for the response and
 * the buffer size. Query string parameters are read directly from the rpc_request object. If a response payload
 * is required it can be written directly to the provided response buffer.
 */
esp_err_t RpcServer::RegisterHandler(const char *uri, httpd_method_t method, rpc_handler_t handler){

    int len = strlen(this->uri) + strlen(uri);
    char ep[len + 1];

    assert(len > 0);

    strcpy(ep, this->uri);
    strcat(ep, uri);

    httpd_uri_t cfg = {
        .uri = ep,
        .method = method,
        .handler = Handle,
        .user_ctx = (void *)handler
    };

    return httpd_register_uri_handler(server, &cfg);
}

/**
 * @brief Global scope handler for custom RPC handlers
 * 
 * @param req The httpd_req_t* provided by httpd
 * @return esp_err_t
 *  - ESP_OK If the response was successfully sent
 *  - See errors for httpd_resp_send for other error possibilities
 * 
 * @details This function always returns ESP_OK. Errors in the handler
 * are mapped to HTTP errors and sent to the client.
 */
esp_err_t RpcServer::Handle(httpd_req_t *req)
{
    rpc_handler_t handler = (rpc_handler_t) req->user_ctx;

    rpc_request rpcReq(req);
    char resp[256];
    esp_err_t res = handler(&rpcReq, resp, 256);

    if(res){
        switch (res)
        {
        case ESP_ERR_INVALID_ARG:
            httpd_resp_set_status(req, HTTPD_400);
            break;
        case ESP_ERR_NOT_FOUND:
            httpd_resp_set_status(req, HTTPD_404);
            break;
        case ESP_FAIL:
        default:
            httpd_resp_set_status(req, HTTPD_500);
            break;
        }
    }

    return httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
}

template <>
esp_err_t RpcServer::HandleSet<short>(httpd_req_t *req){
    auto setter = (rpc_set_handler_t<short>)req->user_ctx;
    char buf[17];
    
    int len = httpd_req_recv(req, buf, 16);
    buf[len] = '\0'; //httpd_req_rcv doesn't write '\0'

    short val;

    if (!TryParseShort(buf, val) || setter(val))
    {
        httpd_resp_set_status(req, HTTPD_400);
    }

    return httpd_resp_send(req, NULL, 0);
}

template <>
esp_err_t RpcServer::HandleSet<ushort>(httpd_req_t *req){
    auto setter = (rpc_set_handler_t<ushort>)req->user_ctx;
    char buf[17];
    
    int len = httpd_req_recv(req, buf, 16);
    buf[len] = '\0'; //httpd_req_rcv doesn't write '\0'

    ushort val;

    if (!TryParseUshort(buf, val) || setter(val))
    {
        httpd_resp_set_status(req, HTTPD_400);
    }

    return httpd_resp_send(req, NULL, 0);
}

template <>
esp_err_t RpcServer::HandleSet<int>(httpd_req_t *req){
    auto setter = (rpc_set_handler_t<int>)req->user_ctx;
    char buf[17];

    int len = httpd_req_recv(req, buf, 16);
    buf[len] = '\0'; //httpd_req_rcv doesn't write '\0'

    int val;

    if (!TryParseInt(buf, val) || setter(val))
    {
        httpd_resp_set_status(req, HTTPD_400);
    }

    return httpd_resp_send(req, NULL, 0);
}

template <>
esp_err_t RpcServer::HandleSet<uint>(httpd_req_t *req){
    auto setter = (rpc_set_handler_t<uint>)req->user_ctx;
    char buf[17];

    int len = httpd_req_recv(req, buf, 16);
    buf[len] = '\0'; //httpd_req_rcv doesn't write '\0'

    uint val;

    if (!TryParseUint(buf, val) || setter(val))
    {
        httpd_resp_set_status(req, HTTPD_400);
    }

    return httpd_resp_send(req, NULL, 0);
}

template<>
esp_err_t RpcServer::HandleSet<long>(httpd_req_t *req){
    auto setter = (rpc_set_handler_t<uint>)req->user_ctx;
    char buf[17];

    int len = httpd_req_recv(req, buf, 16);
    buf[len] = '\0'; //httpd_req_rcv doesn't write '\0'

    long val;

    if (!TryParseLong(buf, val) || setter(val))
    {
        httpd_resp_set_status(req, HTTPD_400);
    }

    return httpd_resp_send(req, NULL, 0);
}

template<>
esp_err_t RpcServer::HandleSet<ulong>(httpd_req_t *req){
    auto setter = (rpc_set_handler_t<uint>)req->user_ctx;
    char buf[17];

    int len = httpd_req_recv(req, buf, 16);
    buf[len] = '\0'; //httpd_req_rcv doesn't write '\0'

    ulong val;

    if (!TryParseUlong(buf, val) || setter(val))
    {
        httpd_resp_set_status(req, HTTPD_400);
    }

    return httpd_resp_send(req, NULL, 0);
}

template <>
esp_err_t RpcServer::HandleSet<const char*>(httpd_req_t* req){
    auto setter = (rpc_set_handler_t<const char*>)req->user_ctx;
    const int len = req->content_len;
    char buf[len + 1];

    httpd_req_recv(req, buf, len);
    buf[len] = '\0';

    if(setter(buf)){
        httpd_resp_set_status(req, HTTPD_400);
    }

    return httpd_resp_send(req, NULL, 0);
}