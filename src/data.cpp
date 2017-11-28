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

#include <unordered_map>

namespace pilecode {

	const Vec2Si32 g_tileCenter(64, 54);

	namespace image {
		Sprite g_pilecode;

		Sprite g_empty;

		Sprite g_tile[kTlMax];
		Sprite g_letter[kLtMax];
		Sprite g_frame;
		Sprite g_tileMask;

		Sprite g_robot;
		Sprite g_robotShadow;

		Sprite g_button_frame;
		Sprite g_button_musicalnote;
		Sprite g_button_nextlevel;
		Sprite g_button_prevlevel;
		Sprite g_button_play;
		Sprite g_button_pause;
		Sprite g_button_rewind;
		Sprite g_button_fastforward;
		Sprite g_button_replay;
		Sprite g_button_minus;
		Sprite g_button_plus;
		Sprite g_button_x1;
		Sprite g_button_x2;
		Sprite g_button_x4;
		Sprite g_button_x8;
		Sprite g_button_robot;
		Sprite g_button_letter[kLtMax];

		Sprite g_background[g_backgroundCount];
	}

	namespace music {
		Sound g_background[g_backgroundCount];
	}

	namespace sfx {
		Sound g_click;
		Sound g_click2;
		Sound g_positive;
		Sound g_negative2;
		Sound g_read;
		Sound g_write;
	}

	void CreateBackground(Sprite& bgSprite, Rgba c1, Rgba c2)
	{
		bgSprite.Create(screen::w, screen::h);
		bgSprite.SetPivot(Vec2Si32(0, 0));

		Rgba* p = bgSprite.RgbaData();
		Si64 rsqMax = 0;
		for (Si32 y = 0; y < screen::h; y++) {
			for (Si32 x = 0; x < screen::w; x++, p++) {
				Si64 rsq = (x - screen::cx)*(x - screen::cx) + (y - screen::cy)*(y - screen::cy);
				if (!rsqMax) {
					rsqMax = rsq;
				}
				Ui32 alpha = Ui32(rsq * 256 / rsqMax);
				*p = RgbaSum(
					RgbaMult(c2, alpha),
					RgbaMult(c1, 256 - alpha)
				);
				p->a = 255;
			}
		}
	}

	// Creates shadow using series of transformations:
	// - use `sprite' alpha-channel as source (RGB channels are ignored)
	// - expand it by `expandRadius' pixels (using square shape)
	// - then apply gaussian blur with `blurRadius' radius in pixels
	// - apply given `color' to result (note that `color.a' is maximum shadow opacity)
	Sprite CreateShadow(Sprite sprite, Si32 blurRadius, Si32 expandRadius, Rgba color)
	{
		Si32 totalRadius = blurRadius + expandRadius;

		Sprite shadow;
		shadow.Create(sprite.Width() + 2 * totalRadius, sprite.Height() + 2 * totalRadius);
		shadow.SetPivot(Vec2Si32(totalRadius, totalRadius));

		// Gaussian blur weight map (with cache)
		static std::unordered_map<Si32, std::pair<Sprite, Ui32>> cache;
		auto& cv = cache[blurRadius];
		Sprite& weight = cv.first;
		Ui32& totalWeight = cv.second;
		if (totalWeight == 0) {
			weight.Create(2 * blurRadius + 1, 2 * blurRadius + 1);
			Rgba* w0 = weight.RgbaData();
			Rgba* w0End = w0 + weight.StridePixels() * weight.Height();
			Si32 dy = -blurRadius;
			float br2 = float(blurRadius * blurRadius);
			for (; w0 != w0End; w0 += weight.StridePixels(), dy++) {
				Si32 dx = -blurRadius;
				for (Rgba *w = w0, *wEnd = w0 + weight.Width(); w != wEnd; w++, dx++) {
					float dx2 = float(dx*dx) / br2 * 6.0f;
					float dy2 = float(dy*dy) / br2 * 6.0f;
					w->rgba = Ui32(256.0 * exp(-(dx2 + dy2)));
					totalWeight += w->rgba;
				}
			}
		}

		// Accumulate
		for (Si32 x = -expandRadius; x < sprite.Width() + expandRadius; x++) {
			for (Si32 y = -expandRadius; y < sprite.Height() + expandRadius; y++) {
				Ui8 value = 0;
				for (Si32 x1 = std::max(0, x - expandRadius), x2 = std::min(sprite.Width(), x + expandRadius + 1); x1 < x2; x1++) {
					for (Si32 y1 = std::max(0, y - expandRadius), y2 = std::min(sprite.Height(), y + expandRadius + 1); y1 < y2; y1++) {
						Rgba src = *(sprite.RgbaData() + sprite.StridePixels() * y1 + x1);
						value = std::max(value, src.a);
					}
				}

				Rgba* dst0 = shadow.RgbaData() + shadow.StridePixels() * (y + expandRadius) + (x + expandRadius);
				Rgba* dst0End = dst0 + shadow.StridePixels() * (2 * blurRadius + 1);
				Rgba* w0 = weight.RgbaData();
				for (; dst0 != dst0End; dst0 += shadow.StridePixels(), w0 += weight.StridePixels()) {
					for (Rgba *dst = dst0, *dstEnd = dst0 + 2 * blurRadius + 1, *w = w0; dst != dstEnd; dst++, w++) {
						dst->rgba += value * w->rgba;
					}
				}
			}
		}

		// Finalize
		Rgba* dst0 = shadow.RgbaData();
		Rgba* dst0End = dst0 + shadow.StridePixels() * shadow.Height();
		for (; dst0 != dst0End; dst0 += shadow.StridePixels()) {
			for (Rgba *dst = dst0, *dstEnd = dst0 + shadow.Width(); dst != dstEnd; dst++) {
				dst->rgba /= totalWeight;
				dst->rgba <<= 24;
				*dst = RgbaMult(*dst, color.a);
				dst->r = color.r;
				dst->g = color.g;
				dst->b = color.b;
			}
		}

		return shadow;
	}

