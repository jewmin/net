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

#ifndef Net_Common_Logger_INCLUDED
#define Net_Common_Logger_INCLUDED

#include "Net.h"

namespace Net {

enum LogMode { kLog, kCrash };

class Logger;
class NET_EXTERN LogItem {
	friend class Logger;
	enum Tag { kStr, kPtr, kSigned, kUnsigned, kEnd };

public:
	LogItem()							: tag_(kEnd)		{}
	LogItem(const i8 * value)			: tag_(kStr)		{ data_.str = value; }
	LogItem(int value)					: tag_(kSigned)		{ data_.snum = value; }
	LogItem(long value)					: tag_(kSigned)		{ data_.snum = value; }
	LogItem(long long value)			: tag_(kSigned)		{ data_.snum = value; }
	LogItem(unsigned int value)			: tag_(kUnsigned)	{ data_.unum = value; }
	LogItem(unsigned long value)		: tag_(kUnsigned)	{ data_.unum = value; }
	LogItem(unsigned long long value)	: tag_(kUnsigned)	{ data_.unum = value; }
	LogItem(const void * value)			: tag_(kPtr)		{ data_.ptr = value; }

private:
	Tag tag_;
	union {
		const i8 * str;
		const void * ptr;
		i64 snum;
		u64 unum;
	} data_;
};

NET_EXTERN void Log(LogMode mode, const i8 * filename, i32 line, LogItem a, LogItem b = LogItem(), LogItem c = LogItem(), LogItem d = LogItem());

}

#endif