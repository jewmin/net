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

#include "ApplicationContext.h"
#include "Logger.h"
#include "Packet.hpp"

#pragma pack(1)
struct tagMsgHeader {
	u8 tag_begin;
	u16 crc_data;
	u8 data;
	u8 tag_end;
};
#pragma pack()

#define HEADER_BEGIN 0xbf
#define HEADER_END 0xef
#define PRIVATE_MAKE_CRC_DATA(x, y, z) (((x) << 8 | (y)) | (z))
#define MAKE_CRC_DATA(x, y, z) PRIVATE_MAKE_CRC_DATA(x, y, z)

Interface::ApplicationContext * Interface::ApplicationContext::instance_ = nullptr;

Interface::ApplicationContext::ApplicationContext(OnUpdateFunc onUpdate, OnSignalFunc onSignal, OnLogFunc onLog)
	: reactor_(new Net::EventReactor()), connector_(new Net::SocketConnector(reactor_))
	, sig_handle_int_(new uv_signal_t()), sig_handle_term_(new uv_signal_t())
	, is_set_signal_(false), signal_num_(0), signal_func_(onSignal), update_func_(onUpdate)
	, servers_(new ServerMap()), clients_(new ClientMap())
	, on_connected_func_(nullptr), on_connect_failed_func_(nullptr), on_disconnected_func_(nullptr)
	, on_recv_msg_func_(nullptr), on_recv_raw_msg_func_(nullptr) {
	Foundation::SetLogFunc(onLog);
	SetupSignalHandler(sig_handle_int_, SIGINT);
	SetupSignalHandler(sig_handle_term_, SIGTERM);
}

Interface::ApplicationContext::~ApplicationContext() {
	for (auto & it: *servers_) {
		delete it.second;
	}
	delete servers_;
	for (auto & it: *clients_) {
		delete it.second;
	}
	delete clients_;
	ReleaseSignalHandler(sig_handle_int_);
	ReleaseSignalHandler(sig_handle_term_);
	connector_->Destroy();
	delete reactor_;
}

void Interface::ApplicationContext::RunOnce() {
	if (is_set_signal_) {
		Foundation::LogInfo("处理关服信号[%d]", signal_num_);
		DispatchSignal(signal_num_);
	}
	reactor_->Dispatch();
	OnUpdate();
}

u64 Interface::ApplicationContext::CreateServer(const char * name, int maxOutBufferSize, int maxInBufferSize) {
	Net::SocketServer * server = new Net::SocketServer(name, reactor_, maxOutBufferSize, maxInBufferSize);
	server->SetEvent(this);
	servers_->insert(ServerPair(reinterpret_cast<u64>(server), server));
	return reinterpret_cast<u64>(server);
}

bool Interface::ApplicationContext::ServerListen(u64 serverId, const char * address, int port) {
	ServerMapIter it = servers_->find(serverId);
	if (it != servers_->end()) {
		Foundation::LogInfo("监听服务[%s:%s:%d]", it->second->GetName().c_str(), address, port);
		return it->second->Listen(address, port);
	}
	return false;
}

bool Interface::ApplicationContext::EndServer(u64 serverId) {
	ServerMapIter it = servers_->find(serverId);
	if (it != servers_->end()) {
		Foundation::LogInfo("终止服务[%s]", it->second->GetName().c_str());
		return it->second->Terminate();
	}
	return false;
}

void Interface::ApplicationContext::DeleteServer(u64 serverId) {
	ServerMapIter it = servers_->find(serverId);
	if (it != servers_->end()) {
		delete it->second;
		servers_->erase(it);
	}
}

u64 Interface::ApplicationContext::CreateClient(const char * name, int maxOutBufferSize, int maxInBufferSize) {
	Net::SocketClient * client = new Net::SocketClient(name, reactor_, connector_, maxOutBufferSize, maxInBufferSize);
	client->SetEvent(this);
	clients_->insert(ClientPair(reinterpret_cast<u64>(client), client));
	return reinterpret_cast<u64>(client);
}

u32 Interface::ApplicationContext::ClientConnect(u64 clientId, const char * address, int port) {
	u32 id = 0;
	ClientMapIter it = clients_->find(clientId);
	if (it != clients_->end()) {
		Foundation::LogInfo("连接服务[%s:%s:%d]", it->second->GetName().c_str(), address, port);
		it->second->Connect(address, port, id);
	}
	return id;
}

