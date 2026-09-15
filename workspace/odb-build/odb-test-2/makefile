main : main.cc student-odb.cxx
	g++ -std=c++17 $^ -o $@ -lgflags -lodb-mysql -lodb -lodb-boost

.PHONY : clean
clean:
	rm -rf main