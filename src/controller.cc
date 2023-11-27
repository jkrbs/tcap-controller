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
    bf_status = this->cap_table->dataFieldIdGet("dstAddr", &this->cap_table_fields.dstAddr);
    assert(bf_status==BF_SUCCESS);
    bf_status = this->cap_table->keyFieldIdGet("port", &this->cap_table_fields.port);
    assert(bf_status==BF_SUCCESS);
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

	std::unique_ptr<bfrt::BfRtTableKey> key_cap_id;
    std::unique_ptr<bfrt::BfRtTableKey> key_src_addr;

	auto bf_status = this->cap_table->keyAllocate(&key_cap_id);
	assert(bf_status == BF_SUCCESS);
    bf_status = this->cap_table->keyAllocate(&key_src_addr);
	assert(bf_status == BF_SUCCESS);

	bf_status = key_cap_id->setValue(this->cap_table_fields.cap_id, hdr->cap_id);
	assert(bf_status == BF_SUCCESS);
	bf_status = key_src_addr->setValue(this->cap_table_fields.src_addr, hdr->object_owner, 10);
	assert(bf_status == BF_SUCCESS);


    auto keys = std::vector<std::unique_ptr<bfrt::BfRtTableKey>>();
    keys.push_back(std::move(key_cap_id));
    keys.push_back(std::move(key_src_addr));


    std::unique_ptr<bfrt::BfRtTableData> data_dest_addr;
    std::unique_ptr<bfrt::BfRtTableData> data_port;

	bf_status = this->cap_table->dataAllocate(&data_dest_addr);
	assert(bf_status == BF_SUCCESS);

	bf_status = this->cap_table->dataAllocate(&data_port);
	assert(bf_status == BF_SUCCESS);


	bf_status = data_dest_addr->setValue(this->cap_table_fields.dstAddr, hdr->cap_owner_ip, 10);
	assert(bf_status == BF_SUCCESS);

	bf_status = data_port->setValue(this->cap_table_fields.port, (uint64_t)0);
	assert(bf_status == BF_SUCCESS);

    auto data = std::vector<std::unique_ptr<bfrt::BfRtTableData>>();
    data.push_back(std::move(data_dest_addr));
    data.push_back(std::move(data_port));


    std::unique_ptr<bfrt::BfRtTableKey> k;
    struct ss {uint64_t c; uint8_t o[10];};
    struct ss s {.c = hdr->cap_id};
    memcpy(s.o, hdr->cap_owner_ip, 10);
    bf_status = k->setValue(1, (uint8_t*)&s, sizeof(struct ss));
	assert(bf_status == BF_SUCCESS);
    
    std::unique_ptr<bfrt::BfRtTableData> d;
    bf_status = d->setValue(1, data);
	assert(bf_status == BF_SUCCESS);

    

	bf_status = this->cap_table->tableEntryAdd(*this->session, *this->device, 0,
									*k.get(), *d.get());
	assert(bf_status == BF_SUCCESS);
}