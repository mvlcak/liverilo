//
// Created by Martin on 14/09/2025.
//

#ifndef C_SOCKETS_CONFIG_H
#define C_SOCKETS_CONFIG_H
typedef struct config {
    char* www_path;
    char* ssl_path;
    int port;
    char* ssl_cert;
    char* ssl_key;
    bool use_https;
} config_t;
config_t* load_config(const char* path);
#endif //C_SOCKETS_CONFIG_H