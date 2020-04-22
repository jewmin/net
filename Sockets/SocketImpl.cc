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

#include "Sockets/SocketImpl.h"
#include "Common/Allocator.h"
#include "Sockets/StreamSocketImpl.h"

namespace Net {

SocketImpl::SocketImpl() : handle_(nullptr) {
}

SocketImpl::~SocketImpl() {
	Close();
}

void SocketImpl::Open(uv_loop_t * loop) {
	if (!handle_) {
		handle_ = static_cast<uv_handle_t *>(jc_malloc(sizeof(uv_tcp_t)));
		uv_tcp_init(loop, reinterpret_cast<uv_tcp_t *>(handle_));
		handle_->data = nullptr;
	}
}

void SocketImpl::Close() {
	if (handle_) {
		if (!uv_is_closing(handle_)) {
			uv_close(handle_, close_cb);
		}
		handle_ = nullptr;
	}
}

i32 SocketImpl::Bind(const SocketAddress & address, bool ipv6_only, bool reuse_address) {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		i32 flags = 0;
		if (ipv6_only) {
			flags |= UV_TCP_IPV6ONLY;
		}
		status = uv_tcp_bind(reinterpret_cast<uv_tcp_t *>(handle_), address.Addr(), flags);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_tcp_bind()", address.ToString().c_str(), uv_strerror(status));
			Close();
		}
	}
	return status;
}

i32 SocketImpl::Listen(i32 backlog) {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		status = uv_listen(reinterpret_cast<uv_stream_t *>(handle_), backlog, connection_cb);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_listen()", LocalAddress().ToString().c_str(), uv_strerror(status));
			Close();
		}
	}
	return status;
}

i32 SocketImpl::Connect(const SocketAddress & address, void * arg) {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		uv_connect_t * req = static_cast<uv_connect_t *>(jc_malloc(sizeof(uv_connect_t)));
		status = uv_tcp_connect(req, reinterpret_cast<uv_tcp_t *>(handle_), address.Addr(), connect_cb);
		if (status < 0) {
			jc_free(req);
			Log(kLog, __FILE__, __LINE__, "uv_tcp_connect()", address.ToString().c_str(), uv_strerror(status));
			Close();
		} else {
			req->data = arg;
		}
	}
	return status;
}

SocketImpl * SocketImpl::AcceptSocket(SocketAddress & client_address) {
	if (handle_ && UV_TCP == handle_->type) {
		SocketImpl * client = new StreamSocketImpl();
		client->Open(handle_->loop);
		i32 status = uv_accept(reinterpret_cast<uv_stream_t *>(handle_), reinterpret_cast<uv_stream_t *>(client->handle_));
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_accept()", LocalAddress().ToString().c_str(), uv_strerror(status));
			client->Release();
		} else {
			client_address = client->RemoteAddress();
			return client;
		}
	}
	return nullptr;
}

i32 SocketImpl::Shutdown(void * arg) {
	i32 status = ShutdownRead();
	if (0 == status) {
		status = ShutdownWrite(arg);
	}
	return status;
}

i32 SocketImpl::ShutdownWrite(void * arg) {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		uv_shutdown_t * req = static_cast<uv_shutdown_t *>(jc_malloc(sizeof(uv_shutdown_t)));
		status = uv_shutdown(req, reinterpret_cast<uv_stream_t *>(handle_), shutdown_cb);
		if (status < 0) {
			jc_free(req);
			Log(kLog, __FILE__, __LINE__, "uv_shutdown()", uv_strerror(status));
		} else {
			req->data = arg;
		}
	}
	return status;
}

i32 SocketImpl::ShutdownRead() {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		status = uv_read_stop(reinterpret_cast<uv_stream_t *>(handle_));
		// if (status < 0) {
		// 	Log(kLog, __FILE__, __LINE__, "uv_read_stop()", uv_strerror(status));
		// }
	}
	return status;
}

i32 SocketImpl::Established() {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		status = uv_read_start(reinterpret_cast<uv_stream_t *>(handle_), alloc_cb, read_cb);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_read_start()", uv_strerror(status));
		}
	}
	return status;
}

i32 SocketImpl::Write(const i8 * data, i32 len, void * arg) {
	if (!handle_ || UV_TCP != handle_->type) {
		return UV_EPROTONOSUPPORT;
	}
	if (!data || len <= 0) {
		return UV_ENOBUFS;
	}
	uv_buf_t buf = uv_buf_init(const_cast<i8 *>(data), len);
	uv_write_t * req = static_cast<uv_write_t *>(jc_malloc(sizeof(uv_write_t)));
	i32 status = uv_write(req, reinterpret_cast<uv_stream_t * >(handle_), &buf, 1, write_cb);
	if (status < 0) {
		jc_free(req);
		Log(kLog, __FILE__, __LINE__, "uv_write()", uv_strerror(status));
		return status;
	} else {
		req->data = arg;
	}
	return len;
}

void SocketImpl::SetSendBufferSize(i32 size) {
	if (handle_) {
		i32 status = uv_send_buffer_size(handle_, &size);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_send_buffer_size() set SO_SNDBUF error", uv_strerror(status));
		}
	}
}

i32 SocketImpl::GetSendBufferSize() const {
	i32 size = 0;
	if (handle_) {
		i32 status = uv_send_buffer_size(handle_, &size);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_send_buffer_size() get SO_SNDBUF error", uv_strerror(status));
		}
	}
	return size;
}

