all : main
main : main.cc
	g++ -std=c++17 $^ -o $@ -lpthread -lboost_system

.PHONY : clean
clean:
	rm -rf main