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

#include "Net.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
	/* Windows - set up dll import/export decorators. */
#	if defined(BUILDING_NET_SHARED)
		/* Building shared library. */
#		define NET_EXTERN __declspec(dllexport)
#	elif defined(USING_NET_SHARED)
		/* Using shared library. */
#		define NET_EXTERN __declspec(dllimport)
#	else
		/* Building static library. */
#		define NET_EXTERN /* nothing */
#	endif
#elif __GNUC__ >= 4
#	define NET_EXTERN __attribute__((visibility("default")))
#else
#	define NET_EXTERN /* nothing */
#endif

// Interface Struct
#pragma pack(1)
typedef struct {
	char address[48];
	u16 port;
} address_t;
#pragma pack()

// Interface Function
typedef void(*OnUpdateFunc)();
typedef void(*OnSignalFunc)(int signum);
// level: debug=0 info=1 warn=2 error=3
typedef void(*OnLogFunc)(int level, const char * msg);

NET_EXTERN void Init(OnUpdateFunc onUpdate, OnSignalFunc onSignal, OnLogFunc onLog);
NET_EXTERN void Unit();
NET_EXTERN void Loop();

NET_EXTERN u64 CreateServer(const char * name, int maxOutBufferSize, int maxInBufferSize);
NET_EXTERN bool ServerListen(u64 serverId, const char * address, int port);
NET_EXTERN bool EndServer(u64 serverId);
NET_EXTERN void DeleteServer(u64 serverId);

NET_EXTERN u64 CreateClient(const char * name, int maxOutBufferSize, int maxInBufferSize);
NET_EXTERN u32 ClientConnect(u64 clientId, const char * address, int port);
NET_EXTERN void DeleteClient(u64 clientId);

NET_EXTERN void ShutdownAllConnection(u64 mgrId);
NET_EXTERN void ShutdownConnection(u64 mgrId, u32 id);
NET_EXTERN void ShutdownConnectionNow(u64 mgrId, u32 id);

NET_EXTERN void SendMsg(u64 mgrId, u32 id, int msgId, const char * data, int size);
NET_EXTERN void SendRawMsg(u64 mgrId, u32 id, const char * data, int size);
NET_EXTERN void SetRawRecv(u64 mgrId, u32 id, bool isRaw);

NET_EXTERN address_t GetOneConnectionRemoteAddress(u64 mgrId, u32 id);

// Callback Function
typedef void(*OnConnectedFunc)(u64 mgrId, u32 id);
typedef void(*OnConnectFailedFunc)(u64 mgrId, u32 id, int reason);
typedef void(*OnDisconnectedFunc)(u64 mgrId, u32 id, bool isRemote);
typedef void(*OnRecvMsgFunc)(u64 mgrId, u32 id, int msgId, const char * data, int size);
typedef void(*OnRecvRawMsgFunc)(u64 mgrId, u32 id, const char * data, int size);
NET_EXTERN void SetCallback(OnConnectedFunc onConnected, OnConnectFailedFunc onConnectFailed, OnDisconnectedFunc onDisconnected, OnRecvMsgFunc onRecvMsg, OnRecvRawMsgFunc onRecvRawMsg);

#ifdef __cplusplus
}
#endif

#endif
