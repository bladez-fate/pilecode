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
#include "music.h"

#include <initializer_list>

namespace pilecode {

	namespace ui {
		inline Rgba HoverColor()
		{
			return Rgba(0x55, 0xff, 0x66, 0x80);
		}

		inline Rgba ContourColor()
		{
			return Rgba(0xff, 0x55, 0xff, 0xff);
		}

		inline Rgba DisabledColor()
		{
			return Rgba(0x63, 0xa8, 0xdd, 0xff);
		}

		inline Rgba PlaceColorBlink()
		{
			double alpha = (0.5 + 0.5 * sin(ae::Time() * 10));
			return Rgba(0x55, 0xff, 0x66, Ui8(0xbb * alpha));
		}

		inline Rgba EraseColorBlink()
		{
			double alpha = (0.5 + 0.5 * sin(ae::Time() * 10));
			return Rgba(0xff, 0x33, 0x44, Ui8(0xbb * alpha));
		}

		inline Rgba ForbidColorBlink()
		{
			return Rgba(0xff, 0x33, 0x44, 0xff);
		}

		inline Ui8 ForbidOpacityBlink()
		{
			double alpha = std::max(0.0, 1.0 * sin(ae::Time() * 10));
			return Ui8(0xaa * alpha);
		}

		inline bool StopAnimationKey()
		{
			return IsKeyOnce(ae::kKeyMouseLeft)
				|| IsKeyOnce(ae::kKeyEscape)
				|| IsKeyOnce(ae::kKeyEnter)
				|| IsKeyOnce(ae::kKeySpace)
                || screen::CheckResize()
				;
		}

        struct Snowflake {
            Sprite sprite;
            Vec2F r;
            Si32 seed;
            
            Snowflake()
            {
                seed = rand();
                sprite = RandomSprite();
                r = RandomPosition();
            }
            
            Sprite RandomSprite()
            {
                Sprite s = image::g_snowflake[rand() % image::g_snowflakeCount];
                s.SetPivot(Vec2Si32(s.Width() / 2, s.Height() / 2));
                return s;
            }
            
            Vec2F RandomPosition()
            {
                return Vec2F(
                    float(rand() % screen::w),
                    float(rand() % screen::h)
                );
            }

            Vec2F RandomPositionOnTop()
            {
                return Vec2F(
                    float(rand() % screen::w),
                    float(screen::h + sprite.Height() / 2)
                );
            }

            void Redrop()
            {
                seed = rand();
                sprite = RandomSprite();
                r = RandomPositionOnTop();
            }
            
            void Update(double time, double delta)
            {
                // Compute speed field at position r
                Si32 len = std::max(screen::w, screen::h);
                Vec2F v = Vec2F(
                    float(sin((r.x + r.y)*10.0/len + time/5.0 + seed % 10000 / 10000.0 * 2.0 * M_PI) *
                    sin((r.x - r.y)*10.0/len + time/5.0) +
                    2.0 * pow(sin((r.x/len + time)/4.0), 9)),
                    float(-1.0 + (rand() % 500 / 1000.0) *
                    sin((r.x + 2 * r.y)*10.0/screen::h + time/10.0))
                );
                
                // Integrate
                r += v * float(len) / 16.0f * float(delta);
                
                // Restart snowflakes if it moves out of the screen
                if (r.y > screen::h + sprite.Height() || r.y < -sprite.Height()) {
                    Redrop();
                }
                if (r.x > screen::w + sprite.Width()) {
                    r.x -= screen::w + 1.5f * sprite.Width();
                }
                if (r.x < -sprite.Width()) {
                    r.x += screen::w + 1.5f * sprite.Width();
                }
            }
            
            void Render()
            {
                sprite.Draw(Si32(r.x), Si32(r.y));
            }
            
