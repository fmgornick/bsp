FLAGS=-std=c11 -O3 -Wall -Iinc -I/usr/local/include/raylib
EMCC_FLAGS=-s USE_GLFW=3 -s ASYNCIFY --shell-file web/shell.html
SANITIZE=-fsanitize=address
# SANITIZE=

SRCS=$(wildcard src/*.c)
OBJS=$(patsubst src/%.c,obj/%.o,$(SRCS))
WEBOBJS=$(patsubst src/%.c,obj/web_%.o,$(SRCS))

# main executable
bsp: $(OBJS)
	clang $(SANITIZE) -o $@ -Iinc $^ -lraylib

# runs main executable on web
wasm: $(WEBOBJS)
	emcc -o web/game.html $^ lib/libraylib.a $(EMCC_FLAGS)
	@echo "\n\033[01;32m  URL: http://localhost:8442/game.html  \033[00m\n"
	@python3 -m http.server --directory web 8442

# run raylib example files on web (usage: make ex<example number>)
ex%: examples/%.c
	@mkdir -p example
	clang -o example/$@ $(FLAGS) $(SANITIZE) $< -lraylib
	@example/$@

obj/%.o: src/%.c
	@mkdir -p obj
	clang -c $(FLAGS) $(SANITIZE) $< -o $@

obj/web_%.o: src/%.c
	@mkdir -p obj
	emcc -c $(FLAGS) $< -o $@

# memory check
check: bsp
	MallocStackLogging=YES leaks --atExit -q -- ./bsp

# all unwanted files listed in .gitignore
clean:
	@git clean -dfX
