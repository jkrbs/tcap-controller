#pragma once
#include <cstdint>
#include <memory>

class Config {
	public:
	uint16_t listen_port = 0;
	std::string listen_address = std::string("");
	std::string listen_interface = std::string("");
    Config(std::string listen_address, uint16_t listen_port, std::string listen_interface) : 
    listen_address(listen_address), 
    listen_interface(listen_interface), 
    listen_port(listen_port) {};

    Config() {};
};
