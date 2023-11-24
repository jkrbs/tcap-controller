#pragma once
#include <transport.hpp>
#include <string>
#include <memory>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <config.hpp>

static Config default_config = Config("10.0.0.1", 1234, "veth10");

class UDPTransport : Transport
{
private:
    struct addrinfo *   f_addrinfo;
    std::string listen_address;
    uint16_t listen_port;
    std::string listen_interface;
    int socket_fd = 0;
    void initialize_socket();
public:
    UDPTransport() : UDPTransport(std::shared_ptr<Config>(&default_config)){};

    UDPTransport(std::shared_ptr<Config> conf) 
    : 
    listen_address(conf->listen_address),
    listen_interface(conf->listen_interface),
    listen_port(conf->listen_port) {
        this->initialize_socket();    
    };

    UDPTransport(std::string listen_address, uint16_t listen_port, std::string listen_interface) : 
    listen_address(listen_address), 
    listen_interface(listen_interface),
    listen_port(listen_port){ 
        this->initialize_socket();    
    };

    ~UDPTransport();

    std::size_t send(std::span<uint8_t> buf);

    std::size_t recv(std::span<uint8_t> buf);
};
