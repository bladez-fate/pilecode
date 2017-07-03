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

class Game {
public:

	std::unique_ptr<WorldParams> g_wparams;
	std::unique_ptr<World> g_world;
	std::unique_ptr<ViewPort> g_vp;

	double pauseBeforeStart_ = 0.2;
	double secondsPerStep_ = 0.5;
	double lastStepTime_ = 0.0;
	double time_ = 0.0;

	void Init()
	{
		InitData();
		ResizeScreen(screen::w, screen::h);

		g_wparams.reset(new WorldParams(200, 200, 10));
		g_world.reset(new World());
		g_vp.reset(new ViewPort(*g_wparams));

		Platform* plat = new Platform({
			{ 0, 0, 1, 1, 1, 1, 0 },
			{ 0, 1, 1, 0, 1, 1, 1 },
			{ 0, 1, 1, 1, 1, 1, 1 },
			{ 0, 1, 1, 2, 1, 1, 1 },
			{ 1, 1, 1, 1, 1, 1, 1 },
			{ 0, 1, 1, 0, 1, 0, 0 }
		});
		g_world->AddPlatform(plat);
		g_world->AddRobot(new Robot(plat, 4, 4, Robot::kDirUp));
	}

	void Update()
	{
		time_ = Time();
		if (lastStepTime_ == 0.0) {
			lastStepTime_ = time_ - secondsPerStep_ + pauseBeforeStart_;
		}

		while (true) {
			double progress = (time_ - lastStepTime_) / secondsPerStep_;
			if (progress > 1.0) {
				g_world->Simulate();
				lastStepTime_ = lastStepTime_ + secondsPerStep_;
				time_ = Time();
			}
			else {
				g_vp->set_progress(progress);
				break;
			}
		}
	}

	void Render()
	{
		Clear();

		g_vp->BeginRender(time_);
		g_world->Draw(g_vp.get());
		g_vp->EndRender();

		ShowFrame();
	}
};

void EasyMain()
{
	Game game;

	srand((int)time(nullptr));

	game.Init();
	while (true) {
		if (IsKey(kKeyEscape)) {
			break;
		}
		game.Update();
		game.Render();
	}
}