void Interface::ApplicationContext::DeleteClient(u64 clientId) {
	ClientMapIter it = clients_->find(clientId);
	if (it != clients_->end()) {
		delete it->second;
		clients_->erase(it);
	}
}

void Interface::ApplicationContext::ShutdownAllConnection(u64 mgrId) {
	ServerMapIter it = servers_->find(mgrId);
	if (it != servers_->end()) {
		it->second->ShutDownAllSocketWrappers();
	} else {
		ClientMapIter it2 = clients_->find(mgrId);
		if (it2 != clients_->end()) {
			it2->second->ShutDownAllSocketWrappers();
		}
	}
}

void Interface::ApplicationContext::ShutdownConnection(u64 mgrId, u32 id) {
	ServerMapIter it = servers_->find(mgrId);
	if (it != servers_->end()) {
		it->second->ShutDownOneSocketWrapper(id);
	} else {
		ClientMapIter it2 = clients_->find(mgrId);
		if (it2 != clients_->end()) {
			it2->second->ShutDownOneSocketWrapper(id);
		}
	}
}

void Interface::ApplicationContext::ShutdownConnectionNow(u64 mgrId, u32 id) {
	ServerMapIter it = servers_->find(mgrId);
	if (it != servers_->end()) {
		Net::SocketWrapper * wrapper = it->second->GetSocketWrapper(id);
		if (wrapper) {
			wrapper->ShutdownNow();
		}
	} else {
		ClientMapIter it2 = clients_->find(mgrId);
		if (it2 != clients_->end()) {
			Net::SocketWrapper * wrapper = it2->second->GetSocketWrapper(id);
			if (wrapper) {
				wrapper->ShutdownNow();
			}
		}
	}
}

void Interface::ApplicationContext::SendMsg(Net::SocketWrapper * wrapper, int msgId, const char * data, int size) {
#define _MAKE_HEADER_BYTES(bytes, header) do {				\
	if (header < 0x100) {									\
		bytes = 1;											\
	} else if (header < 0x10000) {							\
		bytes = 2;											\
	} else {												\
		bytes = 4;											\
	}														\
} while (/*CONSTCOND*/ 0)

#define _WRITE_DATA_LENGTH(packet, header, length) do {		\
	switch (header) {										\
		case 1:												\
			packet.WriteAtom<u8>(length);					\
			break;											\
		case 2:												\
			packet.WriteAtom<u16>(length);					\
			break;											\
		case 4:												\
			packet.WriteAtom<int>(length);					\
			break;											\
	}														\
} while (/*CONSTCOND*/ 0)

	static struct tagMsgHeader header;
	u8 msg_id_header = 0;
	u8 data_header = 0;
	_MAKE_HEADER_BYTES(msg_id_header, msgId);
	_MAKE_HEADER_BYTES(data_header, size);
	// 头赋值，简单计算crc，以后替换
	header.data = msg_id_header << 4 | data_header;
	header.tag_begin = HEADER_BEGIN;
	header.tag_end = HEADER_END;
	header.crc_data = MAKE_CRC_DATA(header.tag_begin, header.tag_end, size);
	// 使用Packet序列化输入
	Foundation::Packet packet;
	packet.Reserve(size + sizeof(header) + 8);
	packet.WriteBinary(reinterpret_cast<const char *>(&header), sizeof(header));
	_WRITE_DATA_LENGTH(packet, msg_id_header, msgId);
	_WRITE_DATA_LENGTH(packet, data_header, size);
	packet.WriteBinary(data, size);
	wrapper->Write(packet.GetMemoryPtr(), packet.GetDataLength());
}

void Interface::ApplicationContext::SendMsg(u64 mgrId, u32 id, int msgId, const char * data, int size) {
	ServerMapIter it = servers_->find(mgrId);
	if (it != servers_->end()) {
		Net::SocketWrapper * wrapper = it->second->GetSocketWrapper(id);
		if (wrapper) {
			SendMsg(wrapper, msgId, data, size);
		}
	} else {
		ClientMapIter it2 = clients_->find(mgrId);
		if (it2 != clients_->end()) {
			Net::SocketWrapper * wrapper = it2->second->GetSocketWrapper(id);
			if (wrapper) {
				SendMsg(wrapper, msgId, data, size);
			}
		}
	}
}

