all : main
main : main.cc
	g++ -o $@ $^ --std=c++17 -lcpr -lelasticlient

.PHONY : clean
clean:
	rm -f main