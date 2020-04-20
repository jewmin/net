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

#include "Common/Logger.h"

void WriteMessage(const i8 * msg, i32 length) {
	write(STDERR_FILENO, msg, length);
}

void DefaultAbort() {
	abort();
}

static log_message_writer jc_logger = WriteMessage;
static log_abort jc_abort = DefaultAbort;

bool jc_replace_logger(log_message_writer log_func) {
	if (nullptr == log_func) {
		return false;
	}

	jc_logger = log_func;
	return true;
}

bool jc_replace_abort(log_abort abort_func) {
	if (nullptr == abort_func) {
		return false;
	}

	jc_abort = abort_func;
	return true;
}

namespace Net {

class Logger {
public:
	bool Add(const LogItem & item);
	bool AddStr(const i8 * str, i32 num);
	bool AddNum(u64 num, i32 base);

public:
	static const i32 kBufSize = 200;

	i8 * p_;
	i8 * end_;
	i8 buf_[kBufSize];
};

bool Logger::Add(const LogItem & item) {
	if (p_ < end_) {
		*p_++ = ' ';
	}

	switch (item.tag_) {
		case LogItem::kStr:
			return AddStr(item.data_.str, static_cast<i32>(std::strlen(item.data_.str)));

		case LogItem::kSigned:
			if (item.data_.snum < 0) {
				return AddStr("-", 1) && AddNum(static_cast<u64>(-item.data_.snum), 10);
			} else {
				return AddNum(static_cast<u64>(item.data_.snum), 10);
			}

		case LogItem::kUnsigned:
			return AddNum(item.data_.unum, 10);

		case LogItem::kPtr:
			return AddStr("0x", 2) && AddNum(reinterpret_cast<uintptr_t>(item.data_.ptr), 16);
		
		default:
			return false;
	}
}

bool Logger::AddStr(const i8 * str, i32 num) {
	if (end_ - p_ < num) {
		num = static_cast<i32>(end_ - p_);
	}
	if (num <= 0) {
		return false;
	} else {
		std::memcpy(p_, str, num);
		p_ += num;
		return true;
	}
}

bool Logger::AddNum(u64 num, i32 base) {
	static const i8 kDigits[] = "0123456789abcdef";
	i8 space[22];
	i8 * end = space + sizeof(space);
	i8 * pos = end;
	do {
		--pos;
		*pos = kDigits[num % base];
		num /= base;
	} while (num > 0 && pos > space);
	return AddStr(pos, static_cast<i32>(end - pos));
}

void Log(LogMode mode, const i8 * filename, i32 line, LogItem a, LogItem b, LogItem c, LogItem d) {
	Logger state;
	state.p_ = state.buf_;
	state.end_ = state.buf_ + sizeof(state.buf_);
	state.AddStr("[", 1)
		&& state.AddStr(filename, static_cast<i32>(std::strlen(filename)))
		&& state.AddStr(":", 1)
		&& state.AddNum(line, 10)
		&& state.AddStr("]", 1)
		&& state.Add(a)
		&& state.Add(b)
		&& state.Add(c)
		&& state.Add(d);

	if (state.p_ >= state.end_) {
		state.p_ = state.end_ - 1;
	}
	*state.p_++ = '\n';

	jc_logger(state.buf_, static_cast<i32>(state.p_ - state.buf_));
	if (kCrash == mode) {
		jc_abort();
	}
}

}