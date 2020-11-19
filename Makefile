ENTRY=2018MT10742

cc: c.tab.cpp c.tab.hpp c.lex.cpp *.cpp *.hpp 
	g++ *.cpp -std=c++17 -lm -lfl -o $@ -g

c.tab.cpp c.tab.hpp: c.y
	bison -o c.tab.cpp -d c.y

c.lex.cpp: c.l c.tab.hpp
	flex -o c.lex.cpp -l c.l

clean::
	rm -f c.tab.cpp c.tab.hpp c.lex.cpp cc c.output $(ENTRY).zip

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