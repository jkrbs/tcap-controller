#include "config.hpp"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"

#include <glog/logging.h>

constexpr size_t mac_str_len = strlen("00:00:00:00:00:00");
void str2macaddr(const char *in, uint8_t *out, size_t out_len)
{

    char str[mac_str_len + 1];
    strncpy(str, in, mac_str_len + 1);

    char *token = std::strtok(str, ":");
    int c = 0;
    while (token)
    {
        out[c] = std::stoul(token, nullptr, 16);
        token = std::strtok(nullptr, ":");
        c++;
        if(c >= out_len) continue;
    }
}

constexpr size_t ipv4a_str_len = strlen("255.255.255.255");
void str2ip4addr(const char *in, uint8_t *out, size_t out_len)
{

    char str[mac_str_len + 1];
    strncpy(str, in, mac_str_len + 1);

    char *token = std::strtok(str, ".");
    int c = 0;
    while (token)
    {
        out[c] = std::atoi(token);
        token = std::strtok(nullptr, ".");
        c++;
        if(c >= out_len) continue;
    }
}

/**
 * {
 *  "ports": [
 *  {
 *      "interface": uint64_t,
 *      "switch_mac": str,
 *      "client_mac": str,
 *      "client_ip4a": "str"
 *  },
 * ...
 * ]
 * }
 */

void Config::add_port_config(std::string path)
{
    char *buf = new char[0xfff];

    rapidjson::FileReadStream input(fopen(path.c_str(), "r"), buf, sizeof(buf));
    rapidjson::Document document;
    document.ParseStream(input);

    for (rapidjson::Value::MemberIterator doc_itr = document.MemberBegin(); doc_itr != document.MemberEnd(); ++doc_itr)
    {
        if (!strcmp(doc_itr->name.GetString(), "ports"))
        {
            LOG(INFO) << "Configuring switch interfaces";
            for (rapidjson::Value::ConstValueIterator port = doc_itr->value.Begin(); port != doc_itr->value.End(); ++port)
            {
                if (port->HasMember("interface") &&
                    port->HasMember("switch_mac") &&
                    port->HasMember("client_mac") &&
                    port->HasMember("client_ip4a") &&
                    port->HasMember("client_udp_port"))
                {

                    PortConfig port_config = {0};
                    port_config.port_number = port->FindMember("interface")->value.GetUint64();
                    if (port->FindMember("switch_mac")->value.GetStringLength() != mac_str_len)
                        return;

                    str2macaddr(port->FindMember("switch_mac")->value.GetString(), port_config.switch_mac_address, 6);
                    str2macaddr(port->FindMember("client_mac")->value.GetString(), port_config.client_mac_address, 6);
                    str2ip4addr(port->FindMember("client_ip4a")->value.GetString(), port_config.client_ip_address, 4);
                    port_config.client_udp_port = static_cast<uint16_t>(port->FindMember("client_udp_port")->value.GetInt64());
                    this->port_configs->push_back(port_config);
                }
            }
        }
        if (!strcmp(doc_itr->name.GetString(), "capabilities")) {
            LOG(INFO) << "Adding predefined capabilities";
            for (rapidjson::Value::ConstValueIterator cap = doc_itr->value.Begin(); cap != doc_itr->value.End(); ++cap)
            {
                if(cap->HasMember("cap_id") &&
                cap->HasMember("src_ip") &&
                cap->HasMember("srcAddr") &&
                cap->HasMember("dstAddr")) {
                    Capability c = {0};

                    // Capabilities are used in network byte order. So we have to swap on x86
                    c.port_number = 0;
                    // c.cap_id = __builtin_bswap64(cap->FindMember("cap_id")->value.GetUint64()) << 64;
                    uint64_t cap_id = cap->FindMember("cap_id")->value.GetUint64();
                    memcpy(c.cap_id, &cap_id, 8);
                    memset(c.cap_id + 8, 0, 8);
                    str2macaddr(cap->FindMember("srcAddr")->value.GetString(), c.srcAddr, 6);
                    str2macaddr(cap->FindMember("dstAddr")->value.GetString(), c.dstAddr, 6);
                    str2ip4addr(cap->FindMember("src_ip")->value.GetString(), c.src_ip, 4);

                    this->capabilities->push_back(c);
                }
            }           
        }
    }

    delete buf;
}

PortConfig Config::GetPortConfigByDestMacAddr(uint8_t* dest_addr) {
    PortConfig port = {0};

    for(PortConfig &p: *this->port_configs) {
        if (0 == memcmp(p.client_mac_address, dest_addr, 6)) {
            memcpy(&port, &p, sizeof(PortConfig));
        }
    }

    return port;
}

PortConfig Config::GetPortConfigByIPAddr(uint8_t* ip_addr) {
    PortConfig port = {0};

    for(PortConfig &p: *this->port_configs) {
        if (0 == memcmp(p.client_ip_address, ip_addr, 4)) {
            memcpy(&port, &p, sizeof(PortConfig));
        }
    }

    return port;
}