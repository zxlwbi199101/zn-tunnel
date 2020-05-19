HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)

CXX = g++
CXXFLAGS += \
	-DBOOST_ALL_DYN_LINK \
	-std=c++2a \
	-fcoroutines-ts \
	-I./include

LDFLAGS += \
	-lboost_log_setup-mt \
	-lboost_log-mt \
	-lboost_filesystem-mt \
	-lboost_thread-mt \
	-lboost_system-mt \
	-lboost_date_time-mt \
	-lboost_regex-mt

ifeq ($(SYSTEM), Darwin)
else
endif

# include third party headers and source code
include ../third_party/Makefile

TEST_SOURCES = $(wildcard test/*.cc test/**/*.cc) third_party_objs
# TEST_SOURCES = test/main.cc test/transmission/Rpc.cc
TEST_OBJS = $(patsubst %cc, %o, $(TEST_SOURCES))

test: test-compile
	./test.o

test-compile: $(TEST_OBJS)
	$(CXX) $(LDFLAGS) $(TEST_OBJS) -o test.o

clean:
	rm -rf $(TEST_OBJS) test.o

rm:
	rm test/transmission/Rpc.o