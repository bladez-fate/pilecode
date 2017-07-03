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

std::unique_ptr<WorldParams> g_wparams;
std::unique_ptr<World> g_world;
std::unique_ptr<ViewPort> g_vp;

void Init()
{
	InitData();
	ResizeScreen(screen::w, screen::h);

	g_wparams.reset(new WorldParams(200, 200, 10));
	g_world.reset(new World());
	g_vp.reset(new ViewPort(*g_wparams));

	Platform* plat = new Platform({
		{ 0, 0, 1, 1, 1, 1, 0 },
		{ 0, 0, 1, 0, 1, 1, 1 },
		{ 0, 0, 1, 1, 1, 0, 1 },
  	    { 0, 1, 1, 2, 1, 0, 1 },
	    { 1, 1, 1, 1, 1, 1, 1 },
	    { 0, 1, 1, 0, 1, 0, 0 }
	});
	g_world->AddPlatform(plat);
	g_world->AddRobot(new Robot(plat, 2, 0, Robot::kDirUp));
	g_world->AddRobot(new Robot(plat, 4, 4, Robot::kDirUp));
}

void Render()
{
	Clear();

	g_world->Draw(g_vp.get());
	g_vp->Render();

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
