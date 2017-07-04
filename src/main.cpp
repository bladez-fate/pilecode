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
	double lastControlTime_ = 0.0;
	double time_ = 0.0;

	void Init()
	{
		InitData();
		ResizeScreen(screen::w, screen::h);

		g_wparams.reset(new WorldParams(200, 200, 2));
		g_world.reset(new World());
		g_vp.reset(new ViewPort(*g_wparams));

		Platform* plat1 = new Platform(
			0, 0, 0,
		{
			{ 0, 0, 1, 1, 1, 1, 0 },
			{ 0, 1, 1, 0, 1, 1, 1 },
			{ 0, 1, 1, 1, 1, 1, 1 },
			{ 0, 1, 1, 2, 1, 1, 1 },
			{ 1, 1, 1, 1, 1, 1, 1 },
			{ 0, 1, 1, 0, 1, 0, 0 }
		});

		Platform* plat2 = new Platform(
			0, 10, 0,
			{
				{ 0, 1, 1, 1, 1, 1, 0 },
				{ 1, 1, 1, 1, 1, 1, 1 },
				{ 1, 1, 1, 1, 1, 1, 1 },
				{ 0, 1, 1, 1, 1, 1, 0 },
			});

		Platform* plat3 = new Platform(
			4, 5, 1,
			{
				{ 0, 1, 1 },
				{ 1, 1, 1 },
			});
		g_world->AddPlatform(plat1);
		g_world->AddPlatform(plat2);
		g_world->AddPlatform(plat3);
		g_world->AddRobot(new Robot(plat1, 4, 4, Robot::kDirUp));
	}

	bool Control()
	{
		double time = Time();
		if (lastControlTime_ == 0.0) {
			lastControlTime_ = time;
		}
		float dt = float(time - lastControlTime_);
		lastControlTime_ = time;

		if (IsKey(kKeyEscape)) {
			return false;
		}

		float movePxlPerSec = float(screen::h) * 0.50f;
		if (IsKey(kKeyUp)) {
			g_vp->Move(-movePxlPerSec * dt * ar::Vec2F(0.0f, 1.0f));
		}
		if (IsKey(kKeyDown)) {
			g_vp->Move(-movePxlPerSec * dt * ar::Vec2F(0.0f, -1.0f));
		}
		if (IsKey(kKeyRight)) {
			g_vp->Move(-movePxlPerSec * dt * ar::Vec2F(1.0f, 0.0f));
		}
		if (IsKey(kKeyLeft)) {
			g_vp->Move(-movePxlPerSec * dt * ar::Vec2F(-1.0f, 0.0f));
		}

		if (IsKey(kKeyA)) {
			g_vp->IncVisibleZ();
			ae::SetKey(kKeyA, false);
		}

		if (IsKey(kKeyZ)) {
			g_vp->DecVisibleZ();
			ae::SetKey(kKeyZ, false);
		}

		return true;
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
		if (!game.Control()) {
			break;
		}
		game.Update();
		game.Render();
	}
}
