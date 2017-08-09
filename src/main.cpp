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
		music::g_background[musicIdx].Play(0.2f);
	}
}

void EasyMain()
{
	Init();
	srand((int)time(nullptr));
	UpdateMusic();

	PlayerProfile profile;
	profile.LoadFromDisk();

	bool exiting = false;
	int level = profile.LastAvailableLevel();
	int prevLevel = 0;
	while (!exiting) {
		Game game;
		if (level >= 0) {
			game.Start(level, prevLevel, profile.LastAvailableLevel(), profile.GetSavedWorld(level));
			if (!profile.IsLevelAvailable(level)) {
				profile.AddLevel(level, game.GetInitWorld());
			}
		}
		else {
			game.Start(level, prevLevel, level, nullptr);
		}
		prevLevel = level;

		while (true) {
			if (!game.Control()) {
				exiting = true;
				break;
			}

			game.Update();
			game.Render();
			if (game.IsComplete()) {
				sfx::g_positive.Play(0.3f);
				level++;
				break;
			}

			int nextLevel = level;
			if (IsKeyOnce(kKeyF2) || game.GetNextLevel() < level) {
				nextLevel = (level > 0 ? level - 1 : level);
			}
			if (IsKeyOnce(kKeyF3) || game.GetNextLevel() > level) {
				nextLevel = (level + 1) % LevelsCount();
			}
			if (nextLevel != level) {
				if (profile.IsLevelAvailable(nextLevel)) {
					level = nextLevel;
					break;
				}
				else {
					sfx::g_negative2.Play();
				}
			}

#ifdef DEV_MODE
			if (IsKeyOnce(kKeyF8)) {
				level = -1;
				break;
			}
			if (IsKeyOnce(kKeyF9)) {
				level++;
				break;
			}
			if (IsKeyOnce(kKeyF10)) {
				level--;
				break;
			}
#endif

			UpdateMusic();
		}

		profile.UpdateLevel(prevLevel, game.GetInitWorld());
		game.Finish(level, prevLevel);
	}
}
