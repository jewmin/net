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
#include "SocketConnection.h"

Net::SocketConnection::SocketConnection(int maxOutBufferSize, int maxInBufferSize)
	: EventHandler(nullptr), connect_state_(kDisconnected), in_buffer_(uv_buf_init(nullptr, 0))
	, shutdown_req_(new uv_shutdown_t()), max_out_buffer_size_(maxOutBufferSize), max_in_buffer_size_(maxInBufferSize)
	, shutdown_(false), called_on_disconnected_(false) {
}

Net::SocketConnection::~SocketConnection() {
	if (shutdown_req_) {
		delete shutdown_req_;
		shutdown_req_ = nullptr;
	}
}

bool Net::SocketConnection::RegisterToReactor() {
	if (kConnecting != connect_state_ && kDisconnected != connect_state_) {
		return false;
	}
	if (socket_.Established(AllocCb, ReadCb) < 0) {
		return false;
	}
	in_buffer_ = uv_buf_init(static_cast<char *>(malloc(max_in_buffer_size_)), 0);
	uv_handle_set_data(socket_.GetHandle(), this);
	Duplicate();
	connect_state_ = kConnected;
	return true;
}

bool Net::SocketConnection::UnRegisterFromReactor() {
	if (kConnected != connect_state_ && kDisconnecting != connect_state_) {
		return false;
	}
	connect_state_ = kDisconnected;
	if (in_buffer_.base) {
		free(in_buffer_.base);
		in_buffer_ = uv_buf_init(nullptr, 0);
	}
	socket_.ShutdownReceive();
	socket_.Close(CloseCb);
	return true;
}

bool Net::SocketConnection::Open() {
	return GetReactor()->AddEventHandler(this);
}

void Net::SocketConnection::Shutdown(bool now) {
	if (kConnecting == connect_state_) {
		shutdown_ = true;
		ShutdownImmediately();
	} else if (kConnected == connect_state_) {
		shutdown_ = true;
		connect_state_ = kDisconnecting;
		if (GetOutBufferUsedSize() > 0 && !now) {
			socket_.ShutdownSend(shutdown_req_, ShutdownCb);
		} else {
			ShutdownImmediately();
			CallOnDisconnected(false);
		}
	}
}

void Net::SocketConnection::ShutdownImmediately() {
	if (kConnecting == connect_state_) {
		connect_state_ = kDisconnected;
		socket_.Close();
		OnConnectFailed(UV_ECANCELED);
	} else if (kConnected == connect_state_ || kDisconnecting == connect_state_) {
		GetReactor()->RemoveEventHandler(this);
	}
}

int Net::SocketConnection::Write(const char * data, int len) {
	if (kConnected != connect_state_) {
		return UV_ENOTCONN;
	}
	if (max_out_buffer_size_ > 0 && GetOutBufferUsedSize() > max_out_buffer_size_) {
		Foundation::LogWarn("---当前写缓冲区 数据大小/需要写入大小/上限:%d / %d / %d----", GetOutBufferUsedSize(), len, max_out_buffer_size_);
		return UV_ENOBUFS;
	}
	uv_write_t * req = new uv_write_t();
	int status = socket_.Send(data, len, req, WriteCb);
	if (status < 0) {
		delete req;
	}
	return status;
}

