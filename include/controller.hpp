#pragma once
#include <memory>
#include <transport_UDP.hpp>
#include <config.hpp>

extern "C" {
	#include <bf_switchd/bf_switchd.h>
}

class Controller {
    private:
    std::shared_ptr<UDPTransport> socket;
    bf_switchd_context_t *switch_context;
    public:
    Controller(bf_switchd_context_t *switchd_ctx);
    Controller(bf_switchd_context_t *switchd_ctx, std::shared_ptr<Config> cfg);
    void run();
};