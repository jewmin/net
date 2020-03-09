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

#ifndef Net_Reactor_EventHandler_INCLUDED
#define Net_Reactor_EventHandler_INCLUDED

#include "Reactor/EventReactor.h"
#include "Common/RefCountedObject.h"

namespace Net {

class EventHandler : public RefCountedObject {
	friend class EventReactor;

public:
	virtual ~EventHandler();

	virtual void Destroy() = 0;
	EventReactor * GetReactor() const;
	void SetReactor(EventReactor * reactor);

protected:
	explicit EventHandler(EventReactor * reactor);
	virtual bool RegisterToReactor() = 0;
	virtual bool UnRegisterFromReactor() = 0;

private:
	EventHandler() = delete;
	EventHandler(EventHandler &&) = delete;
	EventHandler(const EventHandler &) = delete;
	EventHandler & operator=(EventHandler &&) = delete;
	EventHandler & operator=(const EventHandler &) = delete;

private:
	EventReactor * reactor_;
};

inline EventReactor * EventHandler::GetReactor() const {
	return reactor_;
}

inline void EventHandler::SetReactor(EventReactor * reactor) {
	reactor_ = reactor;
}

}

#endif