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

CONVERT = convert
INKSCAPE = /Applications/Inkscape.app/Contents/Resources/bin/inkscape -z
CP = cp

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)

bg: $(BG)/pilecode-1440x900.tga $(BG)/pilecode-1680x1050.tga

$(OBG)/pilecode-1440x900.png: bg/bladez-fate/pilecode.svg
	mkdir -p `dirname $@`
	$(INKSCAPE) -e `pwd`/$@ -w 1440 -h 900 `pwd`/$<

$(OBG)/pilecode-1680x1050.png: bg/bladez-fate/pilecode.svg
	mkdir -p `dirname $@`
	$(INKSCAPE) -e `pwd`/$@ -w 1680 -h 1050 `pwd`/$<

$(DATA)/%.tga: $(OBJ)/%.png
	$(CONVERT) $< $@