            static void Run()
            {
                // Init
                constexpr Si32 size = 42;
                static Snowflake snowflake[size];
                static double time = ae::Time();

                // Check for resize
                static Vec2Si32 screen = ae::ScreenSize();
                Vec2Si32 newScreen = ae::ScreenSize();
                if (screen != newScreen) {
                    screen = newScreen;
                    for (Snowflake& sf : snowflake) {
                        sf = Snowflake();
                    }
                }

                // Step
                double newTime = ae::Time();
                double delta = newTime - time;
                for (Snowflake& sf : snowflake) {
                    sf.Update(time, delta);
                }
                time = newTime;
                
                // Render
                for (Snowflake& sf : snowflake) {
                    sf.Render();
                }
            }
        };

        inline void RenderBgParticles()
        {
#ifdef MOD_XMAS
            Snowflake::Run();
#endif
        }
	}

	enum Align : Si32 {
		kLeftTop = 6, kCenterTop = 7, kRightTop = 8,
		kLeftCenter = 3, kCenter = 4, kRightCenter = 5,
		kLeftBottom = 0, kCenterBottom = 1, kRightBottom = 2
	};

	struct Region {
		Vec2Si32 p1;
		Vec2Si32 p2;

		Region() {}

		explicit Region(Si32 x1, Si32 y1, Si32 x2, Si32 y2)
			: p1(x1, y1)
			, p2(x2, y2)
		{}

		Si32 left() const { return p1.x; }
		Si32 bottom() const { return p1.y; }
		Si32 right() const { return p2.x; }
		Si32 top() const { return p2.y; }

		Si32 x1() const { return p1.x; }
		Si32 y1() const { return p1.y; }
		Si32 x2() const { return p2.x; }
		Si32 y2() const { return p2.y; }

		Si32 width() const { return x2() - x1(); }
		Si32 height() const { return y2() - y1(); }

		Vec2Si32 size() const { return Vec2Si32(width(), height()); }

		void assign(const Region& o)
		{
			*this = o;
		}

		Region Place(Align align, Vec2Si32 size) const
		{
			// Alignment point position
			Si32 xa = align % 3,
				 ya = align / 3;
			Si32 xc = x1() + xa * (width() / 2),
				 yc = y1() + ya * (height() / 2);

			// leftbottom corner
			Si32 x1 = xc - xa * (size.x / 2),
		 		 y1 = yc - ya * (size.y / 2);

			return Region(x1, y1, x1 + size.x, y1 + size.y);
		}

		Region Place(Align align, Sprite sprite) const
		{
			return Place(align, sprite.Size());
		}

		Region Offset(Si32 dx, Si32 dy) const
		{
			return Region(x1() + dx, y1() + dy, x2() + dx, y2() + dy);
		}

		Region Outset(Si32 outset) const
		{
			return Region(x1() - outset, y1() - outset, x2() + outset, y2() + outset);
		}

		Region Inset(Si32 inset) const
		{
			return Outset(-inset);
		}

		static Region Screen()
		{
			return Region(
				ui::g_marginLeft,
				ui::g_marginBottom,
				screen::w - ui::g_marginRight,
				screen::h - ui::g_marginTop
			);
		}

		static Region FullScreen()
		{
			return Region(0, 0,	screen::w, screen::h);
		}
	};

	class GridFrame: public Region {
	public:
		GridFrame(Align align, Si32 xSize, Si32 ySize, Si32 btnWidth, Si32 btnHeight, Si32 xSpacing, Si32 ySpacing)
			: btnWidth_(btnWidth), btnHeight_(btnHeight)
			, xSpacing_(xSpacing), ySpacing_(ySpacing)
		{
			// Frame size
			Si32 w = xSize * (btnWidth  + xSpacing) - xSpacing,
			     h = ySize * (btnHeight + ySpacing) - ySpacing;

			assign(Region::Screen().Place(align, Vec2Si32(w, h)));
		}

