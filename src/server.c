#include "server.h"

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "ssl.h"
#include "file.h"
#include "log.h"

#define BACKLOG 16
#define REQ_BUFFER_SIZE 2048

typedef struct {
    int client_fd;
    SSL_CTX* ssl_ctx;
    char* www_path;
    struct sockaddr_in client_addr;
    bool use_https;
} client_data_t;


char* get_client_ip(struct sockaddr_in* client_addr) {
    char* ip_str = malloc(INET_ADDRSTRLEN);
    if (!ip_str) return NULL;

    if (inet_ntop(AF_INET, &(client_addr->sin_addr), ip_str, INET_ADDRSTRLEN) == NULL) {
        free(ip_str);
        return NULL;
    }

    return ip_str;
}

char* extract_method_from_request(const char* reqbuf) {
    const char* space = strchr(reqbuf, ' ');
    if (!space) return NULL;
    size_t len = space - reqbuf;
    char* method = malloc(len + 1);
    if (!method) return NULL;
    strncpy(method, reqbuf, len);
    method[len] = '\0';
    return method;
}


char* extract_path_from_request(const char* reqbuf) {
    // Find first space (after HTTP method like GET, POST)
    const char* path_start = strchr(reqbuf, ' ');
    if (!path_start) return NULL;
    path_start++; // Skip the space

    // Find second space (before HTTP version)
    const char* path_end = strchr(path_start, ' ');
    if (!path_end) return NULL;

    // Calculate path length
    size_t path_len = path_end - path_start;

    // Allocate and copy path
    char* path = malloc(path_len + 1);
    if (!path) return NULL;

    strncpy(path, path_start, path_len);
    path[path_len] = '\0';

    return path;
}


int get_hdr_len(char header[512], int body_len, enum FILE_TYPE ftype) {
    const char* content_type;

    switch (ftype) {
        case HTML:
            content_type = "text/html; charset=UTF-8";
            break;
        case CSS:
            content_type = "text/css; charset=UTF-8";
            break;
        case JS:
            content_type = "application/javascript";
            break;
        case JSON:
            content_type = "application/json";
            break;
        case GIF:
            content_type = "image/gif";
            break;
        case ICO:
            content_type = "image/x-icon";
            break;
        default:
            content_type = "text/plain";
            break;
    }

    return snprintf(header, 512,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "Server: \r\n"
                    "\r\n",
                    content_type, body_len);
}

enum FILE_TYPE get_file_type(const char* filename) {
    if (!filename) return OTHER;

    if (strcmp(filename,"/")==0) {
        return HTML;
    }
    const char* ext = strrchr(filename, '.');
    if (!ext) return OTHER;

    ext++; // Skip the dot

    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) return HTML;
    if (strcmp(ext, "css") == 0) return CSS;
    if (strcmp(ext, "js") == 0) return JS;
    if (strcmp(ext, "png") == 0) return PNG;
    if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) return JPG;
    if (strcmp(ext, "gif") == 0) return GIF;
    if (strcmp(ext, "ico") == 0) return ICO;
    if (strcmp(ext, "json") == 0) return JSON;
    if (strcmp(ext, "xml") == 0) return XML;
    if (strcmp(ext, "txt") == 0) return TXT;
    if (strcmp(ext, "pdf") == 0) return PDF;

    return OTHER;
}

int is_static_asset(const char* path) {
    if (!path) return 0;

    const char* ext = strrchr(path, '.');
    if (!ext) return 0; // No extension = not a static asset

    ext++; // Skip the dot

    // List of static asset extensions
    return (strcmp(ext, "css") == 0 ||
            strcmp(ext, "js") == 0 ||
            strcmp(ext, "png") == 0 ||
            strcmp(ext, "jpg") == 0 ||
            strcmp(ext, "jpeg") == 0 ||
            strcmp(ext, "gif") == 0 ||
            strcmp(ext, "ico") == 0 ||
            strcmp(ext, "svg") == 0 ||
            strcmp(ext, "woff") == 0 ||
            strcmp(ext, "woff2") == 0 ||
            strcmp(ext, "ttf") == 0) ||
            strcmp(ext, "json") == 0 ||
            strcmp(ext, "md") == 0;
}

void send_404_response(const bool use_https,SSL* ssl, int datafd) {
    const char* response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 47\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>404 - Not Found</h1></body></html>";

    if (use_https) {
        SSL_write(ssl, response, strlen(response));
    } else {
        write(datafd, response, strlen(response));
    }
}


