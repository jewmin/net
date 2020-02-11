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

#include "SocketConnector.h"

namespace Net {

SocketConnector::SocketConnector(EventReactor * reactor) : EventHandler(reactor) {
}

SocketConnector::~SocketConnector() {
}

i32 SocketConnector::Connect(SocketConnection * connection, const SocketAddress & address) {
	connection->GetSocket()->Open(GetReactor()->GetEventLoop());
	uv_connect_t * req = static_cast<uv_connect_t *>(Allocator::Get()->Allocate(sizeof(uv_connect_t)));
	i32 status = connection->GetSocket()->Connect(address, req, ConnectCb);
	if (status < 0) {
		Allocator::Get()->DeAllocate(req, sizeof(uv_connect_t));
		connection->GetSocket()->Close();
		connection->OnConnectFailed(status);
	} else {
		req->data = new Context(this, connection);
		connection->SetConnectState(SocketConnection::kConnecting);
	}
	return status;
}

void SocketConnector::Destroy() {
	Release();
}

bool SocketConnector::RegisterToReactor() {
	return false;
}

bool SocketConnector::UnRegisterFromReactor() {
	return false;
}

void SocketConnector::OnOneConnectSuccess(SocketConnection * connection) {
	connection->GetSocket()->SetNoDelay();
	connection->GetSocket()->SetKeepAlive(60);
	if (ActivateConnection(connection)) {
		connection->OnConnected();
	}
}

bool SocketConnector::ActivateConnection(SocketConnection * connection) {
	connection->SetReactor(GetReactor());
	return connection->Open();
}

void SocketConnector::ConnectCb(uv_connect_t * req, i32 status) {
	Context * context = static_cast<Context *>(req->data);
	SocketConnector * connector = nullptr;
	SocketConnection * connection = nullptr;
	if (context) {
		connector = context->connector_;
		connection = context->connection_;
		delete context;
	}
	Allocator::Get()->DeAllocate(req, sizeof(uv_connect_t));
	if (status < 0) {
		if (UV_ECANCELED != status) {
			connection->SetConnectState(SocketConnection::kDisconnected);
			connection->GetSocket()->Close();
			connection->OnConnectFailed(status);
		}
	} else {
		connector->OnOneConnectSuccess(connection);
	}
}

}