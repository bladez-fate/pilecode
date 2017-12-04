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
#include "result.h"

#include "engine/easy.h"
#include "engine/vec2si32.h"
#include "engine/vec3si32.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>

namespace pilecode {

	namespace screen {
		extern Si32 w;
		extern Si32 h;
		extern Si32 cx;
		extern Si32 cy;
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
		kTlInactive,

		kTlMax
	};

	enum Letter {
		kLtSpace = 0,      // emptyness
		kLtRight,          // x+
		kLtDown,           // y-
		kLtUp,             // y+
		kLtLeft,           // x-
		kLtInput,           // read letter from upper platform into register
		kLtOutput,          // write letter from register to upper platform
		kLtDot,

		kLtMax		
	};

	class Shadow {
	public:
		Shadow();
		Shadow(World* world, Vec3Si32 w);
		bool& ceiling(Si32 dx, Si32 dy);
		bool ceiling(Si32 dx, Si32 dy) const;
	private:
		bool ceiling_[9]; // 3x3 ceiling bitmask (0=sky; 1=ceiling)
	};

	class Tile {
	public:
		// rendering
		void Draw(ViewPort* vp, Si32 wx, Si32 wy, Si32 wz, Si32 color);

		// simulation
		Letter ReadLetter();
		void WriteLetter(Letter letter);

		// utility
		bool IsMovable() const;
		bool IsModifiable() const;
		void SaveTo(std::ostream& s) const;
		void LoadFrom(std::istream& s);

		// accessors
		TileType type() const { return type_;  }
		void set_type(TileType type) { type_ = type;  }
		Letter letter() const { return letter_;  }
		void set_letter(Letter letter) { letter_ = letter; }
		bool touched() const { return touched_; }
		Letter output() const { return output_; }
		void set_output(Letter letter) { output_ = letter; }
		
		static const Tile* none()
		{
			static Tile noneTile;
			return &noneTile;
		}
	private:
		// state
		TileType type_ = kTlNone;
		Letter letter_ = kLtSpace;
		bool touched_ = false;

		// output
		Letter output_ = kLtSpace;
	};

	class WorldData {
	public:
		explicit WorldData(size_t colors);
		Sprite* TileSprite(Si32 color, TileType type);
	private:
		std::vector<std::vector<Sprite>> tileSprite_; // tile_[color][tileType]
	};

	class WorldParams {
	public:
		WorldParams();
		WorldParams(Si32 xsize, Si32 ysize, Si32 zsize, Si32 colors);
		void Init();

		Si32 xsize() const { return xsize_; }
		Si32 ysize() const { return ysize_; }
		Si32 zsize() const { return zsize_; }
		WorldData& data() { return *data_; }

		Si32 size() const { return xyzsize_; }
		Si32 index(Si32 x, Si32 y, Si32 z) const { return z * xysize_ + y * xsize_ + x; }

		// utility
		void SaveTo(std::ostream& s) const;
		void LoadFrom(std::istream& s);

	private:
		Si32 xsize_;
		Si32 ysize_;
		Si32 zsize_;
		Si32 colors_;

		Si32 xysize_;
		Si32 xyzsize_;
		std::shared_ptr<WorldData> data_;
	};

	class Platform {
	public:
		Platform();
		Platform(Si32 x, Si32 y, Si32 z, std::initializer_list<std::initializer_list<Si32>> data);
		void Draw(ViewPort* vp);
		Platform* Clone() const;
		Result<Letter> SetLetter(World* world, Si32 rx, Si32 ry, Letter letter);
		
		Tile* changable_tile(Si32 rx, Si32 ry);
		const Tile* get_tile(Si32 rx, Si32 ry) const;
		bool ReadLetter(Si32 rx, Si32 ry, Letter& letter);
		bool WriteLetter(Si32 rx, Si32 ry, Letter letter);
		bool IsOutputCorrect();

