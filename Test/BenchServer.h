#ifndef Benchmark_BenchServer_INCLUDED
#define Benchmark_BenchServer_INCLUDED

#include "BenchCommon.h"
#include "Interface/Server.h"

class BenchServer : public BenchCommon {
public:
	BenchServer(bool log_detail, bool auto_close, bool echo);
	virtual ~BenchServer();
	virtual void Run(const std::string & address, i32 port) override;
	virtual void OnConnected(Net::Connection * connection) override;
	virtual void OnDisconnected(Net::Connection * connection, bool is_remote) override;
	virtual void ProcessCommand(Net::Connection * connection, const i32 message_size) override;

private:
	bool auto_close_;
	bool echo_;
	Net::Server * server_;
};

#endif