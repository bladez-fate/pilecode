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

#include "data.h"
#include "music.h"

#include "engine/easy.h"

namespace pilecode {

	namespace {
		int g_musicIdx = 0;
		bool g_musicDisabled = false;
        bool g_musicRandomize = true;
	}

	void UpdateMusic()
	{
		if (!g_musicDisabled) {
			// switch background music tracks
			if (!music::g_background[g_musicIdx].IsPlaying()) {
				g_musicIdx = (g_musicIdx + 1) % music::g_background.size();
				music::g_background[g_musicIdx].Play(0.2f);
			}
		}
		else {
			if (music::g_background[g_musicIdx].IsPlaying()) {
				music::g_background[g_musicIdx].Stop();
			}

            if (g_musicRandomize) {
                g_musicIdx = rand() % music::g_background.size();
                g_musicRandomize = false;
            } else {
                g_musicIdx = (g_musicIdx + 1) % music::g_background.size();
            }
		}
	}

	void ToggleMusic()
	{
		g_musicDisabled = !g_musicDisabled;
        g_musicRandomize = true;
	}

	bool IsMusicEnabled()
	{
		return !g_musicDisabled;
	}

}
