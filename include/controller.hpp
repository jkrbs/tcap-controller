#pragma once
#include <memory>
#include <thread>

#include <transport_UDP.hpp>
#include <config.hpp>
#include <request.hpp>

#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_table_data.hpp>
#include <bf_rt/bf_rt_table.hpp>
#include <bf_rt/bf_rt_table_operations.hpp>
#include <bf_rt/bf_rt_info.hpp>
#include <bf_rt/bf_rt_session.hpp>

extern "C" {
	#include <bf_switchd/bf_switchd.h>
}

struct CapTableFieldIds {
  bf_rt_id_t cap_id;
  bf_rt_id_t src_addr;
  bf_rt_id_t dstAddr;
  bf_rt_id_t port;
};

class Controller {
    private:
    std::shared_ptr<UDPTransport> socket;
    bf_switchd_context_t *switch_context = nullptr;
    std::shared_ptr<bfrt::BfRtSession> session = nullptr;
    bf_rt_target_t *device = nullptr;
    const bfrt::BfRtInfo *bfrtInfo = nullptr;
    const bfrt::BfRtTable* cap_table = nullptr;
    std::thread cpu_mirror_listener;
    struct CapTableFieldIds cap_table_fields = { 0 };
    void cap_insert(struct Request::InsertCapHeader* hdr);
    public:
    Controller(bf_switchd_context_t *switchd_ctx);
    Controller(bf_switchd_context_t *switchd_ctx, std::shared_ptr<bfrt::BfRtSession> session,
    bf_rt_target_t *device, const bfrt::BfRtInfo *info, std::shared_ptr<Config> cfg);
    void run();
};