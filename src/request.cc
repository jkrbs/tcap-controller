#include <request.hpp>
#include <glog/logging.h>

namespace Request {
    void Request::parse(std::span<uint8_t> data) {
        if (data.size() >= sizeof(CommonHeader)) {
            common_hdr = reinterpret_cast<struct CommonHeader*>(data.subspan(0, sizeof(CommonHeader)).data());
            util::hexdump(data.data(), data.size());
            switch (CmdType(common_hdr->cmd_type))
            {
            case InsertCap:
                LOG(INFO) << "Received  Insert Cap Request for cap id " << common_hdr->cap_id << std::endl;

                insert_cap_hdr = reinterpret_cast<::Request::InsertCapHeader*>(data.subspan(sizeof(CommonHeader), 
                                                                            sizeof(::Request::InsertCapHeader)).data());
                
                break;
            
            case CapRevoke:
                LOG(INFO) << "Received  Revoke Cap Request for cap id " << common_hdr->cap_id << std::endl;
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