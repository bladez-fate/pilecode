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

#define _USE_MATH_DEFINES
//#define DEV_MODE
//#define SCROLL_DISABLED
//#define MOD_XMAS
#define SHOW_FPS

#include "engine/easy.h"

namespace ae {
	using namespace arctic;
	using namespace arctic::easy;
}

namespace pilecode {
	using ae::Sprite;
	using ae::Sound;
	using ae::Si8;
	using ae::Ui8;
	using ae::Si16;
	using ae::Ui16;
	using ae::Si32;
	using ae::Ui32;
	using ae::Si64;
	using ae::Ui64;
	using ae::Rgba;
	using ae::Vec2F;
	using ae::Vec2Si32;
	using ae::Vec3Si32;

	template <class T>
	bool IsKeyOnce(T t)
	{
		if (ae::IsKeyDown(t)) {
			ae::SetKey(t, false);
			return true;
		}
		else {
			return false;
		}
	}

}
