# ═══════════════════════════════════════════════════════════
#  NEON TETRIS — Cross-Platform Build System
#  Requires: gcc/cc, GLFW3, GLEW, OpenGL, OpenAL
#  Supported: macOS, Linux (Ubuntu/Fedora/Arch), Windows (MSYS2)
# ═══════════════════════════════════════════════════════════

CC       = gcc
CFLAGS   = -Wall -Wextra -O2 -std=c11 -I./include
LDFLAGS  =
TARGET   = neon_tetris

# ─── Detect platform ───
ifeq ($(OS), Windows_NT)
    # ── Windows — MSYS2 MinGW64 / UCRT64 / CLANG64 ──
    TARGET  := $(TARGET).exe
    LDFLAGS += -lglfw3 -lglew32 -lopengl32 -lopenal -lgdi32 -luser32 -lshell32 -limm32 -lwinmm -lm
    RM       = del /f /q 2>nul || rm -f
else
    UNAME := $(shell uname -s)
    ifeq ($(UNAME), Darwin)
        # ── macOS — Homebrew paths, system frameworks ──
        BREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo /opt/homebrew)
        CFLAGS  += -I$(BREW_PREFIX)/include
        LDFLAGS += -L$(BREW_PREFIX)/lib -lglfw -lGLEW -framework OpenGL -framework OpenAL
    else
        # ── Linux (Ubuntu, Fedora, Arch, etc.) ──
        LDFLAGS += -lglfw -lGLEW -lGL -lopenal -lm
    endif
    RM = rm -f
endif

# Source files
SRC = src/main.c

# ─────────── Targets ───────────

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC) include/tetris.h include/shaders.h include/sound.h
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) neon_tetris neon_tetris.exe src/main.o

# ─────────── Dependency Installation ───────────

.PHONY: install-deps-ubuntu install-deps-fedora install-deps-arch install-deps-mac install-deps-msys2

install-deps-ubuntu:
	sudo apt-get update
	sudo apt-get install -y libglfw3-dev libglew-dev libgl-dev libopenal-dev

install-deps-fedora:
	sudo dnf install -y glfw-devel glew-devel mesa-libGL-devel openal-soft-devel

install-deps-arch:
	sudo pacman -S --needed glfw-x11 glew mesa openal

install-deps-mac:
	brew install glfw glew

install-deps-msys2:
	pacman -S --needed ${MINGW_PACKAGE_PREFIX}-gcc ${MINGW_PACKAGE_PREFIX}-glfw ${MINGW_PACKAGE_PREFIX}-glew ${MINGW_PACKAGE_PREFIX}-openal
