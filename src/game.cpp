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

#include "game.h"
#include "levels.h"

namespace pilecode {

	using namespace ae;

	void Game::Restart()
	{
		world_.reset(initWorld_->Clone());
		vp_->set_world(world_.get());

		lastUpdateTime_ = 0.0;
		lastControlTime_ = 0.0;

		lastProgress_ = 1.0;
		simPaused_ = true;
	}

	void Game::ForwardStartTransition()
	{
		frameVisibility_ = false;
		panelVisibility_ = false;

		int N = 50;
		bgTransition_ = 1.0f;
		auto speed = Vec2F(0.0f, -N * 1.0f);
		vp_->MoveNoClamp(Vec2F(0, 1.0f * N*(N + 1) / 2));
		for (int i = 0; i < N; i++) {
			Render();
			if (IsKey(kKeyMouseLeft)) {
				break;
			}
			Sleep(0.01);
			vp_->MoveNoClamp(speed);
			speed += Vec2F(0.0f, 1.0f);
			bgTransition_ -= 1.0f / N;
		}
		bgTransition_ = 0.0f;

		frameVisibility_ = true;
		panelVisibility_ = true;
	}

	void Game::ForwardFinishTransition()
	{
		int dx0 = Pos::dx;
		int dy0 = Pos::dy;
		int dz0 = Pos::dz;

		float dx = (float)Pos::dx;
		float dy = (float)Pos::dy;
		float dz = (float)Pos::dz;

		frameVisibility_ = false;
		panelVisibility_ = false;

		//float speed = 1.01;
		int M = 10;
		for (int i = 0; i < M; i++) {
			Render();
			// TODO: interrupt on mouse click

			Sleep(0.01);
			dx += 4;
			dy += 2;

			vp_->MoveNoClamp(Vec2F(-16.0f, 8.0f));

			Pos::dx = (int)dx;
			Pos::dy = (int)dy;
			Pos::dz = (int)dz;
		}
		Sleep(0.1);

		auto speed = Vec2F(0.0f, -1.0f);
		int N = 50;
		for (int i = 0; i < N; i++) {
			Render();
			Sleep(0.01);
			vp_->MoveNoClamp(speed);
			speed += Vec2F(0.0f, -1.0f);
		}

		frameVisibility_ = true;
		panelVisibility_ = true;

		Pos::dx = dx0;
		Pos::dy = dy0;
		Pos::dz = dz0;
	}

	void Game::BackwardFinishTransition()
	{
		frameVisibility_ = false;
		panelVisibility_ = false;

		int N = 50;
		auto speed = Vec2F(0.0f, 0.0f);
		for (int i = 0; i < N; i++) {
			Render();
			if (IsKey(kKeyMouseLeft)) {
				break;
			}
			Sleep(0.01);
			vp_->MoveNoClamp(speed);
			speed += Vec2F(0.0f, 1.0f);
		}

		frameVisibility_ = true;
		panelVisibility_ = true;
	}

	void Game::BackwardStartTransition()
	{
		int dx0 = Pos::dx;
		int dy0 = Pos::dy;
		int dz0 = Pos::dz;

		int N = 50;
		int M = 10;
		int ddx = 4;
		int ddy = 2;

		float dx = (float)Pos::dx + ddx * M;
		float dy = (float)Pos::dy + ddy * M;
		float dz = (float)Pos::dz;

		Pos::dx = (int)dx;
		Pos::dy = (int)dy;
		Pos::dz = (int)dz;

		auto delta = Vec2F(16.0f, -8.0f);
		vp_->MoveNoClamp(delta * float(-M));

		frameVisibility_ = false;
		panelVisibility_ = false;

		bgTransition_ = 1.0f;
		auto speed = Vec2F(0.0f, N * 1.0f);
		vp_->MoveNoClamp(Vec2F(0, -1.0f * N*(N + 1) / 2));
		for (int i = 0; i < N; i++) {
			Render();
			if (IsKey(kKeyMouseLeft)) {
				break;
			}
			Sleep(0.01);
			vp_->MoveNoClamp(speed);
			speed -= Vec2F(0.0f, 1.0f);
			bgTransition_ -= 1.0f / N;
		}
		bgTransition_ = 0.0f;

		Sleep(0.1);

		for (int i = 0; i < M; i++) {
			Render();
			// TODO: interrupt on mouse click

			Sleep(0.01);
			dx -= ddx;
			dy -= ddy;

			vp_->MoveNoClamp(delta);


			Pos::dx = (int)dx;
			Pos::dy = (int)dy;
			Pos::dz = (int)dz;
		}

		frameVisibility_ = true;
		panelVisibility_ = true;

		Pos::dx = dx0;
		Pos::dy = dy0;
		Pos::dz = dz0;
	}

