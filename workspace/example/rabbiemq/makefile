all : publish consume
publish : publish.cc
	g++ -std=c++17 $^ -o $@ -lamqpcpp -lev -lfmt -lspdlog -lgflags
consume : consume.cc
	g++ -std=c++17 $^ -o $@ -lamqpcpp -lev -lfmt -lspdlog -lgflags

.PHONY : clean
clean:
	rm -rf publish consume