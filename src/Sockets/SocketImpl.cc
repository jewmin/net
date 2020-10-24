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
#include "Sockets/StreamSocketImpl.h"
#include "Allocator.h"
#include "NetworkException.h"

namespace Net {

SocketImpl::SocketImpl() : handle_(nullptr), logger_(Logger::Category::GetCategory("SocketImpl")) {
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
			logger_->Error("uv_tcp_bind() - %s:%s(%d)", *address.ToString(), uv_strerror(status), status);
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
			logger_->Error("uv_listen() - %s:%s(%d)", *LocalAddress().ToString(), uv_strerror(status), status);
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
			logger_->Error("uv_tcp_connect() - %s:%s(%d)", *address.ToString(), uv_strerror(status), status);
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
			logger_->Error("uv_accept() - %s:%s(%d)", *LocalAddress().ToString(), uv_strerror(status), status);
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
			logger_->Error("uv_shutdown() - %s:%s(%d)", *LocalAddress().ToString(), uv_strerror(status), status);
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
	}
	return status;
}

i32 SocketImpl::Established() {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		status = uv_read_start(reinterpret_cast<uv_stream_t *>(handle_), alloc_cb, read_cb);
		if (status < 0) {
			logger_->Error("uv_read_start() - %s:%s(%d)", *LocalAddress().ToString(), uv_strerror(status), status);
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
	i32 status = uv_write(req, reinterpret_cast<uv_stream_t *>(handle_), &buf, 1, write_cb);
	if (status < 0) {
		jc_free(req);
		logger_->Error("uv_write() - %s:%s(%d)", *LocalAddress().ToString(), uv_strerror(status), status);
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
			logger_->Error("uv_send_buffer_size() set SO_SNDBUF - %s(%d)", uv_strerror(status), status);
		}
	}
}

i32 SocketImpl::GetSendBufferSize() const {
	i32 size = 0;
	if (handle_) {
		i32 status = uv_send_buffer_size(handle_, &size);
		if (status < 0) {
			logger_->Error("uv_send_buffer_size() get SO_SNDBUF - %s(%d)", uv_strerror(status), status);
		}
	}
	return size;
}

i32 SocketImpl::GetWriteQueueSize() const {
	i32 size = 0;
	if (handle_ && UV_TCP == handle_->type) {
		size = static_cast<i32>(uv_stream_get_write_queue_size(reinterpret_cast<uv_stream_t *>(handle_)));
	}
	return size;
}

void SocketImpl::SetRecvBufferSize(i32 size) {
	if (handle_) {
		i32 status = uv_recv_buffer_size(handle_, &size);
		if (status < 0) {
			logger_->Error("uv_recv_buffer_size() set SO_RCVBUF - %s(%d)", uv_strerror(status), status);
		}
	}
}

i32 SocketImpl::GetRecvBufferSize() const {
	i32 size = 0;
	if (handle_) {
		i32 status = uv_recv_buffer_size(handle_, &size);
		if (status < 0) {
			logger_->Error("uv_recv_buffer_size() get SO_RCVBUF - %s(%d)", uv_strerror(status), status);
		}
	}
	return size;
}

SocketAddress SocketImpl::LocalAddress() const {
	if (handle_ && UV_TCP == handle_->type) {
		struct sockaddr_storage buffer;
		struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
		socklen_t len = sizeof(buffer);
		i32 status = uv_tcp_getsockname(reinterpret_cast<uv_tcp_t *>(handle_), sa, reinterpret_cast<i32 *>(&len));
		if (status < 0) {
			logger_->Error("uv_tcp_getsockname() - %s(%d)", uv_strerror(status), status);
		} else {
			return SocketAddress(sa, len);
		}
	}
	return SocketAddress();
}

SocketAddress SocketImpl::RemoteAddress() const {
	if (handle_ && UV_TCP == handle_->type) {
		struct sockaddr_storage buffer;
		struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
		socklen_t len = sizeof(buffer);
		i32 status = uv_tcp_getpeername(reinterpret_cast<uv_tcp_t *>(handle_), sa, reinterpret_cast<i32 *>(&len));
		if (status < 0) {
			logger_->Error("uv_tcp_getpeername() - %s(%d)", uv_strerror(status), status);
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
			logger_->Error("uv_tcp_nodelay() - %s(%d)", uv_strerror(status), status);
		}
	}
}

