#include <thread>
#include <controller.hpp>
#include <config.hpp>
#include "transport_UDP.hpp"

Controller::Controller(bf_switchd_context_t *switchd_ctx, std::shared_ptr<Config> cfg) {
    this->socket = std::shared_ptr<UDPTransport>(new UDPTransport());
}

void Controller::run() {
    auto cpu_mirror_listener = std::thread({
        
    });
    cpu_mirror_listener.detach();
}