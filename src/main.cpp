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
#include "levels.h"

#include <functional>
#include <list>

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
private:
	enum PlaceMode {
		kPmSelect = 0,
		kPmRobot,
		kPmLetter,
	};

public:
	void Restart()
	{
		world_.reset(initWorld_->Clone());
		vp_->set_world(world_.get());

		lastUpdateTime_ = 0.0;
		lastControlTime_ = 0.0;

		lastProgress_ = 1.0;
		simPaused_ = true;
	}

	void Start(int level, int prevLevel)
	{
		level_ = level;
		prevLevel_ = prevLevel;
		initWorld_.reset(GenerateLevel(level_));
		vp_.reset(new ViewPort(initWorld_.get()));
		Restart();
		DefaultPlaceMode();

		Control();

		if (!disableAnimation_) {
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
			vp_->Center();

			frameVisibility_ = true;
			panelVisibility_ = true;
		}

		MakeTools();
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

		UpdateTools();
	}

	void DrawPanel(Si32 width, Si32 height)
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

	class Button {
	public:
		Button(Si32 ix, Si32 iy, Sprite sprite)
			: ix_(ix), iy_(iy), sprite_(sprite)
		{
			x1_ = g_xcell * ix_;
			y1_ = g_ycell * iy_;
			x2_ = g_xcell * (ix_ + 1) - 1;
			y2_ = g_ycell * (iy_ + 1) - 1;
		}

		Button* Click(std::function<void(Button*)> onClick)
		{
			onClick_ = onClick;
			return this;
		}

		Button* OnUpdate(std::function<void(Button*)> onUpdate)
		{
			onUpdate_ = onUpdate;
			return this;
		}

		Button* HotKey(char hotkey)
		{
			hotkey_ = hotkey;
			return this;
		}

		void Control()
		{
			hover_ =
				ae::MousePos().x >= x1_ + margin_ &&
				ae::MousePos().y >= y1_ + margin_ &&
				ae::MousePos().x <= x2_ - margin_ &&
				ae::MousePos().y <= y2_ - margin_;

			if (enabled_ && onClick_) {
				if ((hover_ && IsKeyOnce(kKeyMouseLeft))
					|| (hotkey_ && IsKeyOnce(hotkey_))) {
					sfx::g_click2.Play();
					onClick_(this);
				}
			}
		}

		void Update()
		{
			if (onUpdate_) {
				onUpdate_(this);
			}
		}

		void Render()
		{
			auto blend = (enabled_ ?
				(hover_ ? Rgba(0x55, 0xff, 0x66, 0x80) : Rgba(0, 0, 0, 0)) :
				Rgba(0x63, 0xa8, 0xdd, 0xff));

			if (frame_) {
				AlphaDrawAndBlend(image::g_button_frame, x1_, y1_, blend);
			}
			AlphaDrawAndBlend(sprite_, x1_, y1_, blend);
		}

		// accessors
		bool frame() const { return frame_; }
		void set_frame(bool frame) { frame_ = frame; }
		Sprite sprite() const { return sprite_; }
		void set_sprite(Sprite sprite) { sprite_ = sprite; }
		bool enabled() const { return enabled_; }
		void set_enabled(bool enabled) { enabled_ = enabled; }

	private:
		Si32 ix_;
		Si32 iy_;
		Si32 x1_;
		Si32 y1_;
		Si32 x2_;
		Si32 y2_;
		Sprite sprite_;
		std::function<void(Button*)> onClick_;
		std::function<void(Button*)> onUpdate_;
		bool hover_ = false;
		bool frame_ = false;
		bool enabled_ = true;
		char hotkey_ = 0;

		static constexpr Si32 margin_ = 8;
	};

	Button* AddButton(Si32 pos, Sprite sprite)
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

	void ControlTools()
	{
		for (Button& button : buttons_) {
			button.Control();
		}
	}

	bool IsMouseInPanel()
	{
		return ae::MousePos().x < g_xcell * panelWidth_ && ae::MousePos().y < g_ycell * panelHeight_;
	}

	void UpdateTools()
	{
		frameVisibility_ = !IsMouseInPanel();
		for (Button& button : buttons_) {
			button.Update();
		}
	}

	void RenderTools()
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

	void PlayOrPause()
	{
		simPaused_ = !simPaused_;
		if (simPaused_ == false) {
			DefaultPlaceMode();
		}
	}

	void DefaultPlaceMode()
	{
		placeMode_ = kPmSelect;
		placeLetter_ = kLtSpace;
	}

	void SwitchPlaceMode(PlaceMode mode, Letter letter)
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

	void MakeTools()
	{
		buttons_.clear();
		panelWidth_ = 5;
		panelHeight_ = 2;

		// Add playback buttons
		AddButton(0, image::g_button_play)->Click([=](Button* btn) {
			PlayOrPause();
		})->OnUpdate([=](Button* btn) {
			btn->set_sprite(simPaused_ ? image::g_button_play : image::g_button_pause);
		});
		
		AddButton(2, image::g_button_stop)->Click([=](Button* btn) {
			Restart();
		})->OnUpdate([=](Button* btn) {
			btn->set_enabled(world_->steps() != 0);
		});

		// Add robot button
		AddButton(3, image::g_button_robot)->Click([=](Button* btn) {
			SwitchPlaceMode(kPmRobot, kLtSpace);
		})->OnUpdate([=](Button* btn) {
			btn->set_frame(placeMode_ == kPmRobot);
			btn->set_enabled(world_->steps() == 0);
		});

		Si32 btnPos = 4;

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

	Sprite& BgForLevel(int level)
	{
		return image::g_background[abs(level) % image::g_backgroundCount];
	}

	void Render()
	{
		BgForLevel(level_).Draw(0, 0);
		if (bgTransition_ > 0.0f) {
			DrawWithFixedAlphaBlend(BgForLevel(prevLevel_), 0, 0, Ui8(bgTransition_ * 255));
		}

		vp_->BeginRender(Time());
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

		ShowFrame();
	}

	bool IsComplete()
	{
		return world_->IsOutputCorrect();
	}

	void Finish()
	{
		if (!disableAnimation_) {
			int dx0 = Pos::dx;
			int dy0 = Pos::dy;
			int dz0 = Pos::dz;

			float dx = (float)Pos::dx;
			float dy = (float)Pos::dy;
			float dz = (float)Pos::dz;

			frameVisibility_ = false;
			panelVisibility_ = false;

			//float speed = 1.01;
			for (int i = 0; i < 10; i++) {
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
			for (int i = 0; i < 50; i++) {
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
	}

private:
	// control configuration
	float movePxlPerSec_ = float(screen::h) * 0.50f;
	Si32 mouseScrollMargin_ = 5;
	
	// world
	int level_ = 0;
	int prevLevel_ = 0;
	float bgTransition_ = 0.0f;
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
	bool tileHover_ = false;
	Vec3Si32 wmouse_;
	bool frameVisibility_ = true;
	bool panelVisibility_ = true;
	Si32 panelWidth_;
	Si32 panelHeight_;	
	std::list<Button> buttons_;
	PlaceMode placeMode_;
	Letter placeLetter_;

	// debug
#ifdef DEV_MODE
	bool disableAnimation_ = true;
#else
	bool disableAnimation_ = false;
#endif
};

void Init()
{
	InitData();
	ResizeScreen(screen::w, screen::h);
}

void UpdateMusic()
{
	static int musicIdx = 0;

	// switch background music tracks
	if (!music::g_background[musicIdx].IsPlaying()) {
		musicIdx = (musicIdx + 1) % music::g_backgroundCount;
		music::g_background[musicIdx].Play();
	}
}

void EasyMain()
{
	Init();
	srand((int)time(nullptr));

	bool exiting = false;
	int level = 0;
	int prevLevel = 0;
	while (!exiting) {
		Game game;
		game.Start(level, prevLevel);
		prevLevel = level;

		while (true) {
			if (!game.Control()) {
				exiting = true;
				break;
			}

			if (IsKeyOnce(kKeyF1)) {
				level = -1;
				break;
			}

			game.Update();
			game.Render();
			if (game.IsComplete()) {
				sfx::g_positive.Play();
				level++;
				break;
			}
#ifdef DEV_MODE
			if (IsKeyOnce(kKeyF3)) {
				level++;
				break;
			}
			if (IsKeyOnce(kKeyF2)) {
				level--;
				break;
			}
#endif

			UpdateMusic();
		}

		game.Finish();
	}
}
