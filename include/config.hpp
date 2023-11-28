#pragma once
#include <cstdint>
#include <memory>

class Config {
	public:
	uint16_t listen_port = 0;
	std::string listen_address = std::string("");
	std::string listen_interface = std::string("");
    std::string program_name = std::string("");
    Config(std::string listen_address, uint16_t listen_port, std::string listen_interface) : 
    listen_address(listen_address), 
    listen_interface(listen_interface), 
    listen_port(listen_port) {};
    std::ostream &operator<<(std::ostream &stream) {
        stream << "{ address= " << listen_address << ", port: " << listen_port << ", interface: " << listen_interface << " }";
        return stream;
    };
    Config() {};
};
