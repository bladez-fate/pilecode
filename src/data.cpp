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

#include <map>

namespace pilecode {

	const Vec2Si32 g_tileCenter(64, 54);

	namespace image {
        // logo
		Sprite g_pilecode;

        // pilecode
		Sprite g_empty;
		Sprite g_tile[kTlMax];
		Sprite g_letter[kLtMax];
		Sprite g_letter_output[kLtMax];
		Sprite g_letter_output_filled[kLtMax];
		Sprite g_frame;
		Sprite g_boldFrame;
		Sprite g_tileMask;
		Sprite g_robot;
		Sprite g_robotShadow;
        Sprite g_layer;

        // ui
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
        Sprite g_button_cancel;
        Sprite g_button_checked;
		Sprite g_button_x1;
		Sprite g_button_x2;
		Sprite g_button_x4;
		Sprite g_button_x8;
		Sprite g_button_robot;
		Sprite g_button_letter[kLtMax];
        Sprite g_button_credits;

        // bg
		Sprite g_background[g_backgroundCount];
		Sprite g_introBackground;
        Sprite g_credits;

        // for MOD_XMAS
        Sprite g_snowflake[g_snowflakeCount];
    }

	namespace music {
        std::vector<Sound> g_background;
	}

	namespace sfx {
		Sound g_click;
		Sound g_click2;
		Sound g_positive;
		Sound g_negative2;
		Sound g_read;
		Sound g_write;
        Sound g_eraseLetter;
	}

