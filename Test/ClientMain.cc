#include "BenchClient.h"
#ifdef USE_VLD
#include "vld.h"
#endif

bool get_parameters(int argc, const char * * argv, std::string & host, i32 & port, i32 & client_count, i32 & packet_count, i32 & packet_size, bool & log_detail) {
	client_count = 1;
	packet_count = 1;
	packet_size = 64;
	log_detail = false;

	for (int i = 0; i < argc; ++i) {
		bool remove_flag = false;
		const std::string arg_string = argv[i];
		if (argc < 3 || arg_string == "-h" || arg_string == "--help" || arg_string == "/?") {
			std::printf("Usage: BenchClient host port [-c client_count] [-p packet_count] [-s packet_size] [--log]\n");
			std::printf("-c client_count: 客户端并发数量，默认 1\n");
			std::printf("-p packet_count: 每个客户端发送数据包数量，默认 1\n");
			std::printf("-s packet_size : 每个数据包大小，默认 64字节\n");
			std::printf("--log          : 是否输出更多日志，默认 否\n");
			return false;
		} else if (arg_string == "-c") {
			client_count = std::atoi(argv[i + 1]);
			remove_flag = true;
		} else if (arg_string == "-p") {
			packet_count = std::atoi(argv[i + 1]);
			remove_flag = true;
		} else if (arg_string == "-s") {
			packet_size = std::atoi(argv[i + 1]);
			remove_flag = true;
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
	i32 client_count;
	i32 packet_count;
	i32 packet_size;
	bool log_detail;
	if (get_parameters(argc, argv, host, port, client_count, packet_count, packet_size, log_detail)) {
		BenchClient client(client_count, packet_count, packet_size, log_detail);
		client.Run(host, port);
		client.ShowStatus();
	}
	return 0;
}