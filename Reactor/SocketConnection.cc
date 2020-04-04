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

#include "Reactor/SocketConnection.h"
#include "Common/Logger.h"
#include "Common/Allocator.h"

namespace Net {

SocketConnection::SocketConnection(i32 max_out_buffer_size, i32 max_in_buffer_size)
	: EventHandler(nullptr), connect_state_(ConnectState::kDisconnected)
	, max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size)
	, shutdown_(false), called_on_disconnected_(false) {
}

SocketConnection::~SocketConnection() {
	ShutdownImmediately();
}

bool SocketConnection::RegisterToReactor() {
	if (ConnectState::kConnecting != connect_state_ && ConnectState::kDisconnected != connect_state_) {
		return false;
	}
	if (socket_.Established() < 0) {
		return false;
	}
	// TODO: 初始化循环队列
	// in_buffer_ = uv_buf_init(static_cast<i8 *>(jc_malloc(max_in_buffer_size_)), 0);
	socket_.SetUvData(this);
	connect_state_ = ConnectState::kConnected;
	return true;
}

bool SocketConnection::UnRegisterFromReactor() {
	if (ConnectState::kConnected != connect_state_ && ConnectState::kDisconnecting != connect_state_) {
		return false;
	}
	connect_state_ = ConnectState::kDisconnected;
	// TODO: 释放循环队列
	// if (in_buffer_.base) {
	// 	jc_free(in_buffer_.base);
	// 	in_buffer_ = uv_buf_init(nullptr, 0);
	// }
	socket_.ShutdownRead();
	socket_.Close();
	return true;
}

bool SocketConnection::Establish() {
	return GetReactor()->AddEventHandler(this);
}

void SocketConnection::Shutdown(bool now) {
	if (ConnectState::kConnecting == connect_state_) {
		shutdown_ = true;
		ShutdownImmediately();
	} else if (ConnectState::kConnected == connect_state_) {
		shutdown_ = true;
		connect_state_ = ConnectState::kDisconnecting;
		if (socket_.GetWriteQueueSize() > 0 && !now) {
			socket_.ShutdownWrite();
		} else {
			ShutdownImmediately();
			CallOnDisconnected(false);
		}
	}
}

void SocketConnection::Destroy() {
	Shutdown(true);
	EventHandler::Destroy();
}

void SocketConnection::ShutdownImmediately() {
	if (ConnectState::kConnecting == connect_state_) {
		connect_state_ = ConnectState::kDisconnected;
		OnConnectFailed(UV_ECANCELED);
		socket_.Close();
	} else if (ConnectState::kConnected == connect_state_ || ConnectState::kDisconnecting == connect_state_) {
		GetReactor()->RemoveEventHandler(this);
	}
}

void SocketConnection::CallOnDisconnected(bool is_remote) {
	if (!called_on_disconnected_) {
		OnDisconnected(is_remote);
		called_on_disconnected_ = true;
	}
}

void SocketConnection::OnConnected() {
}

void SocketConnection::OnConnectFailed(i32 reason) {
}

void SocketConnection::OnDisconnected(bool is_remote) {
}

void SocketConnection::OnNewDataReceived() {
}

void SocketConnection::OnSomeDataSent() {
}

void SocketConnection::OnError(i32 reason) {
}

void SocketConnection::Error(i32 reason) {
	OnError(reason);
	if (UV_EOF == reason) {
		HandleClose4EOF(reason);
	} else if (UV_ECANCELED != reason) {
		Log(kLog, __FILE__, __LINE__, "Error()", reason, uv_err_name(reason), uv_strerror(reason));
		HandleClose4Error(reason);
	}
}

void SocketConnection::HandleClose4EOF(i32 reason) {
	if (ConnectState::kConnected == connect_state_ || ConnectState::kDisconnecting == connect_state_) {
		connect_state_ = ConnectState::kDisconnecting;
		if (shutdown_) {
			socket_.ShutdownRead();
		} else if (socket_.GetWriteQueueSize() > 0) {
			shutdown_ = true;
			socket_.Shutdown();
		}
		} else {
			if (GetOutBufferUsedSize() > 0) {
				shutdown_ = true;
				socket_.Shutdown(shutdown_req_, ShutdownCb);
			} else {
				ShutdownImmediately();
				CallOnDisconnected(true);
			}
		}
	}
}

