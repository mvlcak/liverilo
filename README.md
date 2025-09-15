# Liverilo
Minimal web server developed in C 

## Config
/usr/local/bin/liverilo
config file path: `/etc/liverilo/server.conf`
```conf
port 443
www_path /var/www/liverilo/html
ssl_path /etc/liverilo/ssl
ssl_cert_name server.crt
ssl_key_name server.key
```

## Installation
sudo apt-get install libssl-dev