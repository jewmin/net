
#ifndef Benchmark_BenchClient_INCLUDED
#define Benchmark_BenchClient_INCLUDED

#include "Net.h"
#include "Reactor/SocketConnector.h"
#include "Interface/INotification.h"
#include "Interface/Connection.h"

class BenchClient : public Net::INotification {
public:
	BenchClient(i32 clientCount, i32 packetCount, i32 packetSize, bool logDetail);
	virtual ~BenchClient();
	void Run(const std::string & address, i32 port);
	void ShowStatus();

protected:
	virtual void OnConnected(Net::Connection * connection) override;
	virtual void OnConnectFailed(Net::Connection * connection, i32 reason) override;
	virtual void OnDisconnected(Net::Connection * connection, bool is_remote) override;
	virtual void OnNewDataReceived(Net::Connection * connection) override;
	virtual void OnSomeDataSent(Net::Connection * connection) override;

private:
	i32 GetMessageSize(Net::Connection * connection) const;
	void ProcessCommand(Net::Connection * connection) const;

private:
	Net::EventReactor * reactor_;
	Net::SocketConnector * connector_;
	bool quit_;
	bool log_detail_;
	i32 client_count_;
	i32 packet_count_;
	i32 packet_size_;
	i32 connected_counter_;
	i32 connect_failed_counter_;
	i32 disconnected_counter_;
	i8 * msg_buffer_;
	std::map<i64, i32> client_packet_map_;
};

#endif