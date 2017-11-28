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

#include "levels.h"

#include <vector>
#include <memory>

namespace pilecode {
	struct Levels {
		std::vector<std::shared_ptr<World>> worlds_;
		std::vector<std::shared_ptr<World>> sandboxes_;

		void AddLevel(World* world)
		{
			worlds_.emplace_back(world);
		}

		void AddSandbox(World* world)
		{
			sandboxes_.emplace_back(world);
		}

		Levels()
		{
			{
				WorldParams wparams(200, 200, 3, 4);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 0, 0, 1, 1, 1, 1, 0 },
						{ 0, 1, 1, 0, 1, 1, 1 },
						{ 0, 1, 2, 2, 2, 1, 1 },
						{ 0, 1, 2, 2, 2, 1, 1 },
						{ 1, 1, 1, 1, 1, 1, 1 },
						{ 0, 1, 1, 0, 1, 0, 0 }
					});

				Platform* plat2 = new Platform(
					0, 10, 0,
					{
						{ 0, 1, 1, 1, 1, 1, 0 },
						{ 1, 1, 1, 1, 1, 1, 1 },
						{ 1, 1, 1, 1, 1, 1, 1 },
						{ 0, 1, 1, 1, 1, 1, 0 },
					});

				Platform* plat3 = new Platform(
					2, 2, 1,
					{
						{ 0, 1, 1 },
						{ 1, 1, 1 },
						{ 0, 0, 1 },
						{ 0, 0, 1 },
						{ 0, 0, 1 },
						{ 0, 0, 2 },
						{ 0, 0, 2 },
						{ 0, 0, 1 },
						{ 0, 0, 1 },
						{ 0, 0, 1 },
					});

