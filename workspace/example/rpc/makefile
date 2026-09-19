all:registry discovery
registry:registry.cc ../brpc_back/main.pb.cc
	g++ -o $@ $^ --std=c++17 -letcd-cpp-api -lcpprest -lspdlog -lfmt -lgflags -lbrpc -lssl -lcrypto -lprotobuf -lleveldb
discovery:discovery.cc ../brpc_back/main.pb.cc
	g++ -o $@ $^ --std=c++17 -letcd-cpp-api -lcpprest -lspdlog -lfmt -lgflags -lbrpc -lssl -lcrypto -lprotobuf -lleveldb

.PHONY:clean
clean:
	rm -rf registry discovery
