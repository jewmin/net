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

#include "Logger.h"

Foundation::LogFunc g_logger = nullptr;

void Foundation::SetLogFunc(LogFunc func) {
	g_logger = func;
}

void Foundation::Log(FILE * stream, LogLevel level, const char * fmt, va_list ap) {
	static char msg[1024];
	static char time_string[32];
	std::vsnprintf(msg, sizeof(msg), fmt, ap);
	if (g_logger) {
		g_logger(level, msg);
	} else {
		static const char * labels[] = { "debug", "info", "warn", "error" };
		std::time_t raw_time = std::time(nullptr);
		std::strftime(time_string, sizeof(time_string), "%F %T", std::localtime(&raw_time));
		std::fprintf(stream, "%s [%s] %s\n", time_string, labels[level], msg);
	}
}

void Foundation::LogDebug(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Log(stdout, kDebug, fmt, ap);
	va_end(ap);
}

void Foundation::LogInfo(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Log(stdout, kInfo, fmt, ap);
	va_end(ap);
}

void Foundation::LogWarn(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Log(stderr, kWarning, fmt, ap);
	va_end(ap);
}

void Foundation::LogErr(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Log(stderr, kError, fmt, ap);
	va_end(ap);
}