	void Game::Start(int level, int prevLevel, int maxLevel, World* savedWorld)
	{
		level_ = level;
		nextLevel_ = level_;
		prevLevel_ = prevLevel;
		maxLevel_ = maxLevel;

		initWorld_.reset(savedWorld ? savedWorld : GenerateLevel(level_));
		vp_.reset(new ViewPort(initWorld_.get()));
		Restart();
		DefaultPlaceMode();

		Control();

		if (!disableAnimation_) {
			if (prevLevel <= level) {
				ForwardStartTransition();
			}
			else {
				BackwardStartTransition();
			}
		}

		vp_->Center();

		MakeTools();
	}


	void Game::Finish(int level, int prevLevel)
	{
		if (!disableAnimation_) {
			if (prevLevel <= level) {
				ForwardFinishTransition();
			}
			else {
				BackwardFinishTransition();
			}
		}
	}

	bool Game::Control()
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

		if (ae::MouseWheelDelta() > 0) {
			vp_->IncVisibleZ();
		}

		if (ae::MouseWheelDelta() < 0) {
			vp_->DecVisibleZ();
		}

		if (IsKeyOnce(kKeySpace)) {
			PlayOrPause();
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

		tileHover_ = vp_->ToWorld(ae::MousePos(), wmouse_);

		if (IsKeyOnce(kKeyMouseRight)) {
			DefaultPlaceMode();
		}

		if (!IsMouseInPanel()) {
			switch (placeMode_) {
			case kPmSelect:
				// TODO: rectangular selection with copy/cut/paste and delete support
				// TODO: show help popups in this mode if single letter/robot is selected
				break;
			case kPmRobot:
				if (tileHover_ && IsKeyOnce(kKeyMouseLeft)) {
					if (world_->steps() > 0) {
						sfx::g_negative2.Play();
					}
					else {
						Robot original; // to sync seed in initWorld_ and world_
						world_->SwitchRobot(wmouse_, original);
						initWorld_->SwitchRobot(wmouse_, original);
						sfx::g_click.Play();
					}
				}
				break;
			case kPmLetter:
				if (tileHover_ && IsKeyOnce(kKeyMouseLeft)) {
					if (world_->IsTouched(wmouse_)) {
						sfx::g_negative2.Play();
					}
					else {
						world_->SetLetter(wmouse_, placeLetter_);
						initWorld_->SetLetter(wmouse_, placeLetter_);
						sfx::g_click.Play();
					}
				}
				break;
			}
		}

		ControlTools();

		return true;
	}

	void Game::Update()
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

