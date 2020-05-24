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

#include "Interface/AppService.h"
#include "Common/Logger.h"
#include "Common/Allocator.h"

namespace Net {

AppService * AppService::instance_ = nullptr;

//*********************************************************************
//Signal
//*********************************************************************

AppService::Signal::Signal(AppService * service, i32 signum)
	: sig_(static_cast<uv_signal_t *>(jc_malloc(sizeof(uv_signal_t)))) {
	if (uv_signal_init(service->GetReactor()->GetUvLoop(), sig_) < 0) {
		Log(kCrash, __FILE__, __LINE__, "捕获信号 [", signum, "] 初始化失败");
	}
	if (uv_signal_start(sig_, signal_cb, signum) < 0) {
		Log(kCrash, __FILE__, __LINE__, "捕获信号 [", signum, "] 启动失败");
	}
	sig_->data = service;
}

AppService::Signal::~Signal() {
	if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(sig_))) {
		uv_close(reinterpret_cast<uv_handle_t *>(sig_), close_cb);
	}
}

void AppService::Signal::close_cb(uv_handle_t * handle) {
	jc_free(handle);
}

void AppService::Signal::signal_cb(uv_signal_t * handle, int signum) {
	AppService * service = static_cast<AppService *>(handle->data);
	if (service) {
		service->CatchSignal(signum);
	}
}

//*********************************************************************
//AppService
//*********************************************************************

AppService * AppService::Get() {
	return instance_;
}

AppService * AppService::CreateInstance() {
	if (!instance_) {
		instance_ = new AppService();
	}
	return instance_;
}

void AppService::ReleaseInstance() {
	if (instance_) {
		delete instance_;
		instance_ = nullptr;
	}
}

void AppService::DestroyConnectionMgr(ConnectionMgr * mgr, void * ud) {
	delete mgr;
}

void AppService::UpdateConnectionMgr(ConnectionMgr * mgr, void * ud) {
	mgr->Update();
}

//*********************************************************************
//AppService
//*********************************************************************

AppService::AppService()
	: reactor_(new EventReactor()), connector_(new SocketConnector(reactor_))
	, socket_mgr_(new ObjectMgr<ConnectionMgr>()), caught_signal_(0)
	, signal_int_(new Signal(this, SIGINT)), signal_term_(new Signal(this, SIGTERM))
	, signal_func_(nullptr), on_connected_(nullptr), on_connect_failed_(nullptr)
	, on_disconnected_(nullptr), on_received_(nullptr), on_sent_(nullptr) {
}

AppService::~AppService() {
	delete signal_int_;
	delete signal_term_;
	socket_mgr_->VisitObj(DestroyConnectionMgr, nullptr);
	delete socket_mgr_;
	delete connector_;
	delete reactor_;
}

void AppService::OnConnected(Connection * connection) {
	Log(kLog, __FILE__, __LINE__, connection->GetMgr()->GetName().c_str(), "连上了一个新连接 [", connection->GetConnectionId(), "]");
	if (on_connected_) {
		on_connected_(connection->GetMgr()->GetMgrId(), connection->GetConnectionId());
	}
}

void AppService::OnConnectFailed(Connection * connection, i32 reason) {
	if (on_connect_failed_) {
		on_connect_failed_(connection->GetMgr()->GetMgrId(), connection->GetConnectionId(), reason);
	}
}

void AppService::OnDisconnected(Connection * connection, bool is_remote) {
	Log(kLog, __FILE__, __LINE__, connection->GetMgr()->GetName().c_str(), "断开了一个连接 [", connection->GetConnectionId(), "]");
	if (on_disconnected_) {
		on_disconnected_(connection->GetMgr()->GetMgrId(), connection->GetConnectionId(), is_remote);
	}
}

void AppService::OnNewDataReceived(Connection * connection) {
	if (on_received_) {
		on_received_(connection->GetMgr()->GetMgrId(), connection->GetConnectionId(), connection->GetRecvData(), connection->GetRecvDataSize());
		if (ConnectState::kConnected == connection->GetConnectState() || ConnectState::kDisconnecting == connection->GetConnectState()) {
			connection->PopRecvData(connection->GetRecvDataSize());
		}
	}
}

void AppService::OnSomeDataSent(Connection * connection) {
	if (on_sent_) {
		on_sent_(connection->GetMgr()->GetMgrId(), connection->GetConnectionId());
	}
}

void AppService::SetSignal(on_signal_func signal_func) {
	signal_func_ = signal_func;
}

void AppService::SetCallback(on_connected_func on_connected, on_connect_failed_func on_connect_failed, on_disconnected_func on_disconnected, on_received_func on_received, on_sent_func on_sent) {
	on_connected_ = on_connected;
	on_connect_failed_ = on_connect_failed;
	on_disconnected_ = on_disconnected;
	on_received_ = on_received;
	on_sent_ = on_sent;
}

