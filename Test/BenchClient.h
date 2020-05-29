
#ifndef Benchmark_BenchClient_INCLUDED
#define Benchmark_BenchClient_INCLUDED

#include "BenchCommon.h"
#include "Interface/Client.h"

class BenchClient : public BenchCommon {
public:
	BenchClient(i32 client_count, i32 packet_count, i32 packet_size, bool log_detail);
	virtual ~BenchClient();
	virtual void Run(const std::string & address, i32 port) override;
	virtual void ShowStatus() override;

protected:
	virtual void OnConnected(Net::Connection * connection) override;
	virtual void OnConnectFailed(Net::Connection * connection, i32 reason) override;
	virtual void OnDisconnected(Net::Connection * connection, bool is_remote) override;
	virtual void OnSomeDataSent(Net::Connection * connection) override;

private:
	Net::SocketConnector * connector_;
	i32 client_count_;
	i32 packet_count_;
	i32 packet_size_;
	i8 * msg_buffer_;
	std::unordered_map<intptr_t, i32> c2p_mapping_;
	std::list<Net::Client *> clients_;
};

#endif