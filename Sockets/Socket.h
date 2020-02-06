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

#ifndef Net_Socket_INCLUDED
#define Net_Socket_INCLUDED

#include "SocketImpl.h"

namespace Net {

class Socket {
public:
	Socket();
	Socket(const Socket & rhs);
	Socket & operator=(const Socket & rhs);
	virtual ~Socket();
	
	void Open(uv_loop_t * loop);
	void Close(uv_close_cb cb = SocketImpl::FreeHandle);
	void SetSendBufferSize(int size);
	int GetSendBufferSize() const;
	void SetReceiveBufferSize(int size);
	int GetReceiveBufferSize() const;
	SocketAddress LocalAddress();
	SocketAddress RemoteAddress();
	void SetNoDelay();
	void SetKeepAlive(int interval);
	void Attach(uv_handle_t * handle);
	uv_handle_t * Detatch();
	uv_handle_t * GetHandle();
	SocketImpl * Impl() const;

	bool operator==(const Socket & rhs) const;
	bool operator!=(const Socket & rhs) const;
	bool operator<(const Socket & rhs) const;
	bool operator<=(const Socket & rhs) const;
	bool operator>(const Socket & rhs) const;
	bool operator>=(const Socket & rhs) const;

protected:
	explicit Socket(SocketImpl * impl);

private:
	SocketImpl * impl_;
};

inline void Socket::Open(uv_loop_t * loop) {
	impl_->Open(loop);
}

inline void Socket::Close(uv_close_cb cb) {
	impl_->Close(cb);
}

inline void Socket::SetSendBufferSize(int size) {
	impl_->SetSendBufferSize(size);
}

inline int Socket::GetSendBufferSize() const {
	return impl_->GetSendBufferSize();
}

inline void Socket::SetReceiveBufferSize(int size) {
	impl_->SetReceiveBufferSize(size);
}

inline int Socket::GetReceiveBufferSize() const {
	return impl_->GetReceiveBufferSize();
}

inline SocketAddress Socket::LocalAddress() {
	return impl_->LocalAddress();
}

inline SocketAddress Socket::RemoteAddress() {
	return impl_->RemoteAddress();
}

inline void Socket::SetNoDelay() {
	impl_->SetNoDelay();
}

inline void Socket::SetKeepAlive(int interval) {
	impl_->SetKeepAlive(interval);
}

inline void Socket::Attach(uv_handle_t * handle) {
	impl_->Attach(handle);
}

inline uv_handle_t * Socket::Detatch() {
	return impl_->Detatch();
}

inline uv_handle_t * Socket::GetHandle() {
	return impl_->GetHandle();
}

inline SocketImpl * Socket::Impl() const {
	return impl_;
}

inline bool Socket::operator==(const Socket & rhs) const {
	return impl_ == rhs.impl_;
}

inline bool Socket::operator!=(const Socket & rhs) const {
	return impl_ != rhs.impl_;
}

inline bool Socket::operator<(const Socket & rhs) const {
	return impl_ < rhs.impl_;
}

inline bool Socket::operator<=(const Socket & rhs) const {
	return impl_ <= rhs.impl_;
}

inline bool Socket::operator>(const Socket & rhs) const {
	return impl_ > rhs.impl_;
}

inline bool Socket::operator>=(const Socket & rhs) const {
	return impl_ >= rhs.impl_;
}

}

#endif