void SocketConnection::HandleClose4Error(i32 reason) {
	if (ConnectState::kConnected == connect_state_ || ConnectState::kDisconnecting == connect_state_) {
		connect_state_ = ConnectState::kDisconnecting;
		ShutdownImmediately();
		CallOnDisconnected(!shutdown_);
	}
}

i32 SocketConnection::Write(const i8 * data, i32 len) {
	if (ConnectState::kConnected != connect_state_) {
		return UV_ENOTCONN;
	}
	if (max_out_buffer_size_ > 0 && GetOutBufferUsedSize() > max_out_buffer_size_) {
		Log(kLog, __FILE__, __LINE__, "当前写缓冲区 数据大小/需要写入大小/上限", GetOutBufferUsedSize(), len, max_out_buffer_size_);
		return UV_ENOBUFS;
	}
	uv_write_t * req = static_cast<uv_write_t *>(Allocator::Get()->Allocate(sizeof(uv_write_t)));
	i32 status = socket_.Send(data, len, req, WriteCb);
	if (status < 0) {
		Allocator::Get()->DeAllocate(req, sizeof(uv_write_t));
	}
	return status;
}

i32 SocketConnection::Read(i8 * data, i32 len) {
	if (!data || len <= 0 || !in_buffer_.base) {
		return UV_ENOBUFS;
	}
	if (static_cast<i32>(in_buffer_.len) < len) {
		len = in_buffer_.len;
	}
	if (len > 0) {
		std::memcpy(data, in_buffer_.base, len);
		in_buffer_.len -= len;
		if (in_buffer_.len > 0) {
			std::memcpy(in_buffer_.base, in_buffer_.base + len, in_buffer_.len);
		}
	}
	return len;
}

i8 * SocketConnection::GetRecvData() const {
	return in_buffer_.base;
}

i32 SocketConnection::GetRecvDataSize() const {
	return in_buffer_.len;
}

void SocketConnection::PopRecvData(i32 size) {
	if (ConnectState::kConnected != connect_state_ && ConnectState::kDisconnecting != connect_state_) {
		Log(kLog, __FILE__, __LINE__, "---在连接状态为非 kConnected /kDisconnecting 上调用了 PopRecvData(i32)!---");
	} else if (!in_buffer_.base) {
		Log(kCrash, __FILE__, __LINE__, "未初始化接收缓冲区");
	} else if (static_cast<i32>(in_buffer_.len) < size) {
		Log(kCrash, __FILE__, __LINE__, "接收缓冲区大小小于size");
	} else if (size > 0) {
		in_buffer_.len -= size;
		if (in_buffer_.len > 0) {
			std::memmove(in_buffer_.base, in_buffer_.base + size, in_buffer_.len);
		}
	}
}

//*********************************************************************
//Callback
//*********************************************************************

void SocketConnection::CloseCallback() {
	shutdown_ = false;
	called_on_disconnected_ = false;
}

void SocketConnection::ShutdownCallback(i32 status, void * arg) {
	ShutdownImmediately();
	CallOnDisconnected(false);
}

void SocketConnection::AllocCallback(uv_buf_t * buf) {
	// TODO：循环队列赋值
	// uv_buf_t * buffer = &connection->in_buffer_;
	// if (buffer->base) {
	// 	*buf = uv_buf_init(buffer->base + buffer->len, connection->max_in_buffer_size_ - buffer->len);
	// }
}

void SocketConnection::ReadCallback(i32 status) {
	if (status < 0) {
		// Error(status);
	} else if (status > 0) {
		// TODO: 循环队列移动
		// connection->in_buffer_.len += static_cast<i32>(status);
		if (ConnectState::kConnected == connect_state_) {
			OnNewDataReceived();
		}
	}
}

void SocketConnection::WrittenCallback(i32 status, void * arg) {
	if (status < 0) {
		// Error(status);
	} else if (ConnectState::kConnected == connect_state_) {
		// TODO: 循环队列移动
		OnSomeDataSent();
	}
}

}