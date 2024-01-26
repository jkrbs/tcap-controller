#pragma once

#include <span>
#include <cstdint>
#include <iostream>

#include <util.hpp>


namespace Request {
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
        MemoryCopy = 10,
        MemoryCopyResponse = 11,
        RequestCreate = 13,
        RequestInvoke = 14,
        /* Gap in OPCode Numbers Caused by Packet Types Unsupported by this implementation */
        RequestReceive = 16,
        RequestResponse = 17,
        /* Gap in OPCode Numbers Caused by Packet Types Unsupported by this implementation */
        None = 32, // None is used as default value

        //P4 Implementation specific OP Codes
        InsertCap = 64,

        ControllerResetSwitch = 128,
        ControllerStop = 129,
        ControllerStartTimer = 130,
        ControllerStopTimer = 131
    };

    enum CapType {

    };

    struct CommonHeader {
        uint64_t size;
        uint32_t stream_id;
        uint32_t cmd_type;
        uint8_t cap_id[16];
    };

    typedef struct util::IpAddress IpAddress;

    struct InsertCapHeader {
        uint8_t cap_owner_ip[4];
        uint8_t cap_id[16];
        uint8_t cap_type;
        uint8_t object_owner[10];
    };
    
    struct RevokeCapHeader {
        uint8_t cap_owner_ip[4];
        uint8_t cap_id[16];
    };

    enum ControllerCMD {
        NONE = 0,
        RESET_SWITCH = 1,
        STOP = 2,
        START_TIMER = 3,
        STOP_TIMER_AND_PRINT = 4
    };

    struct RequestInvokeHeader {
        uint8_t num_of_caps;
        uint8_t cap_parameters[16][4];
    };

    struct CapInvalidHeader {
        uint8_t address[4];
        uint8_t cap_id[16];
    };

class Request {
    public:
    ControllerCMD controller_command = NONE;
    struct CommonHeader* common_hdr = nullptr;
    struct InsertCapHeader* insert_cap_hdr = nullptr;
    struct RevokeCapHeader* revoke_cap_hdr = nullptr;
    struct RequestInvokeHeader* request_invoke_hdr = nullptr;
    struct CapInvalidHeader* cap_invalid_hdr = nullptr;
    Request() {};

    void parse(std::span<uint8_t> data);
};
}