		// transforms coordinates relative to platform to world's frame
		Si32 WorldX(Si32 rx) const { return rx + x_; }
		Si32 WorldY(Si32 ry) const { return ry + y_; }
		Si32 WorldZ(Si32 rz) const { return rz + z_; }
		Vec3Si32 ToWorld(Si32 rx, Si32 ry, Si32 rz) const { return Vec3Si32(rx + x_, ry + y_, rz + z_); }
		Tile* At(Vec3Si32 w);

		// inverse transform
		Si32 PlatformX(Si32 wx) const { return wx - x_; }
		Si32 PlatformY(Si32 wy) const { return wy - y_; }
		Si32 PlatformZ(Si32 wz) const { return wz - z_; }

		// utility
		void ForEachTile(std::function<void(Vec3Si32, Tile*)> func);
		void SaveTo(std::ostream& s) const;
		void LoadFrom(std::istream& s);

		// accessors
		Si32 index() const { return index_; }
		void set_index(Si32 index) { index_ = index; }

	private:
		Si32 index_;

		Si32 x_;
		Si32 y_;
		Si32 z_;

		Si32 w_;
		Si32 h_;

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
		// construction
		Robot();
		Robot(World* world, Vec3Si32 w);
		void Place(World* world, Vec3Si32 w);

		// rendering
		void Draw(ViewPort* vp);

		// simulation
		void SimulateExec(World* world);
		void PrepareMove(World* world);
		static size_t ResolveMove(Robot* r1, Robot* r2);
		void StopMove();
		void SimulateMove(World* world);

		// utility
		Robot* Clone() const;
		Vec2Si32 d_pos() const;
		Vec2Si32 dir_delta() const;
		void SaveTo(std::ostream& s) const;
		void LoadFrom(std::istream& s);
		
		// accessors
		Si32 priority() const { return priority_; }
		Si32 set_priority() const { return priority_; }
		Si32 platform() const { return platform_; }
		Si32 x() const { return x_; }
		Si32 y() const { return y_; }
	private:
		void CalculatePosition(ViewPort* vp, Vec3Si32& w, Vec2Si32& off, Si32& body_off_y) const;
	private:
		// robot configuration
		Si32 seed_;
		Si32 priority_;

		// robot is currently on this platform
		Si32 platform_;

		// coordinates are relative to platform
		Si32 x_;
		Si32 y_;
		Si32 px_; // previous state
		Si32 py_; // previous state 

		// simulation state
		Direction dir_ = kDirHalt; // direction of motion
		Letter reg_ = kLtSpace; // robot has one register that can hold a letter
		bool blocked_ = false; // robot blocks if it cannot execute current instruction
		Si32 executing_ = 0;

		// simulation intermediates (not serializable)
		Vec3Si32 curr_;
		Vec3Si32 next_;
	};

	class World {
	public:
		World();
		explicit World(const WorldParams& wparams);

		// rendering
		void Draw(ViewPort* vp);

		// construction
		void AddPlatform(Platform* platform);
		void AddRobot(Robot* robot);
		Result<Letter> SetLetter(Vec3Si32 w, Letter letter);
		void SwitchRobot(Vec3Si32 w, const Robot& original);
		bool IsTouched(Vec3Si32 w);
		bool IsLetterAllowed(Letter letter);
		void AllowLetter(Letter letter);

		// simulation
		void Simulate();
		bool ReadLetter(Vec3Si32 w, Letter& letter);
		bool WriteLetter(Vec3Si32 w, Letter letter);
		bool IsMovable(Vec3Si32 w);

		// utility
		World* Clone() const;
		Platform* FindPlatform(Vec3Si32 w);
		bool IsOutputCorrect();
		Tile* At(Vec3Si32 w);
		void ForEachTile(std::function<void(Vec3Si32, Tile*)> func);
		void SaveTo(std::ostream& s) const;
		void LoadFrom(std::istream& s);

		// accessors
		Platform* platform(Si32 i) const { return platform_[i].get(); }
		Robot* robot(Si32 i) const { return robot_[i].get(); }
		WorldParams& params() { return wparams_; }
		size_t steps() const { return steps_; }
	private:
		WorldParams wparams_;
		std::vector<std::shared_ptr<Platform>> platform_;
		std::vector<std::shared_ptr<Robot>> robot_;
		bool isLetterAllowed_[kLtMax];
		size_t steps_ = 0;
	};

