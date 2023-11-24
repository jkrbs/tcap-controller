#include <transport_UDP.hpp>
#include <string.h>
#include <unistd.h>

#include <glog/logging.h>

void UDPTransport::initialize_socket() {
    LOG(INFO) << "Initializing Socket listenaddress: " << listen_address <<":" << listen_port;

    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", this->listen_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int r(getaddrinfo(this->listen_address.c_str(), decimal_port, &hints, &f_addrinfo));
    if(r != 0 || f_addrinfo == NULL)
    {
        LOG(FATAL) << "invalid address or port: \"" + this->listen_address + ":" + decimal_port + "\"";
    }
    this->socket_fd = socket(f_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(this->socket_fd == -1)
    {
        freeaddrinfo(f_addrinfo);
        LOG(FATAL)  << "could not create socket for: " << this->listen_address + ":" + decimal_port;
    }
}

std::size_t UDPTransport::send(std::span<uint8_t> buf) {
    ::send(this->socket_fd, buf.data(),buf.size(), NULL);
}


std::size_t UDPTransport::recv(std::span<uint8_t> buf) {
    LOG(INFO) << "Recv on CPU control-plane UDP port " << listen_port;
    return ::recv(this->socket_fd, buf.data(), buf.size(), NULL);
}

UDPTransport::~UDPTransport() {
    freeaddrinfo(this->f_addrinfo);
    close(this->socket_fd);
}