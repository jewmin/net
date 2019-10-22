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

#ifndef Interface_ApplicationContext_INCLUDED
#define Interface_ApplicationContext_INCLUDED

#include "Interface.h"
#include "Core/IEvent.h"
#include "Core/SocketServer.h"
#include "Core/SocketClient.h"

namespace Interface {
	class ApplicationContext : public Net::IEvent {
	public:
		ApplicationContext(OnSignalFunc onSignal, OnLogFunc onLog);
		~ApplicationContext();

		void RunOnce();

		u64 CreateServer(const char * name, int maxOutBufferSize, int maxInBufferSize);
		bool ServerListen(u64 serverId, const char * address, int port);
		bool EndServer(u64 serverId);
		void DeleteServer(u64 serverId);

		u64 CreateClient(const char * name, int maxOutBufferSize, int maxInBufferSize);
		u32 ClientConnect(u64 clientId, const char * address, int port);
		void DeleteClient(u64 clientId);

		void ShutdownAllConnection(u64 mgrId);
		void ShutdownConnection(u64 mgrId, u32 id);
		void ShutdownConnectionNow(u64 mgrId, u32 id);

		void SendMsg(u64 mgrId, u32 id, int msgId, const char * data, int size);
		void SendRawMsg(u64 mgrId, u32 id, const char * data, int size);
		void SetRawRecv(u64 mgrId, u32 id, bool isRaw);
		void SetCallback(OnConnectedFunc onConnected, OnConnectFailedFunc onConnectFailed, OnDisconnectedFunc onDisconnected, OnRecvMsgFunc onRecvMsg, OnRecvRawMsgFunc onRecvRawMsg);

		static ApplicationContext * GetInstance();
		static ApplicationContext * CreateInstance(OnSignalFunc onSignal, OnLogFunc onLog);
		static void ReleaseInstance();

	protected:
		void SetupSignalHandler(uv_signal_t * handle, int signum);
		void ReleaseSignalHandler(uv_signal_t * handle);
		void SetSignal(int signum);
		void DispatchSignal(int signum);
		void OnSignal(int signum);
		void SendMsg(Net::SocketWrapper * wrapper, int msgId, const char * data, int size);

		virtual void OnConnected(Net::SocketWrapper * wrapper);
		virtual void OnConnectFailed(Net::SocketWrapper * wrapper, int reason);
		virtual void OnDisconnected(Net::SocketWrapper * wrapper, bool isRemote);
		virtual void OnNewDataReceived(Net::SocketWrapper * wrapper);
		virtual void OnSomeDataSent(Net::SocketWrapper * wrapper);

	private:
		static void SignalCb(uv_signal_t * handle, int signum);
		static void CloseCb(uv_handle_t * handle);

	private:
		ApplicationContext() = delete;
		ApplicationContext(const ApplicationContext &) = delete;
		ApplicationContext & operator=(const ApplicationContext &) = delete;

		typedef std::unordered_map<u64, Net::SocketServer *> ServerMap;
		typedef std::unordered_map<u64, Net::SocketClient *> ClientMap;
		typedef std::unordered_map<u64, Net::SocketServer *>::iterator ServerMapIter;
		typedef std::unordered_map<u64, Net::SocketClient *>::iterator ClientMapIter;
		typedef std::pair<u64, Net::SocketServer *> ServerPair;
		typedef std::pair<u64, Net::SocketClient *> ClientPair;

		Net::EventReactor * reactor_;
		Net::SocketConnector * connector_;
		uv_signal_t * sig_handle_int_;
		uv_signal_t * sig_handle_term_;
		bool is_set_signal_;
		int signal_num_;
		OnSignalFunc signal_func_;
		ServerMap * servers_;
		ClientMap * clients_;
		OnConnectedFunc on_connected_func_;
		OnConnectFailedFunc on_connect_failed_func_;
		OnDisconnectedFunc on_disconnected_func_;
		OnRecvMsgFunc on_recv_msg_func_;
		OnRecvRawMsgFunc on_recv_raw_msg_func_;

		static ApplicationContext * instance_;
	};
}

#endif