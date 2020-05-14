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
	if (!mgr_) {
		Log(kCrash, __FILE__, __LINE__, "Connection() mgr_ == nullptr");
	}
}

Connection::~Connection() {
}

void Connection::OnConnected() {
	mgr_->Register(this);
	if (mgr_->notification_) {
		if (0 != mgr_->notification_->OnConnected(this)) {
			Shutdown(true);
		}
	}
}

void Connection::OnConnectFailed(i32 reason) {
	if (mgr_->notification_) {
		mgr_->notification_->OnConnectFailed(this, reason);
	}
	mgr_->UnRegister(this);
}

void Connection::OnDisconnected(bool is_remote) {
	if (mgr_->notification_) {
		mgr_->notification_->OnDisconnected(this, is_remote);
	}
	mgr_->UnRegister(this);
}

void Connection::OnNewDataReceived() {
	if (mgr_->notification_) {
		if (0 != mgr_->notification_->OnNewDataReceived(this)) {
			Shutdown(false);
		}
	}
}

void Connection::OnSomeDataSent() {
	if (mgr_->notification_) {
		if (0 != mgr_->notification_->OnSomeDataSent(this)) {
			Shutdown(false);
		}
	}
}

void Connection::OnError(i32 reason) {
	if (UV_EOF == reason) {
		Log(kLog, __FILE__, __LINE__, connection_id_, "收到对端EOF, 正常断开");
	} else {
		Log(kLog, __FILE__, __LINE__, connection_id_, "尝试读写数据时, 产生了错误", uv_err_name(reason), uv_strerror(reason));
	}
}

}