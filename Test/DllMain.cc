#include "Net.h"
#ifdef USE_VLD
#include "vld.h"
#endif

static i64 clientId = 0;
static i64 connId = 0;
static bool quit = false;

void MyOnSignalFunc(int signum) {
	std::printf("收到关机信号 %d\n", signum);
	quit = true;
}

void MyOnConnectedFunc(i64 mgr_id, i64 connection_id) {
	address_t address = jc_get_one_connection_remote_address(mgr_id, connection_id);
	std::printf("连接成功 %lld %lld %s:%d\n", mgr_id, connection_id, address.ip, address.port);
	jc_send_data(mgr_id, connection_id, "hello", sizeof("hello"));
}

void MyOnConnectFailedFunc(i64 mgr_id, i64 connection_id, int reason) {
	std::printf("连接失败 %lld %lld %d\n", mgr_id, connection_id, reason);
	clientId = mgr_id;
	connId = jc_client_connect(clientId, "::1", 6789);
}

void MyOnDisconnectedFunc(i64 mgr_id, i64 connection_id, bool isRemote) {
	std::printf("断开连接 %lld %lld %d\n", mgr_id, connection_id, isRemote);
}

void MyOnRecvMsgFunc(i64 mgr_id, i64 connection_id, const char * data, int size) {
	std::printf("收到数据 %lld %lld, %s, %d\n", mgr_id, connection_id, data, size);
	jc_shutdown_one_connection(mgr_id, connection_id);
}

void MyOnSendMsgFunc(i64 mgr_id, i64 connection_id) {
	std::printf("发送数据 %lld %lld\n", mgr_id, connection_id);
}

int main(int argc, const char * * argv) {
	jc_init();
	jc_replace_signal(MyOnSignalFunc);
	jc_replace_callback(MyOnConnectedFunc, MyOnConnectFailedFunc, MyOnDisconnectedFunc, MyOnRecvMsgFunc, MyOnSendMsgFunc);
	i64 server = jc_create_server("DllServer", 102400, 102400);
	jc_server_listen(server, "::", 6789);
	i64 client1 = jc_create_client("DllClientIPv4", 102400, 102400);
	i64 client2 = jc_create_client("DllClientIPv6", 102400, 102400);
	jc_client_connect(client1, "127.0.0.1", 6789);
	jc_client_connect(client2, "::1", 8888);
	while (!quit) {
		jc_poll();
	}
	jc_end_server(server);
	jc_shutdown_all_connections(server);
	jc_delete_server(server);
	jc_delete_client(client1);
	jc_unit();
	return 0;
}