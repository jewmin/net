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
#include "Reactor/SocketConnection.h"
#include "Reactor/ConnectState.h"
#include "Common/Logger.h"

namespace Net {

SocketConnector::Context::Context(SocketConnector * connector, SocketConnection * connection)
	: connector_reference_(connector->WeakRef()), connection_reference_(connection->WeakRef()) {
}

SocketConnector::Context::~Context() {
	connector_reference_->Release();
	connection_reference_->Release();
}

void SocketConnector::Context::ConnectCallback(i32 status, void * arg) {
	SocketConnector * connector = dynamic_cast<SocketConnector *>(connector_reference_->Get());
	SocketConnection * connection = dynamic_cast<SocketConnection *>(connection_reference_->Get());
	delete this;
	if (connector && connection) {
		StreamSocket * associate_socket = connection->GetSocket();
		associate_socket->SetUvData(nullptr);
		if (status < 0) {
			connection->Shutdown(true, status);
		} else {
			associate_socket->SetNoDelay();
			associate_socket->SetKeepAlive(60);
			if (connector->ActivateConnection(connection)) {
				connection->CallOnConnected();
			} else {
				Log(kLog, __FILE__, __LINE__, "ConnectCallback() ActivateConnection error");
				connection->CallOnConnectFailed(UV_ECANCELED);
			}
		}
	}
}

SocketConnector::SocketConnector(EventReactor * reactor) : EventHandler(reactor) {
}

SocketConnector::~SocketConnector() {
}

bool SocketConnector::Connect(SocketConnection * connection, const SocketAddress & address) {
	StreamSocket * associate_socket = connection->GetSocket();
	associate_socket->Open(GetReactor()->GetUvLoop());
	i32 status = associate_socket->Connect(address);
	if (status < 0) {
		connection->CallOnConnectFailed(status);
		return false;
	} else {
		connection->connect_state_ = ConnectState::kConnecting;
		associate_socket->SetUvData(new Context(this, connection));
		return true;
	}
}

bool SocketConnector::RegisterToReactor() {
	return false;
}

bool SocketConnector::UnRegisterFromReactor() {
	return false;
}

bool SocketConnector::ActivateConnection(SocketConnection * connection) {
	connection->SetReactor(GetReactor());
	return connection->Establish();
}

}