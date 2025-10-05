

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

config_t* load_config(const char* config_file_path) {
    config_t* config = malloc(sizeof(config_t));
    if (!config) return NULL;

    // Initialize with defaults
    config->www_path = strdup("/var/www/liverilo/html");
    config->ssl_path = strdup("/etc/liverilo/ssl");
    if (!config->www_path || !config->ssl_path) {
        free(config->www_path);
        free(config->ssl_path);
        free(config);
        return NULL;
    }

    config->port = 8080;
    config->ssl_cert = NULL;
    config->ssl_key = NULL;
    config->use_https = false;

    FILE* file = fopen(config_file_path, "r");
    if (!file) {
        free(config->www_path);
        free(config->ssl_path);
        free(config);
        return NULL;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = '\0';

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') continue;

        char key[128], value[512];
        if (sscanf(line, "%127s %511s", key, value) == 2) {
            if (strcmp(key, "port") == 0) {
                char* endptr;
                long port_long = strtol(value, &endptr, 10);

                // Check for conversion errors
                if (*endptr != '\0' || port_long < 1 || port_long > 65535) {
                    // Invalid port number, keep default
                    continue;
                }

                config->port = (int)port_long;
            } else if (strcmp(key, "www_path") == 0) {
                char* new_path = strdup(value);
                if (new_path) {
                    free(config->www_path);
                    config->www_path = new_path;
                }
            } else if (strcmp(key, "ssl_path") == 0) {
                char* new_path = strdup(value);
                if (new_path) {
                    free(config->ssl_path);
                    config->ssl_path = new_path;
                    config->use_https = true;
                }
            } else if (strcmp(key, "ssl_cert_name") == 0) {
                char* new_cert = strdup(value);
                if (new_cert) {
                    free(config->ssl_cert);
                    config->ssl_cert = new_cert;
                    config->use_https = true;
                }
            } else if (strcmp(key, "ssl_key_name") == 0) {
                char* new_key = strdup(value);
                if (new_key) {
                    free(config->ssl_key);
                    config->ssl_key = new_key;
                    config->use_https = true;
                }
            }
        }
    }

    fclose(file);
    return config;
}