		Region Place(Si32 xPos, Si32 yPos)
		{
			Region r;
			r.p1.x = x1() + xPos * (btnWidth_  + xSpacing_);
			r.p1.y = y1() + yPos * (btnHeight_ + ySpacing_);
			r.p2.x = r.p1.x + btnWidth_;
			r.p2.y = r.p1.y + btnHeight_;
			return r;
		}

	private:
		Si32 btnWidth_;
		Si32 btnHeight_;
		Si32 xSpacing_;
		Si32 ySpacing_;
	};

	class HorizontalFluidFrame: public Region {
	public:
		HorizontalFluidFrame(Align align, Si32 xSpacing, Region parent = Region::Screen())
			: parent_(parent)
			, align_(align)
			, xSpacing_(xSpacing)
		{}

		HorizontalFluidFrame& Add(Vec2Si32 size)
		{
			elements_.emplace_back(size);
			Update();
			return *this;
		}

		HorizontalFluidFrame& Add(Sprite sprite)
		{
			return Add(sprite.Size());
		}

		HorizontalFluidFrame& Add(Si32 spacing)
		{
			return Add(Vec2Si32(spacing, 0));
		}

		Region Place(Si32 pos)
		{
			Region r;
			r.p1.x = x1();
			r.p1.y = y1();

			Si32 dx = 0;
			for (Vec2Si32 e : elements_) {
				if (pos-- == 0) {
					dx = e.x;
					break;
				}
				else {
					r.p1.x += e.x + xSpacing_;
				}
			}

			r.p2.x = r.p1.x + dx;
			r.p2.y = r.p1.y + height();

			return r;
		}

	private:
		void Update()
		{
			// Recompute size
			Si32 w = -xSpacing_, h = 1;
			for (Vec2Si32 e : elements_) {
				w += e.x + xSpacing_;
				h = std::max(h, e.y);
			}

			assign(parent_.Place(align_, Vec2Si32(w, h)));
		}

	private:
		Region parent_;
		Align align_;
		Si32 xSpacing_;
		std::vector<Vec2Si32> elements_;
	};


	class VerticalFluidFrame : public Region {
	public:
		VerticalFluidFrame(Align align, Si32 ySpacing, Region parent = Region::Screen())
			: parent_(parent)
			, align_(align)
			, ySpacing_(ySpacing)
		{}

		VerticalFluidFrame& Add(Vec2Si32 size)
		{
			elements_.emplace_back(size);
			Update();
			return *this;
		}

		VerticalFluidFrame& Add(Sprite sprite)
		{
			return Add(sprite.Size());
		}

		VerticalFluidFrame& Add(Si32 spacing)
		{
			return Add(Vec2Si32(spacing, 0));
		}

		Region Place(Si32 pos)
		{
			Region r;
			r.p1.x = x1();
			r.p1.y = y1();

			Si32 dy = 0;
			for (Vec2Si32 e : elements_) {
				if (pos-- == 0) {
					dy = e.y;
					break;
				}
				else {
					r.p1.y += e.y + ySpacing_;
				}
			}

			r.p2.x = r.p1.x + width();
			r.p2.y = r.p1.y + dy;

			return r;
		}

	private:
		void Update()
		{
			// Recompute size
			Si32 w = 1, h = -ySpacing_;
			for (Vec2Si32 e : elements_) {
				w = std::max(w, e.x);
				h += e.y + ySpacing_;
			}

			assign(parent_.Place(align_, Vec2Si32(w, h)));
		}

	private:
		Region parent_;
		Align align_;
		Si32 ySpacing_;
		std::vector<Vec2Si32> elements_;
	};

	class PButton {
	public:
		PButton(Sprite sprite, Region region, Align align = kCenter)
			: reg_(region.Place(align, sprite))
		{
			SetSprite(sprite);
		}

        PButton(std::initializer_list<Sprite> sprites, Region region, Align align = kCenter)
            : reg_(region.Place(align, *sprites.begin()))
        {
            Si32 i = 0;
            for (Sprite sprite : sprites) {
                AddSprite(sprite, i++);
            }
            SetSprite(0);
        }

