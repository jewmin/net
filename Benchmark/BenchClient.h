#ifndef Benchmark_BenchClient_INCLUDED
#define Benchmark_BenchClient_INCLUDED

#include "CommonDef.h"
#include "Core/IEvent.h"
#include "Core/SocketWrapper.h"
#include "Reactor/SocketConnector.h"

class BenchClient : public Net::IEvent {
public:
	BenchClient(int clientCount, int packetCount, int packetSize);
	~BenchClient();
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