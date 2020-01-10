#ifndef Benchmark_BenchServer_INCLUDED
#define Benchmark_BenchServer_INCLUDED

#include "Net.h"
#include "Core/IEvent.h"
#include "Core/SocketWrapper.h"
#include "Core/SocketServer.h"

class BenchServer : public Net::IEvent {
public:
	BenchServer();
	~BenchServer();
	void Run(const std::string & address, int port);
	void ShowStatus();

protected:
	virtual int OnConnected(Net::SocketWrapper * wrapper);
	virtual int OnConnectFailed(Net::SocketWrapper * wrapper, int reason);
	virtual int OnDisconnected(Net::SocketWrapper * wrapper, bool isRemote);
	virtual int OnNewDataReceived(Net::SocketWrapper * wrapper);
	virtual int OnSomeDataSent(Net::SocketWrapper * wrapper);

private:
	int GetMessageSize(Net::SocketWrapper * wrapper) const;
	void ProcessCommand(Net::SocketWrapper * wrapper) const;
	static void SignalCb(uv_signal_t * handle, int signum);
	static void CloseCb(uv_handle_t * handle);

private:
	Net::EventReactor * reactor_;
	Net::SocketServer * server_;
	i64 use_time_;
	i64 recv_packet_size_;
	bool quit_;
	int connected_counter_;
	int connect_failed_counter_;
	int disconnected_counter_;
	int packet_counter_;
};

#endif