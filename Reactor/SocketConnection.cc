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
#include "Common/BipBuffer.h"
#include "Common/StraightBuffer.h"

namespace Net {

SocketConnection::SocketIO::SocketIO(i32 max_out_buffer_size, i32 max_in_buffer_size)
	: in_buffer_(new StraightBuffer()), out_buffer_(new BipBuffer()) {
	in_buffer_->Allocate(max_in_buffer_size);
	out_buffer_->Allocate(max_out_buffer_size);
}

SocketConnection::SocketIO::~SocketIO() {
}

SocketConnection::SocketConnection(i32 max_out_buffer_size, i32 max_in_buffer_size)
	: EventHandler(nullptr), io_(nullptr), connect_state_(ConnectState::kDisconnected)
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
	if (io_) {
		Log(kCrash, __FILE__, __LINE__, "RegisterToReactor() io_ is not nullptr");
	}
	io_ = new SocketIO(max_out_buffer_size_, max_in_buffer_size_);
	socket_.SetUvData(this);
	connect_state_ = ConnectState::kConnected;
	return true;
}

bool SocketConnection::UnRegisterFromReactor() {
	if (ConnectState::kConnected != connect_state_ && ConnectState::kDisconnecting != connect_state_) {
		return false;
	}
	connect_state_ = ConnectState::kDisconnected;
	if (io_) {
		delete io_;
		io_ = nullptr;
	} else {
		Log(kLog, __FILE__, __LINE__, "UnRegisterFromReactor() io_ is nullptr");
	}
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
		if (io_->out_buffer_->GetCommitedSize() > 0 && !now) {
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
		} else if (io_->out_buffer_->GetCommitedSize() > 0) {
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

	i32 actually_size = 0;
	i8 * block = io_->out_buffer_->GetReserveBlock(len, actually_size);
	if (!block || actually_size < len) {
		Log(kLog, __FILE__, __LINE__, "Write() buffer status used/need/max", io_->out_buffer_->GetCommitedSize(), len, max_out_buffer_size_);
		return UV_ENOBUFS;
	}

	std::memcpy(block, data, actually_size);
	i32 status = socket_.Write(block, actually_size, reinterpret_cast<void *>(actually_size));
	if (0 == status) {
		io_->out_buffer_->Commit(actually_size);
	}
	return status;
}

i32 SocketConnection::Read(i8 * data, i32 len) {
	if (!io_) {
		Log(kCrash, __FILE__, __LINE__, "Read() io_ is nullptr");
	}
	return io_->in_buffer_->Read(data, len);
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
	i32 actually_size = 0;
	i8 * block = io_->in_buffer_->GetReserveBlock(SocketIO::kReadMax, actually_size);
	if (block && actually_size > 0) {
		*buf = uv_buf_init(block, actually_size);
	}
}

void SocketConnection::ReadCallback(i32 status) {
	if (status < 0) {
		Error(status);
	} else {
		io_->in_buffer_->Commit(status);
		if (ConnectState::kConnected == connect_state_ || ConnectState::kDisconnecting == connect_state_) {
			OnNewDataReceived();
		}
	}
}

void SocketConnection::WrittenCallback(i32 status, void * arg) {
	if (status < 0) {
		Error(status);
	} else {
		io_->out_buffer_->DeCommit(reinterpret_cast<i32>(arg));
		if (ConnectState::kConnected == connect_state_ || ConnectState::kDisconnecting == connect_state_) {
			OnSomeDataSent();
		}
	}
}

}