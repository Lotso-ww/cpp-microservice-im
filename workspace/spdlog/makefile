all:sync async main
main:main.cc
	g++ -o $@ $^ --std=c++17 -lspdlog -lfmt -lgflags
sync:sync.cc
	g++ -o $@ $^ --std=c++17 -lspdlog -lfmt
async:async.cc
	g++ -o $@ $^ --std=c++17 -lspdlog -lfmt

.PHONY:clean
clean:
	rm -rf main sync async sync.log
