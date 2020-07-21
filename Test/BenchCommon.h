#ifndef Benchmark_BenchCommon_INCLUDED
#define Benchmark_BenchCommon_INCLUDED

#include "Net.h"
#include "ProtocolDef.h"
#include "Reactor/EventReactor.h"
#include "Interface/Connection.h"
#include "Interface/INotification.h"

class BenchCommon : public Net::INotification {
public:
	virtual ~BenchCommon();
	virtual void ShowStatus();

protected:
	BenchCommon(bool log_detail);

	virtual void Run(const std::string & address, i32 port) = 0;
	virtual void OnConnected(Net::Connection * connection) override;
	virtual void OnConnectFailed(Net::Connection * connection, i32 reason) override;
	virtual void OnDisconnected(Net::Connection * connection, bool is_remote) override;
	virtual void OnNewDataReceived(Net::Connection * connection) override;
	virtual void OnSomeDataSent(Net::Connection * connection) override;

	void Poll();
	i32 GetMinimumMessageSize() const;
	i32 GetMessageSize(Net::Connection * connection) const;
	virtual void ProcessCommand(Net::Connection * connection, const i32 message_size);
	void Send(Net::Connection * connection, const i8 * data, i32 len);

	uv_signal_t * CreateSignal(i32 signum);
	void DeleteSignal(uv_signal_t * handle);
	static void SignalCb(uv_signal_t * handle, int signum);
	static void CloseCb(uv_handle_t * handle);

protected:
	Net::EventReactor * reactor_;
	bool quit_;
	bool log_detail_;
	i64 recv_packet_size_;
	i32 write_counter_;
	i32 connected_counter_;
	i32 connect_failed_counter_;
	i32 disconnected_counter_;
	uv_signal_t * sig_int_;
	uv_signal_t * sig_term_;
	i64 use_time_;
};

#endif