	struct Pos {
		// world to screen representation parameters
		static Si32 dx;
		static Si32 dy;
		static Si32 dz;

		// screen coordinates
		Si32 x;
		Si32 y;

		// world coordinates
		Si32 wx;
		Si32 wy;
		Si32 wz;

		Pos(Si32 _wx, Si32 _wy, Si32 _wz)
			: x(dx * (_wy - _wx))
			, y(-dy * (_wx + _wy) + dz *_wz)
			, wx(_wx), wy(_wy), wz(_wz)
		{}

		explicit Pos(Vec2Si32 w)
			: Pos(w.x, w.y, 0)
		{}

		explicit Pos(Vec3Si32 w)
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

		Vec3Si32 World() const
		{
			return Vec3Si32(wx, wy, wz);
		}

		Vec2Si32 Screen() const
		{
			return Vec2Si32(x, y);
		}

		static Vec2Si32 ToScreen(Vec2Si32 w)
		{
			return Pos(w).Screen();
		}

		static Vec2Si32 ToScreen(Vec3Si32 w)
		{
			return Pos(w).Screen();
		}

		static Vec3Si32 ToWorld(Vec2Si32 s, Si32 wz)
		{
			s.y -= dz * wz;
			Si32 wx = Si32(-(float(s.x) / dx + float(s.y) / dy) / 2.0f);
			Si32 wy = Si32( (float(s.x) / dx - float(s.y) / dy) / 2.0f);

			return Vec3Si32(wx, wy, wz);
		}

		static Vec2F ToTile(Vec2Si32 s)
		{
			float wx = -(float(s.x) / dx + float(s.y) / dy) / 2.0f;
			float wy =  (float(s.x) / dx - float(s.y) / dy) / 2.0f;

			return Vec2F(wx, wy);
		}
	};

	class ViewPort {
	public:
		struct RenderCmnd;
		friend struct RenderCmnd;
		struct RenderList;
		class EventHandling;

	public:
		enum EventPassing {
			kBlock = 0, // blocks further event passing (e.g. tile brick)
			kPass = 1, // pass event further (e.g. shadow)
			kInteract = 2, // calls event handler (e.g. robot)
		};

		struct RenderCmnd {
			enum Type {
				kSprite = 0,
				kSpriteRgba = 1,
				kShadow = 2,
			};

			enum Filter {
				kFilterNone = 0,
				kFilterTransparent,
			};

			Type type_;

			// for kSprite and kSpriteRgba
			Sprite* sprite_ = nullptr;
			Vec2Si32 off_ = Vec2Si32(0, 0);
			Rgba blend_ = Rgba(Ui32(0));
			Ui8 opacity_ = 0xff;

			// for kShadow
			Shadow shadow_;

			// for event handling
			EventPassing passing_ = kBlock;
			Ui64 tag_ = 0;
			void* data_ = nullptr;
		public:
			RenderCmnd(Type type, Sprite* sprite, Vec2Si32 off_);
			explicit RenderCmnd(const Shadow& shadow);

			RenderCmnd& Blend(Rgba rgba);
			RenderCmnd& Alpha();
			RenderCmnd& Opacity(Ui8 value);
			RenderCmnd& Interactive(Ui64 tag = 0, void* data = nullptr);
			RenderCmnd& PassEventThrough();
		private:
			void Apply(ViewPort* vp, Si32 x, Si32 y, Filter filter);
			bool IsHit(Vec2Si32 s, const EventHandling& eh, Ui8 alphaThreshold = 0x80);
			friend class ViewPort;
		};

		struct RenderList {
			std::vector<RenderCmnd> next;
			std::vector<RenderCmnd> prev;

			void EndRender()
			{
				prev.clear();
				std::swap(next, prev);
			}
		};

		class EventHandling {
		public:
			explicit EventHandling(ViewPort* vp)
				: vp_(vp)
				, p_(0, 0, 0)
			{}
			void StopPropagate() { propagate_ = false; }
			ViewPort* vp() const { return vp_; }
			Pos p() const { return p_; }
			Si32 zl() const { return zl_; }
		private:
			ViewPort* vp_;
			Pos p_;
			Si32 zl_;
			bool propagate_ = true;
			friend class ViewPort;
		};

