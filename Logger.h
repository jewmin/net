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

#ifndef Foundation_Logger_INCLUDED
#define Foundation_Logger_INCLUDED

#include "CommonDef.h"

namespace Foundation {
	enum LogLevel { kDebug = 0, kInfo = 1, kWarning = 2, kError = 3 };
	typedef void (*LogFunc)(int level, const char * msg);
	void SetLogFunc(LogFunc func);
	void Log(FILE * stream, LogLevel level, const char * fmt, va_list ap);
	void LogDebug(const char * fmt, ...);
	void LogInfo(const char * fmt, ...);
	void LogWarn(const char * fmt, ...);
	void LogErr(const char * fmt, ...);
}

#endif