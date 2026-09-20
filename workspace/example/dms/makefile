DMS_INSTALL := ../../DMS/install

all : test

test : test.cc
	g++ -std=c++17 -Wall -Wextra $^ -o $@ \
		-isystem $(DMS_INSTALL)/include \
		-I../../common \
		-L$(DMS_INSTALL)/lib \
		-lalibabacloud_dypnsapi_20170525 \
		-lalibabacloud_open_api_v2 \
		-lalibabacloud_credentials \
		-ldarabonba_core \
		-lcurl -lssl -lcrypto -luuid \
		-lspdlog -lfmt -lgflags \
		-Wl,-rpath,$(DMS_INSTALL)/lib

.PHONY : clean
clean:
	rm -rf test