		PButton* Click(std::function<void(PButton*)> onClick)
		{
			onClick_ = onClick;
			return this;
		}

		PButton* OnUpdate(std::function<void(PButton*)> onUpdate)
		{
			onUpdate_ = onUpdate;
			return this;
		}

		PButton* HotKey(char hotkey)
		{
			hotkey_ = hotkey;
			return this;
		}

		PButton* Padding(Si32 padding)
		{
			padding_ = padding;
			return this;
		}

		PButton* Color(Rgba color)
		{
			set_color(color);
			return this;
		}

		PButton* Hidden()
		{
			set_visible(false);
			return this;
		}

        PButton* HoverUseMask()
        {
            hoverUseMask_ = true;
            return this;
        }
        
		bool Control()
		{
            Vec2Si32 s = ae::MousePos();
            if (hoverUseMask_) {
                // Calculate sprite coordinates
                Vec2Si32 r = s - reg_.p1 + sprite_.Pivot();
                if (r.x >= 0 && r.y < sprite_.Width() && r.y >= 0 && r.y < sprite_.Height()) {
                    Rgba* to = sprite_.RgbaData()
                        + r.y * sprite_.StridePixels()
                        + r.x;
                    hoverNext_ = to->a > 0x80;
                } else {
                    hoverNext_ = false;
                }
            }
            else {
                hoverNext_ =
                    s.x >= reg_.x1() + padding_ &&
                    s.y >= reg_.y1() + padding_ &&
                    s.x <  reg_.x2() - padding_ &&
                    s.y <  reg_.y2() - padding_;
            }
            
            click_ = false;
			if (enabled_) {
				if ((visible_ && hoverNext_ && IsKeyOnce(ae::kKeyMouseLeft))
					|| (hotkey_ && IsKeyOnce(hotkey_))) {
					sfx::g_click2.Play();
                    click_ = true;
                    if (onClick_) {
                        onClick_(this);
                    }
				}
			}

			// Continue checks if button is not hovered
			return !enabled_ || !hoverNext_;
		}

		void Update()
		{
			hover_ = hoverNext_;
			hoverNext_ = false;
            if (onUpdate_) {
				onUpdate_(this);
			}
		}

		void Render()
		{
			if (enabled_ && visible_) {
				auto blend = (hover_ ? ui::HoverColor() : color_);
                if (opacity_ == 0xff) {
                    AlphaDrawAndBlend(shadow_, reg_.x1(), reg_.y1(), Rgba(0, 0, 0, 0xff));
                }
				if (contour_) {
					AlphaDrawAndBlend(contourSprite_, reg_.x1(), reg_.y1(), ui::ContourColor());
				}
				AlphaDrawAndBlend(sprite_, reg_.x1(), reg_.y1(), blend, opacity_);
			}
		}

        void SetSprite(Sprite sprite)
        {
            AddSprite(sprite, 0);
            SetSprite(0);
        }
        
		void AddSprite(Sprite sprite, Si32 index)
		{
            if (index >= spriteArray_.size()) {
                spriteArray_.resize(index + 1);
                shadowArray_.resize(index + 1);
                contourSpriteArray_.resize(index + 1);
            }
			spriteArray_[index] = sprite;
			Si32 size = std::min(sprite.Width(), sprite.Height());
			Si32 shadowBlur = size > 64 ? 17 : 9;
			Si32 contourBlur = size > 64 ? 12 : 6;
			shadowArray_[index] = CreateShadow(sprite, 1, shadowBlur, Rgba(0, 0, 0, 0x80));
			contourSpriteArray_[index] = CreateBoundary(sprite,
				0, 0, contourBlur, contourBlur,
				0x40, 0xff, 2, 8,
				Rgba(0xff, 0xff, 0xff, 0xff));
		}
        
