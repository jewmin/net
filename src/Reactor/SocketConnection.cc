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
#include "Reactor/EventReactor.h"
#include "Category.h"

namespace Net {

SocketConnection::SocketConnection(i32 max_out_buffer_size, i32 max_in_buffer_size)
	: EventHandler(nullptr, Logger::Category::GetCategory("SocketConnection")), connect_state_(ConnectState::kDisconnected)
	, max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size), shutdown_(false)
	, called_on_connected_(false), called_on_disconnected_(false) {
}

SocketConnection::~SocketConnection() {
	ShutdownImmediately();
}

bool SocketConnection::RegisterToReactor() {
	if (ConnectState::kDisconnected != connect_state_) {
		return false;
	}
	if (socket_.Established() < 0) {
		return false;
	}
	out_buffer_.Allocate(max_out_buffer_size_);
	in_buffer_.Allocate(max_in_buffer_size_);
	socket_.SetNoDelay();
	socket_.SetKeepAlive(60);
	socket_.SetUvData(this);
	address_ = socket_.RemoteAddress();
	connect_state_ = ConnectState::kConnected;
	return true;
}

bool SocketConnection::UnRegisterFromReactor() {
	if (ConnectState::kConnected != connect_state_ && ConnectState::kDisconnecting != connect_state_) {
		return false;
	}
	connect_state_ = ConnectState::kDisconnected;
	out_buffer_.DeAllocate();
	in_buffer_.DeAllocate();
	address_ = SocketAddress();
	socket_.ShutdownRead();
	socket_.Close();
	return true;
}

bool SocketConnection::Establish() {
	return GetReactor()->AddEventHandler(this);
}

void SocketConnection::Shutdown(bool now) {
	if (ConnectState::kConnected == connect_state_) {
		shutdown_ = true;
		connect_state_ = ConnectState::kDisconnecting;
		if (out_buffer_.ReadableBytes() > 0 && !now) {
			socket_.ShutdownWrite();
		} else {
			ShutdownImmediately();
			CallOnDisconnected(false);
		}
	}
}

void SocketConnection::ShutdownImmediately() {
	if (ConnectState::kConnected == connect_state_ || ConnectState::kDisconnecting == connect_state_) {
		GetReactor()->RemoveEventHandler(this);
	}
}

void SocketConnection::OnConnected() {
}

void SocketConnection::OnDisconnected(bool is_remote) {
}

void SocketConnection::OnNewDataReceived() {
}

void SocketConnection::OnSomeDataSent() {
}

void SocketConnection::OnError(i32 reason) {
}

void SocketConnection::InternalError(i32 reason) {
	OnError(reason);
	if (UV_EOF == reason) {
		HandleClose4EOF(reason);
	} else if (UV_ECANCELED != reason) {
		logger_->Error("InternalError - %s:%s(%d)", *address_.ToString(), uv_strerror(reason), reason);
		HandleClose4Error(reason);
	}
}

void SocketConnection::HandleClose4EOF(i32 reason) {
	if (ConnectState::kConnected == connect_state_ || ConnectState::kDisconnecting == connect_state_) {
		connect_state_ = ConnectState::kDisconnecting;
		if (shutdown_) {
			socket_.ShutdownRead();
		} else if (out_buffer_.ReadableBytes() > 0) {
			shutdown_ = true;
			socket_.Shutdown();
		} else {
			ShutdownImmediately();
			CallOnDisconnected(true);
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

	i32 writable_size = 0;
	i8 * block = out_buffer_.WritableBlock(len, writable_size);
	if (!block || writable_size < len) {
		logger_->Warn("Write %s:buffer not enough, writable / len / total / max : %d / %d / %d / %d", *address_.ToString(), writable_size, len, out_buffer_.ReadableBytes(), max_out_buffer_size_);
		return UV_ENOBUFS;
	}

	std::memcpy(block, data, writable_size);
	i32 status = socket_.Write(block, writable_size, reinterpret_cast<void *>(static_cast<i64>(writable_size)));
	if (status > 0) {
		out_buffer_.IncWriterIndex(writable_size);
	}
	return status;
}

i32 SocketConnection::Read(i8 * data, i32 len) {
	if (ConnectState::kConnected != connect_state_ && ConnectState::kDisconnecting != connect_state_) {
		return UV_ENOTCONN;
	}
	return in_buffer_.ReadBytes(data, len);
}

//*********************************************************************
//Callback
//*********************************************************************

void SocketConnection::CloseCallback() {
	shutdown_ = false;
	called_on_connected_ = false;
	called_on_disconnected_ = false;
}

void SocketConnection::ShutdownCallback(i32 status, void * arg) {
	ShutdownImmediately();
	CallOnDisconnected(false);
}

void SocketConnection::AllocCallback(uv_buf_t * buf) {
	i32 writable_size = 0;
	i8 * block = in_buffer_.WritableBlock(kReadMax, writable_size);
	if (block && writable_size > 0) {
		*buf = uv_buf_init(block, writable_size);
	}
}

void SocketConnection::ReadCallback(i32 status) {
	if (status < 0) {
		InternalError(status);
	} else {
		in_buffer_.IncWriterIndex(status);
		if (ConnectState::kConnected == connect_state_ || ConnectState::kDisconnecting == connect_state_) {
			OnNewDataReceived();
		}
	}
}

void SocketConnection::WrittenCallback(i32 status, void * arg) {
	if (status < 0) {
		InternalError(status);
	} else {
		out_buffer_.IncReaderIndex(static_cast<i32>(reinterpret_cast<i64>(arg)));
		if (ConnectState::kConnected == connect_state_ || ConnectState::kDisconnecting == connect_state_) {
			OnSomeDataSent();
		}
	}
}

}