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

#ifndef Interface_Interface_INCLUDED
#define Interface_Interface_INCLUDED

#include "CommonDef.h"

// Interface Function
typedef void(*OnSignalFunc)(int signum);
typedef void (*OnLogFunc)(int level, const char * msg);

void Init(OnSignalFunc onSignal, OnLogFunc onLog);
void Unit();
void Loop();

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

int SendMsg(u64 mgrId, u32 id, int msgId, const char * data, int size);
int SendRawMsg(u64 mgrId, u32 id, const char * data, int size);
void SetRawRecv(u64 mgrId, u32 id, bool isRaw);

// Callback Function
typedef void(*OnConnectedFunc)(u64 mgrId, u32 id);
typedef void(*OnConnectFailedFunc)(u64 mgrId, u32 id, int reason);
typedef void(*OnDisconnectedFunc)(u64 mgrId, u32 id, bool isRemote);
typedef void(*OnRecvMsgFunc)(u64 mgrId, u32 id, int msgId, const char * data, int size);
typedef void(*OnRecvRawMsgFunc)(u64 mgrId, u32 id, const char * data, int size);
void SetCallback(OnConnectedFunc onConnected, OnConnectFailedFunc onConnectFailed, OnDisconnectedFunc onDisconnected, OnRecvMsgFunc onRecvMsg, OnRecvRawMsgFunc onRecvRawMsg);

#endif