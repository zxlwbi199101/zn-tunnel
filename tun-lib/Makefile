HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)

CXX = g++
CXXFLAGS += -std=c++11

LDFLAGS += -DBOOST_LOG_DYN_LINK=1 -std=c++11

ifeq ($(SYSTEM),Darwin)
LDFLAGS += \
					 -lboost_log_setup-mt\
					 -lboost_log-mt\
					 -lboost_filesystem-mt\
					 -lboost_thread-mt\
					 -lboost_system-mt\
					 -lboost_date_time-mt\
					 -lboost_regex-mt\
					 -lfmt
else
LDFLAGS += -lboost_log-mt\
					 -lboost_log_setup-mt\
					 -lboost_filesystem-mt\
					 -lboost_thread-mt\
					 -lboost_system\
					 -lboost_regex-mt
endif

tun-lib: src/tun/Tun.cc src/logger.cc src/main.cc
	$(CXX) $^ $(LDFLAGS) -o $@.o

run: tun-lib
	sudo ./tun-lib.o