	void CreateBackground(Sprite& bgSprite, Rgba c1, Rgba c2)
	{
		bgSprite.Create(screen::w, screen::h);

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

	Sprite GaussianBlurKernel(Si32 xBlurRadius, Si32 yBlurRadius, Ui32& normalizeCoef)
	{
		// Get cached gaussian blur weight map
		static std::map<
			std::pair<Si32, Si32>,
			std::pair<Sprite, Ui32>
		> cache;
		auto& cv = cache[std::make_pair(xBlurRadius, yBlurRadius)];
		Sprite& kernel = cv.first;
		Ui32& totalWeight = cv.second;

		if (totalWeight == 0) {
			// Create weight map
			kernel.Create(2 * xBlurRadius + 1, 2 * yBlurRadius + 1);
			Rgba* w0 = kernel.RgbaData();
			Rgba* w0End = w0 + kernel.StridePixels() * kernel.Height();
			Si32 dy = -yBlurRadius;
			float xbr2 = float(xBlurRadius * xBlurRadius);
			float ybr2 = float(yBlurRadius * yBlurRadius);
			for (; w0 != w0End; w0 += kernel.StridePixels(), dy++) {
				Si32 dx = -xBlurRadius;
				for (Rgba *w = w0, *wEnd = w0 + kernel.Width(); w != wEnd; w++, dx++) {
					float dx2 = float(dx*dx) / xbr2 * 6.0f;
					float dy2 = float(dy*dy) / ybr2 * 6.0f;
					w->rgba = Ui32(256.0 * exp(-(dx2 + dy2)));
					totalWeight += w->rgba;
				}
			}
		}

		normalizeCoef = totalWeight;
		return kernel;
	}

	Sprite GaussianBlurKernel(Si32 blurRadius, Ui32& normalizeCoef)
	{
		return GaussianBlurKernel(blurRadius, blurRadius, normalizeCoef);
	}

	// Creates boundary using series of transformations:
	// - use `sprite' alpha-channel as source (RGB channels are ignored)
	// - expand it by `*ExpandRadius' pixels (using square shape)
	// - then apply gaussian blur with `*BlurRadius' radius in pixels
	// - then transform using double boundary step functions at `lo' and `hi' values of brightness
	// - apply given `color' to result (note that `color.a' is maximum result opacity)
	Sprite CreateBoundary(Sprite sprite,
		Si32 xExpandRadius, Si32 yExpandRadius,
		Si32 xBlurRadius, Si32 yBlurRadius,
		Ui8 lo, Ui8 hi, Si32 loSlope, Si32 hiSlope,
		Rgba color)
	{
		Si32 xTotalRadius = xBlurRadius + xExpandRadius;
		Si32 yTotalRadius = yBlurRadius + yExpandRadius;

		Sprite result;
		result.Create(sprite.Width() + 2 * xTotalRadius, sprite.Height() + 2 * yTotalRadius);
		result.SetPivot(Vec2Si32(xTotalRadius, yTotalRadius));

		Ui32 totalWeight;
		Sprite weight = GaussianBlurKernel(xBlurRadius, yBlurRadius, totalWeight);

		// Accumulate
		for (Si32 x = -xExpandRadius; x < sprite.Width() + xExpandRadius; x++) {
			for (Si32 y = -yExpandRadius; y < sprite.Height() + yExpandRadius; y++) {
				Ui8 value = 0;
				for (Si32 x1 = std::max(0, x - xExpandRadius), x2 = std::min(sprite.Width(), x + xExpandRadius + 1); x1 < x2; x1++) {
					for (Si32 y1 = std::max(0, y - yExpandRadius), y2 = std::min(sprite.Height(), y + yExpandRadius + 1); y1 < y2; y1++) {
						Rgba src = *(sprite.RgbaData() + sprite.StridePixels() * y1 + x1);
						value = std::max(value, src.a);
					}
				}

				Rgba* dst0 = result.RgbaData() + result.StridePixels() * (y + yExpandRadius) + (x + xExpandRadius);
				Rgba* dst0End = dst0 + result.StridePixels() * (2 * yBlurRadius + 1);
				Rgba* w0 = weight.RgbaData();
				for (; dst0 != dst0End; dst0 += result.StridePixels(), w0 += weight.StridePixels()) {
					for (Rgba *dst = dst0, *dstEnd = dst0 + 2 * xBlurRadius + 1, *w = w0; dst != dstEnd; dst++, w++) {
						dst->rgba += value * w->rgba;
					}
				}
			}
		}

		// Finalize
		Rgba* dst0 = result.RgbaData();
		Rgba* dst0End = dst0 + result.StridePixels() * result.Height();
		for (; dst0 != dst0End; dst0 += result.StridePixels()) {
			for (Rgba *dst = dst0, *dstEnd = dst0 + result.Width(); dst != dstEnd; dst++) {
				dst->rgba /= totalWeight; // normalize
				dst->rgba <<= 24; // save value in alpha channel
				
				// apply double boundary step transformation
				if (dst->a == 0x00 || dst->a == 0xff) {
					dst->a = 0x00;
				}
				else {
					dst->a = (Ui8)ae::Clamp(std::min(
						loSlope * (Si32(dst->a) - lo) + 0x80,
						hiSlope * (hi - Si32(dst->a)) + 0x80
					), 0, 0xff);
				}

				// apply color and opacity (color.a)
				*dst = RgbaMult(*dst, color.a);
				dst->r = color.r;
				dst->g = color.g;
				dst->b = color.b;
			}
		}

        result.UpdateOpaqueSpans();
		return result;
	}

	// Creates shadow using series of transformations:
	// - use `sprite' alpha-channel as source (RGB channels are ignored)
	// - expand it by `expandRadius' pixels (using square shape)
	// - then apply gaussian blur with `blurRadius' radius in pixels
	// - apply given `color' to result (note that `color.a' is maximum result opacity)
	Sprite CreateShadow(Sprite sprite, Si32 expandRadius, Si32 blurRadius, Rgba color)
	{
		Si32 totalRadius = blurRadius + expandRadius;

		Sprite result;
		result.Create(sprite.Width() + 2 * totalRadius, sprite.Height() + 2 * totalRadius);
		result.SetPivot(Vec2Si32(totalRadius, totalRadius));

		Ui32 totalWeight;
		Sprite weight = GaussianBlurKernel(blurRadius, totalWeight);

		// Accumulate
		// TODO: get rid of the folowwing copy-pase
		for (Si32 x = -expandRadius; x < sprite.Width() + expandRadius; x++) {
			for (Si32 y = -expandRadius; y < sprite.Height() + expandRadius; y++) {
				Ui8 value = 0;
				for (Si32 x1 = std::max(0, x - expandRadius), x2 = std::min(sprite.Width(), x + expandRadius + 1); x1 < x2; x1++) {
					for (Si32 y1 = std::max(0, y - expandRadius), y2 = std::min(sprite.Height(), y + expandRadius + 1); y1 < y2; y1++) {
						Rgba src = *(sprite.RgbaData() + sprite.StridePixels() * y1 + x1);
						value = std::max(value, src.a);
					}
				}

				Rgba* dst0 = result.RgbaData() + result.StridePixels() * (y + expandRadius) + (x + expandRadius);
				Rgba* dst0End = dst0 + result.StridePixels() * (2 * blurRadius + 1);
				Rgba* w0 = weight.RgbaData();
				for (; dst0 != dst0End; dst0 += result.StridePixels(), w0 += weight.StridePixels()) {
					for (Rgba *dst = dst0, *dstEnd = dst0 + 2 * blurRadius + 1, *w = w0; dst != dstEnd; dst++, w++) {
						dst->rgba += value * w->rgba;
					}
				}
			}
		}

		// Finalize
		Rgba* dst0 = result.RgbaData();
		Rgba* dst0End = dst0 + result.StridePixels() * result.Height();
		for (; dst0 != dst0End; dst0 += result.StridePixels()) {
			for (Rgba *dst = dst0, *dstEnd = dst0 + result.Width(); dst != dstEnd; dst++) {
				dst->rgba /= totalWeight;
				dst->rgba <<= 24;
				*dst = RgbaMult(*dst, color.a);
				dst->r = color.r;
				dst->g = color.g;
				dst->b = color.b;
			}
		}

        result.UpdateOpaqueSpans();
		return result;
	}

	void LoadImageFromSpritesheet(Sprite sheet, Si32 width, Si32 height, Si32 posx, Si32 posy, Sprite& sprite)
	{
        Sprite sprite0;
		sprite0.Reference(sheet, width * posx, height * posy, width, height);
        sprite.Clone(sprite0);
        sprite.UpdateOpaqueSpans();
    }

	void LoadLetter(Sprite sheet, Si32 posx, Si32 posy, Letter letter)
	{
		LoadImageFromSpritesheet(sheet, 128, 256, posx, posy, image::g_letter[letter]);
		image::g_letter_output[letter] = CreateBoundary(
			image::g_letter[letter], 0, 0, 8, 4, 0x40, 0xc0, 2, 5, Rgba(0xff, 0xff, 0xff, 0xff));
		image::g_letter_output_filled[letter] = CreateBoundary(
			image::g_letter[letter], 0, 0, 14, 7, 0x30, 0xc0, 2, 5, Rgba(0x66, 0xff, 0x66, 0xff));
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
        sprite.UpdateOpaqueSpans();
	}

    void InitSnowflakes()
    {
        Sprite sheet;
        sheet.Load("data/ui/snowflakes.tga");
        Si32 sw = 70, sh = 80;
        Si32 count = 0;
        for (Si32 iy = 0; iy < image::g_snowflakeH; iy++) {
            for (Si32 ix = 0; ix < image::g_snowflakeW; ix++, count++) {
                LoadImageFromSpritesheet(sheet, sw, sh, ix, iy, image::g_snowflake[count]);
            }
        }
    }
    
	void InitImage()
	{
        image::g_pilecode.Load("data/bg/pilecode-1440x900.tga");

		//CreateBackground(image::g_introBackground, Rgba(0xff, 0xee, 0xaa), Rgba(0xff, 0xcc, 0x77));
		CreateBackground(image::g_introBackground, Rgba(0xaa, 0xee, 0xff), Rgba(0x77, 0xcc, 0xff));
        image::g_credits.Load("data/ui/credits.tga");
        image::g_button_credits.Load("data/ui/btn-credits.tga");

#ifndef MOD_XMAS
        CreateBackground(image::g_background[0], Rgba(0xaa, 0xee, 0xff), Rgba(0x77, 0xcc, 0xff));
        CreateBackground(image::g_background[1], Rgba(0xee, 0xff, 0xaa), Rgba(0xcc, 0xff, 0x77));
        CreateBackground(image::g_background[2], Rgba(0xff, 0xee, 0xaa), Rgba(0xff, 0xcc, 0x77));
#else
		CreateBackground(image::g_background[0], Rgba(0xaa, 0xee, 0xff), Rgba(0x77, 0xcc, 0xff));
		CreateBackground(image::g_background[1], Rgba(0xee, 0xcc, 0xff), Rgba(0xcc, 0x99, 0xff));
		CreateBackground(image::g_background[2], Rgba(0xff, 0xcc, 0xee), Rgba(0xff, 0x99, 0xcc));
#endif
		Sprite sheet;
		sheet.Load("data/game/spritesheet.tga");
		Si32 sw = 128, sh = 256;
		image::g_empty.Load("data/game/empty.tga");
        
		image::g_tile[kTlNone] = image::g_empty;
		LoadImageFromSpritesheet(sheet, sw, sh, 0, 0, image::g_tile[kTlBrick]);
		LoadImageFromSpritesheet(sheet, sw, sh, 1, 1, image::g_tile[kTlInactive]);

		image::g_letter[kLtSpace] = image::g_empty;
		image::g_letter_output[kLtSpace] = image::g_empty;
		image::g_letter_output_filled[kLtSpace] = image::g_empty;
        LoadLetter(sheet, 0, 2, kLtLeft);
        LoadLetter(sheet, 1, 2, kLtRight);
		LoadLetter(sheet, 2, 2, kLtUp);
		LoadLetter(sheet, 3, 2, kLtDown);
		LoadLetter(sheet, 0, 3, kLtInput);
		LoadLetter(sheet, 1, 3, kLtOutput);
        LoadLetter(sheet, 2, 3, kLtCounterClockwise);
        LoadLetter(sheet, 3, 3, kLtClockwise);
        LoadLetter(sheet, 0, 4, kLtEq);
        LoadLetter(sheet, 1, 4, kLtNe);
        LoadLetter(sheet, 2, 4, kLtLt);
        LoadLetter(sheet, 3, 4, kLtGt);
        LoadLetter(sheet, 0, 5, kLtCircles);
        LoadLetter(sheet, 1, 5, kLtContrast);
        LoadLetter(sheet, 2, 5, kLtBrightness);
        LoadLetter(sheet, 3, 5, kLtEmit);

		LoadLetter(sheet, 0, 6, kLtDot);

		image::g_frame.Load("data/game/letter-frame.tga");
		image::g_boldFrame = CreateShadow(image::g_frame, 1, 2, Rgba(0xff, 0xff, 0xff, 0xff));
		image::g_tileMask.Load("data/game/tile-mask.tga");

		LoadImageFromSpritesheet(sheet, sw, sh, 1, 0, image::g_robot);
		LoadImageFromSpritesheet(sheet, sw, sh, 2, 1, image::g_robotShadow);
        LoadImageFromSpritesheet(sheet, sw, sh, 3, 1, image::g_layer);

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
        LoadMask(image::g_button_cancel, "data/ui/cancel.tga");
        LoadMask(image::g_button_checked, "data/ui/checked.tga");
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

#ifdef MOD_XMAS
        InitSnowflakes();
#endif
}

    void AddMusic(const std::string& filename)
    {
        music::g_background.push_back(Sound());
        music::g_background.back().Load(filename);
    }
    
	void InitMusic()
	{
        static bool done = false;
        if (done) {
            return;
        }
        
#ifndef MOD_XMAS
        AddMusic("data/music/observing-the-star.ogg");
#else
        AddMusic("data/music/dance-of-the-sugar-plum-fairy.ogg");
        AddMusic("data/music/deck-the-hall-a.ogg");
        AddMusic("data/music/jingle-bells-calm.ogg");
#endif
        
        done = true;
	}

	void InitSfx()
	{
        static bool done = false;
        if (done) {
            return;
        }
		
        sfx::g_click.Load("data/sfx/click.wav");
		sfx::g_click2.Load("data/sfx/click_2.wav");
		sfx::g_positive.Load("data/sfx/positive.wav");
		sfx::g_negative2.Load("data/sfx/negative_2.wav");
		sfx::g_read.Load("data/sfx/read.wav");
		sfx::g_write.Load("data/sfx/write.wav");
        sfx::g_eraseLetter.Load("data/sfx/erase-letter.ogg");

        done = true;
	}

	void InitData()
	{
		InitImage();
		InitMusic();
		InitSfx();
	}
}
