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
	void Start()
	{
		wparams_.reset(new WorldParams(200, 200, 2));
		world_.reset(new World());
		vp_.reset(new ViewPort(*wparams_));

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
		world_->AddPlatform(plat1);
		world_->AddPlatform(plat2);
		world_->AddPlatform(plat3);
		world_->AddRobot(new Robot(plat1, 4, 4, Robot::kDirUp));
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
			vp_->Move(-movePxlPerSec * dt * ar::Vec2F(0.0f, 1.0f));
		}
		if (IsKey(kKeyDown)) {
			vp_->Move(-movePxlPerSec * dt * ar::Vec2F(0.0f, -1.0f));
		}
		if (IsKey(kKeyRight)) {
			vp_->Move(-movePxlPerSec * dt * ar::Vec2F(1.0f, 0.0f));
		}
		if (IsKey(kKeyLeft)) {
			vp_->Move(-movePxlPerSec * dt * ar::Vec2F(-1.0f, 0.0f));
		}

		if (IsKey(kKeyA)) {
			vp_->IncVisibleZ();
			ae::SetKey(kKeyA, false);
		}

		if (IsKey(kKeyZ)) {
			vp_->DecVisibleZ();
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
				world_->Simulate();
				lastStepTime_ = lastStepTime_ + secondsPerStep_;
				time_ = Time();
			}
			else {
				vp_->set_progress(progress);
				break;
			}
		}
	}

	void Render()
	{
		Clear();

		vp_->BeginRender(time_);
		world_->Draw(vp_.get());
		vp_->EndRender();

		ShowFrame();
	}
private:
	std::unique_ptr<WorldParams> wparams_;
	std::unique_ptr<World> world_;
	std::unique_ptr<ViewPort> vp_;

	double pauseBeforeStart_ = 0.2;
	double secondsPerStep_ = 0.5;
	double lastStepTime_ = 0.0;
	double lastControlTime_ = 0.0;
	double time_ = 0.0;
};

void Init()
{
	InitData();
	ResizeScreen(screen::w, screen::h);
}

void EasyMain()
{
	Init();
	srand((int)time(nullptr));

	Game game;

	game.Start();
	while (true) {
		if (!game.Control()) {
			break;
		}
		game.Update();
		game.Render();
	}
}
