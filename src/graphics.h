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

    namespace screen {
        extern Si32 w;
        extern Si32 h;
        extern Si32 cx;
        extern Si32 cy;
        extern Si64 size;

        void Init();
        bool CheckResize();
    }

	void DrawSprite(Sprite sprite, const Si32 to_x, const Si32 to_y);
	void DrawSprite(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite);
	void DrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend);
	void DrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Rgba blend);
	void DrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend1, Rgba blend2);
	void DrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Rgba blend1, Rgba blend2);
	void DrawWithFixedAlphaBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Ui8 alpha);
	void DrawWithFixedAlphaBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Ui8 alpha);

	void AlphaDraw(Sprite sprite, const Si32 to_x, const Si32 to_y, Ui8 opacity = 0xff);
	void AlphaDraw(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Ui8 opacity = 0xff);
	void AlphaDrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend, Ui8 opacity = 0xff);
	void AlphaDrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Rgba blend, Ui8 opacity = 0xff);
	void AlphaDrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend1, Rgba blend2);
	void AlphaDrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Rgba blend1, Rgba blend2);

	void FilterBrightness(Sprite sprite, Ui8 alpha);

	inline Rgba RgbaMult(Rgba c, Ui32 m)
	{
		Ui32 rb = c.rgba & 0x00ff00ff;
		Ui32 rbm = ((rb * m) >> 8) & 0x00ff00ff;
		Ui32 ga = (c.rgba >> 8) & 0x00ff00ff;
		Ui32 gam = ((ga * m)) & 0xff00ff00;
		return Rgba(rbm | gam);
	}

	inline Rgba RgbSum(Rgba x, Rgba y)
	{
		return Rgba(
			Ui16(x.r) + Ui16(y.r) > 255 ? 255 : x.r + y.r,
			Ui16(x.g) + Ui16(y.g) > 255 ? 255 : x.g + y.g,
			Ui16(x.b) + Ui16(y.b) > 255 ? 255 : x.b + y.b,
			0
		);
	}

	inline Rgba RgbaSum(Rgba x, Rgba y)
	{
		return Rgba(
			Ui16(x.r) + Ui16(y.r) > 255 ? 255 : x.r + y.r,
			Ui16(x.g) + Ui16(y.g) > 255 ? 255 : x.g + y.g,
			Ui16(x.b) + Ui16(y.b) > 255 ? 255 : x.b + y.b,
			Ui16(x.a) + Ui16(y.a) > 255 ? 255 : x.a + y.a
		);
	}

}
