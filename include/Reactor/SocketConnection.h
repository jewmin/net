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

#include "Common.h"
#include "Reactor/EventHandler.h"
#include "Reactor/ConnectState.h"
#include "Reactor/SocketAcceptor.h"
#include "Reactor/SocketConnector.h"
#include "Sockets/StreamSocket.h"
#include "Buffer/StraightBuffer.h"
#include "Buffer/BipBuffer.h"
#include "CObject.h"
#include "Address/SocketAddress.h"

namespace Net {

class COMMON_EXTERN SocketConnection : public EventHandler {
	friend class SocketAcceptor;
	friend class SocketConnector;

public:
	SocketConnection(i32 max_out_buffer_size, i32 max_in_buffer_size);
	virtual ~SocketConnection();

	void Shutdown(bool now);
	i32 Write(const i8 * data, i32 len);
	i32 Read(i8 * data, i32 len);
	i8 * GetRecvData();
	i32 GetRecvDataSize();
	void PopRecvData(i32 size);

	ConnectState::eState GetConnectState() const;
	StreamSocket * GetSocket();
	void SetSocket(const StreamSocket & socket);

protected:
	virtual bool RegisterToReactor() override;
	virtual bool UnRegisterFromReactor() override;

	// 通知应用层
	virtual void OnConnected();
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

	bool Establish();
	void ShutdownImmediately();
	void CallOnConnected();
	void CallOnDisconnected(bool is_remote);
	void InternalError(i32 reason);
	void HandleClose4EOF(i32 reason);
	void HandleClose4Error(i32 reason);

private:
	Common::BipBuffer out_buffer_;
	Common::StraightBuffer in_buffer_;
	StreamSocket socket_;
	SocketAddress address_;
	ConnectState::eState connect_state_;
	i32 max_out_buffer_size_;
	i32 max_in_buffer_size_;
	bool shutdown_;
	bool called_on_connected_;
	bool called_on_disconnected_;

	static const i32 kReadMax = 4096;
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

inline i8 * SocketConnection::GetRecvData() {
	i32 readable_size = 0;
	return in_buffer_.ReadableBlock(readable_size);
}

inline i32 SocketConnection::GetRecvDataSize() {
	i32 readable_size = 0;
	in_buffer_.ReadableBlock(readable_size);
	return readable_size;
}

inline void SocketConnection::PopRecvData(i32 size) {
	in_buffer_.IncReaderIndex(size);
}

inline void SocketConnection::CallOnConnected() {
	if (!called_on_connected_) {
		called_on_connected_ = true;
		OnConnected();
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