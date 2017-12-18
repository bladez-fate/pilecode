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
#include "music.h"
#include "pilecode.h"
#include "ui.h"

#include <list>

namespace pilecode {

    enum SceneTransition {
        kStDisabled,
        kStFly,
        kStFade,
    };
    
	class Game {
	private:
		enum PlaceMode {
			kPmNone = 0,
			kPmRobot,
			kPmLetter,
		};
        
	public:
        void Replay();
		void Restart();
        void SetTransition(SceneTransition transition);
		void ForwardStartTransition();
		void ForwardFinishTransition();
		void BackwardFinishTransition();
		void BackwardStartTransition();
        void FadeStartTransition();
        void FadeFinishTransition();
		void StartWithEditor(int level, int prevLevel, int maxLevel, World* savedWorld);
		void Start(int level, int prevLevel, int maxLevel, World* savedWorld);
		void Finish(int level, int prevLevel);
		void Response(ResultBase status);
        bool Control();
		void Update();
		bool ControlTools();
		void UpdateTools();
		void RenderTools();
        void PlayOrPause();
        void FastForward();
        void DefaultPlaceMode();
		void SwitchPlaceMode(PlaceMode mode, Letter right, Letter down, Letter up, Letter left);
		void SwitchPlaceMode(PlaceMode mode, Letter letter);
		Letter PlaceLetter(Vec2F tp, Letter right, Letter down, Letter up, Letter left);
		void MakeTools();
		Sprite& BgForLevel(int level);
		void Render(bool show = true);
		bool IsComplete();
		World* GetInitWorld() const;
		int GetNextLevel() const;

	private:
		template <class... Args>
		Button* AddButton(Args... args)
		{
			buttons_.emplace_back(args...);
			Button* btn = &buttons_.back();
			return btn;
		}

	private:
		// control configuration
		float movePxlPerSec_ = float(screen::h) * 0.50f;
		Si32 mouseScrollMargin_ = 5;

		// mode
		bool editorMode_ = false;

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
        bool fastForward_ = false;
		double simSpeed_ = 1.0;

		// gameplay
		bool tileHover_ = false;
		double responseDeadline_ = 0.0;
		Vec3Si32 wmouse_;
		Vec2F tilePos_;
		bool frameVisibility_ = true;
		bool toolsVisibility_ = true;
		std::list<Button> buttons_;
		PlaceMode placeMode_;
		Letter placeLetterRight_;
		Letter placeLetterDown_;
		Letter placeLetterUp_;
		Letter placeLetterLeft_;
		Letter placeLetter_; // with respect to current mouse position 

		// transition
        SceneTransition transition_ = kStDisabled;
	};

}
