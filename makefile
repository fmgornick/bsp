FLAGS=-std=c99 -g -Wall -Iinclude -I/usr/local/include/raylib
EMCC_FLAGS=-s USE_GLFW=3 -s ASYNCIFY --shell-file web/shell.html

SRCS=$(wildcard src/*.c)
OBJS=$(patsubst src/%.c,obj/%.o,$(SRCS))
WEBOBJS=$(patsubst src/%.c,obj/web_%.o,$(SRCS))

# main executable
bsp: $(OBJS)
	clang -o $@ -Iinclude $^ -lraylib

# runs main executable on web
wasm: $(WEBOBJS)
	emcc -o web/game.html $^ lib/libraylib.a $(EMCC_FLAGS)
	@echo "\n\033[01;32m  URL: http://localhost:8442/game.html  \033[00m\n"
	@python3 -m http.server --directory web 8442

# run raylib example files on web (usage: make ex<example number>)
ex%: examples/%.c
	@mkdir -p example
	clang -o example/$@ $(FLAGS) $< -lraylib
	@example/$@

obj/%.o: src/%.c
	@mkdir -p obj
	clang -c $(FLAGS) $< -o $@

obj/web_%.o: src/%.c
	@mkdir -p obj
	emcc -c $(FLAGS) $< -o $@

# memory check
check: bsp
	leaks --atExit -q -- ./bsp

# all unwanted files listed in .gitignore
clean:
	@git clean -dfX
