#include <request.hpp>
#include <glog/logging.h>

namespace Request {
    void Request::parse(std::span<uint8_t> data) {
        if (data.size() >= sizeof(CommonHeader)) {
            common_hdr = reinterpret_cast<struct CommonHeader*>(data.subspan(0, sizeof(CommonHeader)).data());
            util::hexdump(data.data(), data.size());
            switch (common_hdr->cmd_type)
            {
            InsertCap:
                LOG(INFO) << "Received  Insert Cap Request for cap id " << common_hdr->cap_id << std::endl;

                insert_cap_hdr = reinterpret_cast<::Request::InserCapHeader*>(data.subspan(sizeof(CommonHeader), 
                                                                            sizeof(::Request::InsertCapHeader)).data());
                
                break;
            
            CapRevoke:
                LOG(INFO) << "Received  Revoke Cap Request for cap id " << common_hdr->cap_id << std::endl;
                break;

            default:
                LOG(FATAL) << "Runtime Error: Unknown CMDType " + common_hdr->cmd_type;
                break;
            }

        }
    }
}