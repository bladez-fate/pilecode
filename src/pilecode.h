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

#pragma once

#include "defs.h"

#include "engine/easy.h"
#include "engine/vec2si32.h"
#include "engine/vec3si32.h"

#include <vector>
#include <cmath>

namespace pilecode {

	namespace screen {
		extern int w;
		extern int h;
		extern int cx;
		extern int cy;
		extern size_t size;
	}

	class Robot;
	class Tile;
	class Platform;
	class WorldParams;
	class World;

	struct Pos;
	class ViewPort;

	enum TileType {
		kTlNone = 0,
		kTlBrick,
		kTlExit,

		kTlMax
	};

	enum Letter {
		kLtSpace = 0,      // emptyness
		kLtUp,             // y+
		kLtDown,           // y-
		kLtRight,          // x+
		kLtLeft,           // x-
		kLtRead,           // read letter from upper platform into register
		kLtWrite,          // write letter from register to upper platform

		kLtMax		
	};

	class Tile {
	public:
		// rendering
		void Draw(ViewPort* vp, int wx, int wy, int wz, int color);

		// simulation
		Letter ReadLetter();
		void WriteLetter(Letter letter);

		bool IsMovable() const;

		TileType type() const { return type_;  }
		Letter letter() const { return letter_;  }
		void set_type(TileType type) { type_ = type;  }
		void set_letter(Letter letter) { letter_ = letter; }
		bool touched() const { return touched_; }

		static const Tile* none()
		{
			static Tile noneTile;
			return &noneTile;
		}
	private:
		TileType type_ = kTlNone;
		Letter letter_ = kLtSpace;
		bool touched_ = false;
	};

	class WorldData {
	public:
		explicit WorldData(size_t colors);
		ae::Sprite* TileSprite(int color, TileType type);
	private:
		std::vector<std::vector<ae::Sprite>> tileSprite_; // tile_[color][tileType]
	};

	class WorldParams {
	public:
		WorldParams(size_t xsize, size_t ysize, size_t zsize, size_t colors);

		size_t xsize() const { return xsize_; }
		size_t ysize() const { return ysize_; }
		size_t zsize() const { return zsize_; }
		WorldData& data() { return *data_; }

		size_t size() const { return xyzsize_; }
		size_t index(int x, int y, int z) const { return z * xysize_ + y * xsize_ + x; }
	private:
		size_t xsize_;
		size_t ysize_;
		size_t zsize_;
		size_t xysize_;
		size_t xyzsize_;
		std::shared_ptr<WorldData> data_;
	};

	class Platform {
	public:
		Platform(int x, int y, int z, std::initializer_list<std::initializer_list<int>> data);
		void Draw(ViewPort* vp);
		Platform* Clone();
		void SwitchLetter(int rx, int ry);

		Tile* changable_tile(int rx, int ry);
		const Tile* get_tile(int rx, int ry) const;
		bool ReadLetter(int rx, int ry, Letter& letter);
		bool WriteLetter(int rx, int ry, Letter letter);

		// transforms coordinates relative to platform to world's frame
		int WorldX(int rx) const { return rx + x_; }
		int WorldY(int ry) const { return ry + y_; }
		int WorldZ(int rz) const { return rz + z_; }
		ar::Vec3Si32 ToWorld(int rx, int ry, int rz) const { return ar::Vec3Si32(rx + x_, ry + y_, rz + z_); }

		// inverse transform
		int PlatformX(int wx) const { return wx - x_; }
		int PlatformY(int wy) const { return wy - y_; }
		int PlatformZ(int wz) const { return wz - z_; }

		// accessors
		int index() const { return index_; }
		void set_index(int index) { index_ = index; }

	private:
		int index_;

		int x_;
		int y_;
		int z_;

		int w_;
		int h_;

		std::vector<Tile> tiles_;
	};

	class Robot {
	public:
		enum Direction {
			kDirHalt,
			kDirRight,
			kDirUp,
			kDirLeft,
			kDirDown,
		};
	public:
		Robot(int platform, int x, int y);
		void Draw(ViewPort* vp);
		void SimulateExec(World* world);
		void SimulateMove(World* world);
		Robot* Clone();

		ar::Vec2Si32 d_pos();

		// accessors
		int platform() const { return platform_; }
		int x() const { return x_; }
		int y() const { return y_; }
	private:
		// robot configuration
		const int seed_;

		// robot is currently on this platform
		int platform_;

		// coordinates are relative to platform
		int x_;
		int y_;
		int px_; // previous state
		int py_; // previous state 

		// simulation state
		Direction dir_ = kDirHalt; // direction of motion
		Letter reg_ = kLtSpace; // robot has one register that can hold a letter
		bool blocked_ = false; // robot blocks if it cannot execute current instruction
	};

	class World {
	public:
		explicit World(const WorldParams& wparams);

		// rendering
		void Draw(ViewPort* vp);

