#ifndef Benchmark_BenchClient_INCLUDED
#define Benchmark_BenchClient_INCLUDED

#include "CommonDef.h"
#include "Core/IEvent.h"
#include "Core/SocketWrapper.h"
#include "Reactor/SocketConnector.h"

class BenchClient : public Net::IEvent {
public:
	BenchClient(int client_count, int packet_count, int packet_size);
	~BenchClient();
	void Run(const std::string & address, int port);
	void ShowStatus();

protected:
	virtual void OnConnected(Net::SocketWrapper * wrapper);
	virtual void OnConnectFailed(Net::SocketWrapper * wrapper, int reason);
	virtual void OnDisconnected(Net::SocketWrapper * wrapper, bool isRemote);
	virtual void OnNewDataReceived(Net::SocketWrapper * wrapper);
	virtual void OnSomeDataSent(Net::SocketWrapper * wrapper);

private:
	int GetMessageSize(Net::SocketWrapper * wrapper) const;
	void ProcessCommand(Net::SocketWrapper * wrapper) const;

private:
	Net::EventReactor * reactor_;
	Net::SocketConnector * connector_;
	bool quit_;
	int client_count_;
	int packet_count_;
	int packet_size_;
	int connected_counter_;
	int connect_failed_counter_;
	int disconnected_counter_;
	char * buffer_;
	std::map<u64, int> client_packet_map_;
};

#endif