	public:
		explicit ViewPort(World* world);

		// drawing
		RenderCmnd::Filter FilterMode(Si32 wz);
		RenderCmnd* GetRenderCmnd(Sprite* sprite, Si32 wx, Si32 wy, Si32 wz);
		RenderCmnd* GetRenderCmnd(Sprite* sprite, Vec3Si32 w);
		RenderCmnd& Draw(Sprite* sprite, Si32 wx, Si32 wy, Si32 wz, Si32 zl, Vec2Si32 off);
		RenderCmnd& Draw(Sprite* sprite, Si32 wx, Si32 wy, Si32 wz, Si32 zl);
		RenderCmnd& Draw(Sprite* sprite, Vec3Si32 w, Si32 zl, Vec2Si32 off);
		RenderCmnd& Draw(Sprite* sprite, Vec3Si32 w, Si32 zl);
		RenderCmnd& DrawShadow(Si32 wx, Si32 wy, Si32 wz, Si32 zl);

		// rendering
		void BeginRender(double time);
		void EndRender(bool drawCeiling, Vec3Si32 w);

		// backtrack
		void Event(Vec2Si32 s, std::function<void(EventHandling& eh, Ui64 tag, void* data)> handler);

		// navigation
		void Move(Vec2F delta);
		void MoveNoClamp(Vec2F delta);
		void Locate(Vec2F loc);
		void Center();
		void IncVisibleZ();
		void DecVisibleZ();

		// simulation support
		double progress() const { return progress_; }
		void set_progress(double progress) { progress_ = progress; }

		// transformations
		Vec3Si32 ToWorldAtZ(Si32 wz, Vec2Si32 p) const;
		Vec3Si32 ToWorldTileAtZ(Si32 wz, Vec2Si32 p, Vec2F& tp) const;
		bool ToWorld(Vec2Si32 p, Vec3Si32& w) const;
		bool ToWorldTile(Vec2Si32 p, Vec3Si32& w, Vec2F& tp) const;

		// world-related
		World* world() const { return world_; }
		void set_world(World* world) { world_ = world; }

	private:
		void ApplyCommands();
		void DrawCeiling(Vec3Si32 w);

		Pos GetPos(Si32 wx, Si32 wy, Si32 wz = 0);
		Sprite ShadowMask(Sprite& surfaceMask, const Shadow& shadow);

		Sprite transparent() { return transparent_; }
		RenderList& renderList(Si32 wx, Si32 wy, Si32 wz, Si32 zl)
		{
			if (!(zl >= 0 && zl < zlSize)) {
				abort();
			}
			return cmnds_[wparams_.index(wx, wy, (wz << zlBits) + zl)];
		}

	private:
		// world
		WorldParams wparams_;
		World* world_ = nullptr;

		// screen offset in pixels
		float x_ = 0;
		float y_ = 0;
		float xmin_ = 0;
		float ymin_ = 0;
		float xmax_ = 0;
		float ymax_ = 0;

		// world rendering parameters
		Si32 visible_z_ = 0;
		Si32 drawn_z_ = 0; // z-layers drawn in current frame

		// time-related
		double lastFrameTime_ = 0.0;
		double curFrameTime_ = 0.0;
		double progress_ = 1.0; // 0 - previous world state, 1 - current world state 

		// rendering artifacts
		static constexpr size_t zlBits = 2ull;
		static constexpr size_t zlSize = 1ull << zlBits;
		std::vector<RenderList> cmnds_;
		Sprite transparent_;
	};

	template <class T>
	void Save(std::ostream& os, const T& t)
	{
		// works fine for PODs if you dont bother about endians
		os.write(reinterpret_cast<const char*>(&t), sizeof(T));
	}

	template <class T>
	void Load(std::istream& is, T& t)
	{
		// works fine for PODs if you dont bother about endians
		is.read(reinterpret_cast<char*>(&t), sizeof(T));
	}
}
