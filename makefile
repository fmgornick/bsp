INCS=-I/opt/homebrew/Cellar/raylib/5.5/include -Iinclude
LIBS=-L/opt/homebrew/Cellar/raylib/5.5/lib -lraylib

SRCS=$(wildcard src/*.c)
OBJS=$(patsubst src/%.c,obj/%.o,$(SRCS))
WEBOBJS=$(patsubst src/%.c,obj/web_%.o,$(SRCS))

# main executable
bsp: $(OBJS)
	clang -o $@ $^ $(LIBS)

# run simple script file(s)
# (manually rename dependency)
# (used for fast iteration)
.PHONY: scratch
scratch: scratch/blah.c
	clang -o scratchexe -g -Wall $(INCS) $< $(LIBS)
	./scratchexe

# runs main executable on web
wasm: $(WEBOBJS)
	emcc -o web/game.html $^ lib/libraylib.a -s USE_GLFW=3 -s ASYNCIFY --shell-file web/shell.html
	@echo "\n\033[01;32m  URL: http://localhost:8442/game.html  \033[00m\n"
	@python3 -m http.server --directory web 8442

# run simple script file(s) on web
# (manually rename dependency)
wscratch: scratch/blah.c
	emcc -o web/game.html $< $(INCS) -Os -Wall lib/libraylib.a -s USE_GLFW=3 -s ASYNCIFY --shell-file web/shell.html
	@echo "\n\033[01;32m  URL: http://localhost:6969/game.html  \033[00m\n"
	@python3 -m http.server --directory web 6969

# run raylib example files on web (usage: make ex<example number>)
ex%: examples/%.c
	@mkdir -p example
	emcc -o example/game.html $< -Os -Wall lib/libraylib.a -s USE_GLFW=3 -s ASYNCIFY --shell-file web/shell.html
	@echo "\n\033[01;32m  URL: http://localhost:8080/game.html  \033[00m\n"
	@python3 -m http.server --directory example 8080

obj/%.o: src/%.c
	@mkdir -p obj
	clang -c -g -Wall $(INCS) $< -o $@

obj/web_%.o: src/%.c
	@mkdir -p obj
	emcc $(INCS) -Os -Wall -c $< -o $@

# memory check
check: bsp
	leaks --atExit -q -- ./bsp

# removes...
# 1. object files
# 2. web object files
# 3. emcc outputted html/json/wasm files
# 4. executables
# 5. any '.dSYM' files (mac prollems :/)
clean:
	@git clean -dfX
