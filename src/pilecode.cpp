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

#include "engine/easy.h"

namespace pilecode {

	int Pos::dx = 14 * 4;
	int Pos::dy = 7 * 4;
	int Pos::dz = 48 * 4;

	namespace screen {
		int w = 1680;
		int h = 1050;
		int cx = w/2;
		int cy = h/2;
		size_t size = w*h;
	}

	Tile::Tile()
		: type_(kTlNone)
		, letter_(kLtSpace)
	{}

	void Tile::Draw(ViewPort * vp, Pos p)
	{
		if (type_ != kTlNone) {
			image::g_tile[type_].Draw(p.x, p.y);
		}
		if (letter_ != kLtSpace) {
			image::g_letter[letter_].Draw(p.x, p.y);
		}
	}

	Platform::Platform(std::initializer_list<std::initializer_list<int>> data)
		: x_(0), y_(0), z_(0)
	{
		w_ = 0;
		h_ = 0;
		for (auto& xdata : data) {
			if (w_ == 0) {
				for (int x : xdata) {
					w_++;
				}
			}
			h_++;
		}

		tiles_.resize(w_ * h_);

		Tile* tile = &tiles_[0];
		for (auto& xdata : data) {
			Tile* t = tile;
			for (int x : xdata) {
				t->set_type(TileType(x));
				if (t->type() != kTlNone) {
					t->set_letter(Letter(1+rand()%4));
				}
				else {
					t->set_letter(kLtSpace);
				}
				t++;
			}
			tile += w_;
		}
	}

	void Platform::Draw(ViewPort * vp)
	{
		Tile* tile = &tiles_[0];
		Pos p = vp->GetPos(x_, y_);
		for (int iy = 0; iy < h_; iy++, p.Up()) {
			Pos p0 = p;
			for (int ix = 0; ix < w_; ix++, p0.Right()) {
				tile->Draw(vp, p0);
				tile++;
			}
		}
	}

	void World::Draw(ViewPort* vp)
	{
		for (auto& p : platform_) {
			p->Draw(vp);
		}
	}

	void World::AddPlatform(Platform* platform)
	{
		platform_.emplace_back(platform);
	}

	Pos ViewPort::GetPos(int wx, int wy, int wz)
	{
		Pos p(wx, wy, wz);

		p.x += cx_ + x_;
		p.y += cy_ + y_;
		return p;
	}
}