#include <thread>
#include <controller.hpp>
#include <config.hpp>
#include "transport_UDP.hpp"
#include <span>
#include <vector>
#include <request.hpp>

#include <glog/logging.h>
#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_table_data.hpp>

Controller::Controller(bf_switchd_context_t *switchd_ctx, std::shared_ptr<bfrt::BfRtSession> session,
    bf_rt_target_t *device, const bfrt::BfRtInfo *info, std::shared_ptr<Config> cfg) {
    this->session = session;
    this->device = device;
    this->switch_context = switchd_ctx;
    this->bfrtInfo = info;
    this->socket = std::shared_ptr<UDPTransport>(new UDPTransport(cfg));

    auto bf_status = this->bfrtInfo->bfrtTableFromNameGet("Ingress.cap_table", &this->cap_table);
  	assert(bf_status == BF_SUCCESS);
    
    bf_status = this->cap_table->keyFieldIdGet("cap_id", &this->cap_table_fields.cap_id);
    assert(bf_status==BF_SUCCESS);
    bf_status = this->cap_table->keyFieldIdGet("src_addr", &this->cap_table_fields.src_addr);
    assert(bf_status==BF_SUCCESS);

    bf_status = this->cap_table->actionIdGet("Ingress.capRevoked", &this->cap_table_fields.action_revoked);
    assert(bf_status==BF_SUCCESS);

    bf_status = this->cap_table->actionIdGet("Ingress.capAllow_forward", &this->cap_table_fields.action_forward);
    assert(bf_status==BF_SUCCESS);

    bf_status = this->cap_table->actionIdGet("Ingress.drop", &this->cap_table_fields.action_drop);
    assert(bf_status==BF_SUCCESS);

    LOG(INFO) << "Ids: \n"
                << "cap_id: " << this->cap_table_fields.cap_id 
                << "\nsrc_addr: " << this->cap_table_fields.src_addr 
                << "\nIngress.capRevoked: " << this->cap_table_fields.action_revoked 
                << "\nIngress.capAllow_forward: " << this->cap_table_fields.action_forward  
                << "\nIngress.drop: " << this->cap_table_fields.action_drop;

    std::vector<bf_rt_id_t> vec = std::vector<bf_rt_id_t>();
    bf_status = this->cap_table->dataFieldIdListGet(this->cap_table_fields.action_forward, &vec);
    assert(bf_status == BF_SUCCESS);
    for (auto &i: vec) {
        
        std::string name = std::string("");
       // bf_status = this->cap_table->dataFieldNameGet(i, &name);
       // assert(bf_status == BF_SUCCESS);

        size_t size = 0;
        //bf_status = this->cap_table->dataFieldSizeGet(i, &size);
        //assert(bf_status == BF_SUCCESS);
//
        LOG(INFO) << "Id: " << i << " name: " << name << " size: " << size;
    }

    // configure front ports
    bf_status = this->enable_device_port(1, BF_SPEED_100G, BF_FEC_TYP_RS);
    assert(bf_status == BF_SUCCESS);
    bf_status = this->enable_device_port(32, BF_SPEED_100G, BF_FEC_TYP_RS);
    assert(bf_status == BF_SUCCESS);

    // start CPU Port mirror session
    bf_status = this->configure_mirroring(128, 64);
    assert(bf_status == BF_SUCCESS);
    bf_status = this->configure_mirror_port(128, 1, 64);
    assert(bf_status == BF_SUCCESS);
    bf_status = this->configure_mirror_port(128, 32, 64);
    assert(bf_status == BF_SUCCESS);
}

void Controller::run() {
    this->cpu_mirror_listener = std::thread([this]{
        auto recv_buffer = std::array<uint8_t, 1024*1024>();

        while(1) {
            size_t len = this->socket->recv(std::span(recv_buffer));

            Request::Request req;
            req.parse(std::span(recv_buffer).subspan(0, len));
            
            switch(req.common_hdr->cmd_type) {
                case Request::CmdType::InsertCap:
                    LOG(INFO) << "Performing Table Insert for Capability";
                    this->cap_insert(req.insert_cap_hdr);
                    break;
                default:
                    LOG(INFO) << "Invalid cmd_type in packet handling. This should have been caught earlier.";
            }
        }
    });
    this->cpu_mirror_listener.detach();
}


