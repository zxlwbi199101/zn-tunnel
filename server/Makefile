CXX = g++
CXXFLAGS += \
	-DBOOST_ALL_DYN_LINK \
	-std=c++11 \
	-I../libtun/include

LDFLAGS += \
	-lboost_log_setup-mt \
	-lboost_log-mt \
	-lboost_filesystem-mt \
	-lboost_thread-mt \
	-lboost_system-mt \
	-lboost_date_time-mt \
	-lboost_regex-mt \
	-lfmt

SOURCES = $(wildcard *.cc)
OBJS = $(addsuffix .o, $(basename $(SOURCES)))

compile: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o server.o

run: compile
	sudo ./server.o

clean:
	rm -f $(OBJS) server.o