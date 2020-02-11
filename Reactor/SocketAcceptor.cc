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

#include "Logger.h"
#include "SocketAcceptor.h"

namespace Net {

SocketAcceptor::SocketAcceptor(EventReactor * reactor) : EventHandler(reactor), opened_(false) {
}

SocketAcceptor::~SocketAcceptor() {
}

bool SocketAcceptor::Open(const SocketAddress & address, i32 backlog, bool ipv6_only) {
	if (opened_) {
		return false;
	}
	socket_.Open(GetReactor()->GetEventLoop());
	if (socket_.Bind(address, ipv6_only) < 0) {
		return false;
	}
	if (socket_.Listen(backlog, AcceptCb) < 0) {
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
	Release();
}

bool SocketAcceptor::RegisterToReactor() {
	uv_handle_set_data(socket_.GetHandle(), this);
	Duplicate();
	opened_ = true;
	return true;
}

bool SocketAcceptor::UnRegisterFromReactor() {
	opened_ = false;
	socket_.Close(CloseCb);
	return true;
}

void SocketAcceptor::AcceptConnection(SocketConnection * connection, uv_handle_t * handle) {
	connection->GetSocket()->Attach(handle);
	connection->GetSocket()->SetNoDelay();
	connection->GetSocket()->SetKeepAlive(60);
}

bool SocketAcceptor::ActivateConnection(SocketConnection * connection) {
	connection->SetReactor(GetReactor());
	return connection->Open();
}

void SocketAcceptor::Accept() {
	StreamSocket client;
	if (!socket_.AcceptConnection(client)) {
		return;
	}
	SocketConnection * connection = nullptr;
	MakeConnection(connection);
	AcceptConnection(connection, client.Detatch());
	if (ActivateConnection(connection)) {
		OnAccepted(connection);
		connection->OnConnected();
	}
}

void SocketAcceptor::CloseCb(uv_handle_t * handle) {
	SocketAcceptor * acceptor = static_cast<SocketAcceptor *>(handle->data);
	SocketImpl::FreeHandle(handle);
	if (acceptor) {
		acceptor->Release();
	}
}

void SocketAcceptor::AcceptCb(uv_stream_t * server, i32 status) {
	SocketAcceptor * acceptor = static_cast<SocketAcceptor *>(server->data);
	if (acceptor) {
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "监听回调失败", uv_strerror(status));
			acceptor->Close();
		} else {
			acceptor->Accept();
		}
	}
}

}