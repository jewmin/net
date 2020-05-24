#ifndef Benchmark_BenchServer_INCLUDED
#define Benchmark_BenchServer_INCLUDED

#include "Net.h"
#include "Reactor/EventReactor.h"
#include "Interface/Server.h"
#include "Interface/Connection.h"

class BenchServer : public Net::INotification {
public:
	BenchServer(bool logDetail);
	virtual ~BenchServer();
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

	static void SignalCb(uv_signal_t * handle, int signum);
	static void CloseCb(uv_handle_t * handle);

private:
	Net::EventReactor * reactor_;
	Net::Server * server_;
	i64 use_time_;
	i64 recv_packet_size_;
	bool quit_;
	bool log_detail_;
	i32 connected_counter_;
	i32 connect_failed_counter_;
	i32 disconnected_counter_;
	i32 packet_counter_;
};

#endif