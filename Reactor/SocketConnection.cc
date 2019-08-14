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

#include "SocketConnection.h"

Net::SocketConnection::SocketConnection() : EventHandler(nullptr), connect_state_(kDisconnected) {
}

Net::SocketConnection::~SocketConnection() {
}

bool Net::SocketConnection::RegisterToReactor() {
	if (kConnecting != connect_state_ || kDisconnected != connect_state_) {
		return false;
	}
	if (socket_.Established(AllocCb, ReadCb) < 0) {
		return false;
	}
	uv_handle_set_data(socket_.GetHandle(), this);
	Duplicate();
	connect_state_ = kConnected;
	return true;
}

bool Net::SocketConnection::UnRegisterFromReactor() {
	if (kConnected != connect_state_ || kDisconnecting != connect_state_) {
		return false;
	}
	connect_state_ = kDisconnected;
	socket_.Close(CloseCb);
	return true;
}

bool Net::SocketConnection::Open() {
	return GetReactor()->AddEventHandler(this);
}

void Net::SocketConnection::Shutdown() {
	GetReactor()->RemoveEventHandler(this);
}

void Net::SocketConnection::OnConnected() {
}

void Net::SocketConnection::OnConnectFailed(int reason) {
}

void Net::SocketConnection::OnDisconnect() {
}

void Net::SocketConnection::OnDisconnected() {
}

void Net::SocketConnection::OnNewDataReceived() {
}

void Net::SocketConnection::OnSomeDataSent() {
}

void Net::SocketConnection::CloseCb(uv_handle_t * handle) {
	SocketConnection * connection = static_cast<SocketConnection *>(handle->data);
	free(handle);
	if (connection) {
		connection->Release();
	}
}