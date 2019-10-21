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

#include "Interface.h"
#include "ApplicationContext.h"

void Init(OnSignalFunc onSignal, OnLogFunc onLog) {
	Interface::ApplicationContext::CreateInstance(onSignal, onLog);
}

void Unit() {
	Interface::ApplicationContext::ReleaseInstance();
}

void Loop() {
	Interface::ApplicationContext::GetInstance()->RunOnce();
}

u64 CreateServer(const char * name, int maxOutBufferSize, int maxInBufferSize) {
	return Interface::ApplicationContext::GetInstance()->CreateServer(name, maxOutBufferSize, maxInBufferSize);
}

bool ServerListen(u64 serverId, const char * address, int port) {
	return Interface::ApplicationContext::GetInstance()->ServerListen(serverId, address, port);
}

bool EndServer(u64 serverId) {
	return Interface::ApplicationContext::GetInstance()->EndServer(serverId);
}

void DeleteServer(u64 serverId) {
	Interface::ApplicationContext::GetInstance()->DeleteServer(serverId);
}

u64 CreateClient(const char * name, int maxOutBufferSize, int maxInBufferSize) {
	return Interface::ApplicationContext::GetInstance()->CreateClient(name, maxOutBufferSize, maxInBufferSize);
}

u32 ClientConnect(u64 clientId, const char * address, int port) {
	return Interface::ApplicationContext::GetInstance()->ClientConnect(clientId, address, port);
}

void DeleteClient(u64 clientId) {
	Interface::ApplicationContext::GetInstance()->DeleteClient(clientId);
}

void ShutdownAllConnection(u64 mgrId) {
	Interface::ApplicationContext::GetInstance()->ShutdownAllConnection(mgrId);
}

void ShutdownConnection(u64 mgrId, u32 id) {
	Interface::ApplicationContext::GetInstance()->ShutdownConnection(mgrId, id);
}

void ShutdownConnectionNow(u64 mgrId, u32 id) {
	Interface::ApplicationContext::GetInstance()->ShutdownConnectionNow(mgrId, id);
}

int SendMsg(u64 mgrId, u32 id, int msgId, const char * data, int size) {
	Interface::ApplicationContext::GetInstance()->SendMsg(mgrId, id, msgId, data, size);
}

int SendRawMsg(u64 mgrId, u32 id, const char * data, int size) {
	Interface::ApplicationContext::GetInstance()->SendRawMsg(mgrId, id, data, size);
}

void SetRawRecv(u64 mgrId, u32 id, bool isRaw) {
	Interface::ApplicationContext::GetInstance()->SetRawRecv(mgrId, id, isRaw);
}

void SetCallback(OnConnectedFunc onConnected, OnConnectFailedFunc onConnectFailed, OnDisconnectedFunc onDisconnected, OnRecvMsgFunc onRecvMsg, OnRecvRawMsgFunc onRecvRawMsg) {
	Interface::ApplicationContext::GetInstance()->SetCallback(onConnected, onConnectFailed, onDisconnected, onRecvMsg, onRecvRawMsg);
}