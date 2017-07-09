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

	void DrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend);
	void DrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend1, Rgba blend2);
	void DrawWithFixedAlphaBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Ui8 alpha);

	void AlphaDraw(Sprite sprite, const Si32 to_x, const Si32 to_y);
	void AlphaDrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend);
	void AlphaDrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend1, Rgba blend2);

}
