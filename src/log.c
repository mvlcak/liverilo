#include "log.h"
#include <stdio.h>

void log_request(const char* ip_address,const char* method, const char* path, int status_code) {
    printf("%s;%s;%s;%d\n",ip_address,method,path,status_code);
};