	void LoadImageFromSpritesheet(Sprite sheet, Si32 width, Si32 height, Si32 posx, Si32 posy, Sprite& sprite)
	{
		sprite.Create(width, height);
		sheet.Draw(0, 0, width, height, width * posx, height * posy, width, height, sprite);
		sprite.SetPivot(Vec2Si32(0, 0));
		sprite.Reference(sheet, width * posx, height * posy, width, height);
		sprite.SetPivot(Vec2Si32(0, 0));
	}


	void LoadMask(Sprite& sprite, const std::string& file_name, Si32 width = 0, Si32 height = 0)
	{
		sprite.Load(file_name);
		size_t left = sprite.Height() * sprite.Width();
		for (Rgba* p = sprite.RgbaData(); left; left--, p++) {
			p->r = 255;
			p->g = 255;
			p->b = 255;
		}
	}

	void LoadImage(Sprite& sprite, const std::string& file_name, Si32 width = 0, Si32 height = 0)
	{
		if (false /* width || height */) {
			Sprite src;
			src.Load(file_name);

			// Try 1
			//sprite.Create(width ? width : src.Width(), height ? height : src.Height());
			//src.Draw(0, 0, 0, std::min(float(sprite.Width()) / src.Width(), float(sprite.Height()) / src.Height()), sprite);

			// Try 2
			//sprite.Create(src.Width(), src.Height());
			//src.Draw(0, 0, sprite.Width(), sprite.Height(), 0, 0, src.Width(), src.Height(), sprite);

			// Try 3
			sprite.SetPivot(Vec2Si32(0, 0));
			sprite.Create(src.Width(), src.Height());
			src.Draw(0, 0, 0, 1.0f, sprite);
			
		}
		else {
			sprite.Load(file_name);
		}
	}

