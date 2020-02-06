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

#ifndef Net_EventReactor_INCLUDED
#define Net_EventReactor_INCLUDED

#include "NetObject.h"

namespace Net {

class EventHandler;
class EventReactor : public NetObject {
public:
	EventReactor();
	virtual ~EventReactor();

	bool AddEventHandler(EventHandler * handler);
	bool RemoveEventHandler(EventHandler * handler);
	bool Dispatch(uv_run_mode mode = UV_RUN_NOWAIT);
	uv_loop_t * GetEventLoop() const;

private:
	EventReactor(EventReactor &&) = delete;
	EventReactor(const EventReactor &) = delete;
	EventReactor & operator=(EventReactor &&) = delete;
	EventReactor & operator=(const EventReactor &) = delete;

private:
	uv_loop_t * loop_;
	std::list<EventHandler *> handlers_;
};

inline bool EventReactor::Dispatch(uv_run_mode mode) {
	return uv_run(loop_, mode) > 0;
}

inline uv_loop_t * EventReactor::GetEventLoop() const {
	return loop_;
}

}

#endif