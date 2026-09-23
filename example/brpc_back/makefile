all : server client
server : server.cc main.pb.cc
	g++ -o $@ $^ --std=c++17 -lbrpc -lgflags -lssl -lcrypto -lprotobuf -lleveldb
client : client.cc main.pb.cc
	g++ -o $@ $^ --std=c++17 -lbrpc -lgflags -lssl -lcrypto -lprotobuf -lleveldb

.PHONY:clean
clean:
	rm -rf server client