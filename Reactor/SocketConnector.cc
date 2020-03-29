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

#include "Reactor/SocketConnector.h"
#include "Reactor/ConnectState.h"

namespace Net {

SocketConnector::Context::Context(SocketConnector * connector, SocketConnection * connection)
	: connector_(connector), connection_(connection) {
	connector_->Duplicate();
	connection_->Duplicate();
}

SocketConnector::Context::~Context() {
	connection_->Release();
	connector_->Release();
}

void SocketConnector::Context::ConnectCallback(i32 status, void * arg) {
	StreamSocket * associate_socket = connection_->GetSocket();
	associate_socket->SetUvData(nullptr);
	if (status < 0) {
		if (connection_->GetConnectState() == ConnectState::kConnecting) {
			connection_->SetConnectState(ConnectState::kDisconnected);
			associate_socket->Close();
			connection_->OnConnectFailed(status);
		}
	} else {
		associate_socket->SetNoDelay();
		associate_socket->SetKeepAlive(60);
		if (connector_->ActivateConnection(connection_)) {
			connection_->OnConnected();
		}
	}
}

SocketConnector::SocketConnector(EventReactor * reactor) : EventHandler(reactor) {
}

SocketConnector::~SocketConnector() {
}

i32 SocketConnector::Connect(SocketConnection * connection, const SocketAddress & address) {
	StreamSocket * associate_socket = connection->GetSocket();
	associate_socket->Open(GetReactor()->GetUvLoop());
	i32 status = associate_socket->Connect(address);
	if (status < 0) {
		connection->OnConnectFailed(status);
	} else {
		connection->SetConnectState(ConnectState::kConnecting);
		associate_socket->SetUvData(new Context(this, connection));
	}
	return status;
}

bool SocketConnector::RegisterToReactor() {
	return false;
}

bool SocketConnector::UnRegisterFromReactor() {
	return false;
}

bool SocketConnector::ActivateConnection(SocketConnection * connection) {
	connection->SetReactor(GetReactor());
	return connection->Open();
}

}