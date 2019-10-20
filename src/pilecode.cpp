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
#include "ui.h"

#include "engine/arctic_math.h"

#include <sstream>
#include <unordered_map>

namespace pilecode {

	Si32 Pos::dx = 14 * 4;
	Si32 Pos::dy = 7 * 4;
	Si32 Pos::dz = 25 * 4;

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

	void Tile::Draw(ViewPort* vp, Si32 wx, Si32 wy, Si32 wz, Si32 color)
	{
		// tile brick
		if (type_ != kTlNone) {
			Sprite* sprite = vp->world()->params().data().TileSprite(color, type_);
			vp->Draw(sprite, wx, wy, wz, 1, Vec2Si32(0, 0))
				.Alpha();
		}

		// output
		if (output_ != kLtSpace) {
			Sprite* sprite = output_ == letter_ ?
				&image::g_letter_output_filled[output_] :
				&image::g_letter_output[output_];
			vp->Draw(sprite, wx, wy, wz, 1)
				.Alpha()
				.PassEventThrough();
		}

		// letter
		if (letter_ != kLtSpace) {
			vp->Draw(&image::g_letter[letter_], wx, wy, wz, 1, Vec2Si32(0, 0))
				.Alpha()
				.PassEventThrough();
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
		for (Si32 wz = 0; wz < colors; wz++) {
			auto& ts = tileSprite_[wz];
			ts.resize(kTlMax);
			float alpha = float(wz) / (colors - 1);
			for (Si32 i = 0; i < kTlMax; i++) {
				TileType t = TileType(i);
				Sprite& src = image::g_tile[t];
				Sprite& dst = ts[i];
				dst.Create(src.Width(), src.Height());
				Rgba* srcIt0 = src.RgbaData();
				Rgba* dstIt0 = dst.RgbaData();
				for (Si32 y = 0; y < src.Height(); srcIt0 += src.StridePixels(), dstIt0 += dst.StridePixels(), y++) {
					Rgba* srcIt = srcIt0;
					Rgba* dstIt = dstIt0;
					for (Si32 x = 0; x < src.Width(); srcIt++, dstIt++, x++) {
						*dstIt = *srcIt;
						Si16 gdelta = Si16(dstIt->g * alpha);
						Si16 bdelta = Si16(dstIt->b * alpha);
						dstIt->g += bdelta - gdelta;
						dstIt->b += gdelta - bdelta;
					}
				}
			}
		}
	}

	Sprite* WorldData::TileSprite(Si32 color, TileType type)
	{
		color = color % tileSprite_.size();
		return &tileSprite_[color][type];
	}

	WorldParams::WorldParams()
	{
		// intended to be used with LoadFrom()
	}

	WorldParams::WorldParams(Si32 xsize, Si32 ysize, Si32 zsize, Si32 colors)
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

	Platform::Platform(Si32 x, Si32 y, Si32 z,
		std::initializer_list<std::initializer_list<Si32>> data)
		: x_(x), y_(y), z_(z)
	{
		w_ = 0;
		h_ = 0;
		for (auto& xdata : data) {
			if (w_ == 0) {
                w_ = (Si32)xdata.size();
			}
			h_++;
		}

		tiles_.resize(w_ * h_);

		Tile* tile = &tiles_[0];
		for (auto& xdata : data) {
			Tile* t = tile;
			for (Si32 x : xdata) {
				t->set_type(TileType(x));
				t++;
			}
			tile += w_;
		}
	}

	void Platform::Draw(ViewPort * vp)
	{
		Tile* tile = &tiles_[0];
		for (Si32 iy = 0; iy < h_; iy++) {
			for (Si32 ix = 0; ix < w_; ix++) {
				tile->Draw(vp, WorldX(ix), WorldY(iy), z_, index());
				tile++;
			}
		}
	}

	Platform* Platform::Clone() const
	{
		return new Platform(*this);
	}

	// Returns previous letter on changed tile iff successful
	Result<Letter> Platform::SetLetter(World* world, Si32 rx, Si32 ry, Letter letter)
	{
		if (Tile* tile = changable_tile(rx, ry)) {
			if (tile->IsModifiable()) {
				Letter prevLetter = tile->letter();
				if (prevLetter != letter) {
					tile->set_letter(letter);
					return MakeResult(kRsOk, prevLetter);
				}
				else if (letter == kLtSpace) {
					return MakeResult(kRsAlready, prevLetter);
				}
				tile->set_letter(kLtSpace);
				return MakeResult(kRsUndone, prevLetter);
			}
			return MakeResult(kRsForbidden);
		}
		return MakeResult(kRsNotFound);
	}

	Tile* Platform::changable_tile(Si32 rx, Si32 ry)
	{
		if (rx >= 0 && rx < w_ && ry >= 0 && ry < h_) {
			return &tiles_[ry * w_ + rx];
		} else {
			return nullptr;
		}
	}

	const Tile* Platform::get_tile(Si32 rx, Si32 ry) const
	{
		if (rx >= 0 && rx < w_ && ry >= 0 && ry < h_) {
			return &tiles_[ry * w_ + rx];
		}
		else {
			return Tile::none();
		}
	}

	bool Platform::ReadLetter(Si32 rx, Si32 ry, Letter& letter)
	{
		if (Tile* tile = changable_tile(rx, ry)) {
			letter = tile->ReadLetter();
			return true;
		}
		else {
			return false;
		}
	}

	bool Platform::WriteLetter(Si32 rx, Si32 ry, Letter letter)
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
		for (Si32 ry = 0; ry < h_; ry++, w.y++) {
			w.x = x_;
			for (Si32 rx = 0; rx < w_; rx++, w.x++, tile++) {
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

	void Robot::CalculatePosition(ViewPort* vp, Vec3Si32& w, Vec2Si32& off, Si32& body_off_y) const
	{
		Platform* p = vp->world()->platform(platform_);
		w = p->ToWorld(px_, py_, 0);

		off = Pos::ToScreen(d_pos());
		off.x = Si32(off.x * vp->progress());
		off.y = Si32(off.y * vp->progress());

		body_off_y = (Si32)round(4.0 * sin((vp->progress() + (seed_ % 1000) / 1000.0) * 2.0 * M_PI));
	}

	void Robot::Draw(ViewPort* vp)
	{
		Vec3Si32 w;
		Vec2Si32 off;
		Si32 body_off_y;
		CalculatePosition(vp, w, off, body_off_y);

		vp->Draw(&image::g_robotShadow, w, 2, off)
			.Alpha()
			.PassEventThrough();
		vp->Draw(&image::g_robot, w, 2, Vec2Si32(off.x, off.y + body_off_y))
			.Alpha()
			.Interactive(0, this);

		if (reg_ != kLtSpace) {
			vp->Draw(&image::g_letter[reg_], w, 2,
				Vec2Si32(off.x, off.y + body_off_y + g_yrobotReg))
				.Alpha()
				.PassEventThrough();
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
                    default: // unknown letter is equivalent to space
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
					case kLtInput: {
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
					case kLtOutput:
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

	Vec2Si32 Robot::d_pos() const
	{
		return Vec2Si32(x_ - px_, y_ - py_);
	}

	Vec2Si32 Robot::dir_delta() const
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
        default:
            return Vec2Si32(0, 0);
		}
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
		// intended to be used with LoadFrom() or LoadFromText()
	}

	World::World(const WorldParams & wparams)
		: wparams_(wparams)
	{
		for (Si32 i = 0; i < kLtMax; i++) {
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
		platform->set_index((Si32)platform_.size());
		platform_.emplace_back(platform);
	}

	void World::AddRobot(Robot * robot)
	{
		robot_.emplace_back(robot);
	}

	// returns previous letter on changed tile iff successful
	Result<Letter> World::SetLetter(Vec3Si32 w, Letter letter)
	{
		for (const auto& p : platform_) {
			if (p->WorldZ(0) == w.z) {
				auto res = p->SetLetter(this, p->PlatformX(w.x), p->PlatformY(w.y), letter);
				if (res.status != kRsNotFound) {
					return res;
				}
			}
		}
		return MakeResult(kRsNotFound);
	}

	void World::SwitchRobot(Vec3Si32 w, const Robot& original)
	{
		if (Platform* p = FindPlatform(w)) {
			Si32 rx = p->PlatformX(w.x);
			Si32 ry = p->PlatformY(w.y);

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
		for (Si32 i = 0; i < kLtMax; i++) {
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

		// adjustment for screen center
		xmin_ += screen::cx;
		xmax_ += screen::cx;
		ymin_ += screen::cy;
		ymax_ += screen::cy;
        
		// adjustment for tile position
		xmin_ -= g_xtileorigin;
		xmax_ -= g_xtileorigin;
		ymin_ -= g_ytileorigin;
		ymax_ -= g_ytileorigin;

        // adjustment for screen size
        float xscreen = (float)screen::w;
        float yscreen = (float)screen::h;
        if (xmax_ - xmin_ < xscreen) {
            xmin_ = xmax_ = (xmin_ + xmax_ ) / 2;
        } else {
            xmin_ += xscreen / 2;
            xmax_ -= xscreen / 2;
        }
        if (ymax_ - ymin_ < yscreen) {
            ymin_ = ymax_ = (ymin_ + ymax_ ) / 2;
        } else {
            ymin_ += yscreen / 2;
            ymax_ -= yscreen / 2;
        }

		Center();
	}

	ViewPort::RenderCmnd* ViewPort::GetRenderCmnd(Sprite* sprite, Si32 wx, Si32 wy, Si32 wz)
	{
		for (Si32 zl = 0; zl < zlSize; zl++) {
			RenderList& rlist = renderList(wx, wy, wz, zl);
			for (RenderCmnd& cmnd : rlist.next) {
				if (cmnd.sprite_ == sprite) {
					return &cmnd;
				}
			}
		}
		return nullptr;
	}

	ViewPort::RenderCmnd* ViewPort::GetRenderCmnd(Sprite* sprite, Vec3Si32 w)
	{
		return GetRenderCmnd(sprite, w.x, w.y, w.z);
	}

	ViewPort::RenderCmnd& ViewPort::Draw(Sprite* sprite, Si32 wx, Si32 wy, Si32 wz, Si32 zl, Vec2Si32 off)
	{
		RenderList& rlist = renderList(wx, wy, wz, zl);
		rlist.next.emplace_back(RenderCmnd(RenderCmnd::kSprite, sprite, off));
		return rlist.next.back();
	}

	ViewPort::RenderCmnd& ViewPort::Draw(Sprite* sprite, Si32 wx, Si32 wy, Si32 wz, Si32 zl)
	{
		return Draw(sprite, wx, wy, wz, zl, Vec2Si32(0, 0));
	}

	ViewPort::RenderCmnd& ViewPort::Draw(Sprite* sprite, Vec3Si32 w, Si32 zl, Vec2Si32 off)
	{
		return Draw(sprite, w.x, w.y, w.z, zl, off);
	}

	ViewPort::RenderCmnd& ViewPort::Draw(Sprite* sprite, Vec3Si32 w, Si32 zl)
	{
		return Draw(sprite, w, zl, Vec2Si32(0, 0));
	}

	ViewPort::RenderCmnd& ViewPort::DrawShadow(Si32 wx, Si32 wy, Si32 wz, Si32 zl)
	{
		RenderList& rlist = renderList(wx, wy, wz, zl);
		rlist.next.emplace_back(RenderCmnd(Shadow(world_, Vec3Si32(wx, wy, wz))));
		return rlist.next.back();
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
		RenderList* rlist = &cmnds_[0];
		drawn_z_ = std::min(visible_z_ + 1, wparams_.zsize());
		for (Pos p2 = GetPos(0, 0); p2.wz < drawn_z_; p2.Ceil()) {
			for (Si32 zl = 0; zl < zlSize; zl++) {
				RenderCmnd::Filter filter = p2.wz < visible_z_ ? RenderCmnd::kFilterNone : RenderCmnd::kFilterTransparent;
				for (Pos p1 = p2; p1.wy < wparams_.ysize(); p1.Up()) {
					for (Pos p0 = p1; p0.wx < wparams_.xsize(); p0.Right()) {
						for (RenderCmnd& cmnd : rlist->next) {
							cmnd.Apply(this, p0.x, p0.y, filter);
						}
						rlist->EndRender();
						rlist++;
					}
				}
			}
		}
	}

	void ViewPort::DrawCeiling(Vec3Si32 w)
	{
		// TODO: start/finish animation???
		Si32 xRadius = Pos::dx;
		Si32 yRadius = Pos::dy;
		Si32 aspect = Pos::dx / Pos::dy;
		Si32 aspectSq = aspect*aspect;

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
                    Ui32 k = std::min(256 - alpha, (Ui32)fg->a);
					*bg = RgbaSum(
						RgbaMult(*fg, k),
						RgbaMult(*bg, 256 - k)
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

	void ViewPort::Event(Vec2Si32 s, std::function<void(EventHandling& eh, Ui64 tag, void* data)> handler)
	{
		EventHandling eh(this);
		Si32 zsize = drawn_z_ - 1; // do not pass events to transparent ceiling z-level
		Pos p2 = GetPos(wparams_.xsize() - 1, wparams_.ysize() - 1, zsize - 1);
		RenderList* rlist = &renderList(p2.wx, p2.wy, p2.wz, zlSize - 1);
		for (; p2.wz >= 0; p2.Floor()) {
			for (eh.zl_ = zlSize - 1; eh.zl_ >= 0; eh.zl_--) {
				for (Pos p1 = p2; p1.wy >= 0; p1.Down()) {
					for (eh.p_ = p1; eh.p_.wx >= 0; eh.p_.Left()) {
						for (auto i = rlist->prev.rbegin(), e = rlist->prev.rend(); i != e; ++i) {
							RenderCmnd& cmnd = *i;
							if (cmnd.passing_ == kPass) {
								continue;
							}
							if (cmnd.IsHit(s, eh)) {
								if (cmnd.passing_ == kBlock) {
									return;
								}
								else { // kInteract
									handler(eh, cmnd.tag_, cmnd.data_);
									if (!eh.propagate_) {
										return;
									}
								}
							}
						}
						rlist--;
					}
				}
			}
		}
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

    void ViewPort::SetVisibleZ(Si32 z)
    {
        visible_z_ = ae::Clamp(z, 1, wparams_.zsize() + 1);
    }
    
	// Converts screen coords `p' into world coords at given z-level `wz'
	Vec3Si32 ViewPort::ToWorldAtZ(Si32 wz, Vec2Si32 p) const
	{
		p.x -= Si32(x_ + 0.5f);
		p.y -= Si32(y_ + 0.5f);
		p.x -= g_xtileorigin;
		p.y -= g_ytileorigin;
		return Pos::ToWorld(p, wz);
	}

	// Converts screen coords `p' into world coords and tile coords `tp' at given z-level `wz'
	Vec3Si32 ViewPort::ToWorldTileAtZ(Si32 wz, Vec2Si32 p, Vec2F& tp) const
	{
		p.x -= Si32(x_ + 0.5f);
		p.y -= Si32(y_ + 0.5f);
		p.x -= g_xtileorigin;
		p.y -= g_ytileorigin;
		Vec3Si32 result = Pos::ToWorld(p, wz);
		p -= Pos::ToScreen(result);
		tp = Pos::ToTile(p);
		return result;
	}

	// Search all z-levels for highest with real tile
	// and converts screen coords `p' into world coords `w' of that tile
	// Returns false iff real tile was not found (`w' is not changed)
	bool ViewPort::ToWorld(Vec2Si32 p, Vec3Si32& w) const
	{
		for (Si32 wz = visible_z_ - 1; wz >= 0; wz--) {
			Vec3Si32 w0 = ToWorldAtZ(wz, p);
			if (Tile* tile = world_->At(w0)) {
				w = w0;
				return true;
			}
		}
		return false;
	}

	// Search all z-levels for highest with real tile
	// and converts screen coords `p' into world coords `w' and tile coords `tp' of that tile
	// Returns false iff real tile was not found (`w' and `tp' are not changed)
	bool ViewPort::ToWorldTile(Vec2Si32 p, Vec3Si32& w, Vec2F& tp) const
	{
		for (Si32 wz = visible_z_ - 1; wz >= 0; wz--) {
			Vec2F tp0;
			Vec3Si32 w0 = ToWorldTileAtZ(wz, p, tp0);
			if (Tile* tile = world_->At(w0)) {
				tp = tp0;
				w = w0;
				return true;
			}
		}
		return false;
	}

	Pos ViewPort::GetPos(Si32 wx, Si32 wy, Si32 wz)
	{
		Pos p(wx, wy, wz);
		p.x += Si32(x_ + 0.5f);
		p.y += Si32(y_ + 0.5f);
		return p;
	}

	Sprite ViewPort::ShadowMask(Sprite& surfaceMask, const Shadow& shadow)
	{
		if (shadow.ceiling(0, 0)) {
			Sprite shadowMask;
			shadowMask.Create(surfaceMask.Width(), surfaceMask.Height());
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
		, passing_(kPass) // shadow shouldn't block events (which is default)
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

	ViewPort::RenderCmnd& ViewPort::RenderCmnd::Opacity(Ui8 value)
	{
		opacity_ = value;
		return *this;
	}

	ViewPort::RenderCmnd& ViewPort::RenderCmnd::Interactive(Ui64 tag, void* data)
	{
		passing_ = kInteract;
		tag_ = tag;
		data_ = data;
		return *this;
	}

	ViewPort::RenderCmnd& ViewPort::RenderCmnd::PassEventThrough()
	{
		passing_ = kPass;
		return *this;
	}

	void ViewPort::RenderCmnd::Apply(ViewPort* vp, Si32 x, Si32 y, ViewPort::RenderCmnd::Filter filter)
	{
		Sprite to_sprite = filter == kFilterTransparent? vp->transparent(): ae::GetEngine()->GetBackbuffer();
		x += off_.x;
		y += off_.y;
		switch (type_) {
		case kSprite:
			if (blend_.a == 0) {
                sprite_->Draw(x, y, sprite_->Width(), sprite_->Height(),
                              0, 0, sprite_->Width(), sprite_->Height(), to_sprite);
            }
			else {
				DrawAndBlend(*sprite_, x, y, to_sprite, blend_);
			}
			break;
		case kSpriteRgba:
			if (blend_.a == 0) {
				AlphaDraw(*sprite_, x, y, to_sprite, opacity_);
			}
			else {
				AlphaDrawAndBlend(*sprite_, x, y, to_sprite, blend_, opacity_);
			}
			break;
		case kShadow:
			AlphaDrawAndBlend(vp->ShadowMask(image::g_tileMask, shadow_), x, y, to_sprite, Rgba(0, 0, 0, 255), opacity_);
			break;
		}
	}

	bool ViewPort::RenderCmnd::IsHit(Vec2Si32 s, const EventHandling& eh, Ui8 alphaThreshold)
	{
		// Calculate sprite coordinates
		Vec2Si32 r = s - eh.p().Screen() + sprite_->Pivot();
		Rgba* to = sprite_->RgbaData()
			+ r.y * sprite_->StridePixels()
			+ r.x;
		return to->a > alphaThreshold;
	}
}
