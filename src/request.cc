#include <request.hpp>
#include <glog/logging.h>

namespace Request {
    void Request::parse(std::span<uint8_t> data) {
        if (data.size() >= sizeof(CommonHeader)) {
            common_hdr = reinterpret_cast<struct CommonHeader*>(data.subspan(0, sizeof(CommonHeader)).data());
            // util::hexdump(data.data(), data.size());
            switch (CmdType(common_hdr->cmd_type))
            {
            case InsertCap:
                LOG(INFO) << "Received  Insert Cap Request for cap id " << std::endl;

                insert_cap_hdr = reinterpret_cast<::Request::InsertCapHeader*>(data.subspan(sizeof(CommonHeader), 
                                                                            sizeof(::Request::InsertCapHeader)).data());
                
                break;
            case CapInvalid:
                util::hexdump(data.data(), data.size());
                cap_invalid_hdr = reinterpret_cast<::Request::CapInvalidHeader*>(data.subspan(sizeof(CommonHeader), 
                                                                            sizeof(::Request::CapInvalidHeader)).data());
                break;

            case CapRevoke:
                LOG(INFO) << "Received  Revoke Cap Request" << std::endl;
                revoke_cap_hdr = reinterpret_cast<::Request::RevokeCapHeader*>(data.subspan(sizeof(CommonHeader), 
                                                                            sizeof(::Request::RevokeCapHeader)).data());
                break;

            case RequestInvoke:
                LOG(INFO) << "Received  request invoke for cap id " << std::endl;
                request_invoke_hdr = reinterpret_cast<::Request::RequestInvokeHeader*>(data.subspan(sizeof(CommonHeader), 
                                                                            sizeof(::Request::RequestInvokeHeader)).data());
                break;

            case ControllerResetSwitch:
                controller_command = RESET_SWITCH;
                break;
            case ControllerStop:
                controller_command = STOP;
                break;
            case ControllerStartTimer:
                controller_command = START_TIMER;
                break;
            case ControllerStopTimer:
                controller_command = STOP_TIMER_AND_PRINT;
                break;
            default:
                LOG(INFO) << "Runtime Error: Unknown CMDType " + common_hdr->cmd_type;
                break;
            }
        }
    }
}