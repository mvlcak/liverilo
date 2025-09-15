
#ifndef LIVERILO_SERVER_H
#define LIVERILO_SERVER_H
#include "config.h"
int start_server_socket(const config_t* config);
typedef enum FILE_TYPE { HTML, CSS, JS, PNG, JPG, GIF, ICO, JSON, XML, TXT, PDF,OTHER } file_type;
#endif //LIVERILO_SERVER_H