        void SetSprite(Si32 index)
        {
            sprite_ = spriteArray_[index];
            shadow_ = shadowArray_[index];
            contourSprite_ = contourSpriteArray_[index];
        }

		// accessors
		bool contour() const { return contour_; }
		void set_contour(bool value) { contour_ = value; }
		Sprite sprite() const { return sprite_; }
		bool enabled() const { return enabled_; }
		void set_enabled(bool value) { enabled_ = value; }
		bool visible() const { return visible_; }
		void set_visible(bool value) { visible_ = value; }
		Rgba color() const { return color_; }
		void set_color(Rgba value) { color_ = value; }
        bool hover() const { return hover_; }
        bool click() const { return click_; }
        float opacity() const { return opacity_; }
        void set_opacity(Ui8 value) { opacity_ = value; }

	private:
		Region reg_;
		Sprite sprite_;
		Sprite shadow_;
		Sprite contourSprite_;
        std::vector<Sprite> spriteArray_;
        std::vector<Sprite> shadowArray_;
        std::vector<Sprite> contourSpriteArray_;
		std::function<void(PButton*)> onClick_;
		std::function<void(PButton*)> onUpdate_;
		bool hover_ = false;
		bool hoverNext_ = false;
        bool click_ = false;
        bool contour_ = false;
		bool enabled_ = true;
		bool visible_ = true;
		char hotkey_ = 0;
		Si32 padding_ = 8;
		Rgba color_ = Rgba(0, 0, 0, 0);
        Ui8 opacity_ = 0xff;
        bool hoverUseMask_ = false;
	};

    inline Sprite MakeBgForModal()
    {
        // Clone backbuffer into bg sprite and do some filters
        Sprite bg;
        bg.Clone(ae::GetEngine()->GetBackbuffer());
        FilterSB(bg, 0.5f, 0.8f);
        
        return bg;
    }
    
    inline bool ConfirmModal()
    {
        Sprite bg = MakeBgForModal();
        
        // Create buttons for YES and NO
        HorizontalFluidFrame frm(kCenter, 128);
        frm
            .Add(image::g_button_cancel)
            .Add(image::g_button_checked)
        ;
        std::unique_ptr<PButton> btn0(new PButton(image::g_button_cancel, frm.Place(0)));
        std::unique_ptr<PButton> btn1(new PButton(image::g_button_checked, frm.Place(1)));
        btn0->HotKey('N');
        btn1->HotKey('Y');
        
        // Wait user action
        while (true) {
            btn0->Control();
            btn1->Control();
            if (btn0->click() || IsKeyOnce(ae::kKeyEscape)) {
                return false;
            }
            if (btn1->click() || IsKeyOnce(ae::kKeyEnter)) {
                return true;
            }

            btn0->Update();
            btn1->Update();
            
            bg.Draw(0, 0, bg.Width(), bg.Height(),
                    0, 0, bg.Width(), bg.Height(),
                    ae::GetEngine()->GetBackbuffer(), ae::kCopyRgba);
            
            btn0->Render();
            btn1->Render();
            ae::ShowFrame();
           
            ae::Sleep(0.01);

            UpdateMusic();
        }
        
        return false;
    }

    inline void SpriteModal(Sprite sprite)
    {
        Sprite bg = MakeBgForModal();

        // Wait user action
        while (true) {
            if (IsKeyOnce(ae::kKeyEscape) || IsKeyOnce(ae::kKeyEnter) || IsKeyOnce(ae::kKeyMouseLeft)) {
                return;
            }
            
            bg.Draw(0, 0, bg.Width(), bg.Height(),
                    0, 0, bg.Width(), bg.Height(),
                    ae::GetEngine()->GetBackbuffer(), ae::kCopyRgba);
            
            Region pos = Region::FullScreen().Place(kCenter, sprite.Size());
            AlphaDraw(sprite, pos.x1(), pos.y1());

            ae::ShowFrame();
            ae::Sleep(0.01);
        }
    }
}