				Platform* plat4 = new Platform(
					1, 7, 2,
					{
						{ 2, 2, 2, 2, 2, 2, 2 },
						{ 2, 2, 2, 2, 2, 2, 2 },
					});
				plat4->changable_tile(0, 0)->set_output(kLtRight);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);
				world->AddPlatform(plat4);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddSandbox(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 1, },
						{ 1, },
						{ 1, },
						{ 1, },
						{ 1, },
						{ 1, },
						{ 1, },
						{ 1, },
						{ 1, },
					});

				Platform* plat2 = new Platform(
					0, 1, 1,
					{
						{ 2 },
					});

				Platform* plat3 = new Platform(
					0, 7, 1,
					{
						{ 2 },
					});

				plat1->changable_tile(0, 0)->set_letter(kLtUp);
				plat1->changable_tile(0, 1)->set_letter(kLtInput);
				plat1->changable_tile(0, 7)->set_letter(kLtOutput);

				plat2->changable_tile(0, 0)->set_letter(kLtDot);
				plat3->changable_tile(0, 0)->set_output(kLtDot);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				Robot* r = new Robot(world, Vec3Si32(0, 0, 0));
				world->AddRobot(r);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
					});

				Platform* plat2 = new Platform(
					1, 1, 1,
					{
						{ 2 },
					});

				Platform* plat3 = new Platform(
					1, 7, 1,
					{
						{ 2 },
					});

				plat2->changable_tile(0, 0)->set_letter(kLtDot);
				plat3->changable_tile(0, 0)->set_output(kLtDot);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 0, 1, },
						{ 1, 0, 1, },
						{ 1, 0, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
					});

				Platform* plat2 = new Platform(
					1, 1, 1,
					{
						{ 2 },
					});

				Platform* plat3 = new Platform(
					1, 7, 1,
					{
						{ 2 },
					});

				plat2->changable_tile(0, 0)->set_letter(kLtDot);
				plat3->changable_tile(0, 0)->set_output(kLtDot);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 1, 0, 1, },
						{ 1, 0, 1, },
						{ 1, 0, 1, },
						{ 1, 0, 1, },
						{ 1, 0, 1, },
						{ 1, 0, 1, },
						{ 1, 0, 1, },
						{ 1, 0, 1, },
						{ 1, 0, 1, },
					});

				Platform* plat2 = new Platform(
					0, 1, 1,
					{
						{ 2, 1, 1 },
					});

				Platform* plat3 = new Platform(
					0, 7, 1,
					{
						{ 2, 1, 2 },
					});

				plat2->changable_tile(0, 0)->set_letter(kLtRight);
				plat3->changable_tile(0, 0)->set_output(kLtRight);
				plat3->changable_tile(2, 0)->set_output(kLtLeft);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
					});

				Platform* plat2 = new Platform(
					1, 1, 1,
					{
						{ 2, 2, 2 },
					});

				Platform* plat3 = new Platform(
					1, 7, 1,
					{
						{ 2, 2, 2 },
					});

				plat2->changable_tile(0, 0)->set_letter(kLtDot);
				plat2->changable_tile(1, 0)->set_letter(kLtDot);
				plat2->changable_tile(2, 0)->set_letter(kLtDot);
				plat3->changable_tile(0, 0)->set_output(kLtDot);
				plat3->changable_tile(1, 0)->set_output(kLtDot);
				plat3->changable_tile(2, 0)->set_output(kLtDot);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, },
					});

				Platform* plat2 = new Platform(
					1, 1, 1,
					{
						{ 2, 0, 2 },
					});

				Platform* plat3 = new Platform(
					1, 3, 1,
					{
						{ 2, 0, 2 },
					});

				plat2->changable_tile(0, 0)->set_letter(kLtUp);
				plat2->changable_tile(2, 0)->set_output(kLtDown);
				plat3->changable_tile(0, 0)->set_output(kLtUp);
				plat3->changable_tile(2, 0)->set_letter(kLtDown);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, 1, },
						{ 1, 1, 1, 1, },
						{ 1, 1, 1, 1, },
						{ 1, 1, 1, 1, },
						{ 1, 1, 1, 1, },
						{ 1, 1, 1, 1, },
						{ 1, 1, 1, 1, },
						{ 1, 1, 1, 1, },
						{ 1, 1, 1, 1, },
					});

				Platform* plat2 = new Platform(
					1, 2, 1,
					{
						{ 2, 2 },
					});

				Platform* plat3 = new Platform(
					1, 6, 1,
					{
						{ 2, 2 },
					});

				plat2->changable_tile(0, 0)->set_letter(kLtLeft);
				plat2->changable_tile(1, 0)->set_letter(kLtRight);
				plat3->changable_tile(0, 0)->set_output(kLtRight);
				plat3->changable_tile(1, 0)->set_output(kLtLeft);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 3, 6);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 0, 1, 0, 1, 0, },
						{ 0, 1, 0, 1, 0, },
						{ 0, 1, 0, 1, 0, },
						{ 0, 1, 0, 1, 0, },
						{ 0, 1, 0, 1, 0, },
					});

				Platform* plat2 = new Platform(
					0, 0, 1,
					{
						{ 0, 0, 0, 0, 0, },
						{ 1, 1, 1, 1, 1, },
						{ 0, 0, 0, 0, 0, },
						{ 1, 2, 2, 2, 1, },
						{ 0, 0, 0, 0, 0, },
					});

				Platform* plat3 = new Platform(
					0, 0, 2,
					{
						{ 0, 2, 0, 2, 0, },
						{ 0, 2, 0, 2, 0, },
						{ 0, 2, 0, 2, 0, },
						{ 0, 2, 0, 2, 0, },
						{ 0, 2, 0, 2, 0, },
					});

				plat3->changable_tile(1, 1)->set_letter(kLtDot);
				plat3->changable_tile(1, 3)->set_letter(kLtDot);
				plat3->changable_tile(3, 1)->set_output(kLtDot);
				plat3->changable_tile(3, 3)->set_output(kLtDot);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, },
					});

				Platform* plat2 = new Platform(
					2, 2, 1,
					{
						{ 2, 2, 2, 2 },
					});

				Platform* plat3 = new Platform(
					2, 8, 1,
					{
						{ 2, 2, 2, 2 },
					});

				plat2->changable_tile(0, 0)->set_letter(kLtUp);
				plat2->changable_tile(1, 0)->set_letter(kLtRight);
				plat2->changable_tile(2, 0)->set_letter(kLtLeft);
				plat2->changable_tile(3, 0)->set_letter(kLtDown);

				plat3->changable_tile(0, 0)->set_output(kLtDown);
				plat3->changable_tile(1, 0)->set_output(kLtLeft);
				plat3->changable_tile(2, 0)->set_output(kLtRight);
				plat3->changable_tile(3, 0)->set_output(kLtUp);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
					});

				Platform* plat2 = new Platform(
					0, 0, 1,
					{
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 2, 2, 2, 2, 2, 1, },
						{ 1, 2, 2, 2, 2, 2, 1, },
						{ 1, 2, 2, 2, 2, 2, 1, },
						{ 1, 2, 2, 2, 2, 2, 1, },
						{ 1, 2, 2, 2, 2, 2, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
					});

				for (int y = 0; y <= 6; y++) {
					for (int x = 0; x <= 6; x++) {
						if (x == 0 || y == 0 || x == 6 || y == 6) {
							plat2->changable_tile(x, y)->set_letter(kLtDot);
						}
						else {
							plat2->changable_tile(x, y)->set_output(kLtDot);
						}
					}
				}

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 3, 2);
				World* world = new World(wparams);
				Platform* plat21 = new Platform(
					1, 3, 2,
					{
						{ 2, },
					});
				Platform* plat22 = new Platform(
					5, 3, 2,
					{
						{ 2, },
					});
				Platform* plat1 = new Platform(
					0, 0, 1,
					{
						{ 0, 0, 0, 0, 0, 0, 0, },
						{ 0, 1, 1, 1, 1, 1, 0, },
						{ 0, 1, 0, 0, 0, 1, 0, },
						{ 0, 2, 0, 1, 0, 2, 0, },
						{ 0, 1, 0, 0, 0, 1, 0, },
						{ 0, 1, 1, 1, 1, 1, 0, },
						{ 0, 0, 0, 0, 0, 0, 0, },
					});
				Platform* plat0 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, },
					});
				plat21->changable_tile(0, 0)->set_letter(kLtDot);
				plat22->changable_tile(0, 0)->set_output(kLtDot);

				world->AddPlatform(plat0);
				world->AddPlatform(plat1);
				world->AddPlatform(plat21);
				world->AddPlatform(plat22);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 3, 2);
				World* world = new World(wparams);
				Platform* plat21 = new Platform(
					2, 2, 2,
					{
						{ 2, },
						{ 2, },
						{ 2, },
					});
				Platform* plat22 = new Platform(
					9, 2, 2,
					{
						{ 2, },
						{ 2, },
						{ 2, },
					});
				Platform* plat1 = new Platform(
					0, 0, 1,
					{
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
					});
				Platform* plat0 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, },
					});
				plat21->changable_tile(0, 0)->set_letter(kLtDown);
				plat21->changable_tile(0, 1)->set_letter(kLtLeft);
				plat21->changable_tile(0, 2)->set_letter(kLtUp);

				plat22->changable_tile(0, 0)->set_output(kLtDown);
				plat22->changable_tile(0, 1)->set_output(kLtLeft);
				plat22->changable_tile(0, 2)->set_output(kLtUp);

				world->AddPlatform(plat0);
				world->AddPlatform(plat1);
				world->AddPlatform(plat21);
				world->AddPlatform(plat22);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 2, 3);
				World* world = new World(wparams);
				Platform* plat1 = new Platform(
					0, 0, 0,
					{
						{ 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
						{ 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, },
					});

				Platform* plat2 = new Platform(
					4, 3, 1,
					{
						{ 1, 1 },
						{ 1, 1 },
					});

				Platform* plat3 = new Platform(
					4, 10, 1,
					{
						{ 2, 2 },
						{ 2, 2 },
					});

				plat3->changable_tile(0, 0)->set_output(kLtDown);
				plat3->changable_tile(0, 1)->set_output(kLtLeft);
				plat3->changable_tile(1, 0)->set_output(kLtRight);
				plat3->changable_tile(1, 1)->set_output(kLtUp);

				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}

			{
				WorldParams wparams(200, 200, 4, 4);
				World* world = new World(wparams);
				Platform* plat3 = new Platform(
					0, 0, 3,
					{
						{ 2, 2, 2, },
						{ 2, 2, 2, },
						{ 2, 2, 2, },
					});
				Platform* plat2 = new Platform(
					0, 0, 2,
					{
						{ 2, 2, 2, },
						{ 2, 2, 2, },
						{ 2, 2, 2, },
					});
				Platform* plat1 = new Platform(
					0, 0, 1,
					{
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
					});
				Platform* plat0 = new Platform(
					0, 0, 0,
					{
						{ 1, 1, 1, },
						{ 1, 1, 1, },
						{ 1, 1, 1, },
					});
				plat3->changable_tile(0, 1)->set_letter(kLtDot);
				plat3->changable_tile(2, 1)->set_output(kLtDot);

				plat2->changable_tile(0, 1)->set_letter(kLtOutput);
				plat2->changable_tile(2, 1)->set_letter(kLtInput);

				plat2->changable_tile(0, 0)->set_letter(kLtRight);
				plat2->changable_tile(2, 0)->set_letter(kLtUp);
				plat2->changable_tile(0, 2)->set_letter(kLtDown);
				plat2->changable_tile(2, 2)->set_letter(kLtLeft);

				world->AddPlatform(plat0);
				world->AddPlatform(plat1);
				world->AddPlatform(plat2);
				world->AddPlatform(plat3);

				world->AllowLetter(kLtUp);
				world->AllowLetter(kLtRight);
				world->AllowLetter(kLtDown);
				world->AllowLetter(kLtLeft);
				world->AllowLetter(kLtInput);
				world->AllowLetter(kLtOutput);

				AddLevel(world);
			}
		}

		static const Levels* Instance()
		{
			static Levels levels;
			return &levels;
		}
	};

	World* GenerateLevel(int level)
	{
		auto levels = Levels::Instance();
		if (level >= 0) {
			size_t idx = level % levels->worlds_.size();
			return levels->worlds_[idx]->Clone();
		}
		else {
			size_t idx = (-level - 1) % levels->sandboxes_.size();
			return levels->sandboxes_[idx]->Clone();
		}
	}

	size_t LevelsCount()
	{
		auto levels = Levels::Instance();
		return levels->worlds_.size();
	}
}
