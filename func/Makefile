
CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++17
ifeq ($(SYSTEM),Darwin)
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -pthread\
           -lgrpc++_reflection\
           -ldl
else
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -lglog\
           -ldl
endif
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = ../protos/

GTEST = -lgtest -lgtest_main

vpath %.proto $(PROTOS_PATH)

all: func_server warble_funcs_unittest

func_server: func.pb.o func.grpc.pb.o func_server.o kvstore.pb.o kvstore.grpc.pb.o kvstore_client.o entity.pb.o entity.grpc.pb.o warble.pb.o warble.grpc.pb.o warble_funcs.o
	$(CXX) $^ $(LDFLAGS) -o $@

func_client: func.pb.o func.grpc.pb.o func_client.o
	$(CXX) $^ $(LDFLAGS) -o $@

warble_funcs_unittest: entity.pb.o entity.grpc.pb.o func.pb.o func.grpc.pb.o warble.pb.o warble.grpc.pb.o kvstore.pb.o kvstore.grpc.pb.o kvstore_client.o warble_funcs.o warble_funcs_unittest.o
	$(CXX) $^ $(LDFLAGS) $(GTEST) -o $@

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h func_client func_server warble_funcs_unittest