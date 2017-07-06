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

#include "engine/easy.h"
#include "engine/vec2si32.h"
#include "engine/vec3si32.h"

#include <vector>

namespace pilecode {

	namespace screen {
		extern int w;
		extern int h;
		extern int cx;
		extern int cy;
		extern size_t size;
	}

	namespace ar = arctic;
	namespace ae = arctic::easy;

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

		kLtMax		
	};

	class Tile {
	public:
		Tile();
		void Draw(ViewPort* vp, int wx, int wy, int wz);

		bool isMovable() const;

		TileType type() const { return type_;  }
		Letter letter() const { return letter_;  }
		void set_type(TileType type) { type_ = type;  }
		void set_letter(Letter letter) { letter_ = letter; }

		static const Tile* none()
		{
			static Tile noneTile;
			return &noneTile;
		}
	private:
		TileType type_;
		Letter letter_;
	};

	class Platform {
	public:
		Platform(int x, int y, int z, std::initializer_list<std::initializer_list<int>> data);
		void Draw(ViewPort* vp);

		const Tile* get_tile(int rx, int ry) const;
		
		// converts coordinates relative to platform to world's frame
		int worldX(int rx) const { return rx + x_; }
		int worldY(int ry) const { return ry + y_; }
		int worldZ(int rz) const { return rz + z_; }
	private:
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
			kDirRight,
			kDirUp,
			kDirLeft,
			kDirDown,
		};
	public:
		Robot(Platform* platform, int x, int y, Direction dir);
		void Draw(ViewPort* vp);
		void Simulate();

		ar::Vec2Si32 d_pos();
	private:
		// robot is currently on this platform
		Platform* platform_;

		// coordinates are relative to platform
		int x_;
		int y_;
		int px_; // previous state
		int py_; // previous state 

		// direction of motion
		Direction dir_;
	};

	class WorldParams {
	public:
		WorldParams(size_t xsize, size_t ysize, size_t zsize)
			: xsize_(xsize), ysize_(ysize), zsize_(zsize)
			, xysize_(xsize * ysize)
			, xyzsize_(xsize * ysize * zsize)
		{}

		size_t xsize() const { return xsize_; }
		size_t ysize() const { return ysize_; }
		size_t zsize() const { return zsize_; }
		
		size_t size() const { return xyzsize_; }
		size_t index(int x, int y, int z) const { return z * xysize_ + y * xsize_ + x;  }
	private:
		size_t xsize_;
		size_t ysize_;
		size_t zsize_;
		size_t xysize_;
		size_t xyzsize_;
	};

	class World {
	public:
		// rendering
		void Draw(ViewPort* vp);

		// construction
		void AddPlatform(Platform* platform);
		void AddRobot(Robot* robot);

		// simulation
		void Simulate();
	private:
		std::vector<std::shared_ptr<Platform>> platform_;
		std::vector<std::shared_ptr<Robot>> robot_;
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

	private:
		Pos GetPos(int wx, int wy, int wz = 0);

	private:
		WorldParams wparams_;

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

			Type type_;

			ae::Sprite* sprite_;
			ar::Vec2Si32 off_;

			RenderCmnd(ae::Sprite* sprite, ar::Vec2Si32 off_);
			void Apply(int x, int y);
		};

		using RenderList = std::vector<RenderCmnd>;
		std::vector<RenderList> cmnds_;
	};
}