int Net::SocketConnection::Read(char * data, int len) {
	if (!data || len <= 0 || !in_buffer_.base) {
		return UV_ENOBUFS;
	}
	if (static_cast<int>(in_buffer_.len) < len) {
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

char * Net::SocketConnection::GetRecvData() const {
	return in_buffer_.base;
}

int Net::SocketConnection::GetRecvDataSize() const {
	return in_buffer_.len;
}

void Net::SocketConnection::PopRecvData(int size) {
	if (kConnected != connect_state_ && kDisconnecting != connect_state_) {
		Foundation::LogErr("---在连接状态为非 kConnected /kDisconnecting 上调用了 PopRecvData(int)!---");
	} else if (!in_buffer_.base) {
		throw std::invalid_argument("未初始化接收缓冲区");
	} else if (static_cast<int>(in_buffer_.len) < size) {
		throw std::invalid_argument("接收缓冲区大小小于size");
	} else if (size > 0) {
		in_buffer_.len -= size;
		if (in_buffer_.len > 0) {
			// std::memcpy(in_buffer_.base, in_buffer_.base + size, in_buffer_.len);
			std::memmove(in_buffer_.base, in_buffer_.base + size, in_buffer_.len);
		}
	}
}

void Net::SocketConnection::Destroy() {
	Shutdown(true);
	Release();
}

void Net::SocketConnection::OnConnected() {
}

void Net::SocketConnection::OnConnectFailed(int reason) {
}

void Net::SocketConnection::OnDisconnected(bool isRemote) {
}

void Net::SocketConnection::OnNewDataReceived() {
}

void Net::SocketConnection::OnSomeDataSent() {
}

void Net::SocketConnection::OnError(int reason) {
}

void Net::SocketConnection::Error(int reason) {
	OnError(reason);
	if (UV_EOF == reason) {
		HandleClose4EOF(reason);
	} else if (UV_ECANCELED != reason) {
		Foundation::LogWarn("网络出错，状态为(%s %s) NO:%d", uv_err_name(reason), uv_strerror(reason), reason);
		HandleClose4Error(reason);
	}
}

void Net::SocketConnection::HandleClose4EOF(int reason) {
	if (kConnected == connect_state_ || kDisconnecting == connect_state_) {
		connect_state_ = kDisconnecting;
		if (shutdown_) {
			socket_.ShutdownReceive();
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

void Net::SocketConnection::HandleClose4Error(int reason) {
	if (kConnected == connect_state_ || kDisconnecting == connect_state_) {
		connect_state_ = kDisconnecting;
		ShutdownImmediately();
		CallOnDisconnected(!shutdown_);
	}
}

void Net::SocketConnection::SetMaxOutBufferSize(int size) {
	if (size > 0) {
		max_out_buffer_size_ = size;
		if (kConnected == connect_state_) {
			socket_.SetSendBufferSize(size);
		}
	}
}

void Net::SocketConnection::SetMaxInBufferSize(int size) {
	if (size > max_in_buffer_size_) {
		max_in_buffer_size_ = size;
		if (kConnected == connect_state_) {
			in_buffer_.base = static_cast<char *>(realloc(in_buffer_.base, size));
			socket_.SetReceiveBufferSize(size);
		}
	}
}

int Net::SocketConnection::GetOutBufferUsedSize() {
	return static_cast<int>(uv_stream_get_write_queue_size(reinterpret_cast<uv_stream_t *>(socket_.GetHandle())));
}

void Net::SocketConnection::CallOnDisconnected(bool isRemote) {
	if (!called_on_disconnected_) {
		OnDisconnected(isRemote);
		called_on_disconnected_ = true;
	}
}

void Net::SocketConnection::CloseCb(uv_handle_t * handle) {
	SocketConnection * connection = static_cast<SocketConnection *>(handle->data);
	free(handle);
	if (connection) {
		connection->shutdown_ = false;
		connection->called_on_disconnected_ = false;
		connection->Release();
	}
}

void Net::SocketConnection::AllocCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
	SocketConnection * connection = static_cast<SocketConnection *>(handle->data);
	if (connection) {
		uv_buf_t * buffer = &connection->in_buffer_;
		if (buffer->base) {
			*buf = uv_buf_init(buffer->base + buffer->len, connection->max_in_buffer_size_ - buffer->len);
		}
	}
}

void Net::SocketConnection::ReadCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
	SocketConnection * connection = static_cast<SocketConnection *>(stream->data);
	if (connection) {
		if (nread < 0) {
			connection->Error(static_cast<int>(nread));
		} else if (nread > 0) {
			connection->in_buffer_.len += static_cast<int>(nread);
			if (kConnected == connection->connect_state_) {
				connection->OnNewDataReceived();
			}
		}
	}
}

void Net::SocketConnection::ShutdownCb(uv_shutdown_t * req, int status) {
	SocketConnection * connection = static_cast<SocketConnection *>(req->handle->data);
	if (connection) {
		connection->ShutdownImmediately();
		connection->CallOnDisconnected(false);
	}
}

void Net::SocketConnection::WriteCb(uv_write_t * req, int status) {
	SocketConnection * connection = static_cast<SocketConnection *>(req->handle->data);
	delete req;
	if (connection) {
		if (status < 0) {
			connection->Error(status);
		} else if (kConnected == connection->connect_state_) {
			connection->OnSomeDataSent();
		}
	}
}
