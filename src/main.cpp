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
#include "graphics.h"

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
			{ 0, 1, 2, 2, 2, 1, 1 },
			{ 0, 1, 2, 2, 2, 1, 1 },
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
				{ 0, 0, 2 },
				{ 0, 0, 2 },
				{ 0, 0, 1 },
				{ 0, 0, 1 },
				{ 0, 0, 1 },
			});

		Platform* plat4 = new Platform(
			1, 7, 2,
			{
				{ 2, 2, 2, 2, 2, 2, 2 },
				{ 2, 2, 2, 2, 2, 2, 2 },
			});
		plat4->changable_tile(0, 0)->set_output(kLtRight);

		world->AddPlatform(plat1);
		world->AddPlatform(plat2);
		world->AddPlatform(plat3);
		world->AddPlatform(plat4);

		return world;
	}

	World* GenerateLevel1()
	{
		WorldParams wparams(200, 200, 2, 3);
		World* world = new World(wparams);
		Platform* plat1 = new Platform(
			0, 0, 0,
			{
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
			});

		Platform* plat2 = new Platform(
			1, 1, 1,
			{
				{ 2 },
			});

		Platform* plat3 = new Platform(
			1, 7, 1,
			{
				{ 2 },
			});

		plat2->changable_tile(0, 0)->set_letter(kLtDot);
		plat3->changable_tile(0, 0)->set_output(kLtDot);

		world->AddPlatform(plat1);
		world->AddPlatform(plat2);
		world->AddPlatform(plat3);

		world->AllowLetter(kLtUp);
		world->AllowLetter(kLtRight);
		world->AllowLetter(kLtDown);
		world->AllowLetter(kLtLeft);
		world->AllowLetter(kLtRead);
		world->AllowLetter(kLtWrite);

		return world;
	}

	World* GenerateLevel2()
	{
		WorldParams wparams(200, 200, 2, 3);
		World* world = new World(wparams);
		Platform* plat1 = new Platform(
			0, 0, 0,
			{
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 0, 1, },
				{ 1, 0, 1, },
				{ 1, 0, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
				{ 1, 1, 1, },
			});

		Platform* plat2 = new Platform(
			1, 1, 1,
			{
				{ 2 },
			});

		Platform* plat3 = new Platform(
			1, 7, 1,
			{
				{ 2 },
			});

		plat2->changable_tile(0, 0)->set_letter(kLtDot);
		plat3->changable_tile(0, 0)->set_output(kLtDot);

		world->AddPlatform(plat1);
		world->AddPlatform(plat2);
		world->AddPlatform(plat3);

		world->AllowLetter(kLtUp);
		world->AllowLetter(kLtRight);
		world->AllowLetter(kLtDown);
		world->AllowLetter(kLtLeft);
		world->AllowLetter(kLtRead);
		world->AllowLetter(kLtWrite);

		return world;
	}

	World* GenerateLevel3()
	{
		WorldParams wparams(200, 200, 2, 3);
		World* world = new World(wparams);
		Platform* plat1 = new Platform(
			0, 0, 0,
			{
				{ 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, },
			});

		Platform* plat2 = new Platform(
			1, 1, 1,
			{
				{ 2, 2, 2 },
			});

		Platform* plat3 = new Platform(
			1, 7, 1,
			{
				{ 2, 2, 2 },
			});

		plat2->changable_tile(0, 0)->set_letter(kLtDot);
		plat2->changable_tile(1, 0)->set_letter(kLtDot);
		plat2->changable_tile(2, 0)->set_letter(kLtDot);
		plat3->changable_tile(0, 0)->set_output(kLtDot);
		plat3->changable_tile(1, 0)->set_output(kLtDot);
		plat3->changable_tile(2, 0)->set_output(kLtDot);

		world->AddPlatform(plat1);
		world->AddPlatform(plat2);
		world->AddPlatform(plat3);

		world->AllowLetter(kLtUp);
		world->AllowLetter(kLtRight);
		world->AllowLetter(kLtDown);
		world->AllowLetter(kLtLeft);
		world->AllowLetter(kLtRead);
		world->AllowLetter(kLtWrite);

		return world;
	}

	World* GenerateLevel4()
	{
		WorldParams wparams(200, 200, 2, 3);
		World* world = new World(wparams);
		Platform* plat1 = new Platform(
			0, 0, 0,
			{
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
			});

		Platform* plat2 = new Platform(
			1, 1, 1,
			{
				{ 2, 2, 2, 2 },
			});

		Platform* plat3 = new Platform(
			1, 7, 1,
			{
				{ 2, 2, 2, 2 },
			});

		plat2->changable_tile(0, 0)->set_letter(kLtUp);
		plat2->changable_tile(1, 0)->set_letter(kLtRight);
		plat2->changable_tile(2, 0)->set_letter(kLtLeft);
		plat2->changable_tile(3, 0)->set_letter(kLtDown);

		plat3->changable_tile(0, 0)->set_output(kLtUp);
		plat3->changable_tile(1, 0)->set_output(kLtRight);
		plat3->changable_tile(2, 0)->set_output(kLtLeft);
		plat3->changable_tile(3, 0)->set_output(kLtDown);

		world->AddPlatform(plat1);
		world->AddPlatform(plat2);
		world->AddPlatform(plat3);

		world->AllowLetter(kLtUp);
		world->AllowLetter(kLtRight);
		world->AllowLetter(kLtDown);
		world->AllowLetter(kLtLeft);
		world->AllowLetter(kLtRead);
		world->AllowLetter(kLtWrite);

		return world;
	}

	World* GenerateLevel5()
	{
		WorldParams wparams(200, 200, 2, 3);
		World* world = new World(wparams);
		Platform* plat1 = new Platform(
			0, 0, 0,
			{
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
				{ 1, 1, 1, 1, 1, 1, },
			});

		Platform* plat2 = new Platform(
			1, 1, 1,
			{
				{ 2, 2, 2, 2 },
			});

		Platform* plat3 = new Platform(
			1, 7, 1,
			{
				{ 2, 2, 2, 2 },
			});

		plat2->changable_tile(0, 0)->set_letter(kLtUp);
		plat2->changable_tile(1, 0)->set_letter(kLtRight);
		plat2->changable_tile(2, 0)->set_letter(kLtLeft);
		plat2->changable_tile(3, 0)->set_letter(kLtDown);

		plat3->changable_tile(0, 0)->set_output(kLtDown);
		plat3->changable_tile(1, 0)->set_output(kLtLeft);
		plat3->changable_tile(2, 0)->set_output(kLtRight);
		plat3->changable_tile(3, 0)->set_output(kLtUp);

		world->AddPlatform(plat1);
		world->AddPlatform(plat2);
		world->AddPlatform(plat3);

		world->AllowLetter(kLtUp);
		world->AllowLetter(kLtRight);
		world->AllowLetter(kLtDown);
		world->AllowLetter(kLtLeft);
		world->AllowLetter(kLtRead);
		world->AllowLetter(kLtWrite);

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

	void Start(int level)
	{
		switch (level % 5) {
		case 0:
			initWorld_.reset(GenerateLevel1());
			break;
		case 1:
			initWorld_.reset(GenerateLevel2());
			break;
		case 2:
			initWorld_.reset(GenerateLevel3());
			break;
		case 3:
			initWorld_.reset(GenerateLevel4());
			break;
		case 4:
			initWorld_.reset(GenerateLevel5());
			break;
		}
		vp_.reset(new ViewPort(initWorld_->params()));
		auto initialCoords = Vec2F(-180.0f, 40.0);
		vp_->Move(initialCoords);
		Restart();

		Control();

		frameVisibility_ = false;

		int N = 50;
		auto speed = Vec2F(0.0f, -N * 1.0f);
		vp_->Move(Vec2F(0, 1.0f * N*(N+1)/2));
		for (int i = 0; i < N; i++) {
			Render();
			if (IsKey(kKeyEscape)) {
				break;
			}
			Sleep(0.01);
			vp_->Move(speed);
			speed += Vec2F(0.0f, 1.0f);
		}

		frameVisibility_ = true;

		vp_->Locate(initialCoords);
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

	// Draws panel of given width and height in squares 256x256 units at bottom left corner
	void DrawPanel(Si32 width)
	{
		Si32 dx = image::g_panel.Width();
		Si32 dy = image::g_panel.Height();

		for (int ix = 0; ix < width; ix++) {
			AlphaDraw(image::g_panel, ix * dx, 0);
			AlphaDraw(image::g_panel_top, ix * dx, dy);
		}

		AlphaDraw(image::g_panel_right, width * dx, 0);
		AlphaDraw(image::g_panel_topright, width * dx, dy);
	}

	void DrawButton(Si32 ix, Si32 iy, Sprite sprite)
	{
		Si32 dx = image::g_panel.Width() / 2;
		Si32 dy = image::g_panel.Height() / 2;

		Si32 x0 = 0;
		Si32 y0 = 0;

		auto blend = Rgba(0, 0, 0, 0x20);

		AlphaDrawAndBlend(image::g_button_frame, dx * ix + x0, dy * iy + y0, blend);
		AlphaDrawAndBlend(sprite, dx * ix + x0, dy * iy + y0, blend);
	}

	void DrawTools()
	{
		DrawPanel(3);
		DrawButton(0, 0, image::g_button_play);
		DrawButton(0, 1, image::g_button_pause);
		DrawButton(1, 0, image::g_button_stop);
		DrawButton(1, 1, image::g_button_robot);
		DrawButton(2, 0, image::g_button_letter[kLtRight]);
		DrawButton(2, 1, image::g_button_letter[kLtDown]);
		DrawButton(3, 0, image::g_button_letter[kLtUp]);
		DrawButton(3, 1, image::g_button_letter[kLtLeft]);
		DrawButton(4, 0, image::g_button_letter[kLtRead]);
		DrawButton(4, 1, image::g_button_letter[kLtWrite]);
	}

	void Render()
	{
		Clear();

		vp_->BeginRender(Time());
		world_->Draw(vp_.get());
		if (frameVisibility_) {
			for (int wz = 0; wz <= wmouse_.z; wz++) {
				vp_->Draw(&image::g_frame, Vec3Si32(wmouse_.x, wmouse_.y, wz), 1);
			}
		}
		vp_->EndRender();

		DrawTools();

		ShowFrame();
	}

	bool IsComplete()
	{
		return world_->IsOutputCorrect();
	}

	void Finish()
	{
		int dx0 = Pos::dx;
		int dy0 = Pos::dy;
		int dz0 = Pos::dz;

		float dx = (float)Pos::dx;
		float dy = (float)Pos::dy;
		float dz = (float)Pos::dz;

		frameVisibility_ = false;

		//float speed = 1.01;
		for (int i = 0; i < 10; i++) {
			Render();
			Sleep(0.01);
			dx += 4;
			dy += 2;

			vp_->Move(Vec2F(-16.0f, 8.0f));

			Pos::dx = (int)dx;
			Pos::dy = (int)dy;
			Pos::dz = (int)dz;
		}
		Sleep(0.1);

		auto speed = Vec2F(0.0f, -1.0f);
		for (int i = 0; i < 50; i++) {
			Render();
			Sleep(0.01);
			vp_->Move(speed);
			speed += Vec2F(0.0f, -1.0f);
		}

		frameVisibility_ = true;

		Pos::dx = dx0;
		Pos::dy = dy0;
		Pos::dz = dz0;
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
	bool frameVisibility_ = true;
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

	bool exiting = false;
	int level = 0;
	while (!exiting) {
		Game game;
		game.Start(level);

		while (true) {
			if (!game.Control()) {
				exiting = true;
				break;
			}
			game.Update();
			game.Render();
			if (game.IsComplete()) {
				break;
			}
		}

		game.Finish();
		level++;
	}
}
