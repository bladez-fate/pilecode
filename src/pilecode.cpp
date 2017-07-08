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

	void Tile::Draw(ViewPort* vp, int wx, int wy, int wz, int color)
	{
		if (type_ != kTlNone) {
			Sprite* sprite = vp->world()->params().data().TileSprite(color, type_);
			vp->Draw(sprite, wx, wy, wz, 1, Vec2Si32(0, 0));
		}

		if (output_ != kLtSpace) {
			vp->Draw(&image::g_letter[output_], wx, wy, wz, 1, Vec2Si32(0, -3))
				.Blend(Rgba(0, 0, 0, 255));
		}

		if (letter_ != kLtSpace) {
			vp->Draw(&image::g_letter[letter_], wx, wy, wz, 1, Vec2Si32(0, 0));
		}
	}

	Letter Tile::ReadLetter()
	{
		touched_ = true;
		return letter();
	}

	void Tile::WriteLetter(Letter letter)
	{
		touched_ = true;
		set_letter(letter);
	}

	bool Tile::IsMovable() const
	{
		return type_ != kTlNone;
	}

	WorldData::WorldData(size_t colors)
	{
		tileSprite_.resize(colors);
		for (int wz = 0; wz < colors; wz++) {
			auto& ts = tileSprite_[wz];
			ts.resize(kTlMax);
			float alpha = float(wz) / (colors - 1);
			for (int i = 0; i < kTlMax; i++) {
				TileType t = TileType(i);
				Sprite& src = image::g_tile[t];
				Sprite& dst = ts[i];
				dst.Create(src.Width(), src.Height());
				Rgba* srcIt = src.RgbaData();
				Rgba* dstIt = dst.RgbaData();
				size_t left = src.Width() * src.Height();
				for (; left > 0; srcIt++, dstIt++, left--) {
					*dstIt = *srcIt;
					Si16 gdelta = Si16(dstIt->g * alpha);
					Si16 bdelta = Si16(dstIt->b * alpha);
					dstIt->g += bdelta - gdelta;
					dstIt->b += gdelta - bdelta;
				}
			}
		}
	}

	Sprite* WorldData::TileSprite(int color, TileType type)
	{
		color = color % tileSprite_.size();
		return &tileSprite_[color][type];
	}

	WorldParams::WorldParams(size_t xsize, size_t ysize, size_t zsize, size_t colors)
		: xsize_(xsize), ysize_(ysize), zsize_(zsize)
		, xysize_(xsize * ysize)
		, xyzsize_(xsize * ysize * zsize)
		, data_(new WorldData(colors))
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
				//if (t->type() != kTlNone && rand()%4 == 0) {
				//	t->set_letter(Letter(1+rand()%4));
				//}
				//else {
				//	t->set_letter(kLtSpace);
				//}
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
				tile->Draw(vp, WorldX(ix), WorldY(iy), z_, index());
				tile++;
			}
		}
	}

	Platform* Platform::Clone() const
	{
		return new Platform(*this);
	}

	void Platform::SwitchLetter(int rx, int ry)
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

	bool Platform::ReadLetter(int rx, int ry, Letter& letter)
	{
		if (Tile* tile = changable_tile(rx, ry)) {
			letter = tile->ReadLetter();
			return true;
		}
		else {
			return false;
		}
	}

	bool Platform::WriteLetter(int rx, int ry, Letter letter)
	{
		if (Tile* tile = changable_tile(rx, ry)) {
			if (tile->IsMovable()) {
				tile->WriteLetter(letter);
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}

	Robot::Robot()
		: seed_(rand())
	{
		// Note that Place() is required 
	}

	Robot::Robot(World* world, Vec3Si32 w)
		: Robot()
	{
		Place(world, w);
	}

	void Robot::Place(World* world, Vec3Si32 w)
	{
		Platform* p = world->FindPlatform(w);
		platform_ = p->index();
		if (!p) {
			abort();
		}
		px_ = x_ = p->PlatformX(w.x);
		py_ = y_ = p->PlatformY(w.y);
	}

	void Robot::Draw(ViewPort * vp)
	{
		Platform* p = vp->world()->platform(platform_);

		Vec2Si32 off = Pos::ToScreen(d_pos());
		off.x = Si32(off.x * vp->progress());
		off.y = Si32(off.y * vp->progress());

		int body_off_y = (int)round(4.0 * sin((vp->progress() + (seed_ % 1000) / 1000.0) * 2.0 * M_PI));

		vp->Draw(&image::g_robotShadow,
			p->WorldX(px_),
			p->WorldY(py_),
			p->WorldZ(0),
			2,
			off).Alpha();
		vp->Draw(&image::g_robot,
			p->WorldX(px_),
			p->WorldY(py_),
			p->WorldZ(0),
			2,
			Vec2Si32(off.x, off.y + body_off_y));
	}

	void Robot::SimulateExec(World* world)
	{
		Platform* p = world->platform(platform_);
		if (Tile* cur_tile = p->changable_tile(x_, y_)) {
			if (cur_tile->IsMovable()) {
				Vec3Si32 w = p->ToWorld(x_, y_, 0);
				Vec3Si32 wu = w;
				wu.z++;

				switch (cur_tile->ReadLetter()) {
				case kLtSpace:
					blocked_ = false;
					break; // just keep moving
				case kLtUp:
					dir_ = kDirUp;
					blocked_ = false;
					break;
				case kLtDown:
					dir_ = kDirDown;
					blocked_ = false;
					break;
				case kLtRight:
					dir_ = kDirRight;
					blocked_ = false;
					break;
				case kLtLeft:
					dir_ = kDirLeft;
					blocked_ = false;
					break;
				case kLtRead:
					blocked_ = !world->ReadLetter(wu, reg_);
					break;
				case kLtWrite:
					blocked_ = !world->WriteLetter(wu, reg_);
					break;
				}
			}
			else {
				dir_ = kDirHalt;
				blocked_ = true;
			}
		}
		else {
			dir_ = kDirHalt;
			blocked_ = true;
		}
	}

	void Robot::PrepareMove(World* world)
	{
		Platform* p = world->platform(platform_);
		curr_ = p->ToWorld(x_, y_, 0);
		if (!blocked_ && dir_ != kDirHalt) {
			Vec2Si32 delta = dir_delta();
			Vec3Si32 next = p->ToWorld(x_ + delta.x, y_ + delta.y, 0);
			next_ = world->IsMovable(next) ? next : curr_;
		}
		else {
			next_ = curr_;
		}
	}
	
	size_t Robot::ResolveMove(Robot* r1, Robot* r2)
	{
		// front-to-front deadlock -- block both
		if (r1->next_ == r2->curr_ && r1->curr_ == r2->next_) {
			r1->StopMove();
			r2->StopMove();
			return 2;
		}

		if (r1->next_ == r2->next_) {
			// movement into stopped robot
			Vec3Si32 d1 = r1->next_ - r1->curr_;
			if (d1.x == 0 && d1.y == 0) {
				r2->StopMove();
				return 1;
			}

			// movement into stopped robot
			Vec3Si32 d2 = r2->next_ - r2->curr_;
			if (d2.x == 0 && d2.y == 0) {
				r1->StopMove();
				return 1;
			}

			// front-to-front -- random wins
			Vec3Si32 dsum = d1 + d2;
			if (dsum.x == 0 && dsum.y == 0) {
				(rand() % 2 ? r1 : r2)->StopMove();
				return 1;
			}

			// crossroad sync -- hindrance to the right
			Si32 dprod = d1.x * d2.y - d1.y * d2.x;
			(dprod > 0 ? r1 : r2)->StopMove();
			return 1;
		}

		return 0;
	}

	// should be called after PrepareMove() and before SimulateMove()
	void Robot::StopMove()
	{
		next_ = curr_;
	}

	void Robot::SimulateMove(World* world)
	{
		// choose platform robot is on
		Platform* p = world->FindPlatform(next_);
		platform_ = p->index();

		// propagate robot
		px_ = x_;
		py_ = y_;
		x_ = p->PlatformX(next_.x);
		y_ = p->PlatformY(next_.y);
	}

	Robot* Robot::Clone() const
	{
		return new Robot(*this);	
	}

	Vec2Si32 Robot::d_pos()
	{
		return Vec2Si32(x_ - px_, y_ - py_);
	}

	Vec2Si32 Robot::dir_delta()
	{
		switch (dir_) {
		case kDirRight:
			return Vec2Si32(1, 0);
		case kDirUp:
			return Vec2Si32(0, 1);
		case kDirLeft:
			return Vec2Si32(-1, 0);
		case kDirDown:
			return Vec2Si32(0, -1);
		}
		return Vec2Si32(0, 0);
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
		platform->set_index((int)platform_.size());
		platform_.emplace_back(platform);
	}

	void World::AddRobot(Robot * robot)
	{
		robot_.emplace_back(robot);
	}

	void World::SwitchLetter(Vec3Si32 w)
	{
		for (const auto& p : platform_) {
			if (p->WorldZ(0) == w.z) {
				p->SwitchLetter(p->PlatformX(w.x), p->PlatformY(w.y));
			}
		}
	}

	void World::SwitchRobot(Vec3Si32 w, const Robot& original)
	{
		Platform* p = FindPlatform(w);
		int rx = p->PlatformX(w.x);
		int ry = p->PlatformY(w.y);

		// try find robot to remove
		for (auto i = robot_.begin(), e = robot_.end(); i != e; ++i) {
			Robot* r = i->get();
			if (r->platform() == p->index() && rx == r->x() && ry == r->y()) {
				robot_.erase(i);
				return;
			}
		}

		// if no robot is under cursor - create one
		Robot* clone = original.Clone();
		clone->Place(this, w);
		AddRobot(clone);
	}

	bool World::IsTouched(Vec3Si32 w)
	{
		for (const auto& p : platform_) {
			if (p->WorldZ(0) == w.z) {
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
			robot->SimulateExec(this);
		}
		for (const auto& robot : robot_) {
			robot->PrepareMove(this);
		}

		// Resolve movements until there are conflicts
		size_t resolved;
		do {
			resolved = 0;
			for (const auto& r1 : robot_) {
				for (const auto& r2 : robot_) {
					if (r1 != r2) {
						resolved += Robot::ResolveMove(r1.get(), r2.get());
					}
				}
			}
		} while (resolved);

		for (const auto& robot : robot_) {
			robot->SimulateMove(this);
		}
		steps_++;
	}

	bool World::ReadLetter(Vec3Si32 w, Letter& letter)
	{
		for (const auto& p : platform_) {
			if (p->WorldZ(0) == w.z) {
				if (p->ReadLetter(p->PlatformX(w.x), p->PlatformY(w.y), letter)) {
					return true;
				}
			}
		}
		return false;
	}

	bool World::WriteLetter(Vec3Si32 w, Letter letter)
	{
		for (const auto& p : platform_) {
			if (p->WorldZ(0) == w.z) {
				if (p->WriteLetter(p->PlatformX(w.x), p->PlatformY(w.y), letter)) {
					return true;
				}
			}
		}
		return false;
	}

	bool World::IsMovable(Vec3Si32 w)
	{
		for (const auto& p : platform_) {
			if (p->WorldZ(0) == w.z) {
				if (Tile* tile = p->changable_tile(p->PlatformX(w.x), p->PlatformY(w.y))) {
					return tile->IsMovable();
				}
			}
		}
		return false;
	}

	World* World::Clone() const
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

	Platform* World::FindPlatform(Vec3Si32 w)
	{
		for (const auto& p : platform_) {
			if (p->WorldZ(0) == w.z) {
				if (Tile* tile = p->changable_tile(p->PlatformX(w.x), p->PlatformY(w.y))) {
					if (tile->type() != kTlNone) {
						return p.get();
					}
				}
			}
		}
		return nullptr;
	}

	ViewPort::ViewPort(const WorldParams& wparams)
		: wparams_(wparams)
		, cmnds_(wparams.size() * zlSize)
		, visible_z_(wparams.zsize())
	{}

	ViewPort::RenderCmnd& ViewPort::Draw(Sprite* sprite, int wx, int wy, int wz, int zl, Vec2Si32 off)
	{
		if (!(zl >= 0 && zl < zlSize)) {
			abort();
		}
		size_t index = wparams_.index(wx, wy, (wz << zlBits) + zl);
		RenderList& rlist = cmnds_[index];
		rlist.emplace_back(RenderCmnd(RenderCmnd::kSprite, sprite, off));
		return rlist.back();
	}

	ViewPort::RenderCmnd& ViewPort::Draw(Sprite* sprite, int wx, int wy, int wz, int zl)
	{
		return Draw(sprite, wx, wy, wz, zl, Vec2Si32(0, 0));
	}

	ViewPort::RenderCmnd& ViewPort::Draw(Sprite* sprite, Vec3Si32 w, int zl, Vec2Si32 off)
	{
		return Draw(sprite, w.x, w.y, w.z, zl, off);
	}

	ViewPort::RenderCmnd& ViewPort::Draw(Sprite* sprite, Vec3Si32 w, int zl)
	{
		return Draw(sprite, w, zl, Vec2Si32(0, 0));
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
		for (int iz = 0, ez = (int)std::min(visible_z_ + 1, wparams_.zsize()); iz < ez; iz++, p2.Ceil()) {
			for (int zl = 0; zl < zlSize; zl++) {
				RenderCmnd::Filter filter = (
					iz == visible_z_ - 1 ?
					RenderCmnd::kFilterNone :
					(iz < visible_z_ ? RenderCmnd::kFilterFog : RenderCmnd::kFilterTransparent)
				);
				Pos p1 = p2;
				for (int iy = 0; iy < wparams_.ysize(); iy++, p1.Up()) {
					Pos p0 = p1;
					for (int ix = 0; ix < wparams_.xsize(); ix++, p0.Right()) {
						for (RenderCmnd& cmnd : *rlist) {
							cmnd.Apply(p0.x, p0.y, filter);
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

	void ViewPort::Move(Vec2F delta)
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

	Vec3Si32 ViewPort::ToWorld(Vec2Si32 p) const
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

	ViewPort::RenderCmnd::RenderCmnd(ViewPort::RenderCmnd::Type type,
		Sprite* sprite, Vec2Si32 off)
		: type_(type)
		, sprite_(sprite)
		, off_(off)
		, blend_(Ui32(0))
	{}

	ViewPort::RenderCmnd& ViewPort::RenderCmnd::Blend(Rgba rgba)
	{
		blend_ = rgba;
		return *this;
	}

	ViewPort::RenderCmnd& ViewPort::RenderCmnd::Alpha()
	{
		type_ = kSpriteRgba;
		return *this;
	}

	void ViewPort::RenderCmnd::Apply(int x, int y, ViewPort::RenderCmnd::Filter filter)
	{
		switch (type_) {
		case kSpriteRgba:
			switch (filter) {
			case kFilterNone:
				AlphaDraw(*sprite_, x + off_.x, y + off_.y);
				break;
			case kFilterFog:
				AlphaDrawAndBlend(*sprite_, x + off_.x, y + off_.y, Rgba(0, 0, 0, 0x60));
				break;
			case kFilterTransparent:
				// TODO: draw this on another sprite with regular draw and then blend
				// into main backbuffer, otherwise all platform "internals" became visible
				//DrawWithFixedAlphaBlend(*sprite_, x + off_.x, y + off_.y, 0x60);
				break;
			}
			break;
		case kSprite:
			switch (filter) {
			case kFilterNone:
				if (blend_.a == 0) {
					sprite_->Draw(x + off_.x, y + off_.y);
				}
				else {
					DrawAndBlend(*sprite_, x + off_.x, y + off_.y, blend_);
				}
				break;
			case kFilterFog:
				if (blend_.a == 0) {
					DrawAndBlend(*sprite_, x + off_.x, y + off_.y, Rgba(0, 0, 0, 0x60));
				}
				else {
					DrawAndBlend2(*sprite_, x + off_.x, y + off_.y, blend_, Rgba(0, 0, 0, 0x60));
				}
				break;
			case kFilterTransparent:
				// TODO: draw this on another sprite with regular draw and then blend
				// into main backbuffer, otherwise all platform "internals" became visible
				//DrawWithFixedAlphaBlend(*sprite_, x + off_.x, y + off_.y, 0x60);
				break;
			}
			break;
		}
	}
}