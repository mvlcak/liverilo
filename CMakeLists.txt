cmake_minimum_required(VERSION 3.11)
project(liverilo C)

set(CMAKE_C_STANDARD 23)
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
add_executable(liverilo src/main.c
        src/server.c
        src/server.h
        src/file.c
        src/file.h
        src/ssl.c
        src/ssl.h
        src/log.c
        src/log.h
        src/config.c
        src/config.h
)
target_link_libraries(liverilo PRIVATE OpenSSL::SSL OpenSSL::Crypto)