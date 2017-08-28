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

#pragma once

#include "defs.h"
#include "data.h"
#include "graphics.h"
#include "pilecode.h"
#include "ui.h"

#include <list>

namespace pilecode {

	class Game {
	private:
		enum PlaceMode {
			kPmSelect = 0,
			kPmRobot,
			kPmLetter,
		};

	public:
		void Restart();
		void ForwardStartTransition();
		void ForwardFinishTransition();
		void BackwardFinishTransition();
		void BackwardStartTransition();
		void StartWithEditor(int level, int prevLevel, int maxLevel, World* savedWorld);
		void Start(int level, int prevLevel, int maxLevel, World* savedWorld);
		void Finish(int level, int prevLevel);
		bool Control();
		void Update();
		void DrawPanel(Si32 width, Si32 height);
		Button* AddButton(Si32 pos, Sprite sprite);
		void ControlTools();
		bool IsMouseInPanel();
		void UpdateTools();
		void RenderTools();
		void PlayOrPause();
		void DefaultPlaceMode();
		void SwitchPlaceMode(PlaceMode mode, Letter letter);
		void MakeTools();
		Sprite& BgForLevel(int level);
		void Render();
		bool IsComplete();
		World* GetInitWorld() const;
		int GetNextLevel() const;

	private:
		// control configuration
		float movePxlPerSec_ = float(screen::h) * 0.50f;
		Si32 mouseScrollMargin_ = 5;

		// mode
		bool editorMode = false;

		// levels and transitions
		int level_ = 0;
		int prevLevel_ = 0;
		int nextLevel_ = 0;
		int maxLevel_ = 0;
		float bgTransition_ = 0.0f;

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
		bool disableTransition_ = true;
#else
		bool disableTransition_ = false;
#endif
	};

}