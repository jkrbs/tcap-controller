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