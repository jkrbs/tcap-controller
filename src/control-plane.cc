#include <glog/logging.h>

#include <bf_rt/bf_rt_info.hpp>
#include <bf_rt/bf_rt_init.hpp>
#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_table_data.hpp>
#include <bf_rt/bf_rt_table.hpp>
#include <bf_rt/bf_rt_table_operations.hpp>

#include <getopt.h>

#include <iostream>

#include "config.hpp"
#include <controller.hpp>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"

extern "C" {
	#include <bf_switchd/bf_switchd.h>
}

static void parse_options(bf_switchd_context_t *switchd_ctx, std::shared_ptr<Config> config, int argc, char **argv) {
	int option_index = 0;

	char* sde_path = std::getenv("SDE_INSTALL");
	printf("The SDE path is: %s \n",sde_path);
	if (sde_path == nullptr) {
		printf("$SDE variable is not set\n");
		//printf(sde_path);
		exit(0);
	}

	static struct option options[] = {
		{"help", no_argument, 0, 'h'},
		{"switch-config", required_argument, 0, 's'},
		{"config", required_argument, 0, 'c'},
		{"interface", required_argument, 0, 'i'},
		{"address", required_argument, 0, 'a'},
		{"port", required_argument, 0, 'p'}
	};

	while (1) {
		int c = getopt_long(argc, argv, "hciapd:", options, &option_index);

		if (c == -1) {
			break;
		}
		
		switch (c) {
			case 's':
				char conf_path[256];
				snprintf(conf_path, 256, "%s/share/p4/targets/tofino/%s.conf", sde_path, optarg);
				config->program_name.assign(optarg);
				switchd_ctx->conf_file = strdup(conf_path);
				printf("Conf-file : %s\n", switchd_ctx->conf_file);
			break;
			case 'i':
				config->listen_interface.assign(optarg);
			break;
			case 'a':
				config->listen_address.assign(optarg);
			break;
			case 'p':
				config->listen_port = atoi(optarg);
			break;
			case 'c':
				config->add_port_config(optarg);
			break;
			case 'h':
			case '?':
				printf("run_controlplane_pie \n");
				printf("Usage : run_controlplane_pie -p <name of the program>\n");
				std::cout << 
				"help message :"
				"TODO"
				 << std::endl;
				exit(c == 'h' ? 0 : 1);
			break;
			default:
				printf("Invalid option\n");
				exit(0);
			break;
		}

	}

	if (switchd_ctx->conf_file == NULL) {
		printf("ERROR : -p must be specified\n");
		exit(0);
	}

	switchd_ctx->install_dir = strdup(sde_path);
	printf("Install Dir: %s\n", switchd_ctx->install_dir);
}

int main(int argc, char* argv[]) {
	google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

	auto session = bfrt::BfRtSession::sessionCreate();
    if(session == nullptr)
        printf("fail to open session");
    
    bf_switchd_context_t *switchd_ctx;
	if ((switchd_ctx = (bf_switchd_context_t *)calloc( 1, sizeof(bf_switchd_context_t))) == NULL) {
		printf("Cannot Allocate switchd context\n");
		exit(1);
	}

	auto config = std::shared_ptr<Config>(new Config());
	
	parse_options(switchd_ctx, config, argc, argv);
	switchd_ctx->running_in_background = true;

	
	auto &devMgr = bfrt::BfRtDevMgr::getInstance();
	const bfrt::BfRtInfo *bfrtInfo = nullptr;

	bf_rt_target_t device_target;
	device_target.dev_id = 0;
	device_target.pipe_id = BF_DEV_PIPE_ALL;

	bf_status_t status = bf_switchd_lib_init(switchd_ctx);

	printf("Status: %s\n", bf_err_str(status));

	status = devMgr.bfRtInfoGet(device_target.dev_id, config->program_name.c_str(), &bfrtInfo);
	printf("Status: %s\n", bf_err_str(status));
	assert(status == BF_SUCCESS);
    auto c = Controller(switchd_ctx, session, &device_target, bfrtInfo, config);
	c.run();

	switchd_ctx->dev_sts_thread = true;
	switchd_ctx->dev_sts_port = 7777;

	while(1) {}

    return EXIT_SUCCESS;
}