void* handle_client(void* arg) {
    client_data_t* data = (client_data_t*)arg;

    SSL* ssl = NULL;
    if (data->use_https) {
        // Create SSL connection
        ssl = SSL_new(data->ssl_ctx);
        SSL_set_fd(ssl, data->client_fd);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(data->client_fd);
            free(data);
            return NULL;
        }
    }




    char reqbuf[REQ_BUFFER_SIZE];
    int r;
    if (data->use_https) {
        r = SSL_read(ssl, reqbuf, sizeof(reqbuf) - 1);
    } else {
        r = read(data->client_fd, reqbuf, sizeof(reqbuf) - 1);
    }
    char* path = NULL;

    if (r > 0) {
        if (r >= REQ_BUFFER_SIZE) r = REQ_BUFFER_SIZE - 1;
        reqbuf[r] = '\0';
        path = extract_path_from_request(reqbuf);
    }

    char full_path[512];
    unsigned char *source = NULL;
    size_t file_size = 0;
    enum FILE_TYPE ftype = OTHER;
    // Check for path traversal attempts
    if (strstr(path, "..") || strstr(path, "//")) {
        send_404_response(data->use_https, ssl, data->client_fd);
        goto cleanup;
    } else if(path && (strcmp(path, "/") == 0 || !is_static_asset(path))) {
        // If path is "/" or not a static asset, serve index.html
        snprintf(full_path, sizeof(full_path),
            "%s/index.html", data->www_path);
        source = read_file_to_bytes(full_path, &file_size);
        ftype = HTML;
    } else if (path) {
        // Try to find the actual file first
        snprintf(full_path, sizeof(full_path), "%s%s", data->www_path, path);
        source = read_file_to_bytes(full_path, &file_size);

        if (!source) {
            // File doesn't exist - check if it's a static asset request
            if (is_static_asset(path)) {
                // Send 404 for missing static assets

                send_404_response(data->use_https,ssl, data->client_fd);

                goto cleanup;
            }
        } else {
            ftype = get_file_type(path);
        }
    }


    char header[512];
    if (source) {
        int body_len = (int)file_size;
        int hdr_len = get_hdr_len(header, body_len, ftype);

        if (data->use_https) {
            // Send headers then body
            SSL_write(ssl, header, hdr_len);
            SSL_write(ssl, source, body_len);
        } else {
            write(data->client_fd,header, hdr_len);
            write(data->client_fd,source, body_len);
        }

        free(source);
    }
    //Get method
    char* method = extract_method_from_request(reqbuf);


    // Get client IP address
    char* ip_address = get_client_ip(&data->client_addr);
    //Log
    log_request(ip_address, method, path, 200);
    cleanup:
    // Cleanup
    if (path) {
        free(path);
    }
    if (ip_address) {
        free(ip_address);
    }
    if (method) {
        free(method);
    }

    if (data->use_https && ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }

    close(data->client_fd);
    free(data);

    return NULL;
}


int start_server_socket(const config_t *config) {

    SSL_CTX* ssl_ctx;
    // Initialize SSL
    if (config->use_https) {
        ssl_ctx = init_ssl(config->ssl_path);
        if (!ssl_ctx) {
            fprintf(stderr, "Failed to initialize SSL\n");
            return 1;
        }
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");

        cleanup_ssl(ssl_ctx, &config->use_https);

        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        if (config->use_https) {
            cleanup_ssl(ssl_ctx, &config->use_https);
        }
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(config->port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        if (config->use_https) {
            cleanup_ssl(ssl_ctx, &config->use_https);
        }
        return 1;
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        if (config->use_https) {
            cleanup_ssl(ssl_ctx, &config->use_https);
        }
        return 1;
    }

    printf("Web server listening on port %d...\n", config->port);

    while (1) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int client_fd = accept(server_fd, (struct sockaddr*)&caddr, &clen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        // Create thread data
        client_data_t* data = malloc(sizeof(client_data_t));
        if (!data) {
            perror("malloc");
            close(client_fd);
            continue;
        }

        data->client_fd = client_fd;
        data->ssl_ctx = ssl_ctx;
        data->www_path = config->www_path;
        data->client_addr = caddr;
        data->use_https = config->use_https;

        // Create thread to handle client
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, data) != 0) {
            perror("pthread_create");
            close(client_fd);
            free(data);
            continue;
        }

        // Detach thread so it cleans up automatically when done
        pthread_detach(thread);
    }

    close(server_fd);
    cleanup_ssl(ssl_ctx, &config->use_https);
    return 0;
}

