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

#ifndef Net_Reactor_SocketConnection_INCLUDED
#define Net_Reactor_SocketConnection_INCLUDED

#include "Reactor/EventHandler.h"
#include "Reactor/ConnectState.h"
#include "Reactor/SocketConnector.h"
#include "Sockets/StreamSocket.h"
#include "Common/IOBuffer.h"

namespace Net {

class NET_EXTERN SocketConnection : public EventHandler {
	friend bool SocketConnector::Connect(SocketConnection * connection, const SocketAddress & address);
	class SocketIO : public NetObject {
	public:
		SocketIO(i32 max_out_buffer_size, i32 max_in_buffer_size);
		virtual ~SocketIO();
		IOBuffer * in_buffer_;
		IOBuffer * out_buffer_;

		static const i32 kReadMax = 4096;
	};

public:
	SocketConnection(i32 max_out_buffer_size, i32 max_in_buffer_size);
	virtual ~SocketConnection();

	bool Establish();
	void Shutdown(bool now, i32 reason = UV_ECANCELED);

	i32 Write(const i8 * data, i32 len);
	i32 Read(i8 * data, i32 len);
	i8 * GetRecvData() const;
	i32 GetRecvDataSize() const;
	void PopRecvData(i32 size);

	ConnectState::eState GetConnectState() const;
	StreamSocket * GetSocket();
	void SetSocket(const StreamSocket & socket);

	void CallOnConnected();
	void CallOnConnectFailed(i32 reason);

protected:
	virtual bool RegisterToReactor() override;
	virtual bool UnRegisterFromReactor() override;

	// 通知应用层
	virtual void OnConnected();
	virtual void OnConnectFailed(i32 reason);
	virtual void OnDisconnected(bool is_remote);
	virtual void OnNewDataReceived();
	virtual void OnSomeDataSent();
	virtual void OnError(i32 reason);

private:
	virtual void CloseCallback() override;
	virtual void ShutdownCallback(i32 status, void * arg) override;
	virtual void AllocCallback(uv_buf_t * buf) override;
	virtual void ReadCallback(i32 status) override;
	virtual void WrittenCallback(i32 status, void * arg) override;

	void ShutdownImmediately(i32 reason = UV_ECANCELED);
	void CallOnDisconnected(bool is_remote);
	void InternalError(i32 reason);
	void HandleClose4EOF(i32 reason);
	void HandleClose4Error(i32 reason);

private:
	SocketIO * io_;
	StreamSocket socket_;
	ConnectState::eState connect_state_;
	i32 max_out_buffer_size_;
	i32 max_in_buffer_size_;
	bool shutdown_;
	bool called_on_connected_;
	bool called_on_connectfailed_;
	bool called_on_disconnected_;
};

inline ConnectState::eState SocketConnection::GetConnectState() const {
	return connect_state_;
}

inline StreamSocket * SocketConnection::GetSocket() {
	return &socket_;
}

inline void SocketConnection::SetSocket(const StreamSocket & socket) {
	socket_ = socket;
}

inline i8 * SocketConnection::GetRecvData() const {
	if (!io_) {
		Log(kCrash, __FILE__, __LINE__, "GetRecvData() io_ == nullptr");
	}
	i32 size = 0;
	return io_->in_buffer_->GetContiguousBlock(size);
}

inline i32 SocketConnection::GetRecvDataSize() const {
	if (!io_) {
		Log(kCrash, __FILE__, __LINE__, "GetRecvDataSize() io_ == nullptr");
	}
	i32 size = 0;
	io_->in_buffer_->GetContiguousBlock(size);
	return size;
}

inline void SocketConnection::PopRecvData(i32 size) {
	if (!io_) {
		Log(kCrash, __FILE__, __LINE__, "PopRecvData() io_ == nullptr");
	}
	io_->in_buffer_->DeCommit(size);
}

inline void SocketConnection::CallOnConnected() {
	if (!called_on_connected_) {
		called_on_connected_ = true;
		OnConnected();
	}
}

inline void SocketConnection::CallOnConnectFailed(i32 reason) {
	if (!called_on_connectfailed_) {
		called_on_connectfailed_ = true;
		OnConnectFailed(reason);
	}
}

inline void SocketConnection::CallOnDisconnected(bool is_remote) {
	if (!called_on_disconnected_) {
		called_on_disconnected_ = true;
		OnDisconnected(is_remote);
	}
}

}

#endif