	void InitImage()
	{
		Sprite sheet;
		sheet.Load("data/spritesheet.tga");
		Si32 sw = 128, sh = 256;
		image::g_empty.Load("data/empty.tga");

		image::g_tile[kTlNone] = image::g_empty;
		LoadImageFromSpritesheet(sheet, sw, sh, 0, 0, image::g_tile[kTlBrick]);
		LoadImageFromSpritesheet(sheet, sw, sh, 1, 1, image::g_tile[kTlInactive]);

		image::g_letter[kLtSpace] = image::g_empty;
		LoadImageFromSpritesheet(sheet, sw, sh, 2, 2, image::g_letter[kLtUp]);
		LoadImageFromSpritesheet(sheet, sw, sh, 3, 2, image::g_letter[kLtDown]);
		LoadImageFromSpritesheet(sheet, sw, sh, 1, 2, image::g_letter[kLtRight]);
		LoadImageFromSpritesheet(sheet, sw, sh, 0, 2, image::g_letter[kLtLeft]);
		LoadImageFromSpritesheet(sheet, sw, sh, 0, 3, image::g_letter[kLtInput]);
		LoadImageFromSpritesheet(sheet, sw, sh, 1, 3, image::g_letter[kLtOutput]);
		LoadImageFromSpritesheet(sheet, sw, sh, 0, 6, image::g_letter[kLtDot]);

		image::g_frame.Load("data/letter-frame.tga");
		image::g_tileMask.Load("data/tile-mask.tga");

		LoadImageFromSpritesheet(sheet, sw, sh, 1, 0, image::g_robot);
		LoadImageFromSpritesheet(sheet, sw, sh, 2, 1, image::g_robotShadow);
	
		image::g_button_frame.Load("data/button-frame.tga");
		LoadMask(image::g_button_musicalnote, "data/ui/musical-note.tga");
		LoadMask(image::g_button_nextlevel, "data/ui/up-arrow.tga");
		LoadMask(image::g_button_prevlevel, "data/ui/down-arrow.tga");
		LoadMask(image::g_button_play, "data/ui/play.tga");
		LoadMask(image::g_button_pause, "data/ui/pause.tga");
		LoadMask(image::g_button_rewind, "data/ui/rewind.tga");
		LoadMask(image::g_button_fastforward, "data/ui/fast-forward.tga");
		LoadMask(image::g_button_replay, "data/ui/replay.tga");
		LoadMask(image::g_button_minus, "data/ui/minus.tga");
		LoadMask(image::g_button_plus, "data/ui/plus.tga");
		image::g_button_x1.Load("data/ui/button-x1.tga");
		image::g_button_x2.Load("data/ui/button-x2.tga");
		image::g_button_x4.Load("data/ui/button-x4.tga");
		image::g_button_x8.Load("data/ui/button-x8.tga");

		// Palette
		Sprite palette;
		palette.Load("data/ui/palette.tga");
		Si32 pw = 80, ph = 80;
		LoadImageFromSpritesheet(palette, pw, ph, 0, 0, image::g_button_robot);
		image::g_button_letter[kLtSpace] = image::g_empty;
		LoadImageFromSpritesheet(palette, pw, ph, 1, 0, image::g_button_letter[kLtUp]);
		LoadImageFromSpritesheet(palette, pw, ph, 1, 0, image::g_button_letter[kLtDown]);
		LoadImageFromSpritesheet(palette, pw, ph, 1, 0, image::g_button_letter[kLtRight]);
		LoadImageFromSpritesheet(palette, pw, ph, 1, 0, image::g_button_letter[kLtLeft]);
		LoadImageFromSpritesheet(palette, pw, ph, 2, 0, image::g_button_letter[kLtInput]);
		LoadImageFromSpritesheet(palette, pw, ph, 3, 0, image::g_button_letter[kLtOutput]);
		image::g_button_letter[kLtDot] = image::g_empty;
	}

	void InitMusic()
	{
		music::g_background[0].Load("data/music-observing-the-star.ogg");
	}

	void InitSfx()
	{
		sfx::g_click.Load("data/lokif/click.wav");
		sfx::g_click2.Load("data/lokif/click_2.wav");
		sfx::g_positive.Load("data/lokif/positive.wav");
		sfx::g_negative2.Load("data/lokif/negative_2.wav");
		sfx::g_read.Load("data/circlerun/read.wav");
		sfx::g_write.Load("data/circlerun/write.wav");
	}

	void PreInitData()
	{
		image::g_pilecode.Load("data/bg/pilecode-1440x900.tga");

		CreateBackground(image::g_background[0], Rgba(0xaa, 0xee, 0xff), Rgba(0x77, 0xcc, 0xff));
		CreateBackground(image::g_background[1], Rgba(0xee, 0xff, 0xaa), Rgba(0xcc, 0xff, 0x77));
		CreateBackground(image::g_background[2], Rgba(0xff, 0xee, 0xaa), Rgba(0xff, 0xcc, 0x77));
	}

	void InitData()
	{
		InitImage();
		InitMusic();
		InitSfx();
	}
}
