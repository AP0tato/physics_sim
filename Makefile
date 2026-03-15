cc := g++
build := build
SDL3_DIR := SDL/build
src := $(wildcard src/*.cpp)
obj := $(patsubst src/%.cpp, $(build)/%.o, $(src))
dep := $(obj:.o=.d)
target := $(build)/main
includes := -I src/inc -I SDL/include
flags := -std=c++17 -g -O0 -Wall -Wextra -pedantic $(includes) -L $(SDL3_DIR) -l SDL3 -Wl,-rpath,@executable_path/../SDL/build
depflags = -MMD -MP -MF $(build)/$*.d

-include $(dep)

.PHONY: all clean run

run: $(target)
	./$(target)

all: $(target)

$(target): $(obj)
	$(cc) $(flags) -o $@ $^

$(build)/%.o: src/%.cpp | $(build)
	$(cc) $(flags) $(depflags) -c $< -o $@

$(build):
	mkdir -p $@

clean:
	rm -rf $(build)