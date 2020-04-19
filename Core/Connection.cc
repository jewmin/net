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

#include "Core/Connection.h"
#include "Core/ConnectionMgr.h"
#include "Common/Logger.h"

namespace Net {

Connection::Connection(ConnectionMgr * mgr, i32 max_out_buffer_size, i32 max_in_buffer_size)
	: SocketConnection(max_out_buffer_size, max_in_buffer_size), mgr_(mgr), connection_id_(-1), is_register2mgr_(false) {
}

Connection::~Connection() {
}

void Connection::Destroy() {
	mgr_ = nullptr;
	SocketConnection::Destroy();
}

void Connection::OnConnected() {
	if (mgr_) {
		mgr_->OnConnected(this);
	}
}

void Connection::OnConnectFailed(i32 reason) {
	if (mgr_) {
		mgr_->OnConnectFailed(this, reason);
	}
}

void Connection::OnDisconnected(bool is_remote) {
	if (mgr_) {
		mgr_->OnDisconnected(this, is_remote);
	}
}

void Connection::OnNewDataReceived() {
	if (mgr_) {
		mgr_->OnNewDataReceived(this);
	}
}

void Connection::OnSomeDataSent() {
	if (mgr_) {
		mgr_->OnSomeDataSent(this);
	}
}

void Connection::OnError(i32 reason) {
	if (UV_ECANCELED != reason) {
		Log(kLog, __FILE__, __LINE__, "connection error:", connection_id_, uv_err_name(reason), uv_strerror(reason));
	}
}

}