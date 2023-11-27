#pragma once

#include <span>
#include <cstdint>
#include <iostream>

#include <util.hpp>

namespace Request {
    typedef uint64_t CapID;

    enum CmdType {
        Nop = 0,
        CapGetInfo = 1,
        CapIsSame = 2,
        CapDiminish = 3,
        /* Gap in OPCode Numbers Caused by Packet Types Unsupported by this implementation */
        CapClose = 5,
        CapRevoke = 6,
        CapInvalid = 7,
        /* Gap in OPCode Numbers Caused by Packet Types Unsupported by this implementation */
        RequestCreate = 13,
        RequestInvoke = 14,
        /* Gap in OPCode Numbers Caused by Packet Types Unsupported by this implementation */
        RequestReceive = 16,
        RequestResponse = 17,
        /* Gap in OPCode Numbers Caused by Packet Types Unsupported by this implementation */
        None = 32, // None is used as default value

        //P4 Implementation specific OP Codes
        InsertCap = 64,
    };

    enum CapType {

    };

    struct CommonHeader {
        uint64_t size;
        uint32_t stream_id;
        uint32_t cmd_type;
        uint64_t cap_id;
    };

    typedef struct util::IpAddress IpAddress;

    struct InsertCapHeader {
        uint8_t cap_owner_ip[10];
        uint64_t cap_id;
        uint8_t cap_type;
        uint8_t object_owner[10];
    };

class Request {
    public:

    struct CommonHeader* common_hdr = nullptr;
    struct InserCapHeader* insert_cap_hdr = nullptr;
    Request() {};

    void parse(std::span<uint8_t> data);
};
}