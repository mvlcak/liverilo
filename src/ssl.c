#include "ssl.h"

#include <stdio.h>

SSL_CTX* init_ssl(char* root_path) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    const SSL_METHOD* method = TLS_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    // Set minimum and maximum TLS versions
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);

    // Configure cipher suites for TLS 1.2
    const char* cipher_list =
        "ECDHE-RSA-AES256-GCM-SHA384:"
        "ECDHE-RSA-AES128-GCM-SHA256:"
        "ECDHE-RSA-AES256-SHA384:"
        "ECDHE-RSA-AES128-SHA256";

    if (SSL_CTX_set_cipher_list(ctx, cipher_list) != 1) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }

    // Load certificate and private key files
    char crt_path[512];
    snprintf(crt_path, sizeof(crt_path), "%s/server.crt", root_path);
    char key_path[512];
    snprintf(key_path, sizeof(key_path), "%s/server.key", root_path);
    if (SSL_CTX_use_certificate_file(ctx, crt_path, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }

    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match certificate\n");
        SSL_CTX_free(ctx);
        return NULL;
    }

    return ctx;
}

void cleanup_ssl(SSL_CTX* ctx,const bool* use_https) {
    if (use_https) {
        SSL_CTX_free(ctx);
        EVP_cleanup();
    }
}
