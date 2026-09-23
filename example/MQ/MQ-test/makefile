all : publish consume
publish : publish.cc
	g++ -std=c++17 $^ -o $@ -lamqpcpp -lev
consume : consume.cc
	g++ -std=c++17 $^ -o $@ -lamqpcpp -lev

.PHONY : clean
clean:
	rm -rf publish consume