all : main
main : main.cc
	g++ -o $@ $^ --std=c++17 -L/usr/local/lib -ljsoncpp 

.PHONY : clean
clean:
	rm -f main