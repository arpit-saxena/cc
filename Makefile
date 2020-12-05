# Part of Makefile taken from https://stackoverflow.com/a/30142139/5585431

ENTRY=2018MT10742

CXX = g++
CXX_FLAGS = $(shell llvm-config --cxxflags --ldflags --libs core) -g -std=c++17 -lm -lfl -lpopt

# Final binary
BIN = cc
# Put all auto generated stuff to this build dir.
BUILD_DIR = ./build

# List of all .cpp source files.
CPP = $(wildcard *.cpp)

# All .o files go to build dir.
OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)
# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

# Actual target of the binary - depends on all .o files.
$(BIN) : c.tab.cpp c.tab.hpp c.lex.cpp $(OBJ)
	$(CXX) $(CXX_FLAGS) $(OBJ) -o $@

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

c.tab.cpp c.tab.hpp: c.y
	bison -o c.tab.cpp -d c.y

c.lex.cpp: c.l c.tab.hpp
	flex -o c.lex.cpp -l c.l

clean::
	rm -f c.tab.cpp c.tab.hpp c.lex.cpp cc c.output $(ENTRY).zip
	rm -rf build

FORCE:

src: FORCE
	@mkdir -p src
	@rsync -a --exclude-from=exclude-list * src/

handin: src
	@mv cc cc.bak
	@mv src cc
	@rm -f $(ENTRY).zip
	@zip -r $(ENTRY).zip cc/
	@rm -r cc
	@mv cc.bak cc

extract:
	@rm -rf submission/
	@unzip $(ENTRY).zip -d submission