// https://github.com/COMSYS/pie-for-tofino/blob/main/bfrt_cpp/src/run_controlplane_pie.cpp
void Controller::cap_insert(struct Request::InsertCapHeader *hdr) {

	auto flag = bfrt::BfRtTable::BfRtTableGetFlag::GET_FROM_HW;

	std::unique_ptr<bfrt::BfRtTableKey> key;

    auto bf_status = this->cap_table->keyAllocate(&key);
	assert(bf_status == BF_SUCCESS);

	bf_status = key->setValue(this->cap_table_fields.cap_id, hdr->cap_id);
	assert(bf_status == BF_SUCCESS);
    key->setValue(this->cap_table_fields.src_addr, hdr->cap_owner_ip, 4);

    std::unique_ptr<bfrt::BfRtTableData> d;
    bf_status = this->cap_table->dataAllocate(this->cap_table_fields.action_forward, &d);
    assert(bf_status == BF_SUCCESS);

    uint8_t destmac[6] = {0xff};
    bf_status = d->setValue(1, destmac, 6);
	assert(bf_status == BF_SUCCESS);

    //port
    bf_status = d->setValue(2, (uint64_t)9);
	assert(bf_status == BF_SUCCESS);

    this->session->beginTransaction(false);
	bf_status = this->cap_table->tableEntryAdd(*this->session, *this->device, 0,
									*key.get(), *d.get());
	assert(bf_status == BF_SUCCESS);
    this->session->commitTransaction(true);
}

bf_status_t Controller::configure_mirroring(uint16_t session_id_val, uint64_t eport) {
    const bfrt::BfRtTable* mirror_cfg = nullptr;

    bf_status_t bf_status = this->bfrtInfo->bfrtTableFromNameGet("$mirror.cfg", &mirror_cfg);
    assert(bf_status == BF_SUCCESS);

    bf_rt_id_t session_id, normal;
    bf_status = mirror_cfg->keyFieldIdGet("$sid", &session_id);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg->actionIdGet("$normal", &normal);
    assert(bf_status == BF_SUCCESS);

    std::unique_ptr<bfrt::BfRtTableKey> mirror_cfg_key;
    std::unique_ptr<bfrt::BfRtTableData> mirror_cfg_data;

    bf_status = mirror_cfg->keyAllocate(&mirror_cfg_key);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg->dataAllocate(normal, &mirror_cfg_data);
    assert(bf_status == BF_SUCCESS);

    bf_status = mirror_cfg_key->setValue(session_id, session_id_val);
    assert(bf_status == BF_SUCCESS);

//    bf_status = mirror_cfg_data->setValue(1, true);
//    assert(bf_status == BF_SUCCESS);
    bf_rt_id_t session_enabled, direction, ucast_egress_port, ucast_egress_port_valid, copy_to_cpu;
    bf_status = mirror_cfg->dataFieldIdGet("$session_enable", &session_enabled);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg->dataFieldIdGet("$direction", &direction);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg->dataFieldIdGet("$ucast_egress_port", &ucast_egress_port);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg->dataFieldIdGet("$ucast_egress_port_valid", &ucast_egress_port_valid);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg->dataFieldIdGet("$copy_to_cpu", &copy_to_cpu);
    assert(bf_status == BF_SUCCESS);

    bf_status = mirror_cfg_data->setValue(session_enabled, true);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg_data->setValue(direction, std::string("BOTH"));
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg_data->setValue(ucast_egress_port, eport);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg_data->setValue(ucast_egress_port_valid, true);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_cfg_data->setValue(copy_to_cpu, true);
    assert(bf_status == BF_SUCCESS);

	bf_status = mirror_cfg->tableEntryAdd(*this->session, *this->device, 0,
									*mirror_cfg_key.get(), *mirror_cfg_data.get());
    LOG(INFO) << "table entry add result:" << bf_err_str(bf_status);
	assert(bf_status == BF_SUCCESS);

    return BF_SUCCESS;
}