void SocketImpl::SetKeepAlive(i32 interval) {
	if (handle_ && UV_TCP == handle_->type) {
		i32 status = uv_tcp_keepalive(reinterpret_cast<uv_tcp_t *>(handle_), 1, interval);
		status = uv_translate_sys_error(status);
		if (status < 0) {
			logger_->Error("uv_tcp_keepalive() - %s(%d)", uv_strerror(status), status);
		}
	}
}

void SocketImpl::SetUvData(UvData * data) {
	if (handle_) {
		if (handle_->data) {
			static_cast<Common::WeakReference *>(handle_->data)->Release();
		}
		handle_->data = data ? data->IncWeakRef() : nullptr;
	}
}

//*********************************************************************
//Callback
//*********************************************************************

void SocketImpl::close_cb(uv_handle_t * handle) {
	Common::WeakReference * reference = static_cast<Common::WeakReference *>(handle->data);
	if (reference) {
		UvData * data = dynamic_cast<UvData *>(reference->Lock());
		if (data) {
			data->CloseCallback();
		} else {
			Logger::Category::GetCategory("SocketImpl")->Warn("close_cb() UvData has been released");
		}
		reference->Release();
	}
	if (UV_TCP == handle->type) {
		jc_free(handle);
	} else {
		throw NetworkException(*Common::SDString::Format("close_cb() free specified handle [%s] error", uv_handle_type_name(handle->type)));
	}
}

void SocketImpl::connection_cb(uv_stream_t * server, int status) {
	Common::WeakReference * reference = static_cast<Common::WeakReference *>(server->data);
	if (reference) {
		UvData * data = dynamic_cast<UvData *>(reference->Lock());
		if (data) {
			data->AcceptCallback(status);
		} else {
			Logger::Category::GetCategory("SocketImpl")->Warn("connection_cb() UvData has been released");
		}
	}
}

void SocketImpl::connect_cb(uv_connect_t * req, int status) {
	Common::WeakReference * reference = static_cast<Common::WeakReference *>(req->handle->data);
	if (reference) {
		UvData * data = dynamic_cast<UvData *>(reference->Lock());
		if (data) {
			data->ConnectCallback(status, req->data);
		} else {
			Logger::Category::GetCategory("SocketImpl")->Warn("connect_cb() UvData has been released");
		}
	}
	jc_free(req);
}

void SocketImpl::shutdown_cb(uv_shutdown_t * req, int status) {
	Common::WeakReference * reference = static_cast<Common::WeakReference *>(req->handle->data);
	if (reference) {
		UvData * data = dynamic_cast<UvData *>(reference->Lock());
		if (data) {
			data->ShutdownCallback(status, req->data);
		} else {
			Logger::Category::GetCategory("SocketImpl")->Warn("shutdown_cb() UvData has been released");
		}
	}
	jc_free(req);
}

void SocketImpl::alloc_cb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
	Common::WeakReference * reference = static_cast<Common::WeakReference *>(handle->data);
	if (reference) {
		UvData * data = dynamic_cast<UvData *>(reference->Lock());
		if (data) {
			data->AllocCallback(buf);
		} else {
			Logger::Category::GetCategory("SocketImpl")->Warn("alloc_cb() UvData has been released");
		}
	}
}

void SocketImpl::read_cb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
	Common::WeakReference * reference = static_cast<Common::WeakReference *>(stream->data);
	if (reference) {
		UvData * data = dynamic_cast<UvData *>(reference->Lock());
		if (data) {
			data->ReadCallback(static_cast<i32>(nread));
		} else {
			Logger::Category::GetCategory("SocketImpl")->Warn("read_cb() UvData has been released");
		}
	}
}

void SocketImpl::write_cb(uv_write_t * req, int status) {
	Common::WeakReference * reference = static_cast<Common::WeakReference *>(req->handle->data);
	if (reference) {
		UvData * data = dynamic_cast<UvData *>(reference->Lock());
		if (data) {
			data->WrittenCallback(status, req->data);
		} else {
			Logger::Category::GetCategory("SocketImpl")->Warn("write_cb() UvData has been released");
		}
	}
	jc_free(req);
}

}