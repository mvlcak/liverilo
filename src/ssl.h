#ifndef LIVERILO_SSL_H
#define LIVERILO_SSL_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

SSL_CTX* init_ssl(char * root_path);
void cleanup_ssl(SSL_CTX* ctx, const bool* use_https);

#endif //LIVERILO_SSL_H
