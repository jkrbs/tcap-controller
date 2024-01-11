#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <sstream>

class Capability {
public:
    uint64_t cap_id;
    uint64_t port_number = 0;
    uint8_t dstAddr[6];
    uint8_t srcAddr[6];
    uint8_t src_ip[4];
};

class PortConfig {
    public:
    uint64_t port_number = 0;
    uint8_t switch_mac_address[6];
    uint8_t client_mac_address[6];
    uint8_t client_ip_address[4];

    //TODO fixme
    std::string pprint() {
        auto s = std::stringstream();
        s << "Port ( " << port_number
            <<" ip: " << client_ip_address[0] << "." << client_ip_address[1] << "." << client_ip_address[2] << "." << client_ip_address[3] 
            << ")"; 
        auto str = std::string();
        s >> str;
        return str;
    };
};

class Config {
	public:
	uint16_t listen_port = 0;
	std::string listen_address = std::string("");
	std::string listen_interface = std::string("");
    std::string program_name = std::string("");
    std::shared_ptr<std::vector<PortConfig>> port_configs = std::shared_ptr<std::vector<PortConfig>>(new std::vector<PortConfig>());
    std::shared_ptr<std::vector<Capability>> capabilities = std::shared_ptr<std::vector<Capability>>(new std::vector<Capability>());

    Config(std::string listen_address, uint16_t listen_port, std::string listen_interface) : 
    listen_address(listen_address), 
    listen_interface(listen_interface), 
    listen_port(listen_port) {};
    std::ostream &operator<<(std::ostream &stream) {
        stream << "{ address= " << listen_address << ", port: " << listen_port << ", interface: " << listen_interface << " }";
        return stream;
    };
    Config() {};
    void add_port_config(std::string path);
    PortConfig GetPortConfigByDestMacAddr(uint8_t* dest_addr);
    PortConfig GetPortConfigByIPAddr(uint8_t* ip_addr);
};