void AppService::RunOnce() {
	DispatchSignal();
	reactor_->Poll();
	socket_mgr_->VisitObj(UpdateConnectionMgr, nullptr);
}

void AppService::DispatchSignal() {
	if (0 != caught_signal_) {
		if (signal_func_) {
			signal_func_(caught_signal_);
		}
		caught_signal_ = 0;
	}
}

void AppService::CatchSignal(i32 signum) {
	switch (signum) {
		case SIGINT:
			Log(kLog, __FILE__, __LINE__, "Received SIGINT, scheduling shutdown...");
			break;

		case SIGTERM:
			Log(kLog, __FILE__, __LINE__, "Received SIGTERM, scheduling shutdown...");
			break;

		default:
			Log(kLog, __FILE__, __LINE__, "Received shutdown signal, scheduling shutdown...");
			break;
	}
	caught_signal_ = signum;
}

i64 AppService::CreateServer(const std::string & name, i32 max_out_buffer_size, i32 max_in_buffer_size) {
	Server * server = new Server(name, reactor_, max_out_buffer_size, max_in_buffer_size);
	server->SetNotification(this);
	i64 server_id = socket_mgr_->AddNewObj(server);
	server->SetMgrId(server_id);
	return server_id;
}

bool AppService::ServerListen(i64 server_id, const std::string & address, i32 port) {
	Server * server = dynamic_cast<Server *>(socket_mgr_->GetObj(server_id));
	if (server) {
		Log(kLog, __FILE__, __LINE__, server->GetName().c_str(), "监听端口", address.c_str(), port);
		return server->Listen(address, port);
	}
	return false;
}

bool AppService::EndServer(i64 server_id) {
	Server * server = dynamic_cast<Server *>(socket_mgr_->GetObj(server_id));
	if (server) {
		Log(kLog, __FILE__, __LINE__, "终止服务", server->GetName().c_str());
		server->Stop();
		return true;
	}
	return false;
}

void AppService::DeleteServer(i64 server_id) {
	Server * server = dynamic_cast<Server *>(socket_mgr_->GetObj(server_id));
	if (server) {
		socket_mgr_->RemoveObj(server_id);
		delete server;
	}
}

i64 AppService::CreateClient(const std::string & name, i32 max_out_buffer_size, i32 max_in_buffer_size) {
	Client * client = new Client(name, reactor_, connector_, max_out_buffer_size, max_in_buffer_size);
	client->SetNotification(this);
	i64 client_id = socket_mgr_->AddNewObj(client);
	client->SetMgrId(client_id);
	return client_id;
}

i64 AppService::ClientConnect(i64 client_id, const std::string & address, i32 port) {
	Client * client = dynamic_cast<Client *>(socket_mgr_->GetObj(client_id));
	if (client) {
		Log(kLog, __FILE__, __LINE__, client->GetName().c_str(), "发起连接", address.c_str(), port);
		return client->Connect(address, port);
	}
	return -1;
}

void AppService::DeleteClient(i64 client_id) {
	Client * client = dynamic_cast<Client *>(socket_mgr_->GetObj(client_id));
	if (client) {
		socket_mgr_->RemoveObj(client_id);
		delete client;
	}
}

i32 AppService::SendData(i64 mgr_id, i64 connection_id, const i8 * data, i32 data_len) {
	ConnectionMgr * mgr = socket_mgr_->GetObj(mgr_id);
	if (mgr) {
		Connection * connection = mgr->GetConnection(connection_id);
		if (connection) {
			return connection->Write(data, data_len);
		}
	}
	return UV_ENOTCONN;
}

void AppService::ShutdownAllConnections(i64 mgr_id) {
	ConnectionMgr * mgr = socket_mgr_->GetObj(mgr_id);
	if (mgr) {
		mgr->ShutDownAllConnections();
	}
}

void AppService::ShutdownConnection(i64 mgr_id, i64 connection_id) {
	ConnectionMgr * mgr = socket_mgr_->GetObj(mgr_id);
	if (mgr) {
		mgr->ShutDownOneConnection(connection_id);
	}
}

void AppService::ShutdownConnectionNow(i64 mgr_id, i64 connection_id) {
	ConnectionMgr * mgr = socket_mgr_->GetObj(mgr_id);
	if (mgr) {
		mgr->ShutDownOneConnection(connection_id, true);
	}
}

SocketAddress AppService::GetConnectionRemoteAddress(i64 mgr_id, i64 connection_id) {
	ConnectionMgr * mgr = socket_mgr_->GetObj(mgr_id);
	if (mgr) {
		Connection * connection = mgr->GetConnection(connection_id);
		if (connection) {
			return connection->GetSocket()->RemoteAddress();
		}
	}
	return SocketAddress();
}

}