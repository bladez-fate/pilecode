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

#include "graphics.h"

#include "engine/easy.h"

namespace pilecode {

	template <class FilterFunc>
	void FilterDraw(Sprite sprite, const Si32 to_x_pivot, const Si32 to_y_pivot,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height,
		Sprite to_sprite,
		FilterFunc filter)
	{
		const Si32 from_stride_pixels = sprite.StridePixels();
		const Si32 to_stride_pixels = to_sprite.Width();

		const Si32 to_x = to_x_pivot - sprite.Pivot().x * to_width / from_width;
		const Si32 to_y = to_y_pivot - sprite.Pivot().y * to_height / from_height;

		Rgba *to = to_sprite.RgbaData()
			+ to_y * to_stride_pixels
			+ to_x;
		const Rgba *from = sprite.RgbaData()
			+ from_y * from_stride_pixels
			+ from_x;

		const Si32 to_y_db = (to_y >= 0 ? 0 : -to_y);
		const Si32 to_y_d_max = to_sprite.Height() - to_y;
		const Si32 to_y_de = (to_height < to_y_d_max ? to_height : to_y_d_max);

		const Si32 to_x_db = (to_x >= 0 ? 0 : -to_x);
		const Si32 to_x_d_max = to_sprite.Width() - to_x;
		const Si32 to_x_de = (to_width < to_x_d_max ? to_width : to_x_d_max);

		for (Si32 to_y_disp = to_y_db; to_y_disp < to_y_de; ++to_y_disp) {
			const Si32 from_y_disp = (from_height * to_y_disp) / to_height;

			const Si32 from_x_b = (from_width * to_x_db) / to_width;
			const Si32 from_x_step_16 = 65536 * from_width / to_width;
			Si32 from_x_acc_16 = 0;

			const Rgba *from_line = from + from_y_disp * from_stride_pixels;
			Rgba *to_line = to + to_y_disp * to_stride_pixels;

			for (Si32 to_x_disp = to_x_db; to_x_disp < to_x_de; ++to_x_disp) {
				Rgba *to_rgba = to_line + to_x_disp;
				const Si32 from_x_disp = from_x_b + (from_x_acc_16 / 65536);
				from_x_acc_16 += from_x_step_16;
				const Rgba *from_rgba = from_line + from_x_disp;
				if (from_rgba->a) {
					*to_rgba = filter(from_rgba, to_rgba);
				}
			}
		}
	}

	template <class FilterFunc>
	void FilterFillColor(const Rgba color, const Si32 to_x_pivot, const Si32 to_y_pivot,
		const Si32 to_width, const Si32 to_height, Sprite to_sprite, FilterFunc filter)
	{
		const Si32 to_stride_pixels = to_sprite.Width();

		const Si32 to_x = to_x_pivot;
		const Si32 to_y = to_y_pivot;

		Rgba *to = to_sprite.RgbaData()
			+ to_y * to_stride_pixels
			+ to_x;

		const Si32 to_y_db = (to_y >= 0 ? 0 : -to_y);
		const Si32 to_y_d_max = to_sprite.Height() - to_y;
		const Si32 to_y_de = (to_height < to_y_d_max ? to_height : to_y_d_max);

		const Si32 to_x_db = (to_x >= 0 ? 0 : -to_x);
		const Si32 to_x_d_max = to_sprite.Width() - to_x;
		const Si32 to_x_de = (to_width < to_x_d_max ? to_width : to_x_d_max);

		for (Si32 to_y_disp = to_y_db; to_y_disp < to_y_de; ++to_y_disp) {
			Rgba *to_line = to + to_y_disp * to_stride_pixels;
			for (Si32 to_x_disp = to_x_db; to_x_disp < to_x_de; ++to_x_disp) {
				Rgba *to_rgba = to_line + to_x_disp;
				*to_rgba = filter(&color, to_rgba);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	void DrawSprite(Sprite sprite, const Si32 to_x, const Si32 to_y)
	{
		sprite.Draw(to_x, to_y);
	}

	void DrawSprite(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite)
	{
		sprite.Draw(to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), to_sprite);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	void DrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height, Sprite to_sprite,
		Rgba blend)
	{
		FilterDraw(sprite, to_x, to_y, to_width, to_height,
			from_x, from_y, from_width, from_height,
			to_sprite, [=](const Rgba* fg, const Rgba*) {
			return RgbaSum(
				RgbaMult(*fg, 256 - blend.a),
				RgbaMult(blend, blend.a)
			);
		});
	}

	void DrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend)
	{
		DrawAndBlend(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), ae::GetEngine()->GetBackbuffer(), blend);
	}

