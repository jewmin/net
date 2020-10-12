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

#include "Net.h"
#include "Interface/AppService.h"

void jc_replace_signal(on_signal_func signal_func) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		service->SetSignal(signal_func);
	}
}

void jc_replace_callback(on_connected_func on_connected, on_connect_failed_func on_connect_failed, on_disconnected_func on_disconnected, on_received_func on_received, on_sent_func on_sent) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		service->SetCallback(on_connected, on_connect_failed, on_disconnected, on_received, on_sent);
	}
}

void jc_init() {
	Net::AppService::CreateInstance();
}

void jc_unit() {
	Net::AppService::ReleaseInstance();
}

void jc_poll() {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		service->RunOnce();
	}
}

i64 jc_create_server(const i8 * name, i32 max_out_buffer_size, i32 max_in_buffer_size) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		return service->CreateServer(name, max_out_buffer_size, max_in_buffer_size);
	}
	return -1;
}

bool jc_server_listen(i64 server_id, const i8 * address, i32 port) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		return service->ServerListen(server_id, address, port);
	}
	return false;
}

bool jc_end_server(i64 server_id) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		return service->EndServer(server_id);
	}
	return false;
}

void jc_delete_server(i64 server_id) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		service->DeleteServer(server_id);
	}
}


i64 jc_create_client(const i8 * name, i32 max_out_buffer_size, i32 max_in_buffer_size) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		return service->CreateClient(name, max_out_buffer_size, max_in_buffer_size);
	}
	return -1;
}

i64 jc_client_connect(i64 client_id, const i8 * address, i32 port) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		return service->ClientConnect(client_id, address, port);
	}
	return -1;
}

void jc_delete_client(i64 client_id) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		service->DeleteClient(client_id);
	}
}


i32 jc_send_data(i64 mgr_id, i64 connection_id, const i8 * data, i32 data_len) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		return service->SendData(mgr_id, connection_id, data, data_len);
	}
	return UV_EPERM;
}

void jc_shutdown_all_connections(i64 mgr_id) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		service->ShutdownAllConnections(mgr_id);
	}
}

void jc_shutdown_one_connection(i64 mgr_id, i64 connection_id) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		service->ShutdownConnection(mgr_id, connection_id);
	}
}

void jc_shutdown_one_connection_now(i64 mgr_id, i64 connection_id) {
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		service->ShutdownConnectionNow(mgr_id, connection_id);
	}
}

address_t jc_get_one_connection_remote_address(i64 mgr_id, i64 connection_id) {
	address_t result;
	std::memset(&result, 0, sizeof(result));
	Net::AppService * service = Net::AppService::Get();
	if (service) {
		Net::SocketAddress address = service->GetConnectionRemoteAddress(mgr_id, connection_id);
		std::strncpy(result.ip, address.Host().ToString().c_str(), sizeof(result.ip));
		result.port = address.Port();
	}
	return result;
}