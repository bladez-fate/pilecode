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

	inline Rgba RgbaMult(Rgba c, Ui32 m)
	{
		Ui32 rb = c.rgba & 0x00ff00ff;
		Ui32 rbm = ((rb * m) >> 8) & 0x00ff00ff;
		Ui32 ga = (c.rgba >> 8) & 0x00ff00ff;
		Ui32 gam = ((ga * m)) & 0xff00ff00;
		return Rgba(rbm | gam);
	}

	inline Rgba RgbSum(Rgba x, Rgba y)
	{
		return Rgba(
			Ui16(x.r) + Ui16(y.r) > 255 ? 255 : x.r + y.r,
			Ui16(x.g) + Ui16(y.g) > 255 ? 255 : x.g + y.g,
			Ui16(x.b) + Ui16(y.b) > 255 ? 255 : x.b + y.b,
			0
		);
	}

	inline Rgba RgbaSum(Rgba x, Rgba y)
	{
		return Rgba(
			Ui16(x.r) + Ui16(y.r) > 255 ? 255 : x.r + y.r,
			Ui16(x.g) + Ui16(y.g) > 255 ? 255 : x.g + y.g,
			Ui16(x.b) + Ui16(y.b) > 255 ? 255 : x.b + y.b,
			Ui16(x.a) + Ui16(y.a) > 255 ? 255 : x.a + y.a
		);
	}

	void DrawAndBlend(Sprite sprite, const Si32 to_x_pivot, const Si32 to_y_pivot,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height,
		Sprite to_sprite,
		Rgba blend)
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
					*to_rgba = RgbaSum(
						RgbaMult(*from_rgba, 256 - blend.a),
						RgbaMult(blend, blend.a)
					);
				}
			}
		}
	}

	void DrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height,
		Rgba blend)
	{
		DrawAndBlend(sprite, to_x, to_y, to_width, to_height,
			from_x, from_y, from_width, from_height,
			ae::GetEngine()->GetBackbuffer(), blend);
	}

	void DrawAndBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Rgba blend)
	{
		//if (!sprite_instance_) {
		//	return;
		//}
		DrawAndBlend(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), blend);
	}

	void DrawWithFixedAlphaBlend(Sprite sprite, const Si32 to_x_pivot, const Si32 to_y_pivot,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height,
		Sprite to_sprite,
		Ui8 alpha)
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
					*to_rgba = RgbaSum(
						RgbaMult(*from_rgba, alpha),
						RgbaMult(*to_rgba, 256 - alpha)
					);
				}
			}
		}
	}

	void DrawWithFixedAlphaBlend(Sprite sprite, const Si32 to_x, const Si32 to_y,
		const Si32 to_width, const Si32 to_height,
		const Si32 from_x, const Si32 from_y,
		const Si32 from_width, const Si32 from_height,
		Ui8 alpha)
	{
		DrawWithFixedAlphaBlend(sprite, to_x, to_y, to_width, to_height,
			from_x, from_y, from_width, from_height,
			ae::GetEngine()->GetBackbuffer(), alpha);
	}

	void DrawWithFixedAlphaBlend(Sprite sprite, const Si32 to_x, const Si32 to_y, Ui8 alpha)
	{
		//if (!sprite_instance_) {
		//	return;
		//}
		DrawWithFixedAlphaBlend(sprite, to_x, to_y, sprite.Width(), sprite.Height(),
			0, 0, sprite.Width(), sprite.Height(), alpha);
	}
}
