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

#ifndef Net_Common_StraightBuffer_INCLUDED
#define Net_Common_StraightBuffer_INCLUDED

#include "Common/IOBuffer.h"

namespace Net {

class NET_EXTERN StraightBuffer : public IOBuffer {
public:
	StraightBuffer();
	virtual ~StraightBuffer();

	virtual i8 * GetReserveBlock(i32 want_size, i32 & actually_size) override;
	virtual i8 * GetContiguousBlock(i32 & size) override;
	virtual void Commit(i32 size) override;
	virtual void DeCommit(i32 size) override;
	virtual i32 GetCommitedSize() const override;
	virtual i32 GetFreeSize() const override;

private:
	i32 offset_;
	i32 size_;
	i32 size_reserve_;
};

}

#endif