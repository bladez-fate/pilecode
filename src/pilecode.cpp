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

#include "engine/arctic_math.h"

#include <sstream>

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

	Shadow::Shadow()
	{
		for (bool& v : ceiling_) {
			v = false;
		}
	}

	Shadow::Shadow(World* world, Vec3Si32 w)
	{
		for (Si32 dx = -1; dx <= 1; dx++) {
			for (Si32 dy = -1; dy <= 1; dy++) {
				ceiling(dx, dy) = world->At(w + Vec3Si32(dx, dy, 1));
			}
		}
	}

	bool& Shadow::ceiling(Si32 dx, Si32 dy)
	{
		// transform ranges: [-1, 0, 1] ---> [0, 1, 2]
		dx++;
		dy++;
		return ceiling_[dy * 3 + dx];
	}

	bool Shadow::ceiling(Si32 dx, Si32 dy) const
	{
		// transform ranges: [-1, 0, 1] ---> [0, 1, 2]
		dx++;
		dy++;
		return ceiling_[dy * 3 + dx];
	}

	void Tile::Draw(ViewPort* vp, int wx, int wy, int wz, int color)
	{
		// tile brick
		if (type_ != kTlNone) {
			Sprite* sprite = vp->world()->params().data().TileSprite(color, type_);
			vp->Draw(sprite, wx, wy, wz, 1, Vec2Si32(0, 0));
		}

		// output
		if (output_ != kLtSpace) {
			vp->Draw(&image::g_letter[output_], wx, wy, wz, 1,
				Vec2Si32(0, letter_ != kLtSpace ? -3 : 0))
				.Blend(Rgba(0, 0, 0, 255)).Alpha();
		}

		// letter
		if (letter_ != kLtSpace) {
			vp->Draw(&image::g_letter[letter_], wx, wy, wz, 1, Vec2Si32(0, 0)).Alpha();
		}

		// shadow
		if (type_ != kTlNone) {
			vp->DrawShadow(wx, wy, wz, 1);
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

	bool Tile::IsModifiable() const
	{
		return type_ == kTlBrick;
	}

	void Tile::SaveTo(std::ostream& s) const
	{
		Save(s, type_);
		Save(s, letter_);
		Save(s, touched_);
		Save(s, output_);
	}

	void Tile::LoadFrom(std::istream& s)
	{
		Load(s, type_);
		Load(s, letter_);
		Load(s, touched_);
		Load(s, output_);
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

	WorldParams::WorldParams()
	{
		// intended to be used with LoadFrom()
	}

	WorldParams::WorldParams(size_t xsize, size_t ysize, size_t zsize, size_t colors)
		: xsize_(xsize), ysize_(ysize), zsize_(zsize)
		, colors_(colors)
	{
		Init();
	}

	void WorldParams::Init()
	{
		xysize_ = xsize_ * ysize_;
		xyzsize_ = xsize_ * ysize_ * zsize_;
		data_.reset(new WorldData(colors_));
	}

	void WorldParams::SaveTo(std::ostream& s) const
	{
		Save(s, xsize_);
		Save(s, ysize_);
		Save(s, zsize_);
		Save(s, colors_);
	}

	void WorldParams::LoadFrom(std::istream& s)
	{
		Load(s, xsize_);
		Load(s, ysize_);
		Load(s, zsize_);
		Load(s, colors_);
		Init();
	}

	Platform::Platform()
	{
		// intended to be used with LoadFrom()
	}

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

	void Platform::SetLetter(World* world, int rx, int ry, Letter letter)
	{
		if (Tile* tile = changable_tile(rx, ry)) {
			if (tile->IsModifiable()) {
				if (tile->letter() != letter) {
					tile->set_letter(letter);
				}
				else {
					tile->set_letter(kLtSpace);
				}
			}
		}
	}

	void Platform::SwitchLetter(World* world, int rx, int ry)
	{
		if (Tile* tile = changable_tile(rx, ry)) {
			if (tile->IsModifiable()) {
				do {
					tile->set_letter(Letter((tile->letter() + 1) % kLtMax));
				} while (!world->IsLetterAllowed(tile->letter()));
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

	bool Platform::IsOutputCorrect()
	{
		for (Tile& tile : tiles_) {
			if (tile.output() != kLtSpace) {
				if (tile.letter() != tile.output()) {
					return false;
				}
			}
		}
		return true;
	}

	Tile* Platform::At(Vec3Si32 w)
	{
		if (w.z == z_) {
			if (Tile* tile = changable_tile(PlatformX(w.x), PlatformY(w.y))) {
				if (tile->type() != kTlNone) {
					return tile;
				}
			}
		}
		return nullptr;
	}

	void Platform::ForEachTile(std::function<void(Vec3Si32, Tile*)> func)
	{
		Tile* tile = &tiles_[0];
		Vec3Si32 w(x_, y_, z_);
		for (int ry = 0; ry < h_; ry++, w.y++) {
			w.x = x_;
			for (int rx = 0; rx < w_; rx++, w.x++, tile++) {
				if (tile->type() != kTlNone) {
					func(w, tile);
				}
			}
		}
	}

	void Platform::SaveTo(std::ostream& s) const
	{
		Save(s, index_);
		Save(s, x_);
		Save(s, y_);
		Save(s, z_);
		Save(s, w_);
		Save(s, h_);
		Save<size_t>(s, tiles_.size());
		for (const auto& tile : tiles_) {
			tile.SaveTo(s);
		}
	}

	void Platform::LoadFrom(std::istream& s)
	{
		Load(s, index_);
		Load(s, x_);
		Load(s, y_);
		Load(s, z_);
		Load(s, w_);
		Load(s, h_);
		size_t tiles;
		Load<size_t>(s, tiles);
		tiles_.resize(tiles);
		for (auto& tile : tiles_) {
			tile.LoadFrom(s);
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
			p->WorldX(px_),	p->WorldY(py_),	p->WorldZ(0),
			2,
			off)
			.Alpha();
		vp->Draw(&image::g_robot,
			p->WorldX(px_),	p->WorldY(py_),	p->WorldZ(0),
			2,
			Vec2Si32(off.x, off.y + body_off_y));

		if (reg_ != kLtSpace) {
			vp->Draw(&image::g_letter[reg_],
				p->WorldX(px_),	p->WorldY(py_),	p->WorldZ(0),
				2,
				Vec2Si32(off.x, off.y + body_off_y + g_yrobotReg))
				.Alpha();
		}
	}

	void Robot::SimulateExec(World* world)
	{
		if (executing_ > 0) {
			// simulate command execution
			executing_--;
		}
		else {
			// read command
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
					case kLtRead: {
						Letter letter;
						blocked_ = !world->ReadLetter(wu, letter);
						if (!blocked_) {
							executing_ = 1;
							if (letter != kLtSpace) {
								reg_ = letter;
							}
							sfx::g_read.Play();
						}
						break;
					}
					case kLtWrite:
						if (reg_ != kLtSpace) {
							blocked_ = !world->WriteLetter(wu, reg_);
							if (!blocked_) {
								executing_ = 1;
								sfx::g_write.Play();
							}
						}
						else {
							blocked_ = false;
						}
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
	}

	void Robot::PrepareMove(World* world)
	{
		Platform* p = world->platform(platform_);
		curr_ = p->ToWorld(x_, y_, 0);
		if (!blocked_ && !executing_ && dir_ != kDirHalt) {
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

			// use priority
			(r1->priority() < r2->priority() ? r1 : r2)->StopMove();
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

	void Robot::SaveTo(std::ostream& s) const
	{
		Save(s, seed_);
		Save(s, priority_);
		Save(s, platform_);
		Save(s, x_);
		Save(s, y_);
		Save(s, px_);
		Save(s, py_);
		Save(s, dir_);
		Save(s, reg_);
		Save(s, blocked_);
		Save(s, executing_);
	}

	void Robot::LoadFrom(std::istream& s)
	{
		Load(s, seed_);
		Load(s, priority_);
		Load(s, platform_);
		Load(s, x_);
		Load(s, y_);
		Load(s, px_);
		Load(s, py_);
		Load(s, dir_);
		Load(s, reg_);
		Load(s, blocked_);
		Load(s, executing_);
	}

	World::World()
	{
		// intended to be used with LoadFrom()
	}

	World::World(const WorldParams & wparams)
		: wparams_(wparams)
	{
		for (int i = 0; i < kLtMax; i++) {
			isLetterAllowed_[i] = false;
		}
		AllowLetter(kLtSpace);
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
		platform->set_index((int)platform_.size());
		platform_.emplace_back(platform);
	}

	void World::AddRobot(Robot * robot)
	{
		robot_.emplace_back(robot);
	}

	void World::SetLetter(Vec3Si32 w, Letter letter)
	{
		for (const auto& p : platform_) {
			if (p->WorldZ(0) == w.z) {
				p->SetLetter(this, p->PlatformX(w.x), p->PlatformY(w.y), letter);
			}
		}
	}

	void World::SwitchLetter(Vec3Si32 w)
	{
		for (const auto& p : platform_) {
			if (p->WorldZ(0) == w.z) {
				p->SwitchLetter(this, p->PlatformX(w.x), p->PlatformY(w.y));
			}
		}
	}

	void World::SwitchRobot(Vec3Si32 w, const Robot& original)
	{
		if (Platform* p = FindPlatform(w)) {
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

	bool World::IsLetterAllowed(Letter letter)
	{
		return isLetterAllowed_[letter];
	}

	void World::AllowLetter(Letter letter)
	{
		isLetterAllowed_[letter] = true;
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
		for (int i = 0; i < kLtMax; i++) {
			clone->isLetterAllowed_[i] = isLetterAllowed_[i];
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

	bool World::IsOutputCorrect()
	{
		for (const auto& p : platform_) {
			if (!p->IsOutputCorrect()) {
				return false;
			}
		}
		return true;
	}

	Tile* World::At(Vec3Si32 w)
	{
		for (const auto& p : platform_) {
			if (Tile* tile = p->At(w)) {
				return tile;
			}
		}
		return nullptr;
	}

	void World::ForEachTile(std::function<void(Vec3Si32, Tile*)> func)
	{
		for (const auto& p : platform_) {
			p->ForEachTile(func);
		}
	}

	void World::SaveTo(std::ostream & s) const
	{
		wparams_.SaveTo(s);
		Save<size_t>(s, platform_.size());
		for (const auto& p : platform_) {
			p->SaveTo(s);
		}
		Save<size_t>(s, robot_.size());
		for (const auto& r : robot_) {
			r->SaveTo(s);
		}
		Save<size_t>(s, kLtMax);
		for (size_t i = 0; i < kLtMax; i++) {
			Save(s, isLetterAllowed_[i]);
		}
		Save(s, steps_);
	}

	void World::LoadFrom(std::istream& s)
	{
		wparams_.LoadFrom(s);
		size_t platforms;
		Load(s, platforms);
		platform_.resize(platforms);
		for (auto& p : platform_) {
			p.reset(new Platform());
			p->LoadFrom(s);
		}
		size_t robots;
		Load(s, robots);
		robot_.resize(robots);
		for (auto& r : robot_) {
			r.reset(new Robot());
			r->LoadFrom(s);
		}
		size_t maxLetters;
		Load(s, maxLetters);
		for (size_t i = 0; i < maxLetters; i++) {
			Load(s, isLetterAllowed_[i]);
		}
		Load(s, steps_);
	}

	ViewPort::ViewPort(World* world)
		: wparams_(world->params())
		, cmnds_(wparams_.size() * zlSize)
		, visible_z_(wparams_.zsize())
	{
		transparent_.Create(screen::w, screen::h);
		transparent_.SetPivot(Vec2Si32(0, 0));
		xmin_ = std::numeric_limits<float>::max();
		ymin_ = std::numeric_limits<float>::max();
		xmax_ = std::numeric_limits<float>::min();
		ymax_ = std::numeric_limits<float>::min();
		world->ForEachTile([=](Vec3Si32 w, Tile*) {
			Pos p(w);
			if (xmin_ > -p.x) {
				xmin_ = -(float)p.x;
			}
			if (ymin_ > -p.y) {
				ymin_ = -(float)p.y;
			}
			if (xmax_ < -p.x) {
				xmax_ = -(float)p.x;
			}
			if (ymax_ < -p.y) {
				ymax_ = -(float)p.y;
			}
		});
		
		//xmin_ = xmax_ = ymin_ = ymax_ = 0;

		// adjustments for the fact that Pos gives coords of the top corner of tile
		xmin_ -= Pos::dx;
		xmax_ += Pos::dx;
		ymin_ -= 2*Pos::dy;
		ymax_ += Pos::dz;

		// adjustment for screen size
		xmin_ += screen::cx;
		xmax_ += screen::cx;
		ymin_ += screen::cy;
		ymax_ += screen::cy;

		// adjustment for tile position
		xmin_ -= g_xtileorigin;
		xmax_ -= g_xtileorigin;
		ymin_ -= g_ytileorigin;
		ymax_ -= g_ytileorigin;

		Center();
	}

	ViewPort::RenderCmnd& ViewPort::Draw(Sprite* sprite, int wx, int wy, int wz, int zl, Vec2Si32 off)
	{
		RenderList& rlist = renderList(wx, wy, wz, zl);
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

	ViewPort::RenderCmnd& ViewPort::DrawShadow(int wx, int wy, int wz, int zl)
	{
		RenderList& rlist = renderList(wx, wy, wz, zl);
		rlist.emplace_back(RenderCmnd(Shadow(world_, Vec3Si32(wx, wy, wz))));
		return rlist.back();
	}

	void ViewPort::BeginRender(double time)
	{
		curFrameTime_ = time;
		if (lastFrameTime_ == 0.0) {
			lastFrameTime_ = curFrameTime_ - 1.0;
		}
		transparent_.Clear();
	}

	void ViewPort::ApplyCommands()
	{
		Pos p2 = GetPos(0, 0);
		RenderList* rlist = &cmnds_[0];
		for (int iz = 0, ez = (int)std::min(visible_z_ + 1, wparams_.zsize()); iz < ez; iz++, p2.Ceil()) {
			for (int zl = 0; zl < zlSize; zl++) {
				RenderCmnd::Filter filter = (
					iz == visible_z_ - 1 ?
					RenderCmnd::kFilterNone :
					(iz < visible_z_ ? RenderCmnd::kFilterNone : RenderCmnd::kFilterTransparent)
					);
				Pos p1 = p2;
				for (int iy = 0; iy < wparams_.ysize(); iy++, p1.Up()) {
					Pos p0 = p1;
					for (int ix = 0; ix < wparams_.xsize(); ix++, p0.Right()) {
						for (RenderCmnd& cmnd : *rlist) {
							cmnd.Apply(this, p0.x, p0.y, filter);
						}
						// note that render lists are cleared after rendering
						rlist->clear();
						rlist++;
					}
				}
			}
		}
	}

	void ViewPort::DrawCeiling(Vec3Si32 w)
	{
		// TODO: start/finish animation???
		int xRadius = Pos::dx;
		int yRadius = Pos::dy;
		int aspect = Pos::dx / Pos::dy;
		int aspectSq = aspect*aspect;

		Si32 rsqMax = xRadius*xRadius + yRadius*yRadius*aspectSq;
		Sprite bb = ae::GetEngine()->GetBackbuffer();
		
		Pos pos = GetPos(w.x, w.y, w.z);
		Si32 cx = pos.x + g_tileCenter.x;
		Si32 cy = pos.y + g_tileCenter.y + Pos::dz;

		Si32 x1 = cx - 2 * xRadius;
		Si32 x2 = cx + 2 * xRadius + 1;
		Si32 y1 = cy - 2 * yRadius;
		Si32 y2 = cy + 2 * yRadius + 1;

		x1 = (x1 < 0 ? 0 : x1);
		x2 = (x2 > bb.Width() ? bb.Width() : x2);
		y1 = (y1 < 0 ? 0 : y1);
		y2 = (y2 > bb.Height() ? bb.Height() : y2);

		Si32 offs = y1 * bb.Width() + x1;
		Rgba* bg = bb.RgbaData() + offs;
		Rgba* fg = transparent_.RgbaData() + offs;

		for (Si32 y = y1; y < y2; y++) {
			Ui64 ysq = (y - cy)*(y - cy)*aspectSq;
			Rgba* bg0 = bg;
			Rgba* fg0 = fg;
			for (Si32 x = x1; x < x2; x++) {
				Ui64 rsq = (x - cx)*(x - cx) + ysq;
				Ui32 alpha = Ui32(arctic::Clamp(float(rsq) / rsqMax, 0.0f, 1.0f) * 256.0f + 0.5f);
				if (fg->a > 0) {
					*bg = RgbaSum(
						RgbaMult(*fg, 256 - alpha),
						RgbaMult(*bg, alpha)
					);
				}
				fg++;
				bg++;
			}
			bg = bg0 + bb.Width();
			fg = fg0 + bb.Width();
		}
		//DrawWithFixedAlphaBlend(transparent_, 0, 0, 128);
	}

	void ViewPort::EndRender(bool drawCeiling, Vec3Si32 w)
	{
		ApplyCommands();
		if (drawCeiling) {
			DrawCeiling(w);
		}
		lastFrameTime_ = curFrameTime_;
	}

	void ViewPort::Move(Vec2F delta)
	{
		x_ = ae::Clamp(x_ + delta.x, xmin_, xmax_);
		y_ = ae::Clamp(y_ + delta.y, ymin_, ymax_);
	}

	void ViewPort::MoveNoClamp(Vec2F delta)
	{
		x_ += delta.x;
		y_ += delta.y;
	}

	void ViewPort::Locate(Vec2F loc)
	{
		x_ = loc.x;
		y_ = loc.y;
	}

	void ViewPort::Center()
	{
		Locate(Vec2F((xmin_ + xmax_) / 2, (ymin_ + ymax_) / 2));
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

	Vec3Si32 ViewPort::ToWorldAtZ(int wz, Vec2Si32 p) const
	{
		p.x -= int(x_ + 0.5f);
		p.y -= int(y_ + 0.5f);
		p.x -= g_xtileorigin;
		p.y -= g_ytileorigin;
		return Pos::ToWorld(p, wz);
	}

	bool ViewPort::ToWorld(Vec2Si32 p, Vec3Si32& w) const
	{
		for (int wz = int(visible_z_) - 1; wz >= 0; wz--) {
			Vec3Si32 w0 = ToWorldAtZ(wz, p);
			if (Tile* tile = world_->At(w0)) {
				w = w0;
				return true;
			}
		}
		return false;
	}

	Pos ViewPort::GetPos(int wx, int wy, int wz)
	{
		Pos p(wx, wy, wz);
		p.x += int(x_ + 0.5f);
		p.y += int(y_ + 0.5f);
		return p;
	}

	Sprite ViewPort::ShadowMask(Sprite& surfaceMask, const Shadow& shadow)
	{
		if (shadow.ceiling(0, 0)) {
			Sprite shadowMask;
			shadowMask.Create(surfaceMask.Width(), surfaceMask.Height());
			shadowMask.SetPivot(Vec2Si32(0, 0));
			Rgba* shad = shadowMask.RgbaData();
			Rgba* surf = surfaceMask.RgbaData();
			for (Si32 y = 0; y < shadowMask.Height(); y++) {
				for (Si32 x = 0; x < shadowMask.Width(); x++, shad++, surf++) {
					if (surf->a > 0) {
						Si32 tx = x - g_xtileorigin;
						Si32 ty = y - g_ytileorigin;
						Vec2F tr = Pos::ToTile(Vec2Si32(tx, ty));
						
						float light = 0.0f;
						float contrast = 6.0f;
						if (!shadow.ceiling(1, 0)) {
							light = std::max(light, 1.0f - (1.0f - tr.x) * contrast);
						}
						if (!shadow.ceiling(0, 1)) {
							light = std::max(light, 1.0f - (1.0f - tr.y) * contrast);
						}
						if (!shadow.ceiling(-1, 0)) {
							light = std::max(light, 1.0f - (tr.x) * contrast);
						}
						if (!shadow.ceiling(0, -1)) {
							light = std::max(light, 1.0f - (tr.y) * contrast);
						}

						if (!shadow.ceiling(1, 1)) {
							light = std::max(light, 1.0f - (2.0f - tr.x - tr.y) * contrast);
						}
						if (!shadow.ceiling(1, -1)) {
							light = std::max(light, 1.0f - (1.0f - tr.x + tr.y) * contrast);
						}
						if (!shadow.ceiling(-1, 1)) {
							light = std::max(light, 1.0f - (1.0f - tr.y + tr.x) * contrast);
						}
						if (!shadow.ceiling(-1, -1)) {
							light = std::max(light, 1.0f - (tr.x + tr.y) * contrast);
						}
						shad->a = (Ui8)std::max(0.0f, std::min(255.0f, float(surf->a) * (1.0f - light) / 8.0f));
					}
				}
			}
			return shadowMask;
		}
		else {
			return image::g_empty;
		}
	}

	ViewPort::RenderCmnd::RenderCmnd(ViewPort::RenderCmnd::Type type,
		Sprite* sprite, Vec2Si32 off)
		: type_(type)
		, sprite_(sprite)
		, off_(off)
	{}

	ViewPort::RenderCmnd::RenderCmnd(const Shadow& shadow)
		: type_(kShadow)
		, shadow_(shadow)
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

	void ViewPort::RenderCmnd::Apply(ViewPort* vp, int x, int y, ViewPort::RenderCmnd::Filter filter)
	{
		Sprite to_sprite = filter == kFilterTransparent? vp->transparent(): ae::GetEngine()->GetBackbuffer();
		x += off_.x;
		y += off_.y;
		switch (type_) {
		case kSprite:
			if (blend_.a == 0) {
				DrawSprite(*sprite_, x, y, to_sprite);
			}
			else {
				DrawAndBlend(*sprite_, x, y, to_sprite, blend_);
			}
			break;
		case kSpriteRgba:
			if (blend_.a == 0) {
				AlphaDraw(*sprite_, x, y, to_sprite);
			}
			else {
				AlphaDrawAndBlend(*sprite_, x, y, to_sprite, blend_);
			}
			break;
		case kShadow:
			AlphaDrawAndBlend(vp->ShadowMask(image::g_tileMask, shadow_), x, y, to_sprite, Rgba(0, 0, 0, 255));
			break;
		}
	}
}