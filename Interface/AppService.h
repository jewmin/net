/*
 * MIT License
 *
 * Copyright (c) 2019 jewmin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef Net_Interface_AppService_INCLUDED
#define Net_Interface_AppService_INCLUDED

#include "Common/NetObject.h"
#include "Interface/Client.h"
#include "Interface/Server.h"
#include "Interface/INotification.h"

namespace Net {

class AppService : public INotification {
	class Signal : public NetObject {
	public:
		Signal(AppService * service, i32 signum);
		virtual ~Signal();

	private:
		static void close_cb(uv_handle_t * handle);
		static void signal_cb(uv_signal_t * handle, int signum);

	private:
		Signal(Signal &&) = delete;
		Signal(const Signal &) = delete;
		Signal & operator=(Signal &&) = delete;
		Signal & operator=(const Signal &) = delete;

	private:
		uv_signal_t * sig_;
	};

public:
	virtual ~AppService();

	i64 CreateServer(const std::string & name, i32 max_out_buffer_size, i32 max_in_buffer_size);
	bool ServerListen(i64 server_id, const std::string & address, i32 port);
	bool EndServer(i64 server_id);
	void DeleteServer(i64 server_id);

	i64 CreateClient(const std::string & name, i32 max_out_buffer_size, i32 max_in_buffer_size);
	i64 ClientConnect(i64 client_id, const std::string & address, i32 port);
	void DeleteClient(i64 client_id);

	i32 SendData(i64 mgr_id, i64 connection_id, const i8 * data, i32 data_len);
	void ShutdownAllConnections(i64 mgr_id);
	void ShutdownConnection(i64 mgr_id, i64 connection_id);
	void ShutdownConnectionNow(i64 mgr_id, i64 connection_id);
	SocketAddress GetConnectionRemoteAddress(i64 mgr_id, i64 connection_id);

	void SetSignal(on_signal_func signal_func);
	void SetCallback(on_connected_func on_connected, on_connect_failed_func on_connect_failed, on_disconnected_func on_disconnected, on_received_func on_received, on_sent_func on_sent);
	virtual void RunOnce();

	EventReactor * GetReactor() const;

	static AppService * Get();
	static AppService * CreateInstance();
	static void ReleaseInstance();

protected:
	AppService();

	void DispatchSignal();
	void CatchSignal(i32 signum);
	virtual void OnConnected(Connection * connection) override;
	virtual void OnConnectFailed(Connection * connection, i32 reason) override;
	virtual void OnDisconnected(Connection * connection, bool is_remote) override;
	virtual void OnNewDataReceived(Connection * connection) override;
	virtual void OnSomeDataSent(Connection * connection) override;

private:
	static void DestroyConnectionMgr(ConnectionMgr * mgr, void * ud);
	static void UpdateConnectionMgr(ConnectionMgr * mgr, void * ud);

private:
	AppService(AppService &&) = delete;
	AppService(const AppService &) = delete;
	AppService & operator=(AppService &&) = delete;
	AppService & operator=(const AppService &) = delete;

private:
	EventReactor * reactor_;
	SocketConnector * connector_;
	ObjectMgr<ConnectionMgr> * socket_mgr_;
	i32 caught_signal_;
	Signal * signal_int_;
	Signal * signal_term_;

	on_signal_func signal_func_;
	on_connected_func on_connected_;
	on_connect_failed_func on_connect_failed_;
	on_disconnected_func on_disconnected_;
	on_received_func on_received_;
	on_sent_func on_sent_;

	static AppService * instance_;
};

inline EventReactor * AppService::GetReactor() const {
	return reactor_;
}

}

#endif