void Interface::ApplicationContext::SendRawMsg(u64 mgrId, u32 id, const char * data, int size) {
	ServerMapIter it = servers_->find(mgrId);
	if (it != servers_->end()) {
		Net::SocketWrapper * wrapper = it->second->GetSocketWrapper(id);
		if (wrapper) {
			wrapper->Write(data, size);
		}
	} else {
		ClientMapIter it2 = clients_->find(mgrId);
		if (it2 != clients_->end()) {
			Net::SocketWrapper * wrapper = it2->second->GetSocketWrapper(id);
			if (wrapper) {
				wrapper->Write(data, size);
			}
		}
	}
}

void Interface::ApplicationContext::SetRawRecv(u64 mgrId, u32 id, bool isRaw) {
	ServerMapIter it = servers_->find(mgrId);
	if (it != servers_->end()) {
		Net::SocketWrapper * wrapper = it->second->GetSocketWrapper(id);
		if (wrapper) {
			wrapper->SetRawRecv(isRaw);
		}
	} else {
		ClientMapIter it2 = clients_->find(mgrId);
		if (it2 != clients_->end()) {
			Net::SocketWrapper * wrapper = it2->second->GetSocketWrapper(id);
			if (wrapper) {
				wrapper->SetRawRecv(isRaw);
			}
		}
	}
}

address_t Interface::ApplicationContext::GetOneConnectionRemoteAddress(u64 mgrId, u32 id) {
	address_t address;
	std::memset(&address, 0, sizeof(address));
	ServerMapIter it = servers_->find(mgrId);
	if (it != servers_->end()) {
		Net::SocketWrapper * wrapper = it->second->GetSocketWrapper(id);
		if (wrapper) {
			Net::SocketAddress addr = wrapper->GetConnection()->GetSocket()->RemoteAddress();
			std::strncpy(address.address, addr.Host().ToString().c_str(), sizeof(address.address));
			address.port = addr.Port();
		}
	} else {
		ClientMapIter it2 = clients_->find(mgrId);
		if (it2 != clients_->end()) {
			Net::SocketWrapper * wrapper = it2->second->GetSocketWrapper(id);
			if (wrapper) {
				Net::SocketAddress addr = wrapper->GetConnection()->GetSocket()->RemoteAddress();
				std::strncpy(address.address, addr.Host().ToString().c_str(), sizeof(address.address));
				address.port = addr.Port();
			}
		}
	}
	return address;
}

void Interface::ApplicationContext::SetCallback(OnConnectedFunc onConnected, OnConnectFailedFunc onConnectFailed, OnDisconnectedFunc onDisconnected, OnRecvMsgFunc onRecvMsg, OnRecvRawMsgFunc onRecvRawMsg) {
	on_connected_func_ = onConnected;
	on_connect_failed_func_ = onConnectFailed;
	on_disconnected_func_ = onDisconnected;
	on_recv_msg_func_ = onRecvMsg;
	on_recv_raw_msg_func_ = onRecvRawMsg;
}

Interface::ApplicationContext * Interface::ApplicationContext::GetInstance() {
	return instance_;
}

Interface::ApplicationContext * Interface::ApplicationContext::CreateInstance(OnUpdateFunc onUpdate, OnSignalFunc onSignal, OnLogFunc onLog) {
	if (!instance_) {
		instance_ = new ApplicationContext(onUpdate, onSignal, onLog);
	}
	return instance_;
}

void Interface::ApplicationContext::ReleaseInstance() {
	if (instance_) {
		delete instance_;
		instance_ = nullptr;
	}
}

void Interface::ApplicationContext::SetupSignalHandler(uv_signal_t * handle, int signum) {
	uv_signal_init(reactor_->GetEventLoop(), handle);
	uv_signal_start(handle, SignalCb, signum);
	// uv_handle_set_data(reinterpret_cast<uv_handle_t *>(handle), this);
}

void Interface::ApplicationContext::ReleaseSignalHandler(uv_signal_t * handle) {
	if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(handle))) {
		uv_close(reinterpret_cast<uv_handle_t *>(handle), CloseCb);
	}
}

void Interface::ApplicationContext::SetSignal(int signum) {
	is_set_signal_ = true;
	signal_num_ = signum;
}

void Interface::ApplicationContext::DispatchSignal(int signum) {
	is_set_signal_ = false;
	OnSignal(signum);
}

