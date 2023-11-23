#include <bf_rt/bf_rt_info.hpp>
#include <bf_rt/bf_rt_init.hpp>
#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_table_data.hpp>
#include <bf_rt/bf_rt_table.hpp>
#include <bf_rt/bf_rt_table_operations.hpp>

#include <getopt.h>

extern "C" {
	#include <bf_switchd/bf_switchd.h>
}


static void parse_options(bf_switchd_context_t *switchd_ctx, int argc, char **argv) {
	int option_index = 0;

	char* sde_path = std::getenv("SDE_INSTALL");
	printf("The SDE path is: %s \n",sde_path);
	if (sde_path == nullptr) {
		printf("$SDE variable is not set\n");
		//printf(sde_path);
		exit(0);
	}

	static struct option options[] = {
		{"help", no_argument, 0, 'h'}
		// {"program", required_argument, 0, 'p'}
	};

	while (1) {
		int c = getopt_long(argc, argv, "hp:", options, &option_index);

		if (c == -1) {
			break;
		}
		
		switch (c) {
			case 'p':
				char conf_path[256];
				snprintf(conf_path, 256, "%s/share/p4/targets/tofino/%s.conf", sde_path, optarg);

				switchd_ctx->conf_file = strdup(conf_path);
				printf("Conf-file : %s\n", switchd_ctx->conf_file);
			break;
			case 'h':
			case '?':
				printf("run_controlplane_pie \n");
				printf("Usage : run_controlplane_pie -p <name of the program>\n");
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

	char install_path[256];
	sprintf(install_path, "%s/install", sde_path);

	switchd_ctx->install_dir = strdup(install_path);
	printf("Install Dir: %s\n", switchd_ctx->install_dir);
}

int main(int argc, char* argv[]) {
    auto session = bfrt::BfRtSession::sessionCreate();
    if(session == nullptr)
        printf("fail to open session");
    
    bf_switchd_context_t *switchd_ctx;
	if ((switchd_ctx = (bf_switchd_context_t *)calloc( 1, sizeof(bf_switchd_context_t))) == NULL) {
		printf("Cannot Allocate switchd context\n");
		exit(1);
	}

	parse_options(switchd_ctx, argc, argv);

	switchd_ctx->dev_sts_thread = true;
	switchd_ctx->dev_sts_port = 7777;

	printf("Give status next\n");
	bf_status_t status = bf_switchd_lib_init(switchd_ctx);

	printf("Status: %s\n", bf_err_str(status));

    

    return EXIT_SUCCESS;
}