i32 SocketImpl::GetWriteQueueSize() const {
	i32 size = 0;
	if (handle_ && UV_TCP == handle_->type) {
		size = static_cast<i32>(uv_stream_get_write_queue_size(reinterpret_cast<uv_stream_t * >(handle_)));
	}
	return size;
}

void SocketImpl::SetRecvBufferSize(i32 size) {
	if (handle_) {
		i32 status = uv_recv_buffer_size(handle_, &size);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_recv_buffer_size() set SO_RCVBUF error", uv_strerror(status));
		}
	}
}

i32 SocketImpl::GetRecvBufferSize() const {
	i32 size = 0;
	if (handle_) {
		i32 status = uv_recv_buffer_size(handle_, &size);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_recv_buffer_size() get SO_RCVBUF error", uv_strerror(status));
		}
	}
	return size;
}

SocketAddress SocketImpl::LocalAddress() {
	if (handle_ && UV_TCP == handle_->type) {
		struct sockaddr_storage buffer;
		struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
		socklen_t len = sizeof(buffer);
		i32 status = uv_tcp_getsockname(reinterpret_cast<uv_tcp_t *>(handle_), sa, reinterpret_cast<i32 *>(&len));
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_tcp_getsockname()", uv_strerror(status));
		} else {
			return SocketAddress(sa, len);
		}
	}
	return SocketAddress();
}

SocketAddress SocketImpl::RemoteAddress() {
	if (handle_ && UV_TCP == handle_->type) {
		struct sockaddr_storage buffer;
		struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
		socklen_t len = sizeof(buffer);
		i32 status = uv_tcp_getpeername(reinterpret_cast<uv_tcp_t *>(handle_), sa, reinterpret_cast<i32 *>(&len));
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_tcp_getpeername()", uv_strerror(status));
		} else {
			return SocketAddress(sa, len);
		}
	}
	return SocketAddress();
}

void SocketImpl::SetNoDelay() {
	if (handle_ && UV_TCP == handle_->type) {
		i32 status = uv_tcp_nodelay(reinterpret_cast<uv_tcp_t *>(handle_), 1);
		status = uv_translate_sys_error(status);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_tcp_nodelay()", uv_strerror(status));
		}
	}
}

void SocketImpl::SetKeepAlive(i32 interval) {
	if (handle_ && UV_TCP == handle_->type) {
		i32 status = uv_tcp_keepalive(reinterpret_cast<uv_tcp_t *>(handle_), 1, interval);
		status = uv_translate_sys_error(status);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "uv_tcp_keepalive()", uv_strerror(status));
		}
	}
}

void SocketImpl::SetUvData(UvData * data) {
	if (!handle_) {
		Log(kCrash, __FILE__, __LINE__, "SetUvData() handle_ == nullptr");
	}
	if (handle_->data != data) {
		if (handle_->data) {
			static_cast<UvData *>(handle_->data)->Release();
		}
		handle_->data = data;
		if (data) {
			data->Duplicate();
		}
	}
}

//*********************************************************************
//Callback
//*********************************************************************

void SocketImpl::close_cb(uv_handle_t * handle) {
	UvData * data = static_cast<UvData *>(handle->data);
	if (data) {
		data->CloseCallback();
		data->Release();
	}
	if (UV_TCP == handle->type) {
		jc_free(handle);
	} else {
		Log(kCrash, __FILE__, __LINE__, "close_cb() free specified handle [", uv_handle_type_name(handle->type), "] error");
	}
}

void SocketImpl::connection_cb(uv_stream_t * server, int status) {
	UvData * data = static_cast<UvData *>(server->data);
	if (data) {
		data->AcceptCallback(status);
	} else {
		Log(kLog, __FILE__, __LINE__, "connection_cb() server->data == nullptr");
	}
}

void SocketImpl::connect_cb(uv_connect_t * req, int status) {
	UvData * data = static_cast<UvData *>(req->handle->data);
	if (data) {
		data->ConnectCallback(status, req->data);
		data->Release();
	} else {
		Log(kLog, __FILE__, __LINE__, "connect_cb() req->handle->data == nullptr");
	}
	jc_free(req);
}

void SocketImpl::shutdown_cb(uv_shutdown_t * req, int status) {
	UvData * data = static_cast<UvData *>(req->handle->data);
	if (data) {
		data->ShutdownCallback(status, req->data);
	} else {
		Log(kLog, __FILE__, __LINE__, "shutdown_cb() req->handle->data == nullptr");
	}
	jc_free(req);
}

void SocketImpl::alloc_cb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
	UvData * data = static_cast<UvData *>(handle->data);
	if (data) {
		data->AllocCallback(buf);
	} else {
		Log(kLog, __FILE__, __LINE__, "alloc_cb() handle->data == nullptr");
	}
}

void SocketImpl::read_cb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
	UvData * data = static_cast<UvData *>(stream->data);
	if (data) {
		data->ReadCallback(static_cast<i32>(nread));
	} else {
		Log(kLog, __FILE__, __LINE__, "read_cb() stream->data == nullptr");
	}
}

void SocketImpl::write_cb(uv_write_t * req, int status) {
	UvData * data = static_cast<UvData *>(req->handle->data);
	if (data) {
		data->WrittenCallback(status, req->data);
	} else {
		Log(kLog, __FILE__, __LINE__, "write_cb() req->handle->data == nullptr");
	}
	jc_free(req);
}

}