		// construction
		void AddPlatform(Platform* platform);
		void AddRobot(Robot* robot);
		void SwitchLetter(ar::Vec3Si32 w);
		void SwitchRobot(ar::Vec3Si32 w);
		bool IsTouched(ar::Vec3Si32 w);

		// simulation
		void Simulate();
		bool ReadLetter(ar::Vec3Si32 w, Letter& letter);
		bool WriteLetter(ar::Vec3Si32 w, Letter letter);
		bool IsMovable(ar::Vec3Si32 w);

		// utility
		World* Clone();
		Platform* FindPlatform(ar::Vec3Si32 w);

		// accessors
		Platform* platform(int i) const { return platform_[i].get(); }
		Robot* robot(int i) const { return robot_[i].get(); }
		WorldParams& params() { return wparams_; }
		size_t steps() const { return steps_; }
	private:
		WorldParams wparams_;
		std::vector<std::shared_ptr<Platform>> platform_;
		std::vector<std::shared_ptr<Robot>> robot_;
		size_t steps_ = 0;
	};

	struct Pos {
		// world to screen representation parameters
		static int dx;
		static int dy;
		static int dz;

		// screen coordinates
		int x;
		int y;

		// world coordinates
		int wx;
		int wy;
		int wz;

		Pos(int _wx, int _wy, int _wz)
			: x(dx * (_wy - _wx))
			, y(-dy * (_wx + _wy) + dz *_wz)
			, wx(_wx), wy(_wy), wz(_wz)
		{}

		explicit Pos(ar::Vec2Si32 w)
			: Pos(w.x, w.y, 0)
		{}

		explicit Pos(ar::Vec3Si32 w)
			: Pos(w.x, w.y, w.z)
		{}

		void Up()
		{
			wy++;
			x += dx;
			y -= dy;
		}

		void Down()
		{
			wy--;
			x -= dx;
			y += dy;
		}

		void Right()
		{
			wx++;
			x -= dx;
			y -= dy;
		}

		void Left()
		{
			wx--;
			x += dx;
			y += dy;
		}

		void Ceil()
		{
			wz++;
			y += dz;
		}

		void Floor()
		{
			wz--;
			y -= dz;
		}

		ar::Vec2Si32 Screen() const
		{
			return ar::Vec2Si32(x, y);
		}

		static ar::Vec2Si32 ToScreen(ar::Vec2Si32 w)
		{
			return Pos(w).Screen();
		}

		static ar::Vec3Si32 ToWorld(ar::Vec2Si32 s, int wz)
		{
			s.y -= dz * wz;
			int wx = int(-(float(s.x) / dx + float(s.y) / dy) / 2);
			int wy = int((float(s.x) / dx - float(s.y) / dy) / 2);

			return ar::Vec3Si32(wx, wy, wz);
		}
	};

	class ViewPort {
	public:
		ViewPort(const WorldParams& wparams);

		// drawing
		void Draw(ae::Sprite* sprite, int wx, int wy, int wz, int zlayer, ar::Vec2Si32 off);
		void Draw(ae::Sprite* sprite, int wx, int wy, int wz, int zlayer);
		void Draw(ae::Sprite* sprite, ar::Vec3Si32 w, int zlayer, ar::Vec2Si32 off);
		void Draw(ae::Sprite* sprite, ar::Vec3Si32 w, int zlayer);

		// rendering
		void BeginRender(double time);
		void EndRender();

		// navigation
		void Move(ar::Vec2F delta);
		void IncVisibleZ();
		void DecVisibleZ();

		// simulation support
		double progress() const { return progress_; }
		void set_progress(double progress) { progress_ = progress; }

		// transformations
		ar::Vec3Si32 ToWorld(ar::Vec2Si32 p) const;

		// world-related
		World* world() const { return world_; }
		void set_world(World* world) { world_ = world;  }

	private:
		Pos GetPos(int wx, int wy, int wz = 0);

	private:
		// world
		WorldParams wparams_;
		World* world_ = nullptr;

		// screen coordinates of origin
		int cx_ = screen::cx;
		int cy_ = screen::cy;

		// screen offset in pixels
		float x_ = 0;
		float y_ = 0;
		
		// world rendering parameters
		size_t visible_z_ = 0;

		// time-related
		double lastFrameTime_ = 0.0;
		double curFrameTime_ = 0.0;
		double progress_ = 1.0; // 0 - previous world state, 1 - current world state 

		// rendering artifacts
		static constexpr size_t zlBits = 2ull;
		static constexpr size_t zlSize = 1ull << zlBits;
		struct RenderCmnd {
			enum Type {
				kSprite = 0,
				kSpriteRgba = 1,
			};

			enum Filter {
				kFilterNone = 0,
				kFilterFog = 1,
			};

			Type type_;

			ae::Sprite* sprite_;
			ar::Vec2Si32 off_;

			RenderCmnd(ae::Sprite* sprite, ar::Vec2Si32 off_);
			void Apply(int x, int y, Filter filter);
		};

		using RenderList = std::vector<RenderCmnd>;
		std::vector<RenderList> cmnds_;
	};
}
