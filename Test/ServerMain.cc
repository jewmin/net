#include "BenchServer.h"
#ifdef USE_VLD
#include "vld.h"
#endif

bool get_parameters(int argc, const char * * argv, std::string & host, i32 & port, bool & log_detail) {
	log_detail = false;

	for (int i = 0; i < argc; ++i) {
		bool remove_flag = false;
		const std::string arg_string = argv[i];
		if (argc < 3 || arg_string == "-h" || arg_string == "--help" || arg_string == "/?") {
			std::printf("Usage: BenchServer host port [--log]\n");
			std::printf("--log          : 是否输出更多日志，默认 否\n");
			return false;
		} else if (arg_string == "--log") {
			log_detail = true;
			remove_flag = true;
		} else if (i == 1) {
			host = arg_string;
		} else if (i == 2) {
			port = std::atoi(argv[i]);
		}
		if (remove_flag) {
			for (int j = i; j != argc; j++) {
				argv[j] = argv[j + 1];
			}
			argc--;
			i--;
		}
	}
	return true;
}

int main(int argc, const char * * argv) {
	std::string host;
	i32 port;
	bool log_detail;
	if (get_parameters(argc, argv, host, port, log_detail)) {
		BenchServer server(log_detail);
		server.Run(host, port);
		server.ShowStatus();
	}
	return 0;
}