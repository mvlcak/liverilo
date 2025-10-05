# Liverilo
Minimal web server developed in C 

## Compilation of executable
1. Install dependencies
```bash
sudo apt-get install libssl-dev gcc make
```
2. Create build directory
```bash
mkdir build && cd build
```
3. Create Makefile with cmake
```bash
cmake ..
```
4. Compile the project
```bash
make
```
Executable will be located in `build/liverilo`

## Configuration
1. place executable at `/usr/local/bin/liverilo`
2. create config directory
```bash
sudo mkdir -p /etc/liverilo
```
3. create www directory
```bash
sudo mkdir -p /var/www/liverilo/html
```
4. create ssl directory
```bash
sudo mkdir -p /etc/liverilo/ssl
```
5. create server configuration file at `/etc/liverilo/server.conf` (example below)
6. place your website files in `/var/www/liverilo/html`
7. Optional: Place your ssl certificate and key in `/etc/liverilo/ssl` (as specified in config file)

### Example of config with ssl certificate and key
```conf
#config file should be located at /etc/liverilo/server.conf
port 443
www_path /var/www/liverilo/html
ssl_path /etc/liverilo/ssl
ssl_cert_name server.crt
ssl_key_name server.key
```

### Example of config without ssl
```conf
#config file should be located at /etc/liverilo/server.conf
port 80
www_path /var/www/liverilo/html
```