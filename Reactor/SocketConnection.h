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

#ifndef Net_SocketConnection_INCLUDED
#define Net_SocketConnection_INCLUDED

#include "EventHandler.h"
#include "Sockets/StreamSocket.h"

namespace Net {
	class SocketConnection : public EventHandler {
	public:
		enum eConnectState { kConnecting, kConnected, kDisconnecting, kDisconnected };
		SocketConnection(int maxOutBufferSize, int maxInBufferSize);
		virtual ~SocketConnection();

		bool Open();
		void Shutdown(bool now);
		virtual void Destroy();
		virtual void OnConnected();
		virtual void OnConnectFailed(int reason);
		virtual void OnDisconnected(bool isRemote);
		virtual void OnNewDataReceived();
		virtual void OnSomeDataSent();
		virtual void OnError(int reason);
		int Write(const char * data, int len);
		int Read(char * data, int len);
		char * GetRecvData() const;
		int GetRecvDataSize() const;
		void PopRecvData(int size);
		eConnectState GetConnectState() const;
		void SetConnectState(eConnectState state);
		StreamSocket * GetSocket();
		void SetMaxOutBufferSize(int size);
		void SetMaxInBufferSize(int size);
		int GetMaxOutBufferSize() const;
		int GetMaxInBufferSize() const;
		int GetOutBufferUsedSize();

	protected:
		virtual bool RegisterToReactor();
		virtual bool UnRegisterFromReactor();

	private:
		void ShutdownImmediately();
		void Error(int reason);
		void HandleClose4EOF(int reason);
		void HandleClose4Error(int reason);
		void CallOnDisconnected(bool isRemote);
		static void CloseCb(uv_handle_t * handle);
		static void AllocCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
		static void ReadCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
		static void ShutdownCb(uv_shutdown_t * req, int status);
		static void WriteCb(uv_write_t * req, int status);

	private:
		StreamSocket socket_;
		eConnectState connect_state_;
		uv_buf_t in_buffer_;
		uv_shutdown_t * shutdown_req_;
		int max_out_buffer_size_;
		int max_in_buffer_size_;
		bool shutdown_;
		bool called_on_disconnected_;
	};
}

inline Net::SocketConnection::eConnectState Net::SocketConnection::GetConnectState() const {
	return connect_state_;
}

inline void Net::SocketConnection::SetConnectState(eConnectState state) {
	connect_state_ = state;
}

inline Net::StreamSocket * Net::SocketConnection::GetSocket() {
	return &socket_;
}

inline int Net::SocketConnection::GetMaxOutBufferSize() const {
	return max_out_buffer_size_;
}

inline int Net::SocketConnection::GetMaxInBufferSize() const {
	return max_in_buffer_size_;
}

#endif