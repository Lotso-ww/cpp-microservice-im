test : test.cc
	g++ -std=c++17 $^ -o $@ -lcurl -lcrypto /usr/local/lib/libjsoncpp.so.19

.PHONY : clean
clean:
	rm -rf test