# JustCoderdev Makefile for C projects v4* (corrupted?)

PROGRAM_NAME = Hasher

SRC_FILES = main.c
OBJ_FILES = $(patsubst %.c,%.o,${SRC_FILES})
SHS_FILES = shs.c
CORE_FILES = lib/core/core_bit.c \
		 lib/core/core_buff.c \
		 lib/core/core_net.c \
		 lib/core/core_str.c \
		 lib/core/core_memdeb.c \
		 lib/core/core_logger.c

CC = gcc
CCFLAGS = -xc -std=c89 -Wall -Wextra -Werror \
		 -g -pedantic-errors -pedantic
#-Wno-unused
IFLAGS = -I./ -I./lib/include
LDFLAGS = -L./
#RAYFLAGS = -lGL -lm -lpthread -ldl -lX11 \
#		-Wl,-rpath=./lib/raylib/ -L./lib/raylib -lraylib
##-lXrandr -lXinerama -lXi -lXcursor

#-DDEBUG_MEMDEB_ENABLE=1
#-DDEBUG_STRING_ENABLE=1
# -DDEBUG_ENABLE=1
DFLAGS =
FLAGS = $(CCFLAGS) $(IFLAGS) $(LDFLAGS) $(RAYFLAGS) $(DFLAGS)

local: build run
#build_so:
#	@echo "Compiling... (auto)"
#	@mkdir -p bin
#	$(CC) $(CCFLAGS) $(IFLAGS) $(RAYFLAGS) $(DFLAGS) \
#		-fPIC -shared hotfile.c $(CORE_FILES) -o bin/libhotfile.so

build:
	@echo "Compiling... "
	@mkdir -p bin
	$(CC) $(FLAGS) $(SRC_FILES) $(CORE_FILES) $(SHS_FILES) -o bin/${PROGRAM_NAME}

local_test: build_test run_test
build_test:
	@echo "Compiling... (test)"
	@mkdir -p bin
	@cp -r test bin/test
	$(CC) $(FLAGS) test.c $(SHS_FILES) $(CORE_FILES) -o bin/${PROGRAM_NAME}_test

run_test:
	@echo -e "Running... (test)\n"
	@chmod +x bin/$(PROGRAM_NAME)_test
	@cd bin && ./$(PROGRAM_NAME)_test


.PHONY: clean
clean:
	@echo "Cleaning..."
	@rm -rf ./bin


# raycompile: rayclean raybuild
# rayclean:
# 	cd lib/sources/raylib/src && make clean

# raybuild:
# 	cd lib/sources/raylib/src && make PLATFORM=PLATFORM_DESKTOP GLFW_LINUX_ENABLE_WAYLAND=OFF

# rayshared:
# 	cd lib/sources/raylib/src && make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED GLFW_LINUX_ENABLE_WAYLAND=OFF
