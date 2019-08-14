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

#include "SocketAcceptor.h"

Net::SocketAcceptor::SocketAcceptor(EventReactor * reactor) : EventHandler(reactor), opened_(false) {
}

Net::SocketAcceptor::~SocketAcceptor() {
}

bool Net::SocketAcceptor::Open(const SocketAddress & address, int backlog, bool ipV6Only) {
	if (opened_) {
		return false;
	}
	socket_.Open(GetReactor()->GetEventLoop());
	if (socket_.Bind(address, ipV6Only) < 0) {
		return false;
	}
	if (socket_.Listen(backlog, AcceptCb) < 0) {
		return false;
	}
	GetReactor()->AddEventHandler(this);
	opened_ = true;
	return true;
}

void Net::SocketAcceptor::Close() {
	if (opened_) {
		opened_ = false;
		GetReactor()->RemoveEventHandler(this);
	}
}

bool Net::SocketAcceptor::RegisterToReactor() {
	uv_handle_set_data(socket_.GetHandle(), this);
	Duplicate();
	return true;
}

bool Net::SocketAcceptor::UnRegisterFromReactor() {
	socket_.Close(CloseCb);
	return true;
}

void Net::SocketAcceptor::AcceptConnection(SocketConnection * connection, uv_handle_t * handle) {
	connection->GetSocket()->Attach(handle);
	connection->GetSocket()->SetNoDelay();
	connection->GetSocket()->SetKeepAlive(60);
}

bool Net::SocketAcceptor::ActivateConnection(SocketConnection * connection) {
	connection->SetReactor(GetReactor());
	return connection->Open();
}

void Net::SocketAcceptor::Accept() {
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

void Net::SocketAcceptor::CloseCb(uv_handle_t * handle) {
	SocketAcceptor * acceptor = static_cast<SocketAcceptor *>(handle->data);
	free(handle);
	if (acceptor) {
		acceptor->Release();
	}
}

void Net::SocketAcceptor::AcceptCb(uv_stream_t * server, int status) {
	SocketAcceptor * acceptor = static_cast<SocketAcceptor *>(server->data);
	if (acceptor) {
		if (status < 0) {
			LogErr("监听回调失败. %s", uv_strerror(status));
			acceptor->Close();
		} else {
			acceptor->Accept();
		}
	}
}