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

#include "pilecode.h"
#include "data.h"

namespace pilecode {
	namespace image {
		Sprite g_tile[kTlMax];
		Sprite g_letter[kLtMax];
		Sprite g_frame;
		Sprite g_robot;
		Sprite g_robotShadow;
	}

	void InitData()
	{
		image::g_tile[kTlNone].Load("data/empty.tga");
		image::g_tile[kTlBrick].Load("data/tile-brick.tga");
		image::g_tile[kTlInactive].Load("data/tile-inactive-brick.tga");

		image::g_letter[kLtSpace].Load("data/empty.tga");
		image::g_letter[kLtUp].Load("data/letter-up.tga");
		image::g_letter[kLtDown].Load("data/letter-down.tga");
		image::g_letter[kLtRight].Load("data/letter-right.tga");
		image::g_letter[kLtLeft].Load("data/letter-left.tga");
		image::g_letter[kLtRead].Load("data/letter-read.tga");
		image::g_letter[kLtWrite].Load("data/letter-write.tga");
		image::g_letter[kLtDot].Load("data/letter-dot.tga");

		image::g_frame.Load("data/letter-frame.tga");
		image::g_robot.Load("data/robot.tga");
		image::g_robotShadow.Load("data/robot-shadow.tga");
	}
}