void Interface::ApplicationContext::OnSignal(int signum) {
	signal_func_(signum);
}

void Interface::ApplicationContext::OnUpdate() {
	for (auto & it: *servers_) {
		it.second->Update();
	}
	for (auto & it: *clients_) {
		it.second->Update();
	}
	if (update_func_) {
		update_func_();
	}
}

int Interface::ApplicationContext::OnConnected(Net::SocketWrapper * wrapper) {
	Foundation::LogDebug("一个Connection[%u]连上了", wrapper->GetId());
	if (on_connected_func_) {
		on_connected_func_(reinterpret_cast<u64>(wrapper->GetMgr()), wrapper->GetId());
	}
	return 0;
}

int Interface::ApplicationContext::OnConnectFailed(Net::SocketWrapper * wrapper, int reason) {
	if (on_connect_failed_func_) {
		on_connect_failed_func_(reinterpret_cast<u64>(wrapper->GetMgr()), wrapper->GetId(), reason);
	}
	return 0;
}

int Interface::ApplicationContext::OnDisconnected(Net::SocketWrapper * wrapper, bool isRemote) {
	Foundation::LogDebug("一个Connection[%u]断开了", wrapper->GetId());
	if (on_disconnected_func_) {
		on_disconnected_func_(reinterpret_cast<u64>(wrapper->GetMgr()), wrapper->GetId(), isRemote);
	}
	return 0;
}

int Interface::ApplicationContext::OnNewDataReceived(Net::SocketWrapper * wrapper) {
#define _READ_DATA_LENGTH(packet, header, length) do {		\
	switch (header) {										\
		case 1:												\
			length = packet.ReadAtom<u8>();					\
			break;											\
		case 2:												\
			length = packet.ReadAtom<u16>();				\
			break;											\
		case 4:												\
			length = packet.ReadAtom<int>();				\
			break;											\
		default:											\
			return 1;										\
	}														\
} while (/*CONSTCOND*/ 0)

	if (wrapper->IsRawRecv()) {
		if (on_recv_raw_msg_func_) {
			on_recv_raw_msg_func_(reinterpret_cast<u64>(wrapper->GetMgr()), wrapper->GetId(), wrapper->GetRecvData(), wrapper->GetRecvDataSize());
		}
		wrapper->PopRecvData(wrapper->GetRecvDataSize());
	} else {
		static struct tagMsgHeader header;
		while (wrapper->GetRecvDataSize() >= sizeof(header)) {
			Foundation::PacketReader reader(wrapper->GetRecvData(), wrapper->GetRecvDataSize());
			reader.ReadBinary(reinterpret_cast<char *>(&header), sizeof(header));
			if (HEADER_BEGIN != header.tag_begin || HEADER_END != header.tag_end) {
				return 1;
			}
			u8 msg_id_header = header.data >> 4;
			u8 data_header = header.data & 0xf;
			if (reader.GetReadableLength() < msg_id_header + data_header) {
				break;
			}
			int msgId, size;
			_READ_DATA_LENGTH(reader, msg_id_header, msgId);
			_READ_DATA_LENGTH(reader, data_header, size);
			if (MAKE_CRC_DATA(header.tag_begin, header.tag_end, size) != header.crc_data) {
				return 1;
			}
			if (reader.GetReadableLength() < size) {
				break;
			}
			if (on_recv_msg_func_) {
				on_recv_msg_func_(reinterpret_cast<u64>(wrapper->GetMgr()), wrapper->GetId(), msgId, reader.GetOffsetPtr(), size);
			}
			wrapper->PopRecvData(reader.GetPosition() + size);
		}
	}
	return 0;
}

int Interface::ApplicationContext::OnSomeDataSent(Net::SocketWrapper * wrapper) {
	return 0;
}

void Interface::ApplicationContext::SignalCb(uv_signal_t * handle, int signum) {
	std::string msg;
	switch (signum) {
		case SIGINT:
			msg = "Received SIGINT scheduling shutdown...";
			break;

		case SIGTERM:
			msg = "Received SIGTERM scheduling shutdown...";
			break;

		default:
			msg = "Received shutdown signal, scheduling shutdown...";
			break;
	}

	Foundation::LogWarn(msg.c_str());
	GetInstance()->SetSignal(signum);
}

void Interface::ApplicationContext::CloseCb(uv_handle_t * handle) {
	delete handle;
}