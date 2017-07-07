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
	int Pos::dz = 25 * 4;

	namespace screen {
		int w = 1680;
		int h = 1050;
		int cx = w/2;
		int cy = h/2;
		size_t size = w*h;
	}

	void Tile::Draw(ViewPort* vp, int wx, int wy, int wz)
	{
		if (type_ != kTlNone) {
			ae::Sprite* sprite = vp->world()->params().data().TileSprite(wz, type_);
			vp->Draw(sprite, wx, wy, wz, 1, ar::Vec2Si32(0, 0));
		}
		if (letter_ != kLtSpace) {
			vp->Draw(&image::g_letter[letter_], wx, wy, wz, 1, ar::Vec2Si32(0, 0));
		}
	}

	Letter Tile::ReadLetter()
	{
		touched_ = true;
		return letter();
	}

	bool Tile::IsMovable() const
	{
		return type_ != kTlNone;
	}

	WorldData::WorldData(size_t zsize)
	{
		tileSprite_.resize(zsize);
		for (int wz = 0; wz < zsize; wz++) {
			auto& ts = tileSprite_[wz];
			ts.resize(kTlMax);
			float alpha = float(wz) / (zsize - 1);
			for (int i = 0; i < kTlMax; i++) {
				TileType t = TileType(i);
				ae::Sprite& src = image::g_tile[t];
				ae::Sprite& dst = ts[i];
				dst.Create(src.Width(), src.Height());
				ar::Rgba* srcIt = src.RgbaData();
				ar::Rgba* dstIt = dst.RgbaData();
				size_t left = src.Width() * src.Height();
				for (; left > 0; srcIt++, dstIt++, left--) {
					*dstIt = *srcIt;
					ar::Si16 gdelta = ar::Si16(dstIt->g * alpha);
					ar::Si16 bdelta = ar::Si16(dstIt->b * alpha);
					dstIt->g += bdelta - gdelta;
					dstIt->b += gdelta - bdelta;
				}
			}
		}
	}

	ae::Sprite* WorldData::TileSprite(int wz, TileType type)
	{
		return &tileSprite_[wz][type];
	}

	WorldParams::WorldParams(size_t xsize, size_t ysize, size_t zsize)
		: xsize_(xsize), ysize_(ysize), zsize_(zsize)
		, xysize_(xsize * ysize)
		, xyzsize_(xsize * ysize * zsize)
		, data_(new WorldData(zsize))
	{}

	Platform::Platform(int x, int y, int z,
		std::initializer_list<std::initializer_list<int>> data)
		: x_(x), y_(y), z_(z)
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
				if (t->type() != kTlNone && rand()%4 == 0) {
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

	Platform* Platform::Clone()
	{
		return new Platform(*this);
	}

	void Platform::ChangeLetter(int rx, int ry)
	{
		if (Tile* tile = changable_tile(rx, ry)) {
			if (tile->IsMovable()) {
				tile->set_letter(Letter((tile->letter() + 1) % kLtMax));
			}
		}
	}

	Tile* Platform::changable_tile(int rx, int ry)
	{
		if (rx >= 0 && rx < w_ && ry >= 0 && ry < h_) {
			return &tiles_[ry * w_ + rx];
		} else {
			return nullptr;
		}
	}

	const Tile* Platform::get_tile(int rx, int ry) const
	{
		if (rx >= 0 && rx < w_ && ry >= 0 && ry < h_) {
			return &tiles_[ry * w_ + rx];
		}
		else {
			return Tile::none();
		}
	}

	Robot::Robot(int platform, int x, int y, Direction dir)
		: seed_(rand())
		, platform_(platform)
		, x_(x), y_(y)
		, px_(x), py_(y)
		, dir_(dir)
	{}

	void Robot::Draw(ViewPort * vp)
	{
		Platform* p = vp->world()->platform(platform_);

		ar::Vec2Si32 off = Pos::ToScreen(d_pos());
		off.x = ar::Si32(off.x * vp->progress());
		off.y = ar::Si32(off.y * vp->progress());

		int body_off_y = (int)round(4.0 * sin((vp->progress() + (seed_ % 1000) / 1000.0) * 2.0 * M_PI));

		vp->Draw(&image::g_robotShadow,
			p->worldX(px_),
			p->worldY(py_),
			p->worldZ(0),
			2,
			off);
		vp->Draw(&image::g_robot,
			p->worldX(px_),
			p->worldY(py_),
			p->worldZ(0),
			2,
			ar::Vec2Si32(off.x, off.y + body_off_y));
	}

	void Robot::Simulate(World* world)
	{
		Platform* p = world->platform(platform_);
		if (Tile* cur_tile = p->changable_tile(x_, y_)) {
			if (cur_tile->IsMovable()) {
				switch (cur_tile->ReadLetter()) {
				case kLtSpace:
					break; // just keep moving
				case kLtUp:
					dir_ = kDirUp;
					break;
				case kLtDown:
					dir_ = kDirDown;
					break;
				case kLtRight:
					dir_ = kDirRight;
					break;
				case kLtLeft:
					dir_ = kDirLeft;
					break;
				}
				int dx;
				int dy;
				switch (dir_) {
				case kDirRight:
					dx = 1;
					dy = 0;
					break;
				case kDirUp:
					dx = 0;
					dy = 1;
					break;
				case kDirLeft:
					dx = -1;
					dy = 0;
					break;
				case kDirDown:
					dx = 0;
					dy = -1;
					break;
				}
				px_ = x_;
				py_ = y_;
				int newx = x_ + dx;
				int newy = y_ + dy;
				if (p->get_tile(newx, newy)->IsMovable()) {
					x_ = newx;
					y_ = newy;
				}
			}
		}
	}

	Robot* Robot::Clone()
	{
		return new Robot(*this);	
	}

	ar::Vec2Si32 Robot::d_pos()
	{
		return ar::Vec2Si32(x_ - px_, y_ - py_);
	}

	World::World(const WorldParams & wparams)
		: wparams_(wparams)
	{}

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

	void World::ChangeLetter(ar::Vec3Si32 w)
	{
		for (const auto& p : platform_) {
			if (p->worldZ(0) == w.z) {
				p->ChangeLetter(p->PlatformX(w.x), p->PlatformY(w.y));
			}
		}
	}

	bool World::IsTouched(ar::Vec3Si32 w)
	{
		for (const auto& p : platform_) {
			if (p->worldZ(0) == w.z) {
				if (p->get_tile(p->PlatformX(w.x), p->PlatformY(w.y))->touched()) {
					return true;
				}
			}
		}
		return false;
	}

	void World::Simulate()
	{
		for (const auto& robot : robot_) {
			robot->Simulate(this);
		}
	}

	World* World::Clone()
	{
		World* clone = new World(wparams_);
		for (const auto& p : platform_) {
			clone->AddPlatform(p->Clone());
		}
		for (const auto& r : robot_) {
			clone->AddRobot(r->Clone());
		}
		return clone;
	}

	ViewPort::ViewPort(const WorldParams& wparams)
		: wparams_(wparams)
		, cmnds_(wparams.size() * zlSize)
		, visible_z_(wparams.zsize())
	{}

	void ViewPort::Draw(ae::Sprite* sprite, int wx, int wy, int wz, int zl, ar::Vec2Si32 off)
	{
		if (!(zl >= 0 && zl < zlSize)) {
			abort();
		}
		size_t index = wparams_.index(wx, wy, (wz << zlBits) + zl);
		RenderList& rlist = cmnds_[index];
		rlist.emplace_back(RenderCmnd(sprite, off));
	}

	void ViewPort::Draw(ae::Sprite* sprite, int wx, int wy, int wz, int zl)
	{
		Draw(sprite, wx, wy, wz, zl, ar::Vec2Si32(0, 0));
	}

	void ViewPort::Draw(ae::Sprite* sprite, ar::Vec3Si32 w, int zl, ar::Vec2Si32 off)
	{
		Draw(sprite, w.x, w.y, w.z, zl, off);
	}

	void ViewPort::Draw(ae::Sprite* sprite, ar::Vec3Si32 w, int zl)
	{
		Draw(sprite, w, zl, ar::Vec2Si32(0, 0));
	}

	void ViewPort::BeginRender(double time)
	{
		curFrameTime_ = time;
		if (lastFrameTime_ == 0.0) {
			lastFrameTime_ = curFrameTime_ - 1.0;
		}
	}

	void ViewPort::EndRender()
	{
		Pos p2 = GetPos(0, 0);
		RenderList* rlist = &cmnds_[0];
		for (int iz = 0; iz < visible_z_; iz++, p2.Ceil()) {
			for (int zl = 0; zl < zlSize; zl++) {
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
		}
		lastFrameTime_ = curFrameTime_;
	}

	void ViewPort::Move(ar::Vec2F delta)
	{
		x_ += delta.x;
		y_ += delta.y;
	}

	void ViewPort::IncVisibleZ()
	{
		if (visible_z_ < wparams_.zsize()) {
			visible_z_++;
		}
	}

	void ViewPort::DecVisibleZ()
	{
		if (visible_z_ > 1) {
			visible_z_--;
		}
	}

	ar::Vec3Si32 ViewPort::ToWorld(ar::Vec2Si32 p) const
	{
		p.x -= cx_ + int(x_ + 0.5f);
		p.y -= cy_ + int(y_ + 0.5f);
		p.x -= 64;
		p.y -= 256 - 174;
		return Pos::ToWorld(p, int(visible_z_) - 1);
	}

	Pos ViewPort::GetPos(int wx, int wy, int wz)
	{
		Pos p(wx, wy, wz);
		p.x += cx_ + int(x_ + 0.5f);
		p.y += cy_ + int(y_ + 0.5f);
		return p;
	}

	ViewPort::RenderCmnd::RenderCmnd(ae::Sprite* sprite, ar::Vec2Si32 off)
		: type_(kSpriteRgba)
		, sprite_(sprite)
		, off_(off)
	{}

	void ViewPort::RenderCmnd::Apply(int x, int y)
	{
		switch (type_) {
		case kSprite:
			sprite_->Draw(x + off_.x, y + off_.y);
			break;
		case kSpriteRgba:
			sprite_->Draw(x + off_.x, y + off_.y);
		}
	}
}