	void DrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Rgba blend)
	{
		DrawAndBlend(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), to_sprite, blend);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	void DrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height, Sprite to_sprite,
		Rgba blend1, Rgba blend2)
	{
		FilterDraw(sprite, to_x, to_y, to_width, to_height,
			from_x, from_y, from_width, from_height,
			to_sprite, [=](const Rgba* fg, const Rgba*) {
			Rgba fg2 = RgbaSum(
				RgbaMult(*fg, 256 - blend1.a),
				RgbaMult(blend1, blend1.a)
			);
			return RgbaSum(
				RgbaMult(fg2, 256 - blend2.a),
				RgbaMult(blend2, blend2.a)
			);
		});
	}

	void DrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend1, Rgba blend2)
	{
		DrawAndBlend2(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), ae::GetEngine()->GetBackbuffer(), blend1, blend2);
	}

	void DrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Rgba blend1, Rgba blend2)
	{
		DrawAndBlend2(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), to_sprite, blend1, blend2);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	void DrawWithFixedAlphaBlend(Sprite sprite, const Si32 to_x, const Si32 to_y,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height, Sprite to_sprite,
		Ui8 alpha)
	{
		FilterDraw(sprite, to_x, to_y, to_width, to_height,
			from_x, from_y, from_width, from_height,
			to_sprite, [=](const Rgba* fg, const Rgba* bg) {
			return RgbaSum(
				RgbaMult(*fg, alpha),
				RgbaMult(*bg, 256 - alpha)
			);
		});
	}

	void DrawWithFixedAlphaBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Ui8 alpha)
	{
		DrawWithFixedAlphaBlend(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), ae::GetEngine()->GetBackbuffer(), alpha);
	}

	void DrawWithFixedAlphaBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Ui8 alpha)
	{
		DrawWithFixedAlphaBlend(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), to_sprite, alpha);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	void AlphaDraw(Sprite sprite, const Si32 to_x, const Si32 to_y,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height, Sprite to_sprite, Ui8 opacity)
	{
		if (opacity == 0xff) {
			FilterDraw(sprite, to_x, to_y, to_width, to_height,
				from_x, from_y, from_width, from_height,
				to_sprite, [=](const Rgba* fg, const Rgba* bg) {
				return RgbaSum(
					RgbaMult(*fg, fg->a),
					RgbaMult(*bg, 256 - fg->a)
				);
			});
		}
		else {
			FilterDraw(sprite, to_x, to_y, to_width, to_height,
				from_x, from_y, from_width, from_height,
				to_sprite, [=](const Rgba* fg, const Rgba* bg) {
				Ui8 a = Ui8(Ui32(fg->a) * Ui32(opacity) >> 8);
				return RgbaSum(
					RgbaMult(*fg, a),
					RgbaMult(*bg, 256 - a)
				);
			});
		}
	}

	void AlphaDraw(Sprite sprite, const Si32 to_x, const Si32 to_y, Ui8 opacity)
	{
		AlphaDraw(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), ae::GetEngine()->GetBackbuffer(), opacity);
	}

	void AlphaDraw(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Ui8 opacity)
	{
		AlphaDraw(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), to_sprite, opacity);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	void AlphaDrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height, Sprite to_sprite, Rgba blend, Ui8 opacity)
	{
		if (opacity == 255) {
			FilterDraw(sprite, to_x, to_y, to_width, to_height,
				from_x, from_y, from_width, from_height,
				to_sprite, [=](const Rgba* fg, const Rgba* bg) {
				Rgba fg2 = RgbaSum(
					RgbaMult(*fg, 256 - blend.a),
					RgbaMult(blend, blend.a)
				);
				fg2.a = fg->a;
				return RgbaSum(
					RgbaMult(fg2, fg2.a),
					RgbaMult(*bg, 256 - fg2.a)
				);
			});
		}
		else {
			FilterDraw(sprite, to_x, to_y, to_width, to_height,
				from_x, from_y, from_width, from_height,
				to_sprite, [=](const Rgba* fg, const Rgba* bg) {
				Rgba fg2 = RgbaSum(
					RgbaMult(*fg, 256 - blend.a),
					RgbaMult(blend, blend.a)
				);
				fg2.a = Ui8(Ui32(fg->a) * Ui32(opacity) >> 8);
				return RgbaSum(
					RgbaMult(fg2, fg2.a),
					RgbaMult(*bg, 256 - fg2.a)
				);
			});
		}
	}

	void AlphaDrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend, Ui8 opacity)
	{
		AlphaDrawAndBlend(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), ae::GetEngine()->GetBackbuffer(), blend, opacity);
	}

	void AlphaDrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Rgba blend, Ui8 opacity)
	{
		AlphaDrawAndBlend(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), to_sprite, blend, opacity);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	void AlphaDrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height, Sprite to_sprite, Rgba blend1, Rgba blend2)
	{
		FilterDraw(sprite, to_x, to_y, to_width, to_height,
			from_x, from_y, from_width, from_height,
			ae::GetEngine()->GetBackbuffer(), [=](const Rgba* fg, const Rgba* bg) {
			Rgba fg2 = RgbaSum(
				RgbaMult(*fg, 256 - blend1.a),
				RgbaMult(blend1, blend1.a)
			);
			fg2 = RgbaSum(
				RgbaMult(fg2, 256 - blend2.a),
				RgbaMult(blend2, blend2.a)
			);
			fg2.a = fg->a;
			return RgbaSum(
				RgbaMult(fg2, fg2.a),
				RgbaMult(*bg, 256 - fg2.a)
			);
		});
	}

	void AlphaDrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend1, Rgba blend2)
	{
		AlphaDrawAndBlend2(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), ae::GetEngine()->GetBackbuffer(), blend1, blend2);
	}

	void AlphaDrawAndBlend2(Sprite sprite, const Si32 to_x, const Si32 to_y, Sprite to_sprite, Rgba blend1, Rgba blend2)
	{
		AlphaDrawAndBlend2(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), to_sprite, blend1, blend2);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	void FilterBrightness(Sprite sprite, Ui8 alpha)
	{
		float a = float(alpha) / 255.0f;
		FilterFillColor(Rgba(), 0, 0, sprite.Width(), sprite.Height(),
			sprite, [=](const Rgba*, const Rgba* bg) {
			return RgbaMult(*bg, alpha);
		});
	}

}
