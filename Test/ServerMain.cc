#include "BenchServer.h"
#ifdef USE_VLD
#include "vld.h"
#endif

bool get_parameters(int argc, const char * * argv, bool & log_detail) {
	log_detail = false;

	for (int i = 1; i < argc; ++i) {
		bool remove_flag = false;
		const std::string arg_string = argv[i];
		if (arg_string == "-h" || arg_string == "--help" || arg_string == "/?") {
			std::printf("Usage: BenchServer [--log]\n");
			std::printf("--log          : 是否输出更多日志，默认 否\n");
			return false;
		} else if (arg_string == "--log") {
			log_detail = true;
			remove_flag = true;
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
	bool log_detail;
	if (get_parameters(argc, argv, log_detail)) {
		BenchServer server(log_detail);
		server.Run("0.0.0.0", 8888);
		server.ShowStatus();
	}
	return 0;
}