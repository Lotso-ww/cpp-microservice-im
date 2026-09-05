all : main
main : main.cc
	g++ -o $@ $^ --std=c++17 -lcpr -lelasticlient -lspdlog -lfmt -lgflags /usr/local/lib/libjsoncpp.so.19

.PHONY : clean
clean:
	rm -f main