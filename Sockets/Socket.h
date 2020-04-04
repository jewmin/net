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

#ifndef Net_Sockets_Socket_INCLUDED
#define Net_Sockets_Socket_INCLUDED

#include "Sockets/SocketImpl.h"

namespace Net {

class Socket {
public:
	Socket();
	Socket(const Socket & rhs);
	Socket & operator=(const Socket & rhs);
	virtual ~Socket();
	
	void Open(uv_loop_t * loop);
	void Close();

	void SetSendBufferSize(i32 size);
	i32 GetSendBufferSize() const;
	i32 GetWriteQueueSize() const;
	void SetRecvBufferSize(i32 size);
	i32 GetRecvBufferSize() const;
	SocketAddress LocalAddress();
	SocketAddress RemoteAddress();
	void SetNoDelay();
	void SetKeepAlive(i32 interval);

	void SetUvData(UvData * data);
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

inline void Socket::Close() {
	impl_->Close();
}

inline void Socket::SetSendBufferSize(i32 size) {
	impl_->SetSendBufferSize(size);
}

inline i32 Socket::GetSendBufferSize() const {
	return impl_->GetSendBufferSize();
}

inline i32 Socket::GetWriteQueueSize() const {
	return impl_->GetWriteQueueSize();
}

inline void Socket::SetRecvBufferSize(i32 size) {
	impl_->SetRecvBufferSize(size);
}

inline i32 Socket::GetRecvBufferSize() const {
	return impl_->GetRecvBufferSize();
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

inline void Socket::SetKeepAlive(i32 interval) {
	impl_->SetKeepAlive(interval);
}

inline void Socket::SetUvData(UvData * data) {
	impl_->SetUvData(data);
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