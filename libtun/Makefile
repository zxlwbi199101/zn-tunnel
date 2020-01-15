HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)

CXX = g++
CXXFLAGS += \
	-DBOOST_LOG_DYN_LINK \
	-std=c++11 \
	-I./include

LDFLAGS += \
	-lboost_log_setup-mt \
	-lboost_log-mt \
	-lboost_filesystem-mt \
	-lboost_thread-mt \
	-lboost_system-mt \
	-lboost_date_time-mt \
	-lboost_regex-mt \
	-lfmt

ifeq ($(SYSTEM),Darwin)
else
endif

SOURCES = \
	$(wildcard src/*.cc) \
	src/Tunnel/Tunnel.cc

OBJS = $(addsuffix .o, $(basename $(SOURCES)))

libtun: $(OBJS)
	mkdir -p lib
	ar rcv lib/libtun.a $(OBJS)

clean:
	rm -r $(OBJS) lib