bf_status_t Controller::configure_mirror_port(uint16_t session_id_val, uint64_t iport, uint64_t eport) {
    const bfrt::BfRtTable* mirror_fwd = nullptr;

    auto bf_status = this->bfrtInfo->bfrtTableFromNameGet("mirror_fwd", &mirror_fwd);
    assert(bf_status == BF_SUCCESS);

    
    std::unique_ptr<bfrt::BfRtTableKey> mirror_fwd_key;
    std::unique_ptr<bfrt::BfRtTableData> mirror_fwd_data;

    bf_rt_id_t ingress_port, set_md;
    bf_status = mirror_fwd->keyFieldIdGet("ig_intr_md.ingress_port", &ingress_port);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_fwd->actionIdGet("Ingress.set_md", &set_md);
    assert(bf_status == BF_SUCCESS);

    bf_status = mirror_fwd->keyAllocate(&mirror_fwd_key);
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_fwd->dataAllocate(set_md, &mirror_fwd_data);
    assert(bf_status == BF_SUCCESS);

    bf_status = mirror_fwd_key->setValue(ingress_port, iport);
    assert(bf_status == BF_SUCCESS);

    // bf_rt_id_t egress_port, ingress_mir, ingress_sid, egress_mir, egress_sid;
    // bf_status = mirror_fwd->dataFieldIdGet("dest_port", &egress_port);
    // LOG(INFO) << "dest_port_get result:" << bf_err_str(bf_status);
    // assert(bf_status == BF_SUCCESS);
    // bf_status = mirror_fwd->dataFieldIdGet("ing_mir", &ingress_mir);
    // assert(bf_status == BF_SUCCESS);
    // bf_status = mirror_fwd->dataFieldIdGet("ing_ses", &ingress_sid);
    // assert(bf_status == BF_SUCCESS);
    // bf_status = mirror_fwd->dataFieldIdGet("egr_mir", &egress_mir);
    // assert(bf_status == BF_SUCCESS);
    // bf_status = mirror_fwd->dataFieldIdGet("egr_ses", &egress_sid);
    // assert(bf_status == BF_SUCCESS);


    bf_status = mirror_fwd_data->setValue(1, eport); // egress_port
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_fwd_data->setValue(2, (uint64_t)1); // ingress_mir
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_fwd_data->setValue(3, (uint64_t)session_id_val); // ingress_sid
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_fwd_data->setValue(4, (uint64_t)1); // egress mir
    assert(bf_status == BF_SUCCESS);
    bf_status = mirror_fwd_data->setValue(5, (uint64_t)session_id_val); // egress sid
    assert(bf_status == BF_SUCCESS);

    bf_status = mirror_fwd->tableEntryAdd(*this->session, *this->device, 0,
									*mirror_fwd_key.get(), *mirror_fwd_data.get());
	    LOG(INFO) << "table entry add result:" << bf_err_str(bf_status);
	assert(bf_status == BF_SUCCESS);
    return BF_SUCCESS;
}

bf_status_t Controller::enable_device_port(uint64_t front_port, bf_port_speed_t speed_d, bf_fec_type_t fec_d) {
    LOG(INFO) << "Enabling Device Ports";

    const bfrt::BfRtTable* port_table = nullptr;

    auto bf_status = this->bfrtInfo->bfrtTableFromNameGet("$PORT", &port_table);
    assert(bf_status == BF_SUCCESS);

    bf_rt_id_t dev_port, port_enable, speed, fec;
    bf_status = port_table->keyFieldIdGet("$DEV_PORT", &dev_port);
    assert(bf_status == BF_SUCCESS);
    bf_status = port_table->dataFieldIdGet("$PORT_ENABLE", &port_enable);
    assert(bf_status == BF_SUCCESS);

    bf_status = port_table->dataFieldIdGet("$SPEED", &speed);
    assert(bf_status == BF_SUCCESS);

    bf_status = port_table->dataFieldIdGet("$FEC", &fec);
    assert(bf_status == BF_SUCCESS);

    std::unique_ptr<bfrt::BfRtTableKey> port_table_key;
    std::unique_ptr<bfrt::BfRtTableData> port_table_data;


    bf_status = port_table->keyAllocate(&port_table_key);
    assert(bf_status == BF_SUCCESS);
    bf_status = port_table->dataAllocate(&port_table_data);
    assert(bf_status == BF_SUCCESS);

    bf_status = port_table_key->setValue(dev_port, front_port);
    assert(bf_status == BF_SUCCESS);

    bf_status = port_table_data->setValue(port_enable, true);
    assert(bf_status == BF_SUCCESS);
    bf_status = port_table_data->setValue(speed, (uint64_t)speed_d);
    assert(bf_status == BF_SUCCESS);
    bf_status = port_table_data->setValue(fec, (uint64_t)fec_d);
    assert(bf_status == BF_SUCCESS);

   
     bf_status = port_table->tableEntryAdd(*this->session, *this->device, 0,
									*port_table_key.get(), *port_table_data.get());
	    LOG(INFO) << "table entry add result:" << bf_err_str(bf_status);
	assert(bf_status == BF_SUCCESS);
  

    return BF_SUCCESS;
}