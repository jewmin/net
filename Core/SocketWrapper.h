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

#ifndef Net_SocketWrapper_INCLUDED
#define Net_SocketWrapper_INCLUDED

#include "INotification.h"
#include "Reactor/SocketConnection.h"

namespace Net {

class SocketWrapperMgr;
class SocketWrapper : public INotification {
public:
	SocketWrapper(SocketWrapperMgr * mgr, i32 max_out_buffer_size, i32 max_in_buffer_size);
	virtual ~SocketWrapper();

	void Shutdown();
	void ShutdownNow();
	void NeedToShutdown();
	int Write(const char * data, int len);
	int Read(char * data, int len);
	char * GetRecvData() const;
	int GetRecvDataSize() const;
	void PopRecvData(int size);
	void SetMaxOutBufferSize(int size);
	void SetMaxInBufferSize(int size);
	int GetMaxOutBufferSize() const;
	int GetMaxInBufferSize() const;
	int GetOutBufferUsedSize();
	void SetId(u32 id);
	u32 GetId() const;
	SocketWrapperMgr * GetMgr() const;
	void SetIsRegister2Mgr(bool reg);
	bool GetIsRegister2Mgr() const;
	SocketConnection * GetConnection() const;
	void SetRawRecv(bool is_raw);
	bool IsRawRecv() const;

protected:
	virtual void OnConnected();
	virtual void OnConnectFailed(int reason);
	virtual void OnDisconnected(bool isRemote);
	virtual void OnNewDataReceived();
	virtual void OnSomeDataSent();
	virtual void OnError(int reason);

private:
	u32 id_;
	bool is_raw_recv_;
	bool register_mgr_;
	SocketWrapperMgr * mgr_;
	SocketConnection * connection_;
};

inline void SocketWrapper::SetId(u32 id) {
	id_ = id;
}

inline u32 SocketWrapper::GetId() const {
	return id_;
}

inline SocketWrapperMgr * SocketWrapper::GetMgr() const {
	return mgr_;
}

inline void SocketWrapper::SetIsRegister2Mgr(bool reg) {
	register_mgr_ = reg;
}

inline bool SocketWrapper::GetIsRegister2Mgr() const {
	return register_mgr_;
}

inline void SocketWrapper::SetRawRecv(bool is_raw) {
	is_raw_recv_ = is_raw;
}

inline bool SocketWrapper::IsRawRecv() const {
	return is_raw_recv_;
}

}

#endif