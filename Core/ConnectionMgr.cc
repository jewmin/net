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

ConnectionMgr::ConnectionMgr(const std::string & name)
	: name_(name), notification_(nullptr), object_mgr_(new ObjectMgr<Connection>()) {
	if (!object_mgr_) {
		Log(kCrash, __FILE__, __LINE__, "ConnectionMgr() object_mgr_ == nullptr");
	}
}

ConnectionMgr::~ConnectionMgr() {
	object_mgr_->VisitObj(DestroyConnection, nullptr);
	delete object_mgr_;
}

void ConnectionMgr::Register(Connection * connection) {
	if (!connection->IsRegister2Mgr()) {
		i64 id = connection->GetConnectionId();
		if (id < 0) {
			id = object_mgr_->AddNewObj(connection);
			connection->SetConnectionId(id);
		} else {
			object_mgr_->AddNewObjById(id, connection);
		}
		connection->SetRegister2Mgr(true);
	}
}

void ConnectionMgr::UnRegister(Connection * connection) {
	if (connection->IsRegister2Mgr()) {
		connection->SetRegister2Mgr(false);
		object_mgr_->RemoveObj(connection->GetConnectionId());
		DestroyConnection(connection, nullptr);
	}
}

void ConnectionMgr::Update() {
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

void ConnectionMgr::DestroyConnection(Connection * connection, void * ud) {
	delete connection;
}

void ConnectionMgr::ShutdownConnection(Connection * connection, void * ud) {
	connection->Shutdown(false);
}

}