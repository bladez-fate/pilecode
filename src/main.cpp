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
#include "music.h"
#include "pilecode.h"
#include "data.h"
#include "levels.h"

#include <functional>
#include <fstream>

using namespace arctic;  // NOLINT
using namespace arctic::easy;  // NOLINT
using namespace pilecode; // NOLINT

class PlayerProfile {
public:
	World* GetSavedWorld(int level)
	{
		if (IsLevelAvailable(level)) {
			return level_[level]->Clone();
		}
		else {
			return nullptr;
		}
	}

	void UpdateLevel(int level, World* world)
	{
		if (level < level_.size()) {
			level_[level] = std::shared_ptr<World>(world);
		}
		SaveToDisk();
	}

	void AddLevel(int level, World* world)
	{
		if (level == level_.size()) {
			level_.emplace_back(world);
		}
		SaveToDisk();
	}

	void SaveTo(std::ostream& s) const
	{
		Save<size_t>(s, level_.size());
		for (const auto& l : level_) {
			l->SaveTo(s);
		}
	}

	void LoadFrom(std::istream& s)
	{
		size_t levels;
		Load<size_t>(s, levels);
		level_.resize(levels);
		for (auto& l : level_) {
			l.reset(new World());
			l->LoadFrom(s);
		}
	}

	void SaveToDisk() const
	{
		std::ofstream ofs("profile.sav");
		SaveTo(ofs);
	}

	void LoadFromDisk()
	{
		std::ifstream ifs("profile.sav");
		if (ifs.good()) {
			LoadFrom(ifs);
		}
	}

	bool IsLevelAvailable(int level) const
	{
		return (size_t)level < level_.size();
	}

	int LastAvailableLevel() const
	{
		return int(level_.empty() ? 0 : level_.size() - 1);
	}
private:
	std::vector<std::shared_ptr<World>> level_;
};

PlayerProfile g_profile;

class IScene {
public:
	virtual ~IScene() {}
	virtual IScene* Run() = 0;
    void set_transition(SceneTransition transition) { transition_ = transition; }
    SceneTransition transition() const { return transition_; }
private:
#ifdef DEV_MODE
    SceneTransition transition_ = kStDisabled;
#else
    SceneTransition transition_ = kStFly;
#endif
};

class GameScene : public IScene {
public:
	GameScene(int _level, int _prevLevel)
		: level_(_level)
		, prevLevel_(_prevLevel)
	{}
	
	IScene* Run() override;

private:
	bool exiting = false;
	int level_ = 0;
	int prevLevel_ = 0;
};

class EditorScene : public IScene {
public:
	explicit EditorScene(int _level)
		: level_(_level)
	{}

	IScene* Run() override;

private:
	bool exiting = false;
	int level_ = 0;
	int prevLevel_ = 0;
};

IScene* GameScene::Run()
{
	Game game;
    game.SetTransition(transition());
	if (level_ >= 0) {
		game.Start(level_, prevLevel_, g_profile.LastAvailableLevel(), g_profile.GetSavedWorld(level_));
		if (!g_profile.IsLevelAvailable(level_)) {
			g_profile.AddLevel(level_, game.GetInitWorld());
		}
	}
	else {
		game.Start(level_, prevLevel_, level_, nullptr);
	}
	prevLevel_ = level_;

	while (true) {
		if (!game.Control()) {
			exiting = true;
			break;
		}

        if (screen::CheckResize()) {
            game.SetTransition(kStDisabled);
            break;
        }
        
		game.Update();
		game.Render();
		if (game.IsComplete()) {
			sfx::g_positive.Play(0.3f);
			level_++;
			break;
		}

		int nextLevel = level_;
		if (game.GetNextLevel() < level_) {
			nextLevel = (level_ > 0 ? level_ - 1 : level_);
		}
		if (game.GetNextLevel() > level_) {
			nextLevel = (level_ + 1) % LevelsCount();
		}
		if (nextLevel != level_) {
			if (g_profile.IsLevelAvailable(nextLevel)) {
				level_ = nextLevel;
				break;
			}
			else {
				sfx::g_negative2.Play();
			}
		}

#ifdef DEV_MODE
		if (IsKeyOnce(kKeyF4)) {
			return new EditorScene(level_);
		}

		if (IsKeyOnce(kKeyF8)) {
			level_ = -1;
			break;
		}
		if (IsKeyOnce(kKeyF9)) {
			level_++;
			break;
		}
		if (IsKeyOnce(kKeyF10)) {
			level_--;
			break;
		}
#endif

		UpdateMusic();
	}

	g_profile.UpdateLevel(prevLevel_, game.GetInitWorld());
	game.Finish(level_, prevLevel_);

	if (exiting) {
		return nullptr;
	}
	else {
		return new GameScene(level_, prevLevel_);
	}
}

