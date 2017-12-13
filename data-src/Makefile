# The MIT License(MIT)
#
# Copyright 2017 bladez-fate
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.


################################################################################
# REQUIREMENTS:
# - only MacOS is supported
# - imagemagick>=7.0.7-7 (brew install imagemagick)
# - inkscape installed in /Applications/ (not by brew)

################################################################################
# Folders
#
SUBDIRS = bg game music sfx ui
DATA = ../data
OBJ = ../data-obj

BG = $(DATA)/bg
GAME = $(DATA)/game
MUSIC = $(DATA)/music
SFX = $(DATA)/sfx
UI = $(DATA)/ui

OBG = $(OBJ)/bg
OGAME = $(OBJ)/game
OMUSIC = $(OBJ)/music
OSFX = $(OBJ)/sfx
OUI = $(OBJ)/ui

################################################################################
# TOOLS
#
CONVERT = convert
INKSCAPE = /Applications/Inkscape.app/Contents/Resources/bin/inkscape -z
CP = cp

################################################################################
# AUXILATY RULES
#
.PHONY: $(SUBDIRS)

all: $(SUBDIRS)

################################################################################
# RULES FOR bg FOLDER
#
bg: $(BG)/pilecode-1440x900.tga $(BG)/pilecode-1680x1050.tga

$(OBG)/pilecode-1440x900.png: bg/bladez-fate/pilecode.svg
	mkdir -p `dirname $@`
	$(INKSCAPE) -e `pwd`/$@ -w 1440 -h 900 `pwd`/$<

$(OBG)/pilecode-1680x1050.png: bg/bladez-fate/pilecode.svg
	mkdir -p `dirname $@`
	$(INKSCAPE) -e `pwd`/$@ -w 1680 -h 1050 `pwd`/$<

################################################################################
# RULERS FOR game FOLDER
#
game: $(GAME)/empty.tga $(GAME)/letter-frame.tga $(GAME)/spritesheet.tga $(GAME)/tile-mask.tga

$(GAME)/%.tga: game/bladez-fate/%.tga
	$(CP) $< $@

$(OGAME)/spritesheet.png: game/bladez-fate/spritesheet.svg
	mkdir -p `dirname $@`
	$(INKSCAPE) -e `pwd`/$@ `pwd`/$<

################################################################################
# COMMON RULES
#
$(DATA)/%.tga: $(OBJ)/%.png
	$(CONVERT) $< $@
