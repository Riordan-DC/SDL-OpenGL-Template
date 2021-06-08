CC = gcc
LD ?= gcc

VERSION = 0.1
BIN_NAME = SDL_OpenGL_Project-$(VERSION)
DIR = build
OBJDIR = ${DIR}/obj
BINDIR = ${DIR}/bin

#SHELL = /bin/bash

CPPFLAGS := # C Preprocessor Flags
LDFLAGS := -lSDL2 # Libraries

CFLAGS = -g -Wall -std=gnu99 # C Compiler Flags
INCLUDE = -Iinclude # Include directories
SRC = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)
OBJS = $(patsubst src/%.o, $(OBJDIR)/%.o, $(SRC:.c=.o))

PLATFORM := $(shell uname)

ifeq ($(findstring Linux, $(PLATFORM)), Linux)
	# Platform detected as Linux
endif

ifeq ($(findstring MINGW, $(PLATFORM)), MINGW)
	# Platform detected as Windows (Using MinGW)
endif

ifeq ($(findstring Darwin, $(PLATFORM)), Darwin)
	# Platform detected as MacOS (Darwin)
	LDFLAGS += -framework OpenGL
endif


all: init ${BINDIR}/${BIN_NAME}
	@echo All Done

${BINDIR}/${BIN_NAME}: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

init:
	@echo Platform detected as $(PLATFORM)
	mkdir -p $(DIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)
	@echo $(SRC)
	@echo $(OBJS)
	@echo Init Done

clean:
	rm -fr $(DIR)

install:
	#@cp -p foo ${BINDIR}/foo
	@echo Install Done

$(OBJDIR)/%.o: src/%.c
	$(CC) $(INCLUDE) -c $(CFLAGS) $< -o $@

.PHONY: clean install init
