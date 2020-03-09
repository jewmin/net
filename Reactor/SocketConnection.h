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
#include "Sockets/StreamSocket.h"

namespace Net {

class SocketConnection : public EventHandler {
public:
	enum eConnectState { kConnecting, kConnected, kDisconnecting, kDisconnected };

	SocketConnection(i32 max_out_buffer_size, i32 max_in_buffer_size);
	virtual ~SocketConnection();

	bool Open();
	void Shutdown(bool now);
	virtual void Destroy();
	virtual void OnConnected();
	virtual void OnConnectFailed(i32 reason);
	virtual void OnDisconnected(bool isRemote);
	virtual void OnNewDataReceived();
	virtual void OnSomeDataSent();
	virtual void OnError(i32 reason);
	void Error(i32 reason);
	i32 Write(const i8 * data, i32 len);
	i32 Read(i8 * data, i32 len);
	i8 * GetRecvData() const;
	i32 GetRecvDataSize() const;
	void PopRecvData(i32 size);
	eConnectState GetConnectState() const;
	void SetConnectState(eConnectState state);
	StreamSocket * GetSocket();
	void SetMaxOutBufferSize(i32 size);
	void SetMaxInBufferSize(i32 size);
	i32 GetMaxOutBufferSize() const;
	i32 GetMaxInBufferSize() const;
	i32 GetOutBufferUsedSize();

protected:
	virtual bool RegisterToReactor();
	virtual bool UnRegisterFromReactor();

private:
	void ShutdownImmediately();
	void HandleClose4EOF(i32 reason);
	void HandleClose4Error(i32 reason);
	void CallOnDisconnected(bool isRemote);

	static void CloseCb(uv_handle_t * handle);
	static void AllocCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
	static void ReadCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
	static void ShutdownCb(uv_shutdown_t * req, i32 status);
	static void WriteCb(uv_write_t * req, i32 status);

private:
	StreamSocket socket_;
	eConnectState connect_state_;
	uv_buf_t in_buffer_;
	uv_shutdown_t * shutdown_req_;
	i32 max_out_buffer_size_;
	i32 max_in_buffer_size_;
	bool shutdown_;
	bool called_on_disconnected_;
};

inline SocketConnection::eConnectState SocketConnection::GetConnectState() const {
	return connect_state_;
}

inline void SocketConnection::SetConnectState(eConnectState state) {
	connect_state_ = state;
}

inline StreamSocket * SocketConnection::GetSocket() {
	return &socket_;
}

inline i32 SocketConnection::GetMaxOutBufferSize() const {
	return max_out_buffer_size_;
}

inline i32 SocketConnection::GetMaxInBufferSize() const {
	return max_in_buffer_size_;
}

}

#endif