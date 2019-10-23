#include "BenchServer.h"
#ifdef USE_VLD
#include "vld.h"
#endif

int main(int argc, const char * * argv) {
	BenchServer server;
	server.Run("0.0.0.0", 8888);
	server.ShowStatus();
	return 0;
}