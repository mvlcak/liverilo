#include "server.h"
#include "config.h"


int main(int argc, char **argv) {
    char* config_file_path;
    if (argc != 2) {
        config_file_path = "/etc/liverilo/server.conf";
    } else {
        config_file_path = argv[1];
    }
    config_t* config = load_config(config_file_path);
    start_server_socket(config);
    return 0;
}
