

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

config_t* load_config(const char* config_file_path) {
    config_t* config = malloc(sizeof(config_t));
    if (!config) return NULL;

    // Initialize with defaults
    config->www_path = "/var/www/liverilo/html";
    config->ssl_path = "/etc/liverilo/ssl";
    config->port = 8080;
    config->ssl_cert = NULL;
    config->ssl_key = NULL;
    config->use_https = false;

    FILE* file = fopen(config_file_path, "r");
    if (!file) {
        free(config);
        return NULL;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = '\0';

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') continue;

        char key[64], value[192];
        if (sscanf(line, "%s %s", key, value) == 2) {
            if (strcmp(key, "port") == 0) {
                config->port = atoi(value);
            } else if (strcmp(key, "www_path") == 0) {
                config->www_path = strdup(value);
            } else if (strcmp(key, "ssl_path") == 0) {
                config->ssl_path = strdup(value);
                config->use_https = true;
            } else if (strcmp(key, "ssl_cert_name") == 0) {
                config->ssl_cert = strdup(value);
                config->use_https = true;
            } else if (strcmp(key, "ssl_key_name") == 0) {
                config->ssl_key = strdup(value);
                config->use_https = true;
            }
        }
    }

    fclose(file);
    return config;
}
