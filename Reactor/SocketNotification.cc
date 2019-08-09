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

#include "SocketNotification.h"

//*********************************************************************
//SocketNotification
//*********************************************************************

Net::SocketNotification::SocketNotification()
	: type_(kNone) {
}

Net::SocketNotification::SocketNotification(NotificationType t)
	: type_(t) {
}

Net::SocketNotification::~SocketNotification() {
}

//*********************************************************************
//ReadableNotification
//*********************************************************************

Net::ReadableNotification::ReadableNotification()
	: SocketNotification(kRead) {
}

Net::ReadableNotification::~ReadableNotification() {
}

//*********************************************************************
//WritableNotification
//*********************************************************************

Net::WritableNotification::WritableNotification()
	: SocketNotification(kWrite) {
}

Net::WritableNotification::~WritableNotification() {
}

//*********************************************************************
//ErrorNotification
//*********************************************************************

Net::ErrorNotification::ErrorNotification()
	: SocketNotification(kError) {
}

Net::ErrorNotification::~ErrorNotification() {
}

//*********************************************************************
//TimeoutNotification
//*********************************************************************

Net::TimeoutNotification::TimeoutNotification()
	: SocketNotification(kTimeout) {
}

Net::TimeoutNotification::~TimeoutNotification() {
}

//*********************************************************************
//ShutdownNotification
//*********************************************************************

Net::ShutdownNotification::ShutdownNotification()
	: SocketNotification(kShutdown) {
}

Net::ShutdownNotification::~ShutdownNotification() {
}