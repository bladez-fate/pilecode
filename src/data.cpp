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
		Sprite g_empty;

		Sprite g_tile[kTlMax];
		Sprite g_letter[kLtMax];
		Sprite g_frame;
		Sprite g_robot;
		Sprite g_robotShadow;

		Sprite g_panel;
		Sprite g_panel_bottomright;
		Sprite g_panel_right;
		Sprite g_panel_top;
		Sprite g_panel_topright;

		Sprite g_button_frame;
		Sprite g_button_play;
		Sprite g_button_pause;
		Sprite g_button_stop;
		Sprite g_button_robot;
		Sprite g_button_letter[kLtMax];
	}

	void InitData()
	{
		image::g_empty.Load("data/empty.tga");

		image::g_tile[kTlNone] = image::g_empty;
		image::g_tile[kTlBrick].Load("data/tile-brick.tga");
		image::g_tile[kTlInactive].Load("data/tile-inactive-brick.tga");

		image::g_letter[kLtSpace] = image::g_empty;
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
	
		image::g_panel.Load("data/panel.tga");
		image::g_panel_bottomright.Load("data/panel-bottomright.tga");
		image::g_panel_right.Load("data/panel-right.tga");
		image::g_panel_top.Load("data/panel-top.tga");
		image::g_panel_topright.Load("data/panel-topright.tga");

		image::g_button_frame.Load("data/button-frame.tga");
		image::g_button_play.Load("data/button-play.tga");
		image::g_button_pause.Load("data/button-pause.tga");
		image::g_button_stop.Load("data/button-stop.tga");
		image::g_button_robot.Load("data/button-robot.tga");
		image::g_button_letter[kLtSpace] = image::g_empty;
		image::g_button_letter[kLtUp].Load("data/button-up.tga");
		image::g_button_letter[kLtDown].Load("data/button-down.tga");
		image::g_button_letter[kLtRight].Load("data/button-right.tga");
		image::g_button_letter[kLtLeft].Load("data/button-left.tga");
		image::g_button_letter[kLtRead].Load("data/button-read.tga");
		image::g_button_letter[kLtWrite].Load("data/button-write.tga");
		image::g_button_letter[kLtDot] = image::g_empty;
	}
}