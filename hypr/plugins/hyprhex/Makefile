CXX ?= g++
CXXFLAGS ?= -O2
CXXFLAGS += -shared -fPIC -std=c++23 -Wall -Wextra -Wpedantic -Wno-unused-parameter

ifeq ($(CXX),g++)
    CXXFLAGS += -fno-gnu-unique
endif

PKG_CFLAGS := $(shell pkg-config --cflags hyprland pixman-1 libdrm pangocairo libinput libudev wayland-server xkbcommon)

.PHONY: all clean

all: hyprhex.so

hyprhex.so: main.cpp
	$(CXX) $(CXXFLAGS) $(PKG_CFLAGS) $< -o $@ -lGLESv2

clean:
	rm -f hyprhex.so
