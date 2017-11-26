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
			if (ui::StopAnimationKey()) {
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
		bool stop = false;
		for (int i = 0; i < M; i++) {
			Render();
			if (ui::StopAnimationKey()) {
				stop = true;
				break;
			}

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
		for (int i = 0; i < N && !stop; i++) {
			if (ui::StopAnimationKey()) {
				break;
			}
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
			if (ui::StopAnimationKey()) {
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
		bool stop = false;
		for (int i = 0; i < N; i++) {
			Render();
			if (ui::StopAnimationKey()) {
				stop = true;
				break;
			}
			Sleep(0.01);
			vp_->MoveNoClamp(speed);
			speed -= Vec2F(0.0f, 1.0f);
			bgTransition_ -= 1.0f / N;
		}
		bgTransition_ = 0.0f;

		Sleep(0.1);

		for (int i = 0; i < M && !stop; i++) {
			Render();
			if (ui::StopAnimationKey()) {
				break;
			}
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

	void Game::StartWithEditor(int level, int prevLevel, int maxLevel, World* savedWorld)
	{
		editorMode_ = true;
		disableTransition_ = true;
		Start(level, prevLevel, maxLevel, savedWorld);
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

		if (!disableTransition_) {
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
		if (!disableTransition_) {
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
		if (IsKeyOnce(kKeyEscape)) {
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

		if (!ControlTools()) { // if any ui buttons is hovered -- stop here
			frameVisibility_ = false;
			return true;
		}
		else { // if buttons are not hovered -- try proceed with placement actions
			frameVisibility_ = true;
			tileHover_ = vp_->ToWorldTile(ae::MousePos(), wmouse_, tilePos_);
			if (tileHover_) {
				placeLetter_ = PlaceLetter(tilePos_, placeLetterRight_, placeLetterDown_, placeLetterUp_, placeLetterLeft_);
			}
			else {
				placeLetter_ = placeLetterRight_;
			}

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

			return true;
		}
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

	bool Game::ControlTools()
	{
		for (auto i = buttons_.rbegin(), e = buttons_.rend(); i != e; ++i) {
			Button& button = *i;
			if (!button.Control()) {
				return false;
			}
		}
		return true;
	}

	void Game::UpdateTools()
	{
		for (Button& button : buttons_) {
			button.Update();
		}
	}

	void Game::RenderTools()
	{
		//DrawPanel(panelWidth_, panelHeight_);
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
		placeLetterRight_ = kLtSpace;
		placeLetterDown_ = kLtSpace;
		placeLetterUp_ = kLtSpace;
		placeLetterLeft_ = kLtSpace;
	}

	void Game::SwitchPlaceMode(PlaceMode mode, Letter right, Letter down, Letter up, Letter left)
	{
		if (placeMode_ != mode
			|| placeLetterRight_ != right
			|| placeLetterDown_ != down
			|| placeLetterUp_ != up
			|| placeLetterLeft_ != left) {
			// Enter new mode
			placeMode_ = mode;
			placeLetterRight_ = right;
			placeLetterDown_ = down;
			placeLetterUp_ = up;
			placeLetterLeft_ = left;
		}
		else { // trying to switch mode we are currently in -- exit to default mode
			DefaultPlaceMode();
		}
	}

	void Game::SwitchPlaceMode(PlaceMode mode, Letter letter)
	{
		SwitchPlaceMode(mode, letter, letter, letter, letter);
	}

	Letter Game::PlaceLetter(Vec2F tp, Letter right, Letter down, Letter up, Letter left)
	{
		//
		//   TILE COORDINATE SYSTEM SCHEME AND PLACEMENT REGIONS
		//
		//  ^ y
		//  |
		// 1+------+
		//  |\    /|
		//  | \  / |
		//  |  \/  |
		//  |  /\  |
		//  | /  \ |
		//  |/    \|
		// 0+------+> x
		//  0      1
		//  kLtRight,          // x+
		//	kLtDown,           // y-
		//	kLtUp,             // y+
		//	kLtLeft,           // x-
		if (tp.y > tp.x) {
			return (tp.y > 1 - tp.x ? up : left);
		}
		else {
			return (tp.y > 1 - tp.x ? right : down);
		}
	}

	void Game::MakeTools()
	{
		buttons_.clear();

		// Add playback buttons
		HorizontalFluidFrame frmPlayback(kCenterBottom, 32);
		frmPlayback
			.Add(image::g_button_rewind)
			.Add(image::g_button_play)
			.Add(image::g_button_fastforward)
			;
		HorizontalFluidFrame frmPlaybackRate(kRightBottom, 1);
		frmPlaybackRate
			.Add(image::g_button_minus)
			.Add(image::g_button_x1)
			.Add(image::g_button_plus)
			;
		AddButton(image::g_button_rewind, frmPlayback.Place(0))->Click([=](Button* btn) {
			Restart();
		})->OnUpdate([=](Button* btn) {
			btn->set_enabled(world_->steps() != 0);
		});
		AddButton(image::g_button_fastforward, frmPlayback.Place(2))->Click([=](Button* btn) {
			//FastForward();
		})->OnUpdate([=](Button* btn) {
			btn->set_enabled(world_->steps() != 0);
		});
		AddButton(image::g_button_play, frmPlayback.Place(1))->Click([=](Button* btn) {
			PlayOrPause();
		})->OnUpdate([=](Button* btn) {
			btn->SetSprite(simPaused_ ? image::g_button_play : image::g_button_pause);
		});

		AddButton(image::g_button_minus, frmPlaybackRate.Place(0))->Click([=](Button* btn) {
			if (simSpeed_ == 2.0f) {
				simSpeed_ = 1.0f;
			}
			else if (simSpeed_ == 4.0f) {
				simSpeed_ = 2.0f;
			}
			else if (simSpeed_ == 8.0f) {
				simSpeed_ = 4.0f;
			}
		})->OnUpdate([=](Button* btn) {
			btn->set_enabled(world_->steps() != 0);
		})->Padding(0);

		AddButton(image::g_button_plus, frmPlaybackRate.Place(2))->Click([=](Button* btn) {
			if (simSpeed_ == 1.0f) {
				simSpeed_ = 2.0f;
			}
			else if (simSpeed_ == 2.0f) {
				simSpeed_ = 4.0f;
			}
			else if (simSpeed_ == 4.0f) {
				simSpeed_ = 8.0f;
			}
		})->OnUpdate([=](Button* btn) {
			btn->set_enabled(world_->steps() != 0);
		})->Padding(0);

		AddButton(image::g_button_x1, frmPlaybackRate.Place(1))->OnUpdate([=](Button* btn) {
			btn->set_enabled(world_->steps() != 0);
			if (simSpeed_ == 1.0f) {
				btn->SetSprite(image::g_button_x1);
			}
			else if (simSpeed_ == 2.0f) {
				btn->SetSprite(image::g_button_x2);
			}
			else if (simSpeed_ == 4.0f) {
				btn->SetSprite(image::g_button_x4);
			}
			else if (simSpeed_ == 8.0f) {
				btn->SetSprite(image::g_button_x8);
			}
		});

		// Add music on/off button
		AddButton(image::g_button_musicalnote, Region::Screen(), kLeftTop)->Click([=](Button* btn) {
			ToggleMusic();
		});

		// Add level switching buttons
		AddButton(image::g_button_prevlevel, Region::Screen(), kRightBottom)->Click([=](Button* btn) {
			nextLevel_ = level_ - 1;
		})->OnUpdate([=](Button* btn) {
			btn->set_enabled(level_ > 0 && world_->steps() == 0);
		});
		AddButton(image::g_button_nextlevel, Region::Screen(), kRightTop)->Click([=](Button* btn) {
			nextLevel_ = level_ + 1;
		})->OnUpdate([=](Button* btn) {
			btn->set_enabled(level_ < maxLevel_ && world_->steps() == 0);
		});

		// Add palette buttons
		Si32 btnPos = 0;
		GridFrame frmPalette(kLeftCenter, 1, 4, image::g_button_robot.Width(), image::g_button_robot.Height(), 10, 10);
		// Add robot button
		AddButton(image::g_button_robot, frmPalette.Place(0, btnPos++))->Click([=](Button* btn) {
			SwitchPlaceMode(kPmRobot, kLtSpace);
		})->OnUpdate([=](Button* btn) {
			btn->set_frame(placeMode_ == kPmRobot);
			btn->set_enabled(world_->steps() == 0);
		});

		if (world_->IsLetterAllowed(kLtRight)
			&& world_->IsLetterAllowed(kLtDown)
			&& world_->IsLetterAllowed(kLtUp)
			&& world_->IsLetterAllowed(kLtLeft)) {
			AddButton(image::g_button_letter[kLtRight], frmPalette.Place(0, btnPos++))->Click([=](Button* btn) {
				SwitchPlaceMode(kPmLetter, kLtRight, kLtDown, kLtUp, kLtLeft);
			})->OnUpdate([=](Button* btn) {
				btn->set_frame(placeMode_ == kPmLetter
					&& placeLetterRight_ == kLtRight
					&& placeLetterDown_ == kLtDown
					&& placeLetterUp_ == kLtUp
					&& placeLetterLeft_ == kLtLeft
				);
				btn->set_enabled(world_->steps() == 0); // TODO: allow to change untouched tiles
			});
		}

		for (auto letter : {kLtRead, kLtWrite, kLtDot}) {
			if (world_->IsLetterAllowed(letter)) {
				AddButton(image::g_button_letter[letter], frmPalette.Place(0, btnPos++))->Click([=](Button* btn) {
					SwitchPlaceMode(kPmLetter, letter);
				})->OnUpdate([=](Button* btn) {
					btn->set_frame(placeMode_ == kPmLetter
						&& placeLetterRight_ == letter
						&& placeLetterDown_ == letter
						&& placeLetterUp_ == letter
						&& placeLetterLeft_ == letter
					);
					btn->set_enabled(world_->steps() == 0); // TODO: allow to change untouched tiles
				});
			}
		}

		if (editorMode_) {
			// Add editor buttons
			// - set map size (x,y,z)
			// - add platform
			// - move platform (6 directions)
			// - delete platform
			// - add tile
			// - remove tile
			// - change tile type
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

		if (tileHover_) {
			// Draw frame on all z-layers
			if (frameVisibility_) {
				for (int wz = 0; wz <= wmouse_.z; wz++) {
					vp_->Draw(&image::g_frame, Vec3Si32(wmouse_.x, wmouse_.y, wz), 1).Alpha();
				}
			}
			// Show letter on tile to be placed
			if (placeMode_ == kPmRobot) {
				vp_->Draw(&image::g_robot, wmouse_, 3).Alpha().Blend(ui::ActiveColorBlink());
			} else if (placeMode_ == kPmLetter) {
				vp_->Draw(&image::g_letter[placeLetter_], wmouse_, 1).Alpha().Blend(ui::ActiveColorBlink());
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
