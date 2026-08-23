all:sync async
sync::sync.cc
	g++ -o $@ $^ --std=c++17 -lspdlog -lfmt
async::async.cc
	g++ -o $@ $^ --std=c++17 -lspdlog -lfmt

.PHONY:clean
clean:
	rm -rf sync async sync.log
