test : test.cc
	g++ -std=c++17 $^ -o $@ -lcurl -lcrypto /usr/local/lib/libjsoncpp.so.19 -lfmt -lgflags

.PHONY : clean
clean:
	rm -rf test