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

#include "Interface/ConnectionMgr.h"
#include "Common/Logger.h"

namespace Net {

ConnectionMgr::ConnectionMgr(const std::string & name)
	: mgr_id_(-1), name_(new std::string(name)), notification_(nullptr), object_mgr_(new ObjectMgr<Connection>())
	, need_delete_list_(new std::list<i64>()) {
}

ConnectionMgr::~ConnectionMgr() {
	delete need_delete_list_;
	object_mgr_->VisitObj(DestroyConnection, nullptr);
	delete object_mgr_;
	delete name_;
}

void ConnectionMgr::Update() {
	CleanDeathConnections();
}

void ConnectionMgr::CleanDeathConnections() {
	Connection * connection = nullptr;
	for (auto & it : *need_delete_list_) {
		connection = object_mgr_->RemoveObj(it);
		if (connection) {
			DestroyConnection(connection, nullptr);
		}
	}
	need_delete_list_->clear();
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
		need_delete_list_->push_back(connection->GetConnectionId());
	}
}

void ConnectionMgr::ShutDownAllConnections() {
	object_mgr_->VisitObj(ShutdownConnection, nullptr);
}

void ConnectionMgr::ShutDownOneConnection(i64 id, bool now) {
	Connection * connection = GetConnection(id);
	if (connection) {
		connection->Shutdown(now);
	}
}

void ConnectionMgr::DestroyConnection(Connection * connection, void * ud) {
	delete connection;
}

void ConnectionMgr::ShutdownConnection(Connection * connection, void * ud) {
	connection->Shutdown(false);
}

}