IScene* EditorScene::Run()
{
	Game game;
    game.SetTransition(transition());
	if (level_ >= 0) {
		game.StartWithEditor(level_, level_, g_profile.LastAvailableLevel(), g_profile.GetSavedWorld(level_));
	}
	else {
		game.StartWithEditor(level_, level_, level_, nullptr);
	}

	while (true) {
		if (!game.Control()) {
			exiting = true;
			break;
		}

        if (screen::CheckResize()) {
            game.SetTransition(kStDisabled);
            break;
        }

		game.Update();
		game.Render();

		int nextLevel = level_;
		if (game.GetNextLevel() < level_) {
			nextLevel = (level_ > 0 ? level_ - 1 : level_);
		}
		if (game.GetNextLevel() > level_) {
			nextLevel = (level_ + 1) % LevelsCount();
		}
		if (nextLevel != level_) {
			level_ = nextLevel;
			break;
		}

		UpdateMusic();
	}

	g_profile.UpdateLevel(prevLevel_, game.GetInitWorld());
	game.Finish(level_, prevLevel_);

	if (exiting) {
		return new GameScene(level_, level_);
	}
	else {
		return new EditorScene(level_);
	}
}

void DrawIntro()
{
	DrawWithFixedAlphaBlend(image::g_introBackground, 0, 0, Ui8(255));
	Region pos = Region::FullScreen().Place(kCenter, image::g_pilecode.Size());
	AlphaDraw(image::g_pilecode, pos.x1(), pos.y1());
}

void Intro()
{
	double transitionSec = 1.2f;

	// Show Intro
	double startTime = ae::Time();
	while (true) {
		if (ui::StopAnimationKey()) {
			break;
		}
		DrawIntro();
		Ui8 brightness = Ui8(ae::Clamp(float((ae::Time() - startTime) * 255 / transitionSec), 0.0f, 255.0f));
		FilterBrightness(ae::GetEngine()->GetBackbuffer(), brightness);
		ae::ShowFrame();
		Sleep(0.010);
	}

	// Hide intro
	double showedTime = ae::Time() - startTime;
	double finishTime = ae::Time() + std::min(transitionSec, showedTime);
	while (ae::Time() < finishTime) {
		DrawIntro();
		Ui8 brightness = Ui8(ae::Clamp(float((finishTime - ae::Time()) * 255 / transitionSec), 0.0f, 255.0f));
		FilterBrightness(ae::GetEngine()->GetBackbuffer(), brightness);
		ae::ShowFrame();
		Sleep(0.010);
	}
	Sleep(0.100);
}

void EasyMain()
{
	// Init system stuff
	srand((int)time(nullptr));

	// Init game
    screen::Init();
	InitData();
	g_profile.LoadFromDisk();

	Intro();

	// Run game
	UpdateMusic();
	IScene* scene = new GameScene(g_profile.LastAvailableLevel(), 0);
    scene->set_transition(kStFade); // Fade transition into first screen
	
    while (scene) {
        if (screen::CheckResize()) {
            screen::Init();
            InitData();
            
            // Do not start scene if window size keeps changing
            Sleep(0.5);
            if (screen::CheckResize()) {
                continue;
            }
            scene->set_transition(kStFade);
        }

		IScene* nextScene = scene->Run();
		delete scene;
		scene = nextScene;
	}
}
