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

using namespace arctic;  // NOLINT
using namespace arctic::easy;  // NOLINT
using namespace pilecode; // NOLINT

template <class T>
bool IsKeyOnce(T t)
{
	if (ae::IsKey(t)) {
		SetKey(t, false);
		return true;
	}
	else {
		return false;
	}
}

class Game {
public:
	World* GenerateWorld()
	{
		WorldParams wparams(200, 200, 3, 4);
		World* world = new World(wparams);
		Platform* plat1 = new Platform(
			0, 0, 0,
		{
			{ 0, 0, 1, 1, 1, 1, 0 },
			{ 0, 1, 1, 0, 1, 1, 1 },
			{ 0, 1, 1, 1, 1, 1, 1 },
			{ 0, 1, 1, 1, 1, 1, 1 },
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
			2, 2, 1,
			{
				{ 0, 1, 1 },
				{ 1, 1, 1 },
				{ 0, 0, 1 },
				{ 0, 0, 1 },
				{ 0, 0, 1 },
				{ 0, 0, 1 },
				{ 0, 0, 1 },
				{ 0, 0, 1 },
				{ 0, 0, 1 },
				{ 0, 0, 1 },
			});

		Platform* plat4 = new Platform(
			1, 7, 2,
			{
				{ 1, 1, 1, 1, 1, 1, 1 },
				{ 1, 1, 1, 1, 1, 1, 1 },
			});

		world->AddPlatform(plat1);
		world->AddPlatform(plat2);
		world->AddPlatform(plat3);
		world->AddPlatform(plat4);
	
		return world;
	}

	void Restart()
	{
		world_.reset(initWorld_->Clone());
		vp_->set_world(world_.get());

		lastUpdateTime_ = 0.0;
		lastControlTime_ = 0.0;

		lastProgress_ = 1.0;
	}

	void Start()
	{
		initWorld_.reset(GenerateWorld());
		vp_.reset(new ViewPort(initWorld_->params()));
		Restart();
	}

	bool Control()
	{
		if (IsKey(kKeyEscape)) {
			return false;
		}

		if (IsKeyOnce(kKeyF5)) {
			Restart();
			return true;
		}

		double time = Time();
		if (lastControlTime_ == 0.0) {
			lastControlTime_ = time;
		}
		float dt = float(time - lastControlTime_);
		lastControlTime_ = time;

		if (IsKey(kKeyUp) || ae::MousePos().y >= screen::h - mouseScrollMargin_) {
			vp_->Move(-movePxlPerSec_ * dt * Vec2F(0.0f, 1.0f));
		}
		if (IsKey(kKeyDown) || ae::MousePos().y < mouseScrollMargin_) {
			vp_->Move(-movePxlPerSec_ * dt * Vec2F(0.0f, -1.0f));
		}
		if (IsKey(kKeyRight) || ae::MousePos().x >= screen::w - mouseScrollMargin_) {
			vp_->Move(-movePxlPerSec_ * dt * Vec2F(1.0f, 0.0f));
		}
		if (IsKey(kKeyLeft) || ae::MousePos().x < mouseScrollMargin_) {
			vp_->Move(-movePxlPerSec_ * dt * Vec2F(-1.0f, 0.0f));
		}

		if (IsKeyOnce(kKeyA) || ae::MouseWheelDelta() > 0) {
			vp_->IncVisibleZ();
		}

		if (IsKeyOnce(kKeyZ) || ae::MouseWheelDelta() < 0) {
			vp_->DecVisibleZ();
		}

		if (IsKeyOnce(kKeySpace)) {
			simPaused_ = !simPaused_;
		}

		if (IsKeyOnce(kKey1)) {
			simSpeed_ = 1.0;
		}

		if (IsKeyOnce(kKey2)) {
			simSpeed_ = 2.0;
		}

		if (IsKeyOnce(kKey3)) {
			simSpeed_ = 4.0;
		}

		if (IsKeyOnce(kKey4)) {
			simSpeed_ = 8.0;
		}

		wmouse_ = vp_->ToWorld(ae::MousePos());

		if (IsKeyOnce(kKeyMouseLeft)) {
			if (world_->IsTouched(wmouse_)) {
				// TODO: play forbidden sound and text reason
			}
			else {
				world_->SwitchLetter(wmouse_);
				initWorld_->SwitchLetter(wmouse_);
			}
		}

		if (IsKeyOnce(kKeyMouseRight)) {
			if (world_->steps() > 0) {
				// TODO: play forbidden sound and text reason
			}
			else {
				Robot original; // to sync seed in initWorld_ and world_
				world_->SwitchRobot(wmouse_, original);
				initWorld_->SwitchRobot(wmouse_, original);
			}
		}

		return true;
	}

	void Update()
	{
		double time = Time();
		double secondsPerStep = secondsPerStepDefault_ / simSpeed_;
		if (lastUpdateTime_ == 0.0) {
			lastUpdateTime_ = time;
		}

		while (true) {
			double progress = lastProgress_ + (time - lastUpdateTime_) / secondsPerStep;
			if (progress >= 1.0) {
				if (simPaused_) {
					lastUpdateTime_ = 0;
					lastProgress_ = 1.0;
					break;
				}
				else {
					world_->Simulate();
					lastUpdateTime_ = time;
					lastProgress_ = progress - 1.0;
					time = Time();
				}
			}
			else {
				vp_->set_progress(progress);
				lastUpdateTime_ = time;
				lastProgress_ = progress;
				break;
			}
		}

		vp_->set_progress(lastProgress_);
	}

	void Render()
	{
		Clear();

		vp_->BeginRender(Time());
		world_->Draw(vp_.get());
		for (int wz = 0; wz <= wmouse_.z; wz++) {
			vp_->Draw(&image::g_frame, Vec3Si32(wmouse_.x, wmouse_.y, wz), 1);
		}
		vp_->EndRender();

		ShowFrame();
	}
private:
	// control configuration
	float movePxlPerSec_ = float(screen::h) * 0.50f;
	Si32 mouseScrollMargin_ = 5;
	
	// world
	std::unique_ptr<WorldParams> wparams_;
	std::unique_ptr<World> initWorld_;
	std::unique_ptr<World> world_;
	std::unique_ptr<ViewPort> vp_;

	// timing
	double secondsPerStepDefault_ = 0.5;
	double lastUpdateTime_ = 0.0;
	double lastControlTime_ = 0.0;
	
	// simulation 
	double lastProgress_ = 1.0;
	bool simPaused_ = true;
	double simSpeed_ = 1.0;

	// gameplay
	Vec3Si32 wmouse_;
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
