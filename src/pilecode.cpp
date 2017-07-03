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

	void Tile::Draw(ViewPort* vp, int wx, int wy, int wz)
	{
		if (type_ != kTlNone) {
			vp->Draw(&image::g_tile[type_], wx, wy, wz, 0, 0);
		}
		if (letter_ != kLtSpace) {
			vp->Draw(&image::g_letter[letter_], wx, wy, wz, 0, 0);
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
		for (int iy = 0; iy < h_; iy++) {
			for (int ix = 0; ix < w_; ix++) {
				tile->Draw(vp, worldX(ix), worldY(iy), z_);
				tile++;
			}
		}
	}

	Robot::Robot(Platform* platform, int x, int y, Direction dir)
		: platform_(platform)
		, x_(x), y_(y)
		, dir_(dir)
	{}

	void Robot::Draw(ViewPort * vp)
	{
		vp->Draw(&image::g_robot,
			platform_->worldX(x_),
			platform_->worldY(y_),
			platform_->worldZ(0),
			0, 0);
		vp->Draw(&image::g_robotShadow,
			platform_->worldX(x_),
			platform_->worldY(y_),
			platform_->worldZ(0),
			0, 0);
	}

	void World::Draw(ViewPort* vp)
	{
		for (auto& p : platform_) {
			p->Draw(vp);
		}
		for (auto& r : robot_) {
			r->Draw(vp);
		}
	}

	void World::AddPlatform(Platform* platform)
	{
		platform_.emplace_back(platform);
	}

	void World::AddRobot(Robot * robot)
	{
		robot_.emplace_back(robot);
	}

	ViewPort::ViewPort(const WorldParams& wparams)
		: wparams_(wparams)
		, cmnds_(wparams.size())
	{}

	void ViewPort::Draw(ae::Sprite* sprite, int wx, int wy, int wz, int off_x, int off_y)
	{
		size_t index = wparams_.index(wx, wy, wz);
		RenderList& rlist = cmnds_[index];
		rlist.emplace_back(RenderCmnd(sprite, off_x, off_y));
	}

	void ViewPort::BeginRender()
	{
		curFrameTime_ = ae::Time();
		if (lastFrameTime_ == 0.0) {
			lastFrameTime_ = curFrameTime_ - 1.0;
		}
	}

	void ViewPort::EndRender()
	{
		Pos p2 = GetPos(0, 0);
		RenderList* rlist = &cmnds_[0];
		for (int iz = 0; iz < wparams_.zsize(); iz++, p2.Ceil()) {
			Pos p1 = p2;
			for (int iy = 0; iy < wparams_.ysize(); iy++, p1.Up()) {
				Pos p0 = p1;
				for (int ix = 0; ix < wparams_.xsize(); ix++, p0.Right()) {
					for (RenderCmnd& cmnd : *rlist) {
						cmnd.Apply(p0.x, p0.y);
					}
					// note that render lists are cleared after rendering
					rlist->clear();
					rlist++;
				}
			}
		}
		lastFrameTime_ = curFrameTime_;
	}

	Pos ViewPort::GetPos(int wx, int wy, int wz)
	{
		Pos p(wx, wy, wz);

		p.x += cx_ + x_;
		p.y += cy_ + y_;
		return p;
	}

	ViewPort::RenderCmnd::RenderCmnd(ae::Sprite* sprite, int off_x, int off_y)
		: type_(kSpriteRgba)
		, sprite_(sprite)
		, off_x_(off_x)
		, off_y_(off_y)
	{}

	void ViewPort::RenderCmnd::Apply(int x, int y)
	{
		switch (type_) {
		case kSprite:
			sprite_->Draw(x + off_x_, y + off_y_);
			break;
		case kSpriteRgba:
			sprite_->Draw(x + off_x_, y + off_y_);
		}
	}
}