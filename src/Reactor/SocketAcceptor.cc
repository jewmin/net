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

#include "Reactor/SocketAcceptor.h"
#include "Reactor/EventReactor.h"
#include "Reactor/SocketConnection.h"
#include "Sockets/StreamSocket.h"
#include "Category.h"

namespace Net {

SocketAcceptor::SocketAcceptor(EventReactor * reactor) : EventHandler(reactor, Logger::Category::GetCategory("SocketAcceptor")), opened_(false) {
}

SocketAcceptor::~SocketAcceptor() {
	Close();
}

bool SocketAcceptor::Open(const SocketAddress & address, i32 backlog, bool ipv6_only) {
	if (opened_) {
		return false;
	}
	socket_.Open(GetReactor()->GetUvLoop());
	if (socket_.Bind(address, ipv6_only) < 0) {
		return false;
	}
	if (socket_.Listen(backlog) < 0) {
		return false;
	}
	return GetReactor()->AddEventHandler(this);
}

void SocketAcceptor::Close() {
	if (opened_) {
		GetReactor()->RemoveEventHandler(this);
	}
}

bool SocketAcceptor::RegisterToReactor() {
	address_ = socket_.LocalAddress();
	socket_.SetUvData(this);
	opened_ = true;
	return true;
}

bool SocketAcceptor::UnRegisterFromReactor() {
	opened_ = false;
	socket_.Close();
	return true;
}

bool SocketAcceptor::ActivateConnection(SocketConnection * connection) {
	connection->SetReactor(GetReactor());
	return connection->Establish();
}

void SocketAcceptor::AcceptCallback(i32 status) {
	if (status < 0) {
		logger_->Error("AcceptCallback - %s:%s(%d)", *GetListenAddress().ToString(), uv_strerror(status), status);
		return;
	}

	StreamSocket client;
	if (!socket_.AcceptSocket(client)) {
		logger_->Error("AcceptCallback - %s:accept socket error", *GetListenAddress().ToString());
		return;
	}

	SocketConnection * connection = CreateConnection();
	if (!connection) {
		logger_->Error("AcceptCallback - %s:create connecton error", *client.RemoteAddress().ToString());
		return;
	}

	connection->SetSocket(client);
	if (ActivateConnection(connection)) {
		connection->CallOnConnected();
	} else {
		logger_->Error("AcceptCallback - %s:activate connecton error", *client.RemoteAddress().ToString());
		DestroyConnection(connection);
	}
}

}