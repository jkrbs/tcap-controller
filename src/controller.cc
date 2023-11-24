#include <thread>
#include <controller.hpp>
#include <config.hpp>
#include "transport_UDP.hpp"
#include <span>
#include <vector>
#include <request.hpp>

#include <glog/logging.h>

Controller::Controller(bf_switchd_context_t *switchd_ctx, std::shared_ptr<Config> cfg) {
    this->switch_context = switchd_ctx;
    this->socket = std::shared_ptr<UDPTransport>(new UDPTransport(cfg));
}

void Controller::run() {
    auto cpu_mirror_listener = std::thread([this]{
        auto recv_buffer = std::array<uint8_t, 1024*1024>();

        while(1) {
            size_t len = this->socket->recv(std::span(recv_buffer));

            Request::Request req;
            req.parse(std::span(recv_buffer).subspan(0, len));
            
            switch(req.common_hdr->cmd_type) {
                case Request::CmdType::InsertCap:
                    LOG(INFO) << "Performing Table Insert for Capability";
                    break;
                default:
                    LOG(FATAL) << "Invalid cmd_type in packet handling. This should have been caught earlier.";
            }
        }
    });
    while(true) {}
}