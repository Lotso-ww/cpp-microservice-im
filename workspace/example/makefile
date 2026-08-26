all:registry discovery
registry:registry.cc
	g++ -o $@ $^ --std=c++17 -letcd-cpp-api -lcpprest -lspdlog -lfmt -lgflags
discovery:discovery.cc
	g++ -o $@ $^ --std=c++17 -letcd-cpp-api -lcpprest -lspdlog -lfmt -lgflags

.PHONY:clean
clean:
	rm -rf registry discovery
