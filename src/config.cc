#include "config.hpp"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"

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
            for (rapidjson::Value::ConstValueIterator port = doc_itr->value.Begin(); port != doc_itr->value.End(); ++port)
            {
                if (port->HasMember("interface") &&
                    port->HasMember("switch_mac") &&
                    port->HasMember("client_mac") &&
                    port->HasMember("client_ip4a"))
                {

                    PortConfig port_config;
                    port_config.port_number = port->FindMember("interface")->value.GetUint64();
                    if (port->FindMember("switch_mac")->value.GetStringLength() != mac_str_len)
                        return;

                    str2macaddr(port->FindMember("switch_mac")->value.GetString(), port_config.switch_mac_address, 6);
                    str2macaddr(port->FindMember("client_mac")->value.GetString(), port_config.client_mac_address, 6);
                    str2ip4addr(port->FindMember("client_ip4a")->value.GetString(), port_config.client_ip_address, 4);

                    this->port_configs->push_back(port_config);
                }
            }
        }
    }

    delete buf;
}