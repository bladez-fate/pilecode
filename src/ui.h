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

namespace pilecode {

	namespace ui {
		inline Rgba HoverColor()
		{
			return Rgba(0x55, 0xff, 0x66, 0x80);
		}

		inline Rgba DisabledColor()
		{
			return Rgba(0x63, 0xa8, 0xdd, 0xff);
		}

		inline Rgba ActiveColorBlink()
		{
			double alpha = (0.5 + 0.5 * sin(ae::Time() * 20));
			return Rgba(0x55, 0xff, 0x66, Ui8(0xbb * alpha));
		}
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
				if ((hover_ && IsKeyOnce(ae::kKeyMouseLeft))
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
			auto blend = (enabled_ ? (hover_ ? ui::HoverColor() : Rgba(0, 0, 0, 0)) : ui::DisabledColor());
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

}