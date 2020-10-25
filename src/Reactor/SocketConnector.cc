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
#include "Reactor/EventReactor.h"
#include "Reactor/SocketConnection.h"
#include "Category.h"

namespace Net {

SocketConnector::SocketConnector(EventReactor * reactor) : EventHandler(reactor, Logger::Category::GetCategory("SocketConnector")), connect_(false) {
}

SocketConnector::~SocketConnector() {
	Close();
}

bool SocketConnector::Connect(const SocketAddress & address) {
	if (connect_) {
		return false;
	}
	socket_.Open(GetReactor()->GetUvLoop());
	if (socket_.Connect(address) < 0) {
		return false;
	}
	return GetReactor()->AddEventHandler(this);
}

void SocketConnector::Close() {
	if (connect_) {
		GetReactor()->RemoveEventHandler(this);
	}
}

bool SocketConnector::RegisterToReactor() {
	socket_.SetUvData(this);
	connect_ = true;
	return true;
}

bool SocketConnector::UnRegisterFromReactor() {
	connect_ = false;
	socket_.Close();
	return true;
}

bool SocketConnector::ActivateConnection(SocketConnection * connection) {
	connection->SetReactor(GetReactor());
	return connection->Establish();
}

void SocketConnector::ConnectCallback(i32 status, void * arg) {
	StreamSocket client(socket_);
	socket_ = StreamSocket();
	Close();
	if (status < 0) {
		logger_->Error("ConnectCallback - %s:%s(%d)", *client.LocalAddress().ToString(), uv_strerror(status), status);
		OnConnectFailed();
		return;
	}

	SocketConnection * connection = CreateConnection();
	if (!connection) {
		logger_->Error("ConnectCallback - %s:create connecton error", *client.RemoteAddress().ToString());
		OnConnectFailed();
		return;
	}

	connection->SetSocket(client);
	if (ActivateConnection(connection)) {
		connection->CallOnConnected();
	} else {
		logger_->Error("ConnectCallback - %s:activate connecton error", *client.RemoteAddress().ToString());
		OnConnectFailed();
		DestroyConnection(connection);
	}
}

}