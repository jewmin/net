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

#ifndef Net_Sockets_UvData_INCLUDED
#define Net_Sockets_UvData_INCLUDED

#include "Common.h"
#include "RefCountedObject.h"

namespace Net {

class SocketImpl;
class COMMON_EXTERN UvData : public Common::StrongRefObject {
	friend class SocketImpl;

public:
	virtual ~UvData() {}

protected:
	// 套接字回调函数
	virtual void CloseCallback() {}
	virtual void AcceptCallback(i32 status) {}
	virtual void ConnectCallback(i32 status, void * arg) {}
	virtual void ShutdownCallback(i32 status, void * arg) {}
	virtual void AllocCallback(uv_buf_t * buf) {}
	virtual void ReadCallback(i32 status) {}
	virtual void WrittenCallback(i32 status, void * arg) {}

protected:
	UvData() {}
};

}

#endif