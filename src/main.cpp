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

#include "engine/easy.h"

using namespace arctic;  // NOLINT
using namespace arctic::easy;  // NOLINT
using namespace pilecode; // NOLINT

World g_world;
ViewPort g_vp;

void Init()
{
	InitData();
	ResizeScreen(640, 400);

	Platform* plat = new Platform({
		{ 2, 1, 1, 1, 1, 1, 2 },
		{ 1, 0, 1, 0, 1, 0, 1 },
		{ 1, 2, 1, 1, 1, 0, 1 },
  	    { 0, 1, 1, 2, 1, 0, 1 },
	    { 1, 0, 1, 1, 1, 0, 1 },
	    { 2, 1, 1, 0, 1, 1, 2 }
	});
	g_world.AddPlatform(plat);
}

void Render()
{
	Clear();

	g_world.Draw(&g_vp);

	ShowFrame();
}

void EasyMain()
{
	Init();
	while (true) {
		if (IsKey(kKeyEscape)) {
			break;
		}
		Render();
	}
}
