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

#include "Core/ConnectionMgr.h"
#include "Common/Logger.h"

namespace Net {

ConnectionMgr::ConnectionMgr(const std::string & name, u32 object_max_count)
	: name_(name), notification_(nullptr), object_mgr_(new ObjectMgr<Connection>(object_max_count))
	, need_to_delete_list_(new std::list<Connection *>()) {
}

ConnectionMgr::~ConnectionMgr() {
	CleanDeathConnections();
	delete need_to_delete_list_;
	object_mgr_->VisitObj(DestroyConnection, nullptr);
	delete object_mgr_;
}

i64 ConnectionMgr::Register(Connection * connection) {
	if (!connection->IsRegister2Mgr()) {
		i64 id = object_mgr_->AddNewObj(connection);
		if (-1 == id) {
			Log(kLog, __FILE__, __LINE__, "Register() error: id == -1");
			connection->Shutdown(true);
		} else {
			connection->SetConnectionId(id);
			connection->SetRegister2Mgr(true);
		}
	}
	return connection->GetConnectionId();
}

void ConnectionMgr::UnRegister(Connection * connection) {
	if (connection->IsRegister2Mgr()) {
		connection->SetRegister2Mgr(false);
		connection->Shutdown(false);
	}
}

void ConnectionMgr::Update() {
	CleanDeathConnections();
}

void ConnectionMgr::CleanDeathConnections() {
	for (auto & it : *need_to_delete_list_) {
		it->Release();
	}
	need_to_delete_list_->clear();
}

void ConnectionMgr::ShutDownAllConnections() {
	object_mgr_->VisitObj(ShutdownConnection, nullptr);
}

void ConnectionMgr::ShutDownOneConnection(i64 id) {
	Connection * connection = GetConnection(id);
	if (connection) {
		connection->Shutdown(false);
	}
}

void ConnectionMgr::OnConnected(Connection * connection) {
	if (Register(connection) >= 0 && notification_ && (notification_->OnConnected(connection) != 0)) {
		connection->Shutdown(true);
	}
}

void ConnectionMgr::OnConnectFailed(Connection * connection, i32 reason) {
	if (notification_) {
		notification_->OnConnectFailed(connection, reason);
	}
	need_to_delete_list_->push_back(connection);
}

void ConnectionMgr::OnDisconnected(Connection * connection, bool is_remote) {
	if (notification_) {
		notification_->OnDisconnected(connection, is_remote);
	}
	UnRegister(connection);
}

void ConnectionMgr::OnNewDataReceived(Connection * connection) {
	if (notification_ && (notification_->OnNewDataReceived(connection) != 0)) {
		connection->Shutdown(true);
	}
}

void ConnectionMgr::OnSomeDataSent(Connection * connection) {
	if (notification_ && (notification_->OnSomeDataSent(connection) != 0)) {
		connection->Shutdown(true);
	}
}

void ConnectionMgr::DestroyConnection(Connection * connection, void * ud) {
	connection->Destroy();
}

void ConnectionMgr::ShutdownConnection(Connection * connection, void * ud) {
	connection->Shutdown(false);
}

}