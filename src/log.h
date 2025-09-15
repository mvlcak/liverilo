
#ifndef C_SOCKETS_LOG_H
#define C_SOCKETS_LOG_H
void log_request(const char* ip_address,const char* method, const char* path, int status_code);
#endif //C_SOCKETS_LOG_H