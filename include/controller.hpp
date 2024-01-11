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
  bf_rt_id_t action_revoked;
  bf_rt_id_t action_forward;
  bf_rt_id_t action_drop;
};

struct ARPTableFieldIds {
  bf_rt_id_t ipaddr;
  bf_rt_id_t arp_reply;
};

struct RoutingTableFieldIds {
  bf_rt_id_t forward;
  bf_rt_id_t ipaddr;
};

class Controller {
    private:
    std::shared_ptr<UDPTransport> socket;
    bf_switchd_context_t *switch_context = nullptr;
    std::shared_ptr<bfrt::BfRtSession> session = nullptr;
    bf_rt_target_t *device = nullptr;
    const bfrt::BfRtInfo *bfrtInfo = nullptr;
    const bfrt::BfRtTable* cap_table = nullptr;
    const bfrt::BfRtTable* arp_table = nullptr;
    const bfrt::BfRtTable* routing_table = nullptr;
    std::thread cpu_mirror_listener;
    struct CapTableFieldIds cap_table_fields = { 0 };
    struct ARPTableFieldIds arp_table_fields = { 0 };
    struct RoutingTableFieldIds routing_table_fields = { 0 };
    void cap_insert(struct Request::InsertCapHeader* hdr);
    
    bf_status_t configure_mirroring(uint16_t session_id_val, uint64_t eport);
    bf_status_t configure_mirror_port(uint16_t session_id_val, uint64_t iport, uint64_t eport);
    
    bf_status_t enable_device_port(uint64_t front_port, bf_port_speed_t speed_d, bf_fec_type_t fec_d);
    bf_status_t setup_arp(std::shared_ptr<std::vector<PortConfig>> ports);
    bf_status_t setup_routing_table(std::shared_ptr<std::vector<PortConfig>> ports);
    
    public:
    Controller(bf_switchd_context_t *switchd_ctx);
    Controller(bf_switchd_context_t *switchd_ctx, std::shared_ptr<bfrt::BfRtSession> session,
    bf_rt_target_t *device, const bfrt::BfRtInfo *info, std::shared_ptr<Config> cfg);
    void run();
};