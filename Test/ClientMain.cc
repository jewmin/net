#include "BenchClient.h"
#ifdef USE_VLD
#include "vld.h"
#endif

int main(int argc, const char * * argv) {
	int client_count = 1;
	int packet_count = 1;
	int packet_size = 64;
	for (int i = 1; i < argc; ++i) {
		bool remove_flag = false;
		const std::string arg_string = argv[i];
		if (arg_string == "-h" || arg_string == "--help" || arg_string == "/?") {
			printf("Usage: BenchClient [-c client_count] [-p packet_count] [-s packet_size]\n");
			printf("-c client_count: 同时运行多少个客户端, 默认为1\n");
			printf("-p packet_count: 每个客户端发多少个包, 默认为1\n");
			printf("-s packet_size : 每个包包含多少个字节, 默认为64\n");
			exit(0);
		} else if (arg_string == "-c") {
			client_count = std::atoi(argv[i + 1]);
			remove_flag = true;
		} else if (arg_string == "-p") {
			packet_count = std::atoi(argv[i + 1]);
			remove_flag = true;
		} else if (arg_string == "-s") {
			packet_size = std::atoi(argv[i + 1]);
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
	BenchClient client(client_count, packet_count, packet_size);
	client.Run("127.0.0.1", 8888);
	client.ShowStatus();
	return 0;
}