		UpdateTools();
	}

	void Game::DrawPanel(Si32 width, Si32 height)
	{
		Si32 dx = image::g_panel.Width();
		Si32 dy = image::g_panel.Height();

		for (int iy = 0; iy < height; iy++) {
			for (int ix = 0; ix < width; ix++) {
				AlphaDraw(image::g_panel, ix * dx, iy * dy);
			}
		}

		for (int ix = 0; ix < width; ix++) {
			AlphaDraw(image::g_panel_top, ix * dx, height * dy);
		}

		AlphaDraw(image::g_panel_bottomright, width * dx, 0);

		for (int iy = 1; iy < height; iy++) {
			AlphaDraw(image::g_panel_right, width * dx, iy * dy);
		}

		AlphaDraw(image::g_panel_topright, width * dx, height * dy);
	}

	Button* Game::AddButton(Si32 pos, Sprite sprite)
	{
		buttons_.emplace_back(pos / panelHeight_, pos % panelHeight_, sprite);
		Button* btn = &buttons_.back();

		char gridHotkeys[] = {
			'A', 'Q', 'S', 'W', 'D', 'E', 'F', 'R', 'G', 'T', 'H', 'Y', 'J', 'U', 'K', 'I', 'L', 'O', ';', 'P'
		};
		if (pos < sizeof(gridHotkeys) / sizeof(*gridHotkeys)) {
			btn->HotKey(gridHotkeys[pos]);
		}

		return btn;
	}

	void Game::ControlTools()
	{
		for (Button& button : buttons_) {
			button.Control();
		}
	}

	bool Game::IsMouseInPanel()
	{
		return ae::MousePos().x < g_xcell * panelWidth_ && ae::MousePos().y < g_ycell * panelHeight_;
	}

	void Game::UpdateTools()
	{
		frameVisibility_ = !IsMouseInPanel();
		for (Button& button : buttons_) {
			button.Update();
		}
	}

	void Game::RenderTools()
	{
		if (placeMode_ == kPmRobot) {
			AlphaDraw(image::g_robot,
				ae::MousePos().x - g_tileCenter.x,
				ae::MousePos().y - g_tileCenter.y);
		}
		else if (placeMode_ == kPmLetter) {
			AlphaDraw(image::g_letter[placeLetter_],
				ae::MousePos().x - g_tileCenter.x,
				ae::MousePos().y - g_tileCenter.y);
		}

		DrawPanel(panelWidth_, panelHeight_);
		for (Button& button : buttons_) {
			button.Render();
		}
	}

	void Game::PlayOrPause()
	{
		simPaused_ = !simPaused_;
		if (simPaused_ == false) {
			DefaultPlaceMode();
		}
	}

	void Game::DefaultPlaceMode()
	{
		placeMode_ = kPmSelect;
		placeLetter_ = kLtSpace;
	}

	void Game::SwitchPlaceMode(PlaceMode mode, Letter letter)
	{
		if (placeMode_ != mode || placeLetter_ != letter) {
			// Enter new mode
			placeMode_ = mode;
			placeLetter_ = letter;
		}
		else { // trying to switch mode we are currently in -- exit to default mode
			DefaultPlaceMode();
		}
	}

	void Game::MakeTools()
	{
		buttons_.clear();
		panelWidth_ = 6;
		panelHeight_ = 2;

		// Add level switching buttons
		AddButton(0, image::g_button_prevlevel)->Click([=](Button* btn) {
			nextLevel_ = level_ - 1;
		})->set_enabled(level_ > 0);

		AddButton(1, image::g_button_nextlevel)->Click([=](Button* btn) {
			nextLevel_ = level_ + 1;
		})->set_enabled(level_ < maxLevel_);

		// Add playback buttons
		AddButton(2, image::g_button_play)->Click([=](Button* btn) {
			PlayOrPause();
		})->OnUpdate([=](Button* btn) {
			btn->set_sprite(simPaused_ ? image::g_button_play : image::g_button_pause);
		});

		AddButton(3, image::g_button_play)->Click([=](Button* btn) {
			if (simSpeed_ == 1.0f) {
				simSpeed_ = 2.0f;
			}
			else if (simSpeed_ == 2.0f) {
				simSpeed_ = 4.0f;
			}
			else if (simSpeed_ == 4.0f) {
				simSpeed_ = 8.0f;
			}
			else if (simSpeed_ == 8.0f) {
				simSpeed_ = 1.0f;
			}
		})->OnUpdate([=](Button* btn) {
			if (simSpeed_ == 1.0f) {
				btn->set_sprite(image::g_button_x1);
			}
			else if (simSpeed_ == 2.0f) {
				btn->set_sprite(image::g_button_x2);
			}
			else if (simSpeed_ == 4.0f) {
				btn->set_sprite(image::g_button_x4);
			}
			else if (simSpeed_ == 8.0f) {
				btn->set_sprite(image::g_button_x8);
			}
		});

		AddButton(4, image::g_button_stop)->Click([=](Button* btn) {
			Restart();
		})->OnUpdate([=](Button* btn) {
			btn->set_enabled(world_->steps() != 0);
		});

		// Add robot button
		AddButton(5, image::g_button_robot)->Click([=](Button* btn) {
			SwitchPlaceMode(kPmRobot, kLtSpace);
		})->OnUpdate([=](Button* btn) {
			btn->set_frame(placeMode_ == kPmRobot);
			btn->set_enabled(world_->steps() == 0);
		});

		Si32 btnPos = 6;

		// Add letter buttons
		for (int k = kLtSpace + 1; k < kLtMax; k++) {
			auto letter = Letter(k);
			if (world_->IsLetterAllowed(letter)) {
				AddButton(btnPos++, image::g_button_letter[letter])->Click([=](Button* btn) {
					SwitchPlaceMode(kPmLetter, letter);
				})->OnUpdate([=](Button* btn) {
					btn->set_frame(placeMode_ == kPmLetter && placeLetter_ == letter);
					btn->set_enabled(world_->steps() == 0); // TODO: allow to change untouched tiles
				});
			}
		}
	}

	Sprite& Game::BgForLevel(int level)
	{
		return image::g_background[abs(level) % image::g_backgroundCount];
	}

	void Game::Render()
	{
		BgForLevel(level_).Draw(0, 0);
		if (bgTransition_ > 0.0f) {
			DrawWithFixedAlphaBlend(BgForLevel(prevLevel_), 0, 0, Ui8(bgTransition_ * 255));
		}

		vp_->BeginRender(ae::Time());
		world_->Draw(vp_.get());

		if (tileHover_ && frameVisibility_) {
			for (int wz = 0; wz <= wmouse_.z; wz++) {
				vp_->Draw(&image::g_frame, Vec3Si32(wmouse_.x, wmouse_.y, wz), 1);
			}
		}

		vp_->EndRender(tileHover_, wmouse_);

		if (panelVisibility_) {
			RenderTools();
		}

		ae::ShowFrame();
	}

	bool Game::IsComplete()
	{
		return world_->IsOutputCorrect();
	}

	World* Game::GetInitWorld() const
	{
		return initWorld_->Clone();
	}

	int Game::GetNextLevel() const
	{
		return nextLevel_;
	}
}
