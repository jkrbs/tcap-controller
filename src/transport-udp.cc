#include <transport_UDP.hpp>
#include <util.hpp>

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <glog/logging.h>

void UDPTransport::initialize_socket() {
    LOG(INFO) << "Initializing Socket listenaddress: " << listen_address <<":" << listen_port;

    
    this->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->socket_fd == -1) {
        printf("Could not create socket");
    }

    struct sockaddr_in hints;
    memset(&hints, 0, sizeof(hints));
    hints.sin_family = AF_INET;
    hints.sin_port = htons(listen_port);
    inet_pton(AF_INET, this->listen_address.c_str(), &hints.sin_addr);
    int r = bind(this->socket_fd, reinterpret_cast<struct sockaddr*>(&hints), sizeof(hints));
    if(r != 0 || f_addrinfo == NULL)
    {
        LOG(FATAL) << "invalid address or port: \"" + this->listen_address + ":" << this->listen_port << "\"";
    }
}

std::size_t UDPTransport::send(std::span<uint8_t> buf) {
    ::send(this->socket_fd, buf.data(),buf.size(), NULL);
}


std::size_t UDPTransport::recv(std::span<uint8_t> buf) {
    LOG(INFO) << "Recv on CPU control-plane UDP port " << listen_port;

    size_t len = ::recv(this->socket_fd, buf.data(), buf.size(), NULL);
    if(len > buf.size_bytes()) {
        LOG(FATAL) << "Received more bytes than the buffer can fit!";
    }    
    LOG(INFO) << "Received " << len << "bytes";
    util::hexdump(buf.data(), len);
    return len;
}

UDPTransport::~UDPTransport() {
    close(this->socket_fd);
}