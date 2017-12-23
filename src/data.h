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

#include "pilecode.h"

#include "engine/easy.h"

namespace pilecode {
	namespace ui {
		constexpr Si32 g_xcell = 96;
		constexpr Si32 g_ycell = 96;
		constexpr Si32 g_marginBottom = 16;
		constexpr Si32 g_marginLeft = 16;
		constexpr Si32 g_marginTop = g_marginBottom;
		constexpr Si32 g_marginRight = g_marginLeft;
	}

	constexpr Si32 g_xtileorigin = 64;
	constexpr Si32 g_ytileorigin = 82;

	constexpr Si32 g_yrobotReg = 64;

	extern const Vec2Si32 g_tileCenter;

	namespace image {
		extern Sprite g_pilecode;

		extern Sprite g_empty;

		extern Sprite g_tile[kTlMax];
		extern Sprite g_letter[kLtMax];
		extern Sprite g_letter_output[kLtMax];
		extern Sprite g_letter_output_filled[kLtMax];
		extern Sprite g_frame;
		extern Sprite g_boldFrame;
		extern Sprite g_tileMask;

		extern Sprite g_robot;
		extern Sprite g_robotShadow;

		extern Sprite g_button_musicalnote;
		extern Sprite g_button_nextlevel;
		extern Sprite g_button_prevlevel;
		extern Sprite g_button_play;
		extern Sprite g_button_pause;
		extern Sprite g_button_rewind;
		extern Sprite g_button_fastforward;
		extern Sprite g_button_replay;
		extern Sprite g_button_minus;
		extern Sprite g_button_plus;
        extern Sprite g_button_cancel;
        extern Sprite g_button_checked;
		extern Sprite g_button_x1;
		extern Sprite g_button_x2;
		extern Sprite g_button_x4;
		extern Sprite g_button_x8;
		extern Sprite g_button_robot;
		extern Sprite g_button_letter[kLtMax];

		constexpr int g_backgroundCount = 3;
		extern Sprite g_background[g_backgroundCount];

#ifdef MOD_XMAS
        constexpr int g_snowflakeW = 6;
        constexpr int g_snowflakeH = 4;
        constexpr int g_snowflakeCount = g_snowflakeW * g_snowflakeH;
        extern Sprite g_snowflake[g_snowflakeCount];
#endif
}

	namespace music {
		constexpr int g_backgroundCount = 1;
		extern Sound g_background[g_backgroundCount];
	}

	namespace sfx {
		extern Sound g_click;
		extern Sound g_click2;
		extern Sound g_positive;
		extern Sound g_negative2;
		extern Sound g_read;
		extern Sound g_write;
        extern Sound g_eraseLetter;
	}

	Sprite CreateBoundary(Sprite sprite,
		Si32 xExpandRadius, Si32 yExpandRadius,
		Si32 xBlurRadius, Si32 yBlurRadius,
		Ui8 lo, Ui8 hi, Si32 loSlope, Si32 hiSlope,
		Rgba color);

	Sprite CreateShadow(Sprite sprite,
		Si32 expandRadius,
		Si32 blurRadius,
		Rgba color);

	void InitData();
}
