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
#include "Common/Logger.h"

namespace Net {

SocketAcceptor::SocketAcceptor(EventReactor * reactor) : EventHandler(reactor), opened_(false) {
}

SocketAcceptor::~SocketAcceptor() {
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

void SocketAcceptor::Destroy() {
	Close();
	EventHandler::Destroy();
}

bool SocketAcceptor::RegisterToReactor() {
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
		Log(kLog, __FILE__, __LINE__, "AcceptCallback()", uv_strerror(status));
		Close();
	} else {
		StreamSocket client;
		if (!socket_.AcceptSocket(client)) {
			Log(kLog, __FILE__, __LINE__, "AcceptCallback() accept socket error");
			return;
		}
		SocketConnection * connection = CreateConnection();
		if (!connection) {
			Log(kLog, __FILE__, __LINE__, "AcceptCallback() connection == null");
			return;
		}
		client.SetNoDelay();
		client.SetKeepAlive(60);
		connection->SetSocket(client);
		if (ActivateConnection(connection)) {
			connection->CallOnConnected();
		} else {
			Log(kLog, __FILE__, __LINE__, "AcceptCallback() ActivateConnection error");
			connection->CallOnConnectFailed(UV_ECANCELED);
		}
	}
}

}