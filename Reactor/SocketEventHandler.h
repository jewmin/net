#include "libuv/include/uv.h"
#include "RefCountedObject.h"
#include "SocketAddress.h"

class EventReactor;
class EventHandler : public Foundation::RefCountedObject {
public:
	EventHandler(EventReactor * event_reactor) {
		event_reactor_ = event_reactor;
	}

	virtual ~EventHandler() {
		event_reactor_ = nullptr;
	}

	virtual bool RegisterToReactor() = 0;
	
	virtual bool UnRegisterFromReactor() = 0;

	inline EventReactor * GetReactor() const {
		return event_reactor_;
	}

	inline void SetReactor(EventReactor * event_reactor) {
		event_reactor_ = event_reactor;
	}

private:
	EventReactor * event_reactor_;
};

class EventReactor {
public:
	EventReactor() {
		event_loop_ = static_cast<uv_loop_t *>(malloc(sizeof(uv_loop_t)));
		int err = uv_loop_init(event_loop_);
		if (err < 0) {
			LogErr("uv_loop_init - %s", uv_strerror(err));
			free(event_loop_);
			event_loop_ = nullptr;
		}
	}

	~EventReactor() {
		while (!event_handlers_.empty()) {
			RemoveEventHandler(event_handlers_.front());
		}
		while (Dispatch()) {
			Dispatch(UV_RUN_ONCE);
		}
		if (event_loop_) {
			uv_loop_close(event_loop_);
			free(event_loop_);
			event_loop_ = nullptr;
		}
	}

	bool AddEventHandler(EventHandler * handler) {
		bool ok = handler->RegisterToReactor();
		if (ok) {
			event_handlers_.push_back(handler);
		}
		return ok;
	}

	bool RemoveEventHandler(EventHandler * handler) {
		bool ok = handler->UnRegisterFromReactor();
		if (ok) {
			event_handlers_.remove(handler);
		}
		return ok;
	}

	inline bool Dispatch(uv_run_mode mode = UV_RUN_NOWAIT) {
		return event_loop_ && uv_run(event_loop_, mode) > 0;
	}

	inline uv_loop_t * GetEventLoop() const {
		return event_loop_;
	}

private:
	uv_loop_t * event_loop_;
	std::list<EventHandler *> event_handlers_;
};

class TcpSocketEventHandler : public EventHandler {
public:
	TcpSocketEventHandler(EventReactor * event_reactor) : EventHandler(event_reactor) {
		handle_ = nullptr;
	}

	virtual ~TcpSocketEventHandler() {
		handle_ = nullptr;
	}

	virtual bool RegisterToReactor() {
		if (!handle_) {
			handle_ = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
			int err = uv_tcp_init(GetReactor()->GetEventLoop(), GetTcp());
			if (0 == err) {
				Duplicate();
				handle_->data = this;
				return true;
			}
			LogErr("RegisterToReactor uv_tcp_init - %s", uv_strerror(err));
			free(handle_);
			handle_ = nullptr;
		}
		return false;
	}
	
	virtual bool UnRegisterFromReactor() {
		if (handle_) {
			if (!uv_is_closing(GetHandle())) {
				uv_close(GetHandle(), CloseCb);
			}
			handle_ = nullptr;
		}
		return true;
	}

	inline uv_handle_t * GetHandle() const {
		return reinterpret_cast<uv_handle_t *>(handle_);
	}

	inline uv_stream_t * GetStream() const {
		return reinterpret_cast<uv_stream_t *>(handle_);
	}

	inline uv_tcp_t * GetTcp() const {
		return handle_;
	}

private:
	static void CloseCb(uv_handle_t * handle) {
		TcpSocketEventHandler * event_handler = static_cast<TcpSocketEventHandler *>(handle->data);
		if (event_handler) {
			event_handler->Release();
		}
		free(handle);
	}

protected:
	uv_tcp_t * handle_;
};

class TcpSocketConnection : public TcpSocketEventHandler {
public:
	TcpSocketConnection() : TcpSocketEventHandler(nullptr) {
	}

	virtual ~TcpSocketConnection() {
	}

	bool Open() {
		return GetReactor()->AddEventHandler(this);
	}

	virtual void OnConnected() = 0;
};

class TcpSocketAcceptor : public TcpSocketEventHandler {
public:
	TcpSocketAcceptor(EventReactor * event_reactor) : TcpSocketEventHandler(event_reactor) {
		opened_ = false;
	}

	virtual ~TcpSocketAcceptor() {
		opened_ = false;
	}

	bool Open(const Net::SocketAddress & address, int backlog = 128, bool ipV6Only = false) {
		if (!opened_) {
			if (GetReactor()->AddEventHandler(this)) {
				int flags = 0;
				if (ipV6Only) {
					flags |= UV_TCP_IPV6ONLY;
				}
				int err = uv_tcp_bind(GetTcp(), address.Addr(), flags);
				if (0 == err) {
					err = uv_listen(GetStream(), backlog, AcceptCb);
					if (0 == err) {
						LogInfo("listen - %s", address.ToString().c_str());
						opened_ = true;
						return true;
					}
				}
				LogErr("uv_tcp_bind | uv_listen - %s", uv_strerror(err));
				GetReactor()->RemoveEventHandler(this);
			}
		}
		return false;
	}

	void Close() {
		if (opened_) {
			opened_ = false;
			GetReactor()->RemoveEventHandler(this);
		}
	}

protected:
	virtual void MakeNewConnection(TcpSocketConnection * & connection) = 0;

	virtual void OnAccepted(TcpSocketConnection * connection) = 0;

	void AcceptNewConnection(TcpSocketConnection * connection) {
		uv_tcp_t * handle = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
		int err = uv_tcp_init(GetReactor()->GetEventLoop(), handle);
		if (err < 0) {
			LogErr("Accept uv_tcp_init - %s", uv_strerror(err));
			free(handle);
			handle = nullptr;
			return;
		}
		err = uv_accept(GetStream(), reinterpret_cast<uv_stream_t *>(handle));
		if (err < 0) {
			LogErr("Accept uv_accept - %s", uv_strerror(err));
			uv_close(reinterpret_cast<uv_handle_t *>(handle), reinterpret_cast<uv_close_cb>(free));
			return;
		}
		connection->
	}

	bool ActivateConnection(TcpSocketConnection * connection) {
		connection->SetReactor(GetReactor());
		if (connection->Open()) {

		}
	}

	void Accept() {
		TcpSocketConnection * connection = nullptr;
		MakeNewConnection(connection);
		AcceptNewConnection(connection);
		if (ActivateConnection(connection)) {
			OnAccepted(connection);
			connection->OnConnected();
		}
	}

private:
	static void AcceptCb(uv_stream_t * server, int status) {
		TcpSocketAcceptor * acceptor = dynamic_cast<TcpSocketAcceptor *>(static_cast<TcpSocketEventHandler *>(server->data));
		if (acceptor) {
			if (status < 0) {
				LogErr("accept - %s", uv_strerror(status));
				acceptor->Close();
			} else {
				acceptor->Accept();
			}
		}
	}

private:
	bool opened_;
};