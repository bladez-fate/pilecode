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

#include <vector>

namespace pilecode {

	namespace screen {
		extern int w;
		extern int h;
		extern int cx;
		extern int cy;
		extern size_t size;
	}

	namespace ae = arctic::easy;

	class Robot;
	class Tile;
	class Platform;
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
		void Draw(ViewPort* vp, Pos p);

		TileType type() const { return type_;  }
		Letter letter() const { return letter_;  }
		void set_type(TileType type) { type_ = type;  }
		void set_letter(Letter letter) { letter_ = letter; }
	private:
		TileType type_;
		Letter letter_;
	};

	class Platform {
	public:
		Platform(std::initializer_list<std::initializer_list<int>> data);
		void Draw(ViewPort* vp);
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
	private:
		Platform* platform_;

		// coordinates are relative to platform
		int x_;
		int y_;

		// direction of motion
		int dx_;
		int dy_;
	};

	class World {
	public:
		void Draw(ViewPort* vp);
		void AddPlatform(Platform* platform);
	private:
		std::vector<std::shared_ptr<Platform>> platform_;
	};

	struct Pos {
		// world to screen representation parameters
		static constexpr int dx = 14 * 4;
		static constexpr int dy = 7 * 4;
		static constexpr int dz = 48 * 4;

		// screen coordinates
		int x;
		int y;

		// world coordinates
		int wx;
		int wy;
		int wz;

		Pos(int _wx, int _wy, int _wz)
			: x(dx * (_wy - _wx))
			, y(-dy * (_wx + _wy))
			, wx(_wx), wy(_wy), wz(_wz)
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
	};

	class ViewPort {
	public:
		Pos GetPos(int wx, int wy, int wz = 0);
	private:
		// screen coordinates of origin
		int cx_ = screen::cx;
		int cy_ = screen::cy;

		// screen offset in pixels
		int x_ = 0;
		int y_ = 0;
	};
}
