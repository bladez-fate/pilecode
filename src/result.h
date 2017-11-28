// The MIT License(MIT)
//
// Copyright 2017 bladez-fate
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#pragma once

#include "defs.h"

namespace pilecode {

	enum ResultStatus {
		// nonnegative is OK
		kRsOk = 0,
		kRsAlready = 1,
		kRsUndone = 2, // Repeated action is undo

		// negative is FAILED
		kRsFailed = -1,
		kRsForbidden = -2,
		kRsNotFound = -3,
	};

	struct ResultBase {
		ResultStatus status;
		ResultBase(ResultStatus _status)
			: status(_status)
		{}
		bool IsOk() const { return status >= 0; }
		bool IsFailed() const { return !IsOk(); }
		operator ResultStatus() const { return status; }
	};

	template <class T>
	struct Result : public ResultBase {
		T data;
		Result(const ResultBase& base)
			: ResultBase(base)
			, data()
		{}
		explicit Result(ResultStatus _status, const T& _data = T())
			: ResultBase(_status)
			, data(_data)
		{}
		operator ResultStatus() const { return status; }
	};

	inline ResultBase MakeResult(ResultStatus status)
	{
		return ResultBase(status);
	}

	template <class T>
	inline Result<T> MakeResult(ResultStatus status, const T& data)
	{
		